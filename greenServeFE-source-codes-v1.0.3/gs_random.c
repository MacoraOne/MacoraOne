#include "module.h"
#include <stdlib.h>
#include <time.h>
static const GSNativeAPI*api;
static GSValue seed_fn(GSValue*a,size_t n){if(n<1)return api->null();srand((unsigned)api->number_value(a[0]));return api->null();}
static GSValue integer(GSValue*a,size_t n){if(n<2)return api->null();long lo=(long)api->number_value(a[0]),hi=(long)api->number_value(a[1]);if(hi<lo){long t=lo;lo=hi;hi=t;}unsigned long span=(unsigned long)(hi-lo+1);return api->number((double)(lo+(long)(rand()%span)));}
static GSValue fraction(GSValue*a,size_t n){(void)a;(void)n;return api->number((double)rand()/(double)RAND_MAX);}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;srand((unsigned)time(NULL));GSValue m=api->object();api->object_set(&m,"seed",api->native(seed_fn));api->object_set(&m,"integer",api->native(integer));api->object_set(&m,"fraction",api->native(fraction));return m;}
