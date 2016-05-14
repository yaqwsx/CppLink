#include <catch.hpp>
#include <sstream>

#include "tests.h"
#include "../src/translator.h"

using namespace cpplink;
using namespace translator;

bool match_module_declaration(ModuleDeclaration a, ModuleDeclaration b) {
	REQUIRE(a.type == b.type);
	REQUIRE(a.name == b.name);
	REQUIRE(a.template_args == b.template_args);
}

bool match_net_command(NetPinCommand a, NetPinCommand b) {
	REQUIRE(a.net == b.net);
	REQUIRE(a.module == b.module);
	REQUIRE(a.pin == b.pin);
	REQUIRE(a.is_out == b.is_out);
}

bool match_net_const(NetConstCommand a, NetConstCommand b) {
	REQUIRE(a.net == b.net);
	REQUIRE(a.parameter == b.parameter);
}

bool match_io_pin(IoPinDeclaration a, IoPinDeclaration b) {
    REQUIRE(a.module == b.module);
	REQUIRE(a.name == b.name);
	REQUIRE(a.is_out == b.is_out);
	CAPTURE(a);
	CAPTURE(b);
}

TEST_CASE("parser:simple") {
	SECTION("comilation error") {
		std::istringstream prog(
        R"(
            TestModule a
            TestModule b
            DoubleModule c
            net
	    )");

        auto parsed_file = parse_file(read_file(prog));
        REQUIRE(parsed_file.isLeft());
	}

	SECTION("successful compilation") {
		std::istringstream prog(
        R"(TestModule a
            TestModule b
            DoubleModule<T, c> c
            net a.pin1 -> nn
            net b.pin2 <- nn
            net 4 -> nn
            net 4.42 -> nn
	    )");

		auto parsed_file = parse_file(read_file(prog));
		if (parsed_file.isLeft()) {
			auto error_list = parsed_file.left();
			CAPTURE(error_list);
			FAIL("Parsing errors occured");
		}

		ParsedFile& res = parsed_file.right();

		REQUIRE(res.declarations.size() == 3);
		match_module_declaration(res.declarations[0], { "TestModule", "a", {}});
		match_module_declaration(res.declarations[1], { "TestModule", "b", { }});
		match_module_declaration(res.declarations[2], { "DoubleModule", "c", { "T", "c" }});

		REQUIRE(res.net_pin.size() == 2);
		match_net_command(res.net_pin[0], { "nn", "a", "pin1", true });
		match_net_command(res.net_pin[1], { "nn", "b", "pin2", false });

		REQUIRE(res.net_const.size() == 2);
		match_net_const(res.net_const[0], { "nn", { int64_t(4) } });
		match_net_const(res.net_const[1], { "nn", { double(4.42) } });
	}

	SECTION("IoPinDeclaration") {
		std::istringstream prog(
        R"(modulein module.pin
           moduleout module.pin
           )");

		auto parsed_file = parse_file(read_file(prog));
		if (parsed_file.isLeft()) {
			auto error_list = parsed_file.left();
			CAPTURE(error_list);
			FAIL("Parsing errors occured");
		}

		ParsedFile& res = parsed_file.right();

		REQUIRE(res.io_pins.size() == 2);
		match_io_pin(res.io_pins[0], { "module", "pin", false });
		match_io_pin(res.io_pins[1], { "module", "pin", true });
	}

	SECTION("Blackbox ok") {
		std::istringstream prog("blackbox 42");

		auto parsed_file = parse_file(read_file(prog));
		if (parsed_file.isLeft()) {
			auto error_list = parsed_file.left();
			CAPTURE(error_list);
			FAIL("Parsing errors occured");
		}

		ParsedFile& res = parsed_file.right();

		REQUIRE(res.blackbox_def.isJust());
		REQUIRE(res.blackbox_def.value().steps == 42);
	}

	SECTION("Multiple blackboxes") {
		std::istringstream prog(
        R"(blackbox 42
           blackbox 56)");

		auto parsed_file = parse_file(read_file(prog));
		REQUIRE(parsed_file.isLeft());
	}
}
