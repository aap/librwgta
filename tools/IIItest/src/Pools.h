typedef CPool<CBuilding, CBuilding> CBuildingPool;
typedef CPool<CTreadable, CTreadable> CTreadablePool;
typedef CPool<CDummy, CDummyPed> CDummyPool;

class CPools
{
	static CBuildingPool *ms_pBuildingPool;
	static CTreadablePool *ms_pTreadablePool;
	static CDummyPool *ms_pDummyPool;
public:
	// static CPool<cAudioScriptObject, cAudioScriptObject> *ms_pAudioScriptObjectPool;
	// static CPool<CObject, CCutsceneHead> *ms_pObjectPool;
	// static CPool<CVehicle, CVehicle> *ms_pVehiclePool;
	// static CPool<CPed, CPlayerPed> *ms_pPedPool;
	// static CPool<CEntryInfoNode, CEntryInfoNode> *ms_pEntryInfoNodePool;
	// static CPool<CPtrNode, CPtrNode> *ms_pPtrNodePool;

	static void Initialise(void);
	static void Shutdown(void);
	// CheckPoolsEmpty(void);

	static CBuildingPool *GetBuildingPool(void) { return ms_pBuildingPool; }
	static CTreadablePool *GetTreadablePool(void) { return ms_pTreadablePool; }
	static CDummyPool *GetDummyPool(void) { return ms_pDummyPool; }
};
