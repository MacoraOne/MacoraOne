#include "module.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const GSNativeAPI *api;
static GSValue digest_fn(GSValue*a,size_t n,const EVP_MD*md){if(n<1)return api->null();const char*s=api->cstring(a[0]);if(!s)return api->null();unsigned char out[EVP_MAX_MD_SIZE];unsigned int len=0;if(EVP_Digest(s,strlen(s),out,&len,md,NULL)!=1)return api->null();char*hex=malloc((size_t)len*2+1);if(!hex)return api->null();static const char h[]="0123456789abcdef";for(unsigned int i=0;i<len;i++){hex[i*2]=h[out[i]>>4];hex[i*2+1]=h[out[i]&15];}hex[len*2]=0;GSValue v=api->string(hex);free(hex);return v;}
static GSValue sha256(GSValue*a,size_t n){return digest_fn(a,n,EVP_sha256());}
static GSValue sha512(GSValue*a,size_t n){return digest_fn(a,n,EVP_sha512());}
static GSValue random_bytes(GSValue*a,size_t n){if(n<1)return api->null();double d=api->number_value(a[0]);if(d<1||d>4096||d!=(double)(size_t)d)return api->null();size_t z=(size_t)d;unsigned char*b=malloc(z);char*hex=malloc(z*2+1);if(!b||!hex){free(b);free(hex);return api->null();}if(RAND_bytes(b,(int)z)!=1){free(b);free(hex);return api->null();}static const char h[]="0123456789abcdef";for(size_t i=0;i<z;i++){hex[i*2]=h[b[i]>>4];hex[i*2+1]=h[b[i]&15];}hex[z*2]=0;GSValue v=api->string(hex);free(b);free(hex);return v;}
GSValue greenServeFE_module_init(const GSNativeAPI*napi){api=napi;GSValue m=api->object();api->object_set(&m,"sha256",api->native(sha256));api->object_set(&m,"sha512",api->native(sha512));api->object_set(&m,"random_bytes",api->native(random_bytes));return m;}
