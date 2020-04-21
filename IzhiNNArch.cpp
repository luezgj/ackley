// ************************************************************
// Source file for Izhikevich Neural network Architecture
// Luciano Giménez
// Nov 13, 2019 - created
//
// ************************************************************
# pragma once
#include "IzhiNNArch.h"

//return the number of the symetric neuron
int IzhiNNArch::sim(int neuronNumber) {
	int number;
	int translate;
	int tam;
	if (neuronNumber < 0) return -1;
	if (neuronNumber < Ne) {
		translate = 0;
		tam = Ne;
	}
	else if (neuronNumber < Ne + Ni) {
		translate = Ne;
		tam = Ni;
	}
	else if (neuronNumber < Ne + Ni + numDistSensors) {
		translate = Ne + Ni;
		tam = numDistSensors;
	}
	else {
		translate = Ne + Ni + numDistSensors;
		tam = numMotorNeurons;
	}
	neuronNumber = neuronNumber - translate;
	if (neuronNumber < floor(tam / 2)) number = neuronNumber + ((floor(tam / 2) - neuronNumber) * 2);
	else number = neuronNumber - ((neuronNumber - (floor(tam / 2))) * 2);
	return ((tam % 2 == 0) ? number - 1 : number) + translate;
}

//Constructor
IzhiNNArch::IzhiNNArch() {
	int i, j, k, jj, dd, exists, r;
	
	for (i = 0; i < Ne; i++) a[i] = 0.02;// RS type
	for (i = Ne; i < N; i++) a[i] = 0.1; // FS type

	for (i = 0; i < Ne; i++) d[i] = 8.0; // RS type
	for (i = Ne; i < N; i++) d[i] = 2.0; // FS type

	//Conecta aleatoriamente las neuronas excitatorias
	cout << "ex";
	for (i = 0; i < Ne/2; i++) for (j = 0; j < M; j++) {
		do {
			exists = 0;		// avoid multiple synapses
			r =UniformRandomInteger(0,Ne+Ni-1);		// no connections to inputs or outputs
			if (r == i) exists = 1;									// no self-synapses
			for (k = 0; k < j; k++) if (post[i][k] == r) exists = 1;	// synapse already exists  
		} while (exists == 1);
		cout << i << "conectada con " << r << endl;
		cout << sim(i) << "conectada con " << sim(r) << endl;
		post[i][j] = r;
		post[sim(i)][j] = sim(r);
	}
	cout << "in";
	//Conecta aleatoriamente las neuronas inhibitorias
	for (i = Ne; i < Ne+(Ni/2); i++) for (j = 0; j < M; j++) {
		do {
			exists = 0;		// avoid multiple synapses
			r = UniformRandomInteger(0, Ne - 1);				// inh -> exc only
			if (r == i) exists = 1;									// no self-synapses
			for (k = 0; k < j; k++) if (post[i][k] == r) exists = 1;	// synapse already exists  
		} while (exists == 1);
		cout << i << "conectada con " << r << endl;
		cout << sim(i) << "conectada con " << sim(r) << endl;
		post[i][j] = r;
		post[sim(i)][j] = sim(r);
	}
	cout << "sen";
	//Conecta aleatoriamente las neuronas sensoras
	for (i = Ne + Ni ; i < Ne+ Ni + ceil(numDistSensors/2); i++) for (j = 0; j < M; j++) {
		do {
			exists = 0;		// avoid multiple synapses
			r = UniformRandomInteger(0, Ne - 1);						// sensor -> to any excitatory interneuron 
			if (r == i) exists = 1;									// no self-synapses
			for (k = 0; k < j; k++) if (post[i][k] == r) exists = 1;	// synapse already exists  
		} while (exists == 1);
		cout << i << "conectada con " << r << endl;
		cout << sim(i) << "conectada con " << sim(r) << endl;
		post[i][j] = r;
		post[sim(i)][j] = sim(r);
	}
	cout << "mot";
	//Conectar interneuronas a las salidas,  solo conecta exhitatorias->salida
	//se guardan presinapsis en la matriz de post
	for (i = Ne + Ni + numDistSensors; i < Ne + Ni + numDistSensors + (numMotorNeurons/2); i++) for (j = 0; j < M; j++) {
		do {
			exists = 0;		// avoid multiple synapses
			r = UniformRandomInteger(0,Ne - 1);		// no connections to inputs or outputs, only exhitatorias->salida
			for (k = 0; k < j; k++) if (post[i][k] == r) exists = 1;	// synapse already exists  
		} while (exists == 1);
		cout << i << "conectada con " << r << endl;
		cout << sim(i) << "conectada con " << sim(r) << endl;
		post[i][j] = r;
		post[sim(i)][j] = sim(r);
	}


	//Setea distribución de delays y setea la cantidad en delays[][][]    Solo de la capa oculta, los demás no tienen delays
	for (i = 0; i < cnsSize; i++) {
		short ind = 0;
		if (i < Ne) {
			for (j = 0; j < D; j++) {
				delays_length[i][j] = M / D;	// uniform distribution of exc. synaptic delays
				for (k = 0; k < delays_length[i][j]; k++)
					delays[i][j][k] = ind++;
			}
		}
		else {
			for (j = 0; j < D; j++) delays_length[i][j] = 0;
			delays_length[i][0] = M;			// all inhibitory delays are 1 ms
			for (k = 0; k < delays_length[i][0]; k++)
				delays[i][0][k] = ind++;
		}
	}

	//Busca las conexiones presinapticas,   ignora las de las motoras xq ya son presinapticas
	for (i = 0; i < N-numMotorNeurons; i++) {
		N_pre[i] = 0;
		for (j = 0; j < Ne; j++)
			for (k = 0; k < M; k++)
				if (post[j][k] == i) {		// find all presynaptic neurons 
					I_pre[i][N_pre[i]] = j;	// add this neuron to the list
					for (dd = 0; dd < D; dd++)	// find the delay
						for (jj = 0; jj < delays_length[j][dd]; jj++)
							if (post[j][delays[j][dd][jj]] == i) D_pre[i][N_pre[i]++] = dd;
				}
	}

	//Copia las conexiones presinapticas de las neu. motoras a las otras matrices
	for (i = N - numMotorNeurons; i < N; i++) {
		N_pre[i] = M;
		for (int m = 0; m < M;m++) {
			I_pre[i][m] = post[i][m];	// add this neuron to the list
			D_pre[i][m] = 0;			//D_pre no tiene sentido porque no tiene delays
		}
	}
}
//destructor
IzhiNNArch::~IzhiNNArch() {
	
}

//Cargar de un archivo la arquitectura
bool IzhiNNArch::loadFile(string fileName){
	ifstream file(fileName);

	if (!file.is_open()){
  	return false;
  }

	for (int nn = 0; nn < N ; nn++){
		for (int mm = 0; mm < M; mm++){
			file >> post[nn][mm];
		}
	}

	for (int nn = 0; nn < N ; nn++){
		for (int dd = 0; dd < D; dd++){
			file >> delays_length[nn][dd];
		}
	}

	for (int nn = 0; nn < N ; nn++){
		for (int dd = 0; dd < D; dd++){
			for (int mm = 0; mm < M; mm++){
				file>>delays[nn][dd][mm];
			}
		}
	}

	for (int nn = 0; nn < N; nn++){
		file>>N_pre[nn];
	}

	for (int nn = 0; nn < N; nn++){
		for (int mm = 0; mm < 3 * M; mm++){
			file>>I_pre[nn][mm];
		}
	}

	for (int nn = 0; nn < N; nn++){
		for (int mm = 0; mm < 3 * M; mm++){
			file>>D_pre[nn][mm];
		}
	}

	for (int nn = 0; nn < N; nn++){
		file>>a[nn];
	}

	for (int nn = 0; nn < N; nn++){
		file>>d[nn];
	}

	file.close();
	return true;
}

//guardar arquitectura en un archivo
void IzhiNNArch::toFile(string fileName){
	ofstream file(fileName);
	file << "post" << endl;
	for (int nn = 0; nn < N ; nn++){
		for (int mm = 0; mm < M; mm++){
			file<<post[nn][mm]<<endl;
		}
	}
	file << "delay_length" << endl;
	for (int nn = 0; nn < N ; nn++){
		for (int dd = 0; dd < D; dd++){
			file<<delays_length[nn][dd]<<endl;
		}
	}
	file << "delays" << endl;
	for (int nn = 0; nn < N ; nn++){
		for (int dd = 0; dd < D; dd++){
			for (int mm = 0; mm < M; mm++){
				file<< "[" << nn << "]" << "[" << dd << "]" << "[" << mm << "]" << delays[nn][dd][mm]<<endl;
			}
		}
	}
	file << "N_pre" << endl;
	for (int nn = 0; nn < N; nn++){
		file<<N_pre[nn]<<endl;
	}

	file << "I_pre" << endl;
	for (int nn = 0; nn < N; nn++){
		for (int mm = 0; mm < 3 * M; mm++){
			file<<I_pre[nn][mm]<<endl;
		}
	}

	file << "D_pre" << endl;
	for (int nn = 0; nn < N; nn++){
		for (int mm = 0; mm < 3 * M; mm++){
			file<<D_pre[nn][mm]<<endl;
		}
	}

	file << "a" << endl;
	for (int nn = 0; nn < N; nn++){
		file<<a[nn]<<endl;
	}

	file << "d" << endl;
	for (int nn = 0; nn < N; nn++){
		file<<d[nn]<<endl;
	}

	file.close();
}

