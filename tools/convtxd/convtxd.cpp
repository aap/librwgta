#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <ctype.h>

#include <rw.h>
#include <args.h>

#include <rwgta.h>

char *argv0;

using namespace std;
using namespace rw;

struct {
	const char *str;
	uint32 val;
} platforms[] = {
	{ "ps2",    PLATFORM_PS2 },
	{ "xbox",   PLATFORM_XBOX },
	{ "d3d8",   PLATFORM_D3D8 },
	{ "d3d9",   PLATFORM_D3D9 },
	{ NULL, 0 }
};

Raster*
xboxToD3d8(Raster *raster)
{
	using namespace xbox;

	Raster *newras;
	if(raster->platform != PLATFORM_XBOX)
		return raster;
	XboxRaster *ras = PLUGINOFFSET(XboxRaster, raster, nativeRasterOffset);

	int32 numLevels = raster->getNumLevels();

	int32 format = raster->format;
//	format &= ~Raster::MIPMAP;
	if(ras->format){
		newras = Raster::create(raster->width, raster->height, raster->depth,
		                        format | raster->type | 0x80, PLATFORM_D3D8);
		int32 dxt = 0;
		switch(ras->format){
		case D3DFMT_DXT1:
			dxt = 1;
			break;
		case D3DFMT_DXT3:
			dxt = 3;
			break;
		case D3DFMT_DXT5:
			dxt = 5;
			break;
		}
		d3d::allocateDXT(newras, dxt, numLevels, ras->hasAlpha);
	}else{
		printf("swizzled!\n");
		newras = Raster::create(raster->width, raster->height, raster->depth,
		                        format | raster->type, PLATFORM_D3D8);
	}

	if(raster->format & Raster::PAL4)
		d3d::setPalette(newras, ras->palette, 32);
	else if(raster->format & Raster::PAL8)
		d3d::setPalette(newras, ras->palette, 256);

	uint8 *data;
	for(int32 i = 0; i < numLevels; i++){
		if(i >= newras->getNumLevels())
			break;
		data = raster->lock(i);
		d3d::setTexels(newras, data, i);
		raster->unlock(i);
	}

	raster->destroy();
	return newras;
}

struct StrAssoc {
	const char *key;
	int val;

	static int get(StrAssoc *desc, char *key);
};

int
StrAssoc::get(StrAssoc *desc, char *key)
{
	for(; desc->key[0] != '\0'; desc++)
		if(strcmp(desc->key, key) == 0)
			return desc->val;
	return desc->val;
}

enum {
	NEW = 1,
	SEARCHPATH,
	ADD,
	LINEAR,
	WRITE
};

StrAssoc cmds[] = {
	{ "new", NEW },
	{ "searchpath", SEARCHPATH },
	{ "add", ADD },
	{ "linear", LINEAR },
	{ "write", WRITE },
	{ "", 0 }
};

char*
skipwhite(char *s)
{
	while(isspace(*s))
		s++;
	return s;
}

char*
getline(void)
{
	static char line[1024];
again:
	if(fgets(line, 1024, stdin) == NULL)
		return NULL;
	// remove leading whitespace
	char *s = skipwhite(line);
	// remove trailing whitespace
	int end = strlen(s);
	char c;
	while(c = s[--end], isspace(c))
		s[end] = '\0';
	// convert ',' -> ' '
	for(char *t = s; *t; t++)
		if(*t == ',') *t = ' ';
	// don't return empty lines
	if(*s == '\0')
		goto again;
	return s;
}

char*
removequotes(char *s)
{
	char *t;
	if(*s == '"'){
		s++;
		for(t = s; *t != '\0'; t++)
			if(*t == '"'){
				*t = '\0';
				break;
			}
	}
	return s;
}

int
runscript(void)
{
	char *s, *arg;
	int cmd;
	TexDictionary::setCurrent(TexDictionary::create());
	rw::Texture::setLoadTextures(true);
	Texture *tex = NULL;
	StreamFile out;
	while(s = getline()){
		if(s[0] == '#')
			continue;
		s = strtok(s, "\t");
		cmd = StrAssoc::get(cmds, s);
		arg = strtok(NULL, "\t");
#define NEEDARG(arg) \
			if(arg == NULL){\
				fprintf(stderr, "missing argument to '%s'\n", s);\
				break;\
			}
		switch(cmd){
		case NEW:
			TexDictionary::getCurrent()->destroy();
			TexDictionary::setCurrent(TexDictionary::create());
			break;
		case SEARCHPATH:
			NEEDARG(arg);
			Image::setSearchPath(arg);
			break;
		case ADD:
			NEEDARG(arg);
			arg = removequotes(arg);
			tex = Texture::read(arg, strtok(NULL, "\t"));
			break;
		case LINEAR:
			if(tex)
				tex->filterAddressing = tex->filterAddressing&~0xFF | Texture::LINEAR;
			break;
		case WRITE:
			NEEDARG(arg);
			if(out.open(arg, "wb")){
				TexDictionary::getCurrent()->streamWrite(&out);
				out.close();
			}else
				fprintf(stderr, "couldn't write txd %s\n", arg);
			break;
		default:
			fprintf(stderr, "unknown command '%s'\n", s);
		}
#undef NEEDARG
	}
	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [-v version] [-o platform] in.txd [out.txd]\n", argv0);
	fprintf(stderr, "\t-v RW version, e.g. 33004 for 3.3.0.4\n");
	fprintf(stderr, "\t-o output platform. ps2, xbox, mobile, d3d8, d3d9\n");
	fprintf(stderr, "\t-t convert palettized to 32 bit\n");
	fprintf(stderr, "\t-s run script from stdin, see source code\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	rw::version = 0x36003;
//	rw::platform = rw::PLATFORM_PS2;
//	rw::platform = rw::PLATFORM_OGL;
//	rw::platform = rw::PLATFORM_XBOX;
	rw::platform = rw::PLATFORM_D3D8;
//	rw::platform = rw::PLATFORM_D3D9;
//	int outplatform = rw::PLATFORM_XBOX;
	int outplatform = rw::PLATFORM_D3D9;
	int script = 0;
	int extract = 0;
	int separatemask = 0;
	int unpalettize = 0;

	rw::Engine::init();
	gta::attachPlugins();
	rw::Engine::open();
	rw::Engine::start(nil);
	rw::Texture::setLoadTextures(false);
//	rw::d3d::isP8supported = 0;

	char *s;
	ARGBEGIN{
	case 'v':
		sscanf(EARGF(usage()), "%x", &rw::version);
		break;
	case 'o':
		s = EARGF(usage());
		for(int i = 0; platforms[i].str; i++){
			if(strcmp(platforms[i].str, s) == 0){
				outplatform = platforms[i].val;
				goto found;
			}
		}
		printf("unknown platform %s\n", s);
		outplatform = PLATFORM_D3D8;
	found:
		break;
	case 's':
		script++;
		break;
	case 'x':
		extract++;
		break;
	case 'm':
		separatemask++;
		break;
	case 't':
		unpalettize++;
		break;
	default:
		usage();
	}ARGEND;

	if(script)
		return runscript();

	if(argc < 1)
		usage();

	rw::StreamFile in;
	if(in.open(argv[0], "rb") == NULL){
		printf("couldn't open file %s\n", argv[1]);
		return 1;
	}
	ChunkHeaderInfo header;
	readChunkHeaderInfo(&in, &header);
	assert(header.type == ID_TEXDICTIONARY);
	rw::TexDictionary *txd;
	txd = rw::TexDictionary::streamRead(&in);
	assert(txd);
	in.close();
	rw::TexDictionary::setCurrent(txd);

	if(rw::version == 0){
		rw::version = header.version;
		rw::build = header.build;
	}

	if(outplatform == PLATFORM_D3D8)
		FORLIST(lnk, txd->textures){
			Texture *tex = Texture::fromDict(lnk);
			tex->raster = xboxToD3d8(tex->raster);
		}
	if(outplatform == PLATFORM_D3D9)
		FORLIST(lnk, txd->textures){
			Texture *tex = Texture::fromDict(lnk);
			if(tex->raster->platform == PLATFORM_D3D8)
				tex->raster->platform = PLATFORM_D3D9;
		}
//	for(Texture *tex = txd->first; tex; tex = tex->next)
//		tex->filterAddressing = (tex->filterAddressing&~0xF) | 0x2;
	rw::platform = outplatform;

	if(unpalettize){
		FORLIST(lnk, txd->textures){
			Texture *tex = Texture::fromDict(lnk);
			Image *img = tex->raster->toImage();
			img->unindex();
			Raster *ras = tex->raster;
			tex->raster = Raster::createFromImage(img);
			ras->destroy();
			img->destroy();
		}
	}

	char filename[1024];
	if(extract){
		FORLIST(lnk, txd->textures){
			Texture *tex = Texture::fromDict(lnk);
			Image *img = tex->raster->toImage();
			img->unindex();

			if(separatemask && tex->mask[0]){
				Image *mask = img->extractMask();
				// write colors without alpha if we have a
				// separate file.
				img->removeMask();

				strncpy(filename, tex->mask, 1024);
				strncat(filename, ".tga", 1024);
				writeTGA(mask, filename);
				mask->destroy();
			}

			strncpy(filename, tex->name, 1024);
			strncat(filename, ".tga", 1024);
			writeTGA(img, filename);

			img->destroy();
		}
	}else{
		rw::StreamFile out;
		if(argc > 1)
			out.open(argv[1], "wb");
		else
			out.open("out.txd", "wb");
		txd->streamWrite(&out);
		out.close();
	}

	return 0;
}
