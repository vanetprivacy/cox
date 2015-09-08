#pragma once
#include "Protocol.h"
#include "RSU.h"
class StationaryMixPointIrregular:public Protocol
{
    RSU Transmitter;
    int AnonymitySetIDEdge;
public:
	int RandDurationMin;
	int RandDurationMax;
	float NextSilentPeriod;
    bool SimulationBegin();
    bool SimulationTick();
    void SimulationEnd();
};