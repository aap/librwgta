namespace rw {
namespace gl3 {

void initializePlatform(void);
void initializeRender(void);

// arguments to glVertexAttribPointer basically
struct AttribDesc
{
	uint32 index;
	int32  type;
	bool32 normalized;
	int32  size;
	uint32 stride;
	uint32 offset;
};

struct InstanceData
{
	uint32    numIndex;
	uint32    minVert;	// not used for rendering
	Material *material;
	bool32    vertexAlpha;
	uint32    program;
	uint32    offset;
};

struct InstanceDataHeader : rw::InstanceDataHeader
{               
	uint32      serialNumber;	// not really needed right now
	uint32      numMeshes;
	uint16     *indexBuffer;
	uint32      primType;
	uint8      *vertexBuffer;
	int32       numAttribs;
	AttribDesc *attribDesc;
	uint32      totalNumIndex;
	uint32      totalNumVertex;

	GLuint      ibo;
	GLuint      vbo;		// or 2?

	InstanceData *inst;
};

void setAttribPointers(InstanceDataHeader *header);

// per Object
void setWorldMatrix(Matrix*);
void setAmbColor(RGBAf*);

// per Scene
void setProjectionMatrix(float32*);
void setViewMatrix(Matrix*);

class ObjPipeline : public rw::ObjPipeline
{       
public: 
	void (*instanceCB)(Geometry *geo, InstanceDataHeader *header);
	void (*uninstanceCB)(Geometry *geo, InstanceDataHeader *header);
	void (*renderCB)(Atomic *atomic, InstanceDataHeader *header);

	ObjPipeline(uint32 platform);
};

void defaultInstanceCB(Geometry *geo, InstanceDataHeader *header);
void defaultUninstanceCB(Geometry *geo, InstanceDataHeader *header);
void defaultRenderCB(Atomic *atomic, InstanceDataHeader *header);

ObjPipeline *makeDefaultPipeline(void);
ObjPipeline *makeSkinPipeline(void);
ObjPipeline *makeMatFXPipeline(void);

// Native Texture and Raster

extern int32 nativeRasterOffset;

struct Gl3Raster
{
	uint32 texid;
};

void registerNativeRaster(void);

}
}
