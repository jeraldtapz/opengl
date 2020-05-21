#include "utils/config.h"

const std::string config::WINDOW_NAME = std::string("LearnOpenGL");

const color config::CLEAR_COLOR = color(1,1,1,1);

const std::string config::VERTEX_SHADER_DEFAULT = 
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos\n;"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}";

const std::string config::FRAGMENT_SHADER_DEFAULT =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"uniform vec4 color;"
	"\n"
	"void main()\n"
	"{\n"
	"    FragColor = color;\n"
	"}";
