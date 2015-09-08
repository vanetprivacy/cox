#include <stdlib.h>
#include <cmath>
#include "StationaryMixPointRegular.h"
#include "Globals.h"
#define ASSIGN_DELAY 0
#define SILENT 30

bool StationaryMixPointRegular::SimulationBegin()
{
	ZoneCount=GetRunVariableInteger("ZoneCount");
	Radius=GetRunVariableInteger("ZoneRadius");
	int temp_x=GetRunVariableInteger("MixPointX");
	int temp_y=GetRunVariableInteger("MixPointY");
	ZoneTime=TimeMax/ZoneCount;
	
	//This loop pre-generates all the anonymity zones, but populates them later.
	for(int i=0;i<ZoneCount;i++)
	{
		int AssignedTime=i*ZoneTime;
		int SilentTime=AssignedTime+ASSIGN_DELAY;
		int ResumeTime=SilentTime+SILENT;
		AnonymitySet* AnonSet=new AnonymitySet(temp_x,temp_y,Radius,AssignedTime,SilentTime,ResumeTime,NULL);
		AnonymitySets.PushObj(AnonSet);
	}
	AnonymitySetIDEdge=-1;
	return true;
}
bool StationaryMixPointRegular::SimulationTick()
{
	int AnonymitySetID=SimulationTime/ZoneTime;
	if(AnonymitySetID>=ZoneCount){AnonymitySetID--;}
	//We're using integer division, and the source files specify time from 0<->N inclusive, rather than 0<->(N-1) like C++ expects.
	if(AnonymitySetID!=AnonymitySetIDEdge)
	{
		AnonymitySetIDEdge=AnonymitySetID;
		LogTraffic(NULL,ANNOUNCEMENT);//We broadcast a new announcement at the beggining of each mix period.
	}
	AnonymitySet* CurrentSet=AnonymitySets[AnonymitySetID];
	for(int i=0;i<Vehicles.Count();i++)
	{
		Vehicle* CurrentVehicle=Vehicles[i];
		//If vehicle is not already a member of an anonymity set, and is not terminated.
		if(CurrentVehicle->AssignedSet==NULL&&CurrentVehicle->v_terminated==-1)
		{
			//assign anonymity set to vehicle, if vehicle in range of beacon
			//assume beacon at center of anon zone
			if(inrange(CurrentVehicle->x,CurrentVehicle->y,CurrentSet->k_assign_x,CurrentSet->k_assign_y,CurrentSet->k_assign_r))
			{
				// assign anon set id to vehicle
				CurrentVehicle->AssignedSet = CurrentSet;
				// add vehicle to anon set
				CurrentSet->AddVehicle(CurrentVehicle);
				// set assign time, x, t to current time, x, y
				CurrentVehicle->v_k_assign_t = SimulationTime;
				CurrentVehicle->v_k_assign_x = CurrentVehicle->x;
				CurrentVehicle->v_k_assign_y = CurrentVehicle->y;
				// set SILENT time to current time + assign_delay time
				CurrentVehicle->v_k_silent_t = SimulationTime + ASSIGN_DELAY;
				// set resume time to current time + assign_delay + SILENT 
				CurrentVehicle->v_k_resume_t = SimulationTime + ASSIGN_DELAY + SILENT;
			}
		}
	}
	return true;
}
void StationaryMixPointRegular::SimulationEnd(){}