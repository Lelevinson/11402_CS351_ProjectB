#include "select.h"

#include <stdexcept>
#include <utility>

#include "strutil.h"
#include "where.h"

namespace csvdb {

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

	// The part after FROM is "table [WHERE ...]".
	std::string tablePart = trim(afterFrom);
	std::string beforeWhere;
	std::string afterWhere;
	const bool hasWhere = splitWhere(afterFrom, beforeWhere, afterWhere);
	if (hasWhere) {
		tablePart = trim(beforeWhere);
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
	if (hasWhere) {
		result.where = parseWhereExpr(afterWhere);
	}

	if (colsPart == "*") {
		result.columns = {"*"};
		return result;
	}

	// Split the column list on commas; an empty token (including a trailing
	// comma, e.g. "name,") is reported as an error rather than silently dropped.
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
	// Rows to keep (WHERE filter applied; validates the WHERE column exists).
	const std::vector<std::size_t> rowIndices = matchingRows(table, query.where);

	Table result;

	// SELECT * keeps every column (still honouring the WHERE filter).
	if (query.columns.size() == 1 && query.columns[0] == "*") {
		result.headers = table.headers;
		result.rows.reserve(rowIndices.size());
		for (const std::size_t i : rowIndices) {
			result.rows.push_back(table.rows[i]);
		}
		return result;
	}

	// Otherwise project the requested columns, in order.
	std::vector<std::size_t> columnIndices;
	columnIndices.reserve(query.columns.size());
	for (const auto& column : query.columns) {
		columnIndices.push_back(table.columnIndex(column));  // throws if missing
	}

	result.headers = query.columns;
	result.rows.reserve(rowIndices.size());
	for (const std::size_t i : rowIndices) {
		std::vector<std::string> projected;
		projected.reserve(columnIndices.size());
		for (const std::size_t columnIndex : columnIndices) {
			projected.push_back(table.rows[i][columnIndex]);
		}
		result.rows.push_back(std::move(projected));
	}
	return result;
}

Table runSelect(const std::string& query, const Table& table) {
	return executeSelect(parseSelect(query), table);
}

}  // namespace csvdb
