// CLI demo for the CSV mini database (RP11402-14).
// Usage: csv_query <file.csv> "<SELECT query>"
//   e.g. csv_query data/students.csv "SELECT name, major FROM students"

#include <iostream>
#include <string>

#include "csv.h"
#include "select.h"

namespace {

// Print a table as CSV (header line + one line per row).
void printTable(const csvdb::Table& table) {
	for (std::size_t i = 0; i < table.headers.size(); ++i) {
		if (i != 0) {
			std::cout << ',';
		}
		std::cout << table.headers[i];
	}
	std::cout << '\n';

	for (const auto& row : table.rows) {
		for (std::size_t i = 0; i < row.size(); ++i) {
			if (i != 0) {
				std::cout << ',';
			}
			std::cout << row[i];
		}
		std::cout << '\n';
	}
}

}  // namespace

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <file.csv> \"<SELECT query>\"\n";
		return 2;
	}

	try {
		const csvdb::Table table = csvdb::loadCsvFile(argv[1]);
		const csvdb::Table result = csvdb::runSelect(argv[2], table);
		printTable(result);
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}
