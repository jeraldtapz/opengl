#include "rendering/shader_program.h"
#include <glm/gtc/type_ptr.hpp>

shader_program::shader_program(const shader* vertex_shader, const shader* fragment_shader) :
	vertex_shader(vertex_shader),
	fragment_shader(fragment_shader), geometry_shader(nullptr)
{
	int success;
	char info_log[512];

	id = glCreateProgram();
	glAttachShader(id, this->vertex_shader->id);
	glAttachShader(id, this->fragment_shader->id);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(id, 512, nullptr, info_log);
		std::cout << "FAILED TO LINK SHADER PROGRAM " << info_log << std::endl;
	}

	glDeleteShader(this->vertex_shader->id);
	glDeleteShader(this->fragment_shader->id);
}

shader_program::shader_program(const shader* vertex_shader, const shader* fragment_shader, const shader* geometry_shader) :
	vertex_shader(vertex_shader),
	fragment_shader(fragment_shader),
	geometry_shader(geometry_shader)
{
	int success;
	char info_log[512];

	id = glCreateProgram();
	glAttachShader(id, this->vertex_shader->id);
	glAttachShader(id, this->fragment_shader->id);
	glAttachShader(id, this->geometry_shader->id);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(id, 512, nullptr, info_log);
		std::cout << "FAILED TO LINK SHADER PROGRAM " << info_log << std::endl;
	}

	glDeleteShader(this->vertex_shader->id);
	glDeleteShader(this->fragment_shader->id);
	glDeleteShader(this->geometry_shader->id);
}

shader_program::~shader_program()
{
	glDeleteProgram(id);
}

void shader_program::use() const
{
	glUseProgram(id);
}

void shader_program::set_bool(const std::string& name, const bool value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void shader_program::set_float(const std::string& name, const float value) const
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void shader_program::set_int(const std::string& name, const int value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void shader_program::set_matrix(const std::string& name, const glm::mat4 matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void shader_program::set_vec3(const std::string& name, const glm::vec3 value) const
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z);
}

void shader_program::set_vec4(const std::string& name, const glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z, value.w);
}

void shader_program::set_float_array(const std::string& name, const unsigned int count, float* value) const
{
	glUniform1fv(glGetUniformLocation(id, name.c_str()), count, value);
}

void shader_program::set_vec2_array(const std::string& name, const unsigned int count, float* value) const
{
	glUniform2fv(glGetUniformLocation(id, name.c_str()), count, value);
}

void shader_program::set_mvp(const mvp matrix) const
{
	set_matrix("model", matrix.model_matrix);
	set_matrix("view", matrix.view);
	set_matrix("projection", matrix.projection);
}