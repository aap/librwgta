#include "III.h"

CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaList;
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaEntityList;

int32 CVisibilityPlugins::ms_atomicPluginOffset;
int32 CVisibilityPlugins::ms_framePluginOffset;
int32 CVisibilityPlugins::ms_clumpPluginOffset;

void
CVisibilityPlugins::Initialise(void)
{
	m_alphaList.Init(20);
	m_alphaList.head.item.sort = 0.0f;
	m_alphaList.tail.item.sort = 100000000.0f;
	m_alphaEntityList.Init(150);
	m_alphaEntityList.head.item.sort = 0.0f;
	m_alphaEntityList.tail.item.sort = 100000000.0f;
}

void
CVisibilityPlugins::InitAlphaEntityList(void)
{
	m_alphaEntityList.Clear();
}

bool
CVisibilityPlugins::InsertEntityIntoSortedList(CEntity *e, float dist)
{
	AlphaObjectInfo item;
	item.entity = e;
	item.sort = dist;
	return !!m_alphaEntityList.InsertSorted(item);
}

void
CVisibilityPlugins::InitAlphaAtomicList(void)
{
	m_alphaList.Clear();
}

void
CVisibilityPlugins::RenderFadingEntities(void)
{
	CLink<AlphaObjectInfo> *node;
	for(node = m_alphaEntityList.tail.prev;
	    node != &m_alphaEntityList.head;
	    node = node->prev){
		CEntity *e = node->item.entity;
		if(e->m_rwObject){
			e->Render();
		}
	}
}

//
// RW Plugins
//

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

