#include "data/transform.h"


transform::transform(const glm::vec3 pos, const glm::vec3 rot, const glm::vec3 scale)
{
	_position = pos;
	_rotation = rot;
	_scale = scale;
	recalculate_directions();
}

transform::transform() : transform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f))
{
	recalculate_directions();
}

void transform::recalculate_directions()
{
	_forward.x = cos(glm::radians(_rotation.x)) * cos(glm::radians(_rotation.y));
	_forward.y = sin(glm::radians(_rotation.x));
	_forward.z = cos(glm::radians(_rotation.x)) * sin(glm::radians(_rotation.y));

	_forward = glm::normalize(_forward);

	_right = glm::cross(glm::vec3(0, 1, 0), _forward);
	_right = glm::normalize(_right);

	_up = glm::cross(_forward, _right);

	/*_forward.x = cos(glm::radians(_rotation.x)) * cos(glm::radians(_rotation.y));
	_forward.y = sin(glm::radians(_rotation.x));
	_forward.z = cos(glm::radians(_rotation.x)) * sin(glm::radians(_rotation.y));*/
	
	/*_forward = glm::vec3((_rotation_mat * glm::vec4(0, 0, 1, 1)));
	_forward = glm::normalize(_forward);

	_right = glm::cross(glm::vec3(0, 1, 0), _forward);
	_right = glm::vec3(_rotation_mat * glm::vec4(1, 0, 0, 1));
	_right = glm::normalize(_right);

	_up = glm::cross(_forward, _right);*/

	//_forward = glm::vec3((_rotation_mat * glm::vec4(0, 1, 0, 1)));

}

glm::mat4 transform::get_model_matrix() const
{
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, this->position());
	model = glm::rotate(model, glm::radians(_rotation.x), right_basis);
	model = glm::rotate(model, glm::radians(_rotation.y), up_basis);
	model = glm::rotate(model, glm::radians(_rotation.z), forward_basis);
	model = glm::scale(model, this->_scale);
	return model;
}

void transform::set_position(const glm::vec3 pos)
{
	_position = pos;
}

void transform::set_rotation(const glm::vec3 rot)
{
	_rotation = rot;
	_rotation_mat = glm::yawPitchRoll(rot.x, rot.y, rot.z);
	recalculate_directions();
}

void transform::set_scale(const glm::vec3 scale)
{
	_scale = scale;
}

glm::vec3 transform::position() const
{
	return _position;
}



glm::vec3 transform::rotation() const
{
	return _rotation;
}

glm::mat4x4 transform::rotation_mat() const
{
	return _rotation_mat;
}


glm::vec3 transform::scale() const
{
	return _scale;
}

glm::vec3* transform::position_ptr()
{
	return &_position;
}

glm::vec3* transform::rotation_ptr()
{
	return &_rotation;
}


glm::vec3* transform::scale_ptr()
{
	return &_scale;
}


glm::vec3 transform::forward() const
{
	return _forward;
}

glm::vec3 transform::right() const
{
	return _right;
}

glm::vec3 transform::up() const
{
	return _up;
}

glm::vec3* transform::forward_ptr()
{
	return &_forward;
}











