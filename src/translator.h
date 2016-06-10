#pragma once
#include <vector>
#include <set>
#include <map>
#include <ostream>
#include <cstdint>

#include <brick-types.h>

namespace cpplink { namespace translator {

template <class T>
auto operator<<(std::ostream& o, const T& t) -> decltype(t.dump(o), std::declval<std::ostream&>())  {
    t.dump(o);
    return o;
}

enum DataType {Int = 0, Real = 1, Bool = 2, Template = 3}; 
enum class Direction {In, Out};

const std::vector<std::string> DataTypeToString{"int64_t","double","bool"};

struct Pin {

    Pin(){}

    Pin(DataType typ, Direction d, unsigned pos=0)
        :
          type(typ),
          dir(d),
          pos(pos)
    {}

    DataType type;
    Direction dir;
    unsigned pos;
};

struct PrimitiveModule {

    PrimitiveModule(){}

    PrimitiveModule(unsigned count,
                    std::vector<std::vector<DataType>> allowed,
                    std::map<std::string, Pin> pins)
        :
          templates_count(count),
          allowed_types(allowed),
          pins(pins)
    {}

    unsigned templates_count;
    std::vector<std::vector<DataType>> allowed_types;
    std::map<std::string, Pin> pins;
};

struct Net {
    Pin* output_pin;
    std::vector<Pin*> input_pins;
    std::string type;
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

struct IoPinDeclaration {
	std::string module;
	std::string name;
	bool is_out;
	size_t line;

    void dump(std::ostream& o) const {
	    o << line << ": IoDefinition: " << (is_out ? "out" : "in")
            << module << "." << name << "\n";
    }
};

struct BlackboxCommand {
	int32_t steps;
	size_t line;

    void dump(std::ostream& o) const {
	    o << line << ": BlackboxCommand: " << steps << "\n";
    }
};

struct GenericDeclaration {
	std::string name;
	size_t line;

    void dump(std::ostream& o) const {
	    o << line << ": GenericDeclaration: " << name << "\n";
    }
};

struct ParsedFile {
	brick::types::Maybe<BlackboxCommand> blackbox_def;
    std::vector<ModuleDeclaration> declarations;
    std::vector<NetPinCommand> net_pin;
    std::vector<NetConstCommand> net_const;
	std::vector<IoPinDeclaration> io_pins;
	std::vector<GenericDeclaration> generics;
    
    void dump(std::ostream& o) const {
        for (const auto& d : declarations)
            d.dump(o);
        for (const auto& n : net_pin)
            n.dump(o);
        for (const auto& n : net_const)
            n.dump(o);
    }

    std::string generateCode(std::map<std::string, const ModuleDeclaration*>&,
        std::map<std::string, std::string> &) const;
};

struct ParseError {
    ParseError(){}

    ParseError(std::string mess, size_t lin)
        :
          message(mess),
          line(lin)
    {}

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

std::string trim(std::string s);

}}
