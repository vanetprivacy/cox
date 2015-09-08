#include "AnonymitySet.h"
#include "Vehicle.h"
#include "Globals.h"

AnonymitySet::AnonymitySet(int x,int y,int Radius,int AssignedTime,int SilentTime,int ResumeTime,NetworkObject* Announcer)
{
	k_assign_x=x;
	k_assign_y=y;
	k_assign_r=Radius;
	k_assign_t=AssignedTime;
	k_silent_t=SilentTime;
	k_resume_t=ResumeTime;
	this->Announcer=Announcer;
}
int AnonymitySet::Count(){return AnonymousVehicles.Count();}
void AnonymitySet::AddVehicle(Vehicle* Add)
{
	AnonymousVehicles.PushObj(Add);
	Add->AssignedSet=this;
	Add->v_k_assign_t=k_assign_t;
	Add->v_k_assign_x=k_assign_x;
	Add->v_k_assign_y=k_assign_y;
	Add->v_k_silent_t=k_silent_t;
	Add->v_k_resume_t=k_resume_t;
}
void AnonymitySet::RemoveVehicle(Vehicle* Rem)
{
	AnonymousVehicles.Remove(Rem);
	Rem->AssignedSet=NULL;
}