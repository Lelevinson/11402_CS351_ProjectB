# Project B: CSV Mini Database and Query Engine

## Overview

Project B involves building a lightweight database system that can read CSV (Comma-Separated Values) files and provide a query engine to search, filter, and manipulate data within those files. Think of it as a simplified version of a spreadsheet application—it allows you to load data from CSV files and ask questions about that data using query commands.

## Core Objectives

- **CSV Data Loading**: Parse and load CSV files into an in-memory data structure
- **Data Storage**: Store the loaded data in a format that makes it easy to access and manipulate
- **Query Engine**: Implement a system to perform common database operations like:
  - Filtering rows based on conditions
  - Selecting specific columns
  - Sorting data
  - Aggregating values (sum, count, average, etc.)
  - Joining data from multiple sources

## Key Features

1. **CSV Parser**: Read and validate CSV files, handling edge cases like quoted fields and escaped characters
2. **Query Language**: Develop a simple query interface (similar to SQL) that allows users to specify what data they want
3. **Data Retrieval**: Return results in an organized format
4. **Performance**: Handle reasonably sized datasets efficiently
5. **Error Handling**: Gracefully manage malformed CSV files or invalid queries

## Use Cases

- Analyzing data from exported files without needing a full database server
- Building a lightweight tool for data analysis and reporting
- Creating a foundation for understanding how real databases work
