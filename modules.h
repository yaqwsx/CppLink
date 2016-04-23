#pragma once

#include "maybe.h"
#include <vector>
#include <random>
#include <ctime>
#include <climits>
#include <functional> //plus,minus..

# define PI       3.14159265358979323846
# define EPSILON  0.00000000000001

namespace cpplink {

bool doubleEqual(double, double);


template <typename T>
struct Pin {
    Maybe<T> value;

    T& getValue() { return value.value; }
    void setValue(T v) { value = v; }
    bool isValid() { return value.isValid(); }
};

template <typename T>
struct InputPin : public Pin<T> {};

template <typename T>
struct OutputPin : public Pin<T> {};



template <typename T>
struct Net {

    void addInputPin(InputPin<T>* in) {
        inputs.push_back(in);
    }

    void setOutputPin(OutputPin<T>* out) {
        output = out;
    }

    void step() {
        for (auto in : this->inputs)
            in->value = output->value;
        output->value = Maybe<T>(); //nothing
    }

private:
    std::vector < InputPin<T>* > inputs;
    OutputPin<T>* output;
};


// ## Generators

template <typename T, T Val>
struct ModuleConst {

    void step() {
        out.value = Val;
    }

    OutputPin<T> out;
};


template <typename T>
struct ModuleRand {

    void step() {
        typedef typename std::conditional<std::is_same<T, int>::value,
                std::uniform_int_distribution<int>,
                std::uniform_real_distribution<double>>::type Type;

        T min_ = min.isValid()? min.getValue() : INT_MIN;
        T max_ = max.isValid()? max.getValue() : INT_MAX;
        Type distr(min_, max_);

        for (size_t i=0; i<3; i++) distr(gen);
        out.value = distr(gen);
    }

    InputPin<T> min;
    InputPin<T> max;
    OutputPin<T> out;

private:
    std::default_random_engine gen{time(NULL)};
};


struct ModuleRandBool {

    void step() {
        std::uniform_int_distribution<int> distr(0, 1);
        for (size_t i=0; i<3; i++) distr(gen);
        out.value = distr(gen);
    }

    OutputPin<bool> out;

private:
    std::default_random_engine gen{time(NULL)};
};


template <double (*F)(double)>
struct ModuleTrigo {

    void step() {
        if (!amplitude.isValid() || !period.isValid() || doubleEqual(period.getValue(), 0)) {
            x = 0;
            out.value = Maybe<double>();
        } else {
            out.value = apply(amplitude.value, period.value,
                              [&,this](double amp, double per){ return amp*F((2*PI*x)/per); });
            x++;
        }
    }

    InputPin<double> amplitude;
    InputPin<double> period;
    OutputPin<double> out;

private:
    double x = 0;
};

using ModuleSin = ModuleTrigo<sin>;
using ModuleCos = ModuleTrigo<cos>;


struct ModuleTan {

    void step() {
        if (!period.isValid() || doubleEqual(period.getValue(),0)) {
            x = 0;
            out.value = Maybe<double>();
        } else {
            out.value = period.value | [&,this](double per) -> Maybe<double> {
                    if (doubleEqual(cos(x*PI/per),0)) return Maybe<double>();
                        else return tan(x*(PI/per)); };
            x++;
        }
    }

    InputPin<double> period;
    OutputPin<double> out;

private:
    double x = 0;
};


// ## Helpers

template <typename T, typename U>
struct ModuleConvert {

    void step() {
        out.value = in.value | [](T i)-> Maybe<U>{ return std::move(static_cast<U>(i)); };
    }

    InputPin<T> in;
    OutputPin<U> out;
};


template <typename T>
struct ModuleIdentity {

    void step() {
        out.value = std::move(in.value);
    }

    InputPin<T> in;
    OutputPin<T> out;
};


template <typename T, typename U>
struct ModuleClamp {

    void step() {
        out.value = apply(min.value,max.value,[&,this](T min, T max) ->U {
            U in_ = in.getValue();
            in_ = in_ < min ? min : in_;
            return in_ > max ? max : in_;
        });
    }

    InputPin<T> min;
    InputPin<T> max;
    InputPin<U> in;
    OutputPin<U> out;
};


// ## Functions

template <typename T, typename F>
struct ModuleFunc {

    void step() {
        out.value = apply(in1.value,in2.value,F());
    }

    InputPin<T> in1;
    InputPin<T> in2;
    OutputPin<T> out;
};

template <typename T>
using ModuleSum = ModuleFunc<T,std::plus<T>>;

template <typename T>
using ModuleDiff = ModuleFunc<T, std::minus<T>>;

template <typename T>
using ModuleMult = ModuleFunc<T, std::multiplies<T>>;


template <typename T, typename F>
struct ModuleFuncThrows {

    using Type = double(*)(T,T);
    Type func = [](T a, T b)->double{
            if (doubleEqual(b,0)) {
                throw std::invalid_argument("Division by 0");
            } else {
                return F()(a,b);
            }
        };

    void step() {
        out.value = apply(in1.value,in2.value,func);
    }

    InputPin<T> in1;
    InputPin<T> in2;
    OutputPin<double> out;
};

template <typename T>
using ModuleDiv = ModuleFuncThrows<T, std::divides<T>>;

template <typename T>
using ModuleMod = ModuleFuncThrows<T, std::modulus<T>>;



bool doubleEqual (double a, double b) {
   return (a - b < EPSILON) && (b - a < EPSILON);
}

} //namespace cpplink

