// ************************************************************
// Header file for Agent's nervous system
// Madhavun Candadai Vasu
// Jun 07, 2016 - created
//
// TODO: Make atrributes private
// ************************************************************

#pragma once

#include "IzhiNN.h"
#include "IzhiNNArch.h"
#include <queue>

class NervousSystem {
private:
	IzhiNN cns;                                     //central nervous sytem
	IzhiNNArch* arch;
	int cnsSize, windowSize;
	queue<double> LMWindow, RMWindow;       // window of activity for each motor
	double LMWinsum, RMWinsum;                      // sum of activity in queue
public:
	NervousSystem() {};
	~NervousSystem() {};
	// TODO make these private and use getters - might slow down the execution though
	TVector<double> distSensorToInter, chemSensorToInter, interToLM, interToRM;  // weights for peripheral nervous system
	// setters for peripehral weights
	//void setDistSensorToInter(TVector<double> dToi);
	//void setChemSensorToInter(TVector<double> cToi);
	//void setInterToLM(TVector<double> iToLM);
	//void setInterToRM(TVector<double> iToRM);
	void setWindowSize(int winSize) { windowSize = winSize; };
	void setInputBias(double inputBias){cns.setInputBias(inputBias);};

	// init cns
	void initCNS(TVector<double> cnsParams,IzhiNNArch * architecture);
	double step(double stepSize, TVector<double> distanceInputs);
	void updateRate();
	void reset();
	double getStateV(int n) { return cns.getStateV(n); };

	void saveSpikes(const char* filename) { cns.saveSpikes(filename); };
}; 