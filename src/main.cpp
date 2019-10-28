/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhoffsch <rhoffsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/18 22:45:30 by rhoffsch          #+#    #+#             */
/*   Updated: 2019/10/28 11:15:10 by rhoffsch         ###   ########.fr       */
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

Obj3d	createMember(Obj3dBP bp, Obj3dPG pg, float longueur, Math::Vector3 color, bool centered = false) {
	Obj3d			member(bp, pg);
	member.displayTexture = false;
	member.setColor(color.x, color.y, color.z);
	member.local.centered = centered;
	member.local.setScale(1,longueur,1);
	return member;
}

void sceneHumanGL() {
	std::string sgl_dir = "SimpleGL/";
	Glfw		glfw(1600, 900); // screen size
	glDisable(GL_CULL_FACE);
	Obj3dPG		obj3d_prog(std::string(sgl_dir + OBJ3D_VS_FILE), std::string(sgl_dir + OBJ3D_FS_FILE));
	SkyboxPG	sky_pg(std::string(sgl_dir + CUBEMAP_VS_FILE), std::string(sgl_dir + CUBEMAP_FS_FILE));

	Obj3dBP::defaultSize = 1.0f;
	Obj3dBP			cubebp(sgl_dir + "obj3d/cube_down.obj", true);
	Math::Vector3	dimensions = cubebp.getDimensions();
	cubebp.setCenterOffset(dimensions.x / 2, 0, dimensions.z / 2);

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
	// bob->setMembersSize(thickness*3, lenght*8);
	// bob->setTrunkSize(thickness * 5, lenght*5);
	// bob->setHeadSize(thickness * 2);

	bob->setLenght(lenght*3);
	bob->setThickness(thickness*3);
	// HumanEvolved		jack = new HumanEvolved(bob);
	// SuperSaiyan1	goku(jack);
	// goku.setHairColor(BLOND);

	// AnimationBH kamehameha;
	// kamehameha.addTarget(&goku);

	// //render loop
	// kamehameha.run()

	obj3dList = bob->getObjList();
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
	b0_rot.transform.rot.y = 80 * defaultFps->tick;
	b0_rot.modeRot = ADDITIVE;
	b0_rot.addTarget(&bob->_trunk);


	TransformBH		b1_rot;
	b1_rot.transform.rot.setUnit(ROT_DEG);
	b1_rot.transform.rot.x = 200 * defaultFps->tick;
	b1_rot.transform.rot.y = 100 * defaultFps->tick;
	b1_rot.transform.rot.z = 60 * defaultFps->tick;
	b1_rot.modeRot = ADDITIVE;
	b1_rot.addTarget(&bob->_rightArm);
	b1_rot.addTarget(&bob->_rightForearm);
	b1_rot.addTarget(&bob->_leftThigh);
	b1_rot.addTarget(&bob->_rightCalf);

	// bob->_rightCalf.addBehavior(&b1_rot);

	TransformBH		b2_rot;
	b2_rot.transform.rot.setUnit(ROT_DEG);
	b2_rot.transform.rot.x = 200 * defaultFps->tick;
	b2_rot.transform.rot.y = 80 * defaultFps->tick;
	b2_rot.transform.rot.y = 250 * defaultFps->tick;
	b2_rot.modeRot = ADDITIVE;
	b2_rot.addTarget(&bob->_leftArm);
	b2_rot.addTarget(&bob->_leftForearm);
	b2_rot.addTarget(&bob->_leftCalf);
	b2_rot.addTarget(&bob->_rightThigh);


	
	if (false) {// the dab !
		bob->_head.local.setRot(-30,0,0);
		bob->_trunk.local.setRot(0,0,-15);
		bob->_leftArm.local.setRot(0,0,-110);
		bob->_rightArm.local.setRot(110,40,-30);
		bob->_rightForearm.local.setRot(90,0,-20);
		bob->_rightThigh.local.setRot(0,0,45);
		bob->_rightCalf.local.setRot(0,0,-45);
	} else if (false) {
		float x = 0.0f;
		float z = 20.0f;
		bob->_leftArm.local.setRot(x, 0, -z);
		bob->_rightArm.local.setRot(x, 0, z);
		bob->_leftThigh.local.setRot(x, 0, -z);
		bob->_rightThigh.local.setRot(x, 0, z);
	}

	// avant_bras_gauche.behaviorsActive = false;
#endif // BEHAVIORS
	Behavior::areActive = false;


	if (false) {
	std::cout << obj3dList.size() << std::endl;
		for (auto i : obj3dList) {
			i->local.getScale().printData();
		}
	}

	Texture*	texture2 = new Texture("SimpleGL/images/skybox4.bmp");//skybox3.bmp bug?
	Skybox		skybox(*texture2, sky_pg);
	
	Cam		cam(glfw);
	cam.local.centered = false;
	cam.local.setPos(0, 0, 10);
	cam.setFov(120);
	cam.lockedMovement = false;
	cam.lockedOrientation = false;
	cam.speed /= 4;

#ifndef RENDER

	double t = glfwGetTime();
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(glfw._window)) {
		// std::cout << glfwGetTime() - t << std::endl;

		if (defaultFps->wait_for_next_frame()) {
			b0_rot.run();
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
