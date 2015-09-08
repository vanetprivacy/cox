#include "Vehicle.h"
#include "Globals.h"
Vehicle::Vehicle(std::string CounterTitle)
{
	Title=CounterTitle;
	this->Clear();
}
void Vehicle::Clear()
{
	v_begin_t=-1;
	v_begin_x=-1;
	v_begin_y=-1;
	x=-1;
	y=-1;
	v_end_t=-1;
	v_end_x=-1;
	v_end_y=-1;
	v_terminated=-1;
	
	AssignedSet=(AnonymitySet*)0;//NULL Wasn't defined in this scope...?
	v_k_assign_t=-1;
	v_k_assign_x=-1;
	v_k_assign_y=-1;
	v_k_silent_t=-1;
	v_k_silent_x=-1;
	v_k_silent_y=-1;
	v_k_resume_t=-1;
	v_k_resume_x=-1;
	v_k_resume_y=-1;
	
	AverageAnonymitySetSizeCounter=0;
	AverageAnonymityDistanceCounter=0;
}
bool Vehicle::CurrentlyWithinSimulation()
{
	return SimulationTime>=v_begin_t&&SimulationTime<=v_end_t;
	
}