#include <stdlib.h>
#include <random>
#include "StationaryMixPointIrregular.h"
#include "Globals.h"
#include "NetworkObject.h"
#include "Vehicle.h"

bool StationaryMixPointIrregular::SimulationBegin()
{
	//Initializations
	SeedRandom();
	Transmitter.x=GetRunVariableInteger("MixPointX");
	Transmitter.y=GetRunVariableInteger("MixPointY");
	Transmitter.Radius=GetRunVariableInteger("ZoneRadius");
	Transmitter.Title="Stationary Mix Point";
	RandDurationMin=GetRunVariableInteger("RandDurationMin");
	RandDurationMax=GetRunVariableInteger("RandDurationMax");
	NextSilentPeriod=0;
	return true;
}
bool StationaryMixPointIrregular::SimulationTick()
{
	//The simulation does nothing until the next scheduled silent period.
	if(SimulationTime>=NextSilentPeriod)
	{
		NextSilentPeriod+=RandInt(RandDurationMin,RandDurationMax);
		LogTraffic(&Transmitter,ANNOUNCEMENT);//Each mix zone gets one announcement frame to signal the other vehicles.
		ArrayList<Vehicle*>* NearbyVehicles=GetVehiclesInRange(&Transmitter);
		AnonymitySet* CurrentSet=new AnonymitySet(Transmitter.x,Transmitter.y,Transmitter.Radius,SimulationTime,SimulationTime,NextSilentPeriod,&Transmitter);
		//Any vehicles in range that do not already have an anonymity set are assigned to the current one.
		for(int i=0;i<NearbyVehicles->Count();i++)
		{
			Vehicle* CurrentVehicle=(*NearbyVehicles)[i];
			if(CurrentVehicle->AssignedSet==NULL){CurrentSet->AddVehicle(CurrentVehicle);}
		}
		if(CurrentSet->Count()>0){AnonymitySets.PushObj(CurrentSet);}//We must remember to register this anonymity set in the global record so the simulator can claculate anonymity stats properly.
		else{delete CurrentSet;}//If the anonymity set is empty, we remove it.
		delete NearbyVehicles;//We no longer need this structure. Note that the objects referenced in the structure belong to the global vehicle roster, so it is not our responsibility to delete it.
	}
	return true;
}
void StationaryMixPointIrregular::SimulationEnd(){}