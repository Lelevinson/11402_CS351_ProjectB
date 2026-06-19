// Unit tests for the WHERE clause (RP11402-19).
// Same self-contained harness style as the other test files; exits non-zero on
// any failure so GitHub Actions blocks merges on a broken WHERE.

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
	return csvdb::parseCsv("name,age,major\nAlice,20,CS\nBob,21,EE\nCarol,22,CS\n");
}

// ---- Parsing -------------------------------------------------------------

void test_parse_where() {
	auto q = csvdb::parseSelect("SELECT name FROM students WHERE major = CS");
	check(q.where.present, "parse: WHERE present");
	check(q.where.column == "major", "parse: WHERE column");
	check(q.where.value == "CS", "parse: WHERE value (unquoted)");
	check(q.columns.size() == 1 && q.columns[0] == "name", "parse: columns unaffected by WHERE");
}

void test_parse_where_quoted_and_case() {
	auto q = csvdb::parseSelect("select * from students where major = \"CS\" ;");
	check(q.where.present && q.where.value == "CS", "parse: quotes stripped, case-insensitive WHERE");
	auto q2 = csvdb::parseSelect("SELECT name FROM students WHERE major = 'EE'");
	check(q2.where.value == "EE", "parse: single quotes stripped");
}

void test_parse_no_where() {
	auto q = csvdb::parseSelect("SELECT name FROM students");
	check(!q.where.present, "parse: no WHERE -> not present");
}

void test_parse_where_errors() {
	expectThrows([] { csvdb::parseSelect("SELECT name FROM students WHERE major"); },
	             "parse error: WHERE without '='");
	expectThrows([] { csvdb::parseSelect("SELECT name FROM students WHERE = CS"); },
	             "parse error: WHERE without a column");
	expectThrows([] { csvdb::parseSelect("SELECT name FROM students WHERE"); },
	             "parse error: empty WHERE clause");
}

// ---- Execution -----------------------------------------------------------

void test_filter_basic() {
	const auto table = sampleTable();
	auto r = csvdb::runSelect("SELECT name FROM students WHERE major = CS", table);
	check(r.rows.size() == 2, "exec: two CS rows kept");
	check(r.rows[0][0] == "Alice" && r.rows[1][0] == "Carol", "exec: correct rows kept");
}

void test_filter_no_match() {
	const auto table = sampleTable();
	auto r = csvdb::runSelect("SELECT name FROM students WHERE major = Bio", table);
	check(r.rows.empty(), "exec: no matching rows -> empty result");
	check(r.headers.size() == 1, "exec: headers still projected when no rows");
}

void test_filter_column_not_selected() {
	const auto table = sampleTable();
	// Filter on 'major' but only select 'name' — WHERE column need not be selected.
	auto r = csvdb::runSelect("SELECT name FROM students WHERE major = EE", table);
	check(r.rows.size() == 1 && r.rows[0][0] == "Bob", "exec: filter on a non-selected column");
}

void test_filter_with_star() {
	const auto table = sampleTable();
	auto r = csvdb::runSelect("SELECT * FROM students WHERE major = CS", table);
	check(r.headers.size() == 3, "exec: '*' keeps all columns");
	check(r.rows.size() == 2 && r.rows[1][0] == "Carol", "exec: '*' honours WHERE filter");
}

void test_filter_unknown_column_throws() {
	const auto table = sampleTable();
	expectThrows([&] { csvdb::runSelect("SELECT name FROM students WHERE gpa = 4", table); },
	             "exec: unknown WHERE column throws");
}

}  // namespace

int main() {
	std::cout << "Running WHERE clause tests...\n";
	test_parse_where();
	test_parse_where_quoted_and_case();
	test_parse_no_where();
	test_parse_where_errors();
	test_filter_basic();
	test_filter_no_match();
	test_filter_column_not_selected();
	test_filter_with_star();
	test_filter_unknown_column_throws();

	std::cout << "\nChecks run: " << g_checks << " | Passed: " << (g_checks - g_failures)
	          << " | Failed: " << g_failures << '\n';
	if (g_failures == 0) {
		std::cout << "All tests passed.\n";
		return 0;
	}
	std::cout << "TESTS FAILED.\n";
	return 1;
}
