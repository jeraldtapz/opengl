#include "rendering/frame_buffer.h"

const frame_buffer* frame_buffer::current_read {nullptr};
const frame_buffer* frame_buffer::current_draw {nullptr};

frame_buffer::frame_buffer()
{
	id = 0;
}

void frame_buffer::generate()
{
	glGenFramebuffers(1, &id);
}

void frame_buffer::attach_texture_2d_color(texture& tex, const GLenum attachment)
{
	if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_STENCIL_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT)
	{
		std::cout << "Texture color attachment failed" << std::endl;
		return;
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.get_is_multi_sampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, tex.get_id(), 0);

	color_attachments.insert(std::pair<GLenum, texture*>(attachment, &tex));
}

void frame_buffer::attach_texture_2d_depth(texture& tex, const GLenum attachment)
{
	if (attachment != GL_DEPTH_ATTACHMENT && attachment)
	{
		std::cout << "Texture depth attachment failed" << std::endl;
		return;
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.get_is_multi_sampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, tex.get_id(), 0);

	depth_attachment = std::make_shared<texture>(tex);
}

void frame_buffer::attach_texture_2d_stencil(texture& tex, const GLenum attachment)
{
	if (attachment != GL_STENCIL_ATTACHMENT)
	{
		std::cout << "Texture stencil attachment failed" << std::endl;
		return;
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.get_is_multi_sampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, tex.get_id(), 0);

	stencil_attachment = std::make_shared<texture>(tex);
}

void frame_buffer::attach_texture_2d_depth_stencil(texture& tex, const GLenum attachment)
{
	if (attachment != GL_DEPTH_STENCIL_ATTACHMENT)
	{
		std::cout << "Texture depth_stencil attachment failed" << std::endl;
		return;
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.get_is_multi_sampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, tex.get_id(), 0);

	depth_attachment = std::make_shared<texture>(tex);
	stencil_attachment = std::make_shared<texture>(tex);
}

void frame_buffer::attach_texture(texture& tex, const GLenum attachment)
{
	if(tex.get_type() != texture_type::cube)
	{
		std::cout << "Provided texture is not of texture_type cube" << std::endl;
		return;
	}
	
	if (attachment == GL_DEPTH_ATTACHMENT)
		depth_attachment = std::make_shared<texture>(tex);
	else if (attachment == GL_STENCIL_ATTACHMENT)
		stencil_attachment = std::make_shared<texture>(tex);
	else if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
	{
		depth_attachment = std::make_shared<texture>(tex);
		stencil_attachment = std::make_shared<texture>(tex);
	}
	else
		color_attachments.insert(std::pair<GLenum, texture*>(attachment, &tex));
	
	glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.get_id(), 0);
}

void frame_buffer::attach_render_buffer(render_buffer& rbo, const GLenum attachment)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo.get_id());
	this->depth_stencil_attachment = std::make_shared<render_buffer>(rbo);

}

void frame_buffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	current_draw = this;
	current_read = this;
}

void frame_buffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	current_draw = nullptr;
	current_read = nullptr;
}

unsigned frame_buffer::get_id() const
{
	return id;
}

unsigned frame_buffer::get_color_attachment(const GLenum attachment) const
{
	const std::map<unsigned, texture*>::const_iterator it = color_attachments.find(attachment);

	if (it != color_attachments.end())
		return it->second->get_id();

	return -1;
}

texture* frame_buffer::get_color_attachment_tex(const GLenum attachment) const
{
	const std::map<unsigned, texture*>::const_iterator it = color_attachments.find(attachment);

	if (it != color_attachments.end())
		return it->second;

	return nullptr;
}


unsigned frame_buffer::get_depth_attachment() const
{
	if (depth_attachment == nullptr)
		return -1;
	
	return depth_attachment->get_id();
}

std::shared_ptr<texture> frame_buffer::get_depth_attachment_tex() const
{
	return depth_attachment;
}

std::shared_ptr<texture> frame_buffer::get_stencil_attachment_tex() const
{
	return stencil_attachment;
}

unsigned frame_buffer::get_stencil_attachment() const
{
	if (stencil_attachment == nullptr)
		return -1;

	return stencil_attachment->get_id();
}


bool frame_buffer::validate()
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void frame_buffer::delete_buffer() const
{
	glDeleteFramebuffers(1, &id);
}


void frame_buffer::set_draw_buffer(const GLenum mode)
{
	if (current_draw != this)
	{
		std::cout << "Unable to set draw buffer, instance is not bound to GL_DRAW_FRAMEBUFFER" << std::endl;
		return;
	}
	glDrawBuffer(mode);
}

void frame_buffer::set_read_buffer(const GLenum mode)
{
	if (current_read != this)
	{
		std::cout << "Unable to set read buffer, instance is not bound to GL_READ_FRAMEBUFFER" << std::endl;
		return;
	}
	glReadBuffer(mode);
}

void frame_buffer::set_draw_buffers(const unsigned int count, const GLenum* attachments)
{
	if (current_draw != this)
	{
		std::cout << "Unable to set draw buffer, instance is not bound" << std::endl;
		return;
	}

	std::vector<GLenum> attachments_v;

	for (auto &color_attachment : color_attachments)
	{
		attachments_v.push_back(color_attachment.first);
	}
	
	glDrawBuffers(count, attachments == nullptr ? &attachments_v[0] : attachments);
}

void frame_buffer::bind_draw() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
}

void frame_buffer::bind_read() const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
}


#pragma region Clearing Buffers

void frame_buffer::clear_color_buffer()
{
	const color col = color(0.15f, 0.15f, 0.15f, 1.0f);
	glClearColor(col.r, col.g, col.b, col.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void frame_buffer::clear_depth_buffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void frame_buffer::clear_stencil_buffer()
{
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void frame_buffer::clear_frame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

#pragma endregion

#pragma region Depth

void frame_buffer::enable_depth_testing()
{
	glEnable(GL_DEPTH_TEST);
}

void frame_buffer::disable_depth_testing()
{
	glDisable(GL_DEPTH_TEST);
}

void frame_buffer::set_depth_testing(const bool flag)
{
	if (flag)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void frame_buffer::set_depth_writing(const bool flag)
{
	glDepthMask(flag ? GL_TRUE : GL_FALSE);
}

#pragma endregion

#pragma region Stencil

void frame_buffer::enable_stencil_testing()
{
	glEnable(GL_STENCIL_TEST);
}

void frame_buffer::disable_stencil_testing()
{
	glDisable(GL_STENCIL_TEST);
}

void frame_buffer::set_stencil_testing(const bool flag)
{
	if (flag)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void frame_buffer::set_stencil_writing(const bool flag)
{
	glStencilMask(flag ? 0xFF : 0x00);
}

void frame_buffer::set_stencil_func(const GLenum func, const GLint ref, const GLuint mask)
{
	glStencilFunc(func, ref, mask);
}

void frame_buffer::set_stencil_func_sep(const GLenum face, const GLenum func, const GLint ref, const GLuint mask)
{
	glStencilFuncSeparate(face, func, ref, mask);
}

void frame_buffer::set_stencil_op(const GLenum s_fail, const GLenum dp_fail, const GLenum dp_pass)
{
	glStencilOp(s_fail, dp_fail, dp_pass);
}

void frame_buffer::set_stencil_op_sep(const GLenum face, const GLenum s_fail, const GLenum dp_fail, const GLenum dp_pass)
{
	glStencilOpSeparate(face, s_fail, dp_fail, dp_pass);
}



#pragma endregion