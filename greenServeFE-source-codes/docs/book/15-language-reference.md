# Complete Language Reference

## Keywords

The lexer recognizes:

~~~text
define if else for in while func return break continue import from
true false null
~~~

## Values

Numbers:

~~~gsve
42;
3.14;
~~~

Strings:

~~~gsve
"hello";
~~~

Booleans and null:

~~~gsve
true;
false;
null;
~~~

Arrays:

~~~gsve
[1, 2, 3];
~~~

Objects:

~~~gsve
{name: "greenServeFE", version: 1};
~~~

## Definitions and assignment

~~~gsve
define name = "greenServe";
name = "greenServeFE";
~~~

Array member:

~~~gsve
items[0] = 99;
~~~

Object member:

~~~gsve
user.name = "Alex";
~~~

## Conditionals

~~~gsve
if (condition) {
    statement;
} else {
    statement;
}
~~~

## Loops

~~~gsve
while (condition) {
    statement;
}

for (item in iterable) {
    statement;
}
~~~

## Functions

~~~gsve
func add(a, b) {
    return a + b;
}
~~~

## Imports

~~~gsve
import server;
from gsnum import sum, average;
~~~

## Expressions

Member:

~~~gsve
user.name;
~~~

Index:

~~~gsve
items[index];
~~~

Call:

~~~gsve
add(1, 2);
~~~

Unary:

~~~gsve
-value;
!condition;
~~~

Binary:

~~~gsve
a + b;
a - b;
a * b;
a / b;
a % b;
a == b;
a != b;
a < b;
a <= b;
a > b;
a >= b;
a && b;
a || b;
~~~

## Iteration semantics

Arrays yield their values. Strings yield one-character strings. Objects yield their keys.

## Runtime built-ins

The core VM installs print, show, len, str, num, and type.

## Current implementation boundary

The source implementation is authoritative. If a future release adds syntax, changes precedence, adds built-ins, or changes native package behavior, this reference must be updated in the same release cycle.
