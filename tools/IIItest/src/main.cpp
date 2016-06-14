#include "III.h"
#include "d3dUtility.h"

IDirect3DDevice9 *Device = 0;

// TODO: fix path for case sensitive systems
FILE*
fopen_ci(const char *path, const char *mode)
{
	return fopen(path, mode);
}

char*
skipWhite(char *s)
{
	while(isspace(*s))
		s++;
	return s;
}

int
StrAssoc::get(StrAssoc *desc, char *key)
{
	for(; desc->key[0] != '\0'; desc++)
		if(strcmp(desc->key, key) == 0)
			return desc->val;
	return desc->val;
}

void*
DatDesc::get(DatDesc *desc, char *name)
{
	for(; desc->name[0] != '\0'; desc++)
		if(strcmp(desc->name, name) == 0)
			return desc->handler;
	return desc->handler;
}

//int
//debug(const char *fmt, ...)
//{
//}

void
dump(void)
{
	CBaseModelInfo *m;
	for(int i = 0; i < MODELINFOSIZE; i++){
		m = CModelInfo::ms_modelInfoPtrs[i];
		if(m == NULL)
			continue;
		//if(m->type == CSimpleModelInfo::ID)
		//	printf("%d %s\n", i, m->name);
		//if(m->type == CTimeModelInfo::ID)
		//	printf("%d %s\n", i, m->name);
		//if(m->type == CClumpModelInfo::ID)
		//	printf("%d %s\n", i, m->name);
		//if(m->type == CPedModelInfo::ID)
		//	printf("%d %s\n", i, m->name);
		//if(m->type == CVehicleModelInfo::ID){
		//	CVehicleModelInfo *vm = (CVehicleModelInfo*)m;
		//	printf("%d %s %d %d %d\n", i, vm->name, vm->vehicleType, vm->vehicleClass, vm->handlingId);
		//}
	}
	//for(int i = 0; i < 850; i++){
	//	char *name = CTxdStore::GetTxdName(i);
	//	if(name)
	//		printf("%d %s\n", i, name);
	//}
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
init(void)
{
	rw::d3d::device = Device;
	CGame::InitialiseRW();
	CGame::InitialiseAfterRW();
	CGame::Initialise();

//	dump();
	CStreaming::RequestModel(731, 1);
	CStreaming::LoadAllRequestedModels();
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
		
	if(!init()){
		MessageBox(0, "init() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(display);

	Device->Release();

	return 0;
}
