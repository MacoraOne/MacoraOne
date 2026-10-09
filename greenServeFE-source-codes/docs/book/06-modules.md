# Modules and the Native ABI

## Why native modules exist

The core interpreter provides the language and execution engine. Native shared libraries add capabilities such as HTTP, numerical functions, and formatted tables.

The VM loads shared libraries dynamically and looks for the initializer symbol greenServeFE_module_init.

## Importing a module

~~~gsve
import gsnum;

print(gsnum.sum(10, 20, 30));
~~~

Selected members can be imported:

~~~gsve
from gsnum import sum, average;

print(sum(1, 2, 3));
print(average(10, 20, 30));
~~~

## Search paths

The VM recognizes local module paths and GREENSERVE_MODULE_PATH.

A typical system layout is:

~~~text
/usr/local/lib/modules/gsnum.so
/usr/local/lib/modules/gs_table.so
/usr/local/lib/modules/server.so
~~~

Set:

~~~text
export GREENSERVE_MODULE_PATH=/usr/local/lib/modules
~~~

## Initializer contract

A native module exports:

~~~c
GSValue greenServeFE_module_init(const GSNativeAPI *api)
~~~

and returns an object.

A minimal package looks like:

~~~c
#include "module.h"

static const GSNativeAPI *api;

static GSValue hello(GSValue *args, size_t argc)
{
    (void)args;
    (void)argc;
    return api->string("hello");
}

GSValue greenServeFE_module_init(const GSNativeAPI *native_api)
{
    api = native_api;
    GSValue module = api->object();
    api->object_set(&module, "hello", api->native(hello));
    return module;
}
~~~

## API groups

The native API supplies constructors, lifetime operations, array operations, object operations, conversion helpers, printing, and the VM callback bridge.

Native code should use these functions instead of depending on private VM fields.

## Ownership

Values returned from access functions are owned values. Native code must release temporary values when the API contract requires it.

Objects and arrays should be manipulated through object_set, object_get, array_push, array_get, and related functions.

## Callback bridge

The API ends with a context pointer and call function:

~~~c
void *context;
int (*call)(void *, GSValue, GSValue *, size_t, GSValue *);
~~~

The server module uses these fields to invoke a GSVE route function with request and response objects.

## ABI version

The current ABI version is 1. When the ABI changes, native packages must be rebuilt and tested with the corresponding runtime.

Do not assume that a shared object from another runtime release is compatible merely because the filename matches.
