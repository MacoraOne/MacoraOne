#include "module.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
static const GSNativeAPI*api;
static GSValue email(GSValue*a,size_t n){if(n<1)return api->boolean(0);const char*s=api->cstring(a[0]);if(!s)return api->boolean(0);const char*@=strchr(s,'@');return api->boolean(@&&@>s&&strchr(@+1,'.')!=NULL&&strchr(@+1,'@')==NULL);}
static GSValue integer(GSValue*a,size_t n){if(n<1)return api->boolean(0);const char*s=api->cstring(a[0]);if(!s||!*s)return api->boolean(0);char*e=NULL;strtol(s,&e,10);return api->boolean(e&&*e==0);}
static GSValue nonempty(GSValue*a,size_t n){if(n<1)return api->boolean(0);const char*s=api->cstring(a[0]);return api->boolean(s&&*s);}
static GSValue numeric(GSValue*a,size_t n){if(n<1)return api->boolean(0);const char*s=api->cstring(a[0]);if(!s||!*s)return api->boolean(0);char*e=NULL;strtod(s,&e);return api->boolean(e&&*e==0);}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"email",api->native(email));api->object_set(&m,"integer",api->native(integer));api->object_set(&m,"numeric",api->native(numeric));api->object_set(&m,"nonempty",api->native(nonempty));return m;}
