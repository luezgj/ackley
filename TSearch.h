// *****************************
// "Evolutionary" search classes
// *****************************

// Uncomment the following line to enable multithreading
#define THREADED_SEARCH
#define THREAD_COUNT 8

#pragma once

#include "VectorMatrix.h"
#include "random.h"
#include "Chronometer.hpp"
#include "params.h"
#ifdef THREADED_SEARCH
#include <pthread.h>
#endif

using namespace std;


// A utility function for clipping a double to lie within the interval [MIN,MAX]
/*
inline double clip(double x, double min, double max)
{
	double temp;

	temp = ((x > min) ? x : min);
	return (temp < max) ? temp : max;
}*/
double clip(double x, double min, double max);


// A utility function for mapping from search parameters to model parameters,
// with the result optionally clipped to the given range.

inline double MapSearchParameter(double x, double min, double max,
	double clipmin = -1.0e99, double clipmax = 1.0e99)
{
	double m = (max - min) / (MaxSearchValue - MinSearchValue);
	double b = min - m * MinSearchValue;

	return clip(m * x + b, clipmin, clipmax);
}


// A utility function for mapping from model parameters to search parameters

inline double InverseMapSearchParameter(double x, double min, double max)
{
	double m = (MaxSearchValue - MinSearchValue) / (max - min);
	double b = MinSearchValue - m * min;

	return m * x + b;
}


// *******************************
// The TSearch class declaration
// *******************************

enum TSelectionMode { FITNESS_PROPORTIONATE, RANK_BASED, RANK_BASED_GIMENEZ };    // Supported selection modes
enum TReproductionMode { HILL_CLIMBING, GENETIC_ALGORITHM_CANDADAI, GENETIC_ALGORITHM_GIMENEZ }; // Supported reproduction modes
enum TCrossoverMode { UNIFORM, TWO_POINT, N_POINT };                  // Supported crossover modes
enum TParentSelectionMode { SUS, TOURNAMENT };                  // Supported crossover modes

class TSearch {
public:
	TVector<RandomState> RandomStates;
	double EvaluateVector(TVector<double>& Vector, RandomState& rs);
	// The constructor
	TSearch(int vectorSize = 0, double (*EvalFn)(TVector<double>&, RandomState&) = NULL);
	// The destructor
	~TSearch();
	// Basic Accessors
	int VectorSize(void) { return vectorSize; };
	void SetVectorSize(int NewSize);
	void SetRandomSeed(long seed) { rs.SetRandomSeed(seed); };
	// GA Parameters Accessors
	void setOffspringPercSuvirv(float percentage) { OffspringPercentageSurvivors = percentage; };
	void setTournSize(int tSize) { tournSize = tSize; };
	void setParentTournSize(int tSize) { parentTournSize = tSize; };
	void setCrossPointsAmount(int pointsAmount) {crossPointsAmount = pointsAmount; };
	// Search Mode Accessors
	TSelectionMode SelectionMode(void) { return SelectMode; };
	void SetSelectionMode(TSelectionMode newmode) { SelectMode = newmode; };
	TReproductionMode ReproductionMode(void) { return RepMode; };
	void SetReproductionMode(TReproductionMode newmode) { RepMode = newmode; };
	TCrossoverMode CrossoverMode(void) { return CrossMode; };
	void SetCrossoverMode(TCrossoverMode newmode) { CrossMode = newmode; };
	TParentSelectionMode ParentSelectionMode(void) { return ParentSelectMode; };
	void SetParentSelectionMode(TParentSelectionMode newmode) { ParentSelectMode = newmode; };
	// Search Parameter Accessors
	int PopulationSize(void) { return Population.Size(); };
	void SetPopulationSize(int NewSize);
	int MaxGenerations(void) { return MaxGens; };
	void SetMaxGenerations(int NewMax);
	double ElitistFraction(void) { return EFraction; };
	void SetElitistFraction(double NewFraction);
	double MaxExpectedOffspring(void) { return MaxExpOffspring; };
	void SetMaxExpectedOffspring(double NewVal);
	double MutationVariance(void) { return MutationVar; };
	void SetMutationVariance(double NewVariance);
	double MutationInputBiasVariance(void) { return InputBiasMutationVar; };
	void SetMutationInputBiasVariance(double NewVariance);
	double CrossoverProbability(void) { return CrossProb; };
	void SetCrossoverProbability(double NewProb);
	TVector<int>& CrossoverTemplate(void) { return crossTemplate; };
	void SetCrossoverTemplate(TVector<int>& NewTemplate);
	TVector<int>& CrossoverPoints(void) { return crossPoints; };
	void SetCrossoverPoints(TVector<int>& NewPoints);
	TVector<int>& SearchConstraint(void) { return ConstraintVector; };
	void SetSearchConstraint(TVector<int>& Constraint);
	void SetSearchConstraint(int Flag);
	int ReEvaluationFlag(void) { return ReEvalFlag; };
	void SetReEvaluationFlag(int flag) { ReEvalFlag = flag; };
	double CheckpointInterval(void) { return CheckpointInt; };
	void SetCheckpointInterval(int NewFreq);
	// Function Pointer Accessors
	void SetEvaluationFunction(double (*EvalFn)(TVector<double>& v, RandomState& rs))
	{
		EvaluationFunction = EvalFn;
	};
	void SetBestActionFunction(void (*BestFn)(int Generation, TVector<double>& v))
	{
		BestActionFunction = BestFn;
	};
	void SetPopulationStatisticsDisplayFunction(void (*DisplayFn)(int Generation, double BestPerf, double AvgPerf, double PerfVar, double executionTime))
	{
		PopulationStatisticsDisplayFunction = DisplayFn;
	};
	void SetSearchTerminationFunction(int (*TerminationFn)(int Generation, double BestPerf, double AvgPerf, double PerfVar))
	{
		SearchTerminationFunction = TerminationFn;
	};
	void SetSearchResultsDisplayFunction(void (*DisplayFn)(TSearch& s))
	{
		SearchResultsDisplayFunction = DisplayFn;
	};
	// Status Accessors
	int Generation(void) { return Gen; };
	TVector<double>& Individual(int i) { return Population(i); };
	double Fitness(int i) { return fitness(i); };
	double Performance(int i) { return Perf(i); };
	double BestPerformance(void) { return BestPerf; };
	TVector<double>& BestIndividual(void) { return bestVector; };
	// Control
	void InitializeSearch(void);
	void ExecuteSearch(void);
	void ResumeSearch(void);
	// Input and output
	void WriteCheckpointFile(void);
	void ReadCheckpointFile(void);
	//friend ostream& operator<<(ostream& os, TSearch& s);
		//friend istream& operator>>(istream& is, TSearch& s);

private:
	// Helper Methods
	void DoSearch(int ResumeFlag);
	int EqualVector(TVector<double>& v1, TVector<double>& v2)
	{
		if (v1.Size() != v2.Size()) return 0;
		if (v1.LowerBound() != v2.LowerBound()) return 0;
		if (v1.UpperBound() != v2.UpperBound()) return 0;
		for (int i = v1.LowerBound(); i <= v1.UpperBound(); i++)
			if (v1[i] != v2[i]) return 0;
		return 1;
	};
	void printPerf(TVector<double>& performances);
	void RandomizeVector(TVector<double>& Vector);
	void RandomizePopulation(void);
	friend void* EvaluatePopulationRange(void* arg, TVector<TVector<double>>& population, TVector<double>& performances);
	void EvaluatePopulation(TVector<TVector<double>>& population, TVector<double>& performances, int start = 1);
	void SortPopulation(TVector<TVector<double>>& population, TVector<double>& perf);
	void UpdatePopulationFitness(TVector<TVector<double>>& population, TVector<double>& perf, TVector<double>& fitn,bool sorted=false);
	void ReproducePopulationHillClimbing(void);
	void ReproducePopulationGeneticAlgorithm(void);
	void ReproducePopulationGeneticAlgorithmGimenez(void);
	void MutateVector(TVector<double>& Vector);
	void MutateVectorGimenez(TVector<double>& Vector);
	void UniformCrossover(TVector<double>& v1, TVector<double>& v2);
	void TwoPointCrossover(TVector<double>& v1, TVector<double>& v2);
	void NPointCrossover(TVector<double>& v1, TVector<double>& v2);
	void SelectNextPopulation(int eliteCount, TVector<TVector<double> > &parents, TVector<TVector<double> > &offsprings);
	void PrintPopulationStatistics(void);
	void ReproducePopulation(void);
	void UpdatePopulationStatistics(void);
	void DisplayPopulationStatistics(void);
	int SearchTerminated(void);
	void DisplaySearchResults(void);

	// Internal State
	float OffspringPercentageSurvivors = 0.75;
	int tournSize;
	int parentTournSize;
	int crossPointsAmount;
	RandomState rs;
	int Gen;
	double GenTime;
	int SearchInitialized;
	TVector<TVector<double> > Population;
	TVector<double> Perf;
	TVector<double> fitness;
	int UpdateBestFlag;
	TVector<double> bestVector;
	double BestPerf;
	double MinPerf, MaxPerf, AvgPerf, PerfVar;
	// Search Modes
	TSelectionMode SelectMode;
	TReproductionMode RepMode;
	TCrossoverMode CrossMode;
	TParentSelectionMode ParentSelectMode;
	// Search Parameters
	int vectorSize;
	int MaxGens;
	double EFraction;
	double MaxExpOffspring;
	double MutationVar;
	double InputBiasMutationVar;
	double CrossProb;
	TVector<int> crossTemplate;
	TVector<int> crossPoints;
	TVector<int> ConstraintVector;
	int ReEvalFlag;
	int CheckpointInt;
	// Function Pointers
	double (*EvaluationFunction)(TVector<double>& v, RandomState& rs);
	void (*BestActionFunction)(int Generation, TVector<double>& v);
	void (*PopulationStatisticsDisplayFunction)(int Generation, double BestPerf, double AvgPerf, double PerfVar, double executionTime);
	int (*SearchTerminationFunction)(int Generation, double BestPerf, double AvgPerf, double PerfVar);
	void (*SearchResultsDisplayFunction)(TSearch& s);
};


// A range specification structure for threaded evaluation

struct PopRangeSpec { TSearch* search; int start; int end; TVector<TVector<double>>* population; TVector<double>* performances; };