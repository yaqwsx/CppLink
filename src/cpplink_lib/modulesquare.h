#pragma once

#ifndef _CPPLINK_EMBEDDED_CODE_
    #include "modules.h"      
#endif // !_CPPLINK_EMBEDDED_CODE_

/*
       /period/-- Sin -- Sgn -- Convert(int->double) -- Mult -- Sum --/out/
       /ampl=1/---/                                    /        /
                                                      /        /
       /ampl/----------------------------------------/        /
       /mid/-------------------------------------------------/
*/


namespace cpplink {

struct ModuleSquare : Module {
    
    ModuleSquare() { //wiring
            n1.addInputPin(&sig.in);
            n1.setOutputPin(&sin.out);
            n2.addInputPin(&mu.in1);
            n2.setOutputPin(&conv.out);
            n3.addInputPin(&su.in1);
            n3.setOutputPin(&mu.out);
            n4.addInputPin(&conv.in);
            n4.setOutputPin(&sig.out);
            sin.amplitude = 1;
            mid = 0;
    }

    void step() {
        for(unsigned i=0; i<modules.size(); i++) {
            modules[i]->step();
            for(auto n: nets) {
                n->step();
            }
        }
    }

    InputPin<double>& period = sin.period;
    InputPin<double>& ampl = mu.in2;
    InputPin<double>& mid = su.in2;
    OutputPin<double>& out = su.out;

    std::vector<Module*> modules{&sin, &sig, &conv, &mu, &su};
    std::vector<BaseNet*> nets{&n1, &n2, &n3, &n4};

private:
    ModuleSignum sig;
    ModuleSin sin;
    ModuleSum<double> su;
    ModuleMult<double> mu;
    ModuleConvert<int64_t, double> conv;
    Net<double> n1; // Sin -> Sgn
    Net<double> n2; // Convert -> Mult
    Net<double> n3; // Mult -> Sum
    Net<int64_t> n4; // Sgn -> Convert
};

}

