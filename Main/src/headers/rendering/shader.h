#pragma once

#include <glad/glad.h>
#include <string>
#include <iostream>


enum class shader_type;

class shader
{
	std::string shader_string;

public:
	unsigned int id; //shader get_id
	explicit shader(const std::string& path, const GLenum shader_type);
	shader();
};

