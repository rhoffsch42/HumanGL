
#pragma once

#include "human.hpp" //BodyPart //FIX mettre la declaration ici, et include extremity.hpp dans human?
#include "obj3dBP.hpp"
#include "obj3dPG.hpp"

class Extremity : public Object
{
public:
	Extremity(Obj3dBP & blueprint, Obj3dPG & program);
	Extremity(const Extremity & src);
	Extremity &		operator=(const Extremity & src);
	~Extremity();
	
	//list de finger qu'on peut changer ? faire des mains avec 8 doigts! 
	BodyPart base;
	BodyPart finger1;
	BodyPart finger2;
	BodyPart finger3;
	BodyPart finger4;
	BodyPart finger5;

	void	scale(float thickness);
	void	position(float thickness);
private:
};
