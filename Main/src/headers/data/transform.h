#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

class transform
{
private:
	glm::vec3 _position{ 0,0,0 };
	glm::vec3 _rotation{ 0,0,0 };
	glm::vec3 _scale{ 1,1,1 };
	glm::vec3 _forward{ 0,0,1 };
	glm::vec3 _up{ 0,1,0 };
	glm::vec3 _right{ 1,0,0 };

	glm::mat4x4 _rotation_mat;


	glm::vec3 right_basis = glm::vec3(1, 0, 0);
	glm::vec3 forward_basis = glm::vec3(0, 0, 1);
	glm::vec3 up_basis = glm::vec3(0, 1, 0);

	void recalculate_directions();

public:
	transform(const glm::vec3, const glm::vec3, const glm::vec3);
	transform();

	glm::vec3 position() const;
	glm::vec3 rotation() const;
	glm::vec3 scale() const;
	glm::mat4x4 rotation_mat() const;

	glm::vec3 forward() const;
	glm::vec3 up() const;
	glm::vec3 right() const;
	
	glm::vec3* position_ptr();
	glm::vec3* rotation_ptr();
	glm::vec3* scale_ptr();
	glm::vec3* forward_ptr();
	
	

	void set_position(const glm::vec3 pos);
	void set_rotation(const glm::vec3 rot);
	void set_scale(const glm::vec3 scale);

	glm::mat4 get_model_matrix() const;
};
