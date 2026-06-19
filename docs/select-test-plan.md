# SELECT Test Plan (RP11402-15)

Test plan and case list for the SELECT feature ([design](select-design.md),
[implementation plan](select-implementation-plan.md)). Implemented in
`tests/test_select.cpp` and run in CI on every push and pull request.

## Objective

Verify that `parseSelect`, `executeSelect`, and `runSelect` behave per the
design rules: select columns by name, preserve requested order, support `*`,
and reject malformed queries and unknown columns with clear errors.

## Approach

- Lightweight assertion harness (same style as `tests/test_csv.cpp`): each
  check increments a counter; any failure makes the binary exit non-zero so a
  red CI run blocks the merge.
- Two groups: **parsing** (string → `SelectQuery`) and **execution**
  (`SelectQuery` + `Table` → result `Table`).

## Test cases

### Parsing
| # | Input | Expected |
|---|-------|----------|
| P1 | `SELECT name FROM students` | 1 column `name`, table `students` |
| P2 | `SELECT name, major FROM students` | 2 columns in order |
| P3 | `SELECT * FROM students` | columns = `{*}` |
| P4 | `select name , major from students ;` | case-insensitive keywords, trimmed names, trailing `;` tolerated |
| P5 | `name, major FROM students` | error — missing SELECT |
| P6 | `SELECT name, major students` | error — missing FROM |
| P7 | `SELECT FROM students` | error — empty column list |
| P8 | `SELECT name FROM` | error — empty table |
| P9 | `SELECT name FROM a b` | error — multi-token table |
| P10 | `SELECT name, FROM students` | error — empty column in list |

### Execution (against `name,age,major` / Alice,20,CS / Bob,21,EE)
| # | Query | Expected |
|---|-------|----------|
| E1 | `SELECT name FROM students` | header `[name]`, values `Alice`, `Bob` |
| E2 | `SELECT name, major FROM students` | 2 columns; row 2 = `Bob, EE` |
| E3 | `SELECT major, name FROM students` | headers/values follow requested order (`CS, Alice`) |
| E4 | `SELECT * FROM students` | all 3 columns, all values preserved |
| E5 | `SELECT gpa FROM students` | error — unknown column |

## CI integration

The GitHub Actions workflow (`.github/workflows/ci.yml`) compiles
`tests/test_select.cpp` with `src/select.cpp` and `src/csv.cpp` using
`g++ -std=c++20 -Wall -Wextra` and runs the resulting binary. A non-zero exit
fails the job and blocks merging to `main`.
