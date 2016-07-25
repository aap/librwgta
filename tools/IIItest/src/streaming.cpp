#include "III.h"

int CdStream::numImages;
char CdStream::imageNames[NUMCDIMAGES][64];
FILE *CdStream::images[NUMCDIMAGES];

void
CdStream::init()
{
	CdStream::numImages = 0;
}

void
CdStream::addImage(const char *name)
{
	images[numImages] = fopen_ci(name, "rb");
	if(images[numImages])
		// TODO: check bounds
		strncpy(imageNames[numImages++], name, 64);
}

void
CdStream::removeImages(void)
{
	for(int i = 0; i < numImages; i++)
		fclose(images[i]);
	numImages = 0;
}

void
CdStream::read(char *buf, uint pos, uint size)
{
	int imgsel = pos >> 24;
	int position = (pos&0xFFFFFF)*2048;
	fseek(images[imgsel], position, SEEK_SET);
	fread(buf, 1, size*2048, images[imgsel]);
}

CDirectory::CDirectory(int size)
{
	m_maxEntries = size;
	m_numEntries = 0;
	m_entries = new DirectoryInfo[size];
}

void
CDirectory::AddItem(DirectoryInfo *dirinfo)
{
	// TODO: check bounds
	m_entries[m_numEntries++] = *dirinfo;
}

bool
CStreamingInfo::GetCdPosnAndSize(uint *pos, uint *size)
{
	if(m_size == 0)
		return false;
	*pos = m_position;
	*size = m_size;
	return true;
}

void
CStreamingInfo::SetCdPosnAndSize(uint pos, uint size)
{
	m_position = pos;
	m_size = size;
}

void
CStreamingInfo::AddToList(CStreamingInfo *link)
{
	m_next = link->m_next;
	m_prev = link;
	link->m_next = this;
	m_next->m_prev = this;
}

void
CStreamingInfo::RemoveFromList(void)
{
	m_next->m_prev = m_prev;
	m_prev->m_next = m_next;
	m_next = nil;
	m_prev = nil;
}

CStreamingInfo CStreaming::ms_aInfoForModel[CStreaming::NUMSTREAMINFO];
CDirectory *CStreaming::ms_pExtraObjectsDir;
CStreamingInfo CStreaming::ms_startLoadedList, CStreaming::ms_endLoadedList;
CStreamingInfo CStreaming::ms_startRequestedList, CStreaming::ms_endRequestedList;
int CStreaming::ms_streamingBufferSize;
char *CStreaming::ms_pStreamingBuffer;

void
CStreaming::Init(void)
{
	for(int i = 0; i < NUMSTREAMINFO; i++){
		ms_aInfoForModel[i].m_loadState = 0;
		ms_aInfoForModel[i].m_next = nil;
		ms_aInfoForModel[i].m_prev = nil;
		ms_aInfoForModel[i].m_nextID = -1;
		ms_aInfoForModel[i].m_size = 0;
		ms_aInfoForModel[i].m_position = 0;
	}
	ms_startLoadedList.m_next = &ms_endLoadedList;
	ms_startLoadedList.m_prev = nil;
	ms_endLoadedList.m_next = nil;
	ms_endLoadedList.m_prev = &ms_startLoadedList;

	ms_startRequestedList.m_next = &ms_endRequestedList;
	ms_startRequestedList.m_prev = nil;
	ms_endRequestedList.m_next = nil;
	ms_endRequestedList.m_prev = &ms_startRequestedList;

	ms_streamingBufferSize = 0;

	ms_pExtraObjectsDir = new CDirectory(EXTRADIRSIZE);
	CStreaming::LoadCdDirectory();
	ms_pStreamingBuffer = new char[ms_streamingBufferSize*2048];
}

void
CStreaming::LoadCdDirectory(void)
{
	char dirname[64];
	char *s;
	for(int i = CdStream::numImages-1; i >= 0; i--){
		strncpy(dirname, CdStream::imageNames[i], sizeof(dirname));
		s = strrchr(dirname, '.');
		strncpy(s+1, "DIR", 4);
		CStreaming::LoadCdDirectory(dirname, i);
	}
}

void
CStreaming::LoadCdDirectory(const char *dirname, int n)
{
	char *ext;
	int txdslot, modelid;
	uint pos, size;
	short lastID;
	CDirectory::DirectoryInfo dirinfo;
	FILE *f = fopen_ci(dirname, "rb");
	if(f == nil)
		return;
	n <<= 24;	// img selector
	lastID = -1;
	while(fread(&dirinfo, 1, sizeof(dirinfo), f) == sizeof(dirinfo)){
		if(dirinfo.size > ms_streamingBufferSize)
			ms_streamingBufferSize = dirinfo.size;
		ext = strrchr(dirinfo.name, '.');
		assert(ext);
		*ext++ = '\0';
		if(rw::strncmp_ci(ext, "dff", 4) == 0){
			CBaseModelInfo *modelinfo =
			  CModelInfo::GetModelInfo(dirinfo.name, &modelid);
			if(modelinfo){
				if(CStreaming::Model(modelid)->GetCdPosnAndSize(&pos, &size))
					debug("%s appears more than once in %s\n", dirinfo.name, dirname);
				else{
					dirinfo.position |= n;
					CStreaming::Model(modelid)->SetCdPosnAndSize(dirinfo.position, dirinfo.size);
					if(lastID != -1)
						ms_aInfoForModel[lastID].m_nextID = MODELOFFSET+modelid;
					lastID = MODELOFFSET+modelid;
					continue;
				}
			}
			ms_pExtraObjectsDir->AddItem(&dirinfo);
		}else if(rw::strncmp_ci(ext, "txd", 4) == 0){
			txdslot = CTxdStore::FindTxdSlot(dirinfo.name);
			if(txdslot < 0)
				txdslot = CTxdStore::AddTxdSlot(dirinfo.name);
			else
			if(CStreaming::Txd(txdslot)->GetCdPosnAndSize(&pos, &size))
				debug("Txd %s appears more than once in %s\n", dirinfo.name, dirname);
			else{
				dirinfo.position |= n;
				CStreaming::Txd(txdslot)->SetCdPosnAndSize(dirinfo.position, dirinfo.size);
				if(lastID != -1)
					ms_aInfoForModel[lastID].m_nextID = TXDOFFSET+txdslot;
				lastID = TXDOFFSET+txdslot;
				continue;
			}
		}
		lastID = -1;
	}
	fclose(f);
}

void
CStreaming::ConvertBufferToObject(char *buffer, int id)
{
	int modelid, slotid;
	rw::StreamMemory stream;
	CStreamingInfo *strinfo = &ms_aInfoForModel[id];
	stream.open((uint8*)buffer, ms_streamingBufferSize*2048);
	if(id >= MODELOFFSET && id < MODELOFFSET+MODELINFOSIZE){
		modelid = id-MODELOFFSET;
		CBaseModelInfo *modelinfo = CModelInfo::GetModelInfo(modelid);
		if(!CTxdStore::isTxdLoaded(modelinfo->m_txdSlot)){
			debug("failed to load %s because TXD %s not in memory\n",
			      modelinfo->GetName(),
			      CTxdStore::GetTxdName(modelinfo->m_txdSlot));
			// TODO: remove and request again
			stream.close();
			return;
		}
		CTxdStore::AddRef(modelinfo->m_txdSlot);
		CTxdStore::SetCurrentTxd(modelinfo->m_txdSlot);
		bool success = true;
		if(modelinfo->m_type == CSimpleModelInfo::ID ||
		   modelinfo->m_type == CTimeModelInfo::ID){
			success = CFileLoader::LoadAtomicFile(&stream, modelid);
		}else
			success = CFileLoader::LoadClumpFile(&stream, modelid);
		CTxdStore::RemoveRefWithoutDelete(modelinfo->m_txdSlot);
		if(!success){
			debug("Failed to load %s\n", modelinfo->GetName());
			// TODO: remove and request again
			stream.close();
			return;
		}
	}else if(id >= TXDOFFSET && id < TXDOFFSET+TXDSTORESIZE){
		slotid = id-TXDOFFSET;
		if(!CTxdStore::LoadTxd(slotid, &stream)){
			debug("Failed to load %s.txd\n", CTxdStore::GetTxdName(slotid));
			// TODO: remove and request again
			stream.close();
			return;
		}
	}
	strinfo->m_loadState = 1;
	stream.close();
}

// TODO
void
CStreaming::RequestModel(int id, int flags)
{
	CStreamingInfo *strinfo = &ms_aInfoForModel[id];
	if(flags & 8)	// priority
		strinfo->m_flags |= 8;
	else
		strinfo->m_flags &= ~8;
	strinfo->m_flags |= flags;
	if(strinfo->m_next == nil)
		strinfo->AddToList(&ms_startRequestedList);
}

int
CStreaming::getNextFile(void)
{
	int retid = -1;
	int id;
again:
	for(CStreamingInfo *strinfo = ms_startRequestedList.m_next;
	    strinfo != &ms_endRequestedList;
	    strinfo = strinfo->m_next){
		id = strinfo-ms_aInfoForModel;
		if(id >= MODELOFFSET && id < MODELOFFSET+MODELINFOSIZE){
			CBaseModelInfo *mi = CModelInfo::GetModelInfo(id);
			if(CStreaming::Txd(mi->m_txdSlot)->m_loadState != 1){
				debug("requesting txd for model %d\n", id);
				RequestModel(mi->m_txdSlot+TXDOFFSET, CStreaming::Model(id)->m_flags);
				continue;
			}
		}
		retid = id;
		strinfo->m_loadState = 1;
		strinfo->RemoveFromList();
		break;
	}
	if(retid == -1 && ms_startRequestedList.m_next != &ms_endRequestedList)
		goto again;
	return retid;
}

void
CStreaming::LoadAllRequestedModels(void)
{
	int id;
	uint pos, size;
	while(ms_endRequestedList.m_prev != &ms_startRequestedList){
		id = CStreaming::getNextFile();
		if(id < 0)
			break;
		CStreamingInfo *strinfo = &CStreaming::ms_aInfoForModel[id];
		strinfo->GetCdPosnAndSize(&pos, &size);
		CdStream::read(ms_pStreamingBuffer, pos, size);
		ConvertBufferToObject(ms_pStreamingBuffer, id);
	}
}

void
CStreaming::dumpRequestList(void)
{
	CStreamingInfo *strinfo;
	for(strinfo = ms_startRequestedList.m_next;
	    strinfo != &ms_endRequestedList;
	    strinfo = strinfo->m_next){
		printf(" %ld %d\n", strinfo-ms_aInfoForModel, strinfo->m_flags);
	}
}
