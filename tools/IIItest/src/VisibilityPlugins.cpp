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
	m_alphaEntityList.Init(350);	// TODO: set back to 150 when things are fixed
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
	bool ret = !!m_alphaEntityList.InsertSorted(item);
	if(!ret)
		printf("list full %d\n", m_alphaEntityList.Count());
	return ret;
}

void
CVisibilityPlugins::InitAlphaAtomicList(void)
{
	m_alphaList.Clear();
}

void
CVisibilityPlugins::RenderFadingAtomic(rw::Atomic *atm, float camdist)
{
	rw::Atomic *distatm;
	float fadefactor;
	uchar alpha;
	CSimpleModelInfo *mi = GetAtomicModelInfo(atm);
	distatm = mi->GetAtomicFromDistance(camdist - 20.0f);
	if(mi->m_additive){
		SetRenderState(rw::DESTBLEND, rw::BLENDONE);
		rw::Atomic::defaultRenderCB(atm);
		SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	}else{
		fadefactor = (mi->GetLargestLodDistance() - (camdist - 20.0f))/20.0f;
		if(fadefactor > 1.0f)
			fadefactor = 1.0f;
		alpha = mi->m_alpha * fadefactor;
		if(alpha == 255)
			rw::Atomic::defaultRenderCB(atm);
		else{
			rw::Geometry *g = distatm->geometry;
			uint32 oldflags = g->flags & 0xFF;
			g->flags |= rw::Geometry::MODULATE;
			for(int32 i = 0; i < g->matList.numMaterials; i++)
				g->matList.materials[i]->color.alpha = alpha;
			if(g != atm->geometry)
				atm->setGeometry(g, 0);
			rw::Atomic::defaultRenderCB(atm);
			for(int32 i = 0; i < g->matList.numMaterials; i++)
				g->matList.materials[i]->color.alpha = 255;
			g->flags = oldflags | g->flags&~0xFF;
		}
	}
}

void
CVisibilityPlugins::RenderFadingEntities(void)
{
	CLink<AlphaObjectInfo> *node;
	CSimpleModelInfo *mi;
	for(node = m_alphaEntityList.tail.prev;
	    node != &m_alphaEntityList.head;
	    node = node->prev){
		CEntity *e = node->item.entity;
		if(e->m_rwObject == nil)
			continue;
		mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(e->m_modelIndex);
		if(mi->m_noZwrite){
			SetRenderState(ALPHATESTFUNC, ALPHANEVER);
			SetRenderState(ZWRITEENABLE, 0);
		}

		if(e->m_isFading){
			DeActivateDirectional();
			SetAmbientColours();
			e->m_isBeingRendered = 1;
			RenderFadingAtomic((rw::Atomic*)e->m_rwObject, node->item.sort);
			e->m_isBeingRendered = 0;
		}else
			CRenderer::RenderOneNonRoad(e);

		if(mi->m_noZwrite){
			SetRenderState(ZWRITEENABLE, 1);
			SetRenderState(ALPHATESTFUNC, ALPHALESS);
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

CSimpleModelInfo*
CVisibilityPlugins::GetAtomicModelInfo(rw::Atomic *atomic)
{
	return ATOMICEXT(atomic)->modelInfo;
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

