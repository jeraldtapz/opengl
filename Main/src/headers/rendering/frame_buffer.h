#pragma once
#include <glad/glad.h>

class frame_buffer
{
public:
	frame_buffer();
	
	void bind() const;
	static void attach_texture_2d(const unsigned int tex_id, const GLenum attachment, const bool is_multi_sampled);
	static void attach_render_buffer(const unsigned int id, const GLenum attachment, const bool is_multi_sampled);
	
	void delete_buffer() const;
	unsigned int get_id() const;

	static void unbind();
	static bool validate();
	

private:
	unsigned int id;
};
