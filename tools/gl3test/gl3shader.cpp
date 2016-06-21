#include <cstdio>
#include <cstring>

#include "gldemo.h"

namespace rw {
namespace gl3 {

UniformRegistry uniformRegistry;

int
registerUniform(const char *name)
{
	int i;
	i = findUniform(name);
	if(i >= 0) return i;
	uniformRegistry.uniformNames[uniformRegistry.numUniforms] = strdup(name);
	return uniformRegistry.numUniforms++;
}

int
findUniform(const char *name)
{
	int i;
	for(i = 0; i < uniformRegistry.numUniforms; i++)
		if(strcmp(name, uniformRegistry.uniformNames[i]) == 0)
			return i;
	return -1;
}

int
registerBlock(const char *name)
{
	int i;
	i = findBlock(name);
	if(i >= 0) return i;
	uniformRegistry.blockNames[uniformRegistry.numBlocks] = strdup(name);
	return uniformRegistry.numBlocks++;
}

int
findBlock(const char *name)
{
	int i;
	for(i = 0; i < uniformRegistry.numBlocks; i++)
		if(strcmp(name, uniformRegistry.blockNames[i]) == 0)
			return i;
	return -1;
}

Shader *currentShader;

Shader*
Shader::fromFiles(const char *vspath, const char *fspath)
{
	GLuint vs, fs, program;
	int i;
	char *src;
	int fail;

	src = loadfile(vspath);
	fail = compileshader(GL_VERTEX_SHADER, src, &vs);
	free(src);
	if(fail)
		return nil;

	src = loadfile(fspath);
	fail = compileshader(GL_FRAGMENT_SHADER, src, &fs);
	free(src);
	if(fail)
		return nil;

	fail = linkprogram(vs, fs, &program);
	if(fail)
		return nil;
	glDeleteProgram(vs);
	glDeleteProgram(fs);

	Shader *sh = new Shader;

	// set uniform block binding
	for(i = 0; i < uniformRegistry.numBlocks; i++){
		int idx = glGetUniformBlockIndex(program,
		                                 uniformRegistry.blockNames[i]);
		if(idx >= 0)
			glUniformBlockBinding(program, idx, i);
	}

	// query uniform locations
	sh->program = program;
	sh->uniformLocations = new GLint[uniformRegistry.numUniforms];
	for(i = 0; i < uniformRegistry.numUniforms; i++)
		sh->uniformLocations[i] = glGetUniformLocation(program,
			uniformRegistry.uniformNames[i]);

	return sh;
}

void
Shader::use(void)
{
	glUseProgram(this->program);
	currentShader = this;
}

}
}
