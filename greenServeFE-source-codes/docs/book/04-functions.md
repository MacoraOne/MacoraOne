# 4. Functions

Functions become child code objects.

~~~gsve
func add(a, b) {
    return a + b;
}

show(add(10, 20));
~~~

## Recursion

~~~gsve
func factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

show(factorial(5));
~~~

## Closures

Functions can retain their defining environment.

~~~gsve
func make_adder(value) {
    func add_value(other) {
        return value + other;
    }
    return add_value;
}

define add_five = make_adder(5);
show(add_five(7));
~~~
