# INSERT Command Design (RP11402-21)

INSERT adds a new row to a table and **persists** it back to the CSV file — the
first mutating command of the CSV mini database.

## Query format

```
INSERT INTO table VALUES (value1, value2, ...)
```

## Example

```
INSERT INTO students VALUES (Carol, 22, Math)
```

appends `Carol,22,Math` to the file and reports `1 row inserted.`

## Rules

- Keywords (`INSERT`, `INTO`, `VALUES`) are case-insensitive; a trailing `;` is tolerated.
- Values are positional and map left-to-right onto the header columns.
- The number of values **must equal** the number of columns, else an error.
- A value may be quoted (single or double) to contain a comma; surrounding
  quotes are stripped. Empty values are allowed (an empty field).
- The CLI loads the file, appends the row, and writes the file back via
  `saveCsvFile` (correct CSV quoting on write).

## Errors

- Missing `INTO` / `VALUES` / parentheses → parse error.
- Value count ≠ column count → execution error.

## Out of scope (v1)

- Column-list form (`INSERT INTO t (a, b) VALUES (...)`).
- Inserting multiple rows in one statement.
- Type/constraint checking (all values are stored as strings).
