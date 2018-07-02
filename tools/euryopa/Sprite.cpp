#include "euryopa.h"

rw::RWDEVICE::Im2DVertex Sprite2d::ma_vertices[4];

// Arguments:
// 2---3
// |   |
// 0---1
void
Sprite2d::SetVertices(CRect const &r, CRGBA const &c0, CRGBA const &c1, CRGBA const &c2, CRGBA const &c3, bool far_)
{
	float screenz, z, recipz;

	if(far_){
		screenz = rw::im2d::GetFarZ();
		z = Scene.camera->farPlane;
	}else{
		screenz = rw::im2d::GetNearZ();
		z = Scene.camera->nearPlane;
	}
	recipz = 1.0f/z;

	// This is what we draw:
	// 0---1
	// | / |
	// 3---2
	ma_vertices[0].setScreenX(r.left);
	ma_vertices[0].setScreenY(r.bottom);
	ma_vertices[0].setScreenZ(screenz);
	ma_vertices[0].setCameraZ(z);
	ma_vertices[0].setRecipCameraZ(recipz);
	ma_vertices[0].setColor(c2.r, c2.g, c2.b, c2.a);
	ma_vertices[0].setU(0.0f, recipz);
	ma_vertices[0].setV(0.0f, recipz);

	ma_vertices[1].setScreenX(r.right);
	ma_vertices[1].setScreenY(r.bottom);
	ma_vertices[1].setScreenZ(screenz);
	ma_vertices[1].setCameraZ(z);
	ma_vertices[1].setRecipCameraZ(recipz);
	ma_vertices[1].setColor(c3.r, c3.g, c3.b, c3.a);
	ma_vertices[1].setU(1.0f, recipz);
	ma_vertices[1].setV(0.0f, recipz);

	ma_vertices[2].setScreenX(r.right);
	ma_vertices[2].setScreenY(r.top);
	ma_vertices[2].setScreenZ(screenz);
	ma_vertices[2].setCameraZ(z);
	ma_vertices[2].setRecipCameraZ(recipz);
	ma_vertices[2].setColor(c1.r, c1.g, c1.b, c1.a);
	ma_vertices[2].setU(1.0f, recipz);
	ma_vertices[2].setV(1.0f, recipz);

	ma_vertices[3].setScreenX(r.left);
	ma_vertices[3].setScreenY(r.top);
	ma_vertices[3].setScreenZ(screenz);
	ma_vertices[3].setCameraZ(z);
	ma_vertices[3].setRecipCameraZ(recipz);
	ma_vertices[3].setColor(c0.r, c0.g, c0.b, c0.a);
	ma_vertices[3].setU(0.0f, recipz);
	ma_vertices[3].setV(1.0f, recipz);
}

void
Sprite2d::DrawRect(CRect const &r, CRGBA const &c0, CRGBA const &c1, CRGBA const &c2, CRGBA const &c3)
{
	static short quadindices[] = { 0, 1, 2, 0, 2, 3 };
	SetVertices(r, c0, c1, c2, c3, false);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);
	rw::SetRenderState(rw::VERTEXALPHA, 0);
	rw::SetRenderState(rw::ZTESTENABLE, 0);
	rw::SetRenderState(rw::ZWRITEENABLE, 0);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&ma_vertices, 4, &quadindices, 6);
	rw::SetRenderState(rw::ZTESTENABLE, 1);
	rw::SetRenderState(rw::ZWRITEENABLE, 1);
}

void
Sprite2d::DrawRectXLU(CRect const &r, CRGBA const &c0, CRGBA const &c1, CRGBA const &c2, CRGBA const &c3)
{
	static short quadindices[] = { 0, 1, 2, 0, 2, 3 };
	SetVertices(r, c0, c1, c2, c3, false);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);
	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::ZTESTENABLE, 0);
	rw::SetRenderState(rw::ZWRITEENABLE, 0);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&ma_vertices, 4, &quadindices, 6);
	rw::SetRenderState(rw::ZTESTENABLE, 1);
	rw::SetRenderState(rw::ZWRITEENABLE, 1);
}




float 
Sprite::CalcHorizonCoors(void)
{
	rw::Matrix *mat = TheCamera.m_rwcam->getFrame()->getLTM();
	rw::Matrix view;

	rw::V3d pos = mat->pos;
	pos.z = 0.0f;
	pos.x += 3000.0f * mat->at.x;
	pos.y += 3000.0f * mat->at.y;
	rw::Matrix::invert(&view, mat);
	rw::V3d::transformPoints(&pos, &pos, 1, &TheCamera.m_rwcam->viewMatrix);
	return pos.y * sk::globals.height / pos.z;
}
