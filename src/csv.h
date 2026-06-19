#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace csvdb {

// An in-memory representation of a loaded CSV file.
// The first CSV line is treated as the column headers; every later line
// becomes one row whose values are positionally aligned to those headers.
struct Table {
	std::vector<std::string> headers;
	std::vector<std::vector<std::string>> rows;

	// Returns true if a column with the given name exists.
	bool hasColumn(const std::string& name) const;

	// Returns the index of the named column.
	// Throws std::out_of_range if the column does not exist.
	std::size_t columnIndex(const std::string& name) const;
};

// Parse CSV text into a Table.
//   - The first line is the header row.
//   - Fields may be quoted with double quotes; a quoted field can contain
//     commas and newlines, and a literal double quote is written as "".
//   - Throws std::runtime_error on malformed input (e.g. a row whose column
//     count does not match the header, or an unterminated quoted field).
Table parseCsv(const std::string& text);

// Load and parse a CSV file from disk.
// Throws std::runtime_error if the file cannot be opened.
Table loadCsvFile(const std::string& path);

// Serialize a Table back to CSV text (header row + one line per row). Fields
// containing a comma, double quote, or newline are quoted, with internal double
// quotes escaped as "". parseCsv(writeCsv(t)) reproduces t.
std::string writeCsv(const Table& table);

// Serialize a Table and write it to a file.
// Throws std::runtime_error if the file cannot be opened for writing.
void saveCsvFile(const std::string& path, const Table& table);

}  // namespace csvdb
