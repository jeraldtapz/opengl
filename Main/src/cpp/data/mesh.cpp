#include "data/mesh.h"

mesh::mesh(const std::vector<vertex>& v)
{
	vertices = v;
	indices = std::vector<unsigned int>();
	textures = std::vector<texture>();

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
}

mesh::mesh(const std::vector<vertex>& v, std::vector<texture>& t)
{
	vertices = v;
	indices = std::vector<unsigned int>();
	textures = t;

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
}

mesh::mesh(const std::vector<vertex>& v, std::vector<unsigned>& i)
{
	vertices = v;
	indices = i;
	textures = std::vector<texture>();

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
}

mesh::mesh(const std::vector<vertex>& v, std::vector<unsigned>& i, std::vector<texture>& t)
{
	vertices = v;
	indices = i;
	textures = t;

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
}