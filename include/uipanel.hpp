#pragma once

#include "humanmanager.hpp"
class HumanManager;
#include "texture.hpp"
#include "framebuffer.hpp"


class UIPanel {
public:
	UIPanel();
	UIPanel(Texture * tex);
	~UIPanel();
	
			bool	isOnPanel(int glX, int glY) const;
	virtual	void	action(int glX, int glY, HumanManager * manager) = 0;

	void		setTexture(Texture * tex);
	//undefined behaviors if positions are fucked up (neg pos, pos2 < pos, neg size)
	void		setPos(int glX, int glY);
	void		setPos2(int glX, int glY);
	void		setSize(int width, int height);

	int			getWidth() const;
	int			getHeight() const;
	GLuint		getFbo() const;
	Texture *	getTexture() const;

	bool		isClickable;
protected:
	GLuint		_fbo;
	Texture *	_texture;
	int			_posX;
	int			_posY;
	int			_width;
	int			_height;
	int			_posX2;
	int			_posY2;

	friend void	init(UIPanel * panel);
	friend void blitToWindow(HumanManager * manager, FrameBuffer * readFramebuffer, GLenum attachmentPoint, UIPanel *panel);
};

#include "obj3d.hpp"
class UIMemberColor : public UIPanel {
public:
	UIMemberColor(Texture * tex);
	void	action(int glX, int glY, HumanManager * manager);
};

class UIAnimation : public UIPanel {
public:
	UIAnimation(Texture * tex);
	void	action(int glX, int glY, HumanManager * manager);
};

class UIMemberLength : public UIPanel {
public:
	UIMemberLength(Texture * tex);
	void	action(int glX, int glY, HumanManager * manager);
};

class UIGlobalLength : public UIPanel {
public:
	UIGlobalLength(Texture * tex);
	void	action(int glX, int glY, HumanManager * manager);
};

class UIThickness : public UIPanel {
public:
	UIThickness(Texture * tex);
	void	action(int glX, int glY, HumanManager * manager);
};

class UIFakeRaycast : public UIPanel {
public:
	UIFakeRaycast(Texture * tex);
	void	action(int glX, int glY, HumanManager * manager);
};

/* sample
class UIAnimation : public UIPanel {
public:
	UIAnimation(Texture * tex) : UIPanel(tex) {}
	void	action(int glX, int glY, HumanManager * manager) {
		
	}
};
*/