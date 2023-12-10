#include "job.h"

Napi::Object PrintJob::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(env, "PrintJob", {
        InstanceMethod<&PrintJob::printPage>("printPage", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    });

    auto *constructor = new Napi::FunctionReference();

    *constructor = Persistent(func);
    exports.Set("PrintJob", func);

    env.SetInstanceData<Napi::FunctionReference>(constructor);
    return exports;
}

PrintJob::PrintJob(const Napi::CallbackInfo& info) : Napi::ObjectWrap<PrintJob>(info) {
    auto arg = info[0];
    if (!arg.IsObject()) {
        typeError(info.Env(), "printer must be a valid object");
        return;
    }

    m_printer = Printer::Unwrap(arg.ToObject());
    m_hdc = CreateDCW(
        nullptr,
        L"Hell no",
        nullptr,
        m_printer->getPropertiesInternal().getDevMode()
    );
};

Napi::Value PrintJob::printPage(const Napi::CallbackInfo& info)
{
    auto arg = info[0];
    if (!arg.IsTypedArray()) {
        return typeError(info.Env(), "imageData must be a TypedArray");
    }

    auto imageData = arg.As<Napi::Uint8Array>();
    uint8_t* buffer = imageData.Data();

    return info.Env().Undefined();
}