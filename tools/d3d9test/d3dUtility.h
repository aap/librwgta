#include <d3d9.h>
#include <string>

namespace d3d
{
bool InitD3D(HINSTANCE hInstance, int width, int height, bool windowed,
             D3DDEVTYPE deviceType, IDirect3DDevice9 **device);

int EnterMsgLoop(bool (*ptr_display)(float timeDelta));

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

}