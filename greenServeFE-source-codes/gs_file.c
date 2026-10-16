#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define GS_FILE_LIMIT (8u * 1024u * 1024u)
static const GSNativeAPI *api;
static GSValue exists_fn(GSValue*a,size_t n){
    if(n<1)return api->boolean(0);const char*p=api->cstring(a[0]);if(!p)return api->boolean(0);
    struct stat st;return api->boolean(stat(p,&st)==0&&S_ISREG(st.st_mode));
}
static GSValue read_text(GSValue*a,size_t n){
    if(n<1)return api->null();const char*p=api->cstring(a[0]);if(!p)return api->null();
    FILE*f=fopen(p,"rb");if(!f)return api->null();
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return api->null();}
    long size=ftell(f);if(size<0||(unsigned long)size>GS_FILE_LIMIT){fclose(f);return api->null();}
    if(fseek(f,0,SEEK_SET)!=0){fclose(f);return api->null();}
    char*b=malloc((size_t)size+1);if(!b){fclose(f);return api->null();}
    size_t got=fread(b,1,(size_t)size,f);int ok=!ferror(f)&&got==(size_t)size;fclose(f);
    if(!ok){free(b);return api->null();}b[got]=0;GSValue v=api->string(b);free(b);return v;
}
static GSValue write_text(GSValue*a,size_t n,int append){
    if(n<2)return api->boolean(0);const char*p=api->cstring(a[0]),*s=api->cstring(a[1]);
    if(!p||!s||strlen(s)>GS_FILE_LIMIT)return api->boolean(0);
    FILE*f=fopen(p,append?"ab":"wb");if(!f)return api->boolean(0);
    size_t z=strlen(s),w=fwrite(s,1,z,f);int close_ok=fclose(f)==0;
    return api->boolean(w==z&&close_ok);
}
static GSValue write_fn(GSValue*a,size_t n){return write_text(a,n,0);}
static GSValue append_fn(GSValue*a,size_t n){return write_text(a,n,1);}
static GSValue size_fn(GSValue*a,size_t n){
    if(n<1)return api->null();const char*p=api->cstring(a[0]);if(!p)return api->null();
    struct stat st;if(stat(p,&st)!=0||!S_ISREG(st.st_mode))return api->null();
    return api->number((double)st.st_size);
}
GSValue greenServeFE_module_init(const GSNativeAPI *native_api){
    api=native_api;GSValue m=api->object();
    api->object_set(&m,"exists",api->native(exists_fn));
    api->object_set(&m,"read_text",api->native(read_text));
    api->object_set(&m,"write_text",api->native(write_fn));
    api->object_set(&m,"append_text",api->native(append_fn));
    api->object_set(&m,"size",api->native(size_fn));
    return m;
}
