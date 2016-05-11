#include <catch.hpp>
#include <string>
#include <typeinfo>
#include <type_traits>

#include "tests.h"
#include "../src/cpplink_lib/modules.h"

#include <iostream>

using namespace std;
using namespace cpplink;

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

    SECTION("chaining") {
        Maybe<string> m1 ("longlonglong");

        auto lengthy = [] (const string & s) -> Maybe<string> {
            string copyS = s;
            if  (s.length()>8) {
              return Maybe<string> (copyS);
            }
            return Maybe<string> (); // nothing
          };


        auto res1 = m1 | lengthy;

        REQUIRE_VALUE(m1, "longlonglong");

        auto res2 = m1 | [](const string & s) -> Maybe<string> { return Maybe<string>(); };
        REQUIRE_INVALID(res2);

        auto res3 = m1 | [](const string & s) -> Maybe<string> { return Maybe<string>(); } |
            [] (const string & s) -> Maybe<int>{ return Maybe<int> (s.length()); };
        REQUIRE_INVALID(res3);
    }


    auto f = [](int i){return i+1;};
    auto ff = [](int i) -> Maybe<int>{ return Maybe<int>(i+1); };
    auto fff = [](int i) -> Maybe<int>{ return ++i; };

    SECTION("mapping apply") {

        Maybe<int> c;
        auto res = c | f; // f once only; returns an int
        auto resA = apply(apply(c,f),f);
        apply(c,[](int i){return i+1;});
        REQUIRE_INVALID(resA); //
        REQUIRE_INVALID(c);
        Maybe<int> resi = c | fff; //with f would produce a "0" (default-constructor for int) then cast to Maybe
        REQUIRE_INVALID(resi)

        Maybe<string> s("abc");
        auto res2 = s | [](const string & ss)->Maybe<string>{ return ss+"de"; };
        //if return type ^ unspecified, returns a string
        REQUIRE_VALUE(res2, "abcde")

        auto sfunc = [](const string & ss){ return ss+"de"; };
        auto resS = apply(apply(apply(s,sfunc),sfunc),sfunc);
        REQUIRE_VALUE(resS, "abcdedede")

        auto resSn = apply(apply(apply(Maybe<string>(),sfunc),sfunc),sfunc);
        REQUIRE_INVALID(resSn)
    }

    SECTION("more chaining") {

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
        //REQUIRE_INVALID(mi.in.value); // input was moved
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

        ModuleRand<bool> mb;
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
        t.period.value = M_PI;
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
        ModuleClamp<int> c;
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

    SECTION("logical") {
        ModuleLogicImpl i;
        i.in1.value = true;
        i.in2.value = true;
        i.step();
        REQUIRE_VALUE(i.out.value, true);
        i.in2.value = false;
        i.step();
        REQUIRE_VALUE(i.out.value, false);
        i.in2.value = Maybe<bool>();
        i.step();
        REQUIRE_INVALID(i.out.value);

    }

    SECTION("xor") {
        ModuleLogicXor x;
        x.in1.value = false;
        x.in2.value = true;
        x.step();
        REQUIRE_VALUE(x.out.value, true);
    }

    SECTION("multiplexor") {
        ModuleMultiplexor<int> m;
        m.vals[0].value = 6;
        m.vals[1].value = Maybe<int>();
        m.vals[2].value = Maybe<int>(-3);
        m.state.value = 0;
        m.step();
        REQUIRE_VALUE(m.out.value, 6);
        m.state.value = 1;
        m.step();
        REQUIRE_INVALID(m.out.value);
        m.state.value = 2;
        m.step();
        REQUIRE_VALUE(m.out.value, -3);
    }

    SECTION("negate") {
        ModuleNegate<bool> b;
        b.in.value = true;
        b.step();
        REQUIRE_VALUE(b.out.value, false);
    }


    SECTION("negate2") {
        ModuleNegate<int64_t> b;
        b.in.value = 5;
        b.step();
        REQUIRE_VALUE(b.out.value, -5);
    }


    SECTION("inverse") {
        ModuleInverse<int> i;
        i.in.value = 5;
        i.step();
        REQUIRE(doubleEqual(i.out.getValue(), 0.2));
    }

    SECTION("log") {
        ModuleLog l;
        l.base = 4;
        l.in = 16;
        l.step();
        REQUIRE(doubleEqual(l.out.getValue(),2));
        l.base = 0;
        l.in = -2;
        l.step();
        REQUIRE_INVALID(l.out.value);
    }

    SECTION("sqrt") {
        ModuleSqrt s;
        s.in.value = 5;
        s.step();
        REQUIRE(doubleEqual(s.out.getValue(), 2.236067977499789696));
        s.in.value = -3;
        s.step();
        REQUIRE_INVALID(s.out.value);

        cout << std::is_same<int, int32_t>::value << '\n';
    }

    SECTION("avg") {
        ModuleAvg a;
        a.in = 5;
        a.step();
        REQUIRE_VALUE(a.out.value, 5);
        a.in = 10;
        a.step();
        REQUIRE_VALUE(a.out.value, 7.5);
    }

    SECTION("sgn") {
        ModuleSignum s;
        s.in = 0;
        s.step();
        REQUIRE_VALUE(s.out.value, 0);
        s.in = 56;
        s.step();
        REQUIRE_VALUE(s.out.value, 1);
        s.in = -2;
        s.step();
        REQUIRE_VALUE(s.out.value, -1);
    }
}

