#pragma once
#include <vector>

#include "vertex.h"
#include "rendering/texture.h"

class mesh
{
	public:
	
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<texture> textures;

	bool is_transparent{ false };
	bool is_indexed{ true };
	bool should_cull_face{ true };
	GLenum cull_face;

	explicit mesh(const std::vector<vertex>&);
	mesh(const std::vector<vertex>&, std::vector<unsigned int>&);
	mesh(const std::vector<vertex>&, std::vector<texture>&);
	mesh(const std::vector<vertex>&, std::vector<unsigned int>& , std::vector<texture>&);

	void replace_textures(const std::vector<texture>& textures);
	void insert_texture(const texture& texture);

};
