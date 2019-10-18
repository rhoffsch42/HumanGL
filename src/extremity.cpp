
#include "extremity.hpp"

#define FINGER_SCALE	Math::Vector3(0.22f, 0.22f, 1.0f)
#define BASE_SCALE		Math::Vector3(1, 0.30f, 1.0f)
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

	this->finger1.model.local.setRot(0, -20, 0);
	this->finger2.model.local.setRot(0, -20, 0);
	this->finger3.model.local.setRot(0, -10, 0);
	this->finger4.model.local.setRot(0, 10, 0);
	this->finger5.model.local.setRot(0, 20, 0);

	this->base.model.setColor(BASE_COLOR);
	this->finger1.model.setColor(FINGER_COLOR);
	this->finger2.model.setColor(FINGER_COLOR);
	this->finger3.model.setColor(FINGER_COLOR);
	this->finger4.model.setColor(FINGER_COLOR);
	this->finger5.model.setColor(FINGER_COLOR);
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
	this->finger1.model.local.setPos(-this->finger1.model.local.getScale().x, 0, this->base.model.local.getScale().z/2);
	this->finger2.model.local.setPos(0, 0, this->base.model.local.getScale().z);
	this->finger3.model.local.setPos((this->finger1.model.local.getScale().x + SPACING * thickness) * 1, 0, this->base.model.local.getScale().z);
	this->finger4.model.local.setPos((this->finger1.model.local.getScale().x + SPACING * thickness) * 2, 0, this->base.model.local.getScale().z);
	this->finger5.model.local.setPos((this->finger1.model.local.getScale().x + SPACING * thickness) * 3, 0, this->base.model.local.getScale().z);
}
