#pragma once

#include "glfw.hpp"
#include <map>
/*
	http://www.songho.ca/opengl/gl_fbo.html
	MSAA https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
	https://alexandre-laurent.developpez.com/tutoriels/OpenGL/OpenGL-FBO/
*/

class FrameBuffer {
public:
	static string getFramebufferStatusInfos(GLenum status);//bind the FB

	FrameBuffer(int width, int height);
	~FrameBuffer();

	void	updateFramebufferStatus();

	int		getWidth() const;
	int		getHeight() const;

	//set all as private?
	GLuint	fbo;
	GLenum	fboStatus;
	GLuint	rbo;
	GLenum  rboFormat;
	GLuint	texId;
	GLenum  texFormat;

private:
	int		_width;
	int		_height;
};