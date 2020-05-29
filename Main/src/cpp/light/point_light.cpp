#include "light/point_light.h"

point_light::point_light(const float linear, const float quadratic)
{
	this->linear = linear;
	this->quadratic = quadratic;
	radius = glm::sqrt(1.0f / (quadratic * 0.01f));
}

point_light::point_light()
{
	linear = 0.0f;
	radius = _transform->scale().x;
	quadratic = 1.0f / (radius * radius * 0.01f);
}

void point_light::set_radius(const float r)
{
	radius = r;
	quadratic = 1.0f / (radius * radius * 0.01f);
}

void point_light::set_radius_from_scale()
{
	radius = _transform->scale().x;
	quadratic = 1.0f / (radius * radius * 0.01f);
}

void point_light::set_uniform_scale() const
{
	_transform->set_scale(glm::vec3(_transform->scale().x));
}
