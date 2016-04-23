#include "modules.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <string>
#include <typeinfo>
#include <type_traits>

#include <iostream>

using namespace std;
using namespace cpplink;

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

        InputPin<int> ip;
        OutputPin<int> op;

        for(auto p : vector<Pin<int>>{ip,op}) {
            REQUIRE_INVALID(p.value);
            p.value = Maybe<int>(2);
            REQUIRE_VALUE(p.value, 2);
        }
    }


    SECTION("net") {

        InputPin<int> ip;
        OutputPin<int> op;

        Net<int> n;
        n.addInputPin(&ip);
        n.setOutputPin(&op);

        op.value = 4;
        n.step();

        REQUIRE_VALUE(ip.value,4)
    }

    SECTION("module id") {

        ModuleIdentity<int> mi;
        ModuleIdentity<int> mi2;

        Net<int> n1;
        n1.addInputPin(&mi2.in);
        n1.setOutputPin(&mi.out); //MI => MI2
        Net<int> n2;
        n2.addInputPin(&mi.in);
        n2.setOutputPin(&mi2.out); //MI2 => MI
        mi.in.value = 3;
        mi2.in.value = 2;

        mi.step();
        mi2.step();

        REQUIRE_VALUE(mi.out.value, 3);
        REQUIRE_INVALID(mi.in.value); // input was moved
        REQUIRE_VALUE(mi2.out.value, 2);

        n1.step();
        n2.step();

        REQUIRE_VALUE(mi.in.value, 2);
        REQUIRE_VALUE(mi2.in.value, 3);

        mi2.in.value = Maybe<int>(); //nothing

        mi.step(); //2
        mi2.step(); //nothing

        REQUIRE_VALUE(mi.out.value, 2);
        REQUIRE_INVALID(mi2.out.value);

        n1.step();
        n2.step();

        REQUIRE_VALUE(mi2.in.value, 2);
        REQUIRE_INVALID(mi.in.value);
    }
}

TEST_CASE("modules") {
    SECTION("const") {
        ModuleConst<bool,true> m;
        m.step();
        REQUIRE_VALUE(m.out.value, true);
    }

    SECTION("rand") {
        ModuleRand<int> mi;
        mi.min.value = -10;
        mi.max.value = 10;
        mi.step();

        ModuleRand<double> md;
        md.min.value = -2.5;
        md.max.value = 1000.99;
        md.step();
        cout << md.out.value.value << '\n';

        ModuleRandBool mb;
        mb.step();
        cout << boolalpha;
        cout << mb.out.value.value << '\n';
    }

    SECTION("sin") {
        ModuleSin s;
        s.amplitude.value = 3;
        s.period.value = 20;
        s.step();
        REQUIRE_VALUE(s.out.value,0)

        for(size_t i=0;i<5;i++) {
            s.step();
            REQUIRE((s.out.value.value >= -3 && s.out.value.value <= 3));
        }
        REQUIRE_VALUE(s.out.value, 3)
    }

    SECTION("tan") {
        ModuleTan tt;
        tt.period.value = 0;
        tt.step();
        REQUIRE_INVALID(tt.out.value)

        ModuleTan t;
        t.period.value = PI;
        t.step();
    }

    SECTION("convert") {
        ModuleConvert<int,double> c;
        c.in.setValue(7);
        c.step();
        REQUIRE_VALUE(c.out.value,7.0)
        REQUIRE(typeid(decltype(c.out.value)) == typeid(Maybe<double>));
    }

    SECTION("clamp") {
        ModuleClamp<int,double> c;
        c.min.setValue(3);
        c.max.setValue(9);
        c.in.setValue(11);
        c.step();
        REQUIRE_VALUE(c.out.value,9);
        c.in.setValue(-2);
        c.step();
        REQUIRE_VALUE(c.out.value,3);
    }

    SECTION("sum") {
        ModuleSum<int> a;
        a.in1.value = 2;
        a.in2.value = -5;
        a.step();
        REQUIRE_VALUE(a.out.value, -3);
    }

    SECTION("diff") {
        ModuleDiff<double> a;
        a.in1.value = 5.6;
        a.in2.value = 3.3;
        a.step();
        REQUIRE(doubleEqual(a.out.getValue(), 2.3));
    }

    SECTION("div") {
        ModuleDiv<int> a;
        a.in1.value = 5;
        a.in2.value = 0;
        REQUIRE_THROWS(a.step(););

        ModuleDiv<double> b;
        b.in1.value = 5;
        b.in2.value = 2;
        b.step();
        REQUIRE(doubleEqual(b.out.getValue(),2.5));

        ModuleDiv<double> c;
        c.in1.value = Maybe<double>();
        c.in2.value = 3;
        c.step();
        REQUIRE_INVALID(c.out.value);
    }
}
