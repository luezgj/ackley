// ************************************************************
// Header file for Izhikevich Neural network
// Madhavun Candadai Vasu
// May 12, 2016 - created
//
// Other headers from Randall Beer
// ************************************************************

# pragma once

#include "VectorMatrix.h"
#include "random.h"
#include "IzhiNNArch.h"
#include "params.h"
#include <iostream>
#include <math.h>

class IzhiNN {
private:
	int t;											// current time step of the network
	int size;                   // number of excitatory, inhibitory, size
	//Borré a b c d (tengo a y d en la arquitectura)
	TVector<double> v, u, vbuf, ubuf;      // states and temp buffers

	IzhiNNArch* arch;
	
	TVector<double> inputs;             // external input
	TVector<int> outputs, prevOutputs;   // outputs are 0 or 1

	float	S[N];
	float	I[N];

	float	s[N][M], sd[N][M];		// matrix of synaptic weights and their derivatives
	float* s_pre[N][3 * M], * sd_pre[N][3 * M];		// presynaptic weights

	float input_bias;                //in range [-10,0]   rango lo maneja el traductor gen->agente

	int		N_firings;				// the number of fired neurons 
	int		firings[N_firings_max][2]; // indeces and timings of spikes
public:
	//constructor
	IzhiNN();
	//destructor
	~IzhiNN();

	//initializer
	void setNetworkVectors(); // defined in cpp file
	void setPreWeigths(); 
	void setArch(IzhiNNArch * architecture){arch=architecture;};

	void setInputBias(double inputBias){input_bias=inputBias;};
	
	//getters and setters - states
	void setStateV(int neuronIndex, double value) { v[neuronIndex] = value; };
	double getStateV(int neuronIndex) { return v[neuronIndex]; };
	void setStateU(int neuronIndex, double value) { u[neuronIndex] = value; };
	double getStateU(int neuronIndex) { return u[neuronIndex]; };

	//getters and setters - inputs/outputs
	void setInput(int neuronIndex, double inputVal) { inputs[neuronIndex] = inputVal; };
	double getInput(int neuronIndex) { return inputs[neuronIndex]; };
	// cannot set output directly; only via v
	//void setOutput(int neuronIndex, double outputVal){outputs[neuronIndex] = outputVal;};
	double getOutput(int neuronIndex) { return outputs[neuronIndex]; };
	
	//getters and setters - weights
	void setWeights(int from, int nConection, double val) { s[from-1][nConection-1] = val; };
	double getWeights(int from, int nConection) { return s[from-1][nConection-1]; };

	// network dynamics
	void eulerStep(double stepSize, TVector<double> externalInputs);
	void randomInitNeuronStates(RandomState& rs);
	void refractoryInitNeuronStates();
	void randomInitNetwork();


	//reseters
	void resetInputs();
	void resetDerivative();
	void resetTime(){t=0;};

	void saveSpikes(const char * filename);
};