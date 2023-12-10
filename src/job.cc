#include "job.h"
#include <iostream>

Napi::Object PrintJob::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(env, "PrintJob", {
        InstanceMethod<&PrintJob::printPage>("printPage", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&PrintJob::end>("end", static_cast<napi_property_attributes>(napi_writable | napi_configurable))
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

    auto docName = info[1];

    if (!docName.IsString()) {
        typeError(info.Env(), "documentName must be a valid string");
        return;
    }

    m_printer = Printer::Unwrap(arg.ToObject());
    m_hdc = CreateDCW(
        nullptr,
        m_printer->getName().c_str(),
        nullptr,
        m_printer->getPropertiesInternal().getDevMode()
    );

    if (m_hdc == nullptr) {
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    std::wstring docNameStr = toWString(docName.ToString().Utf16Value()).c_str();

    DOCINFOW docInfo = {0};
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = docNameStr.c_str();
    docInfo.lpszDatatype = nullptr;
    docInfo.lpszOutput = nullptr;
    docInfo.fwType = 0;

    int status = StartDocW(m_hdc, &docInfo);

    if (status <= 0) {
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }
};

PrintJob::~PrintJob() {
    if (m_hdc != nullptr) {
        EndDoc(m_hdc);
        DeleteDC(m_hdc);

        m_hdc = nullptr;
    }
}

Napi::Value PrintJob::printPage(const Napi::CallbackInfo& info)
{
    auto buffer = info[0];
    auto width = info[1];
    auto height = info[2];

    if (!buffer.IsTypedArray()) {
        return typeError(info.Env(), "imageData must be a TypedArray");
    }

    if (!width.IsNumber() || width.ToNumber().Int32Value() <= 0) {
        return typeError(info.Env(), "width must be a positive integer");
    }
    
    if (!height.IsNumber() || width.ToNumber().Int32Value() <= 0) {
        return typeError(info.Env(), "height must be a positive integer");
    }

    uint8_t* imageData = buffer.As<Napi::Uint8Array>().Data();
    int w = width.ToNumber().Int32Value();
    int h = height.ToNumber().Int32Value();

    int status = StartPage(m_hdc);
    if (status <= 0) {
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    BITMAPINFOHEADER header = {0};
    header.biSize = sizeof(header);
    header.biWidth = w;
    header.biHeight = -h; //top down image
    header.biPlanes = 1;
    header.biBitCount = 8;
    header.biCompression = BI_RGB;

    BITMAPINFO bitmapInfo = {0};
    bitmapInfo.bmiHeader = header;
    bitmapInfo.bmiColors->rgbBlue = 0;
    bitmapInfo.bmiColors->rgbGreen = 0;
    bitmapInfo.bmiColors->rgbRed = 0;
    bitmapInfo.bmiColors->rgbReserved = 0;

    HBITMAP bitmap = CreateDIBitmap(
        m_hdc,
        &header,
        CBM_INIT,
        imageData,
        &bitmapInfo,
        DIB_RGB_COLORS
    );

    if (bitmap == nullptr) {
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    HDC mem = CreateCompatibleDC(m_hdc);
    SelectObject(mem, bitmap);
    
    BitBlt(m_hdc, 0, 0, w, h, mem, 0, 0, SRCCOPY);

    status = EndPage(m_hdc);
    if (status <= 0) {
        DeleteObject(bitmap);
        DeleteDC(mem);
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    DeleteObject(bitmap);
    DeleteDC(mem);

    return info.Env().Undefined();
}

Napi::Value PrintJob::end(const Napi::CallbackInfo& info)
{
    if (m_hdc != nullptr) {
        EndDoc(m_hdc);
        DeleteDC(m_hdc);

        m_hdc = nullptr;
    }

    return info.Env().Undefined();
}