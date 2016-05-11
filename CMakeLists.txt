cmake_minimum_required(VERSION 2.8)
project(CppLink CXX C)


# Setup directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/parser)

# Import packages
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# Setup C++ sources
file(GLOB BIN_SOURCES src/*.cpp)
list(REMOVE_ITEM BIN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tests.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/quoteunquotecompiler.cpp")
file(GLOB TEST_SOURCES tests/*.cpp)

# Setup Bison/Flex sources
bison_target(PARSER src/parsers/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.cpp)
flex_target(SCANNER src/parsers/tokens.l
    ${CMAKE_CURRENT_BINARY_DIR}/parser/tokens.cpp)
add_flex_bison_dependency(SCANNER PARSER)

# Include directories
include_directories(src libs)
include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/parser)


# Setup binaries
add_executable(cpplink src/quoteunquotecompiler.cpp ${BIN_SOURCES}
	${BISON_PARSER_OUTPUTS} ${FLEX_SCANNER_OUTPUTS})
add_executable(tests src/tests.cpp ${BIN_SOURCES} ${TEST_SOURCES}
    ${BISON_PARSER_OUTPUTS} ${FLEX_SCANNER_OUTPUTS})


# Set C++11 standard
add_definitions("-std=c++11")
target_link_libraries(tests  "${LIBRARIES_FROM_REFERENCES}")
