#include "light/light.h"


#include "utils/config.h"

light::light(const color diffuse, const color specular)
{
	this->diffuse = diffuse;
	this->specular = specular;

	diff_intensity = 1.0f;
	spec_intensity = 1.0f;
}

light::light() : light(color::WHITE, color::WHITE)
{
}

