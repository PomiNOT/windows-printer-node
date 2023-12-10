#pragma once

#include "utils.h"
#include "printer.h"
#include "napi.h"

class PrintJob : public Napi::ObjectWrap<PrintJob>
{
public:
    static Napi::Object PrintJob::Init(Napi::Env env, Napi::Object exports);

    PrintJob(const Napi::CallbackInfo &info);

    ~PrintJob();

    Napi::Value printPage(const Napi::CallbackInfo& info);

    Napi::Value end(const Napi::CallbackInfo& info);

private:
    Printer* m_printer = nullptr;
    HDC m_hdc = nullptr;
};