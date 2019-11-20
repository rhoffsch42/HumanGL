#include "framebuffer.hpp"

FrameBuffer::FrameBuffer(int width, int height) {
	this->rboFormat = GL_DEPTH_COMPONENT;
	this->texFormat = GL_RGBA;
	this->_width = width;
	this->_height = height;

	if (1) {//TODO check 0 < width & height < GL_MAX_RENDERBUFFER_SIZE
		//resize
	}

	// frame buffer
	glGenFramebuffers(1, &this->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);

	// render buffer
	glGenRenderbuffers(1, &this->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, this->rboFormat, this->_width, this->_height);

	// texture
	glGenTextures(1, &this->texId);
	glBindTexture(GL_TEXTURE_2D, this->texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->_width, this->_height, 0, this->texFormat, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach
	std::cout << "attach: " << this->fbo << " : " << this->rbo << " | " << this->texId << std::endl;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,	this->texId, 0);// mipmap level: 0(base)

	this->fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);//0 = detach renderbuffer
	glDeleteRenderbuffers(1, &this->rbo);
	glDeleteFramebuffers(1, &this->fbo);
}
void	FrameBuffer::updateFramebufferStatus() {
	this->fboStatus = glCheckFramebufferStatus(this->fbo);
}

map<GLenum, string>	statusStr;
static void		initStatus() {
	statusStr[GL_FRAMEBUFFER_COMPLETE] = "GL_FRAMEBUFFER_COMPLETE";
	statusStr[GL_FRAMEBUFFER_UNDEFINED] = "GL_FRAMEBUFFER_UNDEFINED";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT] = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT] = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER] = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER] = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
	statusStr[GL_FRAMEBUFFER_UNSUPPORTED] = "GL_FRAMEBUFFER_UNSUPPORTED";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE] = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE] = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
	statusStr[GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS] = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
}

string	FrameBuffer::getFramebufferStatusInfos(GLenum status)//bind the FB
{
	if (!statusStr.size())
		initStatus();
	string s = statusStr[status];
	if (s.length())
		return s;
	else
		return "Framebuffer unknow status";
}

int		FrameBuffer::getWidth() const { return this->_width; }
int		FrameBuffer::getHeight() const { return this->_height; }