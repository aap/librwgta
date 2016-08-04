class CRenderer
{
	static int ms_nNoOfVisibleEntities;
	static int ms_nNoOfInVisibleEntities;
	static CEntity *ms_aVisibleEntityPtrs[2000];
	static CVector ms_vecCameraPosition;
public:
	static void ConstructRenderList(void);
	static void ScanWorld(void);
	static bool SetupBigBuildingVisibility(CEntity *ent);
	static void ScanBigBuildingList(CPtrList *list);

	static void RenderEverything(void);
	static void RenderFadingInEntities(void);
};
