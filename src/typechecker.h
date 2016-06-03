#pragma once

#include "translator.h"
#include "cpplink_lib/doubleequal.h"
#include <algorithm>

namespace cpplink { namespace translator {

using DeclarationsMap = std::map<std::string, const ModuleDeclaration*>;


std::map<std::string, Pin>& getPins(const ModuleDeclaration* mod) {
    return moduleInfo[mod->type].pins;
}

bool inferPinType(const ModuleDeclaration* mod, std::string pName, DataType& type){
    Pin result = ((*(getPins(mod).find(pName))).second );
    if (result.type != Template) {
        type = result.type;
        return true;
    }
    if (mod->template_args.size() > result.pos-1) {
        type = _types[mod->template_args[result.pos-1]];
        return true;
    }
    return false; //wrong number of template args for module
}


void typeCheckModDecl(const ModuleDeclaration& d, std::vector<ParseError>& errors) {
    auto mod = moduleInfo.find(d.type);

    if (mod == moduleInfo.end()) {
        errors.push_back(ParseError("Unknown module type: " + d.type, d.line));
    } else {
        unsigned providedTCount = d.template_args.size();
        unsigned expectedTCount = moduleInfo[d.type].templates_count;

        if (expectedTCount != providedTCount) {
            errors.push_back(ParseError("Invalid number of template arguments, expected "
                                        + itos<unsigned>(expectedTCount) + " provided "
                                        + itos<unsigned>(providedTCount), d.line));
        } else {

            for(size_t i = 0; i < providedTCount; i++) {

                auto allowed = (*mod).second.allowed_types[i];
                auto it = std::find( allowed.begin(), allowed.end(), _types[d.template_args[i]] );

                if( it == allowed.end() )
                    errors.push_back(ParseError("Template parameter " + itos<int64_t>(i+1) +
                                                " incorrect.", d.line));
            }

        }
    }
}


std::vector<ParseError> typeCheck(ParsedFile& pf, DeclarationsMap& modules) {
    std::vector<ParseError> errors;

    for (const auto& d : pf.declarations) {
        typeCheckModDecl(d, errors);

        auto mFound = modules.find(d.name);
        if (mFound != modules.end())
            errors.push_back(ParseError("Module with name \"" + d.name + "\" redeclared. "
                                        "First declared: " + itos<size_t>((*mFound).second->line), d.line));
        modules.insert({d.name, &d});
    }

    for (const auto& n : pf.net_const) {
        std::string val;
        DataType type;

        if (n.parameter.is<bool>()) {
            val  = (n.parameter.get<bool>() ? "true" : "false");
            type = Bool;
        }
        else if (n.parameter.is<int64_t>()) {
            val  = itos<int>(n.parameter.get<int64_t>());
            type = Int;
        }
        else {
            val = itos<double>(n.parameter.get<double>());
            type = Real;
        }

        if (constDeclarations.find(n.net) != constDeclarations.end()) {
            constDeclarations[n.net] = {type, val};
            errors.push_back(ParseError("Multiple const nettings within "
                                        + n.net + ".", n.line));
        } else {
            constDeclarations.insert({n.net,{type, val}});
        }
    }


    for (const auto& n : pf.net_pin) {

        auto modIt = modules.find(n.module);

        if (modIt == modules.end()) {
            errors.push_back(ParseError("Module with name \"" + n.module +
                                        "\" undeclared, cannot infer net/pin type.", n.line));

        } else {
            auto it = modules.find(n.net);

            if (it != modules.end()) {
                errors.push_back(ParseError("Structure with name \"" + n.net +
                                            "\" redeclared, from this context: "
                                            + itos<unsigned>((*it).second->line), n.line));
            }

            auto pins = getPins((*modIt).second);
            auto pinIt = pins.find(n.pin);

            if ( pinIt == pins.end() ) {
                errors.push_back(ParseError("Invalid pin name \"" + n.pin +
                                            "\" for module " + (*modIt).second->type, n.line));
            } else {
                if (((*pinIt).second.dir == Direction::Out && !n.is_out) || ((*pinIt).second.dir == Direction::In && n.is_out)) {
                    errors.push_back(ParseError("Direction of the pin does not match.", n.line));
                }

                auto decl = constDeclarations.find(n.net);
                if (decl != constDeclarations.end()) {

                    DataType inferredType;
                    bool check = inferPinType((*modIt).second, n.pin, inferredType);
                    if (check && ((*decl).second.first != inferredType)) {
                        errors.push_back(ParseError("Mismatch in pin types in net: " + n.net, n.line));
                    }

                }
            }
        }
    }

    return errors;
}


}}
