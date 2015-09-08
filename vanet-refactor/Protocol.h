#pragma once
class Protocol
{
public:
    virtual bool SimulationBegin()=0;
    //Attempting to initialize certain things during the constructor causes issues because many global variables have not yet been initialized. Instead, we wait until the beginning of the simulation, after the global environment has been created.
    virtual bool SimulationTick()=0;
    //SimulationTick is called once after each iteration, which in most cases will be after ALL vehicles have moved, and one second has passed.
    //Returns true of the simulation can continue, or false if it encountered an error and must halt.
    virtual void SimulationEnd()=0;
    //SimulationEnd is called after all iterations have completed, and is responsible for generating output.
};