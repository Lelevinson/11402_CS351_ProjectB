// Unit tests for the UPDATE command (RP11402-22).

#include <iostream>
#include <stdexcept>
#include <string>

#include "../src/csv.h"
#include "../src/update.h"

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
	auto c = csvdb::parseUpdate("UPDATE students SET major = Math WHERE name = Bob");
	check(c.table == "students", "parse: table");
	check(c.setColumn == "major" && c.setValue == "Math", "parse: SET column/value");
	check(c.where.present && c.where.column == "name" && c.where.value == "Bob", "parse: WHERE");
}

void test_parse_no_where_and_quoted() {
	auto c = csvdb::parseUpdate("update students set major = \"Data Science\" ;");
	check(!c.where.present, "parse: no WHERE -> not present");
	check(c.setValue == "Data Science", "parse: quoted SET value, case-insensitive");
}

void test_parse_errors() {
	expectThrows([] { csvdb::parseUpdate("UPDATE students major = X"); }, "parse error: missing SET");
	expectThrows([] { csvdb::parseUpdate("UPDATE students SET major"); }, "parse error: SET without '='");
}

void test_execute_with_where() {
	auto table = sampleTable();
	const std::size_t n = csvdb::executeUpdate(
		csvdb::parseUpdate("UPDATE students SET major = Math WHERE name = Bob"), table);
	check(n == 1, "exec: one row updated");
	check(table.rows[1][2] == "Math", "exec: Bob's major changed");
	check(table.rows[0][2] == "CS", "exec: other rows unchanged");
}

void test_execute_no_where_updates_all() {
	auto table = sampleTable();
	const std::size_t n = csvdb::executeUpdate(
		csvdb::parseUpdate("UPDATE students SET major = X"), table);
	check(n == 3, "exec: all rows updated when no WHERE");
	check(table.rows[0][2] == "X" && table.rows[2][2] == "X", "exec: every major set");
}

void test_execute_unknown_columns_throw() {
	auto table = sampleTable();
	expectThrows([&] { csvdb::executeUpdate(csvdb::parseUpdate("UPDATE students SET gpa = 4"), table); },
	             "exec: unknown SET column throws");
	expectThrows([&] { csvdb::executeUpdate(csvdb::parseUpdate("UPDATE students SET major = X WHERE gpa = 4"), table); },
	             "exec: unknown WHERE column throws");
}

}  // namespace

int main() {
	std::cout << "Running UPDATE tests...\n";
	test_parse_with_where();
	test_parse_no_where_and_quoted();
	test_parse_errors();
	test_execute_with_where();
	test_execute_no_where_updates_all();
	test_execute_unknown_columns_throw();

	std::cout << "\nChecks run: " << g_checks << " | Passed: " << (g_checks - g_failures)
	          << " | Failed: " << g_failures << '\n';
	if (g_failures == 0) {
		std::cout << "All tests passed.\n";
		return 0;
	}
	std::cout << "TESTS FAILED.\n";
	return 1;
}
