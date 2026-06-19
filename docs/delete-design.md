# DELETE Command Design (RP11402-23)

DELETE removes rows from a table and **persists** the change to the CSV file. It
reuses the shared WHERE module ([where](where-design.md)).

## Query format

```
DELETE FROM table [WHERE column = value]
```

## Example

```
DELETE FROM students WHERE major = EE
```

removes every row whose `major` is `EE`, writes the file back, and reports
`N row(s) deleted.`

## Rules

- Keywords (`DELETE`, `FROM`, `WHERE`) are case-insensitive; a trailing `;` is tolerated.
- WHERE is optional. **Without WHERE, every row is deleted** (standard SQL
  behaviour) — and the CLI persists it, so use with care. The header row is kept.
- Row matching reuses the shared `matchingRows`.

## Errors

- Missing `FROM` or table name → parse error.
- WHERE column does not exist → execution error.

## Out of scope (v1)

- Operators other than `=` in WHERE; multiple conditions (`AND`/`OR`).
