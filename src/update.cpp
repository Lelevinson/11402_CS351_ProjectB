#include "update.h"

#include <stdexcept>

#include "strutil.h"

namespace csvdb {

UpdateCommand parseUpdate(const std::string& query) {
	std::string q = trim(query);
	if (!q.empty() && q.back() == ';') {
		q.pop_back();
		q = trim(q);
	}

	const std::string upper = toUpper(q);
	const std::string kUpdate = "UPDATE";
	const std::string kSet = "SET";

	if (upper.compare(0, kUpdate.size(), kUpdate) != 0 || !isSpaceAt(q, kUpdate.size())) {
		throw std::runtime_error("UPDATE parse error: must start with UPDATE");
	}

	// Find the SET keyword (standalone) after UPDATE.
	std::size_t setPos = std::string::npos;
	for (std::size_t i = kUpdate.size(); i + kSet.size() <= upper.size(); ++i) {
		if (upper.compare(i, kSet.size(), kSet) == 0 && isSpaceAt(q, i - 1) &&
		    (i + kSet.size() == q.size() || isSpaceAt(q, i + kSet.size()))) {
			setPos = i;
			break;
		}
	}
	if (setPos == std::string::npos) {
		throw std::runtime_error("UPDATE parse error: expected SET");
	}

	const std::string table = trim(q.substr(kUpdate.size(), setPos - kUpdate.size()));
	if (table.empty()) {
		throw std::runtime_error("UPDATE parse error: no table specified");
	}
	if (table.find_first_of(" \t") != std::string::npos) {
		throw std::runtime_error("UPDATE parse error: invalid table name: " + table);
	}

	const std::string afterSet = q.substr(setPos + kSet.size());

	// Separate the assignment from an optional WHERE clause.
	std::string assignPart = trim(afterSet);
	std::string beforeWhere;
	std::string afterWhere;
	const bool hasWhere = splitWhere(afterSet, beforeWhere, afterWhere);
	if (hasWhere) {
		assignPart = trim(beforeWhere);
	}

	// Parse "column = value".
	const std::size_t eq = assignPart.find('=');
	if (eq == std::string::npos) {
		throw std::runtime_error("UPDATE parse error: SET requires 'column = value'");
	}
	const std::string column = trim(assignPart.substr(0, eq));
	std::string value = trim(assignPart.substr(eq + 1));
	if (column.empty()) {
		throw std::runtime_error("UPDATE parse error: SET is missing a column name");
	}
	if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') &&
	    value.back() == value.front()) {
		value = value.substr(1, value.size() - 2);
	}

	UpdateCommand command;
	command.table = table;
	command.setColumn = column;
	command.setValue = value;
	if (hasWhere) {
		command.where = parseWhereExpr(afterWhere);
	}
	return command;
}

std::size_t executeUpdate(const UpdateCommand& command, Table& table) {
	const std::size_t setColumn = table.columnIndex(command.setColumn);  // throws if missing
	const std::vector<std::size_t> rows = matchingRows(table, command.where);
	for (const std::size_t i : rows) {
		table.rows[i][setColumn] = command.setValue;
	}
	return rows.size();
}

}  // namespace csvdb
