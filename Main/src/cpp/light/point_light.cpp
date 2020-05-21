#include "light/point_light.h"

point_light::point_light(const float linear, const float quadratic)
{
	this->linear = linear;
	this->quadratic = quadratic;
}

point_light::point_light()
{
	linear = 0.09f;
	quadratic = 0.032f;
}

