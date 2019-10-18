#pragma once

#include "human.hpp"
#include "math.hpp"
#include "extremity.hpp"

class HumanEvolved : public Human
{
public:
							HumanEvolved(Obj3dBP & blueprint, Obj3dPG & program, float thickness = 1.0f, float lenght = 4.0f);
	// HumanEvolved(const Human & src);//FIX see .cpp
	// virtual HumanEvolved &	operator=(const HumanEvolved & src);
	virtual					~HumanEvolved();
	
	Extremity	_leftFoot;
	Extremity	_rightFoot;
	Extremity	_leftHand;
	Extremity	_rightHand;

protected:// so the children can call it
	virtual void		scaleHuman();		//children need to update their parent's part too, be sure to call this one too when scaling a child
	virtual void		positionMembers();	//children need to update their parent's part too, be sure to call this one too when scaling a child
private:
			void		addExtremityToList(Extremity & Extremity);
};