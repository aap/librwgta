#include "III.h"

CBuildingPool *CPools::ms_pBuildingPool;
CTreadablePool *CPools::ms_pTreadablePool;
CDummyPool *CPools::ms_pDummyPool;
CPtrNodePool *CPools::ms_pPtrNodePool;

void
CPools::Initialise(void)
{
	ms_pBuildingPool = new CBuildingPool(NUMBUILDINGS);
	ms_pTreadablePool = new CTreadablePool(NUMTREADABLES);
	ms_pDummyPool = new CDummyPool(NUMDUMMIES);
	ms_pPtrNodePool = new CPtrNodePool(NUMPTRNODES);
}

void
CPools::Shutdown(void)
{
}
