#include "III.h"

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

int
main(int argc, char *argv[])
{
	CGame::InitialiseRW();
	CGame::InitialiseAfterRW();
	CGame::Initialise();
	dump();
	return 0;
}
