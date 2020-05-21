#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

struct color
{
public:
	static const color WHITE;
	static const color BLACK;
	static const color RED;
	static const color BLUE;
	static const color GREEN;


	float r;
	float g;
	float b;
	float a;

	color();
	explicit color(const glm::vec3 col);
	color(float r, float g, float b, float a);
	glm::vec4 to_vec4() const;
	glm::vec3 to_vec3() const;
};
