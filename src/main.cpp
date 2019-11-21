/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/11/21 16:53:36 by rhoffsch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "humangl.h"
#include "human.hpp"
#include "humanevolved.hpp"
#include "supersaiyan1.hpp"
#include "framebuffer.hpp"
#include "animationbh.hpp"

#include <string>
#include <cstdio>
#include <vector>
#include <list>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <json/json.hpp>
using json = nlohmann::json;

class UIPanel {
public:
	UIPanel() {
		this->texture = nullptr;
		this->posX = 0;
		this->posY = 0;
		this->width = 100;
		this->height = 100;
	}
	UIPanel(Texture * tex) {
		this->texture = tex;
		this->posX = 0;
		this->posY = 0;
		this->width = tex->getWidth();
		this->height = tex->getHeight();
	}
	~UIPanel() {}
	Texture *	texture;
	int			posX;
	int			posY;
	int			width;
	int 		height;
private:
};

class HumanManager : public GameManager {
public:
	HumanManager() : GameManager() {
		this->obj3dList = nullptr;
		this->human = nullptr;
		this->framebuffer = nullptr;
	}
	~HumanManager() {
	}
	list<Obj3d*> *	obj3dList;
	Human *			human;
	FrameBuffer *	framebuffer;
	FrameBuffer *	framebufferUI;
	FrameBuffer *	framebufferUI2;
	UIPanel *		uiPalette;
	UIPanel *		uiLength;
	UIPanel *		uiSelect;
private:
};

static void blitToWindow(HumanManager * manager, FrameBuffer * readFramebuffer, GLenum attachmentPoint, UIPanel *panel) {
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer->fbo);

	(void)manager;
	//glViewport(0, 0, manager->glfw->getWidth(), manager->glfw->getHeight());//size of the window/image or panel width ?
	glReadBuffer(attachmentPoint);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	int w;
	int h;
	if (panel->texture) {
		w = panel->texture->getWidth();
		h = panel->texture->getHeight();
	} else {
		w = readFramebuffer->getWidth();
		h = readFramebuffer->getHeight();
	}
	if (0) {
		std::cout << "copy " << w << "x" << h << "\tresized\t" << panel->width << "x" << panel->height \
			<< "\tat pos\t" << panel->posX << ":" << panel->posY << std::endl;
			// << " -> " << (panel->posX + panel->width) << "x" << (panel->posY + panel->height) << std::endl;
	}
	glBlitFramebuffer(0, 0, w, h, \
		panel->posX, panel->posY, panel->posX + panel->width, panel->posY + panel->height, \
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

static void	getObjectOnClic(int glX, int glY, HumanManager * manager, bool resetMatrixChecks = false) {// opengl system!
	if (!manager->human || !manager->glfw || !manager->cam || !manager->framebuffer) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : one of several HumanManager pointer is null." << std::endl;
		return ;
	}
	// list<Obj3d*>	obj3dList = manager->human->getObjList();//should return full list depending on Human type
	list<Obj3d*> 	obj3dList = *(manager->obj3dList);

	//render in the offscreen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, manager->framebuffer->fbo);
	glClearColor(1, 1, 1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Obj3d*		obj = *(obj3dList.begin());
	Obj3dPG&	pg = obj->getProgram();
	glUseProgram(pg._program);//used once for all obj3d
	Math::Matrix4	proMatrix(manager->cam->getProjectionMatrix());
	Math::Matrix4	viewMatrix = manager->cam->getViewMatrix();
	proMatrix.mult(viewMatrix);// do it in shader ? NO cauz shader will do it for every vertix
	for (Obj3d* object : obj3dList)
		pg.renderUniqueId(*object, proMatrix);
	if (resetMatrixChecks) {
		for (Obj3d* o : obj3dList) { // this has to be done after all parented objects are rendered!
			//we need to update parent to false too, or child will update its matrix continuously
			o->local._matrixChanged = false;
			o->_worldMatrixChanged = false;
		}
	}


	//read the pixel on x y, get the ID
	GLubyte data[4];//RGBA
	glReadPixels(glX, glY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);
	unsigned int id = 0;
	Misc::RGBToInt(&(*data), &id);
	for (auto it : obj3dList) {
		if (it->getId() == id) {
			std::cout << "pos: " << glX << " - " << glY << std::endl;
			std::cout << "id is " << id << " (" << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2] << ")" << std::endl;
			std::cout << "searching from " << obj3dList.size() << " obj3d" << std::endl;
			std::cout << "Found!" << std::endl;
			manager->currentSelection = it;
		}
	}
	//restore window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void	setMemberColor(int glX, int glY, HumanManager * manager) {// opengl system!
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLubyte data[4];//RGBA
	glReadPixels(glX, glY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);
	if (manager->currentSelection) {
		Obj3d *		o = dynamic_cast<Obj3d*>(manager->currentSelection);
		if (!o) { std::cout << __PRETTY_FUNCTION__ << " : Error : dymanic_cast failed" << std::endl; exit(2); }
		std::cout << "color: " << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << std::endl;
		o->setColor(data[0], data[1], data[2]);
	}
	
}
static void	setMemberLenght(int glX, int glY, HumanManager * manager) {// opengl system!
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	(void)glX;
	(void)glY;
	(void)manager;
}

static bool	isOnUIPanel(int glX, int glY, UIPanel * panel) {// opengl system!
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	if (!panel)
		return false;
	int x0 = panel->posX;
	int y0 = panel->posY;
	int x1 = panel->posX + panel->width;
	int y1 = panel->posY + panel->height;

	std::cout << "glX: " << glX << std::endl;
	std::cout << "glY: " << glY << std::endl;
	return (glX >= x0 && glX <= x1 && glY >= y0 && glY <= y1);
}

void	HumanMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	(void)window;(void)button;(void)action;(void)mods;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	if (action == GLFW_PRESS) {
		HumanManager * manager = static_cast<HumanManager*>(glfwGetWindowUserPointer(window));//dynamic_cast cannot work, as void* lose polymorphisme metadata
		if (!manager) {
			std::cout << "Error: Wrong pointer type, HumanManager * expected" << std::endl;
			return ;
		}
		if (manager->glfw->cursorFree == true) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			std::cout << x << " : " << y;
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				std::cout << "\tright button" << std::endl;
			else if (button == GLFW_MOUSE_BUTTON_LEFT)
				std::cout << "\tleft button" << std::endl;

			y = manager->glfw->getHeight() - y;// coordinate system: convert glfw to opengl
			if (isOnUIPanel(x, y, manager->uiPalette)) {
				setMemberColor(x, y, manager);
			} else if (isOnUIPanel(x, y, manager->uiLength)) {
				setMemberLenght(x, y, manager);
			} else {
				getObjectOnClic(x, y, manager);
			}
		} else {
			std::cout << "not cursorFree ?!" << std::endl;
		}
	}
}

void	printHumanKeyFrame(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window;(void)key;(void)scancode;(void)action;(void)mods;
	if (action == GLFW_PRESS) {
		HumanManager * hmanager = static_cast<HumanManager*>(glfwGetWindowUserPointer(window));//dynamic_cast cannot work, as void* lose polymorphisme metadata
		if (!hmanager) {
			std::cout << "Error: Wrong pointer type, HumanManager * expected" << std::endl;
			return ;
		}
		std::cout << hmanager->human->getKeyFrame();
	}
}

void	printPixelRGB(int glX, int glY, GLenum format, string prefix) {// opengl system!
	GLubyte data[4];//RGBA
	glReadPixels(glX, glY, 1, 1, format, GL_UNSIGNED_BYTE, &data);
	std::cout << prefix << "RGB(" << (unsigned int)data[0] << ", " \
						<< (unsigned int)data[1] << ", " \
						<< (unsigned int)data[2] << ")" << std::endl;
}

void	renderObj3d(list<Obj3d*>	obj3dList, Cam& cam) {
	// cout << "render all Obj3d" << endl;
	//assuming all Obj3d have the same program
	Obj3d*		obj = *(obj3dList.begin());
	Obj3dPG&	pg = obj->getProgram();
	glUseProgram(pg._program);//used once for all obj3d
	Math::Matrix4	proMatrix(cam.getProjectionMatrix());
	Math::Matrix4	viewMatrix = cam.getViewMatrix();
	proMatrix.mult(viewMatrix);// do it in shader ? NO cauz shader will do it for every vertix
	for (Obj3d* object : obj3dList)
		object->render(proMatrix);
	for (Obj3d* object : obj3dList) { // this has to be done after all objects are rendered!
		object->local._matrixChanged = false;
		object->_worldMatrixChanged = false;
	}
}

void	renderSkybox(Skybox& skybox, Cam& cam) {
	// cout << "render Skybox" << endl;
	SkyboxPG&	pg = skybox.getProgram();
	glUseProgram(pg._program);//used once
	
	Math::Matrix4	proMatrix(cam.getProjectionMatrix());
	Math::Matrix4&	viewMatrix = cam.getViewMatrix();
	proMatrix.mult(viewMatrix);

	skybox.render(proMatrix);
}

void sceneHumanGL() {
#ifndef INITS
	int		WINX = 1600;
	int		WINY = 900;
	Glfw		glfw(WINX, WINY); // screen size
	glDisable(GL_CULL_FACE);
	GLint maxAttach = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
	std::cout << "GL_MAX_COLOR_ATTACHMENTS " << maxAttach << std::endl;

	std::string sgl_dir = "SimpleGL/";

	Obj3dPG		obj3d_prog(std::string(sgl_dir + OBJ3D_VS_FILE), std::string(sgl_dir + OBJ3D_FS_FILE));
	SkyboxPG	sky_pg(std::string(sgl_dir + CUBEMAP_VS_FILE), std::string(sgl_dir + CUBEMAP_FS_FILE));

	Obj3dBP::defaultSize = 1.0f;
	Obj3dBP			cubebp(sgl_dir + "obj3d/cube_down.obj", true, false);
	Math::Vector3	dimensions = cubebp.getDimensions();

	Texture *	bmpLength = new Texture("assets/length.bmp");
	// exit(0);
	Texture *	lena = new Texture(sgl_dir + "images/lena.bmp");
	Texture *	palette = new Texture("assets/palette256.bmp");
#endif // INITS
	std::list<Obj3d*>	obj3dList;

	Obj3d	floor(cubebp, obj3d_prog);
	floor.setTexture(lena);
	floor.displayTexture = true;
	float si = 500;
	floor.local.setScale(si, 1, si);
	floor.local.setPos(-si/2, -10, -si/2);
	obj3dList.push_back(&floor);
	
#ifndef HUMAN_CLASS
	/*
		class Human { ... };
		class HumanEvolved : public Human { more members };
		class SuperSaiyan1 : HumanEvolved { with hairs! };
		class SuperSaiyan2 : SuperSaiyan1 { longer hairs!! };
		class SuperSaiyan3 : SuperSaiyan2 { even longer hairs!!! };

	*/
	float		thickness = 1.0f;	// default value
	float		lenght = 4.0f;		// default value

	SuperSaiyan1 *		bob = new SuperSaiyan1(cubebp, obj3d_prog);
	std::cout << bob->_trunk.model.getId() << " trunk id" << std::endl;
	std::cout << bob->_head.model.getId() << " head id" << std::endl;
	// bob->setMembersSize(thickness*3, lenght*8);
	// bob->setTrunkSize(thickness * 5, lenght*5);
	// bob->setHeadSize(thickness * 2);

	// bob->setLenght(lenght*3);
	// bob->setThickness(thickness*3);
	// HumanEvolved		jack = new HumanEvolved(bob);
	// SuperSaiyan1	goku(jack);
	bob->setHairColor(0,155,155);


#endif // HUMAN_CLASS

#ifndef FPSS
	Fps	fps144(144);
	Fps	fps60(60);
	Fps	fps20(20);
	Fps* defaultFps = &fps60;
#endif

#ifndef BEHAVIORS
	#ifndef TRANSFORMBH
		TransformBH		b0_rot;
		b0_rot.transform.rot.setUnit(ROT_DEG);
		b0_rot.transform.rot.y = 80 * defaultFps->getTick();
		b0_rot.modeRot = ADDITIVE;
		// b0_rot.addTarget(&bob->_trunk);
		// b0_rot.addTarget(&bob->_rightArm);


		TransformBH		b1_rot;
		b1_rot.transform.rot.setUnit(ROT_DEG);
		b1_rot.transform.rot.x = 200 * defaultFps->getTick();
		b1_rot.transform.rot.y = 100 * defaultFps->getTick();
		b1_rot.transform.rot.z = 60 * defaultFps->getTick();
		b1_rot.modeRot = ADDITIVE;
		b1_rot.addTarget(&bob->_rightArm);
		b1_rot.addTarget(&bob->_rightForearm);
		b1_rot.addTarget(&bob->_leftThigh);
		b1_rot.addTarget(&bob->_rightCalf);

		// bob->_rightCalf.addBehavior(&b1_rot);

		TransformBH		b2_rot;
		b2_rot.transform.rot.setUnit(ROT_DEG);
		b2_rot.transform.rot.x = 200 * defaultFps->getTick();
		b2_rot.transform.rot.y = 80 * defaultFps->getTick();
		b2_rot.transform.rot.y = 250 * defaultFps->getTick();
		b2_rot.modeRot = ADDITIVE;
		b2_rot.addTarget(&bob->_leftArm);
		b2_rot.addTarget(&bob->_leftForearm);
		b2_rot.addTarget(&bob->_leftCalf);
		b2_rot.addTarget(&bob->_rightThigh);

		if (false) {// the dab !
			bob->_head.rotateMember(Math::Rotation(-30,0,0));
			bob->_trunk.rotateMember(Math::Rotation(0,0,-15));
			bob->_leftArm.rotateMember(Math::Rotation(0,0,-110));
			bob->_rightArm.rotateMember(Math::Rotation(110,40,-30));
			bob->_rightForearm.rotateMember(Math::Rotation(90,0,-20));
			bob->_rightThigh.rotateMember(Math::Rotation(0,0,45));
			bob->_rightCalf.rotateMember(Math::Rotation(0,0,-45));
		} else if (false) {
			float x = 0.0f;
			float z = 20.0f;
			bob->_leftArm.rotateMember(Math::Rotation(x, 0, -z));
			bob->_rightArm.rotateMember(Math::Rotation(x, 0, z));
			bob->_leftThigh.rotateMember(Math::Rotation(x, 0, -z));
			bob->_rightThigh.rotateMember(Math::Rotation(x, 0, z));
		}
	#endif //TRANSFORMBH
	#ifndef ANIMATIONBH
		AnimationHumanBH		running("animations/human_run.anim.json");
		running.loop = -1;
		running.setFpsTick(defaultFps->getTick());
		running.setSpeed(1);
		running.addTarget(bob);

		AnimationHumanBH		jumping("animations/human_jump.anim.json");
		jumping.loop = 0;
		jumping.setFpsTick(defaultFps->getTick());
		jumping.setSpeed(1);
		jumping.addTarget(bob);

		AnimationHumanBH		boston("animations/human_boston.anim.json");
		// boston.loop = -1;
		boston.setFpsTick(defaultFps->getTick());
		boston.setSpeed(1);
		boston.addTarget(bob);

		AnimationHumanBH		dab("animations/human_dab.anim.json");
		// boston.loop = -1;
		dab.setFpsTick(defaultFps->getTick());
		dab.setSpeed(1);
		dab.addTarget(bob);
	#endif // ANIMATIONBH

#endif // BEHAVIORS
	// Behavior::areActive = false;
	// b0_rot.isActive = false;
	// b1_rot.isActive = false;
	// b2_rot.isActive = false;


#ifndef CAM_SKYBOX
	Texture*	texture2 = new Texture("SimpleGL/images/skybox4.bmp");//skybox3.bmp bug?
	Skybox		skybox(*texture2, sky_pg);
	
	Cam		cam(glfw);
	cam.local.setPos(0, 0, 10);
	cam.setFov(120);
	cam.lockedMovement = false;
	cam.lockedOrientation = false;
	cam.speed /= 4;

	glfw.setMouseAngle(-1);
#endif

#define TESTCUBE
#ifdef TESTCUBE

	#ifdef TESTSC
		std::cout << "fuck --------------------\n";

		Math::Rotation	rot(1,1,0);
		float c = 0.1f;

		Obj3d	origine(cubebp, obj3d_prog);
		origine.setColor(0,0,0);
		origine.local.setScale(-c,-c,-c);
		std::cout << "fuck " << origine.getId() << std::endl;

		Obj3d	cube(cubebp, obj3d_prog);
		cube.setColor(255,70,0);
		cube.setTexture(lena);
		cube.displayTexture = true;
		std::cout << "fuck " << cube.getId() << std::endl;

		Math::Vector3	s(cube.local.getScale());
		Obj3d	pivot(cubebp, obj3d_prog);
		pivot.setColor(255,0,255);
		pivot.local.setPos(s.x/2, s.y/2, s.z/2);
		pivot.local.setScale(c,c,c);
		pivot.local.setPos(5,5,5);
		std::cout << "fuck " << pivot.getId() << std::endl;

		Obj3d	cubedir(cubebp, obj3d_prog);
		cubedir.local.setScale(c,c,c);
		cubedir.setColor(0,255,0);
		Math::Vector3 pc = pivot.local.getPos();
		pc.add(rot.x, rot.y, rot.z);
		cubedir.local.setPos(pc);
		std::cout << "fuck " << cubedir.getId() << std::endl;

		cube.setParent(&pivot);
		cube.local.setPos(2,2,2);

		Math::Vector3	offset(cube.local.getPos());
		offset.sub(pivot.local.getPos());
		float	mag = abs(offset.magnitude());
		
		// obj3dList.push_back(&origine);	// 96
		// obj3dList.push_back(&pivot);	// 98
		// obj3dList.push_back(&cube);		// 97
		// obj3dList.push_back(&cubedir);	// 99
		std::cout << "fuck --------------------\n";
	#endif //TESTSC
	
	// #define MANYCUBES
	#ifdef MANYCUBES
		TransformBH		b5;
		b5.transform.rot.setUnit(ROT_DEG);
		b5.transform.rot.x = 200 * defaultFps->getTick();
		b5.transform.rot.y = 100 * defaultFps->getTick();
		b5.transform.rot.z = 60 * defaultFps->getTick();
		b5.modeRot = ADDITIVE;
		unsigned int max = 100;
		if (1) {
			for (size_t i = 0; i < max; i++) {
				for (size_t j = 0; j < max; j++) {
					Obj3d * 	o = new Obj3d(cubebp, obj3d_prog);
					std::cout << o->getId() << std::endl;
					unsigned int	colr = ( ((i * 255) / max) ) % 255;
					unsigned int	colg = ( ((j * 255) / max) ) % 255;
					unsigned int	colb = (i + j) % 255;
					o->setColor(colr, colg, 255 - colb);
					float	ii = ((float)i);
					float	jj = ((float)j);
					o->local.translate(ii,0,jj);
					// o->local.setScale(float(colr)/255.0f*5,float(colg)/255.0f*5,float(colb)/255.0f*5);
					
					b5.addTarget(o);
					obj3dList.push_back(o);
				}
			}
		}
		cam.speed *= 10;	
		#endif


#endif

	obj3dList.merge(bob->getObjList());
	// obj3dList = bob->getObjList();
	list<Obj3d*>	raycastList = bob->getObjList();

// #define FRAMEBUFFER
#ifndef FRAMEBUFFER
	FrameBuffer	framebuffer(WINX, WINY);
	FrameBuffer	framebufferUI(WINX, WINY);
	FrameBuffer	framebufferUI2(WINX, WINY);
	if (framebuffer.fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "FrameBuffer() failed : " << FrameBuffer::getFramebufferStatusInfos(framebuffer.fboStatus) << std::endl;
		return ;
	}
	if (framebufferUI.fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "FrameBuffer() failed : " << FrameBuffer::getFramebufferStatusInfos(framebuffer.fboStatus) << std::endl;
		return ;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferUI.fbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, palette->getId(), 0);// mipmap level: 0(base)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferUI2.fbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bmpLength->getId(), 0);// mipmap level: 0(base)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

#endif

#ifndef GAMEMANAGER
	int pad = 10;
	HumanManager	gameManager;
	gameManager.glfw = &glfw;
	gameManager.human = bob;
	gameManager.framebuffer = &framebuffer;
	gameManager.framebufferUI = &framebufferUI;
	gameManager.framebufferUI2 = &framebufferUI2;
	gameManager.cam = &cam;
	gameManager.obj3dList = &raycastList;

	UIPanel	uiSelect;
	uiSelect.posX = 0;
	uiSelect.posY = 0;
	uiSelect.width = WINX / 4;
	uiSelect.height = WINY / 4;
	gameManager.uiSelect = &uiSelect;
	// (0, 0, WINX, WINY, 0, 0, WINX/4, WINY/4,

	UIPanel	uiPalette(palette);
	uiPalette.posX = WINX - palette->getWidth();
	// uiPalette.posY = 0;
	// uiPalette.width = WINX / 4;
	// uiPalette.height = WINY / 4;
	gameManager.uiPalette = &uiPalette;

	UIPanel	uiLength(bmpLength);
	uiLength.posX = WINX - bmpLength->getWidth();
	uiLength.posY = uiPalette.height + pad;
	gameManager.uiLength = &uiLength;

	glfw.activateDefaultCallbacks(&gameManager);
	glfwSetMouseButtonCallback(glfw._window, HumanMouseButtonCallback);//override default callback
	glfw.func[GLFW_KEY_ENTER] = printHumanKeyFrame;
	// glfwSetWindowMonitor(glfw._window, NULL, 100, 100, WINX, WINY, 0);

#endif

#ifndef RENDER
	// glfwSetKeyCallback(glfw._window, key_callback);

	float ti = defaultFps->getTick();
	double t = glfwGetTime();
	// glfwSwapInterval(1);

	if (gameManager.currentSelection)
		std::cout << "Current selection not null: " << gameManager.currentSelection << std::endl;
	else
		std::cout << "Current selection null" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(glfw._window)) {
	// std::cout << ".";
		if (defaultFps->wait_for_next_frame()) {
			// Fps::printGlobalFps();

			// b0_rot.run();
			// b1_rot.run();
			// b2_rot.run();
			// running.run();
			// jumping.run();
			boston.run();
			// dab.run();
			#ifdef MANYCUBES
			// b5.run();
			#endif
			
			#ifdef TESTCUBE
				#ifdef TESTSC
					if (0) {
						std::cout << "tik: " << ti << std::endl;
						float r = 40 * ti;
						offset = cube.local.getPos();
						offset.sub(pivot.local.getPos());
						float m = abs(offset.magnitude());
						std::cout << "mag: " << mag << std::endl;
						std::cout << "m: " << m << std::endl;
						std::cout <<"\n";
						pivot.local.rotate(rot);
						// cube.local.rotateAround2(pivot.local.getPos(), rot, r);
						// cube.local.rotateAround(center.local.getPos(), Math::Rotation(r,r,0));
					}
				#endif //TESTSC
			#endif //TESTCUBE

			// std::cout << "anim: "; running._rotaMap["pos"].printData();
			// std::cout << "bob: "; bob->_trunk.local.getPos().printData();

			if (gameManager.currentSelection && GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_P)) {
				gameManager.currentSelection->local.enlarge(0, 1 * defaultFps->getTick(), 0);
				HumanEvolved * he = dynamic_cast<HumanEvolved*>(gameManager.human);
				if (he) {
					he->_leftHand.position(he->getThickness());
					he->_rightHand.position(he->getThickness());
					he->_leftFoot.position(he->getThickness());
					he->_rightFoot.position(he->getThickness());
				}
				
				gameManager.human->positionMembers();
				gameManager.human->updateMembersAnchor();
			}
			if (gameManager.currentSelection) {//manual rotations
				float v = 30.0f * defaultFps->getTick();
				Math::Rotation rx(v, 0, 0);
				Math::Rotation ry(0, v, 0);
				Math::Rotation rz(0, 0, v);
				Math::Rotation rxm(-v, 0, 0);
				Math::Rotation rym(0, -v, 0);
				Math::Rotation rzm(0, 0, -v);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_BACKSPACE))
					gameManager.currentSelection->getParent()->local.setRot(0,0,0);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_KP_4))
					gameManager.currentSelection->getParent()->local.rotate(rx);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_KP_1))
					gameManager.currentSelection->getParent()->local.rotate(rxm);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_KP_5))
					gameManager.currentSelection->getParent()->local.rotate(ry);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_KP_2))
					gameManager.currentSelection->getParent()->local.rotate(rym);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_KP_6))
					gameManager.currentSelection->getParent()->local.rotate(rz);
				if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_KP_3))
					gameManager.currentSelection->getParent()->local.rotate(rzm);
			}

			glfwPollEvents();
			glfw.updateMouse(); // to do before cam's events
			cam.events(glfw, float(defaultFps->getTick()));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderObj3d(obj3dList, cam);
			renderSkybox(skybox, cam);

		#ifndef FRAMEBUFFER_UI
			blitToWindow(&gameManager, gameManager.framebuffer, GL_COLOR_ATTACHMENT0, gameManager.uiSelect);
			blitToWindow(&gameManager, gameManager.framebufferUI, GL_COLOR_ATTACHMENT0, gameManager.uiPalette);
			blitToWindow(&gameManager, gameManager.framebufferUI2, GL_COLOR_ATTACHMENT0, gameManager.uiLength);
		#endif //FRAMEBUFFER_UI

			glfwSwapBuffers(glfw._window);
			if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_ESCAPE))
				glfwSetWindowShouldClose(glfw._window, GLFW_TRUE);
		}
	}
#endif // RENDER
}

// #define OFFSETCLASS
#ifdef OFFSETCLASS
	void	offsetClass() {
		//remove CFLAGS before compiling
		std::cout << offsetof(GameManager, glfw) << std::endl;
		std::cout << offsetof(GameManager, currentSelection) << std::endl;
		std::cout << offsetof(GameManager, objectList) << std::endl;
		std::cout << offsetof(GameManager, cam) << std::endl;

		std::cout << offsetof(HumanManager, glfw) << std::endl;
		std::cout << offsetof(HumanManager, currentSelection) << std::endl;
		std::cout << offsetof(HumanManager, objectList) << std::endl;
		std::cout << offsetof(HumanManager, cam) << std::endl;
		std::cout << offsetof(HumanManager, obj3dList) << std::endl;
		std::cout << offsetof(HumanManager, human) << std::endl;
		std::cout << offsetof(HumanManager, fb) << std::endl;

		exit(0);
	}
#endif

void	testJson() {

	if (1) {
		std::string filecontent = Misc::getFileContent("animations/human_run.anim");
		std::stringstream ss;
		ss << filecontent;
		json j;
		ss >> j;
		std::cout << j << std::endl;
		std::cout << "size: " << j.size() << std::endl;
		std::cout << j["loop"] << std::endl;
	}

	if (0) {
		std::string s1 = R"(
			{"frame1":{
				"head":[0.0,0.0,0.0],
				"leftArm":[49.5,0.0,0.0],
				"leftCalf":[60.0,0.0,0.0],
				"leftForearm":[-78.5,0.0,0.0],
				"leftThigh":[-35.5,0.0,0.0],
				"rightArm":[-5.5,0.0,0.0],
				"rightCalf":[42.0,0.0,0.0],
				"rightForearm":[-83.0,0.0,0.0],
				"rightThigh":[26.0,0.0,0.0],
				"time":0.5,
				"trunk":[0.0,0.0,0.0]
			},
			"frame2":{
				"head":[0.0,0.0,0.0],
				"leftArm":[-5.5,0.0,0.0],
				"leftCalf":[42.0,0.0,0.0],
				"leftForearm":[-83.0,0.0,0.0],
				"leftThigh":[26.0,0.0,0.0],
				"rightArm":[49.5,0.0,0.0],
				"rightCalf":[60.0,0.0,0.0],
				"rightForearm":[-78.5,0.0,0.0],
				"rightThigh":[-35.5,0.0,0.0],
				"time":0.5,
				"trunk":[0.0,0.0,0.0]
			},
			"loop":-1}
		)";

		std::stringstream ss;
		json res;
		ss << s1;
		ss >> res;
		std::cout << res << std::endl;
		std::cout << "s: " << res.size() << std::endl << std::endl;
	}
	
	if (0) {
		json human =
		{
			{"frame1",{
				{"member1", {1,2,3}},
				{"member2", {1,2,3}},
				{"member3", {1,2,3}},
				{"member4", {1,2,3}},
				{"member5", {1,2,3}},
				{"time", 0.5}
			}},
			{"frame2",{
				{"member1", {1,2,3}},
				{"member2", {1,2,3}},
				{"member3", {1,2,3}},
				{"member4", {1,2,3}},
				{"member5", {1,2,3}},
				{"time", 0.5},
			}}
		};

		std::cout << "hs:" << human.size() << std::endl;
		std::cout << human["frame1"]["member1"][0] << std::endl;
		std::cout << human["frame1"]["member1"][1] << std::endl;
		std::cout << human["frame1"]["member1"][2] << std::endl;

		std::cout << human["frame2"]["member3"][0] << std::endl;
		std::cout << human["frame2"]["member3"][1] << std::endl;
		std::cout << human["frame2"]["member3"][2] << std::endl;

		std::stringstream ss;
		ss << human;
		std::string str = ss.str();

		json human2;
		ss >> human2;

		std::cout << human2;
	}

	exit(0);
}

int		main(void) {
	// testJson();
	std::cout << "____START____" << endl;
	sceneHumanGL();
	return (EXIT_SUCCESS);
}
