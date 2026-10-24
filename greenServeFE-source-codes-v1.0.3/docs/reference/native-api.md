# Native API Reference

The native interface is defined by module.h.

## ABI

GSFE_MODULE_ABI_VERSION is currently 1.

## Constructors

null, number, boolean, string, object, array, native.

## Lifetime

copy, retain, release.

## Arrays

array_push, array_count, array_get, array_set.

## Objects

object_count, object_key, object_set, object_get.

## Conversion

cstring, truthy, number_value.

## Output

print.

## Callback bridge

The native API contains:

~~~c
void *context;
int (*call)(void *, GSValue, GSValue *, size_t, GSValue *);
~~~

The server module uses the callback bridge to invoke GSVE route functions.

## Initializer

~~~c
GSValue greenServeFE_module_init(const GSNativeAPI *api);
~~~

The initializer must return an object.
