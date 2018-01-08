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
char *inputfilename;

void
usage(void)
{
	fprintf(stderr, "usage: %s [-u] [-i] [-f] [-d] [-v version] [-o platform] in.dff [out.dff]\n", argv0);
	fprintf(stderr, "\t-u uninstance\n");
	fprintf(stderr, "\t-i instance\n");
	fprintf(stderr, "\t-f don't flip frame hierarchy\n");
	fprintf(stderr, "\t-d dump frame and hanim hierarchy\n");
	fprintf(stderr, "\t-w correct face winding of tristrips\n");
	fprintf(stderr, "\t-m extract multiclump dff\n");
	fprintf(stderr, "\t-v RW version, e.g. 33004 for 3.3.0.4\n");
	fprintf(stderr, "\t-o output platform. ps2, xbox, mobile, d3d8, d3d9\n");
	fprintf(stderr, "\t--III2VCcar specmap. convert reflections from III to VC using specmap\n");
	fprintf(stderr, "\t--ps2VCcar set up VC vehicle for use with PS2 and Xbox reflections\n");
	fprintf(stderr, "\t--info dump some info about the file\n");
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

			if(0){
			rw::Matrix *mat = h->nodeInfo[i].frame->getLTM();
		//	rw::Matrix *mat = &h->nodeInfo[i].frame->matrix;
			printf("[ [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"
			       "  [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"
			       "  [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"
			       "  [ %8.4f, %8.4f, %8.4f, %8.4f ] ]\n"
				"  %08x == flags\n",
				mat->right.x, mat->up.x, mat->at.x, mat->pos.x,
				mat->right.y, mat->up.y, mat->at.y, mat->pos.y,
				mat->right.z, mat->up.z, mat->at.z, mat->pos.z,
				0.0f, 0.0f, 0.0f, 1.0f,
				mat->flags);
			}
		}
	}
	for(Frame *child = frame->child;
	    child; child = child->next)
		dumpFrameHier(child, ind+1);
}

void
dumpMatFXData(Material *m)
{
	if(MatFX::getEffects(m) != MatFX::ENVMAP)
		return;
	MatFX *matfx = MatFX::get(m);
	int i = matfx->getEffectIndex(MatFX::ENVMAP);
	MatFX::Env *env = &matfx->fx[i].env;
	printf("\tenv: %4.2f ", env->coefficient);
	if(env->tex)
		printf("%-32s ", env->tex->name);
	printf("\n");

}

void
setupMatFX_VCPS2Xbox(Material *m)
{
	if(MatFX::getEffects(m) != MatFX::ENVMAP)
		return;
	MatFX *matfx = MatFX::get(m);
	int i = matfx->getEffectIndex(MatFX::ENVMAP);
	MatFX::Env *env = &matfx->fx[i].env;

	if(env->coefficient){
		env->coefficient = 1.0f;
		m->surfaceProps.specular = 0.5f;
	}else
		m->surfaceProps.specular = 0.0f;
}

void
dumpMat(Material *m)
{
	// remove primary color
//	if(m->color.red == 255 && m->color.green == 0 && m->color.blue == 175){
//		m->color.red = 255;
//		m->color.green = 255;
//		m->color.blue = 255;
//	}
//	if(m->color.red == 60 && m->color.green == 255 && m->color.blue == 0){
//		m->color.red = 255;
//		m->color.green = 255;
//		m->color.blue = 255;
//	}

	printf("%s %3d %3d %3d %3d %-32s %f\n", inputfilename, m->color.red, m->color.green, m->color.blue, m->color.alpha, m->texture ? m->texture->name : "NULL", m->surfaceProps.specular);
	dumpMatFXData(m);
}

int
setSpecMap(Material *m, const char *name)
{
	if(m->surfaceProps.specular != 0.0f){
		MatFX::setEffects(m, MatFX::ENVMAP);
		MatFX *mfx = MatFX::get(m);
		mfx->setEnvCoefficient(m->surfaceProps.specular);
		rw::Texture *tex = rw::Texture::read(name, nil);
		mfx->setEnvTexture(tex);
		return 1;
	}
	return 0;
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

void
extractmultiple(StreamFile &in)
{
	ChunkHeaderInfo header;
	char filename[100];
	StreamFile out;
	for(;readChunkHeaderInfo(&in, &header);){
		if(rw::version == 0){
			rw::version = header.version;
			rw::build = header.build;
		}

		if(header.type != ID_CLUMP){
			in.close();
			return;
		}
		rw::Clump *clump = rw::Clump::streamRead(&in);
		if(clump == nil){
			printf("error: couldn't read clump\n");
			return;
		}
		printf("%s\n", gta::getNodeName(clump->getFrame()));
		strcpy(filename, gta::getNodeName(clump->getFrame()));
		strcat(filename, ".dff");

		if(!out.open(filename, "wb")){
			fprintf(stderr, "Error: couldn't open %s\n", filename);
			return;
		}
		clump->streamWrite(&out);
		out.close();
	}
}

int
vertexAlpha(RGBA *col, int n)
{
	uint8 alpha = 0xFF;
	while(n--)
		alpha &= col++->alpha;
	return alpha != 0xFF;
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
	rw::Engine::open();
	rw::Engine::start(nil);

	rw::Texture::setCreateDummies(1);

	int uninstance = 0;
	int instance = 0;
	int dump = 0;
	int surfprops = 0;
	int removebody = 0;
	int outplatform = rw::PLATFORM_D3D8;
	int correctWinding = 0;
	int multiclump = 0;
	int setwhite = 0;
	int iiiToVcCar = 0;
	char *specmap = "reflection01";
	int ps2vccar = 0;
	int info = 0;

	char *s, *longarg;
	//char *seconddff = NULL;
	ARGBEGIN{
	case '-':
		// hack for long options: _args is the long option
		// and must be an empty string in the end
		longarg = _args;
		_args = "";
		if(strcmp_ci(longarg, "ps2vccar") == 0) ps2vccar++;
		else if(strcmp_ci(longarg, "iii2vccar") == 0){
			iiiToVcCar++;
			specmap = EARGF(usage());
		}else if(strcmp_ci(longarg, "info") == 0) info++;
		else usage();
		break;
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
	case 'm':
		multiclump++;
		break;
	case 'W':
		setwhite++;
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

	inputfilename = argv[0];
	StreamFile in;
	if(!in.open(argv[0], "rb")){
		fprintf(stderr, "Error: couldn't open %s\n", argv[0]);
		return 1;
	}

	if(multiclump){
		extractmultiple(in);
		return 0;
	}

	currentUVAnimDictionary = NULL;
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

	if(setwhite)
		FORLIST(lnk, c->atomics){
			Geometry *g = Atomic::fromClump(lnk)->geometry;
			for(int i = 0; i < g->matList.numMaterials; i++){
				Material *m = g->matList.materials[i];
				m->color.red = 0xFF;
				m->color.green = 0xFF;
				m->color.blue = 0xFF;
			}
		}

/*
	FORLIST(lnk, c->atomics){
		Geometry *g = Atomic::fromClump(lnk)->geometry;
		for(int i = 0; i < g->matList.numMaterials; i++){
			Material *m = g->matList.materials[i];
			dumpMat(m);
	//		dumpReflData(m);
		}
	}
*/

	if(iiiToVcCar)
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
			Geometry *g = a->geometry;
			int hasenv = 0;
			for(int i = 0; i < g->matList.numMaterials; i++){
				Material *m = g->matList.materials[i];
				hasenv |= setSpecMap(m, specmap);
			}
			if(hasenv)
				MatFX::enableEffects(a);
		}

	if(ps2vccar)
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
//			gta::setPipelineID(a, 0);
//			hAnimDoStream = 0;
			Geometry *g = a->geometry;
			for(int i = 0; i < g->matList.numMaterials; i++){
				Material *m = g->matList.materials[i];
				setupMatFX_VCPS2Xbox(m);
			}
		}
	// Make sure we have all pipes attached for uninstance
	FORLIST(lnk, c->atomics)
		gta::attachCustomPipelines(Atomic::fromClump(lnk));;
	int32 platform = findPlatform(c);
	if(platform){
		rw::platform = platform;
		switchPipes(c, rw::platform);
	}

	if(uninstance)
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
			a->uninstance();
			if(outplatform != PLATFORM_PS2)
				ps2::unconvertADC(a->geometry);
		}

	rw::platform = outplatform;
	switchPipes(c, rw::platform);

	if(instance)
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
			if(outplatform != PLATFORM_PS2)
				ps2::convertADC(a->geometry);
			a->instance();
		}

	if(rw::version == 0){
		rw::version = header.version;
		rw::build = header.build;
	}

	if(info){
		int output = 0;
#define PRINT(fmt, ...) do { if(!output) printf("%s: ", inputfilename); else putchar(' '); printf(fmt, __VA_ARGS__); output=1; }while(0)
		if(currentUVAnimDictionary)
			PRINT("uvanim");

		int hasvertalpa = 0;
		int haswetroad = 0;
		int hasskin = 0;
		int isnative = 0;
		int extracols = 0;
		int extranorms = 0;
		FORLIST(lnk, c->atomics){
			Atomic *a = Atomic::fromClump(lnk);
			Geometry *g = a->geometry;

			if(Skin::get(g))
				hasskin = 1;

			if(g->flags & Geometry::NATIVE)
				isnative = 1;

			RGBA *extracol = gta::getExtraVertColors(a);
			V3d *extranorm = gta::getExtraNormals(g);
			if(extracol) extracols = 1;
			if(extranorm) extranorms = 1;
			if(g->flags & Geometry::PRELIT && !isnative){
				assert(g->colors);
				if(vertexAlpha(g->colors, g->numVertices)){
					if(extracol) haswetroad = 1;
					else hasvertalpa = 1;
				}
				if(extracol && vertexAlpha(extracol, g->numVertices))
					hasvertalpa = 1;
			}
		}
		if(isnative) PRINT("instanced");
		if(hasskin) PRINT("skin");
		if(hasvertalpa) PRINT("vertalpha");
		if(extracols) PRINT("extracolors");
		if(extranorms) PRINT("extranormals");
		if(haswetroad) PRINT("wetroad");

		if(output)
			putchar('\n');
	}

	removeUnusedMaterials(c);

	if(correctWinding)
		FORLIST(lnk, c->atomics){
			Geometry *g = Atomic::fromClump(lnk)->geometry;
			g->correctTristripWinding();
		}

/*
	FORLIST(lnk, c->atomics){
		Geometry *g = Atomic::fromClump(lnk)->geometry;
		g->buildMeshes();
	}
*/

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

//	data = rwNewT(uint8, 1024*1024, 0);
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
//	rwFree(data);

	c->destroy();
	if(currentUVAnimDictionary)
		currentUVAnimDictionary->destroy();

	return 0;
}
