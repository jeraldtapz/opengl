#pragma once
#include <glad/glad.h>

class render_buffer
{
public:
	render_buffer(GLenum internal_format, unsigned int width, unsigned int height);
	render_buffer(GLenum internal_format, unsigned int width, unsigned int height, unsigned int samples);

	void reallocate(GLenum internal_format, unsigned int width, unsigned int height);
	void bind() const;
	static void unbind();
	
	unsigned int get_id() const;
	unsigned int get_width() const;
	unsigned int get_height() const;

private:
	unsigned int width;
	unsigned int height;
	unsigned int id;
};