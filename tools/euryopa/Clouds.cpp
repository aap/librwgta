#include "euryopa.h"

namespace Clouds
{

#define SMALLSTRIPHEIGHT 4.0f
#define HORIZSTRIPHEIGHT 32.0f	// Vice City, actually 48 in III

static float ms_cameraRoll;
static float ms_horizonZ;
static CRGBA ms_colourTop;
static CRGBA ms_colourBottom;
// Vice City
static CRGBA ms_colourBkGrd;

bool
UseDarkBackground(void)
{
	return TheCamera.m_rwcam->getFrame()->getLTM()->up.z < -0.9f;
}

void
RenderBackground(int16 topred, int16 topgreen, int16 topblue,
	int16 botred, int16 botgreen, int16 botblue, int16 alpha)
{
	int w, h;
// currently unused, only for VC where horizon can tilt
//	rw::Matrix *mat = TheCamera.m_rwcam->getFrame()->getLTM();
//	float c = sqrt(mat->right.x * mat->right.x + mat->right.y * mat->right.y);
//	if(c > 1.0f)
//		c = 1.0f;
//	ms_cameraRoll = acos(c);
//	if(mat->right.z < 0.0f)
//		ms_cameraRoll = -ms_cameraRoll;

	w = sk::globals.width;
	h = sk::globals.height;

	if(UseDarkBackground()){
		ms_colourTop.r = 50;
		ms_colourTop.g = 50;
		ms_colourTop.b = 50;
		ms_colourTop.a = 255;
		ms_colourBottom = ms_colourTop;
		CRect r(0, 0, w, h);
		Sprite2d::DrawRect(r, ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
	}else{
		ms_horizonZ = Sprite::CalcHorizonCoors();

		// Draw top/bottom gradient
		float gradheight = h/2.0f;
		float topedge = ms_horizonZ - gradheight;
		float botpos, toppos;
		if(ms_horizonZ > 0.0f && topedge < h){
			ms_colourTop.r = topred;
			ms_colourTop.g = topgreen;
			ms_colourTop.b = topblue;
			ms_colourTop.a = alpha;
			ms_colourBottom.r = botred;
			ms_colourBottom.g = botgreen;
			ms_colourBottom.b = botblue;
			ms_colourBottom.a = alpha;

			if(ms_horizonZ < h)
				botpos = ms_horizonZ;
			else{
				float f = (ms_horizonZ - h)/gradheight;
				ms_colourBottom.r = topred*f + (1.0f-f)*botred;
				ms_colourBottom.g = topgreen*f + (1.0f-f)*botgreen;
				ms_colourBottom.b = topblue*f + (1.0f-f)*botblue;
				botpos = h;
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
			Sprite2d::DrawRect(CRect(0, toppos, w, botpos),
				ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
		}

		// draw the small stripe (whatever it's supposed to be)
		if(ms_horizonZ > -SMALLSTRIPHEIGHT && ms_horizonZ < h){
			// Same colour as fog
			ms_colourTop.r = (topred + 2 * botred) / 3;
			ms_colourTop.g = (topgreen + 2 * botgreen) / 3;
			ms_colourTop.b = (topblue + 2 * botblue) / 3;
			Sprite2d::DrawRect(CRect(0, ms_horizonZ, w, ms_horizonZ+SMALLSTRIPHEIGHT),
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

			if(topedge > h)
				botpos = h;
			else
				botpos = topedge;
			Sprite2d::DrawRect(CRect(0, 0, w, botpos),
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
RenderHorizon(void)
{
	int w, h;
	w = sk::globals.width;
	h = sk::globals.height;

	if(UseDarkBackground())
		return;

	ms_colourBottom.a = 230;
	ms_colourTop.a = 80;

	if(ms_horizonZ >= h)
		return;

	float z1 = ms_horizonZ + SMALLSTRIPHEIGHT;
	if(z1 > h)
		z1 = h;
	Sprite2d::DrawRectXLU(CRect(0, ms_horizonZ, w, z1),
		ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);

	// The rest is more or less from Vice City. III does strange things

	float z2 = z1 + HORIZSTRIPHEIGHT;
	if(z2 > h)
		z2 = h;

	// comes from ambient colour in VC
	ms_colourBkGrd.r = 100;
	ms_colourBkGrd.g = 100;
	ms_colourBkGrd.b = 100;
	ms_colourBkGrd.a = 255;

	Sprite2d::DrawRectXLU(CRect(0, z1, w, z2),
		ms_colourBkGrd, ms_colourBkGrd, ms_colourTop, ms_colourTop);

	Sprite2d::DrawRectXLU(CRect(0, z2, w, h),
		ms_colourBkGrd, ms_colourBkGrd, ms_colourBkGrd, ms_colourBkGrd);
}


#define TEMPBUFFERVERTSIZE 40
#define TEMPBUFFERINDEXSIZE 60
static int TempBufferIndicesStored;
static int TempBufferVerticesStored;
static rw::RWDEVICE::Im3DVertex TempVertexBuffer[TEMPBUFFERVERTSIZE];
static uint16 TempIndexBuffer[TEMPBUFFERINDEXSIZE];

static void
RenderAndEmptyRenderBuffer(void)
{
	assert(TempBufferVerticesStored <= TEMPBUFFERVERTSIZE);
	assert(TempBufferIndicesStored <= TEMPBUFFERINDEXSIZE);
	if(TempBufferVerticesStored){
		rw::im3d::Transform(TempVertexBuffer, TempBufferVerticesStored, nil);
		rw::im3d::RenderIndexed(rw::PRIMTYPETRILIST, TempIndexBuffer, TempBufferIndicesStored);
		rw::im3d::End();
	}
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}

void
SetUpOneSkyPoly(rw::V3d tl, rw::V3d tr, rw::V3d bl, rw::V3d br, rw::RGBA coltop, rw::RGBA colbot)
{
	if(TempBufferVerticesStored+4 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+6 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	int i = TempBufferVerticesStored;
	TempVertexBuffer[i + 0].setX(tl.x);
	TempVertexBuffer[i + 0].setY(tl.y);
	TempVertexBuffer[i + 0].setZ(tl.z);
	TempVertexBuffer[i + 0].setU(0.0f);
	TempVertexBuffer[i + 0].setV(0.0f);
	TempVertexBuffer[i + 0].setColor(coltop.red, coltop.green, coltop.blue, coltop.alpha);

	TempVertexBuffer[i + 1].setX(tr.x);
	TempVertexBuffer[i + 1].setY(tr.y);
	TempVertexBuffer[i + 1].setZ(tr.z);
	TempVertexBuffer[i + 1].setU(0.0f);
	TempVertexBuffer[i + 1].setV(0.0f);
	TempVertexBuffer[i + 1].setColor(coltop.red, coltop.green, coltop.blue, coltop.alpha);

	TempVertexBuffer[i + 2].setX(bl.x);
	TempVertexBuffer[i + 2].setY(bl.y);
	TempVertexBuffer[i + 2].setZ(bl.z);
	TempVertexBuffer[i + 2].setU(0.0f);
	TempVertexBuffer[i + 2].setV(0.0f);
	TempVertexBuffer[i + 2].setColor(colbot.red, colbot.green, colbot.blue, colbot.alpha);

	TempVertexBuffer[i + 3].setX(br.x);
	TempVertexBuffer[i + 3].setY(br.y);
	TempVertexBuffer[i + 3].setZ(br.z);
	TempVertexBuffer[i + 3].setU(0.0f);
	TempVertexBuffer[i + 3].setV(0.0f);
	TempVertexBuffer[i + 3].setColor(colbot.red, colbot.green, colbot.blue, colbot.alpha);

	TempBufferVerticesStored += 4;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+3;
}

void
RenderSkyPolys(void)
{
	const float dist = 30.0f;

	rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);
	rw::SetRenderState(rw::VERTEXALPHA, 0);
	rw::SetRenderState(rw::FOGENABLE, 0);
	rw::SetRenderState(rw::ZTESTENABLE, 0);
	rw::SetRenderState(rw::ZWRITEENABLE, 0);
	rw::SetRenderState(rw::CULLMODE, rw::CULLNONE);

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

	rw::V3d at = TheCamera.m_rwcam->getFrame()->getLTM()->at;
	rw::V3d pos = TheCamera.m_position;
	at.z = 0.0f;
	at = normalize(at);
	rw::V3d right = { at.y*1.4f, -at.x*1.4f, 0.0f };
	float f = (pos.z - 25.0f)/80.0f;	// below horizon fog
	f = clampFloat(f, 0.0f, 1.0f);

	rw::RGBA skytop, skybot, belowhoriz;
	rw::convColor(&skytop, &Timecycle::currentColours.skyTop);
	rw::convColor(&skybot, &Timecycle::currentColours.skyBottom);
	belowhoriz.red = Timecycle::belowHorizonColour.red*(1.0f-f) + skybot.red*f;
	belowhoriz.green = Timecycle::belowHorizonColour.green*(1.0f-f) + skybot.green*f;
	belowhoriz.blue = Timecycle::belowHorizonColour.blue*(1.0f-f) + skybot.blue*f;


	rw::V3d frontright = add(pos, scale(add(at, right), dist));
	rw::V3d frontleft = add(pos, scale(sub(at, right), dist));
	rw::V3d backright = sub(pos, scale(sub(at, right), dist));
	rw::V3d backleft = sub(pos, scale(add(at, right), dist));
	rw::V3d topmost = { 0.0f, 0.0f, 0.5f*dist };
	rw::V3d middle = { 0.0f, 0.0f, -0.1f*dist };
	rw::V3d botmost = { 0.0f, 0.0f, -0.3f*dist };

	rw::V3d tl, tr, bl, br;
	tl = add(frontleft, topmost);
	tr = add(frontright, topmost);
	bl = frontleft;
	br = frontright;
	SetUpOneSkyPoly(tl, tr, bl, br, skytop, skybot);

	tl = frontleft;
	tr = frontright;
	bl = add(frontleft, middle);
	br = add(frontright, middle);
	SetUpOneSkyPoly(tl, tr, bl, br, skybot, skybot);

	tl = add(frontleft, middle);
	tr = add(frontright, middle);
	bl = add(frontleft, botmost);
	br = add(frontright, botmost);
	SetUpOneSkyPoly(tl, tr, bl, br, skybot, belowhoriz);

	tl = add(frontleft, topmost);
	tr = add(frontright, topmost);
	bl = add(backleft, topmost);
	br = add(backright, topmost);
	SetUpOneSkyPoly(tl, tr, bl, br, skytop, skytop);

	tl = add(frontleft, botmost);
	tr = add(frontright, botmost);
	bl = add(backleft, botmost);
	br = add(backright, botmost);
	SetUpOneSkyPoly(tl, tr, bl, br, belowhoriz, belowhoriz);

	RenderAndEmptyRenderBuffer();

	rw::SetRenderState(rw::ZTESTENABLE, 1);
	rw::SetRenderState(rw::ZWRITEENABLE, 1);
}

}
