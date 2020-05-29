#include "rendering/shader_program.h"
#include <glm/gtc/type_ptr.hpp>

//TODO implement caching of uniform locations
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
	const auto loc = glGetUniformLocation(id, name.c_str());
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void shader_program::set_int(const std::string& name, const int value) const
{
	const auto loc = glGetUniformLocation(id, name.c_str());
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void shader_program::set_matrix(const std::string& name, const glm::mat4 matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void shader_program::set_vec2(const std::string& name, const glm::vec2 value) const
{
	glUniform2f(glGetUniformLocation(id, name.c_str()), value.x, value.y);
}


void shader_program::set_vec3(const std::string& name, const glm::vec3 value) const
{
	//const GLint loc = glGetUniformLocation(id, name.c_str());
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
	set_model(matrix.model_matrix);
	set_view(matrix.view);
	set_proj(matrix.projection);
}

void shader_program::set_model(const glm::mat4 matrix) const
{
	set_matrix("model", matrix);
}

void shader_program::set_view(const glm::mat4 matrix) const
{
	set_matrix("view", matrix);
}

void shader_program::set_proj(const glm::mat4 matrix) const
{
	set_matrix("projection", matrix);
}

void shader_program::set_tiling_and_offset(const tiling_and_offset& tiling_and_offset) const
{
	set_vec2("tiling", tiling_and_offset.tiling);
	set_vec2("offset", tiling_and_offset.offset);
}

