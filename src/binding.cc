#define NAPI_CPP_EXCEPTIONS

#include <Windows.h>
#include "napi.h"
#include "utils.h"
#include <iostream>

using namespace Napi;

class PrinterHandle {
  public:
    PrinterHandle() {}

    PrinterHandle(const wchar_t* printerName) {
      open(printerName);
    }

    PrinterHandle(const PrinterHandle& other) = delete;

    PrinterHandle& operator=(PrinterHandle& other) {
      if (this == &other) {
        return *this;
      }

      m_handle = other.m_handle;
      other.m_handle = nullptr;

      return *this;
    }

    HANDLE get() const {
      return m_handle;
    }
  
    ~PrinterHandle() {
      if (m_handle != nullptr) {
        ClosePrinter(m_handle);
      }
    }
  private:
    HANDLE m_handle;

    inline void open(const wchar_t* printerName) {
      BOOL success = OpenPrinter2W(printerName, &m_handle, nullptr, nullptr);
      if (!success) {
        throw std::runtime_error(getLastErrorString());
      }
    }
};

class DocProps {
  public:
    enum DocumentOrientation {
      PORTRAIT,
      LANDSCAPE
    };

    DocProps(PDEVMODEW devMode) {
      setCopies(devMode->dmCopies);
      setOrientation(devMode->dmOrientation == DMORIENT_PORTRAIT ? DocumentOrientation::PORTRAIT : DocumentOrientation::LANDSCAPE);
      setPaperWidth((double)devMode->dmPaperWidth / 10);
      setPaperHeight((double)devMode->dmPaperLength / 10);
      setScale((double)devMode->dmScale / 100);
      setDPI((double)devMode->dmPrintQuality);
    }

    inline double getOrientation() const {
      return m_orientation;
    }

    inline double getPaperWidth() const {
      return m_paperWidth;
    }

    inline double getPaperHeight() const {
      return m_paperHeight;
    }

    inline double getScale() const {
      return m_scale;
    }

    inline double getDPI() const {
      return m_dpi;
    }

    inline int getCopies() const {
      return m_copies;
    }

    inline void setCopies(unsigned int copies) {
      m_copies = copies;
    }

    inline void setOrientation(DocumentOrientation orientation) {
      m_orientation = orientation;
    }

    inline void setPaperWidth(double paperWidth) {
      m_paperWidth = paperWidth > 0 ? paperWidth : 0;
    }

    inline void setPaperHeight(double paperHeight) {
      m_paperHeight = paperHeight > 0 ? paperHeight : 0;
    }

    inline void setScale(double scale) {
      m_scale = scale > 0 ? scale : 0;
    }

    inline void setDPI(double dpi) {
      m_dpi = dpi > 0 ? dpi : 0;
    }

    Object toObject(napi_env env) {
      Object ret = Object::New(env);

      ret.Set("copies", m_copies);
      ret.Set("orientation", 
        m_orientation == DocumentOrientation::PORTRAIT ?
          String::From(env, "portrait") : String::From(env, "landscape")
      );
      ret.Set("paperWidth", m_paperWidth);
      ret.Set("paperHeight", m_paperHeight);
      ret.Set("scale", m_scale);
      ret.Set("dpi", m_dpi);

      return ret;
    }
  private:
    DocumentOrientation m_orientation;
    unsigned int m_copies;
    double m_paperWidth;
    double m_paperHeight;
    double m_scale;
    double m_dpi;
};

class MyAddon : public Addon<MyAddon> {
  public:
    MyAddon(Env env, Object exports) {
      DefineAddon(exports, {
          InstanceMethod("message", &MyAddon::messageBox),
          InstanceMethod("enumPrinters", &MyAddon::enumPrinters),
          InstanceMethod("getDocumentProperties", &MyAddon::getDocumentProperties)
      });
    }

  private:
    Value messageBox(const CallbackInfo& info) {
      auto str = info[0];

      if (!str.IsString()) {
        napi_throw_type_error(info.Env(), nullptr, "Message must be a string");
      } else {
        std::u16string message = str.ToString().Utf16Value();
        MessageBoxW(nullptr, std::wstring(message.begin(), message.end()).c_str(), L"Message", MB_OK);
      }

      return info.Env().Undefined();
    }

    Value getDocumentProperties(const CallbackInfo& info) {
      auto arg = info[0];
      if (!arg.IsString()) {
        napi_throw_type_error(info.Env(), nullptr, "Printer name must be a string");
        return info.Env().Undefined();
      }

      PrinterHandle printerHandle;
      std::wstring printerName = toWString(arg.ToString().Utf16Value());

      try {
        printerHandle = PrinterHandle(printerName.c_str());
      } catch (const std::runtime_error& error) {
        throw Error::New(info.Env(), error.what());
      }

      DWORD dwPSize;

      BOOL success = GetPrinterW(
        printerHandle.get(),
        2,
        nullptr,
        0,
        &dwPSize
      );

      std::unique_ptr<unsigned char[]> pInfoBuf(new unsigned char[dwPSize]);

      success = GetPrinterW(
        printerHandle.get(),
        2,
        pInfoBuf.get(),
        dwPSize,
        &dwPSize
      );

      if (!success) {
        throw Error::New(info.Env(), getLastErrorString());
      }

      PRINTER_INFO_2W* printerInfo = (PRINTER_INFO_2W*)pInfoBuf.get();

      LONG result = DocumentPropertiesW(
        nullptr,
        printerHandle.get(),
        (LPWSTR)printerName.c_str(),
        printerInfo->pDevMode,
        printerInfo->pDevMode,
        DM_IN_PROMPT | DM_OUT_BUFFER
      );

      if (result < 0) {
        throw Error::New(info.Env(), getLastErrorString());
      }

      return DocProps(printerInfo->pDevMode).toObject(info.Env());
    }

    Value enumPrinters(const CallbackInfo& info) {
      DWORD length;
      DWORD required;
      DWORD bufSize;

      BOOL success = EnumPrintersW(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, // local printers only
        nullptr, // get all printers
        1,
        nullptr,
        0,
        &required,
        &length
      );

      bufSize = required;
      std::unique_ptr<unsigned char[]> printerBuf(new unsigned char[required]);

      success = EnumPrintersW(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, // local printers only
        nullptr, // get all printers
        1,
        (LPBYTE)printerBuf.get(),
        bufSize,
        &required,
        &length
      );

      if (!success) {
        throw Error::New(info.Env(), getLastErrorString());
      }

      auto ret = Array::New(info.Env(), length);

      for (DWORD i = 0; i < length; i++) {
        LPPRINTER_INFO_1W printer = (LPPRINTER_INFO_1W)(printerBuf.get() + i * sizeof(PRINTER_INFO_1W));
        ret.Set(i, String::New(info.Env(), std::u16string((char16_t*) printer->pName)));
      }

      return ret;
    }
};


NODE_API_ADDON(MyAddon)
