#include "euryopa.h"

/*
 * Streaming limits:
 *
 * III PC:
 * 5500 models
 * 850 tex dicts
 *
 * VC PC:
 * 6500 models
 * 1385 tex dicts
 * 31 collisions
 * 35 ifps
 *
 * SA PC:
 * 20000 models
 * 5000 tex dicts
 * 255 collisions
 * 256 ipls
 * 64 dats
 * 180 ifps
 * 47 rrrs
 * 82 scms
 */

enum
{
	FILE_MODEL = 1,
	FILE_TXD,
	FILE_COL,
	FILE_IPL,
};

struct DirEntry
{
	uint32 position;
	uint32 size;
	char name[24];

	int imgindex;
	int filetype;
};

static FILE *imgfiles[NUMCDIMAGES];
static int numImages;

static DirEntry *directory;
static int directorySize;
static int directoryLimit = 8000;
static uint32 maxFileSize;
static uint8 *streamingBuffer;

static CPtrList requestList;

static void
AddDirEntry(DirEntry *de)
{
	char *ext;
	ext = strrchr(de->name, '.');
	if(ext == nil){
		log("warning: no file extension: %s\n", de->name);
		return;
	}
	*ext++ = '\0';
	// VC introduced IFP COL
	// SA introduced IPL DAT RRR SCM
	if(rw::strncmp_ci(ext, "dff", 3) == 0)
		de->filetype = FILE_MODEL;
	else if(rw::strncmp_ci(ext, "txd", 3) == 0)
		de->filetype = FILE_TXD;
	else if(rw::strncmp_ci(ext, "col", 3) == 0)
		de->filetype = FILE_COL;
	else if(rw::strncmp_ci(ext, "ipl", 3) == 0)
		de->filetype = FILE_IPL;
	else{
//		log("warning: unknown file extension: %s %s\n", ext, de->name);
		return;
	}
	if(de->size > maxFileSize)
		maxFileSize = de->size;
	directory[directorySize++] = *de;
}

static void
ReadDirectory(FILE *dir, int n, int imgindex)
{
	DirEntry de;
	int i;

	if(directory == nil)
		directory = rwNewT(DirEntry, directoryLimit, 0);
	while(directorySize + n > directoryLimit){
		directoryLimit *= 2;
		directory = rwResizeT(DirEntry, directory, directoryLimit, 0);
	}
	for(i = 0 ; i < n; i++){
		fread(&de, 1, 32, dir);
		de.imgindex = imgindex;
		AddDirEntry(&de);
	}
}

void
AddCdImage(const char *path)
{
	FILE *img, *dir;
	char *dirpath, *p;
	int fourcc, n;
	int imgindex;

	img = fopen_ci(path, "rb");
	if(img == nil){
		log("warning: cdimage %s couldn't be opened\n", path);
		return;
	}
	imgindex = numImages++;
	imgfiles[imgindex] = img;
	fread(&fourcc, 1, 4, img);
	if(fourcc == 0x32524556){	// VER2
		// Found a VER2 image, read its directory
		fread(&n, 1, 4, img);
		ReadDirectory(img, n, imgindex);
	}else{
		dirpath = strdup(path);
		p = strrchr(dirpath, '.');
		strcpy(p+1, "dir");
		dir = fopen_ci(dirpath, "rb");
		if(dir == nil){
			log("warning: directory %s couldn't be opened\n", dirpath);
			numImages--;
			fclose(img);
			return;
		}
		fseek(dir, 0, SEEK_END);
		n = ftell(dir);
		fseek(dir, 0, SEEK_SET);
		n /= 32;
		ReadDirectory(dir, n, imgindex);
	}
}

void
InitCdImages(void)
{
	int i, slot;
	ObjectDef *obj;
	TxdDef *txd;
	ColDef *col;
	IplDef *ipl;

	for(i = 0; i < directorySize; i++){
		DirEntry *de = &directory[i];
		switch(de->filetype){
		case FILE_MODEL:
			obj = GetObjectDef(de->name, nil);
			if(obj){
				if(obj->m_imageIndex >= 0)
					log("warning: model %s appears multiple times\n", obj->m_name);
				obj->m_imageIndex = i;
			}
			break;

		case FILE_TXD:
			slot = AddTxdSlot(de->name);
			txd = GetTxdDef(slot);
			if(txd->imageIndex >= 0)
				log("warning: txd %s appears multiple times\n", txd->name);
			txd->imageIndex = i;
			break;

		case FILE_COL:
			slot = AddColSlot(de->name);
			col = GetColDef(slot);
			if(col->imageIndex >= 0)
				log("warning: col %s appears multiple times\n", col->name);
			col->imageIndex = i;
			break;

		case FILE_IPL:
			slot = AddIplSlot(de->name);
			ipl = GetIplDef(slot);
			if(ipl->imageIndex >= 0)
				log("warning: ipl %s appears multiple times\n", ipl->name);
			ipl->imageIndex = i;
			break;
		}
	}

	streamingBuffer = (uint8*)malloc(maxFileSize*2048);
}

uint8*
ReadFileFromImage(int i, int *size)
{
	DirEntry *de = &directory[i];
	fseek(imgfiles[de->imgindex], de->position*2048, SEEK_SET);
	fread(streamingBuffer, 1, de->size*2048, imgfiles[de->imgindex]);
	if(size)
		*size = de->size*2048;
	return streamingBuffer;
}

void
RequestObject(int id)
{
	requestList.InsertItem((void*)id);
}

void
LoadAllRequestedObjects(void)
{
	CPtrNode *p;
	int id;
	for(p = requestList.first; p; p = p->next){
		id = (int)p->item;
		ObjectDef *obj = GetObjectDef(id);
		if(!obj->IsLoaded())
			obj->Load();
	}
	requestList.Flush();
}
