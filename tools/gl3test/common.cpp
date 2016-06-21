#include "gldemo.h"

Dualshock ds3;

int
compileshader(GLenum type, const char *src, GLuint *shader)
{
	GLint shdr, success;

	shdr = glCreateShader(type);
	glShaderSource(shdr, 1, &src, nil);
	glCompileShader(shdr);
	glGetShaderiv(shdr, GL_COMPILE_STATUS, &success);
	if(!success){
		fprintf(stderr, "Error in %s shader\n",
		  type == GL_VERTEX_SHADER ? "vertex" : "fragment");
		printlog(shdr);
		return 1;
	}
	*shader = shdr;
	return 0;
}

int
linkprogram(GLint vs, GLint fs, GLuint *program)
{
	GLint prog, success;

	prog = glCreateProgram();

	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if(!success){
		fprintf(stderr, "Error in linker\n");
		printlog(prog);
		return 1;
	}
	*program = prog;
	return 0;
}

void
printlog(GLuint object)
{
        GLint log_length;
        char *log;

        if (glIsShader(object))
                glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
        else if (glIsProgram(object))
                glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
        else{
                fprintf(stderr, "printlog: Not a shader or a program\n");
                return;
        }

        log = (char*) malloc(log_length);
        if(glIsShader(object))
                glGetShaderInfoLog(object, log_length, nil, log);
        else if(glIsProgram(object))
                glGetProgramInfoLog(object, log_length, nil, log);
        fprintf(stderr, "%s", log);
        free(log);
}

char*
loadfile(const char *path)
{
	FILE *f;
	char *buf;
	long len;

	if(f = fopen(path, "rb"), f == nil){
		fprintf(stderr, "Couldn't open file %s\n", path);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	buf = (char*)malloc(len+1);
	rewind(f);
	fread(buf, 1, len, f);
	buf[len] = '\0';
	fclose(f);
	return buf;
}

void
pollDS3(int n, Dualshock *ds)
{
//	printf("joy: %s\n", glfwGetJoystickName(GLFW_JOYSTICK_1+n));
	int count;
	const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1+n, &count);
//	for(int j = 0; j < count; j++)
//		printf("axis %d: %f\n", j, axes[j]);
	ds->leftX = axes[0];
	ds->leftY = axes[1];
	ds->rightX = axes[2];
	ds->rightY = axes[3];
	const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1+n, &count);
//	for(int j = 0; j < count; j++)
//		printf("button %d: %d\n", j, buttons[j]);
	ds->triangle = buttons[12];
	ds->circle   = buttons[13];
	ds->cross    = buttons[14];
	ds->square   = buttons[15];
	ds->l1       = buttons[10];
	ds->l2       = buttons[8];
	ds->l3       = buttons[1];
	ds->r1       = buttons[11];
	ds->r2       = buttons[9];
	ds->r3       = buttons[2];
	ds->select   = buttons[0];
	ds->start    = buttons[3];
	ds->up       = buttons[4];
	ds->right    = buttons[5];
	ds->down     = buttons[6];
	ds->left     = buttons[7];
}

int
main(int argc, char *argv[])
{
	GLenum status;
	GLFWwindow *win;

	/* Init GLFW */

	if(!glfwInit()){
		fprintf(stderr, "Error: could not initialize GLFW\n");
		return 1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	win = glfwCreateWindow(640, 480, "OpenGL", 0, 0);
	if(win == nil){
		fprintf(stderr, "Error: could not create GLFW window\n");
		glfwTerminate();
		return 1;
	}
	glfwSetKeyCallback(win, keypress);

	int pad = -1;
	for(int i = 0; i < 16; i++){
		int present = glfwJoystickPresent(GLFW_JOYSTICK_1+i);
		if(present){
			pad = i;
			break;
		}
	}
	glfwMakeContextCurrent(win);

	/* Init GLEW */

	glewExperimental = GL_TRUE;
	status = glewInit();
	if(status != GLEW_OK){
		fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
		return 1;
	}
	if(!GLEW_VERSION_3_3){
		fprintf(stderr, "Error: OpenGL 3.3 needed\n");
		return 1;
	}

	if(!init())
		return 1;

	while(!glfwWindowShouldClose(win)){
		if(pad >= 0)
			pollDS3(pad, &ds3);
		pullinput(win);
		display();
		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	shutdown();

	glfwDestroyWindow(win);
	glfwTerminate();
	return 0;
}
