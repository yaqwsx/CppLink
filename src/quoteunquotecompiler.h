#ifndef QUOTEUNQUOTECOMPILER
#define QUOTEUNQUOTECOMPILER

#include <map>
#include <string>
#include "translator.h"

namespace cpplink {

/*
 * Specifies modules & types of their pins, 1 means first template argument, 2 second
 */
std::map<std::string, std::map<std::string, std::string>> modulePinTypes
{
    {"ModuleRand",{{"min","1"},{"max","1"},{"out","1"}}},
    {"ModuleSin",{{"amplitude","double"},{"period","double"},{"out","double"}}},
    {"ModuleCos",{{"amplitude","double"},{"period","double"},{"out","double"}}},
    {"ModuleTan",{{"period","double"},{"out","double"}}},
    {"ModuleConvert",{{"in","1"},{"out","2"}}},
    {"ModuleIdentity",{{"in","1"},{"out","1"}}},
    {"ModuleClamp",{{"min","1"},{"max","1"},{"in","1"},{"out","1"}}},
    {"ModuleSum",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleDiff",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleMult",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleDiv",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleMod",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleLogicAnd",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLogicOr",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLogicXor",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLogicImpl",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLogicXnor",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLogicNand",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLogicNor",{{"in1","bool"},{"in2","bool"},{"out","bool"}}},
    {"ModuleLess",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleLessEqual",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleGreater",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleGreaterEqual",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleEqual",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleNotEqual",{{"in1","1"},{"in2","1"},{"out","1"}}},
    {"ModuleInverse",{{"in","1"},{"out","double"}}},
    {"ModuleNegate",{{"in","1"},{"out","1"}}},
    {"ModuleLog",{{"base","double"},{"in","double"},{"out","double"}}},
    {"ModulePow",{{"base","double"},{"exp","double"},{"out","double"}}},
    {"ModuleSqrt",{{"in","double"},{"out","double"}}},
    {"ModuleAvg",{{"in","double"},{"out","double"}}}
};


/* Stores declared modules in the form "name" -> "ModuleDeclaration"
   for deduction of net types where the modules are wired */

std::map<std::string, const translator::ModuleDeclaration*> moduleDeclarations;

/* Net -> const value */
std::map<std::string, std::string> constDeclarations;


}

#endif // QUOTEUNQUOTECOMPILER

