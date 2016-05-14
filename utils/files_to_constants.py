#! /usr/bin/env python

import sys
import ntpath

escape = [('\\', '\\\\'), ('\'', '\\\''), ('"', '\\"'), ('?', '\\?'), ('\n', '\\n'), ('\r', '')]

if len(sys.argv) < 3:
    print("Invalid usage! Please specify output file and source files")
    sys.exit(1)

with open(sys.argv[1] + ".h", "w") as header, open(sys.argv[1] + ".cpp", "w") as source:
    header.write("#pragma once\n")
    source.write("#include \"{0}.h\"\n".format(sys.argv[1]))

    for item in sys.argv[2:]:
        with open(item) as f:
            content = f.readlines()
        item = ntpath.basename(item).upper().replace(".", "_")
        header.write("extern const char* {0};\n".format(item))
        source.write("const char* {0} = \n".format(item))
        for line in content:
            for pattern, replacement in escape:
                line = line.replace(pattern, replacement)
            source.write("\t\"{0}\"\n".format(line))
        source.write("\t;\n\n")