#include "module.h"
#include <stdlib.h>
#include <string.h>

static const GSNativeAPI *api;

static int append_char(char **buf,size_t *len,size_t *cap,char c){
    if(*len+2>*cap){size_t nc=*cap?*cap*2:64;char*p=realloc(*buf,nc);if(!p)return 0;*buf=p;*cap=nc;}
    (*buf)[(*len)++]=c;(*buf)[*len]=0;return 1;
}
static GSValue parse_csv(GSValue *a,size_t n){
    if(n<1)return api->null();
    const char*s=api->cstring(a[0]);if(!s)return api->null();
    GSValue rows=api->array(),row=api->array();
    char *field=NULL;size_t len=0,cap=0;int quoted=0;
    for(size_t i=0;;i++){
        char c=s[i];
        if(quoted){
            if(c=='"'&&s[i+1]=='"'){if(!append_char(&field,&len,&cap,'"'))goto fail;i++;}
            else if(c=='"')quoted=0;
            else if(!c)goto fail;
            else if(!append_char(&field,&len,&cap,c))goto fail;
        }else if(c=='"'&&len==0){quoted=1;}
        else if(c==','||c=='\n'||c=='\r'||c==0){
            api->array_push(&row,api->string(field?field:""));len=0;if(field)field[0]=0;
            if(c==',')continue;
            if(c=='\r'&&s[i+1]=='\n')i++;
            if(c==0 && len==0 && i>0 && (s[i-1]=='\\n'||s[i-1]=='\\r')) break;
            api->array_push(&rows,row);row=api->array();
            if(!c)break;
        }else if(!append_char(&field,&len,&cap,c))goto fail;
    }
    free(field);return rows;
fail:
    free(field);api->release(rows);api->release(row);return api->null();
}
static int csv_field(char **b,size_t *len,size_t *cap,const char*s){
    int quote=strpbrk(s,",\"\r\n")!=NULL;
    if(quote&&!append_char(b,len,cap,'"'))return 0;
    for(;*s;s++){if(*s=='"'&&!append_char(b,len,cap,'"'))return 0;if(!append_char(b,len,cap,*s))return 0;}
    return !quote||append_char(b,len,cap,'"');
}
static GSValue stringify_csv(GSValue *a,size_t n){
    if(n<1)return api->null();
    GSValue data=a[0];if(data.type!=V_ARRAY)return api->null();
    char*b=NULL;size_t len=0,cap=0;
    size_t nr=api->array_count(data);
    for(size_t r=0;r<nr;r++){
        GSValue row=api->array_get(data,r);
        if(row.type!=V_ARRAY)goto fail;
        size_t nc=api->array_count(row);
        for(size_t c=0;c<nc;c++){
            if(c&&!append_char(&b,&len,&cap,','))goto fail;
            GSValue v=api->array_get(row,c);
            const char*s=api->cstring(v);
            if(!csv_field(&b,&len,&cap,s?s:""))goto fail;
        }
        if(r+1<nr&&!append_char(&b,&len,&cap,'\n'))goto fail;
    }
    if(!b){b=malloc(1);if(!b)return api->null();b[0]=0;}
    GSValue out=api->string(b);free(b);return out;
fail:
    free(b);return api->null();
}
GSValue greenServeFE_module_init(const GSNativeAPI *native_api){
    api=native_api;GSValue m=api->object();
    api->object_set(&m,"parse",api->native(parse_csv));
    api->object_set(&m,"stringify",api->native(stringify_csv));
    return m;
}
