#pragma once

#include "Windows.h"
#include <string>

std::string getLastErrorString();

inline std::wstring toWString(const std::u16string& s) {
    return std::wstring(s.begin(), s.end());
}