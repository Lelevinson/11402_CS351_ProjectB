#include "select.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace csvdb {

namespace {

std::string trim(const std::string& s) {
	std::size_t begin = 0;
	std::size_t end = s.size();
	while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) {
		++begin;
	}
	while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
		--end;
	}
	return s.substr(begin, end - begin);
}

std::string toUpper(const std::string& s) {
	std::string out = s;
	std::transform(out.begin(), out.end(), out.begin(),
	               [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return out;
}

bool isSpaceAt(const std::string& s, std::size_t i) {
	return i < s.size() && std::isspace(static_cast<unsigned char>(s[i]));
}

}  // namespace

SelectQuery parseSelect(const std::string& query) {
	std::string q = trim(query);
	if (!q.empty() && q.back() == ';') {
		q.pop_back();
		q = trim(q);
	}

	const std::string upper = toUpper(q);
	const std::string kSelect = "SELECT";
	const std::string kFrom = "FROM";

	// Must begin with SELECT followed by whitespace.
	if (upper.compare(0, kSelect.size(), kSelect) != 0 || !isSpaceAt(q, kSelect.size())) {
		throw std::runtime_error("SELECT parse error: query must start with SELECT");
	}

	// Find FROM as a standalone, whitespace-delimited keyword.
	std::size_t fromPos = std::string::npos;
	for (std::size_t i = kSelect.size(); i + kFrom.size() <= upper.size(); ++i) {
		if (upper.compare(i, kFrom.size(), kFrom) == 0 &&
		    isSpaceAt(q, i - 1) &&
		    (i + kFrom.size() == q.size() || isSpaceAt(q, i + kFrom.size()))) {
			fromPos = i;
			break;
		}
	}
	if (fromPos == std::string::npos) {
		throw std::runtime_error("SELECT parse error: missing FROM clause");
	}

	const std::string colsPart = trim(q.substr(kSelect.size(), fromPos - kSelect.size()));
	const std::string afterFrom = q.substr(fromPos + kFrom.size());

	// Split the part after FROM into the table name and an optional WHERE clause.
	const std::string upperAfter = toUpper(afterFrom);
	const std::string kWhere = "WHERE";
	std::size_t wherePos = std::string::npos;
	for (std::size_t i = 0; i + kWhere.size() <= upperAfter.size(); ++i) {
		if (upperAfter.compare(i, kWhere.size(), kWhere) == 0 &&
		    (i == 0 || isSpaceAt(afterFrom, i - 1)) &&
		    (i + kWhere.size() == afterFrom.size() || isSpaceAt(afterFrom, i + kWhere.size()))) {
			wherePos = i;
			break;
		}
	}

	std::string tablePart;
	std::string wherePart;
	if (wherePos != std::string::npos) {
		tablePart = trim(afterFrom.substr(0, wherePos));
		wherePart = trim(afterFrom.substr(wherePos + kWhere.size()));
	} else {
		tablePart = trim(afterFrom);
	}

	if (colsPart.empty()) {
		throw std::runtime_error("SELECT parse error: no columns specified");
	}
	if (tablePart.empty()) {
		throw std::runtime_error("SELECT parse error: no table specified");
	}
	if (tablePart.find_first_of(" \t") != std::string::npos) {
		throw std::runtime_error("SELECT parse error: invalid table name: " + tablePart);
	}

	SelectQuery result;
	result.table = tablePart;

	// Parse the optional WHERE clause: column = value.
	if (wherePos != std::string::npos) {
		if (wherePart.empty()) {
			throw std::runtime_error("SELECT parse error: empty WHERE clause");
		}
		const std::size_t eq = wherePart.find('=');
		if (eq == std::string::npos) {
			throw std::runtime_error("SELECT parse error: WHERE requires 'column = value'");
		}
		const std::string column = trim(wherePart.substr(0, eq));
		std::string value = trim(wherePart.substr(eq + 1));
		if (column.empty()) {
			throw std::runtime_error("SELECT parse error: WHERE is missing a column name");
		}
		// Strip a matching pair of surrounding quotes from the value.
		if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') &&
		    value.back() == value.front()) {
			value = value.substr(1, value.size() - 2);
		}
		result.where.present = true;
		result.where.column = column;
		result.where.value = value;
	}

	if (colsPart == "*") {
		result.columns = {"*"};
		return result;
	}

	// Split on commas manually so an empty token (including a trailing comma,
	// e.g. "name,") is reported as an error rather than silently dropped.
	std::size_t start = 0;
	while (true) {
		const std::size_t comma = colsPart.find(',', start);
		const std::string token =
			(comma == std::string::npos) ? colsPart.substr(start)
			                             : colsPart.substr(start, comma - start);
		const std::string name = trim(token);
		if (name.empty()) {
			throw std::runtime_error("SELECT parse error: empty column name in list");
		}
		result.columns.push_back(name);
		if (comma == std::string::npos) {
			break;
		}
		start = comma + 1;
	}
	return result;
}

Table executeSelect(const SelectQuery& query, const Table& table) {
	// Apply the optional WHERE filter first, collecting the rows to keep.
	std::vector<const std::vector<std::string>*> kept;
	if (query.where.present) {
		const std::size_t whereIndex =
			table.columnIndex(query.where.column);  // throws std::out_of_range if missing
		for (const auto& row : table.rows) {
			if (row[whereIndex] == query.where.value) {
				kept.push_back(&row);
			}
		}
	} else {
		for (const auto& row : table.rows) {
			kept.push_back(&row);
		}
	}

	Table result;

	// SELECT * keeps every column (still honouring the WHERE filter above).
	if (query.columns.size() == 1 && query.columns[0] == "*") {
		result.headers = table.headers;
		result.rows.reserve(kept.size());
		for (const auto* row : kept) {
			result.rows.push_back(*row);
		}
		return result;
	}

	// Otherwise project the requested columns, in order.
	std::vector<std::size_t> indices;
	indices.reserve(query.columns.size());
	for (const auto& column : query.columns) {
		indices.push_back(table.columnIndex(column));  // throws std::out_of_range if missing
	}

	result.headers = query.columns;
	result.rows.reserve(kept.size());
	for (const auto* row : kept) {
		std::vector<std::string> projected;
		projected.reserve(indices.size());
		for (const std::size_t index : indices) {
			projected.push_back((*row)[index]);
		}
		result.rows.push_back(std::move(projected));
	}
	return result;
}

Table runSelect(const std::string& query, const Table& table) {
	return executeSelect(parseSelect(query), table);
}

}  // namespace csvdb
