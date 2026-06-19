// Unit tests for the SELECT query engine (RP11402-15).
// Same self-contained harness style as tests/test_csv.cpp; exits non-zero on
// any failure so GitHub Actions blocks merges on a broken SELECT.

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/csv.h"
#include "../src/select.h"

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

// ---- Parsing tests --------------------------------------------------------

void test_parse_single_and_multiple() {
	auto q1 = csvdb::parseSelect("SELECT name FROM students");
	check(q1.columns.size() == 1 && q1.columns[0] == "name", "parse: single column");
	check(q1.table == "students", "parse: table name");

	auto q2 = csvdb::parseSelect("SELECT name, major FROM students");
	check(q2.columns.size() == 2 && q2.columns[1] == "major", "parse: multiple columns in order");
}

void test_parse_star() {
	auto q = csvdb::parseSelect("SELECT * FROM students");
	check(q.columns.size() == 1 && q.columns[0] == "*", "parse: star");
}

void test_parse_case_insensitive_and_semicolon() {
	auto q = csvdb::parseSelect("select name , major from students ;");
	check(q.columns.size() == 2, "parse: lowercase keywords + spaces + trailing ';'");
	check(q.columns[0] == "name" && q.columns[1] == "major", "parse: column names trimmed");
	check(q.table == "students", "parse: table after lowercase from");
}

void test_parse_errors() {
	expectThrows([] { csvdb::parseSelect("name, major FROM students"); }, "parse error: missing SELECT");
	expectThrows([] { csvdb::parseSelect("SELECT name, major students"); }, "parse error: missing FROM");
	expectThrows([] { csvdb::parseSelect("SELECT FROM students"); }, "parse error: empty column list");
	expectThrows([] { csvdb::parseSelect("SELECT name FROM"); }, "parse error: empty table");
	expectThrows([] { csvdb::parseSelect("SELECT name FROM a b"); }, "parse error: multi-token table");
	expectThrows([] { csvdb::parseSelect("SELECT name, FROM students"); }, "parse error: empty column in list");
}

// ---- Execution tests ------------------------------------------------------

void test_execute_projection() {
	const auto table = sampleTable();

	auto one = csvdb::runSelect("SELECT name FROM students", table);
	check(one.headers.size() == 1 && one.headers[0] == "name", "exec: single column header");
	check(one.rows.size() == 2 && one.rows[0][0] == "Alice", "exec: single column values");

	auto two = csvdb::runSelect("SELECT name, major FROM students", table);
	check(two.headers.size() == 2, "exec: two columns");
	check(two.rows[1][0] == "Bob" && two.rows[1][1] == "EE", "exec: two-column row values");
}

void test_execute_reordered() {
	const auto table = sampleTable();
	auto r = csvdb::runSelect("SELECT major, name FROM students", table);
	check(r.headers[0] == "major" && r.headers[1] == "name", "exec: reordered headers");
	check(r.rows[0][0] == "CS" && r.rows[0][1] == "Alice", "exec: reordered values follow header order");
}

void test_execute_star() {
	const auto table = sampleTable();
	auto r = csvdb::runSelect("SELECT * FROM students", table);
	check(r.headers.size() == 3, "exec: star keeps all columns");
	check(r.rows.size() == 2 && r.rows[0][2] == "CS", "exec: star keeps all values");
}

void test_execute_unknown_column_throws() {
	const auto table = sampleTable();
	expectThrows([&] { csvdb::runSelect("SELECT gpa FROM students", table); },
	             "exec: unknown column throws");
}

}  // namespace

int main() {
	std::cout << "Running SELECT engine tests...\n";
	test_parse_single_and_multiple();
	test_parse_star();
	test_parse_case_insensitive_and_semicolon();
	test_parse_errors();
	test_execute_projection();
	test_execute_reordered();
	test_execute_star();
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
