#include "Events.h"

inline bool TEventKey::operator <  (const TEventKey & Other) const
{
	if (TimeStamp != Other.TimeStamp) return TimeStamp < Other.TimeStamp;
	if (Index != Other.Index) return Index < Other.Index;
	return false;
}

inline bool TEventKey::operator == (const TEventKey & Other) const
{
	return (TimeStamp == Other.TimeStamp) && (Index == Other.Index);
}

inline bool TEventKey::operator >  (const TEventKey & Other) const
{	
	if (TimeStamp != Other.TimeStamp) return TimeStamp > Other.TimeStamp;
	if (Index != Other.Index) return Index > Other.Index;
	return false;
}

/*
CEvents::CEvents(void)
{

}

CEvents::~CEvents(void)
{

}
*/