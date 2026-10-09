# Built-in Functions

## print

print writes its arguments separated by spaces and terminates the line.

~~~gsve
print("name:", "greenServeFE");
print(1, 2, 3);
~~~

## len

Returns the length of a string, array, or object.

## str

Converts a value to a string representation.

## num

Obtains a numeric representation through the runtime conversion helper.

## type

Returns the runtime category of a value.

Possible categories include null, number, bool, string, array, object, function, and iterator.

## Example

~~~gsve
define values = [null, 10, true, "text", [1], {x: 1}];

for (value in values) {
    print(type(value));
}
~~~
