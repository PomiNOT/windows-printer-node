#define NAPI_CPP_EXCEPTIONS

#include "napi.h"

using namespace Napi;

class MyAddon : public Addon<MyAddon> {
  public:
    MyAddon(Env env, Object exports) {
      DefineAddon(exports, {
          InstanceValue("one", Number::New(env, 10.0))
      });
    }
};

NODE_API_ADDON(MyAddon)
