#include "III.h"
#include <cstdarg>

uchar work_buff[55000];

char*
getPath(const char *path)
{
	static char cipath[1024];
	strncpy(cipath, path, 1024);
	rw::makePath(cipath);
	return cipath;
}

FILE*
fopen_ci(const char *path, const char *mode)
{
	char cipath[1024];
	strncpy(cipath, path, 1024);
	rw::makePath(cipath);
	return fopen(cipath, mode);
}

char*
skipWhite(char *s)
{
	while(isspace(*s))
		s++;
	return s;
}

int
StrAssoc::get(StrAssoc *desc, const char *key)
{
	for(; desc->key[0] != '\0'; desc++)
		if(strcmp(desc->key, key) == 0)
			return desc->val;
	return desc->val;
}

void*
DatDesc::get(DatDesc *desc, const char *name)
{
	for(; desc->name[0] != '\0'; desc++)
		if(strcmp(desc->name, name) == 0)
			return (void*)desc->handler;
	return (void*)desc->handler;
}

void
debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void
dump(void)
{
	CBaseModelInfo *m;
	for(int i = 0; i < MODELINFOSIZE; i++){
		m = CModelInfo::GetModelInfo(i);
		if(m == nil)
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

void
update(double t)
{
}

void
display(void)
{
}

void
shutdown(void)
{
}

int
init(void)
{
	CGame::InitialiseRW();
	CGame::InitialiseAfterRW();
	CGame::Initialise();

//	dump();
	CStreaming::RequestModel(731, 1);
	CStreaming::LoadAllRequestedModels();
	return 1;
}
