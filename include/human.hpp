#pragma once

#include "obj3dBP.hpp"
#include "obj3dPG.hpp"
#include "obj3d.hpp"

#include <list>

// too simple to be standalone (for now)
class BodyPart : public Object
{
public:
	BodyPart(Obj3dBP & blueprint, Obj3dPG & program) : model(blueprint, program) {
		this->model.setParent(this);
	}
	Obj3d	model;
};

class Human : public Object
{
public:
					Human(Obj3dBP & blueprint, Obj3dPG & program, float thickness = 1.0f, float lenght = 4.0f);
					// Human(const Human & src);//FIX see .cpp
	virtual Human &	operator=(const Human & src);
	virtual			~Human();

	void			setThickness(float thickness);	// override all scales
	void			setLenght(float lenght);		// override all scales
	void			setTrunkSize(float thickness, float lenght);
	void			setHeadSize(float thickness);
	void			setMembersSize(float thickness, float lenght);

	std::list<Obj3d*>	getObjList() const;

	BodyPart	_head;
	BodyPart	_trunk;
	BodyPart	_leftArm;
	BodyPart	_rightArm;
	BodyPart	_leftForearm;
	BodyPart	_rightForearm;
	BodyPart	_leftThigh;
	BodyPart	_rightThigh;
	BodyPart	_leftCalf;
	BodyPart	_rightCalf;

protected:
	float		_thickness;
	float		_lenght;

	std::list<Obj3d*>	_objList;

	virtual void		scaleHuman();		//children need to update their parent's part too, be sure to call this one too when scaling a child
	virtual void		positionMembers();	//children need to update their parent's part too, be sure to call this one too when scaling a child
};
