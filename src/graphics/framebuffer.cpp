#include "framebuffer.h"
#include "texturehandler.h"

#include <sstream>
#include <stdexcept>
#include <cassert>
#include <iomanip>

enum { MAX_TARGETS = 4 };

Framebuffer::Framebuffer():
	resolution_x(0),
	resolution_y(0),
	location(0)
{
}

Framebuffer::Framebuffer(const std::string& prefixx, size_t screen_width, size_t screen_height, bool depth_texture, size_t target_count):
	prefix(prefixx),
	resolution_x(screen_width),
	resolution_y(screen_height),
	location(0)
{
	assert(target_count <= MAX_TARGETS);
	targets.reserve(target_count);

	glGenFramebuffers(1, &location);
	glBindFramebuffer(GL_FRAMEBUFFER, location);

	for(size_t i = 0; i < target_count; ++i)
	{
		targets.push_back(target_name(i));
		TextureHandler::getSingleton().createTexture(targets.back(), resolution_x, resolution_y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID(targets.back()), 0);
	}
	if(!target_count)
	{
		glDrawBuffer(GL_NONE);
	}

	if(depth_texture)
	{
		depthtexture = prefix + "_depthtexture";
		TextureHandler::getSingleton().createDepthTexture(depthtexture, resolution_x, resolution_y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID(depthtexture), 0);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::stringstream ss;
		ss << "Failed to create FBO: '" << prefix << "', error: 0x" << std::hex << status;
		throw std::runtime_error(ss.str());
	}
}

void Framebuffer::destroy()
{
	glDeleteFramebuffers(1, &location);
	for(size_t i = 0; i < targets.size(); ++i)
	{
		TextureHandler::getSingleton().deleteTexture(targets[i]);
	}
	if(!depthtexture.empty())
	{
		TextureHandler::getSingleton().deleteTexture(depthtexture);
	}
}

void Framebuffer::bind() const
{
	bind(targets.size());
}

void Framebuffer::bind(size_t target_count) const
{
	assert(target_count <= targets.size());
	glBindFramebuffer(GL_FRAMEBUFFER, location);
	bind_helper(target_count);
}

void Framebuffer::bind_for_reading() const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, location);
}

void Framebuffer::bind_for_writing() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, location);
	bind_helper(targets.size());
}

void Framebuffer::bind_helper(size_t target_count) const
{
	if(targets.empty())
	{
		glDrawBuffer(GL_NONE);
	}
	else
	{
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(target_count, buffers);
	}
	glViewport(0, 0, resolution_x, resolution_y);
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::string Framebuffer::depth_texture() const
{
	assert(!depthtexture.empty());

	return depthtexture;
}

std::string Framebuffer::texture(size_t target) const
{
	assert(target < targets.size());
	return targets[target];
}

std::string Framebuffer::target_name(size_t target) const
{
	std::stringstream ss;
	ss << prefix << "_texture" << target;
	return ss.str();
}

size_t Framebuffer::width() const
{
	return resolution_x;
}

size_t Framebuffer::height() const
{
	return resolution_y;
}

void Framebuffer::add_float_target()
{
	assert(targets.size() < MAX_TARGETS);
	size_t target = targets.size();
	targets.push_back(target_name(target));
	TextureHandler::getSingleton().createFloatTexture(targets.back(), resolution_x, resolution_y);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + target, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID(targets.back()), 0);
}

size_t Framebuffer::target_count() const
{
	return targets.size();
}

