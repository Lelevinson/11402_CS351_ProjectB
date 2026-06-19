#include "csv.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace csvdb {

bool Table::hasColumn(const std::string& name) const {
	for (const auto& header : headers) {
		if (header == name) {
			return true;
		}
	}
	return false;
}

std::size_t Table::columnIndex(const std::string& name) const {
	for (std::size_t i = 0; i < headers.size(); ++i) {
		if (headers[i] == name) {
			return i;
		}
	}
	throw std::out_of_range("CSV column not found: " + name);
}

namespace {

// Split CSV text into records of fields using a small state machine so that
// commas and newlines inside double-quoted fields are preserved. A doubled
// quote ("") inside a quoted field is treated as a single literal quote.
std::vector<std::vector<std::string>> tokenize(const std::string& text) {
	std::vector<std::vector<std::string>> records;
	std::vector<std::string> field;
	std::string current;
	bool inQuotes = false;

	auto endField = [&]() {
		field.push_back(current);
		current.clear();
	};
	auto endRecord = [&]() {
		endField();
		records.push_back(field);
		field.clear();
	};

	for (std::size_t i = 0; i < text.size(); ++i) {
		const char c = text[i];

		if (inQuotes) {
			if (c == '"') {
				if (i + 1 < text.size() && text[i + 1] == '"') {
					current.push_back('"');  // escaped quote
					++i;
				} else {
					inQuotes = false;  // closing quote
				}
			} else {
				current.push_back(c);
			}
			continue;
		}

		switch (c) {
			case '"':
				inQuotes = true;
				break;
			case ',':
				endField();
				break;
			case '\r':
				break;  // tolerate CRLF; the '\n' ends the record
			case '\n':
				endRecord();
				break;
			default:
				current.push_back(c);
				break;
		}
	}

	if (inQuotes) {
		throw std::runtime_error("CSV parse error: unterminated quoted field");
	}

	// Flush a trailing record that is not newline-terminated, but ignore a
	// single empty trailing line (a file that simply ends with a newline).
	if (!current.empty() || !field.empty()) {
		endRecord();
	}

	return records;
}

}  // namespace

Table parseCsv(const std::string& text) {
	const auto records = tokenize(text);
	if (records.empty()) {
		throw std::runtime_error("CSV parse error: no header row");
	}

	Table table;
	table.headers = records.front();

	for (std::size_t r = 1; r < records.size(); ++r) {
		const auto& record = records[r];
		if (record.size() != table.headers.size()) {
			throw std::runtime_error(
				"CSV parse error: row " + std::to_string(r) + " has " +
				std::to_string(record.size()) + " columns, expected " +
				std::to_string(table.headers.size()));
		}
		table.rows.push_back(record);
	}

	return table;
}

Table loadCsvFile(const std::string& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		throw std::runtime_error("CSV load error: cannot open file: " + path);
	}
	std::ostringstream buffer;
	buffer << in.rdbuf();
	return parseCsv(buffer.str());
}

namespace {

// Quote a field only if it contains a comma, double quote, or newline;
// internal double quotes are escaped by doubling them.
std::string escapeField(const std::string& field) {
	if (field.find_first_of(",\"\n\r") == std::string::npos) {
		return field;
	}
	std::string out = "\"";
	for (const char c : field) {
		if (c == '"') {
			out += "\"\"";
		} else {
			out += c;
		}
	}
	out += '"';
	return out;
}

}  // namespace

std::string writeCsv(const Table& table) {
	std::string out;
	const auto appendRow = [&out](const std::vector<std::string>& fields) {
		for (std::size_t i = 0; i < fields.size(); ++i) {
			if (i != 0) {
				out += ',';
			}
			out += escapeField(fields[i]);
		}
		out += '\n';
	};

	appendRow(table.headers);
	for (const auto& row : table.rows) {
		appendRow(row);
	}
	return out;
}

void saveCsvFile(const std::string& path, const Table& table) {
	std::ofstream out(path, std::ios::binary);
	if (!out) {
		throw std::runtime_error("CSV save error: cannot open file for writing: " + path);
	}
	out << writeCsv(table);
}

}  // namespace csvdb
