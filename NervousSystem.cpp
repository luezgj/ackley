// ************************************************************
// Definition file for Agent's nervous system
// Madhavun Candadai Vasu
// Jun 07, 2016 - created
//
// ************************************************************

#include "NervousSystem.h"
#include "params.h"
#include "IzhiNNArch.h"

//TODO: setters for peripheral nervous system
// init cns
void NervousSystem::initCNS(TVector<double> cnsParams,IzhiNNArch * architecture) {
	int from;
	arch=architecture;

	cns.setNetworkVectors();
	cns.setArch(arch);
 
	int genIndex=1;

	for (from = 1; from <= (Ne/2); from++) //excitatory connections
	{
		for (int connection = 1; connection <= M; connection++)
		{
			cns.setWeights(from, connection, cnsParams[genIndex]);
			int simetrico = IzhiNNArch::sim(from - 1) + 1;
			cns.setWeights(simetrico, connection, cnsParams[genIndex]);
			genIndex++;
		}
	}

	for (from = Ne+1; from <= Ne+(Ni/2); from++) //inhibitory connections
	{
		for (int connection = 1; connection <= M; connection++)
		{
			cns.setWeights(from, connection, inhWeigth);
			cns.setWeights(IzhiNNArch::sim(from - 1) + 1, connection, inhWeigth);
		}
	}

	for (from = Ne+Ni+1; from <= Ne+Ni+ceil(float(numDistSensors)/2.0); from++) //sensory to hidden connections
	{
		for (int connection = 1; connection <= M; connection++)
		{
			cns.setWeights(from, connection, cnsParams[genIndex]);
			cns.setWeights(IzhiNNArch::sim(from - 1) + 1, connection, cnsParams[genIndex]);
			genIndex++;
		}
	}

	for (from = Ne+Ni+numDistSensors+1; from <= Ne+Ni+numDistSensors+(numMotorNeurons/2) ; from++) //hidden to motors connections
	{
		for (int connection = 1; connection <= M; connection++)
		{
			cns.setWeights(from, connection, cnsParams[genIndex]);
			cns.setWeights(IzhiNNArch::sim(from - 1) + 1, connection, cnsParams[genIndex]);
			genIndex++;
		}
	}

	// init states
	cns.refractoryInitNeuronStates();

	// init windows
	for (int i = 1; i <= windowSize; i++) LMWindow.push(0.);
	LMWinsum = 0;
	for (int i = 1; i <= windowSize; i++) RMWindow.push(0.);
	RMWinsum = 0;
}

double NervousSystem::step(double stepSize, TVector<double> distanceInputs) {
	//cout << "\t\t in NervousSystem step" << endl;

	// step izhiNN
	for (int mseg = 0; mseg < simulationMs; mseg++) {
		cns.eulerStep(stepSize, distanceInputs);
	}

	// update windows and return difference in rate
	updateRate();
	//cout << "\t\tRMWINSUM " << RMWinsum << " LMWINSUM " << LMWinsum;
	return (RMWinsum - LMWinsum) / windowSize;
}

void NervousSystem::updateRate() {
	double outSumL, outSumR;

	// compute weighted sum from inter
	outSumL = 0.; outSumR = 0.;
	int leftIndex=Ne+Ni+numDistSensors+1;   //index of 
	int rigthIndex=leftIndex+1;				//the neurons
	for (int mm = 1; mm <= M; mm++) { //all conections to motors
		outSumL += cns.getOutput(arch->getPre(leftIndex,mm)) * cns.getWeights(leftIndex,mm);
		outSumR += cns.getOutput(arch->getPre(rigthIndex,mm)) * cns.getWeights(rigthIndex,mm);
	}

	//update LM
	LMWinsum -= LMWindow.front();
	LMWindow.pop();
	LMWinsum += outSumL;
	LMWindow.push(outSumL);

	// update RM
	RMWinsum -= RMWindow.front();
	RMWindow.pop();
	RMWinsum += outSumR;
	RMWindow.push(outSumR);
}

void NervousSystem::reset() {
	// reset neuron states
	cns.refractoryInitNeuronStates();
	cns.resetInputs();
	cns.resetDerivative();
	cns.resetTime();

	// reset RateCode windows
	int s = LMWindow.size();
	for (int i = 1; i <= s; i++) {
		LMWindow.pop();
		LMWindow.push(0.);
		RMWindow.pop();
		RMWindow.push(0.);
	}
	LMWinsum = 0.;
	RMWinsum = 0.;
}