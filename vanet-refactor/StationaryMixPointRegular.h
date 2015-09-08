#pragma once
#include "Protocol.h"
class StationaryMixPointRegular:public Protocol
{
	int AnonymitySetIDEdge;
public:
	int ZoneCount;
	int Radius;
	int ZoneTime;
	bool SimulationBegin();
	bool SimulationTick();
	void SimulationEnd();
};