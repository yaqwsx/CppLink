#include <catch.hpp>
#include <sstream>
#include "../src/cpplink_lib/table_writer.h"

#include "tests.h"

using namespace cpplink;


TEST_CASE("simple_table") {
	SECTION("Constructor") {
		std::ostringstream s;
		TableWriter<CsvDialect, int, int> table(s, {"A", "B"});
		table.write_line(10, 21);
		REQUIRE(s.str() == "\"A\",\"B\"\n10,21\n");
	}
}
