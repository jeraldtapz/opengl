#include "rendering/render_buffer.h"

render_buffer::render_buffer(const GLenum internal_format, const unsigned int width, const unsigned int height)
{
	id = 0;
	this->width = width;
	this->height = height;
	
	glGenRenderbuffers(1, &id);
	bind();

	glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);

	unbind();
}

void render_buffer::reallocate(const GLenum internal_format, const unsigned int width, const unsigned int height)
{
	bind();
	this->width = width;
	this->height = height;
	glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
	unbind();
}


void render_buffer::bind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, id);
}

void render_buffer::unbind() 
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

unsigned render_buffer::get_id() const
{
	return id;
}

unsigned render_buffer::get_width() const
{
	return width;
}

unsigned render_buffer::get_height() const
{
	return height;
}





