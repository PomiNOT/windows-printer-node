#define NAPI_CPP_EXCEPTIONS

#include "napi.h"
#include <Windows.h>
#include <iostream>

using namespace Napi;

static std::string getLastErrorString();

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
      auto printerName = info[0];
      if (!printerName.IsString()) {
        napi_throw_type_error(info.Env(), nullptr, "Printer name must be a string");
        return info.Env().Undefined();
      }

      HANDLE printer;
      std::u16string s = printerName.ToString().Utf16Value();
      std::wstring ws(s.begin(), s.end());

      BOOL success = OpenPrinter2W(
        ws.c_str(),
        &printer,
        nullptr,
        nullptr
      );

      if (!success) {
        napi_throw_error(info.Env(), nullptr, getLastErrorString().c_str());
        return info.Env().Undefined();
      }

      DWORD dwPSize;

      success = GetPrinterW(
        printer,
        8,
        nullptr,
        0,
        &dwPSize
      );

      std::unique_ptr<unsigned char[]> pInfoBuf(new unsigned char[dwPSize]);

      success = GetPrinterW(
        printer,
        8,
        pInfoBuf.get(),
        dwPSize,
        &dwPSize
      );

      if (!success) {
        napi_throw_error(info.Env(), nullptr, getLastErrorString().c_str());
        return info.Env().Undefined();
      }

      PRINTER_INFO_8W* printerInfo = (PRINTER_INFO_8W*)pInfoBuf.get();

      LONG result = DocumentPropertiesW(
        nullptr,
        printer,
        (LPWSTR)ws.c_str(),
        printerInfo->pDevMode,
        printerInfo->pDevMode,
        DM_IN_PROMPT | DM_OUT_BUFFER
      );

      if (result < 0) {
        napi_throw_error(info.Env(), nullptr, getLastErrorString().c_str());
        return info.Env().Undefined();
      }

      Object ret = Object::New(info.Env());

      ret.Set("copies", Number::From(info.Env(), printerInfo->pDevMode->dmCopies));
      ret.Set("orientation", 
        printerInfo->pDevMode->dmOrientation == DMORIENT_PORTRAIT ?
          String::From(info.Env(), "portrait") : String::From(info.Env(), "landscape")
      );
      ret.Set("paperWidth", Number::From(info.Env(), (double)printerInfo->pDevMode->dmPaperWidth / 10));
      ret.Set("paperHeight", Number::From(info.Env(), (double)printerInfo->pDevMode->dmPaperLength / 10));
      ret.Set("scale", Number::From(info.Env(), (double)printerInfo->pDevMode->dmScale / 100));
      
      Object dpi = Object::New(info.Env());
      dpi.Set("x", printerInfo->pDevMode->dmPrintQuality);
      dpi.Set("y", printerInfo->pDevMode->dmYResolution);

      ret.Set("dpi", dpi);

      return ret;
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
        napi_throw_error(info.Env(), nullptr, getLastErrorString().c_str());
        return info.Env().Undefined();
      }

      auto ret = Array::New(info.Env(), length);

      for (int i = 0; i < length; i++) {
        LPPRINTER_INFO_1W printer = (LPPRINTER_INFO_1W)(printerBuf.get() + i * sizeof(PRINTER_INFO_1W));
        ret.Set(i, String::New(info.Env(), std::u16string((char16_t*) printer->pName)));
      }

      return ret;
    }
};

static std::string getLastErrorString() {
  DWORD lastError = GetLastError();
  if (lastError == 0) return "";

  char buf[256];
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,  lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                buf, 256, NULL);

  return std::string(buf);
}

NODE_API_ADDON(MyAddon)
