#include "III.h"

CTxdStore::TxdDef *CTxdStore::entries;
uchar *CTxdStore::flags;
int CTxdStore::capacity;
int CTxdStore::allocPtr;

rw::TexDictionary *CTxdStore::ms_pStoredTxd;

void
CTxdStore::Initialize(void)
{
	CTxdStore::entries = new TxdDef[TXDSTORESIZE];
	CTxdStore::flags = new uchar[TXDSTORESIZE];
	CTxdStore::capacity = TXDSTORESIZE;
	CTxdStore::allocPtr = 0;
	for(int i = 0; i < TXDSTORESIZE; i++)
		CTxdStore::flags[i] = 0x80;
}

int
CTxdStore::AddTxdSlot(const char *name)
{
	TxdDef *def;
	int idx;
	// TODO, only temporary
	assert(CTxdStore::allocPtr < TXDSTORESIZE);
	def = &CTxdStore::entries[idx = CTxdStore::allocPtr++];
	CTxdStore::flags[idx] = 0;
	def->refCount = 0;
	def->texDict = nil;
	strncpy(def->name, name, 20);
	return idx;
}

int
CTxdStore::FindTxdSlot(const char *name)
{
	TxdDef *def = CTxdStore::entries;
	for(int i = 0; i < CTxdStore::capacity; i++, def++){
		if(CTxdStore::flags[i] & 0x80)
			continue;
		if(rw::strncmp_ci(def->name, name, 24) == 0)
			return i;
	}
	return -1;
}

char*
CTxdStore::GetTxdName(int slot)
{
	if(CTxdStore::flags[slot] & 0x80)
		return nil;
	return CTxdStore::entries[slot].name;
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
	if(CTxdStore::flags[slot] & 0x80)
		return;
	rw::TexDictionary::setCurrent(CTxdStore::entries[slot].texDict);
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

CTxdStore::TxdDef*
CTxdStore::getDef(int slot)
{
	if((CTxdStore::flags[slot] & 0x80) == 0)
		return &CTxdStore::entries[slot];
	return nil;
}

bool
CTxdStore::isTxdLoaded(int slot)
{
	TxdDef *def = getDef(slot);
	return def->texDict != nil;
}
