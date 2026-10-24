#include "module.h"
#include <stdint.h>
#include <string.h>
static const GSNativeAPI*api;
static GSValue fnv1a(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();uint64_t h=1469598103934665603ULL;for(const unsigned char*p=(const unsigned char*)s;*p;p++){h^=*p;h*=1099511628211ULL;}return api->number((double)h);}
static GSValue djb2(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();uint64_t h=5381;for(const unsigned char*p=(const unsigned char*)s;*p;p++)h=((h<<5)+h)^*p;return api->number((double)h);}
static GSValue equal(GSValue*a,size_t n){if(n<2)return api->null();return api->boolean(fnv1a(a,1).as.number==fnv1a(a+1,1).as.number);}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"fnv1a",api->native(fnv1a));api->object_set(&m,"djb2",api->native(djb2));api->object_set(&m,"same_hash",api->native(equal));return m;}
