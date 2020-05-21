#include "rendering/transparent_renderer.h"

transparent_renderer::transparent_renderer(const std::shared_ptr<mesh>& m) : renderer(m)
{
	src_factor = blend_factor::src_alpha;
	dst_factor = blend_factor::one_minus_src_alpha;
}

transparent_renderer::transparent_renderer() : renderer()
{
	src_factor = blend_factor::src_alpha;
	dst_factor = blend_factor::one_minus_src_alpha;
}

GLenum transparent_renderer::to_gl_enum(blend_factor factor)
{
	switch (factor)
	{
	case blend_factor::zero: return GL_ZERO;
	case blend_factor::one: return GL_ONE;
	case blend_factor::src_color: return GL_SRC_COLOR;
	case blend_factor::one_minus_src_color: return GL_ONE_MINUS_SRC_COLOR;
	case blend_factor::dst_color: return GL_DST_COLOR;
	case blend_factor::one_minus_dst_color: return GL_ONE_MINUS_DST_COLOR;
	case blend_factor::src_alpha: return GL_SRC_ALPHA;
	case blend_factor::one_minus_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
	case blend_factor::dst_alpha: return GL_DST_ALPHA;
	case blend_factor::one_minus_dst_alpha: return GL_ONE_MINUS_DST_ALPHA;
	case blend_factor::constant_color: return GL_CONSTANT_COLOR;
	case blend_factor::one_minus_constant_color: return GL_ONE_MINUS_CONSTANT_COLOR;
	case blend_factor::constant_alpha: return GL_CONSTANT_ALPHA;
	case blend_factor::one_minus_constant_alpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
	default: return GL_ONE;
	}
}

void transparent_renderer::draw(const shader_program& program) const
{
	glEnable(GL_BLEND);
	glBlendFunc(to_gl_enum(src_factor), to_gl_enum(dst_factor));
	renderer::draw(program);
	glDisable(GL_BLEND);
}