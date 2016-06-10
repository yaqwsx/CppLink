#pragma once

#include <stdexcept>
#include <memory>
#include <vector>

#include "brick-types.h"
#include "translator.h"

#pragma clang diagnostic ignored "-Wdeprecated-register"

using StatementUnion = brick::types::Union<
    cpplink::translator::ModuleDeclaration,
    cpplink::translator::NetPinCommand,
    cpplink::translator::NetConstCommand,
    cpplink::translator::BlackboxCommand,
    cpplink::translator::IoPinDeclaration,
    cpplink::translator::GenericDeclaration,
    std::string>;

StatementUnion parse_line(const std::string& s);

