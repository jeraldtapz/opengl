#pragma once
#include <memory>

#include "data/mesh.h"
#include "rendering/shader_program.h"

class renderer
{
protected:
	std::shared_ptr<mesh> mesh_ptr;
	virtual void setup();
	unsigned int vao{0};
	unsigned int vbo{0};
	unsigned int ebo{0};

	void draw_with_indices() const;
	void draw_with_raw_vertices() const;
	void draw_with_indices_instanced(const unsigned int count) const;
	void draw_with_raw_vertices_instanced(const unsigned int count) const;
	
public:

	renderer();
	explicit renderer(std::shared_ptr<mesh>);
	virtual void draw(const shader_program &program) const;
	void draw_instanced(const shader_program& program, const unsigned int count) const;
	void draw_cube_map(const shader_program& program) const;
	void deallocate() const;
	virtual ~renderer();
	
};
