#pragma once
#include "NetworkObject.h"
#include "TrafficCounter.h"
class AnonymitySet;
class Vehicle:public NetworkObject
{
public:
	Vehicle(std::string CounterTitle);
	void Clear();
	
	int v_begin_t;
	int v_begin_x;
	int v_begin_y;
	int v_end_t;
	int v_end_x;
	int v_end_y;
	int v_terminated;

	AnonymitySet* AssignedSet;
	int v_k_assign_t;
	int v_k_assign_x;
	int v_k_assign_y;
	int v_k_silent_t;
	int v_k_silent_x;
	int v_k_silent_y;
	int v_k_resume_t;
	int v_k_resume_x;
	int v_k_resume_y;
	
	long long int AverageAnonymitySetSizeCounter;//Increased every tick, then divided by the time it sent in the simulation once the simulation has ended.
	long double AverageAnonymityDistanceCounter;
	
	bool CurrentlyWithinSimulation();
};