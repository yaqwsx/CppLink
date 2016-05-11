#pragma once
#include <iostream>
#include <vector>

#define REQUIRE_VALUE(m, res) \
    { REQUIRE(m.isValid()); \
      REQUIRE(m.value == res); }

#define REQUIRE_INVALID(m) \
    { REQUIRE(!m.isValid()); \
}

namespace std {
template <class T, class A>
std::ostream& operator<<(std::ostream& o, const std::vector<T, A>& t) {
	for (const T& item : t) {
		o << item << "\n";
	}
	return o;
}
}