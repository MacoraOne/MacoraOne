#include "module.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const GSNativeAPI *api;
static int safe(unsigned char c){return isalnum(c)||c=='-'||c=='_'||c=='.'||c=='~';}
static GSValue encode(GSValue*a,size_t n){
    if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();
    size_t z=strlen(s);if(z>(SIZE_MAX-1)/3)return api->null();
    char*b=malloc(z*3+1);if(!b)return api->null();static const char h[]="0123456789ABCDEF";size_t j=0;
    for(size_t i=0;i<z;i++){unsigned char c=(unsigned char)s[i];if(safe(c))b[j++]=(char)c;else{b[j++]='%';b[j++]=h[c>>4];b[j++]=h[c&15];}}
    b[j]=0;GSValue v=api->string(b);free(b);return v;
}
static int hexval(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
static GSValue decode(GSValue*a,size_t n){
    if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();
    size_t z=strlen(s);char*b=malloc(z+1);if(!b)return api->null();size_t j=0;
    for(size_t i=0;i<z;i++){if(s[i]=='%'){if(i+2>=z){free(b);return api->null();}int x=hexval(s[i+1]),y=hexval(s[i+2]);if(x<0||y<0){free(b);return api->null();}b[j++]=(char)((x<<4)|y);i+=2;}else b[j++]=s[i];}
    b[j]=0;GSValue v=api->string(b);free(b);return v;
}
static GSValue query_encode(GSValue*a,size_t n){
    if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();
    size_t z=strlen(s);if(z>(SIZE_MAX-1)/3)return api->null();char*b=malloc(z*3+1);if(!b)return api->null();
    static const char h[]="0123456789ABCDEF";size_t j=0;
    for(size_t i=0;i<z;i++){unsigned char c=(unsigned char)s[i];if(c==' ')b[j++]='+';else if(safe(c))b[j++]=(char)c;else{b[j++]='%';b[j++]=h[c>>4];b[j++]=h[c&15];}}
    b[j]=0;GSValue v=api->string(b);free(b);return v;
}
static GSValue query_decode(GSValue*a,size_t n){
    if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();
    size_t z=strlen(s);char*b=malloc(z+1);if(!b)return api->null();size_t j=0;
    for(size_t i=0;i<z;i++){if(s[i]=='+')b[j++]=' ';else if(s[i]=='%'){if(i+2>=z){free(b);return api->null();}int x=hexval(s[i+1]),y=hexval(s[i+2]);if(x<0||y<0){free(b);return api->null();}b[j++]=(char)((x<<4)|y);i+=2;}else b[j++]=s[i];}
    b[j]=0;GSValue v=api->string(b);free(b);return v;
}
GSValue greenServeFE_module_init(const GSNativeAPI *native_api){
    api=native_api;GSValue m=api->object();
    api->object_set(&m,"encode",api->native(encode));
    api->object_set(&m,"decode",api->native(decode));
    api->object_set(&m,"query_encode",api->native(query_encode));
    api->object_set(&m,"query_decode",api->native(query_decode));
    return m;
}
