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
		       &CTimeCycle::m_nAmbientRed[h][w],
		       &CTimeCycle::m_nAmbientGreen[h][w],
		       &CTimeCycle::m_nAmbientBlue[h][w],
		       &CTimeCycle::m_nDirectionalRed[h][w],
		       &CTimeCycle::m_nDirectionalGreen[h][w],
		       &CTimeCycle::m_nDirectionalBlue[h][w],
		       &CTimeCycle::m_nSkyTopRed[h][w],
		       &CTimeCycle::m_nSkyTopGreen[h][w],
		       &CTimeCycle::m_nSkyTopBlue[h][w],
		       &CTimeCycle::m_nSkyBottomRed[h][w],
		       &CTimeCycle::m_nSkyBottomGreen[h][w],
		       &CTimeCycle::m_nSkyBottomBlue[h][w],
		       &CTimeCycle::m_nSunCoreRed[h][w],
		       &CTimeCycle::m_nSunCoreGreen[h][w],
		       &CTimeCycle::m_nSunCoreBlue[h][w],
		       &CTimeCycle::m_nSunCoronaRed[h][w],
		       &CTimeCycle::m_nSunCoronaGreen[h][w],
		       &CTimeCycle::m_nSunCoronaBlue[h][w],
		       &CTimeCycle::m_fSunSize[h][w],
		       &CTimeCycle::m_fSpriteSize[h][w],
		       &CTimeCycle::m_fSpriteBrightness[h][w],
		       &CTimeCycle::m_nShadowStrength[h][w],
		       &CTimeCycle::m_nLightShadowStrength[h][w],
		       &CTimeCycle::m_nTreeShadowStrength[h][w],
		       &CTimeCycle::m_fFogStart[h][w],
		       &CTimeCycle::m_fFarClip[h][w],
		       &CTimeCycle::m_fLightsOnGroundBrightness[h][w],
		       &CTimeCycle::m_nLowCloudsRed[h][w],
		       &CTimeCycle::m_nLowCloudsGreen[h][w],
		       &CTimeCycle::m_nLowCloudsBlue[h][w],
		       &CTimeCycle::m_nFluffyCloudsTopRed[h][w],
		       &CTimeCycle::m_nFluffyCloudsTopGreen[h][w],
		       &CTimeCycle::m_nFluffyCloudsTopBlue[h][w],
		       &CTimeCycle::m_nFluffyCloudsBottomRed[h][w],
		       &CTimeCycle::m_nFluffyCloudsBottomGreen[h][w],
		       &CTimeCycle::m_nFluffyCloudsBottomBlue[h][w],
		       &CTimeCycle::m_fBlurRed[h][w],
		       &CTimeCycle::m_fBlurGreen[h][w],
		       &CTimeCycle::m_fBlurBlue[h][w],
		       &CTimeCycle::m_fBlurAlpha[h][w]);
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
