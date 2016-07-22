class CVisibilityPlugins
{
public:
	union AtomicExt
	{
		CSimpleModelInfo *modelInfo;
		int flags;
	};
	static void SetAtomicModelInfo(rw::Atomic*, CSimpleModelInfo*);
//	this is a useless wrapper
//	static void SetAtomicRenderCallback(rw::Atomic*, rw::Atomic::RenderCB);

	static void *AtomicConstructor(void *object, int32 offset, int32 len);
	static void *AtomicDestructor(void *object, int32 offset, int32 len);
	static void *AtomicCopyConstructor(void *dst, void *src,
	                                   int32 offset, int32 len);
	static int32 ms_atomicPluginOffset;

	struct FrameExt
	{
		rw::Frame *frame;
	};
	static void *FrameConstructor(void *object, int32 offset, int32 len);
	static void *FrameDestructor(void *object, int32 offset, int32 len);
	static void *FrameCopyConstructor(void *dst, void *src,
	                                  int32 offset, int32 len);
	static int32 ms_framePluginOffset;

	struct ClumpExt
	{
		CClumpModelInfo *modelInfo;
		int alpha;
	};
	static void *ClumpConstructor(void *object, int32 offset, int32 len);
	static void *ClumpDestructor(void *object, int32 offset, int32 len);
	static void *ClumpCopyConstructor(void *dst, void *src,
	                                  int32 offset, int32 len);
	static int32 ms_clumpPluginOffset;

	static bool PluginAttach(void);
};
