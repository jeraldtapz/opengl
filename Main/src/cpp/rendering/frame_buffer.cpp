#include "rendering/frame_buffer.h"
#include "glad/glad.h"


frame_buffer::frame_buffer()
{
	id = 0;
}

void frame_buffer::generate()
{
	glGenFramebuffers(1, &id);
}


void frame_buffer::attach_texture_2d(const unsigned int tex_id, const GLenum attachment, const bool is_multi_sampled)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, is_multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, tex_id, 0);
}

void frame_buffer::attach_texture(const texture& tex, const GLenum attachment)
{
	glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.get_id(), 0);
}


void frame_buffer::attach_render_buffer(const unsigned int id, const GLenum attachment, const bool is_multi_sampled)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, id);
}


void frame_buffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}



void frame_buffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned frame_buffer::get_id() const
{
	return id;
}

bool frame_buffer::validate()
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void frame_buffer::delete_buffer() const
{
	glDeleteFramebuffers(1, &id);
}





