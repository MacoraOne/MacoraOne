# Control Flow

## if and else

A conditional consists of a condition and a statement block:

~~~gsve
define score = 82;

if (score >= 90) {
    print("excellent");
} else if (score >= 70) {
    print("pass");
} else {
    print("retry");
}
~~~

The else branch may contain another if statement.

## while

A while loop repeats while its condition remains truthy:

~~~gsve
define n = 1;
define total = 0;

while (n <= 5) {
    total = total + n;
    n = n + 1;
}

print(total);
~~~

A loop should change state so that its condition eventually becomes false.

## for in

The for form iterates an array, object, or string.

Array:

~~~gsve
for (value in [1, 2, 3, 4]) {
    print(value);
}
~~~

String:

~~~gsve
for (letter in "green") {
    print(letter);
}
~~~

Object:

~~~gsve
for (key in {one: 1, two: 2}) {
    print(key);
}
~~~

## break

break leaves the nearest loop:

~~~gsve
for (n in [1, 2, 3, 4, 5]) {
    if (n == 4) {
        break;
    }
    print(n);
}
~~~

## continue

continue jumps to the next iteration:

~~~gsve
for (n in [1, 2, 3, 4, 5]) {
    if (n == 3) {
        continue;
    }
    print(n);
}
~~~

## Nested loops

Nested iteration is ordinary syntax:

~~~gsve
for (row in [1, 2, 3]) {
    for (column in [1, 2, 3]) {
        print(row * column);
    }
}
~~~

## Conditions over structured data

~~~gsve
define account = {
    active: true,
    balance: 120
};

if (account.active && account.balance >= 100) {
    print("eligible");
} else {
    print("not eligible");
}
~~~

## Design advice

Use a named function when a condition contains enough computation that the surrounding control flow becomes difficult to read. Keep loops small and explicit.
