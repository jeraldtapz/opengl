#pragma once
#include "engine/game_object.h"
#include "rendering/color.h"

class light: public game_object
{
public:
	color diffuse;
	color specular;

	float diff_intensity;
	float spec_intensity;

	bool casts_shadow {false};

	light();
	light(color, color);
};
