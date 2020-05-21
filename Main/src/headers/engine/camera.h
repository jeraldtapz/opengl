#pragma once
#include "game_object.h"

class camera : public game_object
{
public:
	float fov;
	float near;
	float far;

	camera();
	camera(float fov, float near, float far);

	glm::mat4 get_view_matrix() const;
	glm::mat4 get_proj_matrix() const;
};