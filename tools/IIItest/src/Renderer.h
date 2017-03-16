class CRenderer
{
	static int ms_nNoOfVisibleEntities;
	static int ms_nNoOfInVisibleEntities;
	static CEntity *ms_aVisibleEntityPtrs[2000];
	static CVector ms_vecCameraPosition;
public:
	static void ConstructRenderList(void);
	static void ScanWorld(void);
	static void ScanBigBuildingList(CPtrList *list);
	static void ScanSectorList(CPtrList *list);
	static bool SetupBigBuildingVisibility(CEntity *ent);
	static int  SetupEntityVisibility(CEntity *ent);

	static void RenderOneNonRoad(CEntity *ent);
	static void RenderEverything(void);
	static void RenderEverythingBarRoads(void);
	static void RenderRoads(void);
	static void RenderFadingInEntities(void);
};
