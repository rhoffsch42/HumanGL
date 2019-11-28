/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/11/28 04:16:35 by rhoffsch         ###   ########.fr       */
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
	bool	isOnPanel(int glX, int glY) {
		// this can be computed only once at constructor, at setPos(), at setWidth(), at setHeight()
		int x0 = this->posX;
		int y0 = this->posY;
		int x1 = this->posX + this->width;
		int y1 = this->posY + this->height;

		return (glX >= x0 && glX <= x1 && glY >= y0 && glY <= y1);
	}
	Texture *	texture;
	int			posX;
	int			posY;
	int			width;
	int 		height;
private:
};

#define MAX_ANIM 5 // 4 animations + 1 for nullptr (pause)
class HumanManager : public GameManager {
public:
	HumanManager() : GameManager() {
		this->defaultFps = nullptr;
		this->lmbPressed = false;
		this->obj3dList = nullptr;

		this->human = nullptr;
		this->framebuffer = nullptr;
		this->framebufferUI = nullptr;
		this->framebufferUI2 = nullptr;
		this->uiPalette = nullptr;
		this->uiLength = nullptr;
		this->uiSelect = nullptr;
		this->currentAnimation = nullptr;
		this->animationPaused = false;
	}
	~HumanManager() {
	}
	Fps *			defaultFps;
	bool			lmbPressed;
	list<Obj3d*> *	obj3dList;
	//above are for GameManager

	Human *			human;
	FrameBuffer *	framebuffer;
	FrameBuffer *	framebufferUI;
	FrameBuffer *	framebufferUI2;
	FrameBuffer *	framebufferUI3;
	UIPanel *		uiPalette;
	UIPanel *		uiLength;
	UIPanel *		uiSelect;
	UIPanel *		uiAnimButtons;
	AnimationHumanBH *	animations[MAX_ANIM];
	AnimationHumanBH *	currentAnimation;
	bool				animationPaused;
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
			std::cout << "opengl pos: " << glX << " - " << glY << std::endl;
			std::cout << "id is " << id << " (" << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2] << ")" << std::endl;
			manager->currentSelection = it;
		}
	}
	//restore window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void	setMemberColor(int glX, int glY, HumanManager * manager) {// opengl system!
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	if (manager->currentSelection) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLubyte data[4];//RGBA
		glReadPixels(glX, glY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);
		Obj3d *		o = dynamic_cast<Obj3d*>(manager->currentSelection);
		if (!o) { std::cout << __PRETTY_FUNCTION__ << " : Error : dymanic_cast failed" << std::endl; exit(2); }
		std::cout << "color: " << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << std::endl;
		o->setColor(data[0], data[1], data[2]);
	}
}
static void	selectAnimation(int glX, int glY, HumanManager * manager) {// opengl system!
	(void)glY;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	UIPanel *	panel = manager->uiAnimButtons;
	int index = (glX - panel->posX) / (panel->width / MAX_ANIM);
	if (index) {
		manager->animationPaused = false;
		Math::Vector3	pos = manager->human->_trunk.local.getPos();
		pos.y = 0;
		manager->human->_trunk.local.setPos(pos);
		manager->currentAnimation = manager->animations[index];
		manager->currentAnimation->reset();
	} else {
		manager->animationPaused = !manager->animationPaused;
	}
}

#define HUMAN			0
#define HUMAN_EVOLVED	1
#define SUPERSAIYAN1	2
static void	setMemberLenght(int glX, int glY, HumanManager * manager) {// opengl system!
	(void)glY;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	float speed = 3.0f;
	if (glX - manager->uiLength->posX <= manager->uiLength->width / 2) { // reduce length
		if (manager->currentSelection->local.getScale().y < 1.0f) // minimum length
			return ;
		speed *= -1.0f;
	}
	manager->currentSelection->local.enlarge(0, speed * manager->defaultFps->getTick(), 0);
	
	//update members
	int i = HUMAN;
	HumanEvolved * he = dynamic_cast<HumanEvolved*>(manager->human);
	HumanEvolved * ss1 = dynamic_cast<SuperSaiyan1*>(manager->human);
	if (ss1)
		i = SUPERSAIYAN1;
	else if (he)
		i = HUMAN_EVOLVED;
	float t = manager->human->getThickness();
	switch (i) {
		case SUPERSAIYAN1:
			// nothing to do, as it's only the hairs
		case HUMAN_EVOLVED:
			he->_leftHand.position(t);
			he->_rightHand.position(t);
			he->_leftFoot.position(t);
			he->_rightFoot.position(t);
		case HUMAN:
			manager->human->positionMembers();
			manager->human->updateMembersAnchor();
	}		
}

static bool	UIPanelActions(int glX, int glY, HumanManager * manager) {
	bool	isOnAPanel = true;
	if (manager->uiPalette->isOnPanel(glX, glY)) {
		setMemberColor(glX, glY, manager);// transform it: panel->action(x, y, manager); // void	UIPanel::action(...) = 0; or func ptr
	} else if (manager->uiAnimButtons->isOnPanel(glX, glY)) {
		selectAnimation(glX, glY, manager);
	} else if (manager->uiLength->isOnPanel(glX, glY)) {
		// we do nothing here
	} else if (manager->uiSelect->isOnPanel(glX, glY)) {
		// we do nothing here
	} else {
		isOnAPanel = false;
	}
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
			if (UIPanelActions(x, y, manager) == false) {//meaning cursor wasnt on any panel (and nothing has been done)
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
	Texture *	lena = new Texture(sgl_dir + "images/lena.bmp");
	Texture *	palette = new Texture("assets/palette256.bmp");
	Texture *	bmpAnim = new Texture("assets/animations.bmp");
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
	FrameBuffer		framebufferUI(WINX, WINY);
	FrameBuffer		framebufferUI2(WINX, WINY);
	FrameBuffer		framebufferUI3(WINX, WINY);
	FrameBuffer*	fbs[4] = {&framebuffer, &framebufferUI, &framebufferUI2, &framebufferUI3};
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferUI.fbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, palette->getId(), 0);// mipmap level: 0(base)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferUI2.fbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bmpLength->getId(), 0);// mipmap level: 0(base)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferUI3.fbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bmpAnim->getId(), 0);// mipmap level: 0(base)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	for (size_t i = 0; i < 4; i++) {
		if (framebuffer.fboStatus != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "FrameBuffer() failed : " << FrameBuffer::getFramebufferStatusInfos(fbs[i]->fboStatus) << std::endl;
			return ;
		}
	}
#endif

#ifndef UI_PANELS
	int pad = 10;
	UIPanel	uiSelect;
	uiSelect.posX = 0;
	uiSelect.posY = 0;
	uiSelect.width = WINX / 4;
	uiSelect.height = WINY / 4;

	UIPanel	uiPalette(palette);
	uiPalette.posX = WINX - palette->getWidth();

	UIPanel	uiLength(bmpLength);
	uiLength.posX = WINX - bmpLength->getWidth();
	uiLength.posY = uiPalette.height + pad;

	UIPanel	uiAnimButtons(bmpAnim);
	uiAnimButtons.posX = (WINX / 2) - (bmpAnim->getWidth() / 2);
	uiAnimButtons.posY = WINY - bmpAnim->getHeight();
#endif // UI_PANELS

#ifndef GAMEMANAGER
	HumanManager	gameManager;
	gameManager.glfw = &glfw;
	gameManager.cam = &cam;
	gameManager.defaultFps = &fps60;
	
	gameManager.human = bob;
	gameManager.framebuffer = &framebuffer;
	gameManager.framebufferUI = &framebufferUI;
	gameManager.framebufferUI2 = &framebufferUI2;
	gameManager.framebufferUI3 = &framebufferUI3;
	gameManager.obj3dList = &raycastList;
	
	gameManager.uiSelect = &uiSelect;
	gameManager.uiPalette = &uiPalette;
	gameManager.uiLength = &uiLength;
	gameManager.uiAnimButtons = &uiAnimButtons;

	gameManager.animations[0] = nullptr;
	gameManager.animations[1] = &standing;
	gameManager.animations[2] = &running;
	gameManager.animations[3] = &jumping;
	gameManager.animations[4] = &boston;
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
	// std::cout << ".";
		if (defaultFps->wait_for_next_frame()) {
			// Fps::printGlobalFps();

			if (gameManager.currentAnimation && !gameManager.animationPaused)
				gameManager.currentAnimation->run();
			
			// std::cout << "anim: "; running._rotaMap["pos"].printData();
			// std::cout << "bob: "; bob->_trunk.local.getPos().printData();

			if (gameManager.currentSelection) {//manual rotations and some mouse events
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

				// mouse buttons repeat on panels
				if (gameManager.lmbPressed) {
					double x, y;
					glfwGetCursorPos(gameManager.glfw->_window, &x, &y);
					y = gameManager.glfw->getHeight() - y;// coordinate system: convert glfw to opengl
					if (gameManager.uiLength->isOnPanel(x, y)) {
						setMemberLenght(x, y, &gameManager);
					}
				}
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
			blitToWindow(&gameManager, gameManager.framebufferUI3, GL_COLOR_ATTACHMENT0, gameManager.uiAnimButtons);
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

int		main(void) {
	std::cout << "____START____" << endl;
	sceneHumanGL();
	return (EXIT_SUCCESS);
}
