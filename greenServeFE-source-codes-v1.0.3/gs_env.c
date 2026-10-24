#include "module.h"
#include <stdlib.h>
static const GSNativeAPI*api;
static GSValue get(GSValue*a,size_t n){if(n<1)return api->null();const char*k=api->cstring(a[0]);if(!k)return api->null();const char*v=getenv(k);return v?api->string(v):api->null();}
static GSValue has(GSValue*a,size_t n){if(n<1)return api->null();const char*k=api->cstring(a[0]);return api->boolean(k&&getenv(k)!=NULL);}
static GSValue setv(GSValue*a,size_t n){if(n<2)return api->boolean(0);const char*k=api->cstring(a[0]),*v=api->cstring(a[1]);if(!k||!v)return api->boolean(0);return api->boolean(setenv(k,v,1)==0);}
static GSValue unsetv(GSValue*a,size_t n){if(n<1)return api->boolean(0);const char*k=api->cstring(a[0]);return api->boolean(k&&unsetenv(k)==0);}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"get",api->native(get));api->object_set(&m,"has",api->native(has));api->object_set(&m,"set",api->native(setv));api->object_set(&m,"unset",api->native(unsetv));return m;}
