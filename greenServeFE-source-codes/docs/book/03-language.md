# 3. Language Basics

## Variables

~~~gsve
define name = "greenServeFE";
define version = 1;
define ready = true;
show(name);
show(version);
show(ready);
~~~

## Arrays and objects

~~~gsve
define numbers = [10, 20, 30];
show(numbers[0]);

define user = {
    name: "Alex",
    role: "developer"
};
show(user.name);
~~~

## Control flow

~~~gsve
define n = 0;
while (n < 3) {
    show(n);
    n = n + 1;
}

for (item in [1, 2, 3]) {
    show(item);
}
~~~

The VM also supports break and continue.
