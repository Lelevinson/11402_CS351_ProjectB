# UPDATE Command Design (RP11402-22)

UPDATE changes a column's value on selected rows and **persists** the change to
the CSV file. It reuses the shared WHERE module ([where](where-design.md)).

## Query format

```
UPDATE table SET column = value [WHERE column = value]
```

## Example

```
UPDATE students SET major = Math WHERE name = Bob
```

sets Bob's `major` to `Math`, writes the file back, and reports `1 row(s) updated.`

## Rules

- Keywords (`UPDATE`, `SET`, `WHERE`) are case-insensitive; a trailing `;` is tolerated.
- `SET column = value` sets one column; the value may be quoted (quotes stripped)
  and may be empty.
- WHERE is optional. **Without WHERE, every row is updated** (standard SQL
  behaviour) — and the CLI persists it, so use with care.
- Filtering reuses the shared `matchingRows`, so the WHERE column need not be the
  SET column.

## Errors

- Missing `SET` or no `=` in the assignment → parse error.
- SET column or WHERE column does not exist → execution error.

## Out of scope (v1) / known limitation

- Setting multiple columns in one statement (`SET a = 1, b = 2`).
- Operators other than `=` in WHERE.
- A SET value that itself contains the standalone word `WHERE` (the simple
  keyword split would misread it); quote-aware splitting is a future improvement.
