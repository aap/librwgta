#include "III.h"

int   CTimer::m_snPreviousTimeInMilliseconds;
int   CTimer::m_snTimeInMilliseconds;
uint  CTimer::m_FrameCounter;
float CTimer::ms_fTimeScale;
float CTimer::ms_fTimeStep;
bool  CTimer::m_UserPause;
bool  CTimer::m_CodePause;

int oldPcTimer;

void
CTimer::Initialise(void)
{
	debug("Initialising CTimer...\n");
	ms_fTimeScale = 1.0f;
	ms_fTimeStep = 1.0f;
	m_UserPause = false;
	m_CodePause = false;
	m_snPreviousTimeInMilliseconds = 0;
	m_snTimeInMilliseconds = m_snPreviousTimeInMilliseconds;
	m_FrameCounter = 0;
	oldPcTimer = getTimeInMS();
	debug("CTimer ready\n");
}

void
CTimer::Update(void)
{
	m_snPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
	int newtime = getTimeInMS();
	double diff = (newtime - oldPcTimer) * ms_fTimeScale;
	oldPcTimer = newtime;
	if(m_UserPause || m_CodePause)
		ms_fTimeStep = 0.0f;
	else{
		m_snTimeInMilliseconds += diff;
		ms_fTimeStep = diff*50.0f/1000.0f;
	}
	m_FrameCounter++;
}

void
CTimer::Stop(void)
{
	m_snPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
}
