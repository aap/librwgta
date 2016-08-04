class CTimer
{
public:
	static int   m_snPreviousTimeInMilliseconds;
	static int   m_snTimeInMilliseconds;
	static uint  m_fFrameCount;
	static float ms_fTimeScale;
	static float ms_fTimeStep;
	static bool  ms_UserPause;
	static bool  ms_CodePause;

	static void Initialise(void);
};
