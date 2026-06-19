#pragma once

#include <string>
#include <vector>

#include "csv.h"

namespace csvdb {

// An optional "WHERE column = value" equality filter.
struct WhereClause {
	bool present = false;
	std::string column;
	std::string value;
};

// A parsed "SELECT ... FROM ... [WHERE ...]" statement.
struct SelectQuery {
	std::vector<std::string> columns;  // requested columns in order; {"*"} means all
	std::string table;                 // table name from the FROM clause
	WhereClause where;                 // optional equality filter
};

// Parse a SELECT statement of the form:  SELECT col1, col2 FROM table
// Case-insensitive keywords; a trailing ';' is tolerated; "*" selects all columns.
// Throws std::runtime_error on syntax errors (missing SELECT/FROM, empty column
// list, missing/invalid table name).
SelectQuery parseSelect(const std::string& query);

// Execute a parsed query against a table, returning a new Table containing only
// the selected columns, in the requested order.
// Throws std::out_of_range if a requested column does not exist.
Table executeSelect(const SelectQuery& query, const Table& table);

// Convenience: parseSelect followed by executeSelect.
Table runSelect(const std::string& query, const Table& table);

}  // namespace csvdb
