#include "light/spot_light.h"

spot_light::spot_light()
{
	spot_direction = glm::vec3(1.0f);
	cutoff_angle = 15.0f;
	inner_cutoff_angle = 10.0f;
}
