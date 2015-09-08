#pragma once
#include <string>
#include "Enums.h"

struct TrafficCounter
{
	int SimulationTimeEdge;
	int MostPacketsTransmitted;
	int PacketsTransmittedThisTick;
	int MaximumPacketsPerTick;
public:
	TrafficCounter();
	int PacketsTransmitted[TOTAL_TRAFFIC_TYPES];
	int PacketsLost[TOTAL_TRAFFIC_TYPES];
	void LogTraffic(TrafficType Type,bool Successful);
	inline void Clear()
	{
		SimulationTimeEdge=0;
		PacketsTransmittedThisTick=0;
		MostPacketsTransmitted=0;
		MaximumPacketsPerTick=0;
		for(int i=0;i<TOTAL_TRAFFIC_TYPES;i++)
		{
			PacketsTransmitted[i]=0;
			PacketsLost[i]=0;
		}
	}
	inline int GetMostPacketsTransmitted(){return MostPacketsTransmitted;}
	inline bool NotAtMaximum(){return PacketsTransmittedThisTick<MaximumPacketsPerTick||MaximumPacketsPerTick==0;}
	inline int operator[](int Index)
	{
		if(Index==TOTAL_TRAFFIC_TYPES){return Total();}
		return PacketsTransmitted[Index]+PacketsLost[Index];
	}
	inline int Total()
	{
		int RetVal=0;
		for(int i=0;i<TOTAL_TRAFFIC_TYPES;i++){RetVal+=(*this)[i];}
		return RetVal;
	}
	inline int TotalTransmitted()
	{
		int RetVal=0;
		for(int i=0;i<TOTAL_TRAFFIC_TYPES;i++){RetVal+=PacketsTransmitted[i];}
		return RetVal;
	}
	inline int TotalLost()
	{
		int RetVal=0;
		for(int i=0;i<TOTAL_TRAFFIC_TYPES;i++){RetVal+=PacketsLost[i];}
		return RetVal;
	}
	inline float PacketLoss()
	{
		float Total=this->Total();
		if(Total==0.0f){return 0.0f;}
		return (float)TotalLost()/Total;
	}
	inline int Overhead(){return Total()-(*this)[APPLICATION];}//All traffic except APPLICATION.
	inline float OverheadRatio()//Overhead packets divided by total packets.
	{
		float TotalPackets=Total();
		if(TotalPackets==0.0f){return 0;}
		return (float)Overhead()/TotalPackets;
	}
};