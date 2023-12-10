#pragma once

#include <Windows.h>
#include "napi.h"
#include "utils.h"
#include "props.h"

class Printer : public Napi::ObjectWrap<Printer>
{
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  Printer(const Napi::CallbackInfo &info);

  Printer(const Printer &other) = delete;

  Printer &operator=(Printer &other);

  Napi::Value getProperties(const Napi::CallbackInfo &info);

  Napi::Value setProperties(const Napi::CallbackInfo &info);

  Napi::Value chooseProperties(const Napi::CallbackInfo &info);

  HANDLE getHandle();

  const DocProps& getPropertiesInternal() const;

  ~Printer();

private:
  HANDLE m_handle;
  DocProps m_props;
  std::wstring m_printerName;
  unsigned char *m_pInfoBuf = nullptr;

  void open();
  void readPrinterInfo();
};