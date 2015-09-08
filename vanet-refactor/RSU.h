#pragma once
#include "NetworkObject.h"
#include "ArrayList.h"

class RSU:public NetworkObject
{
public:
	float Radius;
	RSU();
	RSU(int x, int y, float Radius, std::string Title);
};