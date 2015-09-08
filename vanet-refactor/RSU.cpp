#include "RSU.h"
#include "Globals.h"
RSU::RSU()
{
	x=0;
	y=0;
	Radius=0;
}
RSU::RSU(int x, int y, float Radius, std::string Title)
{
	this->x=x;
	this->y=y;
	this->Radius=Radius;
	this->Title=Title;
}