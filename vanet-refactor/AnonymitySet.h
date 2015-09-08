#pragma once
#include "ArrayList.h"

class NetworkObject;
class Vehicle;
class AnonymitySet
{
public:
	AnonymitySet(int x,int y,int Radius,int AssignedTime,int SilentTime,int ResumeTime,NetworkObject* Announcer);
	int k_assign_x;
	int k_assign_y;
	int k_assign_r;
	int k_assign_t; // time vehicle are assigned to anon. set
	int k_silent_t; // start of silent period
	int k_resume_t; // time vehicle is anonymized and LBS-connected
	NetworkObject* Announcer;//The network unit which formed the anonymity set. Usually the one that sends the announcement packet.
	ArrayList<Vehicle*> AnonymousVehicles;//Vehicles are still owned by the global repository.
	int Count();
	void AddVehicle(Vehicle* Add);
	void RemoveVehicle(Vehicle* Remove);
};