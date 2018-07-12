namespace rw {
int32 findPlatform(Atomic *a);
int32 findPlatform(Clump *c);
void switchPipes(Atomic *a, int32 platform);
void switchPipes(Clump *c, int32 platform);
}

namespace gta {

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

extern rw::int32 nodeNameOffset;
void registerNodeNamePlugin(void);
char *getNodeName(rw::Frame *f);

// Breakable model

struct Breakable
{
	rw::uint32 position;
	rw::uint32 numVertices;
	rw::uint32 numFaces;
	rw::uint32 numMaterials;

	rw::float32 *vertices;
	rw::float32 *texCoords;
	rw::uint8   *colors;
	rw::uint16  *faces;
	rw::uint16  *matIDs;
	char    (*texNames)[32];
	char    (*maskNames)[32];
	rw::float32 (*surfaceProps)[3];
};

extern rw::int32 breakableOffset;
void registerBreakableModelPlugin(void);

// Extra normals (only on Xbox)

extern rw::int32 extraNormalsOffset;
void registerExtraNormalsPlugin(void);
rw::V3d *allocateExtraNormals(rw::Geometry *g);
void freeExtraNormals(rw::Geometry *g);
rw::V3d *getExtraNormals(rw::Geometry *g);

// Extra vert colors (not on Xbox)

struct ExtraVertColors
{
	rw::RGBA *nightColors;
	rw::RGBA *dayColors;
	float balance;
};

extern rw::int32 extraVertColorOffset;
void allocateExtraVertColors(rw::Geometry *g);
void registerExtraVertColorPlugin(void);
rw::RGBA *getExtraVertColors(rw::Atomic *a);

// Environment mat

struct EnvMat
{
	rw::int8 scaleX, scaleY;
	rw::int8 transScaleX, transScaleY;
	rw::uint8 shininess;
	rw::Texture *texture;

	float getScaleX(void) { return scaleX/8.0f; }
	float getScaleY(void) { return scaleY/8.0f; }
	float getTransScaleX(void) { return transScaleX/8.0f; }
	float getTransScaleY(void) { return transScaleY/8.0f; }
	float getShininess(void) { return shininess/255.0f; }
};

extern rw::int32 envMatOffset;
EnvMat *getEnvMat(rw::Material *mat);

// Specular mat

struct SpecMat
{
	float specularity;
	rw::Texture *texture;
};

extern rw::int32 specMatOffset;

void registerEnvSpecPlugin(void);

enum
{
	// PS2 pipes
	// building
	PDS_PS2_CustomBuilding_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x80),
	PDS_PS2_CustomBuilding_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x81),
	PDS_PS2_CustomBuildingDN_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x82),
	PDS_PS2_CustomBuildingDN_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x83),
	PDS_PS2_CustomBuildingEnvMap_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x8C),
	PDS_PS2_CustomBuildingEnvMap_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x8D),
	PDS_PS2_CustomBuildingDNEnvMap_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x8E),
	PDS_PS2_CustomBuildingDNEnvMap_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x8F),
	PDS_PS2_CustomBuildingUVA_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x90),
	PDS_PS2_CustomBuildingUVA_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x91),
	PDS_PS2_CustomBuildingDNUVA_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x92),
	PDS_PS2_CustomBuildingDNUVA_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x93),
	// car
	PDS_PS2_CustomCar_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x84),
	PDS_PS2_CustomCar_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x85),
	PDS_PS2_CustomCarEnvMap_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x86),
	PDS_PS2_CustomCarEnvMap_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x87),
//	PDS_PS2_CustomCarEnvMapUV2_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x8A),	// this does not exist
	PDS_PS2_CustomCarEnvMapUV2_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x8B),
	// skin
	PDS_PS2_CustomSkinPed_AtmPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x88),
	PDS_PS2_CustomSkinPed_MatPipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x89),

	// PC pipes
	RSPIPE_PC_CustomBuilding_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x9C),
	RSPIPE_PC_CustomBuildingDN_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x98),
	RSPIPE_PC_CustomCarEnvMap_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x9A),	// same as XBOX!

	// Xbox pipes
	RSPIPE_XBOX_CustomBuilding_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x9E),
	RSPIPE_XBOX_CustomBuildingDN_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x96),
	RSPIPE_XBOX_CustomBuildingEnvMap_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0xA0),
	RSPIPE_XBOX_CustomBuildingDNEnvMap_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0xA2),
	RSPIPE_XBOX_CustomCarEnvMap_PipeID = MAKEPIPEID(VEND_ROCKSTAR, 0x9A),	// same as PC!
};

// Pipeline

extern rw::int32 pipelineOffset;

void registerPipelinePlugin(void);
rw::uint32 getPipelineID(rw::Atomic *atomic);
void setPipelineID(rw::Atomic *atomic, rw::uint32 id);

extern rw::ObjPipeline *XboxCustomBuildingPipe;
extern rw::ObjPipeline *XboxCustomBuildingDNPipe;
extern rw::ObjPipeline *XboxCustomBuildingEnvMapPipe;
extern rw::ObjPipeline *XboxCustomBuildingDNEnvMapPipe;
void registerXboxPipes(void);

void attachCustomPipelines(rw::Atomic *atomic);

// 2dEffect

extern rw::int32 twodEffectOffset;

void register2dEffectPlugin(void);

// Collision

extern rw::int32 collisionOffset;

void registerCollisionPlugin(void);

// PDS pipes

struct SaVert : rw::ps2::Vertex {
	rw::RGBA   c1;
};
void insertSAVertex(rw::Geometry *geo, rw::int32 i, rw::uint32 mask, SaVert *v);
rw::int32 findSAVertex(rw::Geometry *g, rw::uint32 flags[], rw::uint32 mask, SaVert *v);

void registerPDSPipes(void);

}
