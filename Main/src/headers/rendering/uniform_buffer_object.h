#pragma once
#include <glad/glad.h>

class uniform_buffer_object
{
public:
	uniform_buffer_object();
	uniform_buffer_object(const unsigned int size, const GLenum usage);

	void buffer_data(void* data) const ;
	void buffer_data_range(const unsigned int offset, const unsigned int size, void* data) const;
	void bind() const;
	static void unbind();
	unsigned int get_id() const;
	
private:
	GLenum usage {GL_STATIC_DRAW};
	unsigned int size{ 0 };
	unsigned int id{0};
	
};