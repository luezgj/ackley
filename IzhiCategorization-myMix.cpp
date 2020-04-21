// ************************************************************
// Main for evolving categorizing Agent
// Madhavun Candadai Vasu
// Jun 20, 2016 - created
//
// ************************************************************
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <math.h>
#include "Agent.h"
#include "TSearch.h"
#include "Environment.h"
#include "IzhiNNArch.h"
#include "params.h"

// params
categEvalParams cEvalParams;

//Flags
int writeFlag = 0;

//Experiment variables indexs
int tourI;
int crossModeI;
int crossPointsAmI;
int elitFractI;
int popSizesI;
int mutationVarI;
int crossProbI;
int offSurvivPercI;

IzhiNNArch* arch = new IzhiNNArch();

//************
// Fitness
//************
void convertGenotypeToPhenotype(TVector<double> g, Agent& a) {
	//cout << "Converting genotype to phenotype" << endl;

	//double input_bias = MapSearchParameter(g[genotypeSize - 2], -10, 0);
	double input_bias = g[genotypeSize - 2];
	a.setInputBias(input_bias);

	// penultimate gene is rateCode
	//double rateCodeGain = MapSearchParameter(g[genotypeSize - 1], 200, 1200);
	double rateCodeGain = g[genotypeSize - 1];
	a.setRateCodeGain(rateCodeGain);

	// last gene is windowSize
	/*double windowSize = round(MapSearchParameter(g[genotypeSize], 5, 25)) / integStepSize;*/
	int windowSize = round(g[genotypeSize]);
	a.setWindowSize(windowSize);

	TVector<double> nsParams;
	nsParams.SetBounds(1, genotypeSize);

	int index = 1;
	for (int evNeuron = 0; evNeuron < evolvableNeurons; evNeuron++) {
		for (int connection = 0; connection < M; connection++) {
			nsParams[index] = g[index];
			index++;
		}
	}

	a.initNS(nsParams, arch);

	if (writeFlag == 1) {
		for (int i = 1; i <= nsParams.Size(); i++)
			cout << nsParams[i] << endl;
		cout << input_bias << endl;
		cout << rateCodeGain << endl;
		cout << windowSize << endl;
	}

	//cout << "Finished converting genotype to phenotype.." << endl;
}

double evaluateCateg(Agent& a) {
	//cout << "\tIn evaluateCateg" << endl;
	Environment env(DIAMETER, ENVWIDTH, OBJVEL, ENVHEIGHT, SENSORDISTANCE);
	double fitness = 0., fit1 = 0., fit2 = 0.;

	// vector of starting positions for object x-coordinate
	TVector<double> startingPositions;
	startingPositions.SetBounds(1, cEvalParams.numTrials * 2);
	for (int i = 1; i <= cEvalParams.numTrials; i++) {
		double posOffset = (i - 1) * (100. / (cEvalParams.numTrials - 1)) - 50.;
		startingPositions[i] = posOffset;                           // for isCircle trials
		startingPositions[i + cEvalParams.numTrials] = posOffset;   // for !isCircle trials
	}
	//cEvalParams.maxDistance = std::abs(startingPositions[1]);

	// externalInputs
	TVector<double> distanceInputs;
	distanceInputs.SetBounds(1, numDistSensors);
	distanceInputs.FillContents(0.);

	double agentPos = 0.;
	// object params
	double oY, oX;
	bool isCircle = 1;

	for (int trial = 1; trial <= cEvalParams.numTrials * 2; trial++) {
		//cout << "\tTrial-" << trial << endl;
		if (trial == cEvalParams.numTrials + 1) isCircle = !isCircle;

		// reset inputs
		distanceInputs.FillContents(0.);
		// reset agent
		a.reset();

		double t = integStepSize;
		oY = ENVHEIGHT;
		oX = startingPositions[trial];
		agentPos = 0.;
		// present object and step agent
		while (oY > 0.) {
			env.getDistanceInputs(distanceInputs, isCircle, agentPos, oX, oY);

			double offset = 0.;
			offset = a.step(integStepSize, distanceInputs);
			agentPos += offset;

			//cout << "\t\toY=" << oY << " oX=" << oX << " agentPos=" << agentPos << endl;
			if (writeFlag == 3) {
				cout << t << " " << isCircle << " " << oX << " " << oY << " " << agentPos << " " << offset / integStepSize << " ";
				for (int d = 1; d < numDistSensors; d++) {
					cout << distanceInputs[d] << " ";
				}
				for (int n = 1; n <= cnsSize; n++) {
					cout << a.getNeuronStateV(n) << " ";
				}
				cout << endl;
			}
			// Update Object's position
			oY = oY - OBJVEL * integStepSize;
			t += integStepSize;
		}

		// Finished presenting object
		double distance = std::abs(agentPos - oX);

		if (distance > cEvalParams.maxDistance) { distance = cEvalParams.maxDistance; }
		distance = distance / cEvalParams.maxDistance;
		//cout << "\tFi1 " << fit1 << "fit2 " << fit2 << endl;
			// Compute Fitness
		if (isCircle) { fit1 += (1 - distance); }
		else { fit2 += distance; }

		if(writeFlag==3){
			string spikesName;
			if (isCircle)
				spikesName="spikes_c"+ to_string(trial) + ".dat";
			else
				spikesName = "spikes_l"+ to_string(trial) + ".dat";

			a.saveSpikes(spikesName.c_str());
		}
	}

	//fitness = (fit1/cEvalParams.numTrials)*(fit2/cEvalParams.numTrials);
	fitness = (fit1 + fit2) / (cEvalParams.numTrials * 2);
	// Average over trials
	return fitness;///(cEvalParams.numTrials*2);
}


double evaluateFitness(TVector<double>& g, RandomState& rs) {
	//cout << "evaluateFitness" << endl;
	// init
	double fitCateg = 0.;

	Agent a;
	convertGenotypeToPhenotype(g, a);
	//cout << "converted Genotype To Phenotype" << endl;
	a.reset();
	fitCateg = evaluateCateg(a);

	return fitCateg;
}


/* ACKLEY*/
/*
double evaluateFitness(TVector<double>& g, RandomState& rs) {
	//cout << "evaluateFitness" << endl;
	// init
	double values[AckleyDimensions];
	for (int i = 1; i <= AckleyDimensions;i++) {
		values[i - 1] = g[i];   //MapSearchParameter(g[i], -32.0, 32.0); 
	}
	double ackley;

	const double PI = 3.141592653589793238463;

	double prom = 0;
	double cosProm = 0;

	for (int i = 0; i < AckleyDimensions;i++) {
		prom += values[i]* values[i];
		cosProm += std::cos(2 * PI * values[i]);
	}

	prom /= AckleyDimensions;
	cosProm /= AckleyDimensions;

	ackley = -20 * exp(-0.2 * std::sqrt(prom)) - std::exp(cosProm) + 22.718282;
	
	return ackley;
}
*/
string getPrefix() {
	/*return "T" + to_string(tourI)
		+ "_CM" + to_string(crossModeI)
		+ "_CP" + to_string(crossPointsAmI)
		+ "_EF" + to_string(elitFractI)
		+ "_PS" + to_string(popSizesI)
		+ "_MV" + to_string(mutationVarI)
		+ "_CP" + to_string(crossProbI)
		+ "_SP" + to_string(offSurvivPercI);
	*/		
	return "";
}

//*************
// Display
//*************

void EvolutionaryRunDisplay(int generation, double bestPerf, double avgPerf, double perfVar, double executionTime) {
	cout << generation << " " << bestPerf << " " << avgPerf << " " << perfVar << " ";
	cout << executionTime << "s";
	cout << endl;
}

void ResultsDisplay(TSearch& s) {

	TVector<double> bestVector;
	ofstream BestIndividualFile;

	bestVector = s.BestIndividual();
	BestIndividualFile.open("./"+ getPrefix() + "best.gen.dat");
	BestIndividualFile << setprecision(32);
	BestIndividualFile << bestVector << endl;
	BestIndividualFile.close();

	// write out best phenotype
	ofstream bestPhenotypeFile;
	bestPhenotypeFile.open("./"+ getPrefix() + "/best.phen.dat");
	cout.rdbuf(bestPhenotypeFile.rdbuf());
	writeFlag = 1;
	Agent a;
	convertGenotypeToPhenotype(bestVector, a);
	bestPhenotypeFile.close();

	/*ofstream bestBehaviorFile;
	bestBehaviorFile.open("./bestBehavior.dat");
	cout.rdbuf(bestBehaviorFile.rdbuf());
	writeFlag = 2;
	RandomState rs;
	evaluateFitness(bestVector, rs);
	bestBehaviorFile.close();*/

	ofstream bestCategFile;
	bestCategFile.open("./" + getPrefix() + "/bestCateg.dat");
	cout.rdbuf(bestCategFile.rdbuf());
	writeFlag = 3;
	//cEvalParams.numTrials = 24;
	RandomState rs;
	/*cout << */evaluateFitness(bestVector, rs);
	bestCategFile.close();
}

//CAPAZ QUE NO ANDA PORQUE METÍ ESTO EN UNA FUNCION ¿?
void execute(TSearch & s ) {
	// redirect standard output to a file
	ofstream evolfile;
	evolfile.open("./"+ getPrefix() + "fitness.dat");
	cout.rdbuf(evolfile.rdbuf());

	// start evolution
	s.ExecuteSearch();

	//evolfile.close();

	// back to old buf
	cout.rdbuf();
}

int main() {

	int tourSizes[5] = {4,7,10,15,20};
	TCrossoverMode crossModes[2] = { N_POINT, UNIFORM };
	int crossPointsAmouts[4] = { 2,4,7,MaxCrossPoints };
	float elitFractions[4] = {0.05, 0.1, 0.2, 0.4};
	int popSizes[3] = { 100, 150, 200 };
	float mutationVariances[3] = {0.05, 0.07 ,0.1};
	float crossProbs[4] = { 0.6, 0.7, 0.75, 0.8 };
	float offSurvivPerc[4] = { 0.5, 0.6, 0.7, 0.8 };


	//arch->loadFile("./architecture.dat");

	TSearch s(genotypeSize);
	// config search
	long randomseed = static_cast<long>(time(NULL));
	//long randomseed = 233456;
	s.SetRandomSeed(randomseed);
	s.SetEvaluationFunction(evaluateFitness);
	////cout << "RandomSeed - " << randomseed << endl;

	//write params to file
	ofstream paramsFile;
	paramsFile.open("./paramsFile.dat");
	paramsFile << randomseed << endl;
	paramsFile << popSize << endl;
	paramsFile << maxGens << endl;
	paramsFile << mutationVariance << endl;
	paramsFile << crossoverProb << endl;
	paramsFile << cnsSize << endl;
	//paramsFile << windowSize << endl;
	paramsFile.close();

	// display functions
	s.SetPopulationStatisticsDisplayFunction(EvolutionaryRunDisplay);
	s.SetSearchResultsDisplayFunction(ResultsDisplay);

	s.SetSelectionMode(RANK_BASED_GIMENEZ);
	s.SetReproductionMode(GENETIC_ALGORITHM_GIMENEZ);
	s.SetParentSelectionMode(TOURNAMENT);
	s.SetMaxGenerations(maxGens);
	s.SetMaxExpectedOffspring(1.1);
	s.SetElitistFraction(0.1);
	s.SetSearchConstraint(1);
	s.SetCheckpointInterval(0);
	s.SetReEvaluationFlag(0);

	/*
	for (tourI=0 ; tourI<5 ; tourI++) {
		for (crossModeI = 0; crossModeI < 2; crossModeI++) {
			for (elitFractI = 0; elitFractI < 4; elitFractI++) {
				for (popSizesI = 0; popSizesI < 3; popSizesI++) {
					for (mutationVarI = 0; mutationVarI < 3; mutationVarI++) {
						for (crossProbI = 0; crossProbI < 4; crossProbI++) {
							for (offSurvivPercI = 0; offSurvivPercI < 4; offSurvivPercI++) {
	*/

		
								//cout << "Genotype Size - " << genotypeSize << endl;

								s.SetElitistFraction(0.4/*elitFractions[elitFractI]*/);
								s.SetPopulationSize(120/*popSizes[popSizesI]*/);
								s.SetMutationVariance(0.05/*mutationVariances[mutationVarI]*/);
								s.SetCrossoverProbability(0.75/*crossProbs[crossProbI]*/);
								s.setOffspringPercSuvirv(0.5/*offSurvivPerc[offSurvivPercI]*/);

								s.setTournSize(5/*tourSizes[tourI]*/);
								s.setParentTournSize(5/*tourSizes[tourI]*/);
								s.SetCrossoverMode(UNIFORM/*crossModes[crossModeI]*/);


								/*if (crossModeI == 0) {
									for (crossPointsAmI = 0; crossPointsAmI < 4; crossPointsAmI++) {
										s.setCrossPointsAmount(crossPointsAmouts[crossPointsAmI]);
										execute(s);
									}
								}
								else {*/
									execute(s);
								//}

							/*}
						}
					}
				}
			}
		}
	}*/
	//cout << "Finished Execution" << endl;
	
}