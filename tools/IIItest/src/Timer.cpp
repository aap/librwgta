#include "III.h"

int   CTimer::m_snPreviousTimeInMilliseconds;
int   CTimer::m_snTimeInMilliseconds;
uint  CTimer::m_fFrameCount;
float CTimer::ms_fTimeScale;
float CTimer::ms_fTimeStep;
bool  CTimer::ms_UserPause;
bool  CTimer::ms_CodePause;


void
CTimer::Initialise(void)
{
	debug("Initialising CTimer...\n");
	m_snPreviousTimeInMilliseconds = 0;
	m_snTimeInMilliseconds = m_snPreviousTimeInMilliseconds;
	m_fFrameCount = 0;
	ms_fTimeScale = 1.0f;
	ms_fTimeStep = 1.0f;
	ms_UserPause = false;
	ms_CodePause = false;
	debug("CTimer ready\n");
}
