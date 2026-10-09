# Native Package Development

## Module contract

A native package is a shared object exporting greenServeFE_module_init.

The initializer receives the GSNativeAPI table and returns a GSValue object containing exported members.

## Minimal function

~~~c
#include "module.h"

static const GSNativeAPI *api;

static GSValue add(GSValue *args, size_t argc)
{
    if (argc != 2)
        return api->null();

    if (args[0].type != V_NUMBER || args[1].type != V_NUMBER)
        return api->null();

    return api->number(
        api->number_value(args[0]) +
        api->number_value(args[1])
    );
}

GSValue greenServeFE_module_init(const GSNativeAPI *native_api)
{
    api = native_api;

    GSValue module = api->object();
    api->object_set(&module, "add", api->native(add));
    return module;
}
~~~

## Argument validation

Every native function should validate:

- argument count;
- argument types;
- assumptions about array/object contents;
- allocation results;
- temporary value ownership.

## Objects

~~~c
GSValue result = api->object();
GSValue text = api->string("ok");

api->object_set(&result, "status", text);
api->release(text);

return result;
~~~

## Arrays

~~~c
GSValue result = api->array();

GSValue first = api->number(10);
api->array_push(&result, first);
api->release(first);

return result;
~~~

Use the API rather than manipulating internal array and object structures.

## Callback bridge

The callback bridge has the form:

~~~c
int (*call)(void *, GSValue, GSValue *, size_t, GSValue *);
~~~

The server uses it to invoke a GSVE handler. The context is the VM instance.

This bridge makes the following source-level pattern possible:

~~~gsve
server.get("/", func(req, res) {
    res.send("hello");
});
~~~

## ABI stability

The ABI version is currently 1. Existing field order should be preserved. New API fields should be appended rather than inserted into the middle of the structure when compatibility is required.

Native packages should be rebuilt and tested whenever the ABI changes.

## CI policy

Do not say a native package passed CI unless the job actually builds or installs it and loads it with the current executable. Core language CI and native package CI can be separate jobs when that better matches the distribution process.
