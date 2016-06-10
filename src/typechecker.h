#pragma once

#include "translator.h"
#include "cpplink_lib/doubleequal.h"
#include <algorithm>

namespace cpplink { namespace translator {

using DeclarationsMap = std::map<std::string, const ModuleDeclaration*>;

std::map<std::string, Pin>& getPins(const ModuleDeclaration* mod);
bool inferPinType(const ModuleDeclaration* mod, std::string pName, DataType& type);
void typeCheckModDecl(const ModuleDeclaration& d, std::vector<ParseError>& errors);
std::vector<ParseError> typeCheck(ParsedFile& pf, DeclarationsMap& modules);

}}
