#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const GSNativeAPI*api;typedef struct{char*p;size_t n,c;}Buf;
static int put(Buf*b,const char*s){size_t z=strlen(s);if(b->n+z+1>b->c){size_t c=b->c?b->c*2:128;while(c<b->n+z+1)c*=2;char*p=realloc(b->p,c);if(!p)return 0;b->p=p;b->c=c;}memcpy(b->p+b->n,s,z);b->n+=z;b->p[b->n]=0;return 1;}
static int esc(Buf*b,const char*s){if(!put(b,"\""))return 0;for(;*s;s++){char x[3]={0};if(*s=='"'||*s=='\\'){x[0]='\\';x[1]=*s;}else if(*s=='\n'){x[0]='\\';x[1]='n';}else if(*s=='\r'){x[0]='\\';x[1]='r';}else if(*s=='\t'){x[0]='\\';x[1]='t';}else{x[0]=*s;x[1]=0;}if(!put(b,x))return 0;}return put(b,"\"");}
static int emit(Buf*b,GSValue v){char t[64];switch(v.type){case V_NULL:return put(b,"null");case V_BOOL:return put(b,v.as.boolean?"true":"false");case V_NUMBER:snprintf(t,sizeof t,"%.17g",v.as.number);return put(b,t);case V_STRING:return esc(b,v.as.string.data?v.as.string.data:"");case V_ARRAY:if(!put(b,"["))return 0;for(size_t i=0;i<api->array_count(v);i++){if(i&&!put(b,","))return 0;if(!emit(b,api->array_get(v,i)))return 0;}return put(b,"]");case V_OBJECT:if(!put(b,"{"))return 0;for(size_t i=0;i<api->object_count(v);i++){if(i&&!put(b,","))return 0;if(!esc(b,api->object_key(v,i)))return 0;if(!put(b,":"))return 0;if(!emit(b,api->object_get(v,api->object_key(v,i))))return 0;}return put(b,"}");default:return 0;}}
static GSValue stringify(GSValue*a,size_t n){if(n<1)return api->null();Buf b={0};if(!emit(&b,a[0])){free(b.p);return api->null();}GSValue v=api->string(b.p);free(b.p);return v;}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"stringify",api->native(stringify));return m;}
