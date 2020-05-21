#include "data/vertex.h"

//vertex::vertex(const glm::vec3 position, const glm::vec3 normal, const glm::vec2 tex_coord, const glm::vec3 tangent, const glm::vec3 bitangent) : position(position), normal(normal), tex_coord(tex_coord), tangent(tangent), bitangent(bitangent)
//{
//	
//}

vertex::vertex()
{
	position = glm::vec3();
	normal = glm::vec3();
	tex_coord = glm::vec2();
	tangent = glm::vec3();
	bitangent = glm::vec3();
}

vertex::vertex(const float x, const float y, const float z)
{
	position = glm::vec3(x,y,z);
	normal = glm::vec3();
	tex_coord = glm::vec2();
	tangent = glm::vec3();
	bitangent = glm::vec3();
}


