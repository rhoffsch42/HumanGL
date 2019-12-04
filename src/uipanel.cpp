#include "uipanel.hpp"
#include "humangl.h"

#define DEFAULT_SIZE 100

void	init(UIPanel * panel) {
	panel->isClickable = true;
	panel->_texture = nullptr;

	panel->_posX = 0;
	panel->_posY = 0;
	panel->_width = DEFAULT_SIZE;
	panel->_height = DEFAULT_SIZE;
	panel->_posX2 = panel->_posX + panel->_width;
	panel->_posY2 = panel->_posY + panel->_height;
}

UIPanel::UIPanel() {
	glGenFramebuffers(1, &this->_fbo);
	init(this);
}

UIPanel::UIPanel(Texture * tex) {
	glGenFramebuffers(1, &this->_fbo);
	init(this);
	if (tex) {
		this->setTexture(tex);
		this->setSize(tex->getWidth(), tex->getHeight());
	}
}

UIPanel::UIPanel(const UIPanel & src) {
	*this = src;
}

UIPanel &	UIPanel::operator=(const UIPanel & src) {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	this->isClickable = src.isClickable;
	glGenFramebuffers(1, &this->_fbo);
	this->setTexture(src._texture);
	this->_posX = src._posX;
	this->_posY = src._posY;
	this->_width = src._width;
	this->_height = src._height;
	this->_posX2 = src._posX2;
	this->_posY2 = src._posY2;
	return (*this);
}


UIPanel::~UIPanel() {
	glDeleteFramebuffers(1, &this->_fbo);
}

bool		UIPanel::isOnPanel(int glX, int glY) const {
	return (glX >= this->_posX && glX <= this->_posX2 && glY >= this->_posY && glY <= this->_posY2);
}

void		UIPanel::setTexture(Texture * tex) {
	this->_texture = tex;
	if (tex) {
		// attach
		std::cout << "UIPanel : attach : " << this->_fbo << " | " << this->_texture->getId() << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, this->_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,	this->_texture->getId(), 0);// mipmap level: 0(base)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}


void		UIPanel::setPos(int glX, int glY) {
	this->_posX = glX;
	this->_posY = glY;
	this->_width = this->_posX2 - this->_posX;
	this->_height = this->_posY2 - this->_posY;
}
void		UIPanel::setPos2(int glX, int glY) {
	this->_posX2 = glX;
	this->_posY2 = glY;
	this->_width = this->_posX2 - this->_posX;
	this->_height = this->_posY2 - this->_posY;
}
void		UIPanel::setSize(int width, int height) {
	this->_width = width;
	this->_height = height;
	this->_posX2 = this->_posX + this->_width;
	this->_posY2 = this->_posY + this->_height;
}

int			UIPanel::getWidth() const { return this->_width; }
int			UIPanel::getHeight() const { return this->_height; }
GLuint		UIPanel::getFbo() const { return this->_fbo; };
Texture *	UIPanel::getTexture() const { return this->_texture; };

///////////////////////////////////////////////////////////////////////////
UIMemberColor::UIMemberColor(Texture * tex) : UIPanel(tex) {}
UIMemberColor::UIMemberColor(const UIPanel & src) : UIPanel(src) {}

void	UIMemberColor::action(int glX, int glY, HumanManager * manager) {
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	if (!manager->currentSelection) {
		std::cout << "Error : no member selected" << std::endl;
		return ;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLubyte data[4];//RGBA
	glReadPixels(glX, glY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);
	Obj3d *		o = dynamic_cast<Obj3d*>(manager->currentSelection);
	if (!o) { std::cout << __PRETTY_FUNCTION__ << " : Error : dymanic_cast failed" << std::endl; exit(2); }
	std::cout << "color: " << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << std::endl;
	o->setColor(data[0], data[1], data[2]);
}
///////////////////////////////////////////////////////////////////////////
#include "math.hpp"
#define MAX_ANIM 6 // 5 animations + 1 for nullptr (pause)
UIAnimation::UIAnimation(Texture * tex) : UIPanel(tex) {}
UIAnimation::UIAnimation(const UIPanel & src) : UIPanel(src) {}

void	UIAnimation::action(int glX, int glY, HumanManager * manager) {
	(void)glY;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	int index = (glX - this->_posX) / (this->_width / MAX_ANIM);
	if (index) {
		manager->animationPaused = false;
		Math::Vector3	pos = manager->human->_trunk.local.getPos();
		pos.y = FLOOR + manager->human->_trunk.model.local.getScale().y + manager->human->_rightCalf.model.local.getScale().y * 2;
		manager->human->_trunk.local.setPos(pos);
		manager->currentAnimation = manager->animations[index];
		manager->currentAnimation->reset();
	} else {
		manager->animationPaused = !manager->animationPaused;
	}
}
///////////////////////////////////////////////////////////////////////////
#include "human.hpp"
#include "humanevolved.hpp"
#include "supersaiyan1.hpp"
#define HUMAN			0
#define HUMAN_EVOLVED	1
#define SUPERSAIYAN1	2
UIMemberLength::UIMemberLength(Texture * tex) : UIPanel(tex) {}
UIMemberLength::UIMemberLength(const UIPanel & src) : UIPanel(src) {}

void	UIMemberLength::action(int glX, int glY, HumanManager * manager) {
	(void)glY;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	if (!manager->currentSelection) {
		std::cout << "Error : no member selected" << std::endl;
		return ;
	}
	float speed = 3.0f;
	if (glX - this->_posX <= this->_width / 2) { // reduce length
		if (manager->currentSelection->local.getScale().y < 1.0f) // minimum length
			return ;
		speed *= -1.0f;
	}
	manager->currentSelection->local.enlarge(0, speed * manager->defaultFps->getTick(), 0);
	
	//update members
	int i = HUMAN;
	HumanEvolved * he = dynamic_cast<HumanEvolved*>(manager->human);
	HumanEvolved * ss1 = dynamic_cast<SuperSaiyan1*>(manager->human);
	if (ss1)
		i = SUPERSAIYAN1;
	else if (he)
		i = HUMAN_EVOLVED;
	float t = manager->human->getThickness();
	switch (i) {
		case SUPERSAIYAN1:
			// nothing to do, it's only hairs
		case HUMAN_EVOLVED:
			he->_leftHand.position(t);
			he->_rightHand.position(t);
			he->_leftFoot.position(t);
			he->_rightFoot.position(t);
		case HUMAN:
			manager->human->positionMembers();
			manager->human->updateMembersAnchor();
	}
}
///////////////////////////////////////////////////////////////////////////
UIGlobalLength::UIGlobalLength(Texture * tex) : UIPanel(tex) {}
UIGlobalLength::UIGlobalLength(const UIPanel & src) : UIPanel(src) {}

void	UIGlobalLength::action(int glX, int glY, HumanManager * manager) {
	(void)glY;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	float speed = 3.0f;
	float l = manager->human->getLenght();
	if (glX - this->_posX <= this->_width / 2) { // reduce length
		speed *= -1.0f;
	}
	l += speed * manager->defaultFps->getTick();
	manager->human->setLenght(l);
	l = manager->human->_trunk.model.local.getScale().y + manager->human->_rightCalf.model.local.getScale().y * 2;
	manager->human->_trunk.local.setPos(0, FLOOR + l, 0);
}
///////////////////////////////////////////////////////////////////////////
UIThickness::UIThickness(Texture * tex) : UIPanel(tex) {}
UIThickness::UIThickness(const UIPanel & src) : UIPanel(src) {}

void	UIThickness::action(int glX, int glY, HumanManager * manager) {
	(void)glY;
	// std::cout << __PRETTY_FUNCTION__ << std::endl;
	float speed = 3.0f;
	float t = manager->human->getThickness();
	if (glX - this->_posX <= this->_width / 2) { // reduce thickness
		speed *= -1.0f;
	}
	t += speed * manager->defaultFps->getTick();
	manager->human->setThickness(t);
}
///////////////////////////////////////////////////////////////////////////
UIFakeRaycast::UIFakeRaycast(Texture * tex) : UIPanel(tex) {}
UIFakeRaycast::UIFakeRaycast(const UIPanel & src) : UIPanel(src) {}

void	UIFakeRaycast::action(int glX, int glY, HumanManager * manager) {
	(void)glX;
	(void)glY;
	(void)manager;
}