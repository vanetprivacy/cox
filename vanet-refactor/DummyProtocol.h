#include "Protocol.h"
class DummyProtocol:public Protocol
{
public:
	bool SimulationBegin();
	bool SimulationTick();
	void SimulationEnd();
};