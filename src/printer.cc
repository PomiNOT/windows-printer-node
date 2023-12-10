#include "printer.h"

Napi::Object Printer::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(env, "Printer", {
        InstanceMethod<&Printer::getProperties>("getProperties", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&Printer::setProperties>("setProperties", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&Printer::chooseProperties>("chooseProperties", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    });

    auto *constructor = new Napi::FunctionReference();

    *constructor = Persistent(func);
    exports.Set("Printer", func);

    env.SetInstanceData<Napi::FunctionReference>(constructor);
    return exports;
}

Printer::Printer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Printer>(info)
{
    auto printerName = info[0];
    if (!printerName.IsString())
    {
        typeError(info.Env(), "printerName must be a string");
        return;
    }

    m_printerName = toWString(printerName.ToString().Utf16Value());

    try
    {
        open();
    }
    catch (const std::runtime_error &e)
    {
        throw Napi::Error::New(info.Env(), e.what());
    }
}

Printer& Printer::operator= (Printer & other)
{
    if (this == &other)
    {
        return *this;
    }

    m_handle = other.m_handle;
    other.m_handle = nullptr;

    m_pInfoBuf = other.m_pInfoBuf;
    other.m_pInfoBuf = nullptr;

    return *this;
}

Printer::~Printer()
{
    if (m_handle != nullptr)
    {
        ClosePrinter(m_handle);
    }

    if (m_pInfoBuf != nullptr)
    {
        delete[] m_pInfoBuf;
    }
}

HANDLE Printer::getHandle() {
    return m_handle;
}

const DocProps& Printer::getPropertiesInternal() const {
    return m_props;
}

Napi::Value Printer::getProperties(const Napi::CallbackInfo &info)
{
    return m_props.toObject(info.Env());
}

Napi::Value Printer::chooseProperties(const Napi::CallbackInfo &info)
{
    LONG result = DocumentPropertiesW(
        nullptr,
        m_handle,
        (LPWSTR)m_printerName.c_str(),
        m_props.getDevMode(),
        m_props.getDevMode(),
        DM_IN_BUFFER | DM_IN_PROMPT | DM_OUT_BUFFER);

    if (result < 0)
    {
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    return info.Env().Undefined();
}

Napi::Value Printer::setProperties(const Napi::CallbackInfo &info)
{
    if (!info[0].IsObject())
    {
        return typeError(info.Env(), "properties isn't an object");
    }

    m_props.fromObject(info[0].ToObject());

    LONG result = DocumentPropertiesW(
        nullptr,
        m_handle,
        (LPWSTR)m_printerName.c_str(),
        nullptr,
        m_props.getDevMode(),
        DM_IN_BUFFER);

    if (result < 0)
    {
        throw Napi::Error::New(info.Env(), getLastErrorString());
    }

    return info.Env().Undefined();
}

void Printer::open()
{
    BOOL success = OpenPrinter2W(m_printerName.c_str(), &m_handle, nullptr, nullptr);
    if (!success)
    {
        throw std::runtime_error(getLastErrorString());
    }

    readPrinterInfo();
}

void Printer::readPrinterInfo()
{
    DWORD dwPSize;

    BOOL success = GetPrinterW(
        m_handle,
        2,
        nullptr,
        0,
        &dwPSize);

    if (m_pInfoBuf != nullptr)
    {
        delete[] m_pInfoBuf;
    }

    m_pInfoBuf = new unsigned char[dwPSize];

    success = GetPrinterW(
        m_handle,
        2,
        m_pInfoBuf,
        dwPSize,
        &dwPSize);

    if (!success)
    {
        throw std::runtime_error(getLastErrorString());
    }

    PRINTER_INFO_2W *infoPtr = reinterpret_cast<PRINTER_INFO_2W *>(m_pInfoBuf);
    m_props = DocProps(infoPtr->pDevMode);
}