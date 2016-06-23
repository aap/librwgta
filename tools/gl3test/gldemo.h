#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <rw.h>
#include "rwgl3.h"
#include "rwgl3shader.h"
#include <rwgta.h>

#include "input.h"
#include "camera.h"

enum
{
	ATTRIB_POS = 0,
	ATTRIB_NORMAL,
	ATTRIB_COLOR,
	ATTRIB_TEXCOORDS0,
	ATTRIB_TEXCOORDS1,
	ATTRIB_TEXCOORDS2,
	ATTRIB_TEXCOORDS3,
	ATTRIB_TEXCOORDS4,
	ATTRIB_TEXCOORDS5,
	ATTRIB_TEXCOORDS6,
	ATTRIB_TEXCOORDS7,
};

/* less screaming */
#define nil NULL

int init(void);
void shutdown(void);
void display(void);
void pullinput(GLFWwindow*);
void keypress(GLFWwindow*, int key, int scancode, int action, int mods);

int compileshader(GLenum type, const char *src, GLuint *shader);
int linkprogram(GLint vs, GLint fs, GLuint *program);
void printlog(GLuint object);
char *loadfile(const char *path);

