# 5. Native Modules

Native modules are shared objects loaded through the ABI declared in module.h.

The loader can search a direct path, the current directory, ./modules, and directories in GREENSERVE_MODULE_PATH.

The required initializer symbol is greenServeFE_module_init.

~~~gsve
import gsnum;

show(gsnum.sum(1, 2, 3));
~~~

The current ABI version is 1. A native initializer must return an object.

Native packages must match the greenServeFE ABI they were built for.
