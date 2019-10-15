/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/04/03 14:58:50 by rhoffsch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "humangl.h"
#include "simplegl.h"

#include "program.hpp"
#include "object.hpp"
#include "obj3dPG.hpp"
#include "obj3dBP.hpp"
#include "obj3d.hpp"
#include "misc.hpp"
#include "cam.hpp"
#include "texture.hpp"
#include "skyboxPG.hpp"
#include "skybox.hpp"
#include "glfw.hpp"
#include "transformBH.hpp"

#include <string>
#include <cstdio>
#include <vector>
#include <list>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

class Fps
{
public:
	Fps(int fps_val) {
		this->fps = fps_val;
		this->old_fps = fps_val;
		this->tick = 1.0 / this->fps;
		this->last_time = glfwGetTime();
		this->ellapsed_time = 0.0;
	}
	double				fps;
	double				old_fps;
	double				tick;
	double				ellapsed_time;
	double				last_time;

	bool		wait_for_next_frame() {
		this->ellapsed_time = glfwGetTime() - this->last_time;
		if (this->ellapsed_time >= this->tick)
		{
			this->last_time += this->ellapsed_time;
			this->ellapsed_time = 0.0;
			return (true);
		}
		else
			return (false);
	}
};

void	printFps() {
	static double last_time = 0;
	static double ellapsed_time = 0;
	double	current_time;
	double	fps;
	double	cent;
	current_time = glfwGetTime();
	ellapsed_time = current_time - last_time;
	fps = 1.0 / ellapsed_time;
	cent = fps - double(int(fps));
	if (cent >= 0.5)
		fps += 1.0;
	cout << (float)current_time << "\t" << int(fps) << "fps" << endl;
	last_time += ellapsed_time;
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
	for (Obj3d* object : obj3dList) {
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
	std::string sgl_dir = "SimpleGL/";
	Glfw		glfw(1600, 900); // screen size
	glDisable(GL_CULL_FACE);
	Obj3dPG		obj3d_prog(std::string(sgl_dir + OBJ3D_VS_FILE), std::string(sgl_dir + OBJ3D_FS_FILE));
	SkyboxPG	sky_pg(std::string(sgl_dir + CUBEMAP_VS_FILE), std::string(sgl_dir + CUBEMAP_FS_FILE));

	Obj3dBP::defaultSize = 1.0f;
	Obj3dBP			cubebp(sgl_dir + "obj3d/cube.obj", true);
	Math::Vector3	dimensions = cubebp.getDimensions();
	cubebp.setCenterOffset(dimensions.x / 2, 0, dimensions.z / 2);

	Texture *	lena = new Texture(sgl_dir + "images/lena.bmp");

#ifndef MEMBERS
	float		epaisseur_tronc = 2.0f;
	bool		centerCubes = false;

	Object			containerTronc;
	Obj3d			tronc(cubebp, obj3d_prog);
	tronc.setTexture(lena);
	tronc.displayTexture = true;
	tronc.local.setScale(epaisseur_tronc, epaisseur_tronc * 3, epaisseur_tronc);//bad, use rescale of obj3d vertices
	tronc.setColor(0xff, 0, 0);
	tronc.local.centered = centerCubes;

	Obj3d			avant_bras_gauche(cubebp, obj3d_prog);
	avant_bras_gauche.displayTexture = false;
	avant_bras_gauche.setColor(0, 0xff, 0);
	avant_bras_gauche.local.centered = centerCubes;

	Obj3d			avant_bras_droit(cubebp, obj3d_prog);
	avant_bras_droit.displayTexture = false;
	avant_bras_droit.setColor(0, 0, 0xff);
	avant_bras_droit.local.centered = centerCubes;

	Obj3d			apres_bras_gauche(cubebp, obj3d_prog);
	apres_bras_gauche.displayTexture = false;
	apres_bras_gauche.setColor(0xff, 0, 0);
	apres_bras_gauche.local.centered = centerCubes;

	Obj3d			apres_bras_droit(cubebp, obj3d_prog);
	apres_bras_droit.displayTexture = false;
	apres_bras_droit.setColor(0, 0xff, 0xff);
	apres_bras_droit.local.centered = centerCubes;

	//hierarchy
	tronc.setParent(&containerTronc);
	avant_bras_gauche.setParent(&containerTronc);
	apres_bras_gauche.setParent(&avant_bras_gauche);
	avant_bras_droit.setParent(&containerTronc);
	apres_bras_droit.setParent(&avant_bras_droit);


	//relative position
	// avant_bras_gauche.local.translate(-1.5, 0, 0);
	avant_bras_droit.local.translate(4.5, 0, 0);
	// apres_bras_gauche.local.translate(VEC3_DOWN);
	// apres_bras_droit.local.translate(VEC3_DOWN);

	//debug
	avant_bras_gauche.local.translate(0, 0.1, 0);
	
	avant_bras_gauche.local.getPos().printData();
	avant_bras_gauche.local.updateMatrix();
	avant_bras_gauche.local.getMatrix().printData();
	cout << "----------\n";
	apres_bras_gauche.local.getPos().printData();
	apres_bras_gauche.local.updateMatrix();
	apres_bras_gauche.local.getMatrix().printData();

	list<Obj3d*>	obj3dList;
	// obj3dList.push_back(&containerTronc);
	obj3dList.push_back(&tronc);
	obj3dList.push_back(&avant_bras_gauche);
	obj3dList.push_back(&avant_bras_droit);
	obj3dList.push_back(&apres_bras_droit);
	obj3dList.push_back(&apres_bras_gauche);
#endif // MEMBERS

	Texture*	texture2 = new Texture("SimpleGL/images/skybox4.bmp");//skybox3.bmp bug?
	Skybox		skybox(*texture2, sky_pg);
	
	Cam		cam(glfw);
	cam.local.centered = false;
	cam.local.setPos(0, 0, 10);
	cam.setFov(90);
	cam.lockedMovement = false;
	cam.lockedOrientation = false;
	cam.speed /= 4;

	Fps	fps144(144);
	Fps	fps60(60);
	Fps	fps20(5);
	Fps* defaultFps = &fps60;

#ifndef BEHAVIORS
	TransformBH		b1_rot;
	b1_rot.transform.rot.setUnit(ROT_DEG);
	b1_rot.transform.rot.x = 20 * defaultFps->tick;
	// b1_rot.transform.rot.y = 20 * defaultFps->tick;
	// b1_rot.transform.rot.z = 20 * defaultFps->tick;
	b1_rot.modeRot = ADDITIVE;
	b1_rot.addTarget(&avant_bras_droit);
	b1_rot.addTarget(&apres_bras_droit);
	// b1_rot.addTarget(&containerTronc);

	TransformBH		b2_rot;
	b2_rot.transform.rot.setUnit(ROT_DEG);
	// b2_rot.transform.rot.x = 20 * defaultFps->tick;
	b2_rot.transform.rot.y = 50 * defaultFps->tick;
	b2_rot.modeRot = ADDITIVE;
	// b2_rot.addTarget(&avant_bras_gauche);
	// b2_rot.addTarget(&avant_bras_droit);
	// b2_rot.addTarget(&apres_bras_droit);

	// avant_bras_gauche.behaviorsActive = false;
	// Behavior::areActive = false;
#endif // BEHAVIORS

#ifndef RENDER

	double t = glfwGetTime();
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(glfw._window)) {
		// std::cout << glfwGetTime() - t << std::endl;

		if (defaultFps->wait_for_next_frame()) {
			b1_rot.run();
			b2_rot.run();

			glfwPollEvents();
			glfw.updateMouse(); // to do before cam's events
			cam.events(glfw, float(defaultFps->tick));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderObj3d(obj3dList, cam);
			renderSkybox(skybox, cam);
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
