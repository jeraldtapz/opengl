#pragma once
#include "model.h"
#include "data/vertex.h"


class primitive
{
public:
	static mesh get_sphere();
	static mesh get_quad();
	static mesh get_cube();

private:
	primitive() = delete;
	static bool is_initialized;
	static mesh sphere_cache;
	static mesh cube_cache;
	static mesh quad_cache;
};
