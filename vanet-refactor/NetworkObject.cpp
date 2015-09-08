#include "NetworkObject.h"
#include "Globals.h"

NetworkObject::NetworkObject()
{
	NetworkObjects.PushObj(this);
	Title="Unknown Network Object";
	x=0;
	y=0;
	TransmissionRadius=0;
	MessagesInRegion=0;
	TooManyMessagesInRegionLock=-1;
}
NetworkObject::NetworkObject(std::string CounterTitle):Title(CounterTitle){NetworkObjects.PushObj(this);}
NetworkObject::~NetworkObject(){NetworkObjects.Remove(this);}
int NetworkObject::GlobalMessageLimit=0;
void NetworkObject::LogTraffic(TrafficType Type)
{
	bool CanTransmit=NetworkMessages.NotAtMaximum()&&TooManyMessagesInRegionLock!=SimulationTime;
	if(CanTransmit&&GlobalMessageLimit!=0&&GlobalTraffic.PacketsTransmittedThisTick>=GlobalMessageLimit)
	{
		int MessagesInRegion=0;
		float Dist2=TransmissionRadius*TransmissionRadius;
		NetworkObject* OtherVehicle;
		for(int i=0;i<VehicleMax;i++)
		{
			OtherVehicle=(NetworkObject*)Vehicles[i];
			if(DistanceSquared(x,y,OtherVehicle->x,OtherVehicle->y)<=Dist2){MessagesInRegion+=OtherVehicle->NetworkMessages.PacketsTransmittedThisTick;}
		}
		if(MessagesInRegion>=GlobalMessageLimit){CanTransmit=false;}
	}
	if(!CanTransmit){TooManyMessagesInRegionLock=SimulationTime;}
	NetworkMessages.LogTraffic(Type,CanTransmit);
}