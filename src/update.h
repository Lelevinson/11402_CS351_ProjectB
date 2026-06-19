#pragma once

#include <cstddef>
#include <string>

#include "csv.h"
#include "where.h"

namespace csvdb {

// A parsed "UPDATE table SET column = value [WHERE ...]" statement.
struct UpdateCommand {
	std::string table;
	std::string setColumn;
	std::string setValue;
	WhereClause where;  // optional; if absent, all rows are updated
};

// Parse: UPDATE table SET column = value [WHERE column = value]
// Keywords are case-insensitive; a trailing ';' is tolerated; the SET value may
// be quoted. Throws std::runtime_error on syntax errors.
UpdateCommand parseUpdate(const std::string& query);

// Set the SET column to the SET value on every row matching the WHERE clause
// (or all rows if there is no WHERE). Returns the number of rows updated.
// Throws std::out_of_range if the SET or WHERE column does not exist.
std::size_t executeUpdate(const UpdateCommand& command, Table& table);

}  // namespace csvdb
