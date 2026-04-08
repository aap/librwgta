namespace gta {

struct CSphere
{
	rw::V3d center;
	float radius;
};

struct CBox
{
	rw::V3d min;
	rw::V3d max;

	void FindMinMax(void){
		float tmp;
		if(max.x < min.x){
			tmp = min.x; min.x = max.x; max.x = tmp;
		}
		if(max.y < min.y){
			tmp = min.y; min.y = max.y; max.y = tmp;
		}
		if(max.z < min.z){
			tmp = min.z; min.z = max.z; max.z = tmp;
		}
	}
};

struct CompressedVector
{
	rw::int16 x, y, z;
	rw::V3d Uncompress(void) {
		rw::V3d v = { x/128.0f, y/128.0f, z/128.0f };
		return v;
	}
};

struct CColSphere
{
	CSphere sph;
	rw::uint8 surface;
	rw::uint8 piece;
	rw::uint8 lighting;

	void Set(float radius, rw::V3d *center, rw::uint8 surf, rw::uint8 piece, rw::uint8 lighting){
		this->sph.center = *center;
		this->sph.radius = radius;
		this->surface = surf;
		this->piece = piece;
		this->lighting = lighting;
	}
};

struct CColBox
{
	CBox box;
	rw::uint8 surface;
	rw::uint8 piece;
	rw::uint8 lighting;

	void Set(rw::V3d *min, rw::V3d *max, rw::uint8 surf, rw::uint8 piece, rw::uint8 lighting){
		this->box.min = *min;
		this->box.max = *max;
		this->surface = surf;
		this->piece = piece;
		this->lighting = lighting;
	}
};

struct CColLine
{
	rw::V3d p0;
	int pad0;
	rw::V3d p1;
	int pad1;

	void Set(rw::V3d *p0, rw::V3d *p1){
		this->p0 = *p0;
		this->p1 =*p1;
	}
};

struct CColTriangle
{
	short a;
	short b;
	short c;
	rw::uint8 surface;
	rw::uint8 lighting;

	void Set(int a, int b, int c, rw::uint8 surf, rw::uint8 lighting){
		this->a = a;
		this->b = b;
		this->c = c;
		this->surface = surf;
		this->lighting = lighting;
	}
};

//struct CColPoint
//{
//	V3d point;
//	int pad1;
//	V3d normal;
//	int pad2;
//	rw::uint8 surfaceTypeA;
//	rw::uint8 pieceTypeA;
//	rw::uint8 pieceTypeB;
//	rw::uint8 surfaceTypeB;
//	float depth;
//};

struct CColModel
{
	CSphere boundingSphere;
	CBox boundingBox;

	short numSpheres;
	short numLines;
	short numBoxes;
	short numTriangles;
	int level;
	//uint8 unk1;
	CColSphere *spheres;
	CColLine *lines;
	CColBox *boxes;
	union {
		rw::V3d *vertices;
		CompressedVector *compVertices;
	};
	CColTriangle *triangles;
	//int unk2;

	rw::uint8 flags;
	rw::uint8 allocFlag;
	rw::uint8 *rawdata;

	CColModel(void);
	~CColModel(void);
};

void readColModel(CColModel *colmodel, rw::uint8 *buf, int size);
void readColModelVer2(CColModel *colmodel, rw::uint8 *buf, int size);
void readColModelVer3(CColModel *colmodel, rw::uint8 *buf, int size);
void readColModelVer4(CColModel *colmodel, rw::uint8 *buf, int size);
rw::uint32 writeColModel(CColModel *colmodel, rw::uint8 **bufp);

}
