/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/12/02 15:49:23 by rhoffsch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "humangl.h"
#include "human.hpp"
#include "humanevolved.hpp"
#include "supersaiyan1.hpp"
#include "framebuffer.hpp"
#include "animationbh.hpp"
#include "humanmanager.hpp"
#include "uipanel.hpp"

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

void blitToWindow(HumanManager * manager, FrameBuffer * readFramebuffer, GLenum attachmentPoint, UIPanel *panel) {
	GLuint fbo;
	if (readFramebuffer) {
		fbo = readFramebuffer->fbo;
	} else {
		fbo = panel->getFbo();
	}
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	(void)manager;
	//glViewport(0, 0, manager->glfw->getWidth(), manager->glfw->getHeight());//size of the window/image or panel width ?
	glReadBuffer(attachmentPoint);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	int w;
	int h;
	if (readFramebuffer) {
		w = readFramebuffer->getWidth();
		h = readFramebuffer->getHeight();
	} else if (panel->getTexture()) {
		w = panel->getTexture()->getWidth();
		h = panel->getTexture()->getHeight();
	} else {
		std::cout << "FUCK " << __PRETTY_FUNCTION__ << std::endl;
		exit(2);
	}
	if (0) {
		std::cout << "copy " << w << "x" << h << "\tresized\t" << panel->_width << "x" << panel->_height \
			<< "\tat pos\t" << panel->_posX << ":" << panel->_posY << std::endl;
			// << " -> " << (panel->posX + panel->width) << "x" << (panel->posY + panel->height) << std::endl;
	}
	glBlitFramebuffer(0, 0, w, h, \
		panel->_posX, panel->_posY, panel->_posX2, panel->_posY2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

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
			std::cout << "opengl pos: " << glX << " - " << glY << std::endl;
			std::cout << "id is " << id << " (" << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2] << ")" << std::endl;
			manager->currentSelection = it;
		}
	}
	//restore window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
static bool	UIPanelActions(int glX, int glY, HumanManager * manager, GLenum action) {
	bool	isOnAPanel = true;
	if (action == GLFW_PRESS && manager->uiPalette->isOnPanel(glX, glY)) {
		manager->uiPalette->action(glX, glY, manager);
	} else if (action == GLFW_PRESS && manager->uiAnimButtons->isOnPanel(glX, glY)) {
		manager->uiAnimButtons->action(glX, glY, manager);
	} else if ((action == GLFW_REPEAT || action == GLFW_PRESS) && manager->uiLength->isOnPanel(glX, glY)) {
		manager->uiLength->action(glX, glY, manager);
	} else if ((action == GLFW_REPEAT || action == GLFW_PRESS) && manager->uiGlobal->isOnPanel(glX, glY)) {
		manager->uiGlobal->action(glX, glY, manager);
	} else if ((action == GLFW_REPEAT || action == GLFW_PRESS) && manager->uiThickness->isOnPanel(glX, glY)) {
		manager->uiThickness->action(glX, glY, manager);
	} else if ((action == GLFW_REPEAT || action == GLFW_PRESS) && manager->uiFakeRaycast->isOnPanel(glX, glY)) {
		manager->uiFakeRaycast->action(glX, glY, manager);
	} else {
		isOnAPanel = false;
	}
	std::cout << "isOneAPanel" << (isOnAPanel ? "true" : "false") << std::endl;
	return isOnAPanel;
}
void	HumanMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	(void)window;(void)button;(void)action;(void)mods;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	
	HumanManager * manager = static_cast<HumanManager*>(glfwGetWindowUserPointer(window));//dynamic_cast cannot work, as void* lose polymorphisme metadata
	if (!manager) {
		std::cout << "Error: Wrong pointer type, HumanManager * expected" << std::endl;
		return ;
	}
	if (action == GLFW_PRESS) {
		manager->lmbPressed = true;

		if (manager->glfw->cursorFree == true) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			std::cout << x << " : " << y;
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				std::cout << "\tright button" << std::endl;
			else if (button == GLFW_MOUSE_BUTTON_LEFT)
				std::cout << "\tleft button" << std::endl;

			y = manager->glfw->getHeight() - y;// coordinate system: convert glfw to opengl
			if (UIPanelActions(x, y, manager, action) == false) {//meaning cursor wasnt on any panel (and nothing has been done)
				getObjectOnClic(x, y, manager);
			}
		} else {//impossible case
			std::cout << "not cursorFree ?!" << std::endl;
		}
	} else if (action == GLFW_RELEASE) {
		manager->lmbPressed = false;
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

Math::Vector3		getPixelRGB(int glX, int glY, GLenum format) {// opengl system!
	GLubyte data[4];//RGBA
	glReadPixels(glX, glY, 1, 1, format, GL_UNSIGNED_BYTE, &data);
	/*
		std::cout << "RGB(" << (unsigned int)data[0] << ", " \
						<< (unsigned int)data[1] << ", " \
						<< (unsigned int)data[2] << ")" << std::endl;
	*/
	return Math::Vector3(data[0], data[1], data[2]);
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
	Texture *	bmpGlobal = new Texture("assets/global.bmp");
	Texture *	bmpThickness = new Texture("assets/thickness.bmp");
	Texture *	bmpAnim = new Texture("assets/animations.bmp");
	Texture *	lena = new Texture(sgl_dir + "images/lena.bmp");
	Texture *	palette = new Texture("assets/palette256.bmp");
#endif // INITS
	std::list<Obj3d*>	obj3dList;

	Obj3d	floor(cubebp, obj3d_prog);
	floor.setTexture(lena);
	floor.displayTexture = true;
	float si = 500;
	floor.local.setScale(si, 1, si);
	floor.local.setPos(-si/2, FLOOR, -si/2);
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

	bob->setHairColor(0,155,155);
#endif // HUMAN_CLASS
	obj3dList.merge(bob->getObjList());
	list<Obj3d*>	raycastList = bob->getObjList();
	
#ifndef FPSS
	Fps	fps60(60);
	Fps* defaultFps = &fps60;
#endif

#ifndef BEHAVIORS
	#ifndef ANIMATIONBH
		AnimationHumanBH		standing("animations/human_stand.anim.json");
		standing.loop = -1;
		standing.setFpsTick(defaultFps->getTick());
		standing.setSpeed(1);
		standing.addTarget(bob);

		AnimationHumanBH		running("animations/human_run.anim.json");
		running.loop = -1;
		running.setFpsTick(defaultFps->getTick());
		running.setSpeed(1);
		running.addTarget(bob);

		AnimationHumanBH		jumping("animations/human_jump.anim.json");
		// jumping.loop = 0;
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


#ifndef CAM_SKYBOX
	Texture*	texture2 = new Texture("SimpleGL/images/skybox4.bmp");//skybox3.bmp bug?
	Skybox		skybox(*texture2, sky_pg);
	
	Cam		cam(glfw);
	cam.local.setPos(0, 0, 10);
	cam.setFov(120);
	cam.lockedMovement = false;
	cam.lockedOrientation = false;
#endif

#ifndef FRAMEBUFFER
	FrameBuffer		framebuffer(WINX, WINY);
	if (framebuffer.fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "FrameBuffer() failed : " << FrameBuffer::getFramebufferStatusInfos(framebuffer.fboStatus) << std::endl;
		return ;
	}
#endif

#ifndef UI_PANELS
	int pad = 10;

	UIFakeRaycast	uiFakeRaycast(nullptr);
	uiFakeRaycast.setSize(WINX / 4, WINY / 4);

	UIMemberColor	uiPalette(palette);
	uiPalette.setPos(WINX - palette->getWidth(), 0);
	uiPalette.setSize(uiPalette.getTexture()->getWidth(), uiPalette.getTexture()->getHeight());

	UIMemberLength	uiLength(bmpLength);
	uiLength.setPos(WINX - bmpLength->getWidth(), uiPalette.getHeight() + pad);
	uiLength.setSize(uiLength.getTexture()->getWidth(), uiLength.getTexture()->getHeight());

	UIThickness		uiThickness(bmpThickness);
	uiThickness.setPos(WINX - bmpThickness->getWidth(), uiPalette.getHeight() + pad + uiLength.getHeight() + pad);
	uiThickness.setSize(uiThickness.getTexture()->getWidth(), uiThickness.getTexture()->getHeight());

	UIGlobalLength	uiGlobal(bmpGlobal);
	uiGlobal.setPos(WINX - bmpGlobal->getWidth(), uiPalette.getHeight() + pad + uiLength.getHeight() + pad + uiThickness.getHeight() + pad);
	uiGlobal.setSize(uiGlobal.getTexture()->getWidth(), uiGlobal.getTexture()->getHeight());
	
	UIAnimation		uiAnimButtons(bmpAnim);
	uiAnimButtons.setPos((WINX / 2) - (bmpAnim->getWidth() / 2), WINY - bmpAnim->getHeight());
	uiAnimButtons.setSize(uiAnimButtons.getTexture()->getWidth(), uiAnimButtons.getTexture()->getHeight());

#endif // UI_PANELS

#ifndef GAMEMANAGER
	HumanManager	gameManager;
	gameManager.glfw = &glfw;
	gameManager.cam = &cam;
	gameManager.defaultFps = &fps60;

	gameManager.human = bob;
	gameManager.framebuffer = &framebuffer;
	gameManager.obj3dList = &raycastList;

	gameManager.uiFakeRaycast = &uiFakeRaycast;
	gameManager.uiPalette = &uiPalette;
	gameManager.uiLength = &uiLength;
	gameManager.uiAnimButtons = &uiAnimButtons;
	gameManager.uiThickness = &uiThickness;
	gameManager.uiGlobal = &uiGlobal;

	gameManager.animations[0] = nullptr;
	gameManager.animations[1] = &standing;
	gameManager.animations[2] = &running;
	gameManager.animations[3] = &jumping;
	gameManager.animations[4] = &boston;
	gameManager.animations[5] = &dab;
	gameManager.currentAnimation = gameManager.animations[0];
	gameManager.animationPaused = false;

	glfw.activateDefaultCallbacks(&gameManager);
	glfwSetMouseButtonCallback(glfw._window, HumanMouseButtonCallback);//override mouse button default callback
	glfw.func[GLFW_KEY_ENTER] = printHumanKeyFrame;

#endif

#ifndef RENDER

	float ti = defaultFps->getTick();
	double t = glfwGetTime();
	glfwSwapInterval(1);

	if (gameManager.currentSelection)
		std::cout << "Current selection not null: " << gameManager.currentSelection << std::endl;
	else
		std::cout << "Current selection null" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


	while (!glfwWindowShouldClose(glfw._window)) {
		if (defaultFps->wait_for_next_frame()) {
			// Fps::printGlobalFps();

			if (gameManager.currentAnimation && !gameManager.animationPaused)
				gameManager.currentAnimation->run();
			
			// std::cout << "anim: "; running._rotaMap["pos"].printData();
			// std::cout << "bob: "; bob->_trunk.local.getPos().printData();

			//manual rotations and some mouse events
			if (gameManager.currentSelection) {
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
			// mouse buttons repeat on panels
			if (gameManager.lmbPressed) {
				double x, y;
				glfwGetCursorPos(gameManager.glfw->_window, &x, &y);
				y = gameManager.glfw->getHeight() - y;// coordinate system: convert glfw to opengl
				UIPanelActions(x, y, &gameManager, GLFW_REPEAT);
			}

			glfwPollEvents();
			glfw.updateMouse(); // to do before cam's events
			cam.events(glfw, float(defaultFps->getTick()));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderObj3d(obj3dList, cam);
			renderSkybox(skybox, cam);

		#ifndef FRAMEBUFFER_UI
			blitToWindow(&gameManager, gameManager.framebuffer, GL_COLOR_ATTACHMENT0, gameManager.uiFakeRaycast);
			blitToWindow(&gameManager, nullptr, GL_COLOR_ATTACHMENT0, gameManager.uiPalette);
			blitToWindow(&gameManager, nullptr, GL_COLOR_ATTACHMENT0, gameManager.uiLength);
			blitToWindow(&gameManager, nullptr, GL_COLOR_ATTACHMENT0, gameManager.uiAnimButtons);
			blitToWindow(&gameManager, nullptr, GL_COLOR_ATTACHMENT0, gameManager.uiGlobal);
			blitToWindow(&gameManager, nullptr, GL_COLOR_ATTACHMENT0, gameManager.uiThickness);


		#endif //FRAMEBUFFER_UI

			glfwSwapBuffers(glfw._window);
			if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_ESCAPE))
				glfwSetWindowShouldClose(glfw._window, GLFW_TRUE);
		}
	}
#endif // RENDER
}

int		main(void) {
	std::cout << "____START____" << endl;
	sceneHumanGL();
	return (EXIT_SUCCESS);
}
