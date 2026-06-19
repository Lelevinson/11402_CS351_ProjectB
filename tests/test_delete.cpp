// Unit tests for the DELETE command (RP11402-23).

#include <iostream>
#include <stdexcept>
#include <string>

#include "../src/csv.h"
#include "../src/delete.h"

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& label) {
	++g_checks;
	if (!condition) {
		++g_failures;
		std::cout << "  [FAIL] " << label << '\n';
	}
}

template <typename F>
void expectThrows(F fn, const std::string& label) {
	++g_checks;
	bool threw = false;
	try {
		fn();
	} catch (const std::exception&) {
		threw = true;
	}
	if (!threw) {
		++g_failures;
		std::cout << "  [FAIL] " << label << " (expected an exception)\n";
	}
}

csvdb::Table sampleTable() {
	return csvdb::parseCsv("name,age,major\nAlice,20,CS\nBob,21,EE\nCarol,22,CS\n");
}

void test_parse_with_where() {
	auto c = csvdb::parseDelete("DELETE FROM students WHERE major = EE");
	check(c.table == "students", "parse: table");
	check(c.where.present && c.where.column == "major" && c.where.value == "EE", "parse: WHERE");
}

void test_parse_no_where_and_case() {
	auto c = csvdb::parseDelete("delete from students ;");
	check(c.table == "students" && !c.where.present, "parse: lowercase, no WHERE");
}

void test_parse_errors() {
	expectThrows([] { csvdb::parseDelete("DELETE students WHERE major = EE"); }, "parse error: missing FROM");
	expectThrows([] { csvdb::parseDelete("DELETE FROM"); }, "parse error: no table");
}

void test_execute_with_where() {
	auto table = sampleTable();
	const std::size_t n = csvdb::executeDelete(csvdb::parseDelete("DELETE FROM students WHERE major = CS"), table);
	check(n == 2, "exec: two CS rows deleted");
	check(table.rows.size() == 1 && table.rows[0][0] == "Bob", "exec: only Bob remains");
}

void test_execute_no_match() {
	auto table = sampleTable();
	const std::size_t n = csvdb::executeDelete(csvdb::parseDelete("DELETE FROM students WHERE major = Bio"), table);
	check(n == 0, "exec: no match deletes nothing");
	check(table.rows.size() == 3, "exec: rows untouched on no match");
}

void test_execute_no_where_deletes_all() {
	auto table = sampleTable();
	const std::size_t n = csvdb::executeDelete(csvdb::parseDelete("DELETE FROM students"), table);
	check(n == 3, "exec: all rows deleted without WHERE");
	check(table.rows.empty(), "exec: table emptied");
	check(table.headers.size() == 3, "exec: headers preserved");
}

void test_execute_unknown_column_throws() {
	auto table = sampleTable();
	expectThrows([&] { csvdb::executeDelete(csvdb::parseDelete("DELETE FROM students WHERE gpa = 4"), table); },
	             "exec: unknown WHERE column throws");
}

}  // namespace

int main() {
	std::cout << "Running DELETE tests...\n";
	test_parse_with_where();
	test_parse_no_where_and_case();
	test_parse_errors();
	test_execute_with_where();
	test_execute_no_match();
	test_execute_no_where_deletes_all();
	test_execute_unknown_column_throws();

	std::cout << "\nChecks run: " << g_checks << " | Passed: " << (g_checks - g_failures)
	          << " | Failed: " << g_failures << '\n';
	if (g_failures == 0) {
		std::cout << "All tests passed.\n";
		return 0;
	}
	std::cout << "TESTS FAILED.\n";
	return 1;
}
