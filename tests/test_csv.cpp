// Unit tests for the CSV parser/loader (RP11402-16).
// Self-contained: a tiny assertion harness plus a main() that runs every test
// and exits non-zero if any assertion fails, so CI can gate merges on it.

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/csv.h"

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

// ---- Tests ----------------------------------------------------------------

void test_basic_parse() {
	const auto t = csvdb::parseCsv("name,age,major\nAlice,20,CS\nBob,21,EE\n");
	check(t.headers.size() == 3, "basic: 3 headers");
	check(t.headers[0] == "name" && t.headers[2] == "major", "basic: header values");
	check(t.rows.size() == 2, "basic: 2 rows");
	check(t.rows[0][0] == "Alice" && t.rows[0][1] == "20", "basic: row 0 values");
	check(t.rows[1][2] == "EE", "basic: row 1 last value");
}

void test_quoted_field_with_comma() {
	const auto t = csvdb::parseCsv("name,note\nAlice,\"hello, world\"\n");
	check(t.rows.size() == 1, "quoted: one row");
	check(t.rows[0][1] == "hello, world", "quoted: comma kept inside quotes");
}

void test_escaped_quotes() {
	const auto t = csvdb::parseCsv("name,quote\nBob,\"she said \"\"hi\"\"\"\n");
	check(t.rows[0][1] == "she said \"hi\"", "escaped: doubled quotes -> one quote");
}

void test_no_trailing_newline() {
	const auto t = csvdb::parseCsv("a,b\n1,2");
	check(t.rows.size() == 1, "no-trailing-newline: row flushed");
	check(t.rows[0][1] == "2", "no-trailing-newline: value");
}

void test_header_only() {
	const auto t = csvdb::parseCsv("id,name\n");
	check(t.headers.size() == 2, "header-only: headers present");
	check(t.rows.empty(), "header-only: no rows");
}

void test_column_helpers() {
	const auto t = csvdb::parseCsv("name,age,major\nAlice,20,CS\n");
	check(t.hasColumn("age"), "helpers: hasColumn true");
	check(!t.hasColumn("gpa"), "helpers: hasColumn false");
	check(t.columnIndex("major") == 2, "helpers: columnIndex");
	expectThrows([&] { t.columnIndex("missing"); }, "helpers: columnIndex throws on missing");
}

void test_malformed_row_throws() {
	expectThrows([] { csvdb::parseCsv("a,b,c\n1,2\n"); },
	             "malformed: too few columns throws");
	expectThrows([] { csvdb::parseCsv("a,b\n1,2,3\n"); },
	             "malformed: too many columns throws");
}

void test_unterminated_quote_throws() {
	expectThrows([] { csvdb::parseCsv("a,b\n1,\"oops\n"); },
	             "malformed: unterminated quote throws");
}

void test_write_roundtrip() {
	// Includes fields that must be quoted on write: an embedded comma and an
	// embedded (escaped) quote.
	const std::string text = "name,note\nAlice,\"hello, world\"\nBob,\"she said \"\"hi\"\"\"\n";
	const auto original = csvdb::parseCsv(text);
	const std::string serialized = csvdb::writeCsv(original);
	const auto reparsed = csvdb::parseCsv(serialized);
	check(reparsed.headers == original.headers, "write: headers round-trip");
	check(reparsed.rows == original.rows, "write: rows round-trip (quoting preserved)");
}

}  // namespace

int main() {
	std::cout << "Running CSV parser tests...\n";
	test_basic_parse();
	test_quoted_field_with_comma();
	test_escaped_quotes();
	test_no_trailing_newline();
	test_header_only();
	test_column_helpers();
	test_malformed_row_throws();
	test_unterminated_quote_throws();
	test_write_roundtrip();

	std::cout << "\nChecks run: " << g_checks << " | Passed: " << (g_checks - g_failures)
	          << " | Failed: " << g_failures << '\n';
	if (g_failures == 0) {
		std::cout << "All tests passed.\n";
		return 0;
	}
	std::cout << "TESTS FAILED.\n";
	return 1;
}
