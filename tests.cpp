#include "modules.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <string>

#include <iostream>

using namespace std;

#define REQUIRE_VALUE(m, res) \
    { REQUIRE(m.isValid()); \
      REQUIRE(m.value == res); }

#define REQUIRE_INVALID(m) \
    { REQUIRE(!m.isValid()); }


TEST_CASE("maybe") {

    SECTION("constructor") {
        Maybe<int> mi;
        REQUIRE_INVALID(mi);

        Maybe<int> m(5); //T&&
        REQUIRE_VALUE(m, 5);

        int i = 4;
        Maybe<int> ma(i); //T&
        REQUIRE_VALUE(ma, 4);

        Maybe<int> may(3);
        Maybe<int> mb(may); //Maybe&
        REQUIRE_VALUE(mb, 3);

        Maybe<int> mc(std::move(may)); //Maybe&&
        REQUIRE_VALUE(mc, 3);
        REQUIRE_INVALID(may); //invalidate after move
    }

    SECTION("assignment") {
        Maybe<double> md;
        Maybe<double> md2 = md;
        REQUIRE_INVALID(md2);

        Maybe<string> ms("s");
        Maybe<string> ms2 = ms;
        REQUIRE_VALUE(ms, "s");
        REQUIRE_VALUE(ms2, "s");

        Maybe<string> msm = std::move(ms);
        REQUIRE_VALUE(msm, "s");
        REQUIRE_INVALID(ms); //invalidate after move
        REQUIRE(ms.value == ""); //
    }

    auto f = [](int i){return i+1;};
    auto ff = [](int i) -> Maybe<int>{ return Maybe<int>(i+1); };
    auto fff = [](int i) -> Maybe<int>{ return ++i; };

    SECTION("mapping apply") {

        Maybe<int> c;
        auto resA = apply(apply(c,f),f);
        REQUIRE_INVALID(resA); //
        REQUIRE_INVALID(c);
        Maybe<int> resi = c | fff; //with f would produce a "0" (default-constructor for int) then cast to Maybe
        REQUIRE_INVALID(resi)

        Maybe<string> s("abc");
        auto res2 = s | [](const string & ss)->Maybe<string>{ return ss+"de"; };
        REQUIRE_VALUE(res2, "abcde")

        auto sfunc = [](const string & ss){ return ss+"de"; };
        auto resS = apply(apply(apply(s,sfunc),sfunc),sfunc);
        REQUIRE_VALUE(resS, "abcdedede")

        auto resSn = apply(apply(apply(Maybe<string>(),sfunc),sfunc),sfunc);
        REQUIRE_INVALID(resSn)
    }

    SECTION("chaining") {

        Maybe<int> z(0);
        auto res2 = z | fff | ff | ff | fff | ff | fff | fff;
        REQUIRE_VALUE(res2,7);

        auto inv = [](int i) -> Maybe<int>{ return Maybe<int>(); };
        Maybe<int> zz(0);
        auto res3 = zz | fff | ff | ff | inv | ff | fff | fff;
        REQUIRE_INVALID(res3);
    }
}



TEST_CASE("modules basic") {

    SECTION("pins") {

        input_pin<int> ip;
        output_pin<int> op;

        for(auto p : vector<pin<int>>{ip,op}) {
            REQUIRE_INVALID(p.value);
            p.value = Maybe<int>(2);
            REQUIRE_VALUE(p.value, 2);
        }
    }


    SECTION("net") {

        input_pin<int>* ip = new input_pin<int>();
        output_pin<int>* op = new output_pin<int>();

        net<int> n(op, {ip});

        op->value = 4;
        n.step();

        REQUIRE_VALUE(ip->value,4)

        for (auto el : vector<pin<int>*>{ip, op})
            delete el;
    }

    SECTION("module id") {

        module_identity<int> mi;
        module_identity<int> mi2;

        net<int> n1(mi.out.get(), mi2.in.get()); //MI => MI2
        net<int> n2(mi2.out.get(), mi.in.get()); //MI2 => MI
        mi.in->value = 3;
        mi2.in->value = 2;

        mi.step();
        mi2.step();

        REQUIRE_VALUE(mi.out->value, 3);
        REQUIRE_VALUE(mi2.out->value, 2);

        n1.step();
        n2.step();

        REQUIRE_VALUE(mi.in->value, 2);
        REQUIRE_VALUE(mi2.in->value, 3);

        mi2.in->value = Maybe<int>(); //nothing

        mi.step(); //2
        mi2.step(); //nothing

        REQUIRE_VALUE(mi.out->value, 2);
        REQUIRE_INVALID(mi2.out->value);

        n1.step();
        n2.step();

        REQUIRE_VALUE(mi2.in->value, 2);
        REQUIRE_INVALID(mi.in->value);
    }
}
