#ifndef _WORLD_H_
#define _WORLD_H_

/* Sectors span from -2000 to 2000 in x and y.
 * With 100x100 sectors, each is 40x40 units. */

#define NUMSECTORS_X 100
#define NUMSECTORS_Y 100

class CSector
{
public:
	// DON'T change, code assumes this is consecutive
	// and it should probably be an array instead
	CPtrList m_buildings;
	CPtrList m_buildingsOverlap;
	CPtrList m_objects;
	CPtrList m_objectsOverlap;
	CPtrList m_vehicles;
	CPtrList m_vehiclesOverlap;
	CPtrList m_peds;
	CPtrList m_pedsOverlap;
	CPtrList m_dummies;
	CPtrList m_dummiesOverlap;
};

class CWorld
{
	static CPtrList ms_bigBuildingsList[4];
	static CPtrList ms_listMovingEntityPtrs;
	static CSector ms_aSectors[NUMSECTORS_X][NUMSECTORS_Y];
	static uint16 ms_nCurrentScanCode;
public:

	static void Add(CEntity*);
	static CSector *GetSector(int x, int y) { return &ms_aSectors[x][y]; }
	static CPtrList &GetBigBuildingList(eLevelName i) { return ms_bigBuildingsList[i]; }
	static CPtrList &GetMovingEntityList(void) { return ms_listMovingEntityPtrs; }
	static uint16 GetCurrentScanCode(void) { return ms_nCurrentScanCode; }
	static void AdvanceCurrentScanCode(void){
		if(++CWorld::ms_nCurrentScanCode == 0){
			CWorld::ClearScanCodes();
			CWorld::ms_nCurrentScanCode = 1;
		}
	}
	static void ClearScanCodes(void);

	static int GetSectorIndexX(float f) { return (int)( (f + 2000.0f)/40.0f ); }
	static int GetSectorIndexY(float f) { return (int)( (f + 2000.0f)/40.0f ); }
};

#endif
