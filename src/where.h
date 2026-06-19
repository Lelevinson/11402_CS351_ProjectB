#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "csv.h"

namespace csvdb {

// An optional "WHERE column = value" equality filter, shared by SELECT, UPDATE
// and DELETE.
struct WhereClause {
	bool present = false;
	std::string column;
	std::string value;
};

// If `text` contains a standalone, whitespace-delimited WHERE keyword
// (case-insensitive), set `before`/`after` to the surrounding text and return
// true; otherwise return false and leave them untouched.
bool splitWhere(const std::string& text, std::string& before, std::string& after);

// Parse the expression that follows WHERE, e.g. "major = CS". Returns a clause
// with present == true. Surrounding quotes are stripped from the value.
// Throws std::runtime_error on syntax errors (empty clause, no '=', empty column).
WhereClause parseWhereExpr(const std::string& expr);

// Indices of the rows in `table` that satisfy `where`. If `where` is not
// present, every row index is returned. Throws std::out_of_range if the WHERE
// column does not exist (even when the table has no rows).
std::vector<std::size_t> matchingRows(const Table& table, const WhereClause& where);

}  // namespace csvdb
