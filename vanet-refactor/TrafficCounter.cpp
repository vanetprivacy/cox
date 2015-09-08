#include "TrafficCounter.h"
#include "Globals.h"

TrafficCounter::TrafficCounter(){Clear();}
void TrafficCounter::LogTraffic(TrafficType Type,bool Successful)
{
	if(SimulationTime!=SimulationTimeEdge)
	{
		PacketsTransmittedThisTick=0;
		SimulationTimeEdge=SimulationTime;
	}
	if(SimulationTime!=GlobalTraffic.SimulationTimeEdge)
	{
		GlobalTraffic.PacketsTransmittedThisTick=0;
		GlobalTraffic.SimulationTimeEdge=SimulationTime;
	}
	PacketsTransmittedThisTick++;
	if(PacketsTransmittedThisTick>MostPacketsTransmitted){MostPacketsTransmitted=PacketsTransmittedThisTick;}
	if(Successful){PacketsTransmitted[Type]++;}
	else{PacketsLost[Type]++;}
	if(this!=&GlobalTraffic){GlobalTraffic.LogTraffic(Type,Successful);}
}