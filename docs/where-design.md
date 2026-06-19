# WHERE Clause Design (RP11402-17)

The WHERE clause adds row filtering on top of the existing
[SELECT feature](select-design.md). It is the second query capability of the
CSV mini database.

## Query format

```
SELECT column1, column2 FROM table WHERE column = value
```

WHERE is optional; a query without it behaves exactly as before.

## Example

Given `data/students.csv`:

```
name,age,major
Alice,20,CS
Bob,21,EE
Carol,22,Math
```

Query:

```
SELECT name, major FROM students WHERE major = CS
```

Output:

```
name,major
Alice,CS
```

## Design rules

- A single equality condition only: `column = value`.
- Comparison is **string equality** against the raw CSV cell value.
- The value may be written **unquoted** (`major = CS`) or **quoted**
  (`major = "CS"` / `major = 'CS'`); surrounding quotes are stripped.
- The WHERE column is matched by name against the header row, and **need not
  appear in the SELECT list** (e.g. you can filter on `major` while selecting
  only `name`).
- Filtering happens **before** column projection.
- `SELECT *` may be combined with WHERE.
- The `WHERE` keyword is case-insensitive, like `SELECT` and `FROM`.

## Error cases

- WHERE column does not exist in the table → error.
- WHERE clause is present but malformed (no `=`, or empty column) → error.

## Out of scope (v1)

- Multiple conditions (`AND` / `OR`).
- Operators other than `=` (e.g. `!=`, `<`, `>`, `LIKE`).
- `ORDER BY`, aggregation, joins.

These are natural future extensions but are intentionally excluded to keep the
first WHERE version small and well-tested.
