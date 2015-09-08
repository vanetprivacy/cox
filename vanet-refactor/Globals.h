#pragma once
#include <iomanip>    // for formatting test setw, setprecision
#include "ArrayList.h"
#include "Dictionary.h"
#include "AnonymitySet.h"
#include "Enums.h"
#include "Protocol.h"
#include "TrafficCounter.h"
#include "Vehicle.h"
class NetworkObject;

bool inrange(float x1, float y1, float x2, float y2, float r);//Returns true if (x1,y1) is within r of (x2,y2).
float get_distance(float x1, float y1, float x2, float y2);//Returns euclidian distance between (x1,y1) and (x2,y2).
inline float DistanceSquared(float x1,float y1,float x2,float y2){return (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);}//Gets the square of the euclidian distance.
ArrayList<Vehicle*>* GetVehiclesInRange(NetworkObject* Transmitter);//Gets an ArrayList of all the vehicles within range of a particular transmitter, including that transmitter.
//WARNING: The ArrayList is owned by the CALLER. It is YOUR responsibility to delete the data structure.
//The referenced vehicles, however, are owned by the global roster and are deleted by the simulation core.
void SetGlobalVehicleMaxMessagePerTick(int Max);//Iterates over all vehicles and sets the maximum number of transmitted messages allowed.
void LogTraffic(NetworkObject* Sender,TrafficType Type);//Used to record a single packet. Sender can be NULL, but it is recommended that you create an RSU object for non-vehicle traffic.
void LogLegitimateTraffic(NetworkObject* ContactPoint);//Iterates over all vehicles and sends a single application packet.
void LogApplicationTraffic(NetworkObject* ContactPoint);//Iterates over all anonymity sets and simulates each vehicle sending both legitimate and dummy traffic.
//ContactPoint is usually the communicating RSU, but it can also be a group leader.
void SeedRandom();//Uses the RunVariable seed value to seed the random number generator.
int RandInt(int Min,int Max);//Convenience function; Generates a random integer within [Min,Max].

//The parameter file will specify these variables, and the following functions are used by the protocols to get their parameters.
bool CheckRunVariableExists(std::string VariableName);//Returns true if this variable was specified by the parameter file.
extern string GetRunVariableString(string Key);//Converts the run variable to a string before returning.
extern int GetRunVariableInteger(string Key);//Converts the run variable to an integer before returning.
extern int GetRunVariableFloat(string Key);//Converts the run variable to a float before returning.
//This macro is for convenience when doing pre-simulation checks in the SimulationBegin method.
//WARNING: DO NOT USE THIS IF YOU DO NOT KNOW WHAT YOU ARE DOING.
#define PreSimulationVariableCheck(s); if(!CheckRunVariableExists(s)){std::cerr<<"ERROR: Missing run variable: "<<(s)<<endl;return false;}

extern ArrayList<AnonymitySet*> AnonymitySets;//Global repository for all anonymity sets, and used to calculate anonymity statistics after the simulation has terminated.
//This ArrayList is the owner of the referenced AnonymitySet objects until the end of the execution.
//Any protocol which removes a reference from this list is responsible for deletion.
extern ArrayList<Vehicle*> Vehicles;//Global repository for all vehicles in the simulation.
//It is not recommended for protocols to modify this list for any reason. Consider it read-only.
extern ArrayList<NetworkObject*> NetworkObjects;//References to all networked objects in the system.
//WARNING: Unlike vehicles and anonymity sets, these objects are NOT OWNED by the global repository.
//WARNING: Do not manipulate this structure. NetworkObjects objects add and remove themselves from this list. Furthermore, even READING this structure is probaby unneeded.
extern Protocol* PrimaryProtocol;//This is specified by the parameter file.
extern Dictionary<string> RunVariables;//Run variables are like environmental variables for each simulation.
extern Dictionary<string> OutputVariables;//Outputs are placed in the dictionary structure so the simulation's output can be dynamically formatted.
extern TrafficCounter GlobalTraffic;//LogTraffic makes note of packets in both the Sender's traffic counter, and this global traffic counter.
extern int VehicleMax;//The number of Vehicles in the simulation.
extern int TimeMax;//The number of time intervals in the simulation.
extern int SimulationTime;//The current time interval, updated each iteration by the core simulator logic.