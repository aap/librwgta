#include "III.h"

CPool<TxdDef,TxdDef> *CTxdStore::ms_pTxdPool;
rw::TexDictionary *CTxdStore::ms_pStoredTxd;

void
CTxdStore::Initialize(void)
{
	ms_pTxdPool = new CPool<TxdDef,TxdDef>(TXDSTORESIZE);
}

int
CTxdStore::AddTxdSlot(const char *name)
{
	TxdDef *def = ms_pTxdPool->New();
	assert(def);
	def->texDict = nil;
	def->refCount = 0;
	strncpy(def->name, name, 20);
	return ms_pTxdPool->GetJustIndex(def);
}

int
CTxdStore::FindTxdSlot(const char *name)
{
	char *defname;
	int size = ms_pTxdPool->GetSize();
	for(int i = 0; i < size; i++){
		defname = GetTxdName(i);
		if(defname && rw::strncmp_ci(defname, name, 24) == 0)
			return i;
	}
	return -1;
}

char*
CTxdStore::GetTxdName(int slot)
{
	TxdDef *def = getDef(slot);
	return def ? def->name : nil;
}

void
CTxdStore::PushCurrentTxd(void)
{
	CTxdStore::ms_pStoredTxd = rw::TexDictionary::getCurrent();
}

void
CTxdStore::PopCurrentTxd(void)
{
	rw::TexDictionary::setCurrent(CTxdStore::ms_pStoredTxd);
	CTxdStore::ms_pStoredTxd = nil;
}

void
CTxdStore::SetCurrentTxd(int slot)
{
	TxdDef *def = getDef(slot);
	if(def)
		rw::TexDictionary::setCurrent(def->texDict);
}

void
CTxdStore::Create(int slot)
{
	getDef(slot)->texDict = rw::TexDictionary::create();
}

void
CTxdStore::AddRef(int slot)
{
	getDef(slot)->refCount++;
}

void
CTxdStore::RemoveRefWithoutDelete(int slot)
{
	getDef(slot)->refCount--;
}

bool
CTxdStore::LoadTxd(int slot, rw::Stream *stream)
{
	TxdDef *def = getDef(slot);
	if(!rw::findChunk(stream, rw::ID_TEXDICTIONARY, nil, nil)){
		printf("Failed to load TXD\n");
		return false;
	}else{
		def->texDict = rw::TexDictionary::streamRead(stream);
		return def->texDict != nil;
	}
}

bool
CTxdStore::LoadTxd(int slot, const char *filename)
{
	rw::StreamFile stream;
	if(stream.open(getPath(filename), "rb")){
		LoadTxd(slot, &stream);
		stream.close();
		return true;
	}
	printf("Failed to open TXD\n");
	return false;
}

TxdDef*
CTxdStore::getDef(int slot)
{
	return ms_pTxdPool->GetSlot(slot);
}

bool
CTxdStore::isTxdLoaded(int slot)
{
	return getDef(slot)->texDict != nil;
}
