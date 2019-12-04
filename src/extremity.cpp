
#include "extremity.hpp"

#define FINGER_SCALE	Math::Vector3(0.22f, 1.0f, 0.22f)
#define BASE_SCALE		Math::Vector3(1.0f, 1.0f, 0.30f)
#define SPACING			0.05f
#define FINGER_COLOR	255, 255, 0
#define BASE_COLOR		255, 0, 0

Extremity::Extremity(Obj3dBP & blueprint, Obj3dPG & program)
: base(blueprint, program), \
finger1(blueprint, program), \
finger2(blueprint, program), \
finger3(blueprint, program), \
finger4(blueprint, program), \
finger5(blueprint, program)
{
	this->base.setParent(this);
	this->finger1.setParent(&base);
	this->finger2.setParent(&base);
	this->finger3.setParent(&base);
	this->finger4.setParent(&base);
	this->finger5.setParent(&base);

	this->finger1.model.local.setRot(0, 0, -20);
	this->finger2.model.local.setRot(0, 0, -20);
	this->finger3.model.local.setRot(0, 0, -10);
	this->finger4.model.local.setRot(0, 0, 10);
	this->finger5.model.local.setRot(0, 0, 20);

	this->base.model.setColor(BASE_COLOR);
	this->finger1.model.setColor(FINGER_COLOR);
	this->finger2.model.setColor(FINGER_COLOR);
	this->finger3.model.setColor(FINGER_COLOR);
	this->finger4.model.setColor(FINGER_COLOR);
	this->finger5.model.setColor(FINGER_COLOR);
}

Extremity::Extremity(const Extremity & src)
: base(src.base.model.getBlueprint(), src.base.model.getProgram()), \
finger1(src.finger1.model.getBlueprint(), src.finger1.model.getProgram()), \
finger2(src.finger2.model.getBlueprint(), src.finger2.model.getProgram()), \
finger3(src.finger3.model.getBlueprint(), src.finger3.model.getProgram()), \
finger4(src.finger4.model.getBlueprint(), src.finger4.model.getProgram()), \
finger5(src.finger5.model.getBlueprint(), src.finger5.model.getProgram())
{
	*this = src;
}
Extremity &		Extremity::operator=(const Extremity & src) {
	this->base = src.base;
	this->finger1 = src.finger1;
	this->finger2 = src.finger2;
	this->finger3 = src.finger3;
	this->finger4 = src.finger4;
	this->finger5 = src.finger5;
	return *this;
}

Extremity::~Extremity() {
}

void	Extremity::scale(float thickness) {
	Math::Vector3	b = BASE_SCALE;
	Math::Vector3 s = FINGER_SCALE;
	b.mult(thickness);
	s.mult(thickness);
	this->base.model.local.setScale(b.x, b.y, b.z);
	this->finger1.model.local.setScale(s.x, s.y, s.z);
	this->finger2.model.local.setScale(s.x, s.y, s.z);
	this->finger3.model.local.setScale(s.x, s.y, s.z);
	this->finger4.model.local.setScale(s.x, s.y, s.z);
	this->finger5.model.local.setScale(s.x, s.y, s.z);
	this->position(thickness);
}

void	Extremity::position(float thickness) {
	this->finger1.model.local.setPos(-this->finger1.model.local.getScale().x, -this->base.model.local.getScale().y/2, 0);
	this->finger2.model.local.setPos(0, -this->base.model.local.getScale().y, 0);
	this->finger3.model.local.setPos((this->finger1.model.local.getScale().x + SPACING * thickness) * 1, -this->base.model.local.getScale().y, 0);
	this->finger4.model.local.setPos((this->finger1.model.local.getScale().x + SPACING * thickness) * 2, -this->base.model.local.getScale().y, 0);
	this->finger5.model.local.setPos((this->finger1.model.local.getScale().x + SPACING * thickness) * 3, -this->base.model.local.getScale().y, 0);
}
