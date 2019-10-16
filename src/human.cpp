#include "human.hpp"

#define CUBE_OBJ		"SimpleGL/obj3d/cube_down.obj"
#define Z_THICKNESS		1.0f
#define SIZE_HEAD		0.8f
#define SIZE_MEMBER		1.2f	// size for entire member (thigh+calf and arm+forearm)
#define MIN_THICKNESS	0.1f
#define MIN_LENGHT		0.1f

Human::Human(Obj3dBP blueprint, Obj3dPG program, float thickness, float lenght)
: _thickness(thickness), _lenght(lenght), \
_head(blueprint, program), \
_trunk(blueprint, program), \
_leftArm(blueprint, program), \
_rightArm(blueprint, program), \
_leftForearm(blueprint, program), \
_rightForearm(blueprint, program), \
_leftThigh(blueprint, program), \
_rightThigh(blueprint, program), \
_leftCalf(blueprint, program), \
_rightCalf(blueprint, program)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	this->scaleHuman();
	//hierarchy
	this->_head.setParent(&this->_trunk);
	this->_leftArm.setParent(&this->_trunk);
	this->_rightArm.setParent(&this->_trunk);
	this->_leftThigh.setParent(&this->_trunk);
	this->_rightThigh.setParent(&this->_trunk);

	this->_leftForearm.setParent(&this->_leftArm);
	this->_rightForearm.setParent(&this->_rightArm);
	this->_leftCalf.setParent(&this->_leftThigh);
	this->_rightCalf.setParent(&this->_rightThigh);

	//colors
	this->_head.model.setColor(155, 155, 0);
	this->_trunk.model.setColor(0xff, 0, 0);
	this->_leftArm.model.setColor(0, 0xff, 0);
	this->_rightArm.model.setColor(0, 0, 0xff);
	this->_leftForearm.model.setColor(0xff, 0, 0);
	this->_rightForearm.model.setColor(0, 0xff, 0xff);

	this->_leftThigh.model.setColor(0xff, 0xff, 0);
	this->_rightThigh.model.setColor(0xff, 0, 0xff);
	this->_leftCalf.model.setColor(0, 0, 0);
	this->_rightCalf.model.setColor(0xff, 0xff, 0xff);

	// invert scale left members on X only
	Math::Vector3	s;
	s = this->_leftArm.model.local.getScale();		this->_leftArm.model.local.setScale(-s.x, s.y, s.z);
	s = this->_leftForearm.model.local.getScale();	this->_leftForearm.model.local.setScale(-s.x, s.y, s.z);
	s = this->_leftThigh.model.local.getScale();	this->_leftThigh.model.local.setScale(-s.x, s.y, s.z);
	s = this->_leftCalf.model.local.getScale();		this->_leftCalf.model.local.setScale(-s.x, s.y, s.z);

	//relative position
	Math::Vector3 offsetMembre = Math::Vector3(0, -this->_leftArm.model.local.getScale().y, 0);
	this->_head.local.translate(0, -this->_head.model.local.getScale().y, 0);
	this->_leftArm.local.translate(0, 0, 0);
	this->_rightArm.local.translate(this->_trunk.model.local.getScale().x, 0, 0);
	this->_leftForearm.local.translate(offsetMembre);
	this->_rightForearm.local.translate(offsetMembre);
	this->_leftThigh.local.translate(this->_trunk.model.local.getScale().x/2, -this->_trunk.model.local.getScale().y, 0);
	this->_rightThigh.local.translate(this->_trunk.model.local.getScale().x/2, -this->_trunk.model.local.getScale().y, 0);
	this->_leftCalf.local.translate(offsetMembre);
	this->_rightCalf.local.translate(offsetMembre);

	this->_objList.push_back(&this->_head.model);
	this->_objList.push_back(&this->_trunk.model);
	this->_objList.push_back(&this->_leftArm.model);
	this->_objList.push_back(&this->_rightArm.model);
	this->_objList.push_back(&this->_leftForearm.model);
	this->_objList.push_back(&this->_rightForearm.model);
	this->_objList.push_back(&this->_leftThigh.model);
	this->_objList.push_back(&this->_rightThigh.model);
	this->_objList.push_back(&this->_leftCalf.model);
	this->_objList.push_back(&this->_rightCalf.model);

	std::cout << "adresses:" << std::endl;
	std::cout << this->_objList.size() << std::endl;
	for (auto i : this->_objList) {
		std::cout << &(*i) << std::endl;
	}
}

// Human::Human(const Human & src) {//FIX not compiling, use only operator= ?
// 	*this = src;
// }

Human &			Human::operator=(const Human & src) {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	this->_blueprint = src._blueprint;
	this->_program = src._program;

	this->_thickness = src._thickness;
	this->_lenght = src._lenght;

	this->_head = src._head;
	this->_trunk = src._trunk;
	this->_leftArm = src._leftArm;
	this->_rightArm = src._rightArm;
	this->_leftForearm = src._leftForearm;
	this->_rightForearm = src._rightForearm;
	this->_leftThigh = src._leftThigh;
	this->_rightThigh = src._rightThigh;
	this->_leftCalf = src._leftCalf;
	this->_rightCalf = src._rightCalf;
	return (*this);
}

Human::~Human() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}
void			Human::setThickness(float thickness) {
	if (thickness >= MIN_THICKNESS) {
		this->_thickness = thickness;
	} else {
		this->_thickness = MIN_THICKNESS;
	}
	this->scaleHuman();
}
void			Human::setLenght(float lenght) {
	if (lenght >= MIN_LENGHT) {
		this->_lenght = lenght;
	} else {
		this->_lenght = MIN_LENGHT;
	}
	this->scaleHuman();
}
void			Human::setTrunkSize(float thickness, float lenght) {
	this->_trunk.model.local.setScale(thickness, lenght, Z_THICKNESS);
}
void			Human::setHeadSize(float thickness) {
	this->_head.model.local.setScale(SIZE_HEAD * thickness, SIZE_HEAD * thickness, Z_THICKNESS);
}
void			Human::setMembersSize(float thickness, float lenght) {
	this->_leftArm.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_rightArm.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_leftForearm.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_rightForearm.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_leftThigh.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_rightThigh.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_leftCalf.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
	this->_rightCalf.model.local.setScale(thickness, lenght * SIZE_MEMBER / 2.0f, Z_THICKNESS);
}

std::list<Obj3d*>	Human::getObjList() const { return (this->_objList); }


void			Human::scaleHuman() {
	this->setTrunkSize(this->_thickness, this->_lenght);
	this->setHeadSize(this->_thickness);
	this->setMembersSize(this->_thickness, this->_lenght);
}
