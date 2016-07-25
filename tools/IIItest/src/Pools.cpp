#include "III.h"

CBuildingPool *CPools::ms_pBuildingPool;
CTreadablePool *CPools::ms_pTreadablePool;
CDummyPool *CPools::ms_pDummyPool;

void
CPools::Initialise(void)
{
	ms_pBuildingPool = new CBuildingPool(NUMBUILDINGS);
	ms_pTreadablePool = new CTreadablePool(NUMTREADABLES);
	ms_pDummyPool = new CDummyPool(NUMDUMMIES);
}

void
CPools::Shutdown(void)
{
}
