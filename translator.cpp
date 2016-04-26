#include "translator.h"
#include "parser_inc.h"

#include <cassert>

namespace cpplink { namespace translator {

std::string strip_comments(const std::string& s) {
    return s.substr(0, s.find('#'));
}

std::vector<std::string> read_file(std::istream& input) {
    std::vector<std::string> file;
    std::string line;
    while (std::getline(input, line)) {
        file.push_back(std::move(line));
    }
    return file;
}

brick::types::Either<std::vector<ParseError>, ParsedFile>
parse_file(const std::vector<std::string>& file)
{
    ParsedFile result;
    std::vector<ParseError> errors;
    size_t line_num = 0;
    for (const std::string line : file) {
        line_num++;

        StatementUnion statement = parse_line(strip_comments(line));
        
        if (statement.is<std::string>()) {
            errors.push_back({ statement.get<std::string>(), line_num });
        }
        else if (statement.is<ModuleDeclaration>()) {
            result.declarations.push_back(add_line_num(
                statement.get<ModuleDeclaration>(), line_num));
        }
        else if (statement.is<NetPinCommand>()) {
            result.net_pin.push_back(add_line_num(
                statement.get<NetPinCommand>(), line_num));
        }
        else if (statement.is<NetConstCommand>()) {
            result.net_const.push_back(add_line_num(
                statement.get<NetConstCommand>(), line_num));
        }
        else {
            assert(false && "Unknown type in union!");
        }
    }

    if (!errors.empty())
        return errors;

    return result;
}

}}