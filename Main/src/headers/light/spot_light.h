#pragma once
#include "light.h"

class spot_light : public light
{
public:
	glm::vec3 spot_direction;
	float cutoff_angle;
	float inner_cutoff_angle;

	spot_light();
};
