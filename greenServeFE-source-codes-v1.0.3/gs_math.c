#include "module.h"
#include <math.h>

static const GSNativeAPI *api;

static GSValue unary(GSValue *a, size_t n, double (*fn)(double)) {
    if (n < 1) return api->null();
    double x = api->number_value(a[0]);
    if (!isfinite(x)) return api->null();
    double y = fn(x);
    return isfinite(y) ? api->number(y) : api->null();
}
static double abs_fn(double x) { return fabs(x); }
static double floor_fn(double x) { return floor(x); }
static double ceil_fn(double x) { return ceil(x); }
static double round_fn(double x) { return round(x); }
static double sqrt_fn(double x) { return x < 0 ? NAN : sqrt(x); }
static double log_fn(double x) { return x <= 0 ? NAN : log(x); }
static double log10_fn(double x) { return x <= 0 ? NAN : log10(x); }
static double exp_fn(double x) { return exp(x); }
static GSValue abs_value(GSValue*a,size_t n){return unary(a,n,abs_fn);}
static GSValue floor_value(GSValue*a,size_t n){return unary(a,n,floor_fn);}
static GSValue ceil_value(GSValue*a,size_t n){return unary(a,n,ceil_fn);}
static GSValue round_value(GSValue*a,size_t n){return unary(a,n,round_fn);}
static GSValue sqrt_value(GSValue*a,size_t n){return unary(a,n,sqrt_fn);}
static GSValue log_value(GSValue*a,size_t n){return unary(a,n,log_fn);}
static GSValue log10_value(GSValue*a,size_t n){return unary(a,n,log10_fn);}
static GSValue exp_value(GSValue*a,size_t n){return unary(a,n,exp_fn);}
static GSValue pow_value(GSValue*a,size_t n){
    if(n<2)return api->null();
    double x=pow(api->number_value(a[0]),api->number_value(a[1]));
    return isfinite(x)?api->number(x):api->null();
}
static GSValue min_value(GSValue*a,size_t n){
    if(n<1)return api->null(); double v=api->number_value(a[0]);
    for(size_t i=1;i<n;i++){double x=api->number_value(a[i]);if(x<v)v=x;}
    return isfinite(v)?api->number(v):api->null();
}
static GSValue max_value(GSValue*a,size_t n){
    if(n<1)return api->null(); double v=api->number_value(a[0]);
    for(size_t i=1;i<n;i++){double x=api->number_value(a[i]);if(x>v)v=x;}
    return isfinite(v)?api->number(v):api->null();
}
static GSValue clamp_value(GSValue*a,size_t n){
    if(n<3)return api->null();
    double x=api->number_value(a[0]),lo=api->number_value(a[1]),hi=api->number_value(a[2]);
    if(lo>hi){double t=lo;lo=hi;hi=t;}
    if(x<lo)x=lo;if(x>hi)x=hi;
    return isfinite(x)?api->number(x):api->null();
}
static GSValue pi_value(GSValue*a,size_t n){(void)a;(void)n;return api->number(3.14159265358979323846);}
static GSValue e_value(GSValue*a,size_t n){(void)a;(void)n;return api->number(2.71828182845904523536);}

GSValue greenServeFE_module_init(const GSNativeAPI *native_api) {
    api=native_api;
    GSValue m=api->object();
    api->object_set(&m,"abs",api->native(abs_value));
    api->object_set(&m,"floor",api->native(floor_value));
    api->object_set(&m,"ceil",api->native(ceil_value));
    api->object_set(&m,"round",api->native(round_value));
    api->object_set(&m,"sqrt",api->native(sqrt_value));
    api->object_set(&m,"log",api->native(log_value));
    api->object_set(&m,"log10",api->native(log10_value));
    api->object_set(&m,"exp",api->native(exp_value));
    api->object_set(&m,"pow",api->native(pow_value));
    api->object_set(&m,"min",api->native(min_value));
    api->object_set(&m,"max",api->native(max_value));
    api->object_set(&m,"clamp",api->native(clamp_value));
    api->object_set(&m,"pi",api->native(pi_value));
    api->object_set(&m,"e",api->native(e_value));
    return m;
}
