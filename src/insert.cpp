#include "insert.h"

#include <stdexcept>

#include "strutil.h"

namespace csvdb {

namespace {

// Split a VALUES list on commas that are not inside quotes, then trim each item
// and strip a matching pair of surrounding quotes. Empty values are allowed
// (an empty CSV field). A whitespace-only list yields no values.
std::vector<std::string> splitValues(const std::string& inside) {
	std::vector<std::string> values;
	if (trim(inside).empty()) {
		return values;
	}

	std::string token;
	bool inQuotes = false;
	char quoteChar = 0;
	for (const char c : inside) {
		if (inQuotes) {
			token += c;
			if (c == quoteChar) {
				inQuotes = false;
			}
		} else if (c == '"' || c == '\'') {
			inQuotes = true;
			quoteChar = c;
			token += c;
		} else if (c == ',') {
			values.push_back(token);
			token.clear();
		} else {
			token += c;
		}
	}
	values.push_back(token);

	for (auto& raw : values) {
		std::string value = trim(raw);
		if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') &&
		    value.back() == value.front()) {
			value = value.substr(1, value.size() - 2);
		}
		raw = value;
	}
	return values;
}

}  // namespace

InsertCommand parseInsert(const std::string& query) {
	std::string q = trim(query);
	if (!q.empty() && q.back() == ';') {
		q.pop_back();
		q = trim(q);
	}

	const std::string upper = toUpper(q);
	const std::string kInsert = "INSERT";
	const std::string kInto = "INTO";
	const std::string kValues = "VALUES";

	if (upper.compare(0, kInsert.size(), kInsert) != 0 || !isSpaceAt(q, kInsert.size())) {
		throw std::runtime_error("INSERT parse error: must start with INSERT");
	}

	std::size_t pos = kInsert.size();
	while (isSpaceAt(q, pos)) {
		++pos;
	}
	if (upper.compare(pos, kInto.size(), kInto) != 0 || !isSpaceAt(q, pos + kInto.size())) {
		throw std::runtime_error("INSERT parse error: expected INTO after INSERT");
	}
	pos += kInto.size();

	// Find the VALUES keyword (standalone; may be followed directly by '(').
	std::size_t valuesPos = std::string::npos;
	for (std::size_t i = pos; i + kValues.size() <= upper.size(); ++i) {
		if (upper.compare(i, kValues.size(), kValues) == 0 && isSpaceAt(q, i - 1) &&
		    (i + kValues.size() == q.size() || isSpaceAt(q, i + kValues.size()) ||
		     q[i + kValues.size()] == '(')) {
			valuesPos = i;
			break;
		}
	}
	if (valuesPos == std::string::npos) {
		throw std::runtime_error("INSERT parse error: expected VALUES");
	}

	const std::string table = trim(q.substr(pos, valuesPos - pos));
	if (table.empty()) {
		throw std::runtime_error("INSERT parse error: no table specified");
	}
	if (table.find_first_of(" \t") != std::string::npos) {
		throw std::runtime_error("INSERT parse error: invalid table name: " + table);
	}

	const std::string rest = q.substr(valuesPos + kValues.size());
	const std::size_t lparen = rest.find('(');
	const std::size_t rparen = rest.rfind(')');
	if (lparen == std::string::npos || rparen == std::string::npos || rparen < lparen) {
		throw std::runtime_error("INSERT parse error: VALUES must be followed by (...)");
	}

	InsertCommand command;
	command.table = table;
	command.values = splitValues(rest.substr(lparen + 1, rparen - lparen - 1));
	return command;
}

void executeInsert(const InsertCommand& command, Table& table) {
	if (command.values.size() != table.headers.size()) {
		throw std::runtime_error(
			"INSERT error: expected " + std::to_string(table.headers.size()) +
			" values, got " + std::to_string(command.values.size()));
	}
	table.rows.push_back(command.values);
}

}  // namespace csvdb
