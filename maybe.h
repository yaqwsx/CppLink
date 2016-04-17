#pragma once
//-std=c++1y

#include <utility> //move
#include <type_traits> //decltype


template <typename T>
struct Maybe {
    T value;
    using value_type = T;

    Maybe() : valid(false) {}
    Maybe(T& val) : value(val), valid(true) {}
    Maybe(T&& val) : value(std::move(val)), valid(true) {}

    Maybe(const Maybe& m) : valid(m.valid) {
        if (m.valid) {
            value = m.value;
        }
    }

    Maybe(Maybe&& m) : valid(m.valid) {
        if (m.valid) {
            value = std::move(m.value);
            m.valid = false;
        }
    }

    Maybe& operator=(const Maybe& m) {
        valid = m.valid;
        if (m.valid) {
            value = m.value;
        }
        return *this;
    }

    Maybe& operator=(Maybe&& m) {
        valid = m.valid;
        if (m.valid) {
            value = std::move(m.value);
            m.valid = false;
        }
        return *this;
    }

    bool isValid() {
        return valid;
    }

private:
    bool valid;
};



template<typename T, typename F> //chaining
auto operator|(Maybe<T> m, F&& f) { //>>= :: (m a) -> (a -> m b) -> (m b)      ||      (m a) -> (a -> b) -> (b)

    using return_value = decltype(f(T()));

    if (m.isValid()) {
        return (std::forward<F>(f)(m.value)); //just
    }
    return return_value(); //nothing
}


template<typename T, typename F> // :: (m a) -> (a -> b) -> (m b)
Maybe<typename std::result_of<F(T)>::type>
apply(Maybe<T> m, F&& f) {

    if (m.isValid()) {
        return (std::forward<F>(f)(m.value)); //just
    }

    return Maybe<decltype(f(T()))>(); //nothing
}






