#pragma once
#include "light.h"

class point_light: public light
{
public:
	float linear;
	float quadratic;
	float radius;

	point_light();
	point_light(float linear, float quadratic);

	void set_radius(float r);
	void set_radius_from_scale();
	void set_uniform_scale() const;
};
