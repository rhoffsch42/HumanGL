#include "humanevolved.hpp"
#include <iostream>

HumanEvolved::HumanEvolved(Obj3dBP & blueprint, Obj3dPG & program, float thickness, float lenght) \
: Human(blueprint, program), \
_leftFoot(blueprint, program), \
_rightFoot(blueprint, program), \
_leftHand(blueprint, program), \
_rightHand(blueprint, program)
{
	(void)thickness;(void)lenght;

	//hierarchy
	this->_leftFoot.setParent(&this->_leftCalf);
	this->_rightFoot.setParent(&this->_rightCalf);
	this->_leftHand.setParent(&this->_leftForearm);
	this->_rightHand.setParent(&this->_rightForearm);

	this->scaleHuman();//calls positionMembers() too

	//rotations
	this->_leftHand.local.setRot(90, 0, 0);
	this->_rightHand.local.setRot(90, 0, 0);

	this->addExtremityToList(this->_leftFoot);
	this->addExtremityToList(this->_rightFoot);
	this->addExtremityToList(this->_leftHand);
	this->addExtremityToList(this->_rightHand);
}

HumanEvolved::~HumanEvolved() {
	// delete opengl objects
}

void	HumanEvolved::scaleHuman() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	Human::scaleHuman();//the call to the mother class function

	// they call extremity.position() too
	this->_leftFoot.scale(this->_thickness);
	this->_rightFoot.scale(this->_thickness);
	this->_leftHand.scale(this->_thickness);
	this->_rightHand.scale(this->_thickness);

	this->positionMembers();
}

void	HumanEvolved::positionMembers() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	Human::positionMembers();//the call to the mother class function

	this->_leftFoot.local.setPos(this->_leftCalf.model.local.getScale().x/2 - this->_leftFoot.base.model.local.getScale().x/2, \
								-this->_leftCalf.model.local.getScale().y, \
								this->_leftCalf.model.local.getScale().z/2);
	this->_rightFoot.local.setPos(this->_leftCalf.model.local.getScale().x/2 - this->_rightFoot.base.model.local.getScale().x/2, \
								-this->_rightCalf.model.local.getScale().y, \
								this->_rightCalf.model.local.getScale().z/2);
	this->_leftHand.local.setPos(this->_leftCalf.model.local.getScale().x/2 - this->_leftHand.base.model.local.getScale().x/2, \
								-this->_leftForearm.model.local.getScale().y, \
								this->_leftForearm.model.local.getScale().z/2);
	this->_rightHand.local.setPos(this->_leftCalf.model.local.getScale().x/2 - this->_rightHand.base.model.local.getScale().x/2, \
								-this->_rightForearm.model.local.getScale().y, \
								this->_rightForearm.model.local.getScale().z/2);

}

void	HumanEvolved::addExtremityToList(Extremity & extremity) {
	this->_objList.push_back(&extremity.base.model);
	this->_objList.push_back(&extremity.finger1.model);
	this->_objList.push_back(&extremity.finger2.model);
	this->_objList.push_back(&extremity.finger3.model);
	this->_objList.push_back(&extremity.finger4.model);
	this->_objList.push_back(&extremity.finger5.model);
}
