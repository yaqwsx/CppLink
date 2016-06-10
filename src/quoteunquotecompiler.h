#pragma once

#include "translator.h"
#include <map>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>


namespace cpplink { namespace translator {

extern std::map<std::string, DataType> _types;
extern std::map<std::string, std::string> typeToStr;

/*
 * Specifies modules & types of their pins:
 * Module name, number of template parameters and type of valid types per template argument,
 * pins: type, direction, template position(1 means first template argument, 2 second).
 */
extern std::map<std::string, PrimitiveModule> moduleInfo;

/* Net -> <type, const value> */
extern std::map<std::string, std::pair<DataType, std::string>> constDeclarations;


template <typename T>
std::string itos(T d) {
    std::ostringstream strs;
    strs << std::setprecision(15) << d;
    return strs.str();
}

}}


