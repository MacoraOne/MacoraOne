#include "module.h"
#include <stdlib.h>
#include <string.h>
static const GSNativeAPI*api;static const char B[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static GSValue enc(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();size_t z=strlen(s),o=4*((z+2)/3);char*b=malloc(o+1);if(!b)return api->null();size_t j=0;for(size_t i=0;i<z;i+=3){unsigned x=(unsigned char)s[i]<<16; if(i+1<z)x|=(unsigned char)s[i+1]<<8;if(i+2<z)x|=(unsigned char)s[i+2];b[j++]=B[(x>>18)&63];b[j++]=B[(x>>12)&63];b[j++]=i+1<z?B[(x>>6)&63]:'=';b[j++]=i+2<z?B[x&63]:'=';}b[j]=0;GSValue v=api->string(b);free(b);return v;}
static int val(char c){const char*p=strchr(B,c);return p?(int)(p-B):-1;}
static GSValue dec(GSValue*a,size_t n){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();size_t z=strlen(s);char*b=malloc(z+1);if(!b)return api->null();size_t o=0;for(size_t i=0;i+3<z;i+=4){int a1=val(s[i]),a2=val(s[i+1]);if(a1<0||a2<0){free(b);return api->null();}int a3=s[i+2]=='='?-1:val(s[i+2]),a4=s[i+3]=='='?-1:val(s[i+3]);if(a3<0&&s[i+2]!='='||a4<0&&s[i+3]!='='){free(b);return api->null();}unsigned x=((unsigned)a1<<18)|((unsigned)a2<<12);b[o++]=(char)(x>>16);if(a3>=0){x|=(unsigned)a3<<6;b[o++]=(char)(x>>8);if(a4>=0){x|=(unsigned)a4;b[o++]=(char)x;}}}b[o]=0;GSValue v=api->string(b);free(b);return v;}
GSValue greenServeFE_module_init(const GSNativeAPI*n){api=n;GSValue m=api->object();api->object_set(&m,"encode",api->native(enc));api->object_set(&m,"decode",api->native(dec));return m;}
