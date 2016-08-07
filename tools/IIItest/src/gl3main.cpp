#include "III.h"

Dualshock ds3;

int
getTimeInMS(void)
{
	return glfwGetTime()*1000;
}

uchar oldKeystates[KEY_NUMKEYS];
uchar newKeystates[KEY_NUMKEYS];

bool
IsKeyJustDown(int key)
{
	return oldKeystates[key] == 0 && newKeystates[key] == 1;
}

bool
IsKeyDown(int key)
{
	return newKeystates[key] == 1;
}

int keymap[GLFW_KEY_LAST];

void
initkeymap(void)
{
	int i;
	for(i = 0; i < GLFW_KEY_LAST; i++)
		keymap[i] = KEY_NULL;
	keymap[GLFW_KEY_SPACE] = ' ';
	keymap[GLFW_KEY_APOSTROPHE] = '\'';
	keymap[GLFW_KEY_COMMA] = ',';
	keymap[GLFW_KEY_MINUS] = '-';
	keymap[GLFW_KEY_PERIOD] = '.';
	keymap[GLFW_KEY_SLASH] = '/';
	keymap[GLFW_KEY_0] = '0';
	keymap[GLFW_KEY_1] = '1';
	keymap[GLFW_KEY_2] = '2';
	keymap[GLFW_KEY_3] = '3';
	keymap[GLFW_KEY_4] = '4';
	keymap[GLFW_KEY_5] = '5';
	keymap[GLFW_KEY_6] = '6';
	keymap[GLFW_KEY_7] = '7';
	keymap[GLFW_KEY_8] = '8';
	keymap[GLFW_KEY_9] = '9';
	keymap[GLFW_KEY_SEMICOLON] = ';';
	keymap[GLFW_KEY_EQUAL] = '=';
	keymap[GLFW_KEY_A] = 'A';
	keymap[GLFW_KEY_B] = 'B';
	keymap[GLFW_KEY_C] = 'C';
	keymap[GLFW_KEY_D] = 'D';
	keymap[GLFW_KEY_E] = 'E';
	keymap[GLFW_KEY_F] = 'F';
	keymap[GLFW_KEY_G] = 'G';
	keymap[GLFW_KEY_H] = 'H';
	keymap[GLFW_KEY_I] = 'I';
	keymap[GLFW_KEY_J] = 'J';
	keymap[GLFW_KEY_K] = 'K';
	keymap[GLFW_KEY_L] = 'L';
	keymap[GLFW_KEY_M] = 'M';
	keymap[GLFW_KEY_N] = 'N';
	keymap[GLFW_KEY_O] = 'O';
	keymap[GLFW_KEY_P] = 'P';
	keymap[GLFW_KEY_Q] = 'Q';
	keymap[GLFW_KEY_R] = 'R';
	keymap[GLFW_KEY_S] = 'S';
	keymap[GLFW_KEY_T] = 'T';
	keymap[GLFW_KEY_U] = 'U';
	keymap[GLFW_KEY_V] = 'V';
	keymap[GLFW_KEY_W] = 'W';
	keymap[GLFW_KEY_X] = 'X';
	keymap[GLFW_KEY_Y] = 'Y';
	keymap[GLFW_KEY_Z] = 'Z';
	keymap[GLFW_KEY_LEFT_BRACKET] = '[';
	keymap[GLFW_KEY_BACKSLASH] = '\\';
	keymap[GLFW_KEY_RIGHT_BRACKET] = ']';
	keymap[GLFW_KEY_GRAVE_ACCENT] = '`';
	keymap[GLFW_KEY_ESCAPE] = KEY_ESC;
	keymap[GLFW_KEY_ENTER] = KEY_ENTER;
	keymap[GLFW_KEY_TAB] = KEY_TAB;
	keymap[GLFW_KEY_BACKSPACE] = KEY_BACKSP;
	keymap[GLFW_KEY_INSERT] = KEY_INS;
	keymap[GLFW_KEY_DELETE] = KEY_DEL;
	keymap[GLFW_KEY_RIGHT] = KEY_RIGHT;
	keymap[GLFW_KEY_LEFT] = KEY_LEFT;
	keymap[GLFW_KEY_DOWN] = KEY_DOWN;
	keymap[GLFW_KEY_UP] = KEY_UP;
	keymap[GLFW_KEY_PAGE_UP] = KEY_PGUP;
	keymap[GLFW_KEY_PAGE_DOWN] = KEY_PGDN;
	keymap[GLFW_KEY_HOME] = KEY_HOME;
	keymap[GLFW_KEY_END] = KEY_END;
	keymap[GLFW_KEY_CAPS_LOCK] = KEY_CAPSLK;
	keymap[GLFW_KEY_SCROLL_LOCK] = KEY_NULL;
	keymap[GLFW_KEY_NUM_LOCK] = KEY_NULL;
	keymap[GLFW_KEY_PRINT_SCREEN] = KEY_NULL;
	keymap[GLFW_KEY_PAUSE] = KEY_NULL;

	keymap[GLFW_KEY_F1] = KEY_F1;
	keymap[GLFW_KEY_F2] = KEY_F2;
	keymap[GLFW_KEY_F3] = KEY_F3;
	keymap[GLFW_KEY_F4] = KEY_F4;
	keymap[GLFW_KEY_F5] = KEY_F5;
	keymap[GLFW_KEY_F6] = KEY_F6;
	keymap[GLFW_KEY_F7] = KEY_F7;
	keymap[GLFW_KEY_F8] = KEY_F8;
	keymap[GLFW_KEY_F9] = KEY_F9;
	keymap[GLFW_KEY_F10] = KEY_F10;
	keymap[GLFW_KEY_F11] = KEY_F11;
	keymap[GLFW_KEY_F12] = KEY_F12;
	keymap[GLFW_KEY_F13] = KEY_NULL;
	keymap[GLFW_KEY_F14] = KEY_NULL;
	keymap[GLFW_KEY_F15] = KEY_NULL;
	keymap[GLFW_KEY_F16] = KEY_NULL;
	keymap[GLFW_KEY_F17] = KEY_NULL;
	keymap[GLFW_KEY_F18] = KEY_NULL;
	keymap[GLFW_KEY_F19] = KEY_NULL;
	keymap[GLFW_KEY_F20] = KEY_NULL;
	keymap[GLFW_KEY_F21] = KEY_NULL;
	keymap[GLFW_KEY_F22] = KEY_NULL;
	keymap[GLFW_KEY_F23] = KEY_NULL;
	keymap[GLFW_KEY_F24] = KEY_NULL;
	keymap[GLFW_KEY_F25] = KEY_NULL;
	keymap[GLFW_KEY_KP_0] = KEY_NULL;
	keymap[GLFW_KEY_KP_1] = KEY_NULL;
	keymap[GLFW_KEY_KP_2] = KEY_NULL;
	keymap[GLFW_KEY_KP_3] = KEY_NULL;
	keymap[GLFW_KEY_KP_4] = KEY_NULL;
	keymap[GLFW_KEY_KP_5] = KEY_NULL;
	keymap[GLFW_KEY_KP_6] = KEY_NULL;
	keymap[GLFW_KEY_KP_7] = KEY_NULL;
	keymap[GLFW_KEY_KP_8] = KEY_NULL;
	keymap[GLFW_KEY_KP_9] = KEY_NULL;
	keymap[GLFW_KEY_KP_DECIMAL] = KEY_NULL;
	keymap[GLFW_KEY_KP_DIVIDE] = KEY_NULL;
	keymap[GLFW_KEY_KP_MULTIPLY] = KEY_NULL;
	keymap[GLFW_KEY_KP_SUBTRACT] = KEY_NULL;
	keymap[GLFW_KEY_KP_ADD] = KEY_NULL;
	keymap[GLFW_KEY_KP_ENTER] = KEY_NULL;
	keymap[GLFW_KEY_KP_EQUAL] = KEY_NULL;
	keymap[GLFW_KEY_LEFT_SHIFT] = KEY_LSHIFT;
	keymap[GLFW_KEY_LEFT_CONTROL] = KEY_LCTRL;
	keymap[GLFW_KEY_LEFT_ALT] = KEY_LALT;
	keymap[GLFW_KEY_LEFT_SUPER] = KEY_NULL;
	keymap[GLFW_KEY_RIGHT_SHIFT] = KEY_RSHIFT;
	keymap[GLFW_KEY_RIGHT_CONTROL] = KEY_RCTRL;
	keymap[GLFW_KEY_RIGHT_ALT] = KEY_RALT;
	keymap[GLFW_KEY_RIGHT_SUPER] = KEY_NULL;
	keymap[GLFW_KEY_MENU] = KEY_NULL;
}

void
keypress(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key >= 0 && key <= GLFW_KEY_LAST){
		if(action == GLFW_RELEASE) newKeystates[keymap[key]] = 0;
		if(action == GLFW_PRESS)   newKeystates[keymap[key]] = 1;
	}
//glfwSetWindowShouldClose(window, GLFW_TRUE);
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

	initkeymap();

	if(!init())
		return 1;

	double lastTime = glfwGetTime();
	double time;
	while(!glfwWindowShouldClose(win)){
		memcpy(oldKeystates, newKeystates, sizeof(oldKeystates));

		if(pad >= 0)
			pollDS3(pad, &ds3);
		pollinput(win);
		glfwPollEvents();

		time = glfwGetTime();
		update(time-lastTime);
		lastTime = time;
		display();
		glfwSwapBuffers(win);
	}

	shutdown();

	glfwDestroyWindow(win);
	glfwTerminate();
	return 0;
}
