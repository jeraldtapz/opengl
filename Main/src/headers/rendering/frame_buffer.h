#pragma once
#include <glad/glad.h>
#include "rendering/texture.h"
#include <iostream>
#include <map>
#include <vector>


#include "rendering/color.h"
#include "render_buffer.h"

class frame_buffer
{
public:
	frame_buffer();

	void generate();
	void bind() const;
	void attach_texture_2d_color(texture& tex, const GLenum attachment);
	void attach_texture_2d_depth(texture& tex, const GLenum attachment);
	void attach_texture_2d_stencil(texture& tex, const GLenum attachment);
	void attach_texture_2d_depth_stencil(texture& tex, const GLenum attachment);
	void attach_texture(texture& tex, const GLenum attachment);
	void attach_render_buffer(render_buffer& rbo, const GLenum attachment);

	void set_draw_buffer(GLenum mode);
	void set_read_buffer(GLenum mode);
	void set_draw_buffers(unsigned int count, const GLenum* attachments);
	void bind_draw() const;
	void bind_read() const;

	
	unsigned int get_color_attachment(const GLenum attachment) const;
	texture* get_color_attachment_tex(const GLenum attachment) const;
	
	unsigned int get_depth_attachment() const;
	std::shared_ptr<texture> get_depth_attachment_tex() const;
	
	unsigned int get_stencil_attachment() const;
	std::shared_ptr<texture> get_stencil_attachment_tex() const;
	
	
	void delete_buffer() const;
	unsigned int get_id() const;

	static void unbind();
	static bool validate();

	static void clear_color_buffer();
	static void clear_depth_buffer();
	static void clear_stencil_buffer();
	static void clear_frame();
	
	
	static void enable_depth_testing();
	static void disable_depth_testing();
	static void set_depth_testing(const bool flag);
	static void set_depth_writing(const bool flag);

	static void enable_stencil_testing();
	static void disable_stencil_testing();
	static void set_stencil_testing(const bool flag);
	static void set_stencil_writing(const bool flag);
	static void set_stencil_func(GLenum func, GLint ref, GLuint mask);
	static void set_stencil_func_sep(GLenum face, GLenum func, GLint ref, GLuint mask);
	static void set_stencil_op(GLenum s_fail, GLenum dp_fail, GLenum dp_pass);
	static void set_stencil_op_sep(GLenum face, GLenum s_fail, GLenum dp_fail, GLenum dp_pass);
	

private:
	static const frame_buffer* current_draw;
	static const frame_buffer* current_read;
	
	unsigned int id;
	std::map<GLenum, texture*> color_attachments;
	std::shared_ptr<texture> depth_attachment;
	std::shared_ptr<texture> stencil_attachment;
	std::shared_ptr <render_buffer> depth_stencil_attachment;
};
