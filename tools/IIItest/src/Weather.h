class CWeather
{
public:
	static float InterpolationValue;
	static short NewWeatherType;
	static short OldWeatherType;
	static short ForcedWeatherType;

	static void Init(void);
	static void Update(void);
};
