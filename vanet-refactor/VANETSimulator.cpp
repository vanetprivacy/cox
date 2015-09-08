// filename    : VANETSimulator.cpp
// author      : REU, 2015-06-02
// description : this program reads .srtt file (form: t, v, x, y, begin, end)
//               then computes stats for each vehicle (k, d, t)
//               and writes the output to a file
// input       : Parameter file with batch instructions for simulator
//               command line arguments: VANETSimulator.exe <ParameterFile>
// output      : Simulation Results
//
// rev:2015-03-21
// use the VANET runner by passing a parameter file as the argument.
//
// overall design: main() function...
//  Initializations
//  Parameter file interpreter
//
// SimulationRun(std::string[]) - Pass in an array of strings which denote the parameters which were specified in this particular run. This is needed for display purposes.
//  Initializations
//  Simulation Bounds Calculations
//  PrimaryProtocol->SimulationBegin();
//  Loop over each line in the trace file, updating vehicle information.
//   If time step has changed, PrimaryProtocol->SimulationTick();
//  PrimaryProtocol->SimulationEnd();


#include <fstream>    // for ifstream, ofstream
#include <string>     // for string data types
#include <iostream>   // for endl, fixed
#include <cmath>      // for sqrt(), max()
#include <stdlib.h>   // for exit()
#include <sstream>    // for istringstream
#include <limits.h>   // For INT_MAX.
#include "Globals.h"
#include "NetworkObject.h"
#include "DummyProtocol.h"
#include "StationaryMixPointRegular.h"
#include "StationaryMixPointIrregular.h"
#include "DummyEventProtocol.h"
//This line is needed to suppress a warning that occurs when I pass a dynamically generated string as a printf argument.
//This warning is intended to alert the developer of a possible injection attack using format strings,
// but that doesn't apply because the user can specify his own format strings anyway when formatting output.
//As well, though the user specifies the output format, the actual values passed into printf are generated only by the simulator with no user input.
//Ironically, the warning is valid, but for the completely wrong reason.
#pragma GCC diagnostic ignored "-Wformat-security"

using namespace std;

//GLOBAL VARIABLES
const int MAX_PARAMETERS_PER_LINE=12;
const char OUTPUT_FORMAT_DELIMITER='%';
const char OUTPUT_FORMAT_SUB_DELIMITER=';';
TrafficCounter GlobalTraffic;
ArrayList<AnonymitySet*> AnonymitySets(1000);
ArrayList<Vehicle*> Vehicles(5000);
ArrayList<NetworkObject*> NetworkObjects(5000);
int VehicleMax;
int TimeMax;
int SimulationTime;
//RUN SPECIFICATION VARIABLES
Protocol* PrimaryProtocol;
string ProtocolName;
string TraceFileName;
Dictionary<string> RunVariables;
Dictionary<string> OutputVariables;
bool LegacyOutput=true;
bool VerboseOutput=false;
string* OutputFormat=NULL;

//SIMULATION API
void AssignPrimaryProtocol(string NewProtocolName)//This was removed from the parameter file interpreter to make it easier to spot when adding new protocols.
{
	ProtocolName=NewProtocolName;
	if(PrimaryProtocol!=NULL)
	{
		delete PrimaryProtocol;
		PrimaryProtocol=NULL;
	}
	//TODO: Find a more elegant way to dynamically assign protocols.
	if(ProtocolName=="DummyProtocol"){PrimaryProtocol=new DummyProtocol();}
	else if(ProtocolName=="StationaryMixPointRegular"){PrimaryProtocol=new StationaryMixPointRegular();}
	else if(ProtocolName=="StationaryMixPointIrregular"){PrimaryProtocol=new StationaryMixPointIrregular();}
	else if(ProtocolName=="StationaryRegularDummyEvents"){PrimaryProtocol=new DummyEventProtocol(false,false,false);}
	else if(ProtocolName=="StationaryIrregularDummyEvents"){PrimaryProtocol=new DummyEventProtocol(false,true,false);}
	else if(ProtocolName=="OnTheFlyRegularDummyEvents"){PrimaryProtocol=new DummyEventProtocol(true,false,false);}
	else if(ProtocolName=="OnTheFlyIrregularDummyEvents"){PrimaryProtocol=new DummyEventProtocol(true,true,false);}
	else{cerr<<"ERROR: Invalid protocol!"<<endl;}
	cout<<"Protocol: "<<ProtocolName<<endl;
}
ArrayList<Vehicle*>* GetVehiclesInRange(NetworkObject* Transmitter)
{
	ArrayList<Vehicle*>* RetVal=new ArrayList<Vehicle*>();
	for(int i=0;i<Vehicles.Count();i++)
	{
		Vehicle* CurrentVehicle=Vehicles[i];
		if(!CurrentVehicle->CurrentlyWithinSimulation()){continue;}
		if(inrange(CurrentVehicle->x,CurrentVehicle->y,Transmitter->x,Transmitter->y,Transmitter->TransmissionRadius)){RetVal->PushObj(CurrentVehicle);}
	}
	return RetVal;//The data structure is owned by the caller and must be deleted after use.
}
void SetGlobalVehicleMaxMessagePerTick(int Max)
{
	for(int i=0;i<VehicleMax;i++){Vehicles[i]->NetworkMessages.MaximumPacketsPerTick=Max;}
}
void LogTraffic(NetworkObject* Sender,TrafficType Type)
{
	if(Sender==NULL){GlobalTraffic.LogTraffic(Type,true);}
	else{Sender->LogTraffic(Type);}
}
void LogLegitimateTraffic(NetworkObject* ContactPoint)
{
	for(int i=0;i<Vehicles.Count();i++)
	{
		Vehicle* CurrentVehicle=Vehicles[i];
		if(!CurrentVehicle->CurrentlyWithinSimulation()||(SimulationTime>=CurrentVehicle->v_k_silent_t&&SimulationTime<=CurrentVehicle->v_k_resume_t)){continue;}
		LogTraffic(Vehicles[i],APPLICATION);
		if(ContactPoint!=NULL){LogTraffic(ContactPoint,APPLICATION);}
	}
}
void LogApplicationTraffic(NetworkObject* ContactPoint)
{
	LogLegitimateTraffic(ContactPoint);
	for(int i=0;i<AnonymitySets.Count();i++)
	{
		AnonymitySet* CurrentSet=AnonymitySets[i];
		if(SimulationTime<CurrentSet->k_resume_t){continue;}
		int VehiclesInSet=CurrentSet->Count();
		for(int io=0;io<VehiclesInSet;io++)
		{
			Vehicle* CurrentVehicle=CurrentSet->AnonymousVehicles[io];
			for(int ie=0;ie<VehiclesInSet-1;ie++)
			{
				if(io==ie){continue;}
				LogTraffic(CurrentVehicle,DUMMY);
				if(ContactPoint!=NULL){LogTraffic(ContactPoint,DUMMY);}
			}
			//Each vehicle sends one application message as a dummy event for every other vehicle in the same set, and one real message for itself.
		}
	}
}
string GetRunVariableString(string Key){return RunVariables.Get(Key);}
bool CheckRunVariableExists(string Key){return RunVariables.HasKey(Key);}
int GetRunVariableInteger(string Key){return atoi(GetRunVariableString(Key).c_str());}
int GetRunVariableFloat(string Key){return atof(GetRunVariableString(Key).c_str());}
void ResetGlobalVariables()
{
	GlobalTraffic.Clear();
	AnonymitySets.Clear();
	OutputVariables.Clear();
	Vehicles.Clear();
	VehicleMax=0;
	TimeMax=0;
	SimulationTime=-1;
}
void SeedRandom(){srand(GetRunVariableInteger("RandomSeed"));}
int RandInt(int Min,int Max)
{
	int RandomRange=(Max+1)-Min;//This range represents the random zone, not the minimum value which is constant.
	//The +1 changes the range of the function from [Min,Max) to [Min,Max].
	if(RandomRange<0)
	{
		cerr<<"ERROR: Random range invalid:("<<Min<<"-"<<Max<<")"<<endl;
		return Min;
	}
	int RandomResult=0;
	if(RandomRange>0)
	{
		//Since most random values do not evenly divide the random of possible values, we must disqualify a number of possibilities in the upper range.
		int MaximumRandValue=(INT_MAX/RandomRange)*RandomRange;
		int Random;
		do{Random=rand();}while(Random>=MaximumRandValue);//Using >= instead of > avoids a very small bias towards a result of zero, which would cause the minimum value to be returned.
		RandomResult=Random%RandomRange;//This gives us an integer in the range of [0,RandomRange).
	}
	return Min+RandomResult;//The minimum amount is added regardless. RandomResult represents the additional value within the specified range.
}

void SimulationRun(string RunParameterNames[]);
int main(int ArgumentsCount, char *Arguments[])
{
	if(ArgumentsCount!=2)
	{
		cerr<<"ERROR: Please supply one argument. (Had "<<(ArgumentsCount-1)<<")."<<endl;
		return 1;
	}
	//COMMAND LINE VARIABLES
	char* InputFile=Arguments[1];
	
	//INITIAL DECLARATIONS
	ifstream ParameterFile(InputFile);
	if(!ParameterFile.is_open())
	{
		cerr<<"ERROR: Input file could not be opened: "<<InputFile<<endl;
		return 1;
	}
	cout<<"Input File: "<<InputFile<<endl;
	string Line;
	string LineParameters[MAX_PARAMETERS_PER_LINE];
	string RunParameterNames[MAX_PARAMETERS_PER_LINE-1];
	PrimaryProtocol=NULL;
	ProtocolName="";
	TraceFileName="";
	//DEFAULT RUN PARAMETERS
	RunVariables.Set("RandomSeed",to_string((long long int)time(NULL)));
	RunVariables.Set("MaxVehiclesInSet","0");
	RunVariables.Set("GlobalMessageLimit","0");
	RunVariables.Set("VehicleMessageLimit","0");
	
	//Loop through parameter file:
	while(!ParameterFile.eof())
	{
		getline(ParameterFile,Line);
		//We need to use a string stream to easily extract individual words from the line.
		istringstream* LineStream=new istringstream(Line);
		for(int i=0;i<MAX_PARAMETERS_PER_LINE;i++)
		{
			if(!(*LineStream>>LineParameters[i])){LineParameters[i]="";}
		}
		delete LineStream;
		
		string Command=LineParameters[0];
		string LineSansCommand("");
		size_t FindFirstArgument=Line.find(' ')+1;
		if(FindFirstArgument<Line.size()){LineSansCommand=Line.substr(FindFirstArgument,Line.size()-FindFirstArgument);}
		
		if(Command=="PROTOCOL")
		{
			AssignPrimaryProtocol(LineParameters[1]);
		}
		else if(Command=="TRACEFILE")
		{
			TraceFileName=LineParameters[1];
			cout<<"Trace File: "<<TraceFileName<<endl;
		}
		else if(Command=="PARAMETERS")
		{
			//The parameters command dictates how run commands are interpreted. If the parameters are A B C, then RUN 1 2 3 will be interpreted as A=1, B=2, C=3, RUN.
			cout<<"Parameters:";
			for(int i=1;i<MAX_PARAMETERS_PER_LINE;i++)
			{
				RunParameterNames[i-1]=LineParameters[i];
				cout<<" "<<LineParameters[i];
			}
			cout<<endl;
		}
		else if(Command=="RUN")
		{
			for(int i=1;i<MAX_PARAMETERS_PER_LINE&&LineParameters[i].length()>0;i++){RunVariables.Set(RunParameterNames[i-1],LineParameters[i]);}
			SimulationRun(RunParameterNames);
		}
		else if(Command=="ECHO"){cout<<LineSansCommand<<endl;}
		else if(Command=="OUTPUT")
		{
			if(LineSansCommand=="LEGACY")
			{
				LegacyOutput=true;
				VerboseOutput=false;
				cout<<"Output: Legacy"<<endl;
			}
			else if(LineSansCommand=="VERBOSE")
			{
				LegacyOutput=false;
				VerboseOutput=true;
				cout<<"Output: Verbose"<<endl;
			}
			else
			{
				LegacyOutput=false;
				VerboseOutput=false;
				if(OutputFormat!=NULL){delete OutputFormat;}
				OutputFormat=new string(LineSansCommand);
				cout<<"Output: "<<(*OutputFormat)<<endl;
			}
		}
		else if(Command!="REM")
		{
			RunVariables.Set(Command,LineParameters[1]);
			cout<<Command<<"="<<LineParameters[1]<<endl;
		}
	}
	cout<<"EXECUTION FINISHED"<<endl;
}

void SimulationRun(string RunParameterNames[])
{
	ifstream TraceFile;
	bool RunValid=true;
	if(PrimaryProtocol==NULL)
	{
		cerr<<"RUN ERROR: No protocol specified!"<<endl;
		RunValid=false;
	}
	if(TraceFileName=="")
	{
		cerr<<"RUN ERROR: No trace file specified!"<<endl;
		RunValid=false;
	}
	if(RunValid)
	{
		TraceFile.open(TraceFileName.c_str());
		if(!TraceFile.is_open())
		{
			cerr<<"RUN ERROR: Cannot open trace file!"<<endl;
			RunValid=false;
		}
		else if(TraceFile.bad()||TraceFile.fail())
		{
			cerr<<"RUN ERROR: Stream was corrupted!"<<endl;
			RunValid=false;
		}
	}
	if(!RunValid){return;}//Immediately break and cancel the run if there are execution errors.
	ResetGlobalVariables();
	int VehicleNum,BeginTime,EndTime;
	float x,y;
	int NextSimulationTime;
	int VehicleNumMask=1;//The modulo of the vehicle num and this value determines whether a vehicle is omitted to get a different set of vehicles in the simulation.
	int VehicleNumMaskOffset=0;
	bool MaskExists=CheckRunVariableExists("VehicleNumMask");
	bool OffsetExists=CheckRunVariableExists("VehicleNumMaskOffset");
	if(MaskExists!=OffsetExists){cerr<<"ERROR: Must specify both VehicleNumMask and VehicleNumMaskOffset for this input to be effective!"<<endl;}
	else if(MaskExists)
	{
		VehicleNumMask=GetRunVariableInteger("VehicleNumMask");
		VehicleNumMaskOffset=GetRunVariableInteger("VehicleNumMaskOffset");
	}
	while (TraceFile >> NextSimulationTime >> VehicleNum >> x >> y >> BeginTime >> EndTime)// loop through all input
	{
		TimeMax=max(NextSimulationTime+1,TimeMax);
		if((--VehicleNum)%VehicleNumMask==VehicleNumMaskOffset)
		{
			VehicleNum/=VehicleNumMask;
			VehicleMax=max(VehicleNum+1,VehicleMax);
			for(int i=Vehicles.Count();i<VehicleMax;i++){Vehicles.PushObj(new Vehicle("OBU#"+to_string((long long int)i)));}
			Vehicle* CurrentVehicle=Vehicles[VehicleNum];
			if(CurrentVehicle->v_begin_t==-1)
			{
				CurrentVehicle->v_begin_t=BeginTime;
				CurrentVehicle->v_begin_x=x;
				CurrentVehicle->v_begin_x=y;
				CurrentVehicle->v_end_t=EndTime;
			}
		}
	}
	OutputVariables.Set("VehicleMax",to_string((long long int)VehicleMax));
	OutputVariables.Set("TimeMax",to_string((long long int)TimeMax));
	OutputVariables.Set("TraceFile",TraceFileName);
	OutputVariables.Set("ProtocolName",ProtocolName);
	
	TraceFile.clear();
	TraceFile.seekg(0, TraceFile.beg);  // reposition to beginning of input file
	if(TraceFile.bad()||TraceFile.fail())
	{
		cerr<<"RUN ERROR: Could not reset stream!"<<endl;
		RunValid=false;
	}
	
	int PeakAnonymitySetSize=0;//Used to calculate one of our metrics later.
	float PeakAnonymityDistance=0;
	if(CheckRunVariableExists("GlobalMessageLimit")){NetworkObject::GlobalMessageLimit=GetRunVariableInteger("GlobalMessageLimit");}
	SetGlobalVehicleMaxMessagePerTick(CheckRunVariableExists("VehicleMessageLimit")?GetRunVariableInteger("VehicleMessageLimit"):0);
	time_t SimulationBeginTime=time(NULL);
	RunValid=RunValid&&PrimaryProtocol->SimulationBegin();
	bool TraceFileEOF=false;
	while(!TraceFileEOF)
	{
		TraceFile>>NextSimulationTime>>VehicleNum>>x>>y>>BeginTime>>EndTime;
		TraceFileEOF=TraceFile.eof();
		if(!TraceFileEOF&&(TraceFile.bad()||TraceFile.fail()))
		{
			cerr<<"RUN ERROR: Stream became corrupted during simulation!"<<endl;
			RunValid=false;
			break;
		}
		if(SimulationTime==-1){SimulationTime=NextSimulationTime;}//This prevents a logic error that causes a simulation tick before the first batch of tracefile data has been processed.
		if(TraceFileEOF||SimulationTime!=NextSimulationTime)
		{
			RunValid=RunValid&&PrimaryProtocol->SimulationTick();
			if(!RunValid){break;}//This short-circuits the execution if a critical error as occurred.
			//This loop adds to the average counters in each vehicle.
			//Average counters are processed after the simulation to get various anonymity statistics.
			//As such, these values are not menaingful until they have been finalized later.
			for(int i=0;i<VehicleMax;i++)
			{
				Vehicle* CurrentVehicle=Vehicles[i];
				if(!CurrentVehicle->CurrentlyWithinSimulation()){continue;}
				if(CurrentVehicle->AssignedSet!=NULL)
				{
					AnonymitySet* CurrentSet=CurrentVehicle->AssignedSet;
					int SetSize=CurrentSet->Count();
					CurrentVehicle->AverageAnonymitySetSizeCounter+=SetSize;
					PeakAnonymitySetSize=max(PeakAnonymitySetSize,SetSize);
					ArrayList<Vehicle*>* VehiclesInSet=&(CurrentSet->AnonymousVehicles);
					float SetDistance=0.0;
					for(int io=0;io<VehiclesInSet->Count();io++)
					{
						Vehicle* OtherVehicle=(*VehiclesInSet)[io];
						if(CurrentVehicle==OtherVehicle){continue;}
						float XDist=CurrentVehicle->x-OtherVehicle->x;
						float YDist=CurrentVehicle->y-OtherVehicle->y;
						float Dist=sqrt(XDist*XDist+YDist*YDist);
						SetDistance+=Dist;
						PeakAnonymityDistance=max(PeakAnonymityDistance,Dist);
					}
					CurrentVehicle->AverageAnonymityDistanceCounter+=SetDistance/SetSize;
				}
				else{CurrentVehicle->AverageAnonymitySetSizeCounter++;}
			}
			for(int i=0;i<VehicleMax;i++)
			{
				Vehicle* CurrentVehicle=Vehicles[i];
				if(SimulationTime==CurrentVehicle->v_end_t)
				{
					CurrentVehicle->v_end_x=CurrentVehicle->x;
					CurrentVehicle->v_end_y=CurrentVehicle->y;
					CurrentVehicle->v_terminated=SimulationTime;
					//We set ending data AFTER the protocol text so the set isn't prematurely decremented.
					if(CurrentVehicle->AssignedSet!=NULL){CurrentVehicle->AssignedSet->RemoveVehicle(CurrentVehicle);}
				}
			}
			if(TraceFileEOF){break;}
			SimulationTime=NextSimulationTime;
		}
		if((--VehicleNum)%VehicleNumMask==VehicleNumMaskOffset)
		{
			VehicleNum=VehicleNum/VehicleNumMask;
			Vehicle* CurrentVehicle=Vehicles[VehicleNum];//We decrement VehicleNum because trace files base their lists with 1 as the first index.
			CurrentVehicle->x=x;
			CurrentVehicle->y=y;
		}
	}
	if(RunValid)
	{
		PrimaryProtocol->SimulationEnd();
		time_t SimulationRunTime=time(NULL)-SimulationBeginTime;
		OutputVariables.Set("RunTime",to_string(((long double)SimulationRunTime)/60.0));
		
		//This loop adds inputs as outputs, so the resulting printed line can also reference parameters used in the run.
		for(int i=0;i<RunVariables.Count();i++){OutputVariables.Set(RunVariables.GetKey(i),RunVariables.Get(i));}
		
		//This loop calculates various averages.
		int AnonymousVehicleCount=0;
		int AverageAnonymitySamples=0;
		float AverageAnonymitySetSize=0.0;
		float AverageAnonymityDistance=0.0;
		float AverageAnonymityTime=0.0;
		float PeakAnonymityTime=0.0;
		for(int i=0;i<Vehicles.Count();i++)
		{
			Vehicle* CurrentVehicle=Vehicles[i];
			if(CurrentVehicle->v_terminated==-1)
			{
				cerr<<"TERMINATE ERROR: Vehicle#"<<i<<" was supposed to terminate at "<<CurrentVehicle->v_end_t<<"/"<<SimulationTime<<endl;
				CurrentVehicle->v_terminated=CurrentVehicle->v_end_t;
			}
			AverageAnonymitySamples+=(CurrentVehicle->v_terminated-CurrentVehicle->v_begin_t)+1;
			AverageAnonymitySetSize+=CurrentVehicle->AverageAnonymitySetSizeCounter;
			AverageAnonymityDistance+=CurrentVehicle->AverageAnonymityDistanceCounter;
			if(CurrentVehicle->v_k_assign_t!=-1)
			{
				AnonymousVehicleCount++;
				float AnonTime=CurrentVehicle->v_terminated-CurrentVehicle->v_k_silent_t;
				AverageAnonymityTime+=AnonTime;
				PeakAnonymityTime=max(PeakAnonymityTime,AnonTime);
			}
		}
		if(AnonymousVehicleCount>0){AverageAnonymityTime/=AnonymousVehicleCount;}
		if(AverageAnonymitySamples==0)
		{
			AverageAnonymitySetSize=0.0f;
			AverageAnonymityDistance=0.0f;
		}
		else
		{
			AverageAnonymitySetSize/=AverageAnonymitySamples;
			AverageAnonymityDistance/=AverageAnonymitySamples;
		}
		OutputVariables.Set("AverageAnonymitySetSize",to_string((long double)AverageAnonymitySetSize));
		OutputVariables.Set("PeakAnonymitySetSize",to_string((long long int)PeakAnonymitySetSize));
		OutputVariables.Set("AverageAnonymityDistance",to_string((long double)AverageAnonymityDistance));
		OutputVariables.Set("PeakAnonymityDistance",to_string((long double)PeakAnonymityDistance));
		OutputVariables.Set("AverageAnonymityTime",to_string((long double)AverageAnonymityTime));
		OutputVariables.Set("PeakAnonymityTime",to_string((long double)PeakAnonymityTime));
		OutputVariables.Set("AnonymousVehicleCount",to_string((long long int)AnonymousVehicleCount));
		OutputVariables.Set("TotalAnonymitySets",to_string((long long int)AnonymitySets.Count()));
		
		//This loop calculates the most congested networked unit.
		int CongestionCount=0;
		NetworkObject* CongestedUnit=NULL;
		for(int i=0;i<NetworkObjects.Count();i++)
		{
			NetworkObject* CurrentCounter=NetworkObjects[i];
			int MaxPackets=CurrentCounter->NetworkMessages.GetMostPacketsTransmitted();
			if(MaxPackets>CongestionCount)
			{
				CongestedUnit=CurrentCounter;
				CongestionCount=MaxPackets;
			}
		}
		if(CongestedUnit==NULL){OutputVariables.Set("PeakTrafficUnit","None");}
		else{OutputVariables.Set("PeakTrafficUnit",CongestedUnit->Title);}
		OutputVariables.Set("PeakTrafficAmount",to_string((long long int)CongestionCount));
		
		//This loop adds the traffic counters to the list of OutputVariables.
		for(int i=0;i<=TOTAL_TRAFFIC_TYPES;i++){OutputVariables.Set(string("Packet Count: ")+GetEnumName((TrafficType)i),to_string((long long int)GlobalTraffic[i]));}
		OutputVariables.Set(string("Packet Count: Overhead"),to_string((long long int)GlobalTraffic.Overhead()));
		OutputVariables.Set(string("Packet Count: Overhead Ratio"),to_string((long double)GlobalTraffic.OverheadRatio()));
		OutputVariables.Set(string("Packet Count: Successful"),to_string((long long int)GlobalTraffic.TotalTransmitted()));
		OutputVariables.Set(string("Packet Count: Lost"),to_string((long long int)GlobalTraffic.TotalLost()));
		OutputVariables.Set(string("PacketLoss"),to_string((long double)GlobalTraffic.PacketLoss()));
		OutputVariables.Set(string("Throughput"),to_string((long double)GlobalTraffic.TotalTransmitted()/TimeMax));
		
		//OUTPUT
		if(LegacyOutput)
		{
			cout<<"RUN:";
			for(int i=0;i<MAX_PARAMETERS_PER_LINE&&RunParameterNames[i].length()>0;i++)
			{
				if(i>0){cout<<",";}
				cout<<RunParameterNames[i]<<"="<<setw(3)<<RunVariables.Get(RunParameterNames[i]);
			}
			cout<<": ";
			cout<<fixed<<
				"K="<<setw(5)<<setprecision(2)<<AverageAnonymitySetSize<<"("<<AnonymitySets.Count()<<"), "<<
				"D="<<setw(6)<<setprecision(2)<<AverageAnonymityDistance<<", "<<
				"T="<<setw(5)<<setprecision(2)<<AverageAnonymityTime<<", "<<
				"Anonymous="<<setw(4)<<AnonymousVehicleCount<<"/"<<setw(4)<<VehicleMax;
			for(int i=0;i<=TOTAL_TRAFFIC_TYPES;i++){cout<<", "<<GetEnumName((TrafficType)i)<<":"<<GlobalTraffic[i];}
			if(CongestedUnit!=NULL){cout<<", Congested: "<<CongestedUnit->Title<<"("<<CongestionCount<<")";}
			cout<<endl;
		}
		else if(VerboseOutput)
		{
			for(int i=0;i<OutputVariables.Count();i++)
			{
				if(i!=0){cout<<", ";}
				cout<<OutputVariables.GetKey(i)<<"="<<OutputVariables.Get(i);
			}
			cout<<endl;
		}
		else
		{
			//TODO: Get the output to interpret different variable types.
			int FormatNeedle=0;//This advances through the format string as we process each section.
			while(FormatNeedle<OutputFormat->size())
			{
				//These first few lines find the next special character, or the end of the string, and OutputVariables any part of the string which was passed over.
				size_t FoundIndex=OutputFormat->find(OUTPUT_FORMAT_DELIMITER,FormatNeedle);
				if(FoundIndex==string::npos){FoundIndex=OutputFormat->size();}
				cout<<OutputFormat->substr(FormatNeedle,FoundIndex-FormatNeedle);
				FormatNeedle=FoundIndex+1;
				//If the end of the string was not reached, we then try to parse the next block of characters, which are special notation.
				if(FoundIndex<OutputFormat->size())
				{
					FoundIndex=OutputFormat->find(OUTPUT_FORMAT_DELIMITER,FormatNeedle);
					if(FoundIndex==string::npos)
					{
						cerr<<"ERROR: Misformatted Output String: No Closing '"<<OUTPUT_FORMAT_DELIMITER<<"'!"<<endl;
						break;
					}
					string FormatSpecifierArguments("s");
					size_t FoundSubIndex=OutputFormat->find(OUTPUT_FORMAT_SUB_DELIMITER,FormatNeedle);
					if(FoundSubIndex!=string::npos)
					{
						FormatSpecifierArguments=OutputFormat->substr(FormatNeedle,FoundSubIndex-FormatNeedle);
						FormatNeedle=FoundSubIndex+1;
					}
					string OutputValue("Unspecified");
					string VariableName=OutputFormat->substr(FormatNeedle,FoundIndex-FormatNeedle);
					if(OutputVariables.HasKey(VariableName)){OutputValue=OutputVariables.Get(VariableName);}
					string FormatSpecifier("%");
					FormatSpecifier=FormatSpecifier+FormatSpecifierArguments;
					char VariableTypeSpecifier=FormatSpecifier.at(FormatSpecifier.size()-1);
					switch(VariableTypeSpecifier)
					{
					case 'd': case 'i': case 'u':
						printf(FormatSpecifier.c_str(),atoi(OutputValue.c_str()));
						break;
					case 'f': case 'F':
						printf(FormatSpecifier.c_str(),atof(OutputValue.c_str()));
						break;
					default:
						printf(FormatSpecifier.c_str(),OutputValue.c_str());
						break;
					}
					FormatNeedle=FoundIndex+1;
				}
			}
			cout<<endl<<flush;//We have to use this operator to push a newline to cout, or the Cloud9 interface will cache it in a strange way.
		}
	}
	//CLEANUP
	fflush(NULL);
	TraceFile.close();
	while(AnonymitySets.Count()>0){delete AnonymitySets.PopObj();}
	while(Vehicles.Count()>0){delete Vehicles.PopObj();}
}

// ----- function: inrange --------------------------------------------------
// returns true if (x1,y1) is within distance r of (x2,y2); else returns false
bool inrange(float x1, float y1, float x2, float y2, float r)
{
	if(r*r>=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)){return true;}
	else{return false;}
}

// ----- function: get_distance ---------------------------------------------
float get_distance(float x1, float y1, float x2, float y2)
{
	return sqrt ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}