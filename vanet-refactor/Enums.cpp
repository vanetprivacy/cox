#include "Enums.h"
const char* GetEnumName(TrafficType E)
{
	switch(E)
	{
	case NOMINATE: return "NOMINATE";
	case ANNOUNCEMENT: return "ANNOUNCE";
	case JOIN: return "JOIN";
	case JOIN_CONFIRMATION: return "JOIN CONFIRM";
	case APPLICATION: return "APPLICATION";
	case DUMMY: return "DUMMY";
	case TOTAL_TRAFFIC_TYPES: return "TOTAL";
	default: return "Unnamed";
	}
}