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
#include "human.hpp"

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

#ifndef MEMBERS
	float		epaisseur_tronc = 2.0f;
	bool		centerCubes = false;
	float		longueurBras = epaisseur_tronc * 4/2;

	Object			containerT;
	Object			containerL1;
	Object			containerL2;
	Object			containerR1;
	Object			containerR2;
	Object			containerLL1;
	Object			containerLL2;
	Object			containerLR1;
	Object			containerLR2;

	float			grosseTete = 1.8f;
	Obj3d			head(cubebp, obj3d_prog);
	head.local.setScale(grosseTete, grosseTete, grosseTete);
	head.setColor(155, 155, 0);
	head.local.centered = centerCubes;

	Obj3d			tronc(cubebp, obj3d_prog);
	tronc.setTexture(lena);
	tronc.displayTexture = true;
	tronc.local.setScale(epaisseur_tronc, epaisseur_tronc * 3, epaisseur_tronc);
	tronc.setColor(0xff, 0, 0);
	tronc.local.centered = centerCubes;

	Obj3d avant_bras_gauche = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0,0xff,0));
	Obj3d avant_bras_droit = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0,0,0xff));
	Obj3d apres_bras_gauche = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0xff,0,0));
	Obj3d apres_bras_droit = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0,0xff,0xff));

	Obj3d avant_leg_gauche = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0xff,0xff,0));
	Obj3d avant_leg_droit = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0xff,0,0xff));
	Obj3d apres_leg_gauche = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0,0,0));
	Obj3d apres_leg_droit = createMember(cubebp, obj3d_prog, longueurBras, Math::Vector3(0xff,0xff,0xff));

	// invert scale left members on X only
	Math::Vector3	s;
	s = avant_bras_gauche.local.getScale();	avant_bras_gauche.local.setScale(-s.x, s.y, s.z);
	s = apres_bras_gauche.local.getScale();	apres_bras_gauche.local.setScale(-s.x, s.y, s.z);
	s = avant_leg_gauche.local.getScale();	avant_leg_gauche.local.setScale(-s.x, s.y, s.z);
	s = apres_leg_gauche.local.getScale();	apres_leg_gauche.local.setScale(-s.x, s.y, s.z);

	//hierarchy
	tronc.setParent(&containerT);
	head.setParent(&containerT);
	avant_bras_gauche.setParent(&containerL1);
	apres_bras_gauche.setParent(&containerL2);
	avant_bras_droit.setParent(&containerR1);
	apres_bras_droit.setParent(&containerR2);
	containerL1.setParent(&containerT);
	containerR1.setParent(&containerT);
	containerL2.setParent(&containerL1);
	containerR2.setParent(&containerR1);

	avant_leg_gauche.setParent(&containerLL1);
	apres_leg_gauche.setParent(&containerLL2);
	avant_leg_droit.setParent(&containerLR1);
	apres_leg_droit.setParent(&containerLR2);
	containerLL1.setParent(&containerT);
	containerLR1.setParent(&containerT);
	containerLL2.setParent(&containerLL1);
	containerLR2.setParent(&containerLR1);


	//relative position
	if (centerCubes) {
		Math::Vector3 offsetMembre = Math::Vector3(0, -longueurBras, 0);
		head.local.translate(0, head.local.getScale().y, 0);
		containerL1.local.translate(-tronc.local.getScale().x, 0, 0);
	} else {
		Math::Vector3 offsetMembre = Math::Vector3(0, -longueurBras, 0);
		head.local.translate(0, head.local.getScale().y, 0);
		containerL1.local.translate(0, 0, 0);
		containerR1.local.translate(tronc.local.getScale().x, 0, 0);
		containerL2.local.translate(offsetMembre);
		containerR2.local.translate(offsetMembre);
		containerLL1.local.translate(tronc.local.getScale().x/2, -tronc.local.getScale().y, 0);
		containerLR1.local.translate(tronc.local.getScale().x/2, -tronc.local.getScale().y, 0);
		containerLL2.local.translate(offsetMembre);
		containerLR2.local.translate(offsetMembre);
	}

	//debug
	avant_bras_gauche.local.getPos().printData();
	avant_bras_gauche.local.updateMatrix();
	avant_bras_gauche.local.getMatrix().printData();
	cout << "----------\n";
	apres_bras_gauche.local.getPos().printData();
	apres_bras_gauche.local.updateMatrix();
	apres_bras_gauche.local.getMatrix().printData();

	list<Obj3d*>	obj3dList;
	// obj3dList.push_back(&containerTronc);
	obj3dList.push_back(&head);
	obj3dList.push_back(&tronc);
	obj3dList.push_back(&avant_bras_gauche);
	obj3dList.push_back(&avant_bras_droit);
	obj3dList.push_back(&apres_bras_droit);
	obj3dList.push_back(&apres_bras_gauche);
	obj3dList.push_back(&avant_leg_gauche);
	obj3dList.push_back(&avant_leg_droit);
	obj3dList.push_back(&apres_leg_droit);
	obj3dList.push_back(&apres_leg_gauche);
#endif // MEMBERS

	Texture*	texture2 = new Texture("SimpleGL/images/skybox4.bmp");//skybox3.bmp bug?
	Skybox		skybox(*texture2, sky_pg);
	
	Cam		cam(glfw);
	cam.local.centered = false;
	cam.local.setPos(0, 0, 10);
	cam.setFov(120);
	cam.lockedMovement = false;
	cam.lockedOrientation = false;
	cam.speed /= 4;

	Fps	fps144(144);
	Fps	fps60(60);
	Fps	fps20(5);
	Fps* defaultFps = &fps60;

#ifndef BEHAVIORS
	TransformBH		b0_rot;
	b0_rot.transform.rot.setUnit(ROT_DEG);
	b0_rot.transform.rot.y = 80 * defaultFps->tick;
	b0_rot.modeRot = ADDITIVE;
	b0_rot.addTarget(&containerT);

	TransformBH		b1_rot;
	b1_rot.transform.rot.setUnit(ROT_DEG);
	b1_rot.transform.rot.x = 200 * defaultFps->tick;
	b1_rot.transform.rot.y = 100 * defaultFps->tick;
	b1_rot.transform.rot.z = 60 * defaultFps->tick;
	b1_rot.modeRot = ADDITIVE;
	b1_rot.addTarget(&containerR1);
	b1_rot.addTarget(&containerR2);
	b1_rot.addTarget(&containerLL1);
	b1_rot.addTarget(&containerLR2);

	TransformBH		b2_rot;
	b2_rot.transform.rot.setUnit(ROT_DEG);
	b2_rot.transform.rot.x = 200 * defaultFps->tick;
	b2_rot.transform.rot.y = 80 * defaultFps->tick;
	b2_rot.transform.rot.y = 250 * defaultFps->tick;
	b2_rot.modeRot = ADDITIVE;
	b2_rot.addTarget(&containerL1);
	b2_rot.addTarget(&containerL2);
	b2_rot.addTarget(&containerLL2);
	b2_rot.addTarget(&containerLR1);


	// the dab !
	if (true) {
		head.local.setRot(30,0,0);
		containerT.local.setRot(0,180,-15);
		containerL1.local.setRot(0,0,-110);
		containerR1.local.setRot(110,40,-30);
		containerR2.local.setRot(90,0,-20);
		containerLR1.local.setRot(0,0,45);
		containerLR2.local.setRot(0,0,-45);
	}


	// avant_bras_gauche.behaviorsActive = false;
#endif // BEHAVIORS
	Behavior::areActive = false;

#ifndef HUMAN_CLASS
	/*
		class Human { ... };
		class HumanEvolved : public Human { more members };
		class SuperSaiyan1 : HumanEvolved { with hairs! };
		class SuperSaiyan2 : SuperSaiyan2 { longer hairs!! };
		class SuperSaiyan3 : SuperSaiyan3 { even longer hairs!!! };

		class AnimationBH : Behavior { ... can repeat! };
	*/
	float		thickness = 1.0f;	// default value
	float		lenght = 4.0f;		// default value

	Human *		bob = new Human(cubebp, obj3d_prog);
	bob->setMembersSize(thickness, lenght*3);
	bob->setTrunkSize(thickness * 5, lenght*8);
	bob->setHeadSize(thickness * 1.5);

	// bob->setLenght(lenght * 8);
	// bob->setThickness(thickness * 8);

	// SuperSaiyan1	goku(bob);
	// goku.setHairColor(BLOND);

	// AnimationBH kamehameha;
	// kamehameha.addTarget(&goku);

	// //render loop
	// kamehameha.run()

	list<Obj3d*>	obj3dList2 = bob->getObjList();
	// std::cout << "adresses main:" << std::endl;
	// std::cout << obj3dList2.size() << std::endl;
	// for (auto i : obj3dList2) {
	// 	std::cout << &(*i) << std::endl;
	// 	i->local.getScale().printData();
	// }

#endif // HUMAN_CLASS

	if (false) {
	std::cout << obj3dList.size() << std::endl;
		for (auto i : obj3dList) {
			i->local.getScale().printData();
		}
	}


#ifndef RENDER

	double t = glfwGetTime();
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(glfw._window)) {
		// std::cout << glfwGetTime() - t << std::endl;

		if (defaultFps->wait_for_next_frame()) {
			// b0_rot.run();
			b1_rot.run();
			b2_rot.run();

			glfwPollEvents();
			glfw.updateMouse(); // to do before cam's events
			cam.events(glfw, float(defaultFps->tick));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderObj3d(obj3dList2, cam);
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
