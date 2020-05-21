#include "rendering/color.h"

const color color::BLACK = color(0, 0, 0, 0);
const color color::WHITE = color(1, 1, 1, 1);
const color color::RED = color(1, 0, 0, 1);
const color color::GREEN = color(0, 1, 0, 1);
const color color::BLUE = color(0, 0, 1, 1);


color::color() : color(1,1,1, 1)
{
	
}

color::color(const float r, const float g, const float b, const float a): r(r), g(g), b(b), a(a)
{
}

color::color(const glm::vec3 col): color(col.r, col.g, col.b, 1.0)
{
	
}


glm::vec4 color::to_vec4() const
{
	return glm::vec4(this->r, this->g, this->b, this->a);
}

glm::vec3 color::to_vec3() const
{
	return glm::vec3(this->r, this->g, this->b);
}

