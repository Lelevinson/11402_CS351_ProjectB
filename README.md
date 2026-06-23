# CSV Mini Database — Project B

[![CI](https://github.com/Lelevinson/11402_CS351_ProjectB/actions/workflows/ci.yml/badge.svg)](https://github.com/Lelevinson/11402_CS351_ProjectB/actions/workflows/ci.yml)

A lightweight CSV-backed database with a small SQL-like query engine, in C++20. Load a CSV file, then
read and change it from the command line. Think of it as a simplified spreadsheet you can query.
Built for CS351, AI-Assisted Software Development.

Part of my CS351 coursework. See the [course front page](https://github.com/Lelevinson/11402_CS351).

## What it does

One command-line tool, `csv_query`, runs a single query against a CSV file:

```
csv_query <file.csv> "<query>"
```

It supports five commands:

| Command | Example | Effect |
| --- | --- | --- |
| `SELECT` | `SELECT name, major FROM students` | print chosen columns (or `*`) |
| `SELECT ... WHERE` | `SELECT name FROM students WHERE major = CS` | filter rows by an equality test |
| `INSERT` | `INSERT INTO students VALUES (Dave, 23, Math)` | append a row, save the file |
| `UPDATE` | `UPDATE students SET major = Physics WHERE name = Dave` | change matching rows, save the file |
| `DELETE` | `DELETE FROM students WHERE major = EE` | remove matching rows, save the file |

`SELECT` prints to the screen. `INSERT`, `UPDATE`, and `DELETE` write the change back to the CSV file,
so the data persists. That is what makes it a real little database rather than just a reader.

## Build and run

Requires a C++20 compiler.

```bash
g++ -std=c++20 -Wall -Wextra -o csv_query \
  src/main.cpp src/select.cpp src/where.cpp src/insert.cpp src/update.cpp src/delete.cpp src/csv.cpp
```

A sample file ships in `data/`:

```bash
$ ./csv_query data/students.csv "SELECT name, major FROM students WHERE major = CS"
name,major
Alice,CS
```

## How it is organised

Small, single-purpose modules under [`src/`](src/), so each command stays easy to read and test:

| Module | Responsibility |
| --- | --- |
| `csv` | parse a CSV file into an in-memory `Table`, and serialise it back |
| `where` | the shared equality filter, reused by SELECT, UPDATE, and DELETE |
| `select` | column projection |
| `insert` / `update` / `delete` | the persisted write commands |
| `strutil` | small string helpers (trim, upper-case, splitting) |

The CSV layer handles the awkward parts of the format: quoted fields, commas inside a value, escaped
quotes (`""`), and CRLF line endings. The serialiser re-quotes correctly on the way out, so saved
files stay valid.

## Tests and CI

Six unit-test suites, one per module (`csv`, `select`, `where`, `insert`, `update`, `delete`), run in
GitHub Actions on every push and pull request, followed by a scripted end-to-end demo that runs
SELECT, INSERT, UPDATE, and DELETE against the sample data. See
[`.github/workflows/ci.yml`](.github/workflows/ci.yml). A red check blocks the merge.

Run a suite locally, for example:

```bash
g++ -std=c++20 -Wall -Wextra -o select_tests \
  tests/test_select.cpp src/select.cpp src/where.cpp src/csv.cpp
./select_tests
```

## Project structure

```
11402_CS351_ProjectB/
├── src/        # csv, where, select, insert, update, delete, strutil + main (the CLI)
├── tests/      # one test suite per module
├── data/       # students.csv sample
├── docs/       # per-feature design and test plans
└── .github/    # GitHub Actions CI
```

## A note on the query syntax

This is a deliberately small subset of SQL: one command per run, equality-only `WHERE`, and unquoted
values in the examples. The goal is to show the engine and the workflow clearly, not to be a complete
SQL implementation.

---

Levinson · ID 1123542 · CS351, Yuan Ze University
