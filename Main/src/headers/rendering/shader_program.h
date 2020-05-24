#pragma once
#include "shader.h"
#include "data/mvp.h"
#include "data/tiling_and_offset.h"
#include "glm/glm.hpp"

class shader_program
{
public:
	unsigned int id; // shader program get_id
	const shader* vertex_shader;
	const shader* fragment_shader;
	const shader* geometry_shader;
	shader_program(const shader* vertex_shader, const shader* fragment_shader);
	shader_program(const shader* vertex_shader, const shader* fragment_shader, const shader* geometry_shader);
	~shader_program();

	void use() const;

	void set_bool(const std::string& name, const bool value) const;
	void set_int(const std::string& name, const int value) const;
	void set_float(const std::string& name, const float value) const;
	void set_matrix(const std::string& name, const glm::mat4 matrix) const;
	void set_vec2(const std::string& name, const glm::vec2 value) const;
	void set_vec3(const std::string& name, const glm::vec3 value) const;
	void set_vec4(const std::string& name, const glm::vec4 value) const;
	void set_float_array(const std::string& name, const unsigned int count, float* value) const;
	void set_vec2_array(const std::string& name, const unsigned int count, float* value) const;
	void set_mvp(const mvp matrix) const;
	void set_model(const glm::mat4 matrix) const;
	void set_view(const glm::mat4 matrix) const;
	void set_proj(const glm::mat4 matrix) const;
	void set_tiling_and_offset(const tiling_and_offset& tiling_and_offset) const;
};