#pragma once
#include "TrafficCounter.h"

class NetworkObject
{
public:
	NetworkObject();
	NetworkObject(std::string CounterTitle);
	~NetworkObject();
	std::string Title;
	int x;
	int y;
	float TransmissionRadius;
	TrafficCounter NetworkMessages;
	int MessagesInRegion;
	int TooManyMessagesInRegionLock;//This number is used to short-circuit the regional message count calculation.
	static int GlobalMessageLimit;
	void LogTraffic(TrafficType Type);
};