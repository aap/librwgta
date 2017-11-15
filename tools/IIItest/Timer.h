class CTimer
{
public:
	static int   m_snPreviousTimeInMilliseconds;
	static int   m_snTimeInMilliseconds;
	static uint  m_FrameCounter;
	static float ms_fTimeScale;
	static float ms_fTimeStep;
	static bool  m_UserPause;
	static bool  m_CodePause;

	static void Initialise(void);
	static void Update(void);
	static void Stop(void);
};
