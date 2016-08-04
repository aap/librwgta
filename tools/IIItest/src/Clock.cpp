#include "III.h"

uchar  CClock::ms_nGameClockHours;
uchar  CClock::ms_nGameClockMinutes;
ushort CClock::ms_nGameClockSeconds;
uchar  CClock::ms_Stored_nGameClockHours;
uchar  CClock::ms_Stored_nGameClockMinutes;
ushort CClock::ms_Stored_nGameClockSeconds;
uint   CClock::ms_nMillisecondsPerGameMinute;
int    CClock::ms_nLastClockTick;
bool   CClock::ms_bClockHasBeenStored;

void
CClock::Initialise(uint scale)
{
	debug("Initialising CClock...\n");
	ms_nGameClockHours = 12;
	ms_nGameClockMinutes = 0;
	ms_nGameClockSeconds = 0;
	ms_nMillisecondsPerGameMinute = scale;
	ms_nLastClockTick = CTimer::m_snTimeInMilliseconds;
	ms_bClockHasBeenStored = false;
	debug("CClock ready\n");
}

bool
CClock::GetIsTimeInRange(uchar h1, uchar h2)
{
	if(h1 > h2)
		return ms_nGameClockHours >= h1 || ms_nGameClockHours < h2;
	else
		return ms_nGameClockHours >= h1 && ms_nGameClockHours < h2;
}
