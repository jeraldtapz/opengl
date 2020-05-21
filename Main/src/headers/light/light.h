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

	light();
	light(color, color);
};
