
enum ModelInfoType
{
	MODELINFO_SIMPLE = 1,
	MODELINFO_TIME   = 3,
	MODELINFO_PED    = 7,
};

struct CBaseModelInfo
{
	int32	field0;
	int32	field4;
	uint32	hashKey;
	const char *name;	// we'll use it for the name for now
//	int32	fieldC;	// char *name?
	uint8	type;
	int8	num2dFx;
	bool	ownsColModel;
	int8	field15;	// alpha?
	void	*colModel;
	int16	_2dfxIndex;
	int16	objectIndex;
	int16	refCount;
	int16	txdSlot;
	int16	unknownIndex;
	void	*vtable;
};

struct CSimpleModelInfo : public CBaseModelInfo
{
	RslObject **objects;
	float	drawDistances[3];
	uint8	numObjects;
	uint16	flags;
	CSimpleModelInfo *relatedObject;
};

struct CTimeModelInfo : public CSimpleModelInfo
{
	int32	timeOn;
	int32	timeOff;
	int16	otherModel;
	// int16 pad
};

struct TexListDef
{
	RslTexList *texlist;
	int32 refCount;
	char name[20];
};
