#include "rendering/uniform_buffer_object.h"

uniform_buffer_object::uniform_buffer_object() = default;

uniform_buffer_object::uniform_buffer_object(const unsigned int size, const GLenum usage)
{
	this->usage = usage;
	this->size = size;
	glGenBuffers(1, &id);
	bind();
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
	unbind();
}

void uniform_buffer_object::buffer_data(void* data) const
{
	bind();
	glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
	unbind();
}

void uniform_buffer_object::buffer_data_range(const unsigned int offset, const unsigned int size, void* data) const
{
	bind();
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	unbind();
}



void uniform_buffer_object::bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
}

void uniform_buffer_object::unbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned uniform_buffer_object::get_id() const
{
	return id;
}




