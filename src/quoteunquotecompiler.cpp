#include "quoteunquotecompiler.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <docopt/docopt.h>

using std::string;

static const char USAGE[] =
R"(CppLink.

Usage:
    cpplink <input_file> <output_file> [--interface --watch --steps]
    cpplink -h | --help
    cpplink --version

Options:
    -h --help     Show help.
    --version     Show version.
    --interface   Specifies output interface of produces code: silent, csv or plan
    --watch       Comma separated list with net names, which will be watched
    --steps       Number of iterations
)";

namespace cpplink {

using namespace translator;

std::map<string, string> _types{{"REAL", "double"}, {"INT", "int64_t"}, {"BOOL", "bool"}};


string generateHeaders() {
    return "#include \"modules.h\" \n"
           "#include \"translator.h\" \n"
           "#include <iostream>\n\n"
           "using namespace cpplink;\n\n";
}

string tabs(unsigned u) {
    string res;
    for (unsigned i=0; i<u; i++)
        res += "\t";
    return res;
}

template <typename T>
string itos(T d) {
    std::ostringstream strs;
    strs << std::setprecision(15) << d;
    return strs.str();
}

string getPinType(string moduleName, string pinName) {
    std::vector<string> types{"double","int64_t","bool"};
    const ModuleDeclaration* m = moduleDeclarations[moduleName];
    string potential = modulePinTypes[m->type][pinName];

    if(std::find(types.begin(), types.end(), potential) != types.end()) {
        return potential;
    }
    return _types[m->template_args[std::stoi(potential)-1]];
}

string generateSystemSteps(std::vector<string>& mods, std::set<string>& nets) {
    string res = tabs(1) + "std::vector<std::pair<Module*, std::string>> modules{";
    for(unsigned i=0; i<mods.size(); i++) {
        res += "{&" + mods[i] + ", " + "\"" + mods[i] + "\"}";
        if (i<mods.size()-1) res += ", ";
    }
    res += "};\n";

    res += tabs(1) + "for(size_t i=0; i<10; i++) {\n" +
            tabs(2) + "for(auto m : modules) {\n" +
            tabs(3) + "std::cout << m.second << \"\\n\";\n" +
            tabs(3) + "m.first->step();\n" + tabs(2) + "}\n" + tabs(1) + "}\n";

    return res;
}


string generateNetDeclaration(string name, string type) {
    return tabs(1) + "Net< " + type + " > " + name + ";\n";
}

string generateConstWiring(const NetPinCommand& n) {
    return tabs(1) + n.module + "." + n.pin + " = " + constDeclarations[n.net] + ";\n";
}

string ModuleDeclaration::generateCode() const {
    string res = tabs(1) + type;
    size_t argsize = this->template_args.size();

    if (argsize) {
        res += "< ";
        for(unsigned i=0; i<argsize; i++) {
            res += _types.find(template_args[i])->second;
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

string ParsedFile::generateCode(std::vector<string>& mods, std::set<string>& nets) const {
        std::string res;

        for (const auto& d : declarations) {
            res += d.generateCode();
            mods.push_back(d.name);
            moduleDeclarations.insert({d.name, &d});
        }
        res += "\n";

        for (const auto& n : net_const) {
            string val;

            if (n.parameter.is<bool>()) {
                val  = (n.parameter.get<bool>() ? "true" : "false");
            }
            else if (n.parameter.is<int64_t>()) {
                val  = itos<int>(n.parameter.get<int64_t>());
            }
            else {
                val = itos<double>(n.parameter.get<double>());
            }

            nets.insert(n.net);
            constDeclarations.insert({n.net, val});
        }

        for (const auto& n : net_pin) {
            if (constDeclarations.find(n.net) != constDeclarations.end()) {
                res += generateConstWiring(n);
            } else {
                if (nets.find(n.net) == nets.end()) {
                    res += generateNetDeclaration(n.net, getPinType(n.module, n.pin));
                }
                res += n.generateCode();
            }
            nets.insert(n.net);
        }
        res += "\n";

        return res + generateSystemSteps(mods, nets);
}

} //namespace cpplink


using namespace cpplink;

int main(int argc, char* argv[]) {

	std::map<std::string, docopt::value> args = docopt::docopt(USAGE, 
		{ argv + 1, argv + argc },
		true,
		"CppLink 0.1");

    std::ifstream file(args["<input_file>"].asString());
    // ToDo: Check if file is open

    std::vector<string> vecs = translator::read_file(file);
    auto res = translator::parse_file(vecs);

    std::vector<string> modules;
    std::set<string> nets;

    std::ofstream fileout(args["<output_file>"].asString());
    // ToDo: Check if file open

    fileout << generateHeaders() << "int main(int argc, char* argv[]){\n"
            << res.right().generateCode(modules, nets) << tabs(1) << "return 0;\n" << "}\n";

}
