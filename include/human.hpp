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
		this->updateAnchor();
	}
	Obj3d			model;
	Math::Vector3	anchor;
	void			updateAnchor(void) {
		// std::cout << __PRETTY_FUNCTION__ << std::endl;
		BodyPart *	parent = dynamic_cast<BodyPart*>(this->model.getParent());
		Math::Vector3	ps = parent->model.local.getScale();
		Math::Vector3	s = this->model.local.getScale();
		// this->anchor = Math::Vector3(ps.x, 0, 0);
		this->anchor = this->local.getPos();//care, if the rotation of the object is not 0,0,0 the pos is not what we're looking for
		this->anchor.add(Math::Vector3(s.x/2, -(0), s.z/2));//negative y because the cube has negative y vertices
	}
	void			rotateMember(Math::Rotation rot) {
		this->local.rotate(rot);
		// this->local.rotateAround(this->anchor, rot);//FIX not working (SimpleGL)
	}
};

class Human : public Object
{
public:
					Human(Obj3dBP & blueprint, Obj3dPG & program, float thickness = 1.0f, float lenght = 4.0f);
					// Human(const Human & src);//FIX see .cpp
	virtual Human &	operator=(const Human & src);
	virtual			~Human();
	std::string		getKeyFrame() const;


	void			setThickness(float thickness);	// override all scales
	void			setLenght(float lenght);		// override all scales
	void			setTrunkSize(float thickness, float lenght);
	void			setHeadSize(float thickness);
	void			setMembersSize(float thickness, float lenght);

	float				getThickness() const;
	float				getLenght() const;
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

	virtual void		scaleHuman();			//children need to update their parent's part too, be sure to call this one too when scaling a child
	virtual void		positionMembers();		//children need to update their parent's part too, be sure to call this one too when scaling a child
	virtual void		updateMembersAnchor();	//children need to update their parent's part too, be sure to call this one too when scaling a child

protected:
	float		_thickness;
	float		_lenght;

	std::list<Obj3d*>	_objList;

};
