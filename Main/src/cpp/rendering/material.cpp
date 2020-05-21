#include "rendering/material.h"
#include "rendering/color.h"

material::material()
{
	diffuse_tint = color::WHITE;
	specular_tint = color::WHITE;

	diffuse = nullptr;
	specular = nullptr;

	shininess = 32;
}

material::material(texture& diff, texture& spec)
{
	diffuse_tint = color::WHITE;
	specular_tint = color::WHITE;

	diffuse = std::make_shared<texture>(diff);
	specular = std::make_shared<texture>(spec);

	shininess = 32;
}

material::material(texture& diff, texture& spec, const glm::vec3 diff_tint, const glm::vec3 spec_tint)
{
	diffuse_tint = color(diff_tint);
	specular_tint = color(spec_tint);

	diffuse = std::make_shared<texture>(diff);
	specular = std::make_shared<texture>(spec);

	shininess = 32;
}

material::material(const glm::vec3 diff_tint, const glm::vec3 spec_tint)
{
	diffuse_tint = color(diff_tint);
	specular_tint = color(spec_tint);
	shininess = 32;
}

material::material(const color diff_tint, const color spec_tint)
{
	diffuse_tint = diff_tint;
	specular_tint = spec_tint;
	shininess = 32;
}

material::material(texture& diff, texture& spec, color diff_tint, color spec_tint) : material(diff_tint, spec_tint)
{
	diffuse = std::make_shared<texture>(diff);
	specular = std::make_shared<texture>(spec);
}
