#pragma once

#include "gamemanager.hpp"
#include "framebuffer.hpp"
#include "animationbh.hpp"
#include "uipanel.hpp"
class UIPanel;
//tmp
#include "fps.hpp"
#include "obj3d.hpp"

#define MAX_ANIM 6 // 5 animations + 1 for nullptr (pause)

class HumanManager : public GameManager {
public:
	HumanManager() : GameManager() {
		this->defaultFps = nullptr;
		this->lmbPressed = false;
		this->obj3dList = nullptr;

		this->human = nullptr;
		this->framebuffer = nullptr;
		this->framebufferUI = nullptr;
		this->framebufferUI2 = nullptr;
		this->framebufferUI3 = nullptr;
		this->framebufferUI4 = nullptr;
		this->framebufferUI5 = nullptr;
		this->uiPalette = nullptr;
		this->uiLength = nullptr;
		this->uiFakeRaycast = nullptr;
		this->uiGlobal = nullptr;
		this->uiThickness = nullptr;
		this->uiAnimButtons = nullptr;
		this->currentAnimation = nullptr;
		this->animationPaused = false;
	}
	~HumanManager() {
	}
	Fps *			defaultFps;
	bool			lmbPressed;
	list<Obj3d*> *	obj3dList;
	//above are for GameManager

	Human *			human;
	FrameBuffer *	framebuffer;
	FrameBuffer *	framebufferUI;
	FrameBuffer *	framebufferUI2;
	FrameBuffer *	framebufferUI3;
	FrameBuffer *	framebufferUI4;
	FrameBuffer *	framebufferUI5;
	UIPanel *		uiPalette;
	UIPanel *		uiLength;
	UIPanel *		uiFakeRaycast;
	UIPanel *		uiGlobal;
	UIPanel *		uiThickness;
	UIPanel *		uiAnimButtons;
	AnimationHumanBH *	animations[MAX_ANIM];
	AnimationHumanBH *	currentAnimation;
	bool				animationPaused;
private:
};
