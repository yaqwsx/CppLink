#pragma once

#include "maybe.h"
#include <memory> //unique_ptr
#include <vector>

template <typename T>
struct pin {
    Maybe<T> value;
};

template <typename T>
struct input_pin : public pin<T> {};

template <typename T>
struct output_pin : public pin<T> {};


template <typename T>
struct net {

    net(output_pin<T>* out, std::vector<input_pin<T>*> in) {
        output = out;
        inputs = in;
    }

    net(output_pin<T> *out, input_pin<T> *in) {
        output = out;
        inputs.push_back(in);
    }

    void step() {
        for (auto in : this->inputs)
            in->value = output->value;
    }

private:
    std::vector < input_pin<T>* > inputs;
    output_pin<T>* output;
};


template <typename T>
struct module_identity {

    module_identity()
        : in(new input_pin<T>()),
          out(new output_pin<T>())
    {}

    void step() {
        out->value = in->value;
    }

//private:
    std::unique_ptr<input_pin<T>> in;
    std::unique_ptr<output_pin<T>> out;
};


