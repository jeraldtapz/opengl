#pragma once
#include <memory>
#include <glm/vec3.hpp>


#include "color.h"
#include "texture.h"


class material
{
public:
	std::shared_ptr<texture> diffuse;
	std::shared_ptr<texture> specular;

	color diffuse_tint{};
	color specular_tint{};

	float shininess;

	material();
	material(texture &diff, texture &spec);
	material(texture& diff, texture& spec, glm::vec3 diff_tint, glm::vec3 spec_tint);
	material(texture& diff, texture& spec, color diff_tint, color spec_tint);
	material(glm::vec3 diff_tint, glm::vec3 spec_tint);
	material(color diff_tint, color spec_tint);
};
