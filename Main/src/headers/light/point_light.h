#pragma once
#include "light.h"

class point_light: public light
{
public:
	float linear;
	float quadratic;

	point_light();
	point_light(float linear, float quadratic);
};
