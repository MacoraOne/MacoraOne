# gsnum

## Overview

gsnum is a native numeric package. It provides common aggregation and mathematical operations.

Import it with:

~~~gsve
import gsnum;
~~~

## Aggregation

sum accepts multiple numbers or one array:

~~~gsve
print(gsnum.sum(1, 2, 3, 4));
print(gsnum.sum([1, 2, 3, 4]));
~~~

product multiplies arguments:

~~~gsve
print(gsnum.product(2, 3, 4));
~~~

min and max:

~~~gsve
print(gsnum.min(8, 3, 12, 4));
print(gsnum.max(8, 3, 12, 4));
~~~

average accepts multiple arguments. mean accepts multiple arguments or one array:

~~~gsve
print(gsnum.average(10, 20, 30));
print(gsnum.mean([10, 20, 30]));
~~~

## Mathematical functions

The package exports:

- pow
- sqrt
- abs
- floor
- ceil
- round
- sin
- cos
- tan
- mod
- clamp
- factorial

Example:

~~~gsve
print(gsnum.pow(2, 8));
print(gsnum.sqrt(144));
print(gsnum.abs(-12));
print(gsnum.floor(4.9));
print(gsnum.ceil(4.1));
print(gsnum.round(4.5));
print(gsnum.mod(17, 5));
print(gsnum.clamp(120, 0, 100));
print(gsnum.factorial(5));
~~~

## Validation

Native numeric operations return null when their low-level argument requirements are not satisfied. Factorial requires a non-negative integer within the supported implementation range.

Applications should validate important external inputs before calculations.

## Data report

~~~gsve
import gsnum;

define prices = [12.5, 20, 7.5, 30];

define total = gsnum.sum(prices);
define average = gsnum.mean(prices);

print(total);
print(average);
~~~
