#pragma once

#include "humanevolved.hpp"

#define NB_HAIR	 11

class SuperSaiyan1 : public HumanEvolved
{
public:
	SuperSaiyan1(Obj3dBP & blueprint, Obj3dPG & program, float thickness = 1.0f, float lenght = 4.0f);
	~SuperSaiyan1();
	void	setHairColor(uint8_t r, uint8_t g, uint8_t b);

	BodyPart	*hairs[NB_HAIR]; // array
protected:
	virtual void		scaleHuman();		//children need to update their parent's part too, be sure to call this one too when scaling a child
	virtual void		positionMembers();	//children need to update their parent's part too, be sure to call this one too when scaling a child
private:
			void		addHairsToList();
};
