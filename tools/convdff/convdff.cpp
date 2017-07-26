#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>
#include <args.h>

#include <rwgta.h>

using namespace std;
using namespace rw;

static struct {
	const char *str;
	uint32 val;
} platforms[] = {
	{ "mobile", PLATFORM_WDGL },
	{ "ps2",    PLATFORM_PS2 },
	{ "xbox",   PLATFORM_XBOX },
	{ "d3d8",   PLATFORM_D3D8 },
	{ "d3d9",   PLATFORM_D3D9 },
	{ NULL, 0 }
};

char *argv0;

void
usage(void)
{
	fprintf(stderr, "usage: %s [-u] [-i] [-f] [-d] [-v version] [-o platform] in.dff [out.dff]\n", argv0);
	fprintf(stderr, "\t-u uninstance\n");
	fprintf(stderr, "\t-i instance\n");
	fprintf(stderr, "\t-f don't flip frame hierarchy\n");
	fprintf(stderr, "\t-d dump frame and hanim hierarchy\n");
	fprintf(stderr, "\t-w correct face winding of tristrips\n");
	fprintf(stderr, "\t-v RW version, e.g. 33004 for 3.3.0.4\n");
	fprintf(stderr, "\t-o output platform. ps2, xbox, mobile, d3d8, d3d9\n");
	exit(1);
}

void
dumpUVAnim(Animation *anim)
{
	UVAnimCustomData *cust = (UVAnimCustomData*)anim->customData;
	printf(" %s", cust->name);
	for(int i = 0; i < 8; i++)
		printf(" %d", cust->nodeToUVChannel[i]);
	printf("\n %d %x\n", anim->numFrames, anim->interpInfo->id);
	UVAnimKeyFrame *kf = (UVAnimKeyFrame*)anim->keyframes;
	for(int i = 0; i < anim->numFrames; i++){
		printf(" %f\n", kf->time);
		printf("  %f %f %f %f %f %f\n", kf->uv[0], kf->uv[1], kf->uv[2], kf->uv[3], kf->uv[4], kf->uv[5]);
		kf++;
	}
}

void
dumpFrameHier(Frame *frame, int ind = 0)
{
	for(int i = 0; i < ind; i++)
		printf("  ");
	const char *name = gta::getNodeName(frame);
	HAnimData *hanim = HAnimData::get(frame);
	printf("*%s %d %d %s\n", name[0] ? name : "---", frame->objectList.count(), hanim->id, hanim->hierarchy ? "HIERARCHY" : "");
	if(hanim->hierarchy){
		HAnimHierarchy *h = hanim->hierarchy;
		for(int i = 0; i < h->numNodes; i++){
			name = h->nodeInfo[i].frame ? gta::getNodeName(h->nodeInfo[i].frame) : "";
			printf("\t\t%d %d\t%p %s\n", h->nodeInfo[i].id, h->nodeInfo[i].flags, h->nodeInfo[i].frame, name);

			/*{
			h->nodeInfo[i].frame->getLTM();
			float *mat = (float*)&h->nodeInfo[i].frame->ltm;
			printf("[ [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"
			       "  [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"
			       "  [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"
			       "  [ %8.4f, %8.4f, %8.4f, %8.4f ] ]\n",
				mat[0], mat[4], mat[8], mat[12],
				mat[1], mat[5], mat[9], mat[13],
				mat[2], mat[6], mat[10], mat[14],
				mat[3], mat[7], mat[11], mat[15]);
			}*/
		}
	}
	for(Frame *child = frame->child;
	    child; child = child->next)
		dumpFrameHier(child, ind+1);
}

void
dumpReflData(Material *m)
{
	using namespace gta;

	MatFX *matfx = *PLUGINOFFSET(MatFX*, m, matFXGlobals.materialOffset);
	if(m->pipeline->pluginID != ID_PDS)
		return;
	printf("%x ", m->pipeline->pluginData & 0xf);
	if(matfx){
		printf("matfx(");
		int i = matfx->getEffectIndex(MatFX::ENVMAP);
		Texture *tex = i >= 0 ? matfx->fx[i].env.tex : NULL;
		printf("%d ", matfx->type);
		if(tex)
			printf("%s ", tex->name);
		printf(") ");
	}

	SpecMat *spec = *PLUGINOFFSET(SpecMat*, m, specMatOffset);
	if(spec)
		printf("spec(%.3f) ", spec->specularity);

	EnvMat *env = *PLUGINOFFSET(EnvMat*, m, envMatOffset);
	if(env)
		printf("env(%.3f) ", env->shininess/255.0f);

	printf("\n");
}

bool
isPrimaryColor(RGBA *c)
{
	return c->red == 0x3C && c->green == 0xFF && c->blue == 0x00;
}

bool
isSecondaryColor(RGBA *c)
{
	return c->red == 0xFF && c->green == 0x00 && c->blue == 0xAF;
}

void
removeBodyTextures(Clump *clump)
{
	FORLIST(lnk, clump->atomics){
		Geometry *g = Atomic::fromClump(lnk)->geometry;
		for(int32 i = 0; i < g->matList.numMaterials; i++){
			Material *m = g->matList.materials[i];
			if(m->texture == nil) continue;
			if(strstr(m->texture->name, "body") &&
			   (isPrimaryColor(&m->color) || isSecondaryColor(&m->color))){
				m->texture->destroy();
				m->texture = nil;
			}
		}
	}
}

void
resetSurfProps(Clump *clump)
{
	FORLIST(lnk, clump->atomics){
		Geometry *g = Atomic::fromClump(lnk)->geometry;
		for(int32 i = 0; i < g->matList.numMaterials; i++){
			Material *m = g->matList.materials[i];
			//if(m->texture)
			//	printf("%24s ", m->texture->name);
			//printf("%f %f %f\n", m->surfaceProps.ambient, m->surfaceProps.diffuse, m->surfaceProps.specular);
			//m->surfaceProps.diffuse = 0.0f;
			m->surfaceProps.ambient = 1.0f;
			m->surfaceProps.diffuse = 1.0f;
			m->surfaceProps.specular = 0.0f;
		}
	}
}

void
removeUnusedMaterials(Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		Atomic::fromClump(lnk)->geometry->removeUnusedMaterials();
}

void
removeEffects(Atomic *atomic)
{
	Geometry *geo = atomic->geometry;
	for(int i = 0; i < geo->matList.numMaterials; i++){
		Material *mat = geo->matList.materials[i];
		MatFX *matfx = *PLUGINOFFSET(MatFX*, mat, matFXGlobals.materialOffset);
		if(matfx){
			matfx->type = 0;
			matfx->fx[0].type = 0;
			matfx->fx[1].type = 0;
		}
		*PLUGINOFFSET(int32*, atomic, matFXGlobals.atomicOffset) = 0;
	}
}

int
main(int argc, char *argv[])
{
	rw::version = 0;
//	rw::version = 0x34003;
//	rw::version = 0x33002;
//	rw::platform = rw::PLATFORM_PS2;
//	rw::platform = rw::PLATFORM_OGL;
//	rw::platform = rw::PLATFORM_XBOX;
	rw::platform = rw::PLATFORM_D3D8;
//	rw::platform = rw::PLATFORM_D3D9;

	rw::Engine::init();
	gta::attachPlugins();
	rw::Driver::open();

	int uninstance = 0;
	int instance = 0;
	int dump = 0;
	int surfprops = 0;
	int removebody = 0;
	int outplatform = rw::PLATFORM_D3D8;
	int correctWinding = 0;

	char *s;
	//char *seconddff = NULL;
	ARGBEGIN{
	case 'u':
		uninstance++;
		break;
	case 'i':
		instance++;
		break;
	//case 'c':
	//	seconddff = EARGF(usage());
	//	break;
	case 'v':
		sscanf(EARGF(usage()), "%x", &rw::version);
		break;
	case 'f':
		rw::streamAppendFrames = 1;
		break;
	case 'd':
		dump++;
		break;
	case 'r':
		surfprops++;
		break;
	case 'b':
		removebody++;
		break;
	case 'w':
		correctWinding++;
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
	default:
		usage();
	}ARGEND;

	if(uninstance && instance){
		fprintf(stderr, "cannot both instance and uninstance, choose one!\n");
		return 1;
	}

	if(argc < 1)
		usage();

	Clump *c;
	//uint32 len;
	//uint8 *data = getFileContents(argv[0], &len);
	//assert(data != NULL);
	//StreamMemory in;
	//in.open(data, len);
	StreamFile in;
	if(!in.open(argv[0], "rb")){
		fprintf(stderr, "Error: couldn't open %s\n", argv[0]);
		return 1;
	}
	currentUVAnimDictionary = NULL;
	TexDictionary::setCurrent(TexDictionary::create());
	ChunkHeaderInfo header;
	readChunkHeaderInfo(&in, &header);
	if(header.type == ID_UVANIMDICT){
		UVAnimDictionary *dict = UVAnimDictionary::streamRead(&in);
		currentUVAnimDictionary = dict;
		readChunkHeaderInfo(&in, &header);
	}
	if(header.type != ID_CLUMP){
		in.close();
		return 0;
	}
	debugFile = argv[0];
	c = Clump::streamRead(&in);
	in.close();
	if(c == NULL){
		fprintf(stderr, "Error: couldn't read clump\n");
		return 1;
	}

	if(surfprops)
		resetSurfProps(c);
	if(removebody)
		removeBodyTextures(c);

	//Clump *colclump = NULL;
	//if(seconddff){
	//	in.open(seconddff, "rb");
	//	readChunkHeaderInfo(&in, &header);
	//	if(header.type != ID_CLUMP){
	//		in.close();
	//		return 0;
	//	}
	//	debugFile = seconddff;
	//	colclump = Clump::streamRead(&in);
	//	assert(colclump != NULL);
	//	in.close();
	//
	//	c->copyPlugins(colclump);
	//}

//	printf("%s\n", argv[arg]);

/*
	for(int32 i = 0; i < c->numAtomics; i++){
		Atomic *a = c->atomicList[i];
		Pipeline *ap = a->pipeline;
		Geometry *g = a->geometry;
		for(int32 j = 0; j < g->numMaterials; j++){
			Pipeline *mp = g->materialList[j]->pipeline;
			if(ap && mp)
				printf("%s %x %x\n", argv[arg], ap->pluginData, mp->pluginData);
		}
	}
*/
	//FORLIST(lnk, c->lights){
	//	Light *l = Light::fromClump(lnk);
	//	printf("%p %p\n", l, lnk);
	//	printf("%d %f %f %f\n", l->getType(), l->color.red, l->color.green, l->color.blue);
	//}

	if(dump){
		HAnimHierarchy *hier = HAnimHierarchy::find(c->getFrame());
		if(hier)
			hier->attach();
		dumpFrameHier(c->getFrame());
	}

	//if(currentUVAnimDictionary){
	//	FORLIST(lnk, currentUVAnimDictionary->animations){
	//		UVAnimDictEntry *de = UVAnimDictEntry::fromDict(lnk);
	//		Animation *anim = de->anim;
	//		dumpUVAnim(anim);
	//	}
	//}

	//FORLIST(lnk, c->atomics){
	//	Atomic *atomic = Atomic::fromClump(lnk);
	//	Frame *f = atomic->getFrame();
	//	if(strstr(gta::getNodeName(f), "wheel"))
	//		removeEffects(atomic);
	//}

	//FORLIST(lnk, c->atomics){
	//	Geometry *g = Atomic::fromClump(lnk)->geometry;
	//	for(int i = 0; i < g->numMaterials; i++){
	//		Material *m = g->materialList[i];
	//		dumpReflData(m);
	//	}
	//}

	int32 platform = findPlatform(c);
	if(platform){
		rw::platform = platform;
		switchPipes(c, rw::platform);
	}

	if(uninstance)
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
			ObjPipeline *p = a->getPipeline();
			p->uninstance(a);
			if(outplatform != PLATFORM_PS2)
				ps2::unconvertADC(a->geometry);
		}

	rw::platform = outplatform;
	switchPipes(c, rw::platform);

	if(instance)
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
			ObjPipeline *p = a->getPipeline();
			p->instance(a);
			if(outplatform != PLATFORM_PS2)
				ps2::convertADC(a->geometry);
		}

	if(rw::version == 0){
		rw::version = header.version;
		rw::build = header.build;
	}

	removeUnusedMaterials(c);

	if(correctWinding)
		FORLIST(lnk, c->atomics){
			Geometry *g = Atomic::fromClump(lnk)->geometry;
			g->correctTristripWinding();
		}

	FORLIST(lnk, c->atomics){
		Geometry *g = Atomic::fromClump(lnk)->geometry;
		g->buildMeshes();
	}

	StreamFile out;
	const char *file;
	if(argc > 1)
		file = argv[1];
	else
		file = "out.dff";
	if(!out.open(file, "wb")){
		fprintf(stderr, "Error: couldn't open %s\n", file);
		return 1;
	}
	if(currentUVAnimDictionary)
		currentUVAnimDictionary->streamWrite(&out);
	c->streamWrite(&out);
	out.close();

//	data = new rw::uint8[1024*1024];
//	rw::StreamMemory out;
//	out.open(data, 0, 1024*1024);
//	if(currentUVAnimDictionary)
//		currentUVAnimDictionary->streamWrite(&out);
//	c->streamWrite(&out);
//
//	FILE *cf;
//	if(argc > 1)
//		cf = fopen(argv[1], "wb");
//	else
//		cf = fopen("out.dff", "wb");
//	assert(cf != NULL);
//	fwrite(data, out.getLength(), 1, cf);
//	fclose(cf);
//	out.close();
//	delete[] data;

	c->destroy();
	if(currentUVAnimDictionary)
		currentUVAnimDictionary->destroy();
	TexDictionary::getCurrent()->destroy();

	return 0;
}
