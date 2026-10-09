#include "module.h"

#include <math.h>

static const GSNativeAPI *api;

static int number(GSValue value, double *out)
{
    if (value.type != V_NUMBER) return 0;
    *out = api->number_value(value);
    return 1;
}

static GSValue sum(GSValue *args, size_t argc)
{
    if (argc == 1 && args[0].type == V_ARRAY) {
        double total = 0;
        size_t count = api->array_count(args[0]);
        for (size_t i = 0; i < count; i++) {
            GSValue item = api->array_get(args[0], i);
            double value;
            if (!number(item, &value)) {
                api->release(item);
                return api->null();
            }
            total += value;
            api->release(item);
        }
        return api->number(total);
    }

    double total = 0;
    for (size_t i = 0; i < argc; i++) {
        double value;
        if (!number(args[i], &value)) return api->null();
        total += value;
    }
    return api->number(total);
}

static GSValue product(GSValue *args, size_t argc)
{
    double result = 1;
    for (size_t i = 0; i < argc; i++) {
        double value;
        if (!number(args[i], &value)) return api->null();
        result *= value;
    }
    return api->number(result);
}

static GSValue min_value(GSValue *args, size_t argc)
{
    if (!argc) return api->null();
    double result;
    if (!number(args[0], &result)) return api->null();

    for (size_t i = 1; i < argc; i++) {
        double value;
        if (!number(args[i], &value)) return api->null();
        if (value < result) result = value;
    }

    return api->number(result);
}

static GSValue max_value(GSValue *args, size_t argc)
{
    if (!argc) return api->null();
    double result;
    if (!number(args[0], &result)) return api->null();

    for (size_t i = 1; i < argc; i++) {
        double value;
        if (!number(args[i], &value)) return api->null();
        if (value > result) result = value;
    }

    return api->number(result);
}

static GSValue average(GSValue *args, size_t argc)
{
    if (!argc) return api->null();
    GSValue total = sum(args, argc);
    if (total.type != V_NUMBER) return total;
    double result = api->number_value(total) / (double)argc;
    api->release(total);
    return api->number(result);
}

static GSValue unary_math(GSValue *args, size_t argc, double (*fn)(double))
{
    if (argc != 1) return api->null();
    double value;
    if (!number(args[0], &value)) return api->null();
    return api->number(fn(value));
}

static GSValue pow_value(GSValue *args, size_t argc)
{
    double a, b;
    if (argc != 2 || !number(args[0], &a) || !number(args[1], &b))
        return api->null();
    return api->number(pow(a, b));
}

static GSValue mod_value(GSValue *args, size_t argc)
{
    double a, b;
    if (argc != 2 || !number(args[0], &a) || !number(args[1], &b) || b == 0)
        return api->null();
    return api->number(fmod(a, b));
}

static GSValue clamp_value(GSValue *args, size_t argc)
{
    double value, low, high;
    if (argc != 3 ||
        !number(args[0], &value) ||
        !number(args[1], &low) ||
        !number(args[2], &high) ||
        low > high) return api->null();

    if (value < low) value = low;
    if (value > high) value = high;
    return api->number(value);
}

static GSValue factorial(GSValue *args, size_t argc)
{
    if (argc != 1) return api->null();
    double value;
    if (!number(args[0], &value) ||
        value < 0 ||
        floor(value) != value ||
        value > 170) return api->null();

    double result = 1;
    for (unsigned long long i = 2; i <= (unsigned long long)value; i++)
        result *= (double)i;

    return api->number(result);
}

static GSValue sqrt_value(GSValue *args, size_t argc) { return unary_math(args, argc, sqrt); }
static GSValue abs_value(GSValue *args, size_t argc) { return unary_math(args, argc, fabs); }
static GSValue floor_value(GSValue *args, size_t argc) { return unary_math(args, argc, floor); }
static GSValue ceil_value(GSValue *args, size_t argc) { return unary_math(args, argc, ceil); }
static GSValue round_value(GSValue *args, size_t argc) { return unary_math(args, argc, round); }
static GSValue sin_value(GSValue *args, size_t argc) { return unary_math(args, argc, sin); }
static GSValue cos_value(GSValue *args, size_t argc) { return unary_math(args, argc, cos); }
static GSValue tan_value(GSValue *args, size_t argc) { return unary_math(args, argc, tan); }

static GSValue mean(GSValue *args, size_t argc)
{
    if (argc == 1 && args[0].type == V_ARRAY) {
        size_t count = api->array_count(args[0]);
        if (!count) return api->null();

        double total = 0;
        for (size_t i = 0; i < count; i++) {
            GSValue item = api->array_get(args[0], i);
            double value;
            if (!number(item, &value)) {
                api->release(item);
                return api->null();
            }
            total += value;
            api->release(item);
        }

        return api->number(total / (double)count);
    }

    return average(args, argc);
}

GSValue greenServeFE_module_init(const GSNativeAPI *native_api)
{
    api = native_api;
    GSValue module = api->object();

    api->object_set(&module, "sum", api->native(sum));
    api->object_set(&module, "product", api->native(product));
    api->object_set(&module, "min", api->native(min_value));
    api->object_set(&module, "max", api->native(max_value));
    api->object_set(&module, "average", api->native(average));
    api->object_set(&module, "mean", api->native(mean));
    api->object_set(&module, "pow", api->native(pow_value));
    api->object_set(&module, "sqrt", api->native(sqrt_value));
    api->object_set(&module, "abs", api->native(abs_value));
    api->object_set(&module, "floor", api->native(floor_value));
    api->object_set(&module, "ceil", api->native(ceil_value));
    api->object_set(&module, "round", api->native(round_value));
    api->object_set(&module, "sin", api->native(sin_value));
    api->object_set(&module, "cos", api->native(cos_value));
    api->object_set(&module, "tan", api->native(tan_value));
    api->object_set(&module, "mod", api->native(mod_value));
    api->object_set(&module, "clamp", api->native(clamp_value));
    api->object_set(&module, "factorial", api->native(factorial));

    return module;
}
