#pragma once

#include "translator.h"
#include <map>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>


namespace cpplink { namespace translator {

std::map<std::string, DataType> _types{{"INT", Int},{"REAL", Real}, {"BOOL", Bool}};
std::map<std::string, std::string> typeToStr{{"INT", "int64_t"},{"REAL", "double"}, {"BOOL", "bool"}};

/*
 * Specifies modules & types of their pins:
 * Module name, number of template parameters and type of valid types per template argument,
 * pins: type, direction, template position(1 means first template argument, 2 second).
 */


std::map<std::string, PrimitiveModule> moduleInfo
{
    {"ModuleRand",
        PrimitiveModule(1, {{Int, Real, Bool}},
        {{"min",Pin(Template,Direction::In,1)},{"max",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleSin",
        PrimitiveModule(0, {},
        {{"amplitude",Pin(Real,Direction::In)},{"period",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleSaw",
        PrimitiveModule(0, {},
        {{"amplitude",Pin(Real,Direction::In)},{"period",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleCos",
        PrimitiveModule(0, {},
        {{"amplitude",Pin(Real,Direction::In)},{"period",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleTan",
        PrimitiveModule(0, {},
        {{"period",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleLinear",
        PrimitiveModule(0, {},
        {{"out",Pin(Int,Direction::Out)}})},
    {"ModuleConvert",
        PrimitiveModule(2, {{Int,Real},{Int,Real}},
        {{"in",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,2)}})},
    {"ModuleIdentity",
        PrimitiveModule(1, {{Int,Real,Bool}},
        {{"in",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleClamp",
        PrimitiveModule(1, {{Int,Real}},
        {{"min",Pin(Template,Direction::In,1)},{"max",Pin(Template,Direction::In,1)},{"in",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleSum",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleDiff",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleMult",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleDiv",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleMod",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleLogicAnd",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLogicOr",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLogicXor",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLogicImpl",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLogicXnor",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLogicNand",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLogicNor",
        PrimitiveModule(0, {},
        {{"in1",Pin(Bool,Direction::In)},{"in2",Pin(Bool,Direction::In)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLess",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleLessEqual",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleGreater",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleGreaterEqual",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleEqual",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleNotEqual",
        PrimitiveModule(1, {{Int,Real}},
        {{"in1",Pin(Template,Direction::In,1)},{"in2",Pin(Template,Direction::In,1)},{"out",Pin(Bool,Direction::Out)}})},
    {"ModuleInverse",
        PrimitiveModule(1, {{Int,Real}},
        {{"in",Pin(Template,Direction::In,1)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleNegate",
        PrimitiveModule(1, {{Int,Real,Bool}},
        {{"in",Pin(Template,Direction::In,1)},{"out",Pin(Template,Direction::Out,1)}})},
    {"ModuleLog",
        PrimitiveModule(0, {},
        {{"base",Pin(Real,Direction::In)},{"in",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModulePow",
        PrimitiveModule(0, {},
        {{"base",Pin(Real,Direction::In)},{"exp",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleSqrt",
        PrimitiveModule(0, {},
        {{"in",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})},
    {"ModuleAvg",
        PrimitiveModule(0, {},
        {{"in",Pin(Real,Direction::In)},{"out",Pin(Real,Direction::Out)}})}
};


/* Net -> <type, const value> */
std::map<std::string, std::pair<DataType, std::string>> constDeclarations;


template <typename T>
std::string itos(T d) {
    std::ostringstream strs;
    strs << std::setprecision(15) << d;
    return strs.str();
}

}}


