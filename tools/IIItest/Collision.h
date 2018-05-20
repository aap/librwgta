struct CColSphere
{
	CVector center;
	float radius;
	uint8 surface;
	uint8 piece;

	void Set(float radius, const CVector &center, uint8 surf, uint8 piece);
};

struct CColBox
{
	CVector min;
	CVector max;
	uint8 surface;
	uint8 piece;

	void Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece);
};

struct CColLine
{
	CVector p0;
	int pad0;
	CVector p1;
	int pad1;

	void Set(const CVector &p0, const CVector &p1);
};

struct CColTriangle
{
	int16 a;
	int16 b;
	int16 c;
	uint8 surface;

	void Set(int a, int b, int c, uint8 surf);
};

//struct CColPoint
//{
//	rw::V3d point;
//	int pad1;
//	rw::V3d normal;
//	int pad2;
//	rw::uint8 surfaceTypeA;
//	rw::uint8 pieceTypeA;
//	rw::uint8 pieceTypeB;
//	rw::uint8 surfaceTypeB;
//	float depth;
//};

struct CColModel
{
	CColSphere boundingSphere;
	CColBox boundingBox;
	short numSpheres;
	short numLines;
	short numBoxes;
	short numTriangles;
	int level;
	//rw::uint8 unk1;
	CColSphere *spheres;
	CColLine *lines;
	CColBox *boxes;
	CVector *vertices;
	CColTriangle *triangles;
	//int unk2;

	CColModel(void);
	~CColModel(void);
};

class CCollision
{
public:
	static eLevelName ms_collisionInMemory;

	static void Update(void);
	static void LoadCollisionWhenINeedIt(bool changeLevel);
	static void DrawColModel(const CMatrix &mat, const CColModel &colModel);
};
