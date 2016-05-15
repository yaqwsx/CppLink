# CppLink

Simple data-flow programming in C/C++.

CppLink is inspired by Simulink and HAL from the LinuxCNC project. CppLink takes
the basics from data-flow programming and presents them in the simplest form
available.

CppLink contains two parts - C++ data-flow programming library with predefined
modules for data transformation and CppLink translator which translates netlist
files into C++ code that simulates the system described by the netlist file.

# How to use it

There are two ways to use CppLink. Either you can use the data-flow C++ library
directly and write a C++ code using it, or you can describe your system's netlist
in CppLink language and let CppLink perform the translation to C++ code.

Consider the following simple example:

```
ModuleSaw saw
net saw.amplitude <- sawamp
net saw.period <- sawper
net saw.out -> a
net 10.0 -> sawamp
net 25.0 -> sawper

ModuleSin sin
net sin.amplitude <- sinamp
net sin.period <- sinper
net sin.out -> b
net 10.0 -> sinamp
net 10.0 -> sinper

ModuleSum<REAL> sum
net sum.in1 <- a
net sum.in2 <- b
net sum.out -> out
```

This example instantiates three data-transformation boxes -- one saw generator
with a period of 25 ticks and amplitude 10 and a sine wave generator with
amplitude 10 and a period of 10 ticks. Output signal of these generators is then
combined together.

Issuing `cpplink examples.cpplink output.cpp --steps=200 --interface=csv
--watch=a,b,out` translates the netlist into C++ code that performs simulation
of 200 steps of this system and produces CSV table output with values of nets
`a`, `b` and `out` on stdout. The produced code has to be translated with C++11
standard.

Issuing `cpplink examples.cpplink output.cpp --steps=-1` produces C++ code that
has no output and simulates an infinite number of system steps. This code is not
meant to be run, however it can be verified using any verification tool that
accepts C++ as an input language.

# Translation options

There are several options user can specify when translating netlist into C++
code:

* By-default produced code is self-contained (CppLink library is embedded into
  the source code). This produces quite large output files. If CppLink is
  installed and correct include paths to CppLink libraries are present,
  `--uselib` flag produces code without embedded library.

* There are three supported output interfaces. These interfaces can be specified
  using the `--interface=<type>` flag.

  - `csv` - simple CSV output format with quoted strings

  - `excel` - CSV output with MS Excel-specific dialect

  - `plain` - tab-separated columns. This format is suitable e.g. for GNUplot.

* To specify which columns should be output in the type, a comma-separated list
  of net names can be specified using `--watch=<columns>`.

# Authors

Developed by Zuzana Baranová and Jan Mrázek as a project in PB173 class at FI
MUNI.
