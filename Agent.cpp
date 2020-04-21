// ************************************************************
// Agent class definition
// Madhavun Candadai Vasu
// Jun 06, 2016 - created
//
// TODO: in initNS use setters to set private attributes
// ************************************************************

//************
// Constructor
//************
#include "Agent.h"

//************
// Initialize
//************
void Agent::initNS(TVector<double> nsParams,IzhiNNArch * arch){
    //cout << "Initializing NS from Agent class" << endl;

    //cout << "\tcalling initCNS" << endl;
    ns.initCNS(nsParams,arch);
}

//************
// Control
//************
double Agent::step(double stepSize, TVector<double> distanceInputs){
    //cout << "\t\tIn Agent step" << endl;
    double motorNeuronsDiff = ns.step(stepSize, distanceInputs);
    double velocity = (motorNeuronsDiff*rateCodeGain)/       /*ver que onda esto (antes estaba bien pero ahora ya no se que onda, lo dejo con Ne+Ni)-->*/ cnsSize;

    double offset = stepSize*velocity;
    //cout << "\t\tVelocity = " << velocity << " offset = " << offset << endl;
    return offset;
}

void Agent::reset(){
    ns.reset();
}