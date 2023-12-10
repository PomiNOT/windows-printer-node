#include "utils.h"

std::string getLastErrorString()
{
  DWORD lastError = GetLastError();
  if (lastError == 0)
    return "";

  char buf[256];
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 buf, 256, NULL);

  return std::string(buf);
}