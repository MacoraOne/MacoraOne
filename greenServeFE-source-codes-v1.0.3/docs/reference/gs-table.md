# gs_table Reference

Functions:

- render(headers, rows, options)
- csv(headers, rows)
- markdown(headers, rows)
- row_count(rows)
- column_count(array)

Render options include title, padding, widths, align, border, and header.

~~~gsve
import gs_table;

define headers = ["Name", "Score"];
define rows = [["Alex", 95], ["Sam", 88]];

print(gs_table.render(headers, rows, {
    title: "Scores",
    align: ["left", "right"]
}));

print(gs_table.csv(headers, rows));
print(gs_table.markdown(headers, rows));
~~~
