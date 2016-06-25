namespace rw {
int32 findPlatform(Clump *c);
void switchPipes(Clump *c, int32 platform);
};

namespace gta {
using namespace rw;

enum
{
	VEND_ROCKSTAR     = 0x0253F2
};

enum
{
	ID_VISIBILITYATOMIC = MAKEPLUGINID(VEND_ROCKSTAR, 0x00),
	ID_VISIBILITYCLUMP  = MAKEPLUGINID(VEND_ROCKSTAR, 0x01),
	ID_VISIBILITYFRAME  = MAKEPLUGINID(VEND_ROCKSTAR, 0x02),
	ID_EXTRANORMALS     = MAKEPLUGINID(VEND_ROCKSTAR, 0xF2),
	ID_PIPELINE         = MAKEPLUGINID(VEND_ROCKSTAR, 0xF3),
	ID_ENVATM           = MAKEPLUGINID(VEND_ROCKSTAR, 0xF4),
	ID_TXDSTORE         = MAKEPLUGINID(VEND_ROCKSTAR, 0xF5),
	ID_SPECMAT          = MAKEPLUGINID(VEND_ROCKSTAR, 0xF6),
	ID_2DEFFECT         = MAKEPLUGINID(VEND_ROCKSTAR, 0xF8),
	ID_EXTRAVERTCOLORS  = MAKEPLUGINID(VEND_ROCKSTAR, 0xF9),
	ID_COLLISION        = MAKEPLUGINID(VEND_ROCKSTAR, 0xFA),
	ID_ANIMBLENDCLUMPSA = MAKEPLUGINID(VEND_ROCKSTAR, 0xFB),
	ID_ENVMAT           = MAKEPLUGINID(VEND_ROCKSTAR, 0xFC),
	ID_BREAKABLE        = MAKEPLUGINID(VEND_ROCKSTAR, 0xFD),
	ID_ANIMBLENDCLUMP   = MAKEPLUGINID(VEND_ROCKSTAR, 0xFD),
	ID_NODENAME         = MAKEPLUGINID(VEND_ROCKSTAR, 0xFE),
};

void attachPlugins(void);

// Node name

extern int32 nodeNameOffset;
void registerNodeNamePlugin(void);
char *getNodeName(Frame *f);

// Breakable model

struct Breakable
{
	uint32 position;
	uint32 numVertices;
	uint32 numFaces;
	uint32 numMaterials;

	float32 *vertices;
	float32 *texCoords;
	uint8   *colors;
	uint16  *faces;
	uint16  *matIDs;
	char    (*texNames)[32];
	char    (*maskNames)[32];
	float32 (*surfaceProps)[3];
};

extern int32 breakableOffset;
void registerBreakableModelPlugin(void);

// Extra normals (only on Xbox)

extern int32 extraNormalsOffset;
void registerExtraNormalsPlugin(void);

// Extra vert colors (not on Xbox)

struct ExtraVertColors
{
	uint8 *nightColors;
	uint8 *dayColors;
	float balance;
};

extern int32 extraVertColorOffset;
void allocateExtraVertColors(Geometry *g);
void registerExtraVertColorPlugin(void);

// Environment mat

struct EnvMat
{
	int8 scaleX, scaleY;
	int8 transScaleX, transScaleY;
	uint8 shininess;
	Texture *texture;
};

extern int32 envMatOffset;

// Specular mat

struct SpecMat
{
	float specularity;
	Texture *texture;
};

extern int32 specMatOffset;

void registerEnvSpecPlugin(void);

// Pipeline

// 0x53F2009A		CCustomCarEnvMapPipeline
//
// PC & Mobile:
// 0x53F20098		CCustomBuildingDNPipeline
// 0x53F2009C		CCustomBuildingPipeline
//
// Xbox
// 0x53F2009E		building  !N !EN
// 0x53F20096		building   N !EN
// 0x53F200A0		building  !N  EN   (also env) non-DN	 custom instanceCB!
// 0x53F200A2		building   N  EN   (also env) DN	 custom instanceCB!
//
// PS2 (PDS)
//
// 0x53F20080	world
//  0x53F20081	world
// 0x53F20082	world
//  0x53F20083	world
// 0x53F20084	vehicle
//  0x53F20085	vehicle
// 0x53F20086	vehicle (unused)
//  0x53F20087	vehicle
// 0x53F20088	skin
//  0x53F20089	skin
// 0x53F2008A	world (unused)
//  0x53F2008B	vehicle
// 0x53F2008C	world (unused)
//  0x53F2008D	world
// 0x53F2008E	world (unused)
//  0x53F2008F	world (unused)
// 0x53F20090	world (unused)
//  0x53F20091	world (unused)

// pds dffs:
//    n   atomic   material
//  1892  53f20080 53f20081	// ?			   no night colors
//     1  53f20080 53f2008d	// triad_buddha01.dff	   no night colors
// 56430  53f20082 53f20083	// world		   night colors
//    39  53f20082 53f2008f	// reflective world	   night colors
//  6941  53f20084 53f20085	// vehicles
//  3423  53f20084 53f20087	// vehicles
//  4640  53f20084 53f2008b	// vehicles
//   418  53f20088 53f20089	// peds

extern int32 pipelineOffset;

void registerPipelinePlugin(void);
uint32 getPipelineID(Atomic *atomic);
void setPipelineID(Atomic *atomic, uint32 id);

// 2dEffect

extern int32 twodEffectOffset;

void register2dEffectPlugin(void);

// Collision

extern int32 collisionOffset;

void registerCollisionPlugin(void);

// PDS pipes

struct SaVert : ps2::Vertex {
	uint8   c1[4];
};
void insertSAVertex(Geometry *geo, int32 i, uint32 mask, SaVert *v);
int32 findSAVertex(Geometry *g, uint32 flags[], uint32 mask, SaVert *v);

void registerPDSPipes(void);

}
