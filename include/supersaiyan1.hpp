#pragma once

#include "humanevolved.hpp"

#define NB_HAIR	 10

class SuperSaiyan1 : protected HumanEvolved
{
public:
	SuperSaiyan1(Obj3dBP & blueprint, Obj3dPG & program, float thickness = 1.0f, float lenght = 4.0f);
	~SuperSaiyan1();

	// BodyPart	hairs[NB_HAIR];//FIX see .cpp
protected:
	virtual void		scaleHuman();		//children need to update their parent's part too, be sure to call this one too when scaling a child
	virtual void		positionMembers();	//children need to update their parent's part too, be sure to call this one too when scaling a child
private:
			void		addHairsToList();
};
