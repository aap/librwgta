#include "III.h"

#define SMALLSTRIPHEIGHT 4.0f
#define HORIZSTRIPHEIGHT 32.0f	// Vice City, actually 48 in III

float CClouds::ms_cameraRoll;
float CClouds::ms_horizonZ;
CRGBA CClouds::ms_colourTop;
CRGBA CClouds::ms_colourBottom;
// Vice City
CRGBA CClouds::ms_colourBkGrd;

bool
UseDarkBackground(void)
{
	return TheCamera.m_rwcam->getFrame()->getLTM()->up.z < -0.9f;
}

void
CClouds::RenderBackground(int16 topred, int16 topgreen, int16 topblue,
	int16 botred, int16 botgreen, int16 botblue, int16 alpha)
{
	rw::Matrix *mat = TheCamera.m_rwcam->getFrame()->getLTM();
	float c = sqrt(mat->right.x * mat->right.x + mat->right.y * mat->right.y);
	if(c > 1.0f)
		c = 1.0f;
	ms_cameraRoll = acos(c);
	if(mat->right.z < 0.0f)
		ms_cameraRoll = -ms_cameraRoll;

	if(UseDarkBackground()){
		ms_colourTop.r = 50;
		ms_colourTop.g = 50;
		ms_colourTop.b = 50;
		ms_colourTop.a = 255;
		ms_colourBottom = ms_colourTop;
		CRect r(0, 0, globals.width, globals.height);
		Sprite2d::DrawRect(r, ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
	}else{
		ms_horizonZ = Sprite::CalcHorizonCoors();

		// Draw top/bottom gradient
		float gradheight = globals.height/2.0f;
		float topedge = ms_horizonZ - gradheight;
		float botpos, toppos;
		if(ms_horizonZ > 0.0f && topedge < globals.height){
			ms_colourTop.r = topred;
			ms_colourTop.g = topgreen;
			ms_colourTop.b = topblue;
			ms_colourTop.a = alpha;
			ms_colourBottom.r = botred;
			ms_colourBottom.g = botgreen;
			ms_colourBottom.b = botblue;
			ms_colourBottom.a = alpha;

			if(ms_horizonZ < globals.height)
				botpos = ms_horizonZ;
			else{
				float f = (ms_horizonZ - globals.height)/gradheight;
				ms_colourBottom.r = topred*f + (1.0f-f)*botred;
				ms_colourBottom.g = topgreen*f + (1.0f-f)*botgreen;
				ms_colourBottom.b = topblue*f + (1.0f-f)*botblue;
				botpos = globals.height;
			}
			if(topedge >= 0.0f)
				toppos = topedge;
			else{
				float f = (0.0f - topedge)/gradheight;
				ms_colourTop.r = botred*f + (1.0f-f)*topred;
				ms_colourTop.g = botgreen*f + (1.0f-f)*topgreen;
				ms_colourTop.b = botblue*f + (1.0f-f)*topblue;
				toppos = 0.0f;
			}
			Sprite2d::DrawRect(CRect(0, toppos, globals.width, botpos),
				ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
		}

		// draw the small stripe (whatever it's supposed to be)
		if(ms_horizonZ > -SMALLSTRIPHEIGHT && ms_horizonZ < globals.height){
			// Same colour as fog
			ms_colourTop.r = (topred + 2 * botred) / 3;
			ms_colourTop.g = (topgreen + 2 * botgreen) / 3;
			ms_colourTop.b = (topblue + 2 * botblue) / 3;
			Sprite2d::DrawRect(CRect(0, ms_horizonZ, globals.width, ms_horizonZ+SMALLSTRIPHEIGHT),
				ms_colourTop, ms_colourTop, ms_colourTop, ms_colourTop);
		}

		// Only top
		if(topedge > 0.0f){
			ms_colourTop.r = topred;
			ms_colourTop.g = topgreen;
			ms_colourTop.b = topblue;
			ms_colourTop.a = alpha;
			ms_colourBottom.r = topred;
			ms_colourBottom.g = topgreen;
			ms_colourBottom.b = topblue;
			ms_colourBottom.a = alpha;

			if(topedge > globals.height)
				botpos = globals.height;
			else
				botpos = topedge;
			Sprite2d::DrawRect(CRect(0, 0, globals.width, botpos),
				ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
		}

		// Set both to fog colour for RenderHorizon
		ms_colourTop.r = (topred + 2 * botred) / 3;
		ms_colourTop.g = (topgreen + 2 * botgreen) / 3;
		ms_colourTop.b = (topblue + 2 * botblue) / 3;
		ms_colourBottom.r = (topred + 2 * botred) / 3;
		ms_colourBottom.g = (topgreen + 2 * botgreen) / 3;
		ms_colourBottom.b = (topblue + 2 * botblue) / 3;
	}
}

void
CClouds::RenderHorizon(void)
{
	if(UseDarkBackground())
		return;

	ms_colourBottom.a = 230;
	ms_colourTop.a = 80;

	if(ms_horizonZ >= globals.height)
		return;

	float z1 = ms_horizonZ + SMALLSTRIPHEIGHT;
	if(z1 > globals.height)
		z1 = globals.height;
	Sprite2d::DrawRectXLU(CRect(0, ms_horizonZ, globals.width, z1),
		ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);

	// The rest is more or less from Vice City. III does strange things

	float z2 = z1 + HORIZSTRIPHEIGHT;
	if(z2 > globals.height)
		z2 = globals.height;

	// comes from ambient colour in VC
	ms_colourBkGrd.r = 100;
	ms_colourBkGrd.g = 100;
	ms_colourBkGrd.b = 100;
	ms_colourBkGrd.a = 255;

	Sprite2d::DrawRectXLU(CRect(0, z1, globals.width, z2),
		ms_colourBkGrd, ms_colourBkGrd, ms_colourTop, ms_colourTop);

	Sprite2d::DrawRectXLU(CRect(0, z2, globals.width, globals.height),
		ms_colourBkGrd, ms_colourBkGrd, ms_colourBkGrd, ms_colourBkGrd);
}
