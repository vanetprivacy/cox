#include "Protocol.h"
#include "RSU.h"

class DummyEventProtocol:public Protocol
{
	bool OnTheFly;
	bool Irregular;
	bool GroupLeader;
	int SilentPeriod;
	int MaxVehiclesInSet;
	float AnnouncerRadius;
	RSU MainTransmitter;
public:
	DummyEventProtocol(bool OnTheFly,bool Irregular,bool GroupLeader);
	bool SimulationBegin();
	bool SimulationTick();
	void SimulationEnd();
};