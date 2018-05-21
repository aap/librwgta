#ifndef _SPRITE_H_
#define _SPRITE_H_

class Sprite2d
{
	static rw::RWDEVICE::Im2DVertex ma_vertices[4];
public:
	static void DrawRect(CRect const &r, CRGBA const &c0, CRGBA const &c1, CRGBA const &c2, CRGBA const &c3);
	static void DrawRectXLU(CRect const &r, CRGBA const &c0, CRGBA const &c1, CRGBA const &c2, CRGBA const &c3);
	static void SetVertices(CRect const &r, CRGBA const &c0, CRGBA const &c1, CRGBA const &c2, CRGBA const &c3, bool far);
};

class Sprite
{
public:
	static float CalcHorizonCoors(void);
};

#endif
