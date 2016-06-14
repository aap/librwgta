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
CTxdStore::AddTxdSlot(char *name)
{
	TxdDef *def;
	int idx;
	// TODO, only temporary
	def = &CTxdStore::entries[idx = CTxdStore::allocPtr++];
	CTxdStore::flags[idx] = 0;
	def->refCount = 0;
	def->texDict = NULL;
	strncpy(def->name, name, 20);
	return idx;
}

int
CTxdStore::FindTxdSlot(char *name)
{
	TxdDef *def = CTxdStore::entries;
	for(int i = 0; i < CTxdStore::capacity; i++, def++){
		if(CTxdStore::flags[i] & 0x80)
			continue;
		if(strcmp(def->name, name) == 0)
			return i;
	}
	return -1;
}

char*
CTxdStore::GetTxdName(int slot)
{
	if(CTxdStore::flags[slot] & 0x80)
		return NULL;
	return CTxdStore::entries[slot].name;
}

void
CTxdStore::PushCurrentTxd(void)
{
	CTxdStore::ms_pStoredTxd = rw::currentTexDictionary;
}

void
CTxdStore::PopCurrentTxd(void)
{
	rw::currentTexDictionary = CTxdStore::ms_pStoredTxd;
	CTxdStore::ms_pStoredTxd = NULL;
}

void
CTxdStore::SetCurrentTxd(int slot)
{
	if(CTxdStore::flags[slot] & 0x80)
		return;
	rw::currentTexDictionary = CTxdStore::entries[slot].texDict;
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
	if(!rw::findChunk(stream, rw::ID_TEXDICTIONARY, NULL, NULL)){
		printf("Failed to load TXD\n");
		return false;
	}else{
		def->texDict = rw::TexDictionary::streamRead(stream);
		return def->texDict != NULL;
	}
}

CTxdStore::TxdDef*
CTxdStore::getDef(int slot)
{
	if((CTxdStore::flags[slot] & 0x80) == 0)
		return &CTxdStore::entries[slot];
	return NULL;
}

bool
CTxdStore::isTxdLoaded(int slot)
{
	TxdDef *def = getDef(slot);
	return def->texDict != NULL;
}