/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/11/15 18:33:14 by rhoffsch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "humangl.h"
#include "human.hpp"
#include "humanevolved.hpp"
#include "supersaiyan1.hpp"
#include "framebuffer.hpp"

#include <string>
#include <cstdio>
#include <vector>
#include <list>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

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
private:
};

void	getObjectOnClic(int x, int y, HumanManager * manager, bool resetMatrixChecks = false) {
	if (!manager->human || !manager->glfw || !manager->cam || !manager->framebuffer) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : one of several HumanManager pointer is null." << std::endl;
		return ;
	}
	// list<Obj3d*>	obj3dList = manager->human->getObjList();//should return full list depending on Human type
	list<Obj3d*> 	obj3dList = *(manager->obj3dList);
	y = manager->glfw->getHeight() - y;

	GLubyte data[4];//RGBA

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
	glReadPixels(x,	y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);
	unsigned int id = 0;
	Misc::RGBToInt(&(*data), &id);
	for (auto it : obj3dList) {
		if (it->getId() == id) {
			std::cout << "pos: " << x << " - " << y << std::endl;
			std::cout << "id is " << id << " (" << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2] << ")" << std::endl;
			std::cout << "searching from " << obj3dList.size() << " obj3d" << std::endl;
			std::cout << "Found!" << std::endl;
			manager->currentSelection = it;
		}
	}
	//restore window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void	HumanMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	(void)window;(void)button;(void)action;(void)mods;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	if (action == GLFW_PRESS) {
		HumanManager * hmanager = static_cast<HumanManager*>(glfwGetWindowUserPointer(window));//dynamic_cast cannot work, as void* lose polymorphisme metadata
		if (!hmanager) {
			std::cout << "Error: Wrong pointer type, HumanManager * expected" << std::endl;
			return ;
		}
		if (hmanager->glfw->cursorFree == true) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			std::cout << x << " : " << y;
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				std::cout << "\tright button" << std::endl;
			else if (button == GLFW_MOUSE_BUTTON_LEFT)
				std::cout << "\tleft button" << std::endl;
			getObjectOnClic(x, y, hmanager);
		} else {
			std::cout << "not cursorFree ?!" << std::endl;
		}
	}
}


void	printPixelRGB(int x, int y, GLenum format, string prefix) {
	GLubyte data[4];//RGBA
	glReadPixels(x,	y, 1, 1, format, GL_UNSIGNED_BYTE, &data);
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
	int		WINX = 1600;
	int		WINY = 900;
	std::string sgl_dir = "SimpleGL/";
	Glfw		glfw(WINX, WINY); // screen size
	glDisable(GL_CULL_FACE);

	Obj3dPG		obj3d_prog(std::string(sgl_dir + OBJ3D_VS_FILE), std::string(sgl_dir + OBJ3D_FS_FILE));
	SkyboxPG	sky_pg(std::string(sgl_dir + CUBEMAP_VS_FILE), std::string(sgl_dir + CUBEMAP_FS_FILE));

	Obj3dBP::defaultSize = 1.0f;
	Obj3dBP			cubebp(sgl_dir + "obj3d/cube_down.obj", true, false);
	Math::Vector3	dimensions = cubebp.getDimensions();

	Texture *	lena = new Texture(sgl_dir + "images/lena.bmp");

#ifndef HUMAN_CLASS
	std::list<Obj3d*>	obj3dList;
	/*
		class Human { ... };
		class HumanEvolved : public Human { more members };
		class SuperSaiyan1 : HumanEvolved { with hairs! };
		class SuperSaiyan2 : SuperSaiyan1 { longer hairs!! };
		class SuperSaiyan3 : SuperSaiyan2 { even longer hairs!!! };

		class AnimationBH : Behavior { ... can repeat! };
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

	// AnimationBH kamehameha;
	// kamehameha.addTarget(&goku);

	// //render loop
	// kamehameha.run()


	// std::cout << "adresses main:" << std::endl;
	// std::cout << obj3dList.size() << std::endl;
	// for (auto i : obj3dList) {
	// 	std::cout << &(*i) << std::endl;
	// 	i->local.getScale().printData();
	// }


#endif // HUMAN_CLASS

	Fps	fps144(144);
	Fps	fps60(60);
	Fps	fps20(20);
	Fps* defaultFps = &fps60;

#ifndef BEHAVIORS

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

#endif // BEHAVIORS
	Behavior::areActive = false;
	// b0_rot.isActive = false;
	// b1_rot.isActive = false;
	// b2_rot.isActive = false;


	if (false) {
		std::cout << obj3dList.size() << std::endl;
		for (auto i : obj3dList) {
			i->local.getScale().printData();
		}
	}

	Texture*	texture2 = new Texture("SimpleGL/images/skybox4.bmp");//skybox3.bmp bug?
	Skybox		skybox(*texture2, sky_pg);
	
	Cam		cam(glfw);
	cam.local.setPos(0, 0, 10);
	cam.setFov(120);
	cam.lockedMovement = false;
	cam.lockedOrientation = false;
	cam.speed /= 4;

glfw.setMouseAngle(-1);

// #define TESTCUBE
#ifdef TESTCUBE
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
	std::cout << "fuck --------------------\n";

	#define MANYCUBES
	#ifdef MANYCUBES
		TransformBH		b5;
		b5.transform.rot.setUnit(ROT_DEG);
		b5.transform.rot.x = 200 * defaultFps->getTick();
		b5.transform.rot.y = 100 * defaultFps->getTick();
		b5.transform.rot.z = 60 * defaultFps->getTick();
		b5.modeRot = ADDITIVE;
		unsigned int max = 50;
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

	obj3dList.push_back(&origine);	// 96
	obj3dList.push_back(&pivot);	// 98
	obj3dList.push_back(&cube);		// 97
	obj3dList.push_back(&cubedir);	// 99
#endif

	// obj3dList.merge(bob->getObjList());
	obj3dList = bob->getObjList();

	list<Obj3d*>	raycastList = obj3dList;

// #define FRAMEBUFFER
#ifndef FRAMEBUFFER
	FrameBuffer	framebuffer(WINX, WINY);
	if (framebuffer.fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "FrameBuffer() failed : " << FrameBuffer::getFramebufferStatusInfos(framebuffer.fboStatus) << std::endl;
		return ;
	}

#endif
#ifndef GAMEMANAGER
	HumanManager	gameManager;
	gameManager.glfw = &glfw;
	gameManager.human = bob;
	gameManager.framebuffer = &framebuffer;
	gameManager.cam = &cam;
	gameManager.obj3dList = &raycastList;

	glfw.activateDefaultCallbacks(&gameManager);
	glfwSetMouseButtonCallback(glfw._window, HumanMouseButtonCallback);//override default callback
	// glfwSetWindowMonitor(glfw._window, NULL, 100, 100, WINX, WINY, 0);

#endif

#ifndef RENDER
	// glfwSetKeyCallback(glfw._window, key_callback);

	float ti = defaultFps->getTick();
	double t = glfwGetTime();
	// glfwSwapInterval(1);

	while (!glfwWindowShouldClose(glfw._window)) {
		if (defaultFps->wait_for_next_frame()) {
			// Fps::printGlobalFps();

			b0_rot.run();
			b1_rot.run();
			b2_rot.run();
			#ifdef MANYCUBES
			b5.run();
			#endif
			
			// printPixelRGB(0,0,GL_RGB, "");
			
			#ifdef TESTCUBE
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
			#endif

			// getObjectOnClic(0,0, &gameManager);

			glfwPollEvents();
			glfw.updateMouse(); // to do before cam's events
			cam.events(glfw, float(defaultFps->getTick()));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderObj3d(obj3dList, cam);
			renderSkybox(skybox, cam);

			if (1) {//copy FB on window-system-provided framebuffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glBlitFramebuffer(0, 0, WINX, WINY, 0, 0, WINX/4, WINY/4, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

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
