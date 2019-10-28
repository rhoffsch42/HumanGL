#include "supersaiyan1.hpp"

#define HAIR_THICKNESS	0.1f
#define HAIR_LEN		0.5f
#define HAIR_ANGLE		5.0f
#define HAIR_COLOR		1, 0, 0 //yellow

/*
	https://en.cppreference.com/w/cpp/language/list_initialization
*/
SuperSaiyan1::SuperSaiyan1(Obj3dBP & blueprint, Obj3dPG & program, float thickness, float lenght)
: HumanEvolved(blueprint, program, thickness, lenght)
{
	Math::Vector3	headScale = this->_head.model.local.getScale();
	for (int i = 0; i < NB_HAIR; i++) {
			this->hairs[i] = new BodyPart(blueprint, program);
			BodyPart* h = this->hairs[i];
			h->model.setColor(HAIR_COLOR);
			h->setParent(&this->_head);
			h->local.rotate(Math::Rotation(90, 0, (i - (NB_HAIR/2)) * HAIR_ANGLE));

			this->_objList.push_back(&h->model);
	}
	this->scaleHuman();//calls positionMembers() too
}

SuperSaiyan1::~SuperSaiyan1() {
	for (int i = 0; i < NB_HAIR; i++) {
		delete this->hairs[i];
	}
}

void		SuperSaiyan1::scaleHuman() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	HumanEvolved::scaleHuman();//the call to the mother class function
	Math::Vector3	headScale = this->_head.model.local.getScale();
	for (int i = 0; i < NB_HAIR; i++) {
			BodyPart* h = this->hairs[i];
			h->model.local.setScale(HAIR_THICKNESS * this->_thickness, HAIR_LEN * this->_lenght, HAIR_THICKNESS * this->_thickness);
	}
	this->positionMembers();
}
void		SuperSaiyan1::positionMembers() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	HumanEvolved::positionMembers();//the call to the mother class function
	
	Math::Vector3	headScale = this->_head.model.local.getScale();
	float offsetSide = 0.01f * this->_thickness;
	float p = headScale.x / NB_HAIR;
	p += (p - this->hairs[0]->model.local.getScale().x) / NB_HAIR;
	p += offsetSide*2 / NB_HAIR;

	for (int i = 0; i < NB_HAIR; i++) {
			this->hairs[i]->local.setPos(float(i) * p - offsetSide, offsetSide, 1.0f + offsetSide*5);
			std::cout << float(i) * p - 0 << std::endl;
	}
}