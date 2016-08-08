#include "III.h"

float CWeather::InterpolationValue;
short CWeather::NewWeatherType;
short CWeather::OldWeatherType;
short CWeather::ForcedWeatherType;

enum eWeather
{
	WEATHER_SUNNY = 0,
	WEATHER_CLOUDY,
	WEATHER_RAINY,
	WEATHER_FOGGY,
};

void
CWeather::Init(void)
{
	NewWeatherType = WEATHER_SUNNY;
//	OldWeatherType = WEATHER_CLOUDY;
	OldWeatherType = WEATHER_SUNNY;
	ForcedWeatherType = -1;
	InterpolationValue = 0.0f;
}

void
CWeather::Update(void)
{
	// nothing right now
}
