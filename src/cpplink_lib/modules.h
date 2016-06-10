#pragma once

#ifndef _CPPLINK_EMBEDDED_CODE_
    #include "maybe.h"
    #include "doubleequal.h"			  
#endif // !_CPPLINK_EMBEDDED_CODE_


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
        value = std::move(m);
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


struct BaseNet{
    virtual void step() = 0;
};

template <typename T>
struct Net : BaseNet {

    void addInputPin(InputPin<T>& in) {
        inputs.push_back(&in);
    }

    void setOutputPin(OutputPin<T>& out) {
        output = &out;
    }

    void step() {
        for (auto in : this->inputs)
            in->value = output->value;
        //output->value = Maybe<T>(); //nothing
    }
    
    Maybe<T> getValue() {
        return output->value;
    }

private:
    std::vector < InputPin<T>* > inputs;
    OutputPin<T>* output;
};


struct Module {
    virtual void step() = 0;
}; 

// ## Generators

template <typename T>
struct ModuleRand : Module {

    void step() {
        using GeneratorDistribution =  
                typename std::conditional<std::is_same<T, int64_t>::value,
                std::uniform_int_distribution<int>,
                std::uniform_real_distribution<double>>::type;

        T min_ = min.isValid()? min.getValue() : INT_MIN;
        T max_ = max.isValid()? max.getValue() : INT_MAX;
        GeneratorDistribution distr(min_, max_);

        out = distr(gen);
    }

    InputPin<T> min;
    InputPin<T> max;
    OutputPin<T> out;

private:
    std::default_random_engine gen{static_cast<unsigned long>(time(NULL))};
};

template <>
struct ModuleRand<bool> : Module {

    void step() {
        std::uniform_int_distribution<int> distr(0, 1);

        out = distr(gen);
    }

    OutputPin<bool> out;

private:
    std::default_random_engine gen{static_cast<unsigned long>(time(NULL))};
};


template <double (*F)(double)>
struct ModuleTrigo : Module {

    void step() {
        if (!amplitude.isValid() || !period.isValid() || doubleEqual(period.getValue(), 0)) {
            x = 0;
            out = Maybe<double>();
        } else {
            out = apply(amplitude.value, period.value,
                              [&,this](double amp, double per){
                double val = in.isValid()? in.getValue() : x;
                return amp*F((2*M_PI*val)/per); });
            x++;
        }
    }

    InputPin<double> amplitude;
    InputPin<double> period;
    InputPin<double> in;
    OutputPin<double> out;

private:
    double x = 0;
};

using ModuleSin = ModuleTrigo<sin>;
using ModuleCos = ModuleTrigo<cos>;


struct ModuleTan : Module {

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

struct ModuleSaw : Module {
 
    void step() {
        if (!period.isValid() || doubleEqual(period.getValue(), 0) || !amplitude.isValid()) {
            phase = 0;
            out = Maybe<double>();
            return;
        }
        phase += 1;
        while (phase > period.getValue())
            phase -= period.getValue();

        double q_period = period.getValue() / 4.0;
        if (phase < q_period) {
            out = phase / q_period * amplitude.getValue();
        }
        else if (phase < 3 * q_period) {
            out = amplitude.getValue() - (phase - q_period) / q_period * amplitude.getValue();
        }
        else {
            out = -amplitude.getValue() + (phase - 3 * q_period) / q_period * amplitude.getValue();
        }
    }
    InputPin<double> amplitude;
    InputPin<double> period;
    OutputPin<double> out;
private:
    double phase = 0;
};


struct ModuleLinear : Module {

    void step() {
        out = x++;
    }

    OutputPin<int64_t> out;

private:
    int64_t x = 0;
};


// ## Helpers

template <typename T, typename U>
struct ModuleConvert : Module {

    void step() {
        out = in.value | [](T i)-> Maybe<U>{ return static_cast<U>(i); };
    }

    InputPin<T> in;
    OutputPin<U> out;
};


template <typename T>
struct ModuleIdentity : Module {

    void step() {
        out.value = in.value;
    }

    InputPin<T> in;
    OutputPin<T> out;
};


template <typename T>
struct ModuleClamp : Module {

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
struct ModuleFunc : Module {

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
struct ModuleFuncThrows : Module {

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


struct FuncImpl {

    bool operator()(bool a, bool b) {
        return !(a && !b);
    }
};
using ModuleLogicImpl = ModuleFunc<bool, FuncImpl>;


struct FuncXnor {

    bool operator()(bool a, bool b) {
        return (a == b);
    }
};
using ModuleLogicXnor = ModuleFunc<bool, FuncXnor>;


struct FuncNand {

    bool operator()(bool a, bool b) {
        return (!a || !b);
    }
};
using ModuleLogicNand = ModuleFunc<bool, FuncNand>;


struct FuncNor {

    bool operator()(bool a, bool b) {
        return (!a && !b);
    }
};
using ModuleLogicNor = ModuleFunc<bool, FuncNor>;


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
struct ModuleInverse : Module {

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
struct ModuleNegate : Module {

    void step() {
        out = apply(in.value, [](T t)->T{ return -t; });
    }

    InputPin<T> in;
    OutputPin<T> out;
};

template <>
struct ModuleNegate<bool> : Module{

    void step() {
        out = apply(in.value, [](bool b)->bool{ return !b; });
    }

    InputPin<bool> in;
    OutputPin<bool> out;
};

struct ModuleLog : Module {

    void step() {
        out = apply(in.value, base.value, [](double val, double base)
                    ->double{ return log(val)/log(base); });
        if (std::isnan(out.getValue())) out = Maybe<double>();
    }

    InputPin<double> base;
    InputPin<double> in;
    OutputPin<double> out;
};

struct ModulePow : Module {

    void step() {
        out = apply(base.value, exp.value, [](double b, double e)
                    ->double{ return pow(b,e); });
    }

    InputPin<double> base;
    InputPin<double> exp;
    OutputPin<double> out;
};

struct ModuleSqrt : Module {

    void step() {
        out = apply(in.value, [](double d)
                    ->double{ return sqrt(d); });
        if (std::isnan(out.getValue())) out = Maybe<double>();
    }

    InputPin<double> in;
    OutputPin<double> out;
};

struct ModuleSignum : Module {

    void step() {
        out = apply(in.value, [](double d) ->int64_t{
            if (doubleEqual(d,0)) return 0;
            if (d<0) return -1;
            return 1;});
    }

    InputPin<double> in;
    OutputPin<int64_t> out;
};


struct ModuleAvg : Module {

    void step() {

        if (avgs.size()==10) avgs.erase(avgs.begin());
        avgs.push_back(in.value);

        if ( avgs.size() == 0 || std::any_of(avgs.begin(), avgs.end(), [](Maybe<double> d){ return !d.isValid(); }) )
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
struct ModuleMultiplexor : Module {

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
        return i < 32 && i >= 0;
    }
};


} //namespace cpplink

