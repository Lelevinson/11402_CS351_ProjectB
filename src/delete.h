#pragma once

#include <cstddef>
#include <string>

#include "csv.h"
#include "where.h"

namespace csvdb {

// A parsed "DELETE FROM table [WHERE ...]" statement.
struct DeleteCommand {
	std::string table;
	WhereClause where;  // optional; if absent, all rows are deleted
};

// Parse: DELETE FROM table [WHERE column = value]
// Keywords are case-insensitive; a trailing ';' is tolerated.
// Throws std::runtime_error on syntax errors.
DeleteCommand parseDelete(const std::string& query);

// Remove every row matching the WHERE clause (or all rows if there is no
// WHERE). Returns the number of rows deleted. Throws std::out_of_range if the
// WHERE column does not exist.
std::size_t executeDelete(const DeleteCommand& command, Table& table);

}  // namespace csvdb
