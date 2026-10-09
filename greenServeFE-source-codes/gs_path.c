#include "module.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
static const GSNativeAPI*api;
static GSValue join(GSValue*a,size_t n){if(n<1)return api->null();size_t z=1;for(size_t i=0;i<n;i++){const char*s=api->cstring(a[i]);if(!s)return api->null();z+=strlen(s)+1;}char*b=malloc(z);if(!b)return api->null();b[0]=0;for(size_t i=0;i<n;i++){const char*s=api->cstring(a[i]);if(i&&b[0]&&b[strlen(b)-1]!='/')strcat(b,"/");while(*s=='/'&&i&&b[0])s++;strcat(b,s);}GSValue v=api->string(b);free(b);return v;}
static GSValue basename_fn(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();const char*p=strrchr(s,'/');return api->string(p?p+1:s);}
static GSValue dirname_fn(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();const char*p=strrchr(s,'/');if(!p)return api->string(".");size_t z=(size_t)(p-s);if(z==0)z=1;char*b=malloc(z+1);if(!b)return api->null();memcpy(b,s,z);b[z]=0;GSValue v=api->string(b);free(b);return v;}
static GSValue ext(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();const char*p=strrchr(s,'/');const char*d=strrchr(p?p:s,'.');if(!d||d==(p?p:s))return api->string("");return api->string(d+1);}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"join",api->native(join));api->object_set(&m,"basename",api->native(basename_fn));api->object_set(&m,"dirname",api->native(dirname_fn));api->object_set(&m,"extension",api->native(ext));return m;}
