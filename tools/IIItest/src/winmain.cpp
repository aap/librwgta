#include "III.h"
#ifdef RW_D3D9
#include "d3dUtility.h"

IDirect3DDevice9 *Device = 0;

bool plWindowclosed( void )
{
    return true;
}

void plPresent( void )
{
    return;
}

void plHandleEvents( void )
{
    return;
}

void plCapturePad( int arg )
{
    return;
}

void plUpdatePad( CControllerState *cs )
{
    return;
}

int plGetTimeInMS( void )
{
    return (int)GetTickCount();
}

bool
display(float timeDelta)
{
	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
	              0xff808080, 1.0f, 0);
	Device->BeginScene();


	Device->EndScene();
	Device->Present(0, 0, 0, 0);
	return true;
}

int
wininit(void)
{
	rw::d3d::device = Device;
	return 1;
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
			break;
		}
		break;
	case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
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

	if(!wininit()){
		MessageBox(0, "wininit() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(display);

	Device->Release();

	return 0;
}

#endif
