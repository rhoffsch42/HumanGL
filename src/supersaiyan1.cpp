#include "supersaiyan1.hpp"


/*
	https://en.cppreference.com/w/cpp/language/list_initialization
*/

SuperSaiyan1::SuperSaiyan1(Obj3dBP & blueprint, Obj3dPG & program, float thickness = 1.0f, float lenght = 4.0f)
: HumanEvolved(blueprint, program)
{
/*
	//FIX faire un constructor par default en private (Obj3d), + friend SuperSaiyan1;
*/
	for (int i = 0; i < NB_HAIR; i++) {
			this->hairs[i] = BodyPart(blueprint, program);
			BodyPart* h = &this->hairs[i];
			h->setParent(&this->_head);
			float   val = cosf(Math::toRadian(i * 10)) * 10;
			float   coef = 1.0f;
			h->local.translate(float(i) / coef, val / coef, this->_head.model.local.getScale().z);
			// h->local.rotate(Math::Rotation(i * 5, i * 5, i * 5));

			this->_objList.push_back(&h->model);
	}
}

SuperSaiyan1::~SuperSaiyan1() {
}
