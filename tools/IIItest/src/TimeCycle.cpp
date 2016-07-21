#include "III.h"

int   CTimeCycle::m_nAmbientRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nAmbientBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nDirectionalRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nDirectionalGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nDirectionalBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyTopRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyTopGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyTopBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyBottomRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyBottomGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSkyBottomBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoreRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoreGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoreBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoronaRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoronaGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nSunCoronaBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fSunSize[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fSpriteSize[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fSpriteBrightness[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nShadowStrength[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLightShadowStrength[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nTreeShadowStrength[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fFogStart[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fFarClip[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fLightsOnGroundBrightness[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLowCloudsRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLowCloudsGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nLowCloudsBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsTopRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsTopGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsTopBlue[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsBottomRed[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsBottomGreen[NUMHOURS][NUMWEATHERS];
int   CTimeCycle::m_nFluffyCloudsBottomBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurRed[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurGreen[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurBlue[NUMHOURS][NUMWEATHERS];
float CTimeCycle::m_fBlurAlpha[NUMHOURS][NUMWEATHERS];

void
CTimeCycle::Initialise(void)
{
	FILE *file;
	char *line;
	if(file = fopen_ci("data/timecyc.dat", "rb"), file == nil)
		return;
	int h = 0, w = 0;
	while(line = CFileLoader::LoadLine(file)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		sscanf(line, "%d %d %d  %d %d %d  %d %d %d  %d %d %d "
		             "%d %d %d  %d %d %d  %f %f %f %d %d %d %f %f %f "
		             "%d %d %d  %d %d %d  %d %d %d  %f %f %f %f",
		       &CTimeCycle::m_nAmbientRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nAmbientGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nAmbientBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nDirectionalRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nDirectionalGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nDirectionalBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSkyTopRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSkyTopGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSkyTopBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSkyBottomRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSkyBottomGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSkyBottomBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSunCoreRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSunCoreGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSunCoreBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSunCoronaRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSunCoronaGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nSunCoronaBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fSunSize[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fSpriteSize[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fSpriteBrightness[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nShadowStrength[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nLightShadowStrength[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nTreeShadowStrength[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fFogStart[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fFarClip[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fLightsOnGroundBrightness[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nLowCloudsRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nLowCloudsGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nLowCloudsBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nFluffyCloudsTopRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nFluffyCloudsTopGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nFluffyCloudsTopBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nFluffyCloudsBottomRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nFluffyCloudsBottomGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_nFluffyCloudsBottomBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fBlurRed[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fBlurGreen[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fBlurBlue[NUMHOURS][NUMWEATHERS],
		       &CTimeCycle::m_fBlurAlpha[NUMHOURS][NUMWEATHERS]);
		h++;
		if(h == NUMHOURS){
			h = 0;
			w++;
		}
		if(w == NUMWEATHERS)
			break;
	}
	fclose(file);
}
