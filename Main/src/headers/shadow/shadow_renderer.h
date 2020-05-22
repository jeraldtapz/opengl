#pragma once
#include <memory>
#include "data/mesh.h"
#include "rendering/shader_program.h"

class shadow_renderer
{
public:
	shadow_renderer();
	explicit shadow_renderer(std::shared_ptr<mesh> mesh);
	void draw(const shader_program& program) const;

protected:

	std::shared_ptr<mesh> mesh_ptr;
	void setup();
	unsigned int vao{ 0 };
	unsigned int vbo{ 0 };
	unsigned int ebo{ 0 };
	
	void draw_with_indices() const;
	void draw_with_raw_vertices() const;
	
};