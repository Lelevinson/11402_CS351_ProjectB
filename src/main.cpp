// CLI for the CSV mini database.
// Usage: csv_query <file.csv> "<query>"
//   SELECT ... FROM table [WHERE col = val]      -> prints the result as CSV
//   INSERT INTO table VALUES (...)               -> appends a row, saves the file
//
// SELECT prints to stdout; mutating commands write the file back and report.

#include <cctype>
#include <iostream>
#include <string>

#include "csv.h"
#include "insert.h"
#include "select.h"
#include "strutil.h"

namespace {

// Print a table as CSV (header line + one line per row), with the same quoting
// rules as the on-disk format.
void printTable(const csvdb::Table& table) {
	std::cout << csvdb::writeCsv(table);
}

// The leading keyword of the query, upper-cased (e.g. "SELECT", "INSERT").
std::string firstKeyword(const std::string& query) {
	const std::string s = csvdb::trim(query);
	std::size_t i = 0;
	while (i < s.size() && !std::isspace(static_cast<unsigned char>(s[i]))) {
		++i;
	}
	return csvdb::toUpper(s.substr(0, i));
}

}  // namespace

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <file.csv> \"<query>\"\n";
		return 2;
	}

	const std::string path = argv[1];
	const std::string query = argv[2];

	try {
		const std::string keyword = firstKeyword(query);

		if (keyword == "SELECT") {
			const csvdb::Table table = csvdb::loadCsvFile(path);
			printTable(csvdb::runSelect(query, table));
		} else if (keyword == "INSERT") {
			csvdb::Table table = csvdb::loadCsvFile(path);
			csvdb::executeInsert(csvdb::parseInsert(query), table);
			csvdb::saveCsvFile(path, table);
			std::cout << "1 row inserted.\n";
		} else {
			std::cerr << "Error: unsupported command: " << keyword << '\n';
			return 1;
		}
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}
