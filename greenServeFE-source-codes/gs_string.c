#include "module.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static const GSNativeAPI *api;
static GSValue ret_string(char *s){ GSValue v=api->string(s?s:""); free(s); return v; }
static GSValue lower(GSValue *a,size_t n){ if(n<1)return api->null(); const char*s=api->cstring(a[0]); if(!s)return api->null(); char*b=malloc(strlen(s)+1); if(!b)return api->null(); for(size_t i=0;s[i];i++)b[i]=(char)tolower((unsigned char)s[i]); b[strlen(s)]=0; return ret_string(b); }
static GSValue upper(GSValue *a,size_t n){ if(n<1)return api->null(); const char*s=api->cstring(a[0]); if(!s)return api->null(); char*b=malloc(strlen(s)+1); if(!b)return api->null(); for(size_t i=0;s[i];i++)b[i]=(char)toupper((unsigned char)s[i]); b[strlen(s)]=0; return ret_string(b); }
static GSValue trim(GSValue *a,size_t n){ if(n<1)return api->null(); const char*s=api->cstring(a[0]); if(!s)return api->null(); while(isspace((unsigned char)*s))s++; const char*e=s+strlen(s); while(e>s&&isspace((unsigned char)e[-1]))e--; size_t z=(size_t)(e-s); char*b=malloc(z+1); if(!b)return api->null(); memcpy(b,s,z); b[z]=0; return ret_string(b); }
static GSValue contains(GSValue*a,size_t n){if(n<2)return api->null();const char*x=api->cstring(a[0]),*y=api->cstring(a[1]);return api->boolean(x&&y&&strstr(x,y)!=NULL);}
static GSValue starts(GSValue*a,size_t n){if(n<2)return api->null();const char*x=api->cstring(a[0]),*y=api->cstring(a[1]);return api->boolean(x&&y&&strncmp(x,y,strlen(y))==0);}
static GSValue ends(GSValue*a,size_t n){if(n<2)return api->null();const char*x=api->cstring(a[0]),*y=api->cstring(a[1]);if(!x||!y)return api->boolean(0);size_t xz=strlen(x),yz=strlen(y);return api->boolean(xz>=yz&&strcmp(x+xz-yz,y)==0);}
GSValue greenServeFE_module_init(const GSNativeAPI*native_api){api=native_api;GSValue m=api->object();api->object_set(&m,"lower",api->native(lower));api->object_set(&m,"upper",api->native(upper));api->object_set(&m,"trim",api->native(trim));api->object_set(&m,"contains",api->native(contains));api->object_set(&m,"starts_with",api->native(starts));api->object_set(&m,"ends_with",api->native(ends));return m;}
