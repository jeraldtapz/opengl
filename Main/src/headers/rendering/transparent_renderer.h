#pragma once
#include "blend_factor.h"
#include "renderer.h"

class transparent_renderer final : public renderer
{
public:
	transparent_renderer();
	explicit transparent_renderer(const std::shared_ptr<mesh>&);
	blend_factor src_factor;
	blend_factor dst_factor;

	static GLenum to_gl_enum(blend_factor factor);
	void draw(const shader_program& program) const override;
};
