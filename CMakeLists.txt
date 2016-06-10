cmake_minimum_required(VERSION 2.8)
project(CppLink CXX C)


# Setup directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/parser)

# Setup external libs
execute_process(COMMAND "git" "submodule" "update" "--init")

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

# Setup sources to constants
add_custom_command(
	OUTPUT cpplink_const_lib.h cpplink_const_lib.cpp
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/utils/files_to_constants.py"
			"${CMAKE_CURRENT_BINARY_DIR}/cpplink_const_lib"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/cpplink_lib/*"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/cpplink_lib/*")

# Include directories
include_directories(src libs)
include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/parser)


# Setup binaries
add_executable(cpplink src/quoteunquotecompiler.cpp ${BIN_SOURCES}
	${BISON_PARSER_OUTPUTS} ${FLEX_SCANNER_OUTPUTS} libs/docopt/docopt.cpp cpplink_const_lib.cpp)
add_executable(tests src/tests.cpp ${BIN_SOURCES} ${TEST_SOURCES}
    ${BISON_PARSER_OUTPUTS} ${FLEX_SCANNER_OUTPUTS})


# Set C++11 standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
target_link_libraries(tests  "${LIBRARIES_FROM_REFERENCES}")
target_link_libraries(cpplink  "${LIBRARIES_FROM_REFERENCES}")
