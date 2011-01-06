#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "opengl.h"

#include <string>
#include <vector>

class Framebuffer
{
public:
	Framebuffer();
	Framebuffer(const std::string& prefix, size_t screen_width, size_t screen_height, bool depth_texture, size_t target_count);
	void add_float_target();

	void destroy();

	void bind() const;
	void bind(size_t target_count) const;

	std::string depth_texture() const;
	std::string texture(size_t target) const;

	size_t width() const;
	size_t height() const;

	size_t target_count() const;

	static void unbind();
private:
	std::string target_name(size_t target) const;

	std::string prefix;
	size_t resolution_x;
	size_t resolution_y;

	GLuint location;
	std::string depthtexture;
	std::vector<std::string> targets;
};

#endif

