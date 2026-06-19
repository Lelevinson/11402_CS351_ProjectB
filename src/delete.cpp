#include "delete.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include "strutil.h"

namespace csvdb {

DeleteCommand parseDelete(const std::string& query) {
	std::string q = trim(query);
	if (!q.empty() && q.back() == ';') {
		q.pop_back();
		q = trim(q);
	}

	const std::string upper = toUpper(q);
	const std::string kDelete = "DELETE";
	const std::string kFrom = "FROM";

	if (upper.compare(0, kDelete.size(), kDelete) != 0 || !isSpaceAt(q, kDelete.size())) {
		throw std::runtime_error("DELETE parse error: must start with DELETE");
	}

	std::size_t pos = kDelete.size();
	while (isSpaceAt(q, pos)) {
		++pos;
	}
	if (upper.compare(pos, kFrom.size(), kFrom) != 0 ||
	    !(pos + kFrom.size() == q.size() || isSpaceAt(q, pos + kFrom.size()))) {
		throw std::runtime_error("DELETE parse error: expected FROM after DELETE");
	}
	pos += kFrom.size();

	const std::string afterFrom = q.substr(pos);
	std::string tablePart = trim(afterFrom);
	std::string beforeWhere;
	std::string afterWhere;
	const bool hasWhere = splitWhere(afterFrom, beforeWhere, afterWhere);
	if (hasWhere) {
		tablePart = trim(beforeWhere);
	}

	if (tablePart.empty()) {
		throw std::runtime_error("DELETE parse error: no table specified");
	}
	if (tablePart.find_first_of(" \t") != std::string::npos) {
		throw std::runtime_error("DELETE parse error: invalid table name: " + tablePart);
	}

	DeleteCommand command;
	command.table = tablePart;
	if (hasWhere) {
		command.where = parseWhereExpr(afterWhere);
	}
	return command;
}

std::size_t executeDelete(const DeleteCommand& command, Table& table) {
	const std::vector<std::size_t> toRemove = matchingRows(table, command.where);
	if (toRemove.empty()) {
		return 0;
	}

	std::vector<bool> remove(table.rows.size(), false);
	for (const std::size_t i : toRemove) {
		remove[i] = true;
	}

	std::vector<std::vector<std::string>> kept;
	kept.reserve(table.rows.size() - toRemove.size());
	for (std::size_t i = 0; i < table.rows.size(); ++i) {
		if (!remove[i]) {
			kept.push_back(std::move(table.rows[i]));
		}
	}
	table.rows = std::move(kept);
	return toRemove.size();
}

}  // namespace csvdb
