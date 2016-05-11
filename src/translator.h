#pragma once
#include <vector>
#include <set>
#include <map>
#include <ostream>
#include <cstdint>

#include <brick-types.h>

namespace std {
// Define operator<< for types with dump method
template <class T>
auto operator<<(std::ostream& o, const T& t) -> decltype(t.dump(o), std::declval<std::ostream&>())  {
	t.dump(o);
	return o;
}
}

namespace cpplink { namespace translator {

enum DataType {Int = 0, Real = 1, Bool = 2, Template = 3}; 
enum class Direction {In, Out};

struct Pin {
    DataType type;
    Direction dir;
    std::string name;
};

struct PrimitiveModule {
    unsigned templates_count;
    std::vector<std::vector<DataType>> allowed_types;
    std::map<std::string, Pin> pins;
    std::string name;
};

struct Net {
    Pin* output_pin;
    std::vector<Pin*> input_pins;
    std::string name;
};

struct ModuleDeclaration {
    std::string type;
    std::string name;
    std::vector<std::string> template_args;
    size_t line;
    
    void dump(std::ostream& o) const {
        o << line << ": ";
        o << "Module decl: " << type << " " << name << "< ";
        for (auto type : template_args)
            o << type << " ";
        o << ">\n";
    }

    std::string generateCode() const;
};

struct NetPinCommand {
    std::string net;
    std::string module;
    std::string pin;
    bool is_out;
    size_t line;
    
    void dump(std::ostream& o) const {
        o << line << ": ";
        o << "NetPinCmd: " << module << "." << pin << (is_out ? " ->" : " <-") <<
            " " << net << "\n";
    }

    std::string generateCode() const;
};

struct NetConstCommand {
    std::string net;
    brick::types::Union<bool, int64_t, double> parameter;
    size_t line;
    
    void dump(std::ostream& o) const {
        o << line << ": ";
        o << "NetConstCmd: ";
        if (parameter.is<bool>()) {
            o << (parameter.get<bool>() ? "true" : "false");
        }
        else if (parameter.is<int64_t>()) {
            o << parameter.get<int64_t>();
        }
        else {
            o << parameter.get<double>();
        }
        
        o << " -> " << net << "\n";
    }
};

struct ParsedFile {
    std::vector<ModuleDeclaration> declarations;
    std::vector<NetPinCommand> net_pin;
    std::vector<NetConstCommand> net_const;
    
    void dump(std::ostream& o) const {
        for (const auto& d : declarations)
            d.dump(o);
        for (const auto& n : net_pin)
            n.dump(o);
        for (const auto& n : net_const)
            n.dump(o);
    }

    std::string generateCode(std::vector<std::string> &mod, std::set<std::string> &net) const;
};

struct ParseError {
    std::string message;
    size_t line;
    
    void dump(std::ostream& o) const {
        o << line << ": " << message << "\n";
    }
};

std::vector<std::string> read_file(std::istream& i);

brick::types::Either<std::vector<ParseError>, ParsedFile>
parse_file(const std::vector<std::string>& file);

template <class T>
T add_line_num(T&& t, size_t line) {
    t.line = line;
    return t;
}

}}
