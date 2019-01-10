#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>

#include <rw.h>
#include <args.h>

#include <rwgta.h>

// shorter display, make examining the structure easier
//#define SKIPDATA

using namespace std;
using namespace rw;


static void
pr(int ind, FILE *f, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	while(ind--)
		fprintf(f, "    ");
	vfprintf(f, fmt, ap);
	va_end(ap);
}

static void
dumptex(Texture *tex, FILE *f, int ind)
{
	pr(ind++, f, "{\n");
	pr(ind, f, "name: \"%s\",\n", tex->name);
	pr(ind, f, "mask: \"%s\"\n", tex->mask);
	pr(--ind, f, "}");
}

static void
dumpmaterial(Material *m, FILE *f, int ind)
{
	pr(ind++, f, "{\n");
	pr(ind, f, "color: [ %d, %d, %d, %d ],\n",
		m->color.red, m->color.green, m->color.blue, m->color.alpha);
	pr(ind, f, "surfaceProperties: [ %f, %f, %f ]",
		m->surfaceProps.ambient, m->surfaceProps.specular, m->surfaceProps.diffuse);
	if(m->texture){
		pr(0, f, ",\n");
		pr(ind, f, "texture:\n");
		dumptex(m->texture, f, ind);
	}
	if(MatFX::getEffects(m)){
		MatFX *matfx = MatFX::get(m);
		pr(0, f, ",\n");
		pr(ind++, f, "matfx: {\n");
		pr(ind, f, "type: %d", matfx->type);
		if(matfx->type == MatFX::ENVMAP){
			pr(0, f, ",\n");
			pr(ind, f, "envCoefficient: %f", matfx->getEnvCoefficient());
			if(matfx->getEnvTexture()){
				pr(0, f, ",\n");
				pr(ind, f, "envTex:\n");
				dumptex(matfx->getEnvTexture(), f, ind);
			}
		}
		pr(0, f, "\n");
		pr(--ind, f, "}");
	}

	gta::EnvMat *env = gta::getEnvMat(m);
	if(env){
		pr(0, f, ",\n");
		pr(ind++, f, "envMap: {\n");
		pr(ind, f, "scale: [ %f, %f ],\n", env->getScaleX(), env->getScaleY());
		pr(ind, f, "transScale: [ %f, %f ],\n", env->getTransScaleX(), env->getTransScaleY());
		pr(ind, f, "shininess: %f\n", env->getShininess());
		pr(--ind, f, "}");
	}

	gta::SpecMat *spec = gta::getSpecMat(m);
	if(spec){
		pr(0, f, ",\n");
		pr(ind++, f, "specMap: {\n");
		pr(ind, f, "specularity: %f,\n", spec->specularity);
		assert(spec->texture);
		pr(ind, f, "texture: \"%s\"\n", spec->texture->name);
		pr(--ind, f, "}");
	}

	pr(0, f, "\n");
	pr(--ind, f, "}");
}

static void
dumpgeo(Geometry *g, FILE *f, int ind)
{
	int i, j;

	pr(ind++, f, "{\n");
	if(g->flags & Geometry::NATIVE){
		pr(ind, f, "NATIVE\n");
		pr(--ind, f, "}");
		return;
	}

	if(g->colors){
		pr(ind++, f, "prelit: [\n");
#ifndef SKIPDATA
		for(j = 0; j < g->numVertices; j++)
			pr(ind, f, "[ %d, %d, %d, %d ]%s\n",
				g->colors[j].red,
				g->colors[j].green,
				g->colors[j].blue,
				g->colors[j].alpha,
				j == g->numVertices-1 ? "" : ",");
#endif
		pr(--ind, f, "],\n");
	}

	if(g->numTexCoordSets){
		pr(ind++, f, "texCoords: [\n");
		for(i = 0; i < g->numTexCoordSets; i++){
			pr(ind++, f, "[\n");
#ifndef SKIPDATA
			for(j = 0; j < g->numVertices; j++)
				pr(ind, f, "[ %f, %f ]%s\n",
					g->texCoords[i][j].u,
					g->texCoords[i][j].v,
					j == g->numVertices-1 ? "" : ",");
#endif
			pr(--ind, f, "]%s\n", i==g->numTexCoordSets-1 ? "" : ",");
		}
		pr(--ind, f, "],\n");
	}

	pr(ind++, f, "triangles: [\n");
#ifndef SKIPDATA
	for(i = 0; i < g->numTriangles; i++)
		pr(ind, f, "[ %d, %d, %d, %d ]%s\n",
			g->triangles[i].v[0],
			g->triangles[i].v[1],
			g->triangles[i].v[2],
			g->triangles[i].matId,
			i == g->numTriangles-1 ? "" : ",");
#endif
	pr(--ind, f, "],\n");

	pr(ind++, f, "morphTargets: [\n");
	for(i = 0; i < g->numMorphTargets; i++){
		pr(ind++, f, "{\n");
		MorphTarget *mt = &g->morphTargets[i];
		if(mt->vertices){
			pr(ind++, f, "vertices: [\n");
#ifndef SKIPDATA
			for(j = 0; j < g->numVertices; j++)
				pr(ind, f, "[ %f, %f, %f ]%s\n",
					mt->vertices[j].x,
					mt->vertices[j].y,
					mt->vertices[j].z,
					j == g->numVertices-1 ? "" : ",");
#endif
			pr(--ind, f, "]%s\n", mt->normals ? "," : "");
		}
		if(mt->normals){
			pr(ind++, f, "normals: [\n");
#ifndef SKIPDATA
			for(j = 0; j < g->numVertices; j++)
				pr(ind, f, "[ %f, %f, %f ]%s\n",
					mt->normals[j].x,
					mt->normals[j].y,
					mt->normals[j].z,
					j == g->numVertices-1 ? "" : ",");
#endif
			pr(--ind, f, "]\n");
		}
		pr(--ind, f, "}%s\n", i == g->numMorphTargets-1 ? "" : ",");
	}
	pr(--ind, f, "],\n");

	pr(ind++, f, "materials: [\n");
	for(i = 0; i < g->matList.numMaterials; i++){
		dumpmaterial(g->matList.materials[i], f, ind);
		pr(0, f, "%s\n", i == g->matList.numMaterials-1 ? "" : ",");
	}
	pr(--ind, f, "],\n");

	MeshHeader *h = g->meshHeader;
	pr(ind, f, "meshtype: %d,\n", h->flags);
	pr(ind++, f, "meshes: [\n");
	Mesh *m = h->getMeshes();
	for(i = 0; i < h->numMeshes; i++){
		pr(ind++, f, "{\n");
		pr(ind, f, "matId: %d,\n", findPointer(m->material, (void**)g->matList.materials, g->matList.numMaterials));
		pr(ind++, f, "indices: [\n");
#ifndef SKIPDATA
		for(j = 0; j < m->numIndices; j++)
			pr(ind, f, "%d%s\n", m->indices[j],
				j == m->numIndices-1 ? "" : ",");
#endif
		pr(--ind, f, "]\n");
		pr(--ind, f, "}%s\n", i == h->numMeshes-1 ? "" : ",");
		m++;
	}
	pr(--ind, f, "]\n");

	pr(--ind, f, "}");
}

static void
dumpatomic(Atomic *a, FrameList_ *frmlist, FILE *f, int ind)
{
	pr(ind++, f, "{\n");
	pr(ind, f, "frame: %d,\n", findPointer(a->getFrame(), (void**)frmlist->frames, frmlist->numFrames));
	pr(ind, f, "geometry:\n");
	dumpgeo(a->geometry, f, ind);
	pr(0, f, ",\n");
	pr(ind, f, "matfx: %d\n", MatFX::getEffects(a));
	pr(--ind, f, "}");
}

static void
dumpframe(Frame *frm, FrameList_ *frmlist, FILE *f, int ind)
{
	pr(ind++, f, "{\n");
	pr(ind, f, "parent: %d,\n", findPointer(frm->getParent(), (void**)frmlist->frames, frmlist->numFrames));
	pr(ind, f, "matrix: [ %f, %f, %f,  %f, %f, %f,  %f, %f, %f,  %f, %f, %f ],\n",
		frm->matrix.right.x, frm->matrix.right.y, frm->matrix.right.z,
		frm->matrix.up.x, frm->matrix.up.y, frm->matrix.up.z,
		frm->matrix.at.x, frm->matrix.at.y, frm->matrix.at.z,
		frm->matrix.pos.x, frm->matrix.pos.y, frm->matrix.pos.z);
	// TODO? hanim
	pr(ind, f, "name: \"%s\"\n", gta::getNodeName(frm));
	pr(--ind, f, "}");
}

static void
dumpframes(FrameList_ *frmlist, FILE *f, int ind)
{
	int i;

	pr(ind++, f, "frames: [\n");
	int first = 1;
	for(i = 0; i < frmlist->numFrames; i++){
		if(!first)
			pr(0, f, ",\n");
		first = 0;
		dumpframe(frmlist->frames[i], frmlist, f, ind);
	}
	pr(0, f, "\n");
	pr(--ind, f, "],\n");
}

static void
dumpclump(Clump *c, FILE *f, int ind)
{
	pr(ind++, f, "{\n");

	FrameList_ frmlst;
	frmlst.numFrames = c->getFrame()->count();
	frmlst.frames = (Frame**)rwMalloc(frmlst.numFrames*sizeof(Frame*), MEMDUR_FUNCTION | ID_CLUMP);
	makeFrameList(c->getFrame(), frmlst.frames);
	dumpframes(&frmlst, f, ind);

	pr(ind++, f, "atomics: [\n");
	int first = 1;
	FORLIST(lnk, c->atomics){
		Atomic *a = Atomic::fromClump(lnk);
		if(!first)
			pr(0, f, ",\n");
		first = 0;
		dumpatomic(a, &frmlst, f, ind);
	}
	pr(0, f, "\n");
	pr(--ind, f, "]\n");

	rwFree(frmlst.frames);

	pr(--ind, f, "}\n");
}

void
dumpjson(Clump *c, const char *file)
{
	FILE *f;
	f = fopen(file, "w");
	if(f == nil)
		return;
	dumpclump(c, f, 0);
	fclose(f);
}
