#include "gldemo.h"

Camera *camera;
rw::Clump *clump;
rw::gl3::Shader *simpleshader;

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->update();
	camera->m_rwcam->beginUpdate();

	simpleshader->use();
	clump->render();

	camera->m_rwcam->endUpdate();
}

void
pullinput(GLFWwindow *window)
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
	if(ds->square) camera->zoom(0.4f*sensitivity);
	if(ds->cross) camera->zoom(-0.4f*sensitivity);
	camera->orbit(ds->leftX/30.0f*sensitivity,
	              -ds->leftY/30.0f*sensitivity);
	camera->turn(-ds->rightX/30.0f*sensitivity,
	             -ds->rightY/30.0f*sensitivity);
	if(ds->up)
		camera->dolly(0.4f*sensitivity);
	if(ds->down)
		camera->dolly(-0.4f*sensitivity);
}

void
keypress(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_RELEASE)
		return;
	switch(key){
	case GLFW_KEY_W:
		camera->orbit(0.0f, 0.1f);
		break;

	case GLFW_KEY_S:
		camera->orbit(0.0f, -0.1f);
		break;

	case GLFW_KEY_A:
		camera->orbit(-0.1f, 0.0f);
		break;

	case GLFW_KEY_D:
		camera->orbit(0.1f, 0.0f);
		break;

	case GLFW_KEY_UP:
		camera->turn(0.0f, 0.1f);
		break;

	case GLFW_KEY_DOWN:
		camera->turn(0.0f, -0.1f);
		break;

	case GLFW_KEY_LEFT:
		camera->turn(0.1f, 0.0f);
		break;

	case GLFW_KEY_RIGHT:
		camera->turn(-0.1f, 0.0f);
		break;

	case GLFW_KEY_R:
		camera->zoom(-0.4f);
		break;

	case GLFW_KEY_F:
		camera->zoom(0.4f);
		break;

	case GLFW_KEY_ESCAPE:
	case GLFW_KEY_Q:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	}
}

int
initrw(void)
{
	using namespace rw;

	StreamFile in;

	rw::version = 0x34000;
	rw::platform = PLATFORM_GL3;
	gta::attachPlugins();
	gl3::registerNativeRaster();
	gl3::initializePlatform();


//	char *f = "/home/aap/gamedata/pc/gtavc/models/gta3_archive/player.dff";
//	char *f = "/home/aap/gamedata/pc/gtavc/models/gta3_archive/od_newscafe_dy.dff";
	char *f = "/home/aap/gamedata/pc/gtavc/models/gta3_archive/admiral.dff";
//	char *f = "/home/aap/gamedata/pc/gta3/models/gta3_archive/player.DFF";
	if(in.open(f, "rb") == nil)
		return 0;
	findChunk(&in, ID_CLUMP, nil, nil);
	clump = Clump::streamRead(&in);
	assert(clump);
	in.close();

	return 1;
}

int
init(void)
{
	if(!initrw())
		return 0;

	simpleshader = rw::gl3::Shader::fromFiles("1.vert", "1.frag");
	if(simpleshader == nil)
		return 0;

	glClearColor(0.08, 0.21, 0.26, 1.0);
//	glClearColor(0xA1/255.0f, 0xA1/255.0f, 0xA1/255.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	camera = new Camera;
	camera->m_rwcam = rw::Camera::create();
	camera->m_rwcam->setFrame(rw::Frame::create());
	camera->m_aspectRatio = 640.0f/480.0f;
	camera->m_near = 0.1f;
	camera->m_far = 450.0f;
	camera->m_target.set(0.0f, 0.0f, 0.0f);
	//camera->m_position.set(0.0f, -30.0f, 4.0f);
	camera->m_position.set(0.0f, -10.0f, 4.0f);

	return 1;
}

void
shutdown(void)
{
}
