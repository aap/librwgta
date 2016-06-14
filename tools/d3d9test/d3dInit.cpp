#include <assert.h>
#include "d3dUtility.h"
//#include <DirectXMath.h>
//using namespace DirectX;

#define RW_D3D9
#include <rw.h>
#include "rwgta.h"
#include "camera.h"

IDirect3DDevice9 *Device = 0;

Camera *camera;

rw::Clump *clump;

void
initrw(void)
{
	gta::attachPlugins();
	rw::d3d::registerNativeRaster();
	rw::loadTextures = 1;

	rw::currentTexDictionary = rw::TexDictionary::create();
	rw::Image::setSearchPath("Y:\\ps2\\gta3\\MODELS\\gta3_archive\\txd_extracted\\;"
	                         "Y:\\ps2\\gtavc\\MODELS\\gta3_archive\\txd_extracted\\;"
	                         "Y:\\ps2\\gtasa\\models\\gta3_archive\\txd_extracted\\");

	rw::platform = rw::PLATFORM_D3D9;
	rw::d3d::device = Device;

	if(1){
		//char *filename = "Y:\\pc\\gtasa\\models\\gta3_archive\\admiral.txd";
		char *filename = "Y:\\pc\\gtavc\\models\\gta3_archive\\admiral.txd";
		rw::StreamFile in;
		if(in.open(filename, "rb") == NULL){
			MessageBox(0, "couldn't open file\n", 0, 0);
			printf("couldn't open file\n");
		}
		rw::findChunk(&in, rw::ID_TEXDICTIONARY, NULL, NULL);
		rw::TexDictionary *txd;
		txd = rw::TexDictionary::streamRead(&in);
		assert(txd);
		in.close();
		rw::currentTexDictionary = txd;
	}

	char *filename = "Y:\\pc\\gtavc\\models\\gta3_archive\\admiral.dff";
//	char *filename = "Y:\\pc\\gta3\\models\\gta3_archive\\kuruma.dff";
//	char *filename = "Y:\\pc\\gtavc\\models\\gta3_archive\\player.dff";
//	char *filename = "Y:\\pc\\gtavc\\models\\gta3_archive\\od_newscafe_dy.dff";
//	char *filename = "Y:\\pc\\gtasa\\models\\gta3_archive\\admiral.dff";
//	char *filename = "Y:\\pc\\gtasa\\models\\gta3_archive\\lae2_roads89.dff";
//	char *filename = "Y:\\pc\\gtasa\\models\\gta3_archive\\casinoblock41_nt.dff";
//	char *filename = "Y:\\pc\\gtasa\\models\\cutscene_archive\\csremington92.dff";
//	char *filename = "C:\\gtasa\\test\\hanger.dff";
//	char *filename = "C:\\Users\\aap\\Desktop\\tmp\\out.dff";
//	char *filename = "out2.dff";
//	char *filename = "C:\\Users\\aap\\src\\librw\\tools\\insttest\\out.dff";
	rw::StreamFile in;
	if(in.open(filename, "rb") == NULL){
		MessageBox(0, "couldn't open file\n", 0, 0);
		printf("couldn't open file\n");
	}
	rw::findChunk(&in, rw::ID_CLUMP, NULL, NULL);
	clump = rw::Clump::streamRead(&in);
	assert(clump);
	in.close();

	FORLIST(lnk, clump->atomics){
		rw::Atomic *a = rw::Atomic::fromClump(lnk);
		if(a->pipeline && a->pipeline->platform != rw::platform)
			a->pipeline = NULL;
		//a->getPipeline()->instance(a);

		char *name = gta::getNodeName(a->getFrame());
		if(strstr(name, "_dam") || strstr(name, "_vlo"))
			a->object.flags &= ~rw::Atomic::RENDER;
	}

	//rw::StreamFile out;
	//out.open("out.dff", "wb");
	//clump->streamWrite(&out);
	//out.close();

	//out.open("out.txd", "wb");
	//rw::currentTexDictionary->streamWrite(&out);
	//out.close();
}

bool
Setup()
{
	D3DLIGHT9 light;
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse =  { 0.8f, 0.8f, 0.8f, 1.0f };
	light.Specular = { 0.0f, 0.0f, 0.0f, 0.0f };
	light.Ambient =  { 0.0f, 0.0f, 0.0f, 0.0f };
	light.Position = { 0.0f, 0.0f, 0.0f };
	light.Direction = { 0.0f, 0.0f, -1.0f };
	light.Range = 0.0f;
	light.Falloff = 0.0f;
	light.Attenuation0 = 0.0f;
	light.Attenuation1 = 0.0f;
	light.Attenuation2 = 0.0f;
	light.Theta = 0.0f;
	light.Phi = 0.0f;

	initrw();

	Device->SetRenderState(D3DRS_LIGHTING, true);
	Device->SetLight(0, &light);
	Device->LightEnable(0, 1);

	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);


	camera = new Camera;
	camera->m_rwcam = rw::Camera::create();
	camera->m_rwcam->setFrame(rw::Frame::create());
	camera->m_aspectRatio = 640.0f/480.0f;
	camera->m_near = 0.1f;
	camera->m_far = 450.0f;
	camera->m_target.set(0.0f, 0.0f, 0.0f);
	camera->m_position.set(0.0f, -10.0f, 0.0f);
//	camera->setPosition(Vec3(0.0f, 5.0f, 0.0f));
//	camera->setPosition(Vec3(0.0f, -70.0f, 0.0f));
//	camera->setPosition(Vec3(0.0f, -1.0f, 3.0f));
	camera->update();

	return true;
}

void
setcamera(rw::Camera *cam)
{
	rw::Matrix viewmat;
	rw::Matrix::invert(&viewmat, cam->getFrame()->getLTM());
	viewmat.right.x = -viewmat.right.x;
	viewmat.rightw = 0.0;
	viewmat.up.x = -viewmat.up.x;
	viewmat.upw = 0.0;
	viewmat.at.x = -viewmat.at.x;
	viewmat.atw = 0.0;
	viewmat.pos.x = -viewmat.pos.x;
	viewmat.posw = 1.0;
	Device->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&viewmat);
	Device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)cam->projMat);
}

bool
Display(float timeDelta)
{
	if(Device == NULL)
		return true;
	
	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
	              0xff808080, 1.0f, 0);
	Device->BeginScene();

	camera->update();
	setcamera(camera->m_rwcam);

	clump->render();

	Device->EndScene();

	Device->Present(0, 0, 0, 0);
	return true;
}

LRESULT CALLBACK
d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch(wParam){
		case 'W':
			camera->orbit(0.0f, 0.1f);
			break;
		case 'S':
			camera->orbit(0.0f, -0.1f);
			break;
		case 'A':
			camera->orbit(-0.1f, 0.0f);
			break;
		case 'D':
			camera->orbit(0.1f, 0.0f);
			break;
		case VK_UP:
			camera->turn(0.0f, 0.1f);
			break;
		case VK_DOWN:
			camera->turn(0.0f, -0.1f);
			break;
		case VK_LEFT:
			camera->turn(0.1f, 0.0f);
			break;
		case VK_RIGHT:
			camera->turn(-0.1f, 0.0f);
			break;
		case 'R':
			camera->zoom(0.1f);
			break;
		case 'F':
			camera->zoom(-0.1f);
			break;
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		default:
			printf("%d\n", wParam);
		}
		break;
	case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void
Cleanup()
{
}

int WINAPI
WinMain(HINSTANCE hinstance, HINSTANCE prevInstance,
        PSTR cmdLine, int showCmd)
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	if(!d3d::InitD3D(hinstance, 640, 480, true, D3DDEVTYPE_HAL, &Device)){
		MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup()){
		MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}