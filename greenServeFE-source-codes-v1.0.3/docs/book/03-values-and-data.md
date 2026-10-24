# Values, Arrays, and Objects

## Runtime values

The runtime represents null, numbers, booleans, strings, arrays, objects, functions, native functions, and iterators.

These types are not merely documentation categories. The VM uses them when executing calls, indexing, iteration, member access, and operators.

## Arrays

Create an array with brackets:

~~~gsve
define numbers = [10, 20, 30, 40];
print(numbers[0]);
print(numbers[3]);
~~~

Indexes can be expressions:

~~~gsve
define index = 1;
define names = ["Ana", "Ben", "Chris"];
print(names[index]);
~~~

Assignment to an index is supported:

~~~gsve
define numbers = [1, 2, 3];
numbers[1] = 20;
print(numbers);
~~~

Arrays are iterable:

~~~gsve
for (number in [10, 20, 30]) {
    print(number);
}
~~~

## Objects

Objects contain named properties:

~~~gsve
define user = {
    name: "Alex",
    role: "developer",
    active: true
};

print(user.name);
print(user.role);
~~~

Members can be assigned:

~~~gsve
user.active = false;
print(user.active);
~~~

A computed key can use index syntax:

~~~gsve
define key = "name";
print(user[key]);
~~~

## Object iteration

The current iterator yields object keys:

~~~gsve
define settings = {
    host: "127.0.0.1",
    port: 8080,
    debug: true
};

for (key in settings) {
    print(key);
    print(settings[key]);
}
~~~

## Nested data

Arrays and objects can be nested:

~~~gsve
define response = {
    status: 200,
    user: {
        name: "Alex",
        roles: ["admin", "developer"]
    }
};

print(response.user.name);
print(response.user.roles[0]);
~~~

## Data modeling

Use arrays for ordered collections and objects for records:

~~~gsve
define books = [
    {title: "Programming", price: 30},
    {title: "Networking", price: 25},
    {title: "Databases", price: 40}
];

for (book in books) {
    print(book.title);
    print(book.price);
}
~~~

This model is especially useful for JSON-like server responses and template data.
