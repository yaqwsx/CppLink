# Modules in CppLink

This document describes all primitive modules available in CppLink.

# List of modules

# Generators

## const\<T, Val\>

- output pins: `out(T)`
- valid types for T: INT, REAL, BOOL
- description: Produces constant Val of type T at the `out` pin

## rand\<T\>

- output pins: `out(T)`
- description: Returns a random value of type T

## sin/cos

- input pins: `amplitude(REAL)`, `period(REAL)`
- output pins: `out(REAL)`
- description: Generates a sine/cosine wave that adheres to the `amplitude` and `period` provided on input. If either is `nothing`, produces `nothing` and returns to original position

## tan

- input pins: `period(REAL)`
- output pins: `out(REAL)`
- description: Generates a tangent wave with specified `period`

## triangle/saw

- input pins: `amplitude(REAL)`, `period(REAL)`
- output pins: `out(REAL)`
- description: Generates a triangle/saw wave with specified `amplitude` and `period`

## square

- input pins: `period(REAL)`, `amplitude(REAL)`, `mid(REAL)`
- output pins: `out(REAL)`
- description: Generates a square wave with specified `amplitude` and `period` whose middle value between minimal and maximal is `mid`

# Helpers

## conversion\<T,U\>

- input pins: `in(T)`
- output pins: `out(U)`
- valid T,U combinations: INT,REAL and vice versa
- description: Converts a value of type T to a value of type U

## identity\<T\>

- input pins: `in(T)`
- output pins: `out(T)`
- valid types for T: INT, REAL, BOOL
- description: Outputs the `in` value

## clamp\<T, U\>

- input pins: `min(T)`, `max(T)`, `in(U)`
- output pins: `out(U)`
- valid types for T/U: INT, REAL
- description: Limits the output value to min and max bounds, suitable for restricting generators

#Functions

## sum\<T\>

- input pins: `in1(T)`, `in2(T)`
- output pins: `out(T)`
- valid types for T: INT, REAL
- description: Outputs the sum of values on `in1` and `in2`

## diff\<T\>

- input pins: `in1(T)`, `in2(T)`
- output pins: `out(T)`
- valid types for T: INT, REAL
- description: Outputs the difference of values on `in1` and `in2`

## mult\<T\>

- input pins: `in1(T)`, `in2(T)`
- output pins: `out(T)`
- valid types for T: INT, REAL
- description: Outputs the product of `in1` and `in2`


## div\<T\>

- input pins: `in1(T)`, `in2(T)`
- output pins: `out(REAL)`
- valid types for T: INT, REAL
- description: Outputs the quotient of values on `in1` and `in2`. Exception is thrown on div by 0

## mod

- input pins: `in1(INT)`, `in2(INT)`
- output pins: `out(INT)`
- description: Outputs the remainder of division of `in1` by `in2`. Exception is thrown on mod by 0

## binary logic operators

- input pins: `in1(BOOL)`, `in2(BOOL)`
- output pins: `out(BOOL)`
- description: Provide basic logic functions - and, or, implication, biconditional, xor, xnor, nand, nor

## relational operators \<T\>

- input pins: `in(T)`, `val(T)`
- output pins: `out(BOOL)`
- valid types for T: INT, REAL
- description: Tests the relation between `in` and `val` based on relational expression of the function: <=, <, >=, >, !=, ==

## inversion\<T\>

- input pins: `in(T)`
- output pins: `out(T)`
- description: Inverts the value on `in`, i.e. outputs 1/`in`

## neg\<T\>

- input pins: `in(T)`
- output pins: `out(T)`
- valid types for T: INT, REAL, BOOL
- description: Negates the `in` value

## log

- input pins: `base(REAL)`, `in(REAL)`
- output pins: `out(REAL)`
- description: Outputs the logarithm of base `base` of `in`


## pow

- input pins: `base(REAL)`, `exp(REAL)`
- output pins: `out(REAL)`
- description: Outputs the exponentiation of `base` to the power of exponent `exp`

## sqrt

- input pins: `in(REAL)`
- output pins: `out(REAL)`
- description: Outputs the square root of `in`


## avg

- input pins: `in(REAL)`
- output pins: `out(REAL)`
- description: Computes a running average of last 10 input values. Outputs `nothing` in case any of the 10 values was `nothing`

## multiplexor\<T\>

- input pins: `in1(T)` ... `in32(T)`, `state(INT)`
- output pins: `out(T)`
- valid types for T: INT, REAL, BOOL
- description: Allows the choice of up to 32 values to forward based on the `state` input