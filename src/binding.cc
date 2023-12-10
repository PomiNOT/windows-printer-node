#include "napi.h"
#include "printer.h"
#include "job.h"

class PrinterAddon : public Napi::Addon<PrinterAddon>
{
public:
  PrinterAddon(Napi::Env env, Napi::Object exports)
  {
    DefineAddon(exports, {
      InstanceMethod("enumPrinters", &PrinterAddon::enumPrinters),
    });

    Printer::Init(env, exports);
    PrintJob::Init(env, exports);
  }

private:
  Napi::Value enumPrinters(const Napi::CallbackInfo &info)
  {
    DWORD length;
    DWORD required;
    DWORD bufSize;

    BOOL success = EnumPrintersW(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, // local printers only
        nullptr,                                       // get all printers
        1,
        nullptr,
        0,
        &required,
        &length);

    bufSize = required;
    std::unique_ptr<unsigned char[]> printerBuf(new unsigned char[required]);

    success = EnumPrintersW(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, // local printers only
        nullptr,                                       // get all printers
        1,
        (LPBYTE)printerBuf.get(),
        bufSize,
        &required,
        &length);

    if (!success)
    {
      throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    auto ret = Napi::Array::New(info.Env(), length);

    for (DWORD i = 0; i < length; i++)
    {
      LPPRINTER_INFO_1W printer = (LPPRINTER_INFO_1W)(printerBuf.get() + i * sizeof(PRINTER_INFO_1W));
      ret.Set(i, Napi::String::New(info.Env(), std::u16string((char16_t *)printer->pName)));
    }

    return ret;
  }
};

NODE_API_ADDON(PrinterAddon)
