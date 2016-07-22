#include "III.h"

int32 CVisibilityPlugins::ms_atomicPluginOffset;
int32 CVisibilityPlugins::ms_framePluginOffset;
int32 CVisibilityPlugins::ms_clumpPluginOffset;

bool
CVisibilityPlugins::PluginAttach(void)
{
	using namespace rw;

	ms_atomicPluginOffset = Atomic::registerPlugin(sizeof(AtomicExt),
		gta::ID_VISIBILITYATOMIC,
		AtomicConstructor, AtomicDestructor, AtomicCopyConstructor);

	ms_framePluginOffset = Frame::registerPlugin(sizeof(FrameExt),
		gta::ID_VISIBILITYFRAME,
		FrameConstructor, FrameDestructor, FrameCopyConstructor);

	ms_clumpPluginOffset = Clump::registerPlugin(sizeof(ClumpExt),
		gta::ID_VISIBILITYCLUMP,
		ClumpConstructor, ClumpDestructor, ClumpCopyConstructor);
	return true;
}

#define ATOMICEXT(o) ((AtomicExt*)PLUGINOFFSET(AtomicExt, o, ms_atomicPluginOffset))
#define FRAMEEXT(o) ((FrameExt*)PLUGINOFFSET(FrameExt, o, ms_framePluginOffset))
#define CLUMPEXT(o) ((ClumpExt*)PLUGINOFFSET(ClumpExt, o, ms_clumpPluginOffset))

//
// Atomic
//

void*
CVisibilityPlugins::AtomicConstructor(void *object, int32, int32)
{
	ATOMICEXT(object)->modelInfo = nil;
	return object;
}

void*
CVisibilityPlugins::AtomicDestructor(void *object, int32, int32)
{
	return object;
}

void*
CVisibilityPlugins::AtomicCopyConstructor(void *dst, void *src, int32, int32)
{
	*ATOMICEXT(dst) = *ATOMICEXT(src);
	return dst;
}

void
CVisibilityPlugins::SetAtomicModelInfo(rw::Atomic *atomic,
                                       CSimpleModelInfo *modelInfo)
{
	AtomicExt *ext = ATOMICEXT(atomic);
	ext->modelInfo = modelInfo;
	// TODO: set renderCB from modelInfo->type
}

//
// Frame
//

void*
CVisibilityPlugins::FrameConstructor(void *object, int32, int32)
{
	FRAMEEXT(object)->frame = nil;
	return object;
}

void*
CVisibilityPlugins::FrameDestructor(void *object, int32, int32)
{
	return object;
}

void*
CVisibilityPlugins::FrameCopyConstructor(void *dst, void *src, int32, int32)
{
	*FRAMEEXT(dst) = *FRAMEEXT(src);
	return dst;
}

//
// Clump
//

void*
CVisibilityPlugins::ClumpConstructor(void *object, int32, int32)
{
	ClumpExt *ext = CLUMPEXT(object);
	ext->modelInfo = nil;
	ext->alpha = 0xFF;
	return object;
}

void*
CVisibilityPlugins::ClumpDestructor(void *object, int32, int32)
{
	return object;
}

void*
CVisibilityPlugins::ClumpCopyConstructor(void *dst, void *src, int32, int32)
{
	CLUMPEXT(dst)->modelInfo = CLUMPEXT(src)->modelInfo;
	return dst;
}

