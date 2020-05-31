#include "data/mesh.h"

bool check_if_transparent(std::vector<texture>& textures)
{
	for (auto && texture : textures)
	{
		if (texture.get_type() == texture_type::diffuse && texture.get_channels() == 4)
			return true;
	}

	return false;
}

mesh::mesh(const std::vector<vertex>& v)
{
	vertices = v;
	indices = std::vector<unsigned int>();
	textures = std::vector<texture>();

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;

	//is_transparent = check_if_transparent(textures);
}

mesh::mesh(const std::vector<vertex>& v, std::vector<texture>& t)
{
	vertices = v;
	indices = std::vector<unsigned int>();
	textures = t;

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
	//is_transparent = check_if_transparent(textures);
}

mesh::mesh(const std::vector<vertex>& v, std::vector<unsigned>& i)
{
	vertices = v;
	indices = i;
	textures = std::vector<texture>();

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
	//is_transparent = check_if_transparent(textures);
}

mesh::mesh(const std::vector<vertex>& v, std::vector<unsigned>& i, std::vector<texture>& t)
{
	vertices = v;
	indices = i;
	textures = t;

	is_indexed = false;
	cull_face = GL_BACK;
	should_cull_face = true;
	//is_transparent = check_if_transparent(textures);
}

void mesh::replace_textures(const std::vector<texture>& textures)
{
	this->textures.clear();
	this->textures.insert(this->textures.begin(), textures.begin(), textures.end());
	//is_transparent = check_if_transparent(this->textures);
}

void mesh::insert_texture(const texture& texture)
{
	this->textures.push_back(texture);
	//is_transparent = check_if_transparent(this->textures);
}

