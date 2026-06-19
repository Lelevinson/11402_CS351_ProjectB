# SELECT Implementation Plan (RP11402-13)

This plan turns the [SELECT feature design](select-design.md) into a concrete
implementation approach. It defines the modules, function signatures, parsing
strategy, error handling, and testing approach so the code work (RP11402-14)
and the test work (RP11402-15) have a clear blueprint.

## 1. Scope (from the design doc)

Support exactly:

```
SELECT column1, column2 FROM table
```

- First CSV row = column headers (already provided by the CSV parser, RP11402-16).
- SELECT picks one or more columns **by name**; output preserves the requested order.
- Unknown column → clear error.
- Out of scope for this version: `WHERE`, `ORDER BY`, `JOIN`, aggregation.
- Small convenience extension: `SELECT *` returns all columns (documented, not in the original doc).

## 2. Modules and files

| File | Responsibility |
|------|----------------|
| `src/csv.{h,cpp}` | (done, RP11402-16) Load CSV → `Table` (headers + rows). |
| `src/select.{h,cpp}` | **New.** Parse a SELECT string and execute it against a `Table`. |
| `src/main.cpp` | **New.** CLI demo: load a CSV, run a query, print the result as CSV. |
| `tests/test_select.cpp` | (RP11402-15) Unit tests for parsing + execution. |

## 3. Public interface (`src/select.h`)

```cpp
struct SelectQuery {
    std::vector<std::string> columns;  // requested columns, in order; {"*"} = all
    std::string table;                 // table name from the FROM clause
};

SelectQuery parseSelect(const std::string& query);          // syntax errors -> std::runtime_error
Table       executeSelect(const SelectQuery&, const Table&); // unknown column -> std::out_of_range
Table       runSelect(const std::string& query, const Table&); // parse + execute
```

`executeSelect` returns a new `Table` whose `headers` are the selected columns
(in requested order) and whose `rows` contain only those columns.

## 4. Parsing strategy

A deliberately small, case-insensitive parser (no general SQL grammar):

1. Trim surrounding whitespace; strip a trailing `;` if present.
2. Require the statement to begin with the keyword `SELECT` (case-insensitive).
3. Split on the keyword `FROM` (case-insensitive, surrounded by whitespace) into
   a **column section** and a **table section**.
4. Column section: split on commas, trim each name. `*` alone → `{"*"}`.
5. Table section: trim; must be exactly one token.

### Error cases (all raise `std::runtime_error` with a clear message)
- Missing `SELECT` keyword.
- Missing `FROM` keyword.
- Empty column list (e.g. `SELECT FROM t`).
- Empty or multi-token table name.

## 5. Execution strategy

- `SELECT *` → return a copy of the input table.
- Otherwise resolve each requested column to an index via `Table::columnIndex`
  (which throws `std::out_of_range` if the column is absent — satisfies the
  "unknown column → error" rule).
- Build the result: `headers` = requested names; for each input row, copy the
  values at the resolved indices in order.

## 6. CLI demo (`src/main.cpp`)

Usage: `csv_query <file.csv> "<query>"`. Loads the CSV, runs the query, prints
the result as CSV to stdout; prints the error message and exits non-zero on any
parse/execution failure. This gives CI something to compile + run, and gives the
live demo a real command to show.

## 7. Testing approach (handed to RP11402-15)

- **Parsing**: column lists, `*`, case-insensitivity, trailing `;`, and each error case.
- **Execution**: single/multiple/reordered columns, `*`, unknown-column error.
- **End-to-end**: `runSelect` over the sample `data/students.csv`.
- Reuse the lightweight assertion harness style from `tests/test_csv.cpp`; wire
  the new test binary into the GitHub Actions workflow so failures block merges.

## 8. Build / CI

- Compile with `g++ -std=c++20 -Wall -Wextra` (matches existing CI).
- RP11402-14 extends CI to build the demo program (`src/main.cpp src/select.cpp src/csv.cpp`).
- RP11402-15 adds a step to compile and run `tests/test_select.cpp`.
</content>
