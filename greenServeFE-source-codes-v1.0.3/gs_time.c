#include "module.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const GSNativeAPI*api;
static GSValue now(GSValue*a,size_t n){(void)a;(void)n;return api->number((double)time(NULL));}
static GSValue format(GSValue*a,size_t n){if(n<1)return api->null();time_t t=(time_t)api->number_value(a[0]);const char*f=n>1?api->cstring(a[1]):"%Y-%m-%d %H:%M:%S";if(!f)return api->null();struct tm tmv;if(!localtime_r(&t,&tmv))return api->null();size_t z=128;char*b=malloc(z);if(!b)return api->null();size_t r=strftime(b,z,f,&tmv);if(!r){free(b);return api->null();}GSValue v=api->string(b);free(b);return v;}
static GSValue utc(GSValue*a,size_t n){if(n<1)return api->null();time_t t=(time_t)api->number_value(a[0]);struct tm tmv;if(!gmtime_r(&t,&tmv))return api->null();char b[64];if(!strftime(b,sizeof b,"%Y-%m-%dT%H:%M:%SZ",&tmv))return api->null();return api->string(b);}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"now",api->native(now));api->object_set(&m,"format",api->native(format));api->object_set(&m,"utc",api->native(utc));return m;}
