#include "rendering/shader.h"
#include <strstream>
#include <iosfwd>
#include <fstream>
#include <iostream>


shader::shader(const std::string &path, const GLenum shader_type)
{
	const std::string actual = std::string("res/shaders/")
	.append(shader_type == GL_VERTEX_SHADER ? "vertex/" : "pixel/")
	.append(path).append(".glsl");
	
	std::ifstream inf;

	try
	{
		inf.open(actual);
		inf.seekg(0, std::ifstream::end);

		const auto len = static_cast<const size_t>(inf.tellg());

		inf.seekg(0);

		shader_string = std::string(len + 1, '\0');
		inf.read(&shader_string[0], len);
	}
	catch (std::ifstream::failure &e)
	{
		std::cerr << e.what() << std::endl;
	}

	inf.close();

	const char* shader_code = shader_string.c_str();
	int success;
	char info_log[512];

	id = glCreateShader(shader_type);
	glShaderSource(id, 1, &shader_code, nullptr);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if(!success)
	{
		glGetShaderInfoLog(id, 512, nullptr, info_log);
		std::cout << "FAILED TO COMPILE SHADER" << info_log << std::endl;
	}
}

shader::shader()
{
	this->id = 0;
}