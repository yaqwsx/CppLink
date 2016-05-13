#include "translator.h"
#include "parsers/parser_inc.h"

#include <cassert>
#include <cctype>
#include <algorithm>

namespace cpplink { namespace translator {

std::string ltrim(std::string s) {
	auto end = std::find_if(s.begin(), s.end(),
        [] (char c) { return !std::isspace(c); });
	s.erase(s.begin(), end);
	return s;
}

std::string rtrim(std::string s) {
	auto begin = std::find_if(s.rbegin(), s.rend(),
        [] (char c) { return !std::isspace(c); });
	s.erase(begin.base(), s.end());
	return s;
}

std::string trim(std::string s) {
	return ltrim(rtrim(s));
}

std::string strip_comments(const std::string& s) {
    return trim(s.substr(0, s.find('#')));
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

	    std::string stripped_line = strip_comments(line);
        if (stripped_line.empty())
            continue;
	    StatementUnion statement = parse_line(stripped_line);
        
	    if (statement.is<std::string>()) {
		    errors.push_back({ statement.get<std::string>(), line_num });
	    }
	    else if (statement.is<ModuleDeclaration>()) {
		    result.declarations.push_back(add_line_num(
		        statement.get<ModuleDeclaration>(),
			    line_num));
	    }
	    else if (statement.is<NetPinCommand>()) {
		    result.net_pin.push_back(add_line_num(
		        statement.get<NetPinCommand>(),
			    line_num));
	    }
	    else if (statement.is<NetConstCommand>()) {
		    result.net_const.push_back(add_line_num(
		        statement.get<NetConstCommand>(),
			    line_num));
	    }
	    else if (statement.is<BlackboxCommand>()) {
		    if (result.blackbox_def) {
			    errors.push_back({
                     "Redefinition of blackbox steps! See line "
                     + std::to_string(result.blackbox_def.value().line)
                     + " for previous declaration", line_num
                     });
		    }
            else {
	            result.blackbox_def = brick::types::Maybe<BlackboxCommand>::Just(
                    add_line_num(statement.get<BlackboxCommand>(), line_num));
            }
        }
	    else if (statement.is<IoPinDeclaration>()) {
		    result.io_pins.push_back(add_line_num(
                statement.get<IoPinDeclaration>(),
                line_num));
	    }
	    else if (statement.is<GenericDeclaration>()) {
		    result.generics.push_back(add_line_num(
                statement.get<GenericDeclaration>(),
                line_num));
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
