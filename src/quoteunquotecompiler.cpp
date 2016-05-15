#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <docopt/docopt.h>

#include "quoteunquotecompiler.h"
#include "typechecker.h"
#include <cpplink_const_lib.h>

using std::string;

static const char USAGE[] =
R"(CppLink.

Usage:
    cpplink <input_file> <output_file> --steps=<x> [--interface=<type> --watch=<list>] [--uselib]
    cpplink -h | --help
    cpplink --version

Options:
    -h --help             Show help.
    --version             Show version.
    --interface=<type>    Specifies output interface of produces code: csv, excel or plain.
    --watch=<list>        Comma separated list with net names, which will be watched.
    --steps=<x>           Number of iterations, -1 for infinity.
    --uselib              Use #include <cpplink_lib.h> instead of embedding it.
)";

namespace cpplink {

using namespace translator;

std::map<string, string> _types{{"REAL", "double"}, {"INT", "int64_t"}, {"BOOL", "bool"}};


string generateHeaders(bool embed) {
	std::string res;
	res += "// CppLink header begin ===========================================================\n";
	res += "#include <iostream>\n";
	if (embed) {
		res += "#define _CPPLINK_EMBEDDED_CODE_\n";
		res += "#include <iostream>\n";
		res += MAYBE_H;
		res += "\n";
		res += DOUBLEEQUAL_H;
		res += "\n";
		res += MODULES_H;
		res += "\n";
		res += TABLE_WRITER_H;
		res += "\n";
	}
	else {
		res += "#include <cpplink_lib.h>\n";
    }

	res += "\nusing namespace cpplink;\n";
	res += "// CppLink header end =============================================================\n\n\n";

	return res;
}

string tabs(unsigned u) {
    string res;
    for (unsigned i=0; i<u; i++)
        res += "\t";
    return res;
}

string getPinType(DeclarationsMap& modules, string moduleName, string pinName) {
    std::vector<DataType> types{Int, Real, Bool};

    const ModuleDeclaration* m = modules[moduleName];
    Pin pin = moduleInfo[m->type].pins[pinName];
    DataType type = pin.type;

    if(std::find(types.begin(), types.end(), type) != types.end()) {
        return DataTypeToString[static_cast<unsigned>(type)];
    }
    return typeToStr[m->template_args[pin.pos - 1]];
}

string generateSystemSteps(const std::vector<ModuleDeclaration>& modules,
    const std::map<std::string, std::string>& nets,
    const std::vector<string>& watched_nets, long steps)
{
	std::string res;
	if (steps == -1)
		res += tabs(1) + "for(long __cpplink_i = 0; true ; __cpplink_i++) {\n";
	else
		res += tabs(1) + "for(long __cpplink_i = 0; "
            "__cpplink_i != " + std::to_string(steps) + "; __cpplink_i++) {\n";

	res += tabs(2) + "// Propagate values through nets\n";
	for (const auto& net : nets)
		res += tabs(2) + net.first + ".step();\n";

	res += "\n";
	res += tabs(2) + "// Do step in each module\n";

	for (const auto& module : modules)
		res += tabs(2) + module.name + ".step();\n";

	if (!watched_nets.empty()) {
		res += "\n";
		res += tabs(2) + "// Output values in this step\n";
		res += tabs(2) + "__cpplink_table.write_line(\n";
		res += tabs(3) + "__cpplink_i";
		for (const std::string& net : watched_nets)
			res += ",\n" + tabs(3) + net + ".getValue()";
		res += "\n" + tabs(2) + ");\n";
    }

	res += tabs(1) + "}\n";
	return res;
}


string generateNetDeclaration(string name, string type) {
    return tabs(1) + "Net< " + type + " > " + name + ";\n";
}

string generateConstWiring(const NetPinCommand& n) {
    return tabs(1) + n.module + "." + n.pin + " = " + constDeclarations[n.net].second + ";\n";
}

string ModuleDeclaration::generateCode() const {
    string res = tabs(1) + type;
    size_t argsize = this->template_args.size();

    if (argsize) {
        res += "< ";
        for(unsigned i=0; i<argsize; i++) {
            res += typeToStr.find(template_args[i])->second;
            if (i<argsize-1) res += ", ";
        }
        res += " >";
    }
    return res + " " + name + ";\n";
}

string NetPinCommand::generateCode() const {
    string res = tabs(1) + net + ".";
    res += is_out? "setOutputPin" : "addInputPin";
    res += "(&" + module + "." + pin + ");\n";

    return res;
}

string ParsedFile::generateCode(DeclarationsMap& modules, std::map<string, string>& nets) const {
        std::string res;

        for (const auto& d : declarations) {
            res += d.generateCode();
        }
        res += "\n";

        for (const auto& n : net_pin) {
	        string net_type;
            if (constDeclarations.find(n.net) != constDeclarations.end()) {
                res += generateConstWiring(n);
            } else {
                if (nets.find(n.net) == nets.end()) {
	                net_type = getPinType(modules, n.module, n.pin);
                    res += generateNetDeclaration(n.net, net_type);
                }
                res += n.generateCode();
	            nets.insert({ n.net, net_type });
            }
            
        }
        res += "\n";
        return res;
}

void print_error_messages(std::ostream& o, std::vector<translator::ParseError>& errors,
		std::vector<string>& source)
{
	std::cerr << errors.size() <<  " translation errors occurred!\n";
	for (const translator::ParseError& e : errors) {
		std::cerr << "On line " << e.line << ": " << e.message << "\n";
		std::cerr << "   line: " << source[e.line - 1] << "\n";
	}

	std::cerr << "Translation aborted\n";
}

std::pair<std::vector<std::string>, bool>
nets_to_watch(std::string config, translator::ParsedFile& file) {
	std::vector<std::string> errs;
	std::vector<std::string> nets;

	std::set<std::string> net_names;
	for (const auto& cmd : file.net_pin)
		net_names.insert(cmd.net);
	for (const auto& cmd : file.net_const)
		net_names.insert(cmd.net);

	std::istringstream in(config);
	std::string net_name;
    while(getline(in, net_name, ',')) {
	    if (net_names.find(net_name) == net_names.end()) {
		    errs.push_back("\"" + net_name + "\" is not a valid net name.");
	    }
	    else {
		    nets.push_back(net_name);
        }
    }

	if (errs.empty())
		return { nets, true };
	return { errs, false };
}

std::string generate_output(std::string output_type, const std::map<std::string, std::string>& nets,
		const std::vector<std::string>& watched)
{
	if (output_type == "silent")
		return {};
	if (output_type == "excel")
		output_type = "ExcelCsvDialect";
	else if (output_type == "csv")
		output_type = "CsvDialect";
	else if (output_type == "plain")
		output_type = "PlainTextDialect";
	else
		assert(false && "Invalid output type specified");

	std::string res;
	res += tabs(1) + "TableWriter<" + output_type + ", int";
	for (const std::string& net : watched) {
		res += ",\n";
		res += tabs(3) + "Maybe<" + nets.find(net)->second + ">";
    }
	res += "\n" + tabs(2) + "> __cpplink_table (std::cout, {\"step\"";
	for (const std::string& net : watched) {
		res += ",\n";
		res += tabs(3) + '"' + net + '"';
    }
	res += "\n" + tabs(2) + "});\n\n";

	return res;
}

} //namespace cpplink


using namespace cpplink;

int main(int argc, char* argv[]) {

	std::map<std::string, docopt::value> args = docopt::docopt(USAGE, 
		{ argv + 1, argv + argc },
		true,
		"CppLink 0.1");

	std::string in_file = args["<input_file>"].asString();
	std::string out_file = args["<output_file>"].asString();
	std::string output_type = args["--interface"].isString() ? args["--interface"].asString() : "silent";
	std::transform(output_type.begin(), output_type.end(), output_type.begin(), ::tolower);
	std::string to_watch = args["--watch"].isString() ? args["--watch"].asString() : "";
	long        step_num = args["--steps"].asLong();
	bool        embed_lib = !args["--uselib"].asBool();

	if (step_num < -1) {
		std::cerr << "Invalid number of steps! Please specify positive number or -1 for infinite loop\n";
		return 1;
    }

    std::ifstream filein(in_file);
	if (!filein.is_open()) {
		std::cerr << "Cannot open input file " << in_file << "!\n";
		return 1;
	}

	std::ofstream fileout(out_file);
	if (!fileout.is_open()) {
		std::cerr << "Cannot open output file " << out_file << "!\n";
		return 1;
	}

    std::vector<string> vecs = translator::read_file(filein);
	if (!filein.good() && !filein.eof()) {
		std::cerr << "Cannot read from input file " << in_file << "!\n";
		return 1;
	}

    auto res = translator::parse_file(vecs);
	if (res.isLeft()) {
		print_error_messages(std::cerr, res.left(), vecs);
		return 1;
	}

    DeclarationsMap modules; //"name" -> ModuleDeclaration
    std::map<string, string> nets; // net -> type
    
    ParsedFile parsedFile = res.right();
    auto errors = typeCheck(parsedFile, modules);
    
	if (!errors.empty()) {
		std::cerr << "Could not produce .cpp file, following errors occurred:\n\n";
		std::sort(errors.begin(), errors.end(), [](ParseError& a, ParseError& b){ return a.line < b.line; });
		print_error_messages(std::cerr, errors, vecs);
		return 1;
	}

	std::vector<std::string> net_watch;
	bool valid;
	std::tie(net_watch, valid) = nets_to_watch(to_watch, parsedFile);
	if (!valid) {
		std::cerr << "Invalid net watch list:\n";
		for (const std::string& err : net_watch)
			std::cerr << "\t" << err << "\n";
		return 1;
    }

	fileout << generateHeaders(embed_lib)
	        << "int main(int argc, char* argv[]){\n"
	        << parsedFile.generateCode(modules, nets)
            << generate_output(output_type, nets, net_watch)
            << generateSystemSteps(parsedFile.declarations, nets, net_watch, step_num)
            << tabs(1) << "return 0;\n" << "}\n";
                
    if (!fileout.good()) {
		std::cerr << "Cannot write to output file " << out_file << "!\n";
		return 1;
	}

	return 0;
}
