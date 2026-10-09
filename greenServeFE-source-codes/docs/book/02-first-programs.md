# First Programs and Expressions

## Statements

A source file is a sequence of statements. Statements can define names, assign values, evaluate calls, control execution, declare functions, or load modules.

The smallest program is:

~~~gsve
print("hello");
~~~

Several statements execute in source order:

~~~gsve
print("first");
print("second");
print("third");
~~~

## Definitions

Use define:

~~~gsve
define language = "greenServeFE";
define version = 1;
define message = language + " " + str(version);
print(message);
~~~

A definition may contain a complete expression:

~~~gsve
define total = 10 + 20 * 3;
print(total);
~~~

## Assignment

A defined name can be reassigned:

~~~gsve
define total = 10;
total = total + 5;
print(total);
~~~

Assignment also works with array indexes and object members.

## Operators

Arithmetic:

~~~text
+  -  *  /  %
~~~

Comparison:

~~~text
==  !=  <  <=  >  >=
~~~

Logical:

~~~text
&&  ||
~~~

Unary:

~~~text
-  !
~~~

Example:

~~~gsve
define a = 12;
define b = 5;

print(a + b);
print(a - b);
print(a * b);
print(a / b);
print(a % b);
print(a > b);
print(a == b);
print(a != b);
print(a > 10 && b < 10);
~~~

## Built-ins

The core VM provides print, len, str, num, and type.

Inspect a value:

~~~gsve
define value = 42;
print(type(value));
print(str(value));
print(num(value));
~~~

Measure values:

~~~gsve
define name = "greenServe";
define items = [10, 20, 30];
define record = {name: "Alex", role: "developer"};

print(len(name));
print(len(items));
print(len(record));
~~~

## Parentheses

Use parentheses whenever they make a calculation easier to read:

~~~gsve
define total = (price + tax) * quantity;
~~~

Do not write complicated expressions merely to rely on precedence rules. Clear source is easier to maintain and debug.
