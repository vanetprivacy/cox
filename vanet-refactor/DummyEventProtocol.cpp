#include "DummyEventProtocol.h"
#include "Vehicle.h"
#include "Globals.h"
#include <string>
#include <cmath>      // for sqrt(), max()
#define ON_THE_FLY_SELECT_ANNOUNCER_ATTEMPTS 200

DummyEventProtocol::DummyEventProtocol(bool OnTheFly,bool Irregular,bool GroupLeader):MainTransmitter(0,0,0,std::string("Main RSU"))
{
	this->OnTheFly=OnTheFly;
	this->Irregular=Irregular;
	this->GroupLeader=GroupLeader;//Group leader is unimplemented and, for now, ignored.
}
bool DummyEventProtocol::SimulationBegin()
{
	PreSimulationVariableCheck("SilentPeriod");
	PreSimulationVariableCheck("MaxVehiclesInSet");
	PreSimulationVariableCheck("VehicleTransmissionRadius");
	SeedRandom();
	SilentPeriod=GetRunVariableInteger("SilentPeriod");
	MaxVehiclesInSet=GetRunVariableInteger("MaxVehiclesInSet");
	AnnouncerRadius=GetRunVariableInteger("VehicleTransmissionRadius");
	int MinRandomDuration=0;
	int MaxRandomDuration=0;
	if(!OnTheFly)
	{
		PreSimulationVariableCheck("MixPointX");
		PreSimulationVariableCheck("MixPointY");
		MainTransmitter.x=GetRunVariableInteger("MixPointX");
		MainTransmitter.y=GetRunVariableInteger("MixPointY");
	}
	else
	{
		for(int i=0;i<VehicleMax;i++){Vehicles[i]->TransmissionRadius=AnnouncerRadius;}
	}
	if(Irregular)
	{
		PreSimulationVariableCheck("MinRandomDuration");
		PreSimulationVariableCheck("MaxRandomDuration");
		MinRandomDuration=GetRunVariableInteger("MinRandomDuration");
		MaxRandomDuration=GetRunVariableInteger("MaxRandomDuration");
	}
	MainTransmitter.TransmissionRadius=AnnouncerRadius;
	for(int i=0;i<TimeMax;i+=(Irregular?RandInt(MinRandomDuration,MaxRandomDuration):SilentPeriod))
	{
		AnonymitySet* CurrentSet=new AnonymitySet(MainTransmitter.x,MainTransmitter.y,AnnouncerRadius,i,i,i+SilentPeriod,&MainTransmitter);
		AnonymitySets.PushObj(CurrentSet);
	}
	return true;
}
bool DummyEventProtocol::SimulationTick()
{
	//Outermost loop cycles through all anonymity sets, until the simulation reaches the assignment point for that set and builds the list of vehicles.
	for(int i=0;i<AnonymitySets.Count();i++)
	{
		AnonymitySet* CurrentSet=AnonymitySets[i];
		if(SimulationTime==CurrentSet->k_assign_t)
		{
			//when the assignment point is reached, all vehicles in range that are not already part of a set are added.
			if(OnTheFly)
			{
				LogTraffic(&MainTransmitter,NOMINATE);//In the case of on-the-fly assignment, the transmitter will first nominate the group founder.
				//This loop finds a random vehicle within the simulation to act as the announcer.
				Vehicle* Announcer=NULL;
				for(int Attempts=1;Attempts<=ON_THE_FLY_SELECT_ANNOUNCER_ATTEMPTS;Attempts++)
				{
					Announcer=Vehicles[RandInt(0,VehicleMax-1)];
					if(Announcer->CurrentlyWithinSimulation()&&Announcer->AssignedSet==NULL){break;}
					if(Attempts==ON_THE_FLY_SELECT_ANNOUNCER_ATTEMPTS){Announcer=NULL;}
				}
				if(Announcer==NULL)
				{
					cerr<<"WARNING: No On-the-Fly announcer could be selected at t="<<SimulationTime<<"!"<<endl;
					continue;
				}
				//cerr<<"ANNOUNCER: ("<<Announcer->x<<","<<Announcer->y<<")";
				CurrentSet->Announcer=Announcer;
				CurrentSet->k_assign_x=CurrentSet->Announcer->x;
				CurrentSet->k_assign_y=CurrentSet->Announcer->y;//We also need to change the position of the anonymity set to the announcer's current location.
			}
			//Otherwise, the transmitter acts as the group founder, and no nomination is needed because the transmitter can announce the group immediately.
			LogTraffic(CurrentSet->Announcer,ANNOUNCEMENT);
			ArrayList<Vehicle*>* AnonymityCandidates=GetVehiclesInRange(CurrentSet->Announcer);
			//cerr<<":"<<AnonymityCandidates->Count()<<endl;
			for(int io=0;io<AnonymityCandidates->Count();)
			{
				Vehicle* CurrentVehicle=(*AnonymityCandidates)[io];
				if(CurrentVehicle->AssignedSet==NULL)
				{
					if(CurrentVehicle!=CurrentSet->Announcer){LogTraffic(CurrentVehicle,JOIN);}//All vehicles that wish to join send a confirmation packet, regardless of whether the group founder accepts them.
					io++;
				}
				else{AnonymityCandidates->Remove(io);}
			}
			if(MaxVehiclesInSet>0)
			{
				//For as many cars as exceed the maximum allowed vehicles in the set, the farthest one is removed.
				for(int VehiclesToRemove=AnonymityCandidates->Count()-MaxVehiclesInSet;VehiclesToRemove>0;VehiclesToRemove--)
				{
					float FarthestDistanceSquared=0;
					Vehicle* FarthestVehicle;
					for(int io=0;io<AnonymityCandidates->Count();io++)
					{
						Vehicle* CheckVehicle=(*AnonymityCandidates)[io];
						float XDist=CurrentSet->k_assign_x-CheckVehicle->x;
						float YDist=CurrentSet->k_assign_y-CheckVehicle->y;
						float DistSqr=XDist*XDist+YDist*YDist;
						if(DistSqr>=FarthestDistanceSquared)
						{
							FarthestVehicle=CheckVehicle;
							FarthestDistanceSquared=DistSqr;
						}
					}
					AnonymityCandidates->Remove(FarthestVehicle);
				}
			}
			//If only one vehicle participates, the anonymous set is invalid, and the participating vehicle is free to join a later group.
			if(AnonymityCandidates->Count()>1)
			{
				//The announcer will only announce confirmations for those vehicles which were not omitted due to the set size minimum or maximum.
				for(int io=0;io<AnonymityCandidates->Count();io++)
				{
					Vehicle* CurrentVehicle=(*AnonymityCandidates)[io];
					if(Vehicles.IndexOf(CurrentVehicle)==8)
					{
						//cerr<<"CONFIRM!"<<endl;
					}
					CurrentSet->AddVehicle(CurrentVehicle);
					if(CurrentVehicle!=CurrentSet->Announcer){LogTraffic(CurrentSet->Announcer,JOIN_CONFIRMATION);}
				}
			}
			delete AnonymityCandidates;
		}
	}
	LogApplicationTraffic(&MainTransmitter);//Even if the RSU is not the group founder, messages are sent there.
	return true;
}
void DummyEventProtocol::SimulationEnd(){}