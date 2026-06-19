#include "where.h"

#include <stdexcept>

#include "strutil.h"

namespace csvdb {

bool splitWhere(const std::string& text, std::string& before, std::string& after) {
	const std::string upper = toUpper(text);
	const std::string kWhere = "WHERE";
	for (std::size_t i = 0; i + kWhere.size() <= upper.size(); ++i) {
		if (upper.compare(i, kWhere.size(), kWhere) == 0 &&
		    (i == 0 || isSpaceAt(text, i - 1)) &&
		    (i + kWhere.size() == text.size() || isSpaceAt(text, i + kWhere.size()))) {
			before = text.substr(0, i);
			after = text.substr(i + kWhere.size());
			return true;
		}
	}
	return false;
}

WhereClause parseWhereExpr(const std::string& expr) {
	const std::string e = trim(expr);
	if (e.empty()) {
		throw std::runtime_error("WHERE parse error: empty WHERE clause");
	}
	const std::size_t eq = e.find('=');
	if (eq == std::string::npos) {
		throw std::runtime_error("WHERE parse error: WHERE requires 'column = value'");
	}
	const std::string column = trim(e.substr(0, eq));
	std::string value = trim(e.substr(eq + 1));
	if (column.empty()) {
		throw std::runtime_error("WHERE parse error: WHERE is missing a column name");
	}
	if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') &&
	    value.back() == value.front()) {
		value = value.substr(1, value.size() - 2);
	}

	WhereClause clause;
	clause.present = true;
	clause.column = column;
	clause.value = value;
	return clause;
}

std::vector<std::size_t> matchingRows(const Table& table, const WhereClause& where) {
	std::vector<std::size_t> result;
	if (!where.present) {
		result.reserve(table.rows.size());
		for (std::size_t i = 0; i < table.rows.size(); ++i) {
			result.push_back(i);
		}
		return result;
	}

	const std::size_t column = table.columnIndex(where.column);  // throws if missing
	for (std::size_t i = 0; i < table.rows.size(); ++i) {
		if (table.rows[i][column] == where.value) {
			result.push_back(i);
		}
	}
	return result;
}

}  // namespace csvdb
