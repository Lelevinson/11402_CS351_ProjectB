// Unit tests for the INSERT command (RP11402-21).

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/csv.h"
#include "../src/insert.h"

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
	return csvdb::parseCsv("name,age,major\nAlice,20,CS\nBob,21,EE\n");
}

void test_parse_basic() {
	auto c = csvdb::parseInsert("INSERT INTO students VALUES (Carol, 22, Math)");
	check(c.table == "students", "parse: table");
	check(c.values.size() == 3, "parse: three values");
	check(c.values[0] == "Carol" && c.values[2] == "Math", "parse: value contents");
}

void test_parse_quoted_value_with_comma() {
	auto c = csvdb::parseInsert("INSERT INTO t VALUES (\"Doe, John\", 30)");
	check(c.values.size() == 2, "parse: quoted comma stays one value");
	check(c.values[0] == "Doe, John", "parse: quotes stripped, comma preserved");
}

void test_parse_case_insensitive() {
	auto c = csvdb::parseInsert("insert into students values (Dave, 23, EE) ;");
	check(c.table == "students" && c.values.size() == 3, "parse: lowercase keywords + trailing ';'");
}

void test_parse_errors() {
	expectThrows([] { csvdb::parseInsert("INSERT students VALUES (a)"); }, "parse error: missing INTO");
	expectThrows([] { csvdb::parseInsert("INSERT INTO students (a, b)"); }, "parse error: missing VALUES");
	expectThrows([] { csvdb::parseInsert("INSERT INTO students VALUES a, b"); }, "parse error: missing parentheses");
}

void test_execute_append() {
	auto table = sampleTable();
	csvdb::executeInsert(csvdb::parseInsert("INSERT INTO students VALUES (Carol, 22, Math)"), table);
	check(table.rows.size() == 3, "exec: row count grew to 3");
	check(table.rows[2][0] == "Carol" && table.rows[2][2] == "Math", "exec: appended values correct");
}

void test_execute_count_mismatch() {
	auto table = sampleTable();
	expectThrows([&] { csvdb::executeInsert(csvdb::parseInsert("INSERT INTO students VALUES (X, 1)"), table); },
	             "exec: too few values throws");
	expectThrows([&] { csvdb::executeInsert(csvdb::parseInsert("INSERT INTO students VALUES (X, 1, CS, extra)"), table); },
	             "exec: too many values throws");
}

}  // namespace

int main() {
	std::cout << "Running INSERT tests...\n";
	test_parse_basic();
	test_parse_quoted_value_with_comma();
	test_parse_case_insensitive();
	test_parse_errors();
	test_execute_append();
	test_execute_count_mismatch();

	std::cout << "\nChecks run: " << g_checks << " | Passed: " << (g_checks - g_failures)
	          << " | Failed: " << g_failures << '\n';
	if (g_failures == 0) {
		std::cout << "All tests passed.\n";
		return 0;
	}
	std::cout << "TESTS FAILED.\n";
	return 1;
}
