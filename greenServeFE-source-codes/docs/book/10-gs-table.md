# gs_table

## Overview

gs_table converts arrays into formatted tables, CSV, and Markdown.

Import:

~~~gsve
import gs_table;
~~~

## Render

~~~gsve
define headers = ["Name", "Age", "Role"];
define rows = [
    ["Alex", 19, "developer"],
    ["Sam", 21, "designer"]
];

print(gs_table.render(headers, rows));
~~~

## Render options

The renderer supports title, padding, widths, align, border, and header.

Example:

~~~gsve
define options = {
    title: "Users",
    padding: 2,
    align: ["left", "right", "left"]
};

print(gs_table.render(
    ["Name", "Age", "Role"],
    [["Alex", 19, "developer"], ["Sam", 21, "designer"]],
    options
));
~~~

Alignment recognizes left, right, and center. A compact border style removes normal border lines.

## CSV

CSV generation quotes values containing commas, quotes, or line breaks:

~~~gsve
print(gs_table.csv(
    ["Name", "Note"],
    [["Alex", "hello, world"], ["Sam", "plain"]]
));
~~~

## Markdown

~~~gsve
print(gs_table.markdown(
    ["Name", "Score"],
    [["Alex", 90], ["Sam", 84]]
));
~~~

## Counts

row_count returns the number of rows. column_count returns the number of elements in the supplied array.

~~~gsve
define rows = [[1, 2], [3, 4], [5, 6]];

print(gs_table.row_count(rows));
print(gs_table.column_count(rows[0]));
~~~

## Reporting pattern

~~~gsve
import gsnum;
import gs_table;

define values = [10, 20, 30, 40];

define rows = [
    ["Total", gsnum.sum(values)],
    ["Average", gsnum.mean(values)],
    ["Minimum", gsnum.min(10, 20, 30, 40)],
    ["Maximum", gsnum.max(10, 20, 30, 40)]
];

print(gs_table.render(
    ["Metric", "Value"],
    rows,
    {title: "Report", align: ["left", "right"]}
));
~~~
