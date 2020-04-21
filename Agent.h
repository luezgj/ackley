// ************************************************************
// Header file for Agent
// Madhavun Candadai Vasu
// Jun 06, 2016 - created
//
// ************************************************************
#pragma once
#include "NervousSystem.h"
#include "IzhiNNArch.h"

class Agent{
    private:
        double rateCodeGain;
        NervousSystem ns; // nervous system
    public:
        Agent(){rateCodeGain = 750;};
        ~Agent(){};
        //void senseChemInputs(TVector<double> chemInputs);
        void initNS(TVector<double> nsParams,IzhiNNArch * arch);
        double step(double stepSize, TVector<double> distanceInputs); // one step of the agent's NS and body (same time scale?) and return offset in position
        void reset();
        double getNeuronStateV(int n){return ns.getStateV(n);};
        void setRateCodeGain(double rg){rateCodeGain = rg;};
        void setWindowSize(int ws){ns.setWindowSize(ws);};
        void setInputBias(double inputBias){ns.setInputBias(inputBias);};

		void saveSpikes(const char* filename) { ns.saveSpikes(filename); };
};