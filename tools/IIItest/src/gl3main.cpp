#include "III.h"

Dualshock ds3;

int
getTimeInMS(void)
{
	return glfwGetTime()*1000;
}

void
keypress(GLFWwindow *window, int key, int scancode, int action, int mods)
{
        if(action == GLFW_RELEASE)
                return;
        switch(key){
	case GLFW_KEY_W:
		TheCamera.orbit(0.0f, 0.1f);
		break;

	case GLFW_KEY_S:
		TheCamera.orbit(0.0f, -0.1f);
		break;

	case GLFW_KEY_A:
		TheCamera.orbit(-0.1f, 0.0f);
		break;

	case GLFW_KEY_D:
		TheCamera.orbit(0.1f, 0.0f);
		break;

	case GLFW_KEY_UP:
		TheCamera.turn(0.0f, 0.1f);
		break;

	case GLFW_KEY_DOWN:
		TheCamera.turn(0.0f, -0.1f);
		break;

	case GLFW_KEY_LEFT:
		TheCamera.turn(0.1f, 0.0f);
		break;

	case GLFW_KEY_RIGHT:
		TheCamera.turn(-0.1f, 0.0f);
		break;

	case GLFW_KEY_R:
//		TheCamera.zoom(0.4f);
		TheCamera.zoom(10.0f);
		break;

	case GLFW_KEY_F:
//		TheCamera.zoom(-0.4f);
		TheCamera.zoom(-10.0f);
		break;

        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
	}
}

void
pollinput(GLFWwindow *window)
{
	Dualshock *ds = &ds3;
	if(ds->start && ds->select)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	float sensitivity = 1.0f;
	if(ds->r2){
		sensitivity = 2.0f;
		if(ds->l2)
			sensitivity = 4.0f;
	}
	if(ds->square) TheCamera.zoom(0.4f*sensitivity);
	if(ds->cross) TheCamera.zoom(-0.4f*sensitivity);
	TheCamera.orbit(ds->leftX/30.0f*sensitivity,
	                -ds->leftY/30.0f*sensitivity);
	TheCamera.turn(-ds->rightX/30.0f*sensitivity,
	               ds->rightY/30.0f*sensitivity);
	if(ds->up)
		TheCamera.dolly(2.0f*sensitivity);
	if(ds->down)
		TheCamera.dolly(-2.0f*sensitivity);
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

	double lastTime = glfwGetTime();
	double time;
	while(!glfwWindowShouldClose(win)){
		if(pad >= 0)
			pollDS3(pad, &ds3);
		pollinput(win);
		time = glfwGetTime();
		update(time-lastTime);
		lastTime = time;
		display();
		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	shutdown();

	glfwDestroyWindow(win);
	glfwTerminate();
	return 0;
}
