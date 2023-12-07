#include <Windows.h>
#include "napi.h"
#include "utils.h"
#include <iostream>

using namespace Napi;

class DocProps {
  public:
    enum DocumentOrientation {
      PORTRAIT,
      LANDSCAPE
    };

    DocProps() {}

    DocProps(PDEVMODEW devMode) : m_devMode(devMode) {
      updateDevMode();
    }

    double getOrientation() const {
      return m_orientation;
    }

    double getPaperWidth() const {
      return m_paperWidth;
    }

    double getPaperHeight() const {
      return m_paperHeight;
    }

    double getScale() const {
      return m_scale;
    }

    double getDPI() const {
      return m_dpi;
    }

    int getCopies() const {
      return m_copies;
    }

    PDEVMODEW getDevMode() const {
      return m_devMode;
    }

    void setCopies(short copies) {
      if (m_devMode == nullptr) return;

      m_copies = copies > 0 ? copies : 0;
      m_devMode->dmCopies = m_copies;
    }

    void setOrientation(DocumentOrientation orientation) {
      if (m_devMode == nullptr) return;

      m_orientation = orientation;
      m_devMode->dmOrientation = m_orientation == DocumentOrientation::PORTRAIT ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
    }

    void setPaperWidth(double paperWidth) {
      if (m_devMode == nullptr) return;

      m_paperWidth = paperWidth > 0 ? paperWidth : 0;
      m_devMode->dmPaperWidth = (short)(m_paperWidth * 10);
    }

    void setPaperHeight(double paperHeight) {
      if (m_devMode == nullptr) return;

      m_paperHeight = paperHeight > 0 ? paperHeight : 0;
      m_devMode->dmPaperLength = (short)(m_paperHeight * 10);
    }

    void setScale(double scale) {
      if (m_devMode == nullptr) return;

      m_scale = scale > 0 ? scale : 0;
      m_devMode->dmScale = (short)(m_scale * 100);
    }

    void setDPI(double dpi) {
      if (m_devMode == nullptr) return;

      m_dpi = dpi > 0 ? dpi : 0;
      m_devMode->dmPrintQuality = (short)m_dpi;
    }

    void fromObject(const Object& object) {
      if (object.Has("copies")) {
        setCopies(object.Get("copies").ToNumber().Int32Value());
      }
      
      if (object.Has("orientation")) {
        setOrientation(
          object.Get("orientation").ToString().Utf8Value() == "portrait" ?
            DocumentOrientation::PORTRAIT : DocumentOrientation::LANDSCAPE
        );
      }

      if (object.Has("paperWidth")) {
        setPaperWidth(object.Get("paperWidth").ToNumber().DoubleValue());
      }
      
      if (object.Has("paperHeight")) {
        setPaperHeight(object.Get("paperHeight").ToNumber().DoubleValue());
      }
      
      if (object.Has("scale")) {
        setScale(object.Get("scale").ToNumber().DoubleValue());
      }

      if (object.Has("dpi")) {
        setDPI(object.Get("dpi").ToNumber().DoubleValue());
      }
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
    PDEVMODEW m_devMode = nullptr;

    DocumentOrientation m_orientation = DocumentOrientation::PORTRAIT;
    short m_copies;
    double m_paperWidth;
    double m_paperHeight;
    double m_scale;
    double m_dpi;

    void updateDevMode() {
      setCopies(m_devMode->dmCopies);
      setOrientation(m_devMode->dmOrientation == DMORIENT_PORTRAIT ? DocumentOrientation::PORTRAIT : DocumentOrientation::LANDSCAPE);
      setPaperWidth((double)m_devMode->dmPaperWidth / 10);
      setPaperHeight((double)m_devMode->dmPaperLength / 10);
      setScale((double)m_devMode->dmScale / 100);
      setDPI((double)m_devMode->dmPrintQuality);
    }

  friend class Printer;
};

class Printer : public ObjectWrap<Printer> {
  public:
    static Object Init(Napi::Env env, Object exports) {
      Function func = DefineClass(env, "Printer", {
          InstanceMethod<&Printer::getProperties>("getProperties", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
          InstanceMethod<&Printer::setProperties>("setProperties", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
          InstanceMethod<&Printer::chooseProperties>("chooseProperties", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
      });

      FunctionReference *constructor = new FunctionReference();

      *constructor = Persistent(func);
      exports.Set("Printer", func);

      env.SetInstanceData<FunctionReference>(constructor);
      return exports;
    }

    Printer(const CallbackInfo& info) : ObjectWrap<Printer>(info) {
      auto printerName = info[0];
      if (!printerName.IsString()) {
        typeError(info.Env(), "printerName must be a string");
        return;
      }

      m_printerName = toWString(printerName.ToString().Utf16Value());

      try {
        open();
      } catch (const std::runtime_error& e) {
        throw Error::New(info.Env(), e.what());
      }
    }

    Printer(const Printer& other) = delete;

    Printer& operator=(Printer& other) {
      if (this == &other) {
        return *this;
      }

      m_handle = other.m_handle;
      other.m_handle = nullptr;

      m_pInfoBuf = other.m_pInfoBuf;
      other.m_pInfoBuf = nullptr;

      return *this;
    }

    Napi::Value getProperties(const CallbackInfo& info) {
      return m_props.toObject(info.Env());
    }

    Napi::Value setProperties(const CallbackInfo& info) {
      if (!info[0].IsObject()) {
        return typeError(info.Env(), "properties isn't an object");
      }

      m_props.fromObject(info[0].ToObject());
      return info.Env().Undefined();
    }

    Napi::Value chooseProperties(const CallbackInfo& info) {
      LONG result = DocumentPropertiesW(
        nullptr,
        m_handle,
        (LPWSTR)m_printerName.c_str(),
        m_props.getDevMode(),
        m_props.getDevMode(),
        DM_IN_BUFFER | DM_IN_PROMPT | DM_OUT_BUFFER
      );

      if (result < 0) {
        throw Error::New(info.Env(), getLastErrorString());
      }

      m_props.updateDevMode();

      return info.Env().Undefined();
    }
  
    ~Printer() {
      if (m_handle != nullptr) {
        ClosePrinter(m_handle);
      }

      if (m_pInfoBuf != nullptr) {
        delete[] m_pInfoBuf;
      }
    }
  private:
    HANDLE m_handle;
    DocProps m_props;
    std::wstring m_printerName;
    unsigned char* m_pInfoBuf = nullptr;

    void open() {
      BOOL success = OpenPrinter2W(m_printerName.c_str(), &m_handle, nullptr, nullptr);
      if (!success) {
        throw std::runtime_error(getLastErrorString());
      }

      readPrinterInfo();
    }

    void readPrinterInfo() {
      DWORD dwPSize;

      BOOL success = GetPrinterW(
        m_handle,
        2,
        nullptr,
        0,
        &dwPSize
      );

      if (m_pInfoBuf != nullptr) {
        delete[] m_pInfoBuf;
      }

      m_pInfoBuf = new unsigned char[dwPSize];

      success = GetPrinterW(
        m_handle,
        2,
        m_pInfoBuf,
        dwPSize,
        &dwPSize
      );

      if (!success) {
        throw std::runtime_error(getLastErrorString());
      }

      PRINTER_INFO_2W* infoPtr = reinterpret_cast<PRINTER_INFO_2W*>(m_pInfoBuf);
      m_props = DocProps(infoPtr->pDevMode);
    }
};

class PrinterAddon : public Addon<PrinterAddon> {
  public:
    PrinterAddon(Env env, Object exports) {
      DefineAddon(exports, {
          InstanceMethod("enumPrinters", &PrinterAddon::enumPrinters),
      });

      Printer::Init(env, exports);
    }

  private:
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

NODE_API_ADDON(PrinterAddon)
