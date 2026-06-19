# WHERE Test Plan (RP11402-19)

Test plan and case list for the WHERE clause ([design](where-design.md)).
Implemented in `tests/test_where.cpp` and run in CI on every push and pull
request.

## Objective

Verify that WHERE parses into a `WhereClause` and that `executeSelect` filters
rows by equality before projection, per the design rules (string equality,
quoted/unquoted value, WHERE column need not be selected, `*` honours the
filter), and rejects malformed clauses and unknown columns.

## Test cases

### Parsing
| # | Input | Expected |
|---|-------|----------|
| W1 | `SELECT name FROM students WHERE major = CS` | WHERE present, column `major`, value `CS`; columns unaffected |
| W2 | `select * from students where major = "CS" ;` | case-insensitive `WHERE`, quotes stripped |
| W3 | `SELECT name FROM students WHERE major = 'EE'` | single quotes stripped |
| W4 | `SELECT name FROM students` | no WHERE → `present == false` |
| W5 | `... WHERE major` | error — no `=` |
| W6 | `... WHERE = CS` | error — no column |
| W7 | `... WHERE` | error — empty WHERE clause |

### Execution (against `name,age,major` / Alice,20,CS / Bob,21,EE / Carol,22,CS)
| # | Query | Expected |
|---|-------|----------|
| X1 | `SELECT name FROM students WHERE major = CS` | rows `Alice`, `Carol` |
| X2 | `SELECT name FROM students WHERE major = Bio` | no rows; header still present |
| X3 | `SELECT name FROM students WHERE major = EE` | filter on a non-selected column → `Bob` |
| X4 | `SELECT * FROM students WHERE major = CS` | all columns, 2 rows (`Alice`, `Carol`) |
| X5 | `SELECT name FROM students WHERE gpa = 4` | error — unknown WHERE column |

## CI integration

The GitHub Actions workflow compiles `tests/test_where.cpp` with
`src/select.cpp` and `src/csv.cpp` (`g++ -std=c++20 -Wall -Wextra`) and runs the
binary; a non-zero exit fails the job and blocks merging to `main`.
