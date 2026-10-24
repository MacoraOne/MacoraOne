#include "module.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
static const GSNativeAPI *api;
static const char*strarg(GSValue*a,size_t n,size_t i){return i<n?api->cstring(a[i]):NULL;}
static GSValue trim_fn(GSValue*a,size_t n){const char*s=strarg(a,n,0);if(!s)return api->null();while(isspace((unsigned char)*s))s++;size_t z=strlen(s);while(z&&isspace((unsigned char)s[z-1]))z--;char*b=malloc(z+1);if(!b)return api->null();memcpy(b,s,z);b[z]=0;GSValue v=api->string(b);free(b);return v;}
static GSValue required_fn(GSValue*a,size_t n){if(!n||a[0].type==V_NULL)return api->boolean(0);const char*s=api->cstring(a[0]);if(!s)return api->boolean(1);while(isspace((unsigned char)*s))s++;return api->boolean(*s!=0);}
static GSValue email_fn(GSValue*a,size_t n){const char*s=strarg(a,n,0);if(!s)return api->boolean(0);size_t z=strlen(s);if(z<3||z>254)return api->boolean(0);const char*at=strchr(s,'@');if(!at||at==s||strchr(at+1,'@')||at[1]==0)return api->boolean(0);const char*dot=strrchr(at+1,'.');if(!dot||dot==at+1||dot[1]==0||strlen(dot+1)<2)return api->boolean(0);for(const unsigned char*p=(const unsigned char*)s;*p;p++)if(isspace(*p)||*p<33)return api->boolean(0);return api->boolean(1);}
static GSValue number_fn(GSValue*a,size_t n){const char*s=strarg(a,n,0);if(!s)return api->null();while(isspace((unsigned char)*s))s++;if(!*s)return api->null();errno=0;char*end=NULL;double d=strtod(s,&end);while(end&&isspace((unsigned char)*end))end++;if(errno||!end||end==s||*end||!isfinite(d))return api->null();return api->number(d);}
static GSValue bool_fn(GSValue*a,size_t n){const char*s=strarg(a,n,0);if(!s)return api->null();if(!strcasecmp(s,"true")||!strcasecmp(s,"yes")||!strcmp(s,"1")||!strcasecmp(s,"on"))return api->boolean(1);if(!strcasecmp(s,"false")||!strcasecmp(s,"no")||!strcmp(s,"0")||!strcasecmp(s,"off"))return api->boolean(0);return api->null();}
static GSValue min_length(GSValue*a,size_t n){const char*s=strarg(a,n,0);if(!s||n<2)return api->boolean(0);double d=api->number_value(a[1]);return api->boolean(d>=0&&d==(double)(size_t)d&&strlen(s)>=(size_t)d);}
static GSValue max_length(GSValue*a,size_t n){const char*s=strarg(a,n,0);if(!s||n<2)return api->boolean(0);double d=api->number_value(a[1]);return api->boolean(d>=0&&d==(double)(size_t)d&&strlen(s)<=(size_t)d);}
static GSValue range_fn(GSValue*a,size_t n){if(n<3)return api->boolean(0);double v=api->number_value(a[0]),lo=api->number_value(a[1]),hi=api->number_value(a[2]);return api->boolean(isfinite(v)&&isfinite(lo)&&isfinite(hi)&&lo<=hi&&v>=lo&&v<=hi);}
GSValue greenServeFE_module_init(const GSNativeAPI*napi){api=napi;GSValue m=api->object();api->object_set(&m,"trim",api->native(trim_fn));api->object_set(&m,"required",api->native(required_fn));api->object_set(&m,"is_email",api->native(email_fn));api->object_set(&m,"to_number",api->native(number_fn));api->object_set(&m,"to_bool",api->native(bool_fn));api->object_set(&m,"min_length",api->native(min_length));api->object_set(&m,"max_length",api->native(max_length));api->object_set(&m,"in_range",api->native(range_fn));return m;}
