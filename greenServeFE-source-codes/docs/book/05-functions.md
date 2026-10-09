# Functions and Closures

## Declaration

Functions use func:

~~~gsve
func add(a, b) {
    return a + b;
}

print(add(10, 5));
~~~

Parameters are positional.

## Return

return exits a function and supplies its value:

~~~gsve
func absolute(value) {
    if (value < 0) {
        return -value;
    }
    return value;
}

print(absolute(-12));
~~~

A function that reaches the end has an implicit null return.

## Missing arguments

The current VM supplies null for a missing parameter:

~~~gsve
func greet(name) {
    if (name == null) {
        return "Hello";
    }
    return "Hello " + name;
}

print(greet());
print(greet("Alex"));
~~~

This is useful for optional-style functions, but required application arguments should still be validated.

## Functions as values

Function values can be stored:

~~~gsve
func multiply(a, b) {
    return a * b;
}

define operation = multiply;
print(operation(6, 7));
~~~

## Closures

A nested function captures the environment in which it was created:

~~~gsve
func make_adder(base) {
    func add(value) {
        return base + value;
    }
    return add;
}

define add10 = make_adder(10);

print(add10(5));
print(add10(20));
~~~

The outer variable base remains available to the returned function.

## Mutable captured state

A closure can capture and update state:

~~~gsve
func counter() {
    define value = 0;

    func next() {
        value = value + 1;
        return value;
    }

    return next;
}

define next_value = counter();

print(next_value());
print(next_value());
print(next_value());
~~~

## Function factories

Factories can create configured functions:

~~~gsve
func make_multiplier(factor) {
    func multiply(value) {
        return value * factor;
    }
    return multiply;
}

define double = make_multiplier(2);
define triple = make_multiplier(3);

print(double(8));
print(triple(8));
~~~

## Server handlers

Functions are the reason route handlers can be written naturally:

~~~gsve
import server;

func home(req, res) {
    res.send("Hello from a GSVE function");
}

server.get("/", home);
~~~

The server module invokes the function through the native VM callback bridge.
