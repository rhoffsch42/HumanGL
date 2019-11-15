/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/11/15 11:16:52 by rhoffsch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "humangl.h"
#include "human.hpp"
#include "humanevolved.hpp"
#include "supersaiyan1.hpp"

#include <string>
#include <cstdio>
#include <vector>
#include <list>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

class FrameBuffer {
	/*
		http://www.songho.ca/opengl/gl_fbo.html
		MSAA https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
		https://alexandre-laurent.developpez.com/tutoriels/OpenGL/OpenGL-FBO/
	*/
public:
	FrameBuffer(int w, int h) {
		this->rbo_format = GL_DEPTH_COMPONENT;
		this->tex_format = GL_RGBA;
		this->width = w;
		this->height = h;

		if (1) {//TODO check 0 < width & height < GL_MAX_RENDERBUFFER_SIZE
			//resize
		}

		// frame buffer
		glGenFramebuffers(1, &this->fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo_id);

		// render buffer
		glGenRenderbuffers(1, &this->rbo_id);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rbo_id);
		glRenderbufferStorage(GL_RENDERBUFFER, this->rbo_format, this->width, this->height);

		// texture
		glGenTextures(1, &this->tex_id);
		glBindTexture(GL_TEXTURE_2D, this->tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, this->tex_format, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		// attach
		std::cout << "attach: " << this->fbo_id << " : " << this->rbo_id << " | " << this->tex_id << std::endl;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rbo_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,	this->tex_id, 0);// mipmap level: 0(base)

		this->fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	~FrameBuffer() {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);//0 = detach renderbuffer
		glDeleteRenderbuffers(1, &this->rbo_id);
		glDeleteFramebuffers(1, &this->fbo_id);
	}
	GLuint	fbo_id;
	GLenum	fbo_status;
	int		width;
	int		height;
	GLuint	rbo_id;
	GLenum  rbo_format;
	GLuint	tex_id;
	GLenum  tex_format;

	bool checkFramebufferStatus(GLuint fbo)
	{
		// check FBO status
		glBindFramebuffer(GL_FRAMEBUFFER, fbo); // bind
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch(status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			std::cout << "Framebuffer complete." << std::endl;
			return true;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
			return false;
		/*
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
			std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
			return false;
		*/
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout << "[ERROR] Framebuffer incomplete: Multisample." << std::endl;
			return false;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation." << std::endl;
			return false;

		default:
			std::cout << "[ERROR] Framebuffer incomplete: Unknown error." << std::endl;
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);   // unbind
	}

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
private:
};

void	getObjectOnClic(int x, int y, HumanManager * manager) {
	if (!manager->human || !manager->glfw || !manager->cam || !manager->framebuffer) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : one of several HumanManager pointer is null." << std::endl;
		return ;
	}
	list<Obj3d*>	obj3dList = manager->human->getObjList();//should return full list depending on Human type
	std::cout << "searching from " << obj3dList.size() << " obj3d" << std::endl;
	y = manager->glfw->getHeight() - y;
	std::cout << "pos: " << x << " - " << y << std::endl;
	GLubyte data[4];//RGBA

	//render in the offscreen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, manager->framebuffer->fbo_id);
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

	//read the pixel on x y, get the ID
	glReadPixels(x,	y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);
	unsigned int id = 0;
	Misc::RGBToInt(&(*data), &id);
	std::cout << "id is " << id << " (" << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2] << ")" << std::endl;
	for (auto it : obj3dList) {
		if (it->getId() == id)
			std::cout << "Found!" << std::endl;
	}
	//restore window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void		HumanMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
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

	#ifdef MANYCUBES
		TransformBH		b5;
		b5.transform.rot.setUnit(ROT_DEG);
		b5.transform.rot.x = 200 * defaultFps->getTick();
		b5.transform.rot.y = 100 * defaultFps->getTick();
		b5.transform.rot.z = 60 * defaultFps->getTick();
		b5.modeRot = ADDITIVE;
		unsigned int max = 100;
		if (0) {
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
		// cam.speed *= 10;	
		#endif

	obj3dList.push_back(&origine);	// 96
	obj3dList.push_back(&pivot);	// 98
	obj3dList.push_back(&cube);		// 97
	obj3dList.push_back(&cubedir);	// 99
#endif

	obj3dList = bob->getObjList();

// #define FRAMEBUFFER
#ifndef FRAMEBUFFER
	FrameBuffer	fb1(WINX, WINY);
	FrameBuffer	fb2(WINX, WINY);
	if (fb1.fbo_status != GL_FRAMEBUFFER_COMPLETE || fb2.fbo_status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "FraneBuffer() failed : " << fb1.fbo_status << std::endl;
		fb1.checkFramebufferStatus(fb1.fbo_id);
		fb2.checkFramebufferStatus(fb2.fbo_id);
		exit(10);
	}
	std::cout << std::endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	FrameBuffer * list[2] = {&fb1, &fb2};
	if (1) {
		for (size_t i = 0; i < 1; i++) {
			FrameBuffer * fb = list[i];
			std::cout << "fb: " << fb->fbo_id << std::endl;
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->fbo_id);
			// glClearColor(0.5f/(i+1), 0.5f/(i+1), 0.5f/(i+1), 1.0f);//= RGB(25, 51, 102);	//to do before glClear()
			glClearColor(1, 1, 1, 1.0f);//= RGB(25, 51, 102);	//to do before glClear()
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, fb->fbo_id);
			printPixelRGB(WINX/2,WINY/2, fb->tex_format, "clear ");
			printPixelRGB(WINX/2,WINY/2, fb->rbo_format, "rbo ");

			// glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->fbo_id);
			// renderObj3d(obj3dList, cam);
			// renderSkybox(skybox, cam);

			printPixelRGB(WINX/2,WINY/2, fb->tex_format, "fb ");

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			std::cout << std::endl;
		}
	}
	if (false) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb1.fbo_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb2.fbo_id);
		glBlitFramebuffer(0, 0, WINX, WINY, 0, 0, WINX, WINY, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb2.fbo_id);
		printPixelRGB(WINX/2,WINY/2, fb2.tex_format, "fb2 tex");
		printPixelRGB(WINX/2,WINY/2, fb2.rbo_format, "fb2 rbo");
	}
	if (false) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb1.fbo_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, WINX, WINY, 0, 0, WINX, WINY, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	fb1.checkFramebufferStatus(0);
	fb1.checkFramebufferStatus(fb1.fbo_id);
	fb1.checkFramebufferStatus(fb2.fbo_id);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

// exit(0);
#endif
#ifndef GAMEMANAGER
	HumanManager	gameManager;
	gameManager.glfw = &glfw;
	gameManager.human = bob;
	gameManager.framebuffer = &fb1;
	gameManager.cam = &cam;

	glfw.activateDefaultCallbacks(&gameManager);
	glfwSetMouseButtonCallback(glfw._window, HumanMouseButtonCallback);//override default callback


	// GLFWwindow *	fullWin = glfwCreateWindow(WINX, WINY, "My Title", glfwGetPrimaryMonitor(), NULL);//fullscreen
	GLFWwindow *	winUsed = glfw._window;
	GLFWmonitor *	monitor = glfwGetWindowMonitor(winUsed);
	if (monitor) {//fullscreen
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(winUsed, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		std::cout << "monitor refresh rate: " << mode->refreshRate << std::endl;
	} else {
		glfwSetWindowMonitor(winUsed, NULL, 100, 100, WINX, WINY, 0);
	}
	// glfwMaximizeWindow(winUsed);
	// glfwRestoreWindow(winUsed);

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
			// b5.run();
			
			// printPixelRGB(0,0,GL_RGB, "");
			
			#ifdef TESTCUBE
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
			#endif


			glfwPollEvents();
			glfw.updateMouse(); // to do before cam's events
			cam.events(glfw, float(defaultFps->getTick()));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderObj3d(obj3dList, cam);
			renderSkybox(skybox, cam);

			if (false) {//override window-system-provided framebuffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, fb1.fbo_id);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glBlitFramebuffer(0, 0, WINX, WINY, 0, 0, WINX, WINY, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		
			glfwSwapBuffers(glfw._window);
			if (GLFW_PRESS == glfwGetKey(glfw._window, GLFW_KEY_ESCAPE))
				glfwSetWindowShouldClose(glfw._window, GLFW_TRUE);
		}
	}
#endif // RENDER
}

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
