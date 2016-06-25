#include "gldemo.h"

Camera *camera;
rw::World *world;
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

void
hideSomeAtomics(rw::Clump *clump)
{
	using namespace rw;
	FORLIST(lnk, clump->atomics){
		Atomic *a = Atomic::fromClump(lnk);
		char *name = gta::getNodeName(a->getFrame());
		if(strstr(name, "_dam") || strstr(name, "_vlo"))
			a->object.object.flags &= ~Atomic::RENDER;
	}
}

int
initrw(void)
{
	using namespace rw;

	StreamFile in;

	rw::version = 0x34000;
	rw::platform = PLATFORM_GL3;
	rw::loadTextures = 1;

	rw::Engine::init();
	gta::attachPlugins();
	rw::Engine::open();

	rw::currentTexDictionary = TexDictionary::create();
	Image::setSearchPath("/home/aap/vc_textures/");

//	char *f = "/home/aap/gamedata/pc/gtavc/models/gta3_archive/player.dff";
//	char *f = "/home/aap/gamedata/pc/gtavc/models/gta3_archive/od_newscafe_dy.dff";
	const char *f = "models/admiral.dff";
//	char *f = "/home/aap/gamedata/pc/gta3/models/gta3_archive/player.DFF";
	if(in.open(f, "rb") == nil)
		return 0;
	findChunk(&in, ID_CLUMP, nil, nil);
	clump = Clump::streamRead(&in);
	assert(clump);
	in.close();

	hideSomeAtomics(clump);

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

	camera = new Camera;
	camera->m_rwcam = rw::Camera::create();
	camera->m_rwcam->setFrame(rw::Frame::create());
	camera->m_aspectRatio = 640.0f/480.0f;
	camera->m_near = 0.1f;
	camera->m_far = 450.0f;
	camera->m_target.set(0.0f, 0.0f, 0.0f);
	//camera->m_position.set(0.0f, -30.0f, 4.0f);
	camera->m_position.set(3.0f, 5.0f, 1.0f);

	world = rw::World::create();
	world->addCamera(camera->m_rwcam);

	// Ambient light
	rw::Light *light = rw::Light::create(rw::Light::AMBIENT);
	light->setColor(0.3f, 0.3f, 0.3f);
	world->addLight(light);

	// Diffuse light
	light = rw::Light::create(rw::Light::DIRECTIONAL);
	rw::Frame *f = rw::Frame::create();
	light->setFrame(f);
	f->matrix.pointInDirection((rw::V3d){1.0, 1.0, -1.0},
	                           (rw::V3d){0.0, 0.0, 1.0});
	f->updateObjects();
	world->addLight(light);

	return 1;
}

void
shutdown(void)
{
}
