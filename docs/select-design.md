# SELECT Feature Design Plan

## Purpose

The SELECT feature allows users to choose specific columns from CSV data in Project B.

## Basic Query Format

The first version will support a simple format:

SELECT column1, column2 FROM table

## Example

Given CSV data:

name,age,major
Alice,20,CS
Bob,21,EE

Query:

SELECT name, major FROM students

Expected output:

name,major
Alice,CS
Bob,EE

## Design Rules

- The first row of the CSV file is treated as the column header.
- SELECT chooses one or more columns by name.
- The output keeps the selected column order.
- If a column does not exist, the program should show an error.
- This first version does not include WHERE, ORDER BY, JOIN, or aggregation.

## Completion Criteria

This task is complete when the expected SELECT behavior, query format, example, and design rules are clearly documented.
