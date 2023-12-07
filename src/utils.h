#pragma once

#include "Windows.h"
#include "napi.h"
#include <string>

std::string getLastErrorString();

inline std::wstring toWString(const std::u16string& s) {
    return std::wstring(s.begin(), s.end());
}

inline Napi::Value typeError(Napi::Env env, const char* message) {
    napi_throw_type_error(env, nullptr, message);
    return env.Undefined();
}