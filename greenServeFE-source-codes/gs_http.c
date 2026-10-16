#include "module.h"
#include <curl/curl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const GSNativeAPI *api;
typedef struct { char *data; size_t len, cap; } Buffer;
static int append(Buffer *b,const char *p,size_t n){if(n>SIZE_MAX-b->len-1)return 0;size_t need=b->len+n+1;if(need>b->cap){size_t c=b->cap?b->cap:256;while(c<need){if(c>SIZE_MAX/2){c=need;break;}c*=2;}char*q=realloc(b->data,c);if(!q)return 0;b->data=q;b->cap=c;}memcpy(b->data+b->len,p,n);b->len+=n;b->data[b->len]=0;return 1;}
static size_t body_cb(char*p,size_t s,size_t n,void*u){size_t z=s*n;return append((Buffer*)u,p,z)?z:0;}
typedef struct {Buffer body; GSValue headers;} Response;
static size_t response_header(char*p,size_t s,size_t n,void*u){size_t z=s*n;Response*r=u;const char*colon=memchr(p,':',z);if(colon&&colon>p){size_t k=(size_t)(colon-p),i=k+1,e=z;while(k&&isspace((unsigned char)p[k-1]))k--;while(i<z&&isspace((unsigned char)p[i]))i++;while(e>i&&isspace((unsigned char)p[e-1]))e--;if(k&&e>=i){char*key=malloc(k+1),*val=malloc(e-i+1);if(key&&val){memcpy(key,p,k);key[k]=0;memcpy(val,p+i,e-i);val[e-i]=0;api->object_set(&r->headers,key,api->string(val));}free(key);free(val);}}return z;}
static GSValue request_fn(GSValue*a,size_t n){
 if(n<2)return api->null();const char*method=api->cstring(a[0]),*url=api->cstring(a[1]);if(!method||!url||!*method||!*url)return api->null();
 static int curl_ready=0;if(!curl_ready){if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK)return api->null();curl_ready=1;}
 CURL*c=curl_easy_init();if(!c)return api->null();Response r={{0},api->object()};struct curl_slist*hs=NULL;
 if(n>3&&a[3].type==V_OBJECT){size_t count=api->object_count(a[3]);for(size_t i=0;i<count;i++){const char*k=api->object_key(a[3],i);GSValue v=api->object_get(a[3],k);const char*val=api->cstring(v);if(k&&val){size_t z=strlen(k)+strlen(val)+3;char*line=malloc(z);if(line){snprintf(line,z,"%s: %s",k,val);struct curl_slist*next=curl_slist_append(hs,line);free(line);if(next)hs=next;}}}}
 curl_easy_setopt(c,CURLOPT_URL,url);curl_easy_setopt(c,CURLOPT_CUSTOMREQUEST,method);curl_easy_setopt(c,CURLOPT_FOLLOWLOCATION,1L);curl_easy_setopt(c,CURLOPT_MAXREDIRS,5L);curl_easy_setopt(c,CURLOPT_CONNECTTIMEOUT_MS,10000L);curl_easy_setopt(c,CURLOPT_TIMEOUT_MS,30000L);curl_easy_setopt(c,CURLOPT_SSL_VERIFYPEER,1L);curl_easy_setopt(c,CURLOPT_SSL_VERIFYHOST,2L);curl_easy_setopt(c,CURLOPT_NOSIGNAL,1L);curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,body_cb);curl_easy_setopt(c,CURLOPT_WRITEDATA,&r.body);curl_easy_setopt(c,CURLOPT_HEADERFUNCTION,response_header);curl_easy_setopt(c,CURLOPT_HEADERDATA,&r);if(hs)curl_easy_setopt(c,CURLOPT_HTTPHEADER,hs);
 if(n>2&&a[2].type!=V_NULL){const char*body=api->cstring(a[2]);if(body){curl_easy_setopt(c,CURLOPT_POSTFIELDS,body);curl_easy_setopt(c,CURLOPT_POSTFIELDSIZE_LARGE,(curl_off_t)strlen(body));}}
 CURLcode code=curl_easy_perform(c);long status=0;if(code==CURLE_OK)curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&status);
 GSValue out=api->object();api->object_set(&out,"status",api->number((double)status));api->object_set(&out,"ok",api->boolean(code==CURLE_OK&&status>=200&&status<300));api->object_set(&out,"body",api->string(r.body.data?r.body.data:""));api->object_set(&out,"headers",r.headers);api->object_set(&out,"error",api->string(code==CURLE_OK?"":curl_easy_strerror(code)));
 free(r.body.data);curl_slist_free_all(hs);curl_easy_cleanup(c);return out;
}
static GSValue get_fn(GSValue*a,size_t n){if(n<1)return api->null();GSValue args[4]={api->string("GET"),api->copy(a[0]),api->null(),api->null()};return request_fn(args,2);}
static GSValue post_fn(GSValue*a,size_t n){if(n<2)return api->null();GSValue args[4]={api->string("POST"),api->copy(a[0]),api->copy(a[1]),n>2?api->copy(a[2]):api->null()};return request_fn(args,n>2?4:3);}
GSValue greenServeFE_module_init(const GSNativeAPI*napi){api=napi;if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK)return api->null();GSValue m=api->object();api->object_set(&m,"request",api->native(request_fn));api->object_set(&m,"get",api->native(get_fn));api->object_set(&m,"post",api->native(post_fn));return m;}
