#pragma once

#include <string>
#include <vector>

#include "csv.h"

namespace csvdb {

// A parsed "INSERT INTO table VALUES (...)" statement.
struct InsertCommand {
	std::string table;
	std::vector<std::string> values;
};

// Parse: INSERT INTO table VALUES (v1, v2, ...)
// Keywords are case-insensitive; a trailing ';' is tolerated. Values may be
// quoted (single or double) to contain commas; surrounding quotes are stripped.
// Throws std::runtime_error on syntax errors.
InsertCommand parseInsert(const std::string& query);

// Append the command's values as a new row. Throws std::runtime_error if the
// number of values does not match the table's column count.
void executeInsert(const InsertCommand& command, Table& table);

}  // namespace csvdb
