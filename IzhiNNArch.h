// ************************************************************
// Header file for Izhikevich Neural network Architecture
// Luciano Gimenez
// Nov 13, 2019 - created
//
// Other headers from Randall Beer
// ************************************************************

# pragma once

#include "VectorMatrix.h"
#include "random.h"
#include "params.h"
#include <iostream>
#include <math.h>

class IzhiNNArch {
private:
	int		post[N][M];				// indeces of postsynaptic neurons
								// En las conexiones postsinapticas de las salidas (que no tienen xq no se conecta nada para adelante), guardo las conexiones presinapticas
	short	delays_length[N][D];	// distribution of delays
	short	delays[N][D][M];		// arrangement of delays
	int		N_pre[N], I_pre[N][3 * M], D_pre[N][3 * M];	// presynaptic information
	float	a[N], d[N];				// neuronal dynamics parameters
public:
	//constructor
	IzhiNNArch();
	//destructor
	~IzhiNNArch();

	static int sim(int neuronNumber);

	float getA(int neuron){return a[neuron-1];};
	float getD(int neuron){return d[neuron-1];};

	int getPost(int from, int conNumber){return post[from-1][conNumber-1]+1;};
	int getPre(int from, int conNumber){return I_pre[from-1][conNumber-1]+1;};

	int getDelayLength(int neuron, int delay){return delays_length[neuron-1][delay-1];};
	int getDelay(int from, int conNumber){return I_pre[from-1][conNumber-1];};
	int getConnNumberByDelay(int from, int delay, int order/*numero de conexion con ese delay*/){return (delays[from-1][delay-1][order-1])+1;};

	bool loadFile(string fileName);
	void toFile(string fileName);

};