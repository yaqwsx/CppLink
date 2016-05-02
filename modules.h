#pragma once

#include "maybe.h"
#include "doubleequal.h"

#include <vector>
#include <array>
#include <algorithm> //std::any_of
#include <random>
#include <ctime>
#include <climits>
#include <functional> //plus,minus..
//#define _USE_MATH_DEFINES
#include <cmath>


namespace cpplink {


template <typename T>
struct Pin {
    Maybe<T> value;

    T& getValue() { return value.value; }
    void setValue(T v) { value = v; }
    bool isValid() { return value.isValid(); }

    Pin& operator=(const Maybe<T>& m) {
        value = m;
        return *this;
    }

    Pin& operator=(Maybe<T>&& m) {
        value = m;
        return *this;
    }
};

template <typename T>
struct InputPin : public Pin<T> {
    using Pin<T>::operator =;
};

template <typename T>
struct OutputPin : public Pin<T> {
    using Pin<T>::operator =;
};



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
        out = Val;
    }

    OutputPin<T> out;
};


template <typename T>
struct ModuleRand {

    void step() {
        typedef typename std::conditional<std::is_same<T, int64_t>::value,
                std::uniform_int_distribution<int>,
                std::uniform_real_distribution<double>>::type Type;

        T min_ = min.isValid()? min.getValue() : INT_MIN;
        T max_ = max.isValid()? max.getValue() : INT_MAX;
        Type distr(min_, max_);

        for (size_t i=0; i<3; i++) distr(gen);
        out = distr(gen);
    }

    InputPin<T> min;
    InputPin<T> max;
    OutputPin<T> out;

private:
    std::default_random_engine gen{time(NULL)};
};

template <>
struct ModuleRand<bool> {

    void step() {
        std::uniform_int_distribution<int> distr(0, 1);
        for (size_t i=0; i<3; i++) distr(gen);
        out = distr(gen);
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
            out = Maybe<double>();
        } else {
            out = apply(amplitude.value, period.value,
                              [&,this](double amp, double per){ return amp*F((2*M_PI*x)/per); });
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
            out = Maybe<double>();
        } else {
            out = period.value | [&,this](double per) -> Maybe<double> {
                    if (doubleEqual(cos(x*M_PI/per),0)) return Maybe<double>();
                        else return tan(x*(M_PI/per)); };
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
        out = in.value | [](T i)-> Maybe<U>{ return std::move(static_cast<U>(i)); };
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


template <typename T>
struct ModuleClamp {

    void step() {
        out = apply(min.value, max.value, [&,this](T min, T max) ->T {
            T in_ = in.getValue();
            in_ = in_ < min ? min : in_;
            return in_ > max ? max : in_;
        });
    }

    InputPin<T> min;
    InputPin<T> max;
    InputPin<T> in;
    OutputPin<T> out;
};


// ## Functions

template <typename T, typename F>
struct ModuleFunc {

    void step() {
        out = apply(in1.value, in2.value, F());
    }

    InputPin<T> in1;
    InputPin<T> in2;
    OutputPin<T> out;
};


template <typename T>
using ModuleSum = ModuleFunc<T, std::plus<T>>;

template <typename T>
using ModuleDiff = ModuleFunc<T, std::minus<T>>;

template <typename T>
using ModuleMult = ModuleFunc<T, std::multiplies<T>>;


template <typename T, typename F>
struct ModuleFuncThrows {

    using Type = T(*)(T,T);
    Type func = [](T a, T b)->T{
            if (doubleEqual(b,0)) {
                throw std::invalid_argument("Division by 0");
            } else {
                return F()(a,b);
            }
        };

    void step() {
        out = apply(in1.value, in2.value, func);
    }

    InputPin<T> in1;
    InputPin<T> in2;
    OutputPin<T> out;
};

template <typename T>
using ModuleDiv = ModuleFuncThrows<T, std::divides<T>>;

template <typename T>
using ModuleMod = ModuleFuncThrows<T, std::modulus<T>>;


// ## logical

using ModuleLogicAnd = ModuleFunc<bool, std::logical_and<bool>>;
using ModuleLogicOr  = ModuleFunc<bool, std::logical_or<bool>>;
using ModuleLogicXor = ModuleFunc<bool, std::bit_xor<bool>>;



template <typename F>
struct ModuleLambda {
    void step() {
        out = apply(in1.value, in2.value, F());
    }

    InputPin<bool> in1;
    InputPin<bool> in2;
    OutputPin<bool> out;
};

struct FuncImpl {

    bool operator()(bool a, bool b) {
        return !(a && !b);
    }
};
using ModuleLogicImpl = ModuleLambda<FuncImpl>;


struct FuncXnor {

    bool operator()(bool a, bool b) {
        return (a == b);
    }
};
using ModuleLogicXnor = ModuleLambda<FuncXnor>;


struct FuncNand {

    bool operator()(bool a, bool b) {
        return (!a || !b);
    }
};
using ModuleLogicNand = ModuleLambda<FuncNand>;


struct FuncNor {

    bool operator()(bool a, bool b) {
        return (!a && !b);
    }
};
using ModuleLogicNor = ModuleLambda<FuncNor>;


// ## relational

template <typename T>
using ModuleLess = ModuleFunc<T, std::less<T>>;

template <typename T>
using ModuleLessEqual = ModuleFunc<T, std::less_equal<T>>;

template <typename T>
using ModuleGreater = ModuleFunc<T, std::greater<T>>;

template <typename T>
using ModuleGreaterEqual = ModuleFunc<T, std::greater_equal<T>>;

template <typename T>
using ModuleEqual = ModuleFunc<T, std::equal_to<T>>;

template <typename T>
using ModuleNotEqual = ModuleFunc<T, std::not_equal_to<T>>;


template <typename T>
struct ModuleInverse {

    using Type = double(*)(double);
    Type func = [](double a)-> double{
            if (doubleEqual(a,0)) {
                throw std::invalid_argument("Division by 0");
            } else {
                return 1/a;
            }
        };

    void step() {
        out = apply(in.value, func);
    }

    InputPin<T> in;
    OutputPin<double> out;
};

template <typename T>
struct ModuleNegate {
    void step() {
        out = apply(in.value, [](T& t){ return -t; });
    }

    InputPin<bool> in;
    OutputPin<bool> out;
};

template <>
struct ModuleNegate<bool> {
    void step() {
        out = apply(in.value, [](bool b){ return !b; });
    }

    InputPin<bool> in;
    OutputPin<bool> out;
};

struct ModuleLog {

    void step() {
        out = apply(in.value, base.value, [](double val, double base)
                    ->double{ return log(val)/log(base); });
        if (std::isnan(out.getValue())) out = Maybe<double>();
    }

    InputPin<double> base;
    InputPin<double> in;
    OutputPin<double> out;
};

struct ModulePow {

    void step() {
        out = apply(base.value, exp.value, [](double b, double e)
                    ->double{ return pow(b,e); });
    }

    InputPin<double> base;
    InputPin<double> exp;
    OutputPin<double> out;
};

struct ModuleSqrt {

    void step() {
        out = apply(in.value, [](double d)
                    ->double{ return sqrt(d); });
        if (std::isnan(out.getValue())) out = Maybe<double>();
    }

    InputPin<double> in;
    OutputPin<double> out;
};


struct ModuleAvg {

    void step() {
        if (avgs.size()==10) avgs.erase(avgs.begin());
        avgs.push_back(in.value);

        if ( std::any_of(avgs.begin(), avgs.end(), [](Maybe<double> d){ return !d.isValid(); }) )
            out = Maybe<double>();
        else {
            auto sum = std::accumulate(avgs.begin(), avgs.end(), Maybe<double>(0),
                            [](Maybe<double>& m, Maybe<double>& n) {
                                return apply(m, n, std::plus<double>()); } );
            out = sum.value / avgs.size();
        }

    }

    InputPin<double> in;
    OutputPin<double> out;
private:
    std::vector<Maybe<double>> avgs;
};


template <typename T>
struct ModuleMultiplexor {

    void step() {
        if (state.isValid() && inRange(state.getValue())) {
            out = vals[state.getValue()].value;
        } else {
            out = Maybe<T>();
        }
    }

    InputPin<int64_t> state;
    OutputPin<T> out;
    std::array<InputPin<T>, 32> vals;

private:
    bool inRange(int64_t i) {
        return i <= 32 && i >= 0;
    }
};


} //namespace cpplink

