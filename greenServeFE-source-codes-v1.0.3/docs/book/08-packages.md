# 8. Packages

## gsnum

The v1.0.0 module exports sum, product, min, max, average, mean, pow, sqrt, abs, floor, ceil, round, sin, cos, tan, mod, clamp, and factorial.

~~~gsve
import gsnum;

show(gsnum.sum(1, 2, 3, 4));
show(gsnum.product(2, 3, 4));
show(gsnum.sqrt(81));
show(gsnum.factorial(5));
~~~

## gs_table

The v1.0.0 module exports render, csv, markdown, row_count, and column_count.

~~~gsve
import gs_table;

define headers = ["Name", "Value"];
define rows = [
    ["greenServeFE", "VM"],
    ["version", "1"]
];

show(gs_table.render(headers, rows));
show(gs_table.markdown(headers, rows));
show(gs_table.csv(headers, rows));
~~~
