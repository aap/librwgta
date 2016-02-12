enum {
	TEX_IDENT  = 0x00746578,
	MDL_IDENT  = 0x006D646C,
	WRLD_IDENT = 0x57524C44
};

struct RslStream {
	uint32   ident;
	bool32   isMulti;
	uint32   fileSize;
	uint32   dataSize;
	uint8 ***reloc;
	uint32   relocSize;
	uint8  **hashTab;   // ??
	uint16   hashTabSize;
	uint16   numElements;

	uint8 *data;
	void relocate(void);
};

struct RslObject;
struct RslObjectHasNode;
struct RslElementGroup;        // Clump
struct RslElement;             // Atomic
struct RslNode;                // Frame
struct RslNativeGeometry;      // unofficial name
struct RslNativeMesh;	       // unofficial name
struct RslGeometry;
struct RslSkin;
struct RslMaterial;
struct RslTAnimTree;           // HAnimHierarchy
struct RslTAnimNode;           // HAnimNode
struct RslPS2ResEntryHeader;   // unofficial name
struct RslPS2InstanceData;     // unofficial name
struct RslTexList;             // TexDictionary
struct RslTexture;

typedef RslNode *(*RslNodeCallBack)(RslNode *frame, void *data);
typedef RslElementGroup *(*RslElementGroupCallBack)(RslElementGroup *clump, void *data);
typedef RslElement *(*RslElementCallBack)(RslElement *atomic, void *data);
typedef RslMaterial *(*RslMaterialCallBack)(RslMaterial *material, void *data);
typedef RslTexture *(*RslTextureCallBack)(RslTexture *texture, void *pData);

struct RslV3
{
	float32 x, y, z;
};

struct RslMatrix
{
	RslV3 right;
	uint32 flags;
	RslV3 up;
	uint32 pad1;
	RslV3 at;
	uint32 pad2;
	RslV3 pos;
	uint32 pad3;
};

void RslMatrixSetIdentity(RslMatrix *matrix);

struct RslLLLink
{
	RslLLLink *next;
	RslLLLink *prev;
};

#define rslLLLinkGetData(linkvar,type,entry)                           \
    ((type*)(((uint8*)(linkvar))-offsetof(type,entry)))
#define rslLLLinkGetNext(linkvar)                                      \
    ((linkvar)->next)
#define rslLLLinkGetPrevious(linkvar)                                  \
    ((linkvar)->prev)
#define rslLLLinkInitialize(linkvar)                                   \
    ((linkvar)->prev = (RslLLLink*)NULL,                               \
     (linkvar)->next = (RslLLLink*)NULL)
#define rslLLLinkAttached(linkvar)                                     \
    ((linkvar)->next)

struct RslLinkList
{
	RslLLLink link;
};

#define rslLinkListInitialize(list)                                    \
    ((list)->link.next = ((RslLLLink*)(list)),                         \
     (list)->link.prev = ((RslLLLink*)(list)))
#define rslLinkListEmpty(list)                                         \
    (((list)->link.next) == (&(list)->link))
#define rslLinkListAddLLLink(list, linkvar)                            \
    ((linkvar)->next = (list)->link.next,                              \
     (linkvar)->prev = (&(list)->link),                                \
     ((list)->link.next)->prev = (linkvar),                            \
     (list)->link.next = (linkvar) )
#define rslLinkListRemoveLLLink(linkvar)                               \
    (((linkvar)->prev)->next = (linkvar)->next,                        \
     ((linkvar)->next)->prev = (linkvar)->prev)
#define rslLinkListGetFirstLLLink(list)                                \
    ((list)->link.next)
#define rslLinkListGetLastLLLink(list)                                 \
    ((list)->link.prev)
#define rslLinkListGetTerminator(list)                                 \
    (&((list)->link))


struct RslObject {
	uint8  type;
	uint8  subType;
	uint8  flags;
	uint8  privateFlags;
	void  *parent;
};

#define rslObjectInitialize(o, t, s)                  \
{                                                     \
    ((RslObject*)(o))->type = (uint8)(t);             \
    ((RslObject*)(o))->subType = (uint8)(s);          \
    ((RslObject*)(o))->flags = 0;                     \
    ((RslObject*)(o))->privateFlags = 0;              \
    ((RslObject*)(o))->parent = NULL;                 \
}

#define rslObjectGetParent(object) (((RslObject*)(object))->parent)
#define rslObjectSetParent(c,p)    (((RslObject*)(c))->parent) = (void*)(p)

struct RslObjectHasNode {
	RslObject   object;
	RslLLLink   lNode;
	void      (*sync)();
};

void rslObjectHasNodeSetNode(RslObjectHasNode *object, RslNode *f);

struct RslRasterPS2 {
	uint8 *data;
	uint32 flags;
};

struct RslRasterPSP {
	uint32 unk1;
	uint8 *data;
	uint16 flags1;
	uint8  width;    // log of width
	uint8  height;   // log of height
	uint32 flags2;
};

struct RslPs2StreamRaster {
	uint32 width;
	uint32 height;
	uint32 depth;
	uint32 mipmaps;
	uint32 unused;
};

union RslRaster {
	RslRasterPS2 ps2;
	RslRasterPSP psp;
};

RslRaster *RslCreateRasterPS2(uint32 w, uint32 h, uint32 d, uint32 mipmaps);

struct RslTexList {
	RslObject   object;
	RslLinkList texturesInDict;
	RslLLLink   lInInstance;
};

RslTexList *RslTexListStreamRead(Stream *stream);
RslTexList *RslTexListCreate(void);
RslTexture *RslTexListAddTexture(RslTexList *dict, RslTexture *tex);
RslTexList *RslTexListForAllTextures(RslTexList *dict, RslTextureCallBack fpCallBack, void *pData);

struct RslTexture {
	RslRaster        *raster;
	RslTexList       *dict;
	RslLLLink         lInDictionary;
	char              name[32];
	char              mask[32];
};

RslTexture *RslTextureCreate(RslRaster *raster);
void RslTextureDestroy(RslTexture *texture);
RslTexture *RslTextureStreamRead(Stream *stream);
RslTexture *RslReadNativeTexturePS2(Stream *stream);

struct RslNode {
	RslObject         object;
	RslLinkList       objectList;
		         
	RslMatrix         modelling;
	RslMatrix         ltm;
	RslNode          *child;
	RslNode          *next;
	RslNode          *root;

	// RwHAnimNodeExtension
	int32              nodeId;
	RslTAnimTree      *hier;
	// R* Node name
	char              *name;
	// R* Visibility
	int32              hierId;
};

RslNode *RslNodeCreate(void);
RslNode *RslNodeAddChild(RslNode *parent, RslNode *child);
int32 RslNodeCount(RslNode *f);
RslNode *RslNodeForAllChildren(RslNode *frame, RslNodeCallBack callBack, void *data);

struct rslNodeList
{
	RslNode **frames;
	int32 numNodes;
};

void rslNodeListStreamRead(Stream *stream, rslNodeList *framelist);
void rslNodeListInitialize(rslNodeList *frameList, RslNode *root);

struct RslElementGroup {
	RslObject   object;
	RslLinkList atomicList;
};

#define RslElementGroupGetNode(_clump)                                    \
    ((RslNode*)rslObjectGetParent(_clump))

#define RslElementGroupSetNode(_clump, _frame)                            \
    (rslObjectSetParent(_clump, _frame),                            \
     (_clump))

RslElementGroup *RslElementGroupCreate(void);
RslElementGroup *RslElementGroupStreamRead(Stream *stream);
RslElementGroup *RslElementGroupAddElement(RslElementGroup *clump, RslElement *a);
int32 RslElementGroupGetNumElements(RslElementGroup *clump);
RslElementGroup *RslElementGroupForAllElements(RslElementGroup *clump, RslElementCallBack callback, void *pData);

struct RslElement {
	RslObjectHasNode  object;
	RslGeometry       *geometry;
	RslElementGroup          *clump;
	RslLLLink          inElementGroupLink;

	// what's this? rpWorldObj?
	uint32             unk1;
	uint16             unk2;
	uint16             unk3;
	// RpSkin
	RslTAnimTree      *hier;
	// what about visibility? matfx?
	int32              pad;	// 0xAAAAAAAA
};

#define RslElementGetNode(_atomic)                                  \
    ((RslNode*)rslObjectGetParent(_atomic))

RslElement *RslElementCreate(void);
RslElement *RslElementSetNode(RslElement *atomic, RslNode *frame);
RslElement *RslElementStreamRead(Stream *stream, rslNodeList *framelist);

struct RslMaterialList {
	RslMaterial **materials;
	int32         numMaterials;
	int32         space;
};

void rslMaterialListStreamRead(Stream *stream, RslMaterialList *matlist);

struct RslGeometry {
	RslObject       object;
	int16           refCount;
	int16           pad1;

	RslMaterialList matList;

	RslSkin        *skin;
	uint32          pad2;            // 0xAAAAAAAA
};

RslGeometry *RslGeometryCreatePS2(uint32 sz);
RslGeometry *RslGeometryForAllMaterials(RslGeometry *geometry, RslMaterialCallBack fpCallBack, void *pData);

struct RslMatFXEnv {
	RslNode *frame;
	union {
		char       *texname;
		RslTexture *texture;
	};
	float32   intensity;
};

struct RslMatFX {
	union {
		RslMatFXEnv env;
	};
	int32     effectType;
};

struct RslMaterial {
	union {
		char       *texname;
		RslTexture *texture;
	};
	RGBA        color;
	uint32      refCount;
	RslMatFX   *matfx;
};

RslMaterial *RslMaterialCreate(void);
RslMaterial *RslMaterialStreamRead(Stream *stream);

struct RslTAnimNodeInfo {
	int8      id;
	int8      index;
	int8      flags;
	RslNode  *frame;
};

struct RslTAnimTree {
	int32              flags;
	int32              numNodes;
	void              *pCurrentAnim;
	float32            currentTime;
	void              *pNextFrame;
	void             (*pAnimCallBack)();
	void              *pAnimCallBackData;
	float32            animCallBackTime;
	void             (*pAnimLoopCallBack)();
	void              *pAnimLoopCallBackData;
	float32           *pMatrixArray;
	void              *pMatrixArrayUnaligned;
	RslTAnimNodeInfo  *pNodeInfo;
	RslNode           *parentNode;
	int32              maxKeyFrameSize;
	int32              currentKeyFrameSize;
	void             (*keyFrameToMatrixCB)();
	void             (*keyFrameBlendCB)();
	void             (*keyFrameInterpolateCB)();
	void             (*keyFrameAddCB)();
	RslTAnimTree      *parentTree;
	int32              offsetInParent;
	int32              rootParentOffset;
};

struct RslSkin {
	uint32   numBones;
	uint32   numUsedBones;	// == numBones
	uint8   *usedBones;	// NULL
	float32 *invMatrices;
	int32    numWeights;	// 0
	uint8   *indices;	// NULL
	float32 *weights;	// NULL
	uint32   unk1;          // 0
	uint32   unk2;          // 0
	uint32   unk3;          // 0
	uint32   unk4;          // 0
	uint32   unk5;          // 0
	void    *data;          // NULL
};

RslSkin *RslSkinStreamRead(Stream *stream, RslGeometry *g);

 // sPspGeometry but should be sPs2Geometry obviously...
struct sPs2Geometry {
	float32 bound[4];
	uint32  size;		// and numMeshes
	int32   flags;
	uint32  unk1;
	uint32  unk2;
	uint32  unk3;
	uint32  unk4;
	float32 scale[3];
	float32 pos[3];
};

struct sPs2GeometryMesh {
	float32  bound[4];
	float32  uvScale[2];
	int32    unknown;
	uint32   dmaPacket;
	uint16   numTriangles;
	int16    matID;
	int16    min[3];          // bounding box
	int16    max[3];
};

struct RslStreamHeader {
	uint32   ident;
	uint32   unk;
	uint32   fileEnd;      //
	uint32   dataEnd;      // relative to beginning of header
	uint32   reloc;	       //
	uint32   relocSize;
	uint32   root;        // absolute
	uint16   zero;
	uint16   numElements;
};

struct RslWorldGeometry {
	uint16 numMeshes;
	uint16 size;
	// array of numMeshes RslWorldMesh
	// dma data
};

struct RslWorldMesh {
	uint16 texID;           // index into resource table
	uint16 dmaSize;
	uint16 uvScale[2];	// half precision float
	uint16 unk1;
	int16  min[3];          // bounding box
	int16  max[3];
};

struct Resource {
	union {
		RslRaster        *raster;
		RslWorldGeometry *geometry;
		uint8            *raw;
	};
	uint32 *dma;
};

struct OverlayResource {
	int32 id;
	union {
		RslRaster        *raster;
		RslWorldGeometry *geometry;
		uint8            *raw;
	};
};

struct Placement {
	uint16 id;
	uint16 resId;
	int16 bound[4];
	int32 pad;
	float matrix[16];
};

struct Sector {
	OverlayResource *resources;
	uint16           numResources;
	uint16           unk1;
	Placement       *sectionA;
	Placement       *sectionB;
	Placement       *sectionC;
	Placement       *sectionD;
	Placement       *sectionE;
	Placement       *sectionF;
	Placement       *sectionG;
	Placement       *sectionEnd;
	uint16           unk2;
	uint16           unk3;
	uint32           unk4;
};

struct SectorEntry {
	RslStreamHeader *sector;
	uint32           num;
};

struct World {
	Resource    *resources;
	SectorEntry  sectors[47];
	uint32       numResources;
	uint8        pad[0x180];

	uint32 numX;
	void *tabX;	// size 0x4

	uint32 numY;
	void *tabY;	// size 0x30

	uint32 numZ;
	void *tabZ;	// size 0x6

	uint32           numTextures;
	RslStreamHeader *textures; // stream headers
};
