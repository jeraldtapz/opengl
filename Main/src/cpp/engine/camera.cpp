#include "engine/camera.h"

#include <iostream>

#include "utils/config.h"

camera::camera()
{
	near = 0.1f;
	far = 100.0f;
	fov = 45.0f;
	_transform = std::make_shared<transform>(transform());
}

camera::camera(const float fov, const float near, const float far)
{
	this->near = near;
	this->far = far;
	this->fov = fov;

	_transform = std::make_shared<transform>(transform());
}

glm::mat4 camera::get_view_matrix	() const
{
	//we can get away with using 0,1,0 as up because we don't have bank/roll on our camera system
	//it should be the camera's up vector which is computed based on heading and pitch
	return glm::lookAt(_transform->position(),
		_transform->position() + _transform->forward() , _transform->up()/*glm::vec3(0, 1, 0)*/);
}

glm::mat4 camera::get_proj_matrix() const
{
	return glm::perspective(glm::radians(fov), config::WIDTH / static_cast<float>(config::HEIGHT), near, far);
}
