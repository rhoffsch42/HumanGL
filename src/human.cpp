#include "human.hpp"
#include <json/json.hpp>
using json = nlohmann::json;

#define Z_THICKNESS		1.0f
#define SIZE_HEAD		1.5f
#define SIZE_MEMBER		1.2f	// size for entire member (thigh+calf and arm+forearm)
#define SIZE_TRUNK		2.0f	// only X
#define MIN_THICKNESS	0.1f
#define MIN_LENGHT		0.1f

Human::Human(Obj3dBP & blueprint, Obj3dPG & program, float thickness, float lenght)
: _head(blueprint, program), \
_trunk(blueprint, program), \
_leftArm(blueprint, program), \
_rightArm(blueprint, program), \
_leftForearm(blueprint, program), \
_rightForearm(blueprint, program), \
_leftThigh(blueprint, program), \
_rightThigh(blueprint, program), \
_leftCalf(blueprint, program), \
_rightCalf(blueprint, program), \
_thickness(thickness), _lenght(lenght)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	this->scaleHuman();
	this->positionMembers();

	//rescale left members tmp //FIX 
	this->_leftArm.local.setScale(-1, 1, 1);
	this->_leftThigh.local.setScale(-1, 1, 1);

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
	this->_head.model.setColor((char)1, (char)1, 0);
	this->_trunk.model.setColor(50, 20, 10);
	this->_leftArm.model.setColor(0, 0xff, 0);
	this->_rightArm.model.setColor(0, 0, 0xff);
	this->_leftForearm.model.setColor(0xff, 0, 0);
	this->_rightForearm.model.setColor(0, 0xff, 0xff);

	this->_leftThigh.model.setColor(0xff, 0xff, 0);
	this->_rightThigh.model.setColor(0xff, 0, 0);
	this->_leftCalf.model.setColor(0, 0, 0);
	this->_rightCalf.model.setColor(0xff, 0xff, 0xff);

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
}

// Human::Human(const Human & src) {//FIX not compiling, use only operator= ?
// 	*this = src;
// }

Human &			Human::operator=(const Human & src) {//FIX Obj3d model are well copied ?
	std::cout << __PRETTY_FUNCTION__ << std::endl;
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

std::string		Human::getKeyFrame() const {
	std::stringstream ss;
	Math::Rotation r[10];

	r[0] = this->_head.local.getRot();
	r[1] = this->_trunk.local.getRot();
	r[2] = this->_leftArm.local.getRot();
	r[3] = this->_leftForearm.local.getRot();
	r[4] = this->_rightArm.local.getRot();
	r[5] = this->_rightForearm.local.getRot();
	r[6] = this->_leftThigh.local.getRot();
	r[7] = this->_leftCalf.local.getRot();
	r[8] = this->_rightThigh.local.getRot();
	r[9] = this->_rightCalf.local.getRot();

	json human =
	{
		{"frame",
			{
				{"head",		{r[0].x, r[0].y, r[0].z}},
				{"trunk",		{r[1].x, r[1].y, r[1].z}},
				{"leftArm",		{r[2].x, r[2].y, r[2].z}},
				{"leftForearm",	{r[3].x, r[3].y, r[3].z}},
				{"rightArm",	{r[4].x, r[4].y, r[4].z}},
				{"rightForearm",{r[5].x, r[5].y, r[5].z}},
				{"leftThigh",	{r[6].x, r[6].y, r[6].z}},
				{"leftCalf",	{r[7].x, r[7].y, r[7].z}},
				{"rightThigh",	{r[8].x, r[8].y, r[8].z}},
				{"rightCalf",	{r[9].x, r[9].y, r[9].z}},
				{"time", 0.5}
			}
		}
	};
	ss << human << std::endl;
	return ss.str();
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
	this->_trunk.model.local.setScale(thickness * SIZE_TRUNK, lenght, Z_THICKNESS);
	this->positionMembers();
}

void			Human::setHeadSize(float thickness) {
	this->_head.model.local.setScale(SIZE_HEAD * thickness, SIZE_HEAD * thickness, Z_THICKNESS);
	this->_head.local.setPos(this->_trunk.model.local.getScale().x/2 - this->_head.model.local.getScale().x/2, this->_head.model.local.getScale().y, 0);
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

	this->positionMembers();
}

std::list<Obj3d*>	Human::getObjList() const { return (this->_objList); }
float				Human::getThickness() const { return this->_thickness; }
float				Human::getLenght() const { return this->_lenght; }

void			Human::scaleHuman() {
	// std::cout << __PRETTY_FUNCTION__ << std::endl;

	this->setTrunkSize(this->_thickness, this->_lenght);
	this->setHeadSize(this->_thickness);
	this->setMembersSize(this->_thickness, this->_lenght);

	this->updateMembersAnchor();
}

void			Human::positionMembers() {
	// std::cout << __PRETTY_FUNCTION__ << std::endl;

	// care, if you want to position a left member, its scale.x is negative! Use absolute value!
	this->_head.local.setPos(this->_trunk.model.local.getScale().x/2 - this->_head.model.local.getScale().x/2, this->_head.model.local.getScale().y, 0);
	this->_leftArm.local.setPos(0, 0, 0);
	this->_rightArm.local.setPos(this->_trunk.model.local.getScale().x, 0, 0);
	this->_leftForearm.local.setPos(Math::Vector3(0, -this->_leftArm.model.local.getScale().y, 0));
	this->_rightForearm.local.setPos(Math::Vector3(0, -this->_rightArm.model.local.getScale().y, 0));
	this->_leftThigh.local.setPos(abs(0 + this->_leftThigh.model.local.getScale().x), -this->_trunk.model.local.getScale().y, 0);
	this->_rightThigh.local.setPos(this->_trunk.model.local.getScale().x - this->_rightThigh.model.local.getScale().x, -this->_trunk.model.local.getScale().y, 0);
	this->_leftCalf.local.setPos(Math::Vector3(0, -this->_leftThigh.model.local.getScale().y, 0));
	this->_rightCalf.local.setPos(Math::Vector3(0, -this->_rightThigh.model.local.getScale().y, 0));
}

void			Human::updateMembersAnchor() {
	this->_head.updateAnchor();
	this->_trunk.updateAnchor();
	this->_leftArm.updateAnchor();
	this->_rightArm.updateAnchor();
	this->_leftForearm.updateAnchor();
	this->_rightForearm.updateAnchor();
	this->_leftThigh.updateAnchor();
	this->_rightThigh.updateAnchor();
	this->_leftCalf.updateAnchor();
	this->_rightCalf.updateAnchor();
}