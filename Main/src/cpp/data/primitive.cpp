#include "data/primitive.h"



mesh create_cube()
{
	std::vector<vertex> cube_vertices = {

		vertex(-0.5f, -0.5f, -0.5f),
		vertex(0.5f, -0.5f, -0.5f),
		vertex(0.5f,  0.5f, -0.5f),
		vertex(0.5f,  0.5f, -0.5f),
		vertex(-0.5f,  0.5f, -0.5f),
		vertex(-0.5f, -0.5f, -0.5f),

		vertex(-0.5f, -0.5f,  0.5f),
		vertex(0.5f, -0.5f,  0.5f),
		vertex(0.5f,  0.5f,  0.5f),
		vertex(0.5f,  0.5f,  0.5f),
		vertex(-0.5f,  0.5f,  0.5f),
		vertex(-0.5f, -0.5f,  0.5f),

		vertex(-0.5f,  0.5f,  0.5f),
		vertex(-0.5f,  0.5f, -0.5f),
		vertex(-0.5f, -0.5f, -0.5f),
		vertex(-0.5f, -0.5f, -0.5f),
		vertex(-0.5f, -0.5f,  0.5f),
		vertex(-0.5f,  0.5f,  0.5f),

		vertex(0.5f,  0.5f,  0.5f),
		vertex(0.5f,  0.5f, -0.5f),
		vertex(0.5f, -0.5f, -0.5f),
		vertex(0.5f, -0.5f, -0.5f),
		vertex(0.5f, -0.5f,  0.5f),
		vertex(0.5f,  0.5f,  0.5f),

		vertex(-0.5f, -0.5f, -0.5f),
		vertex(0.5f, -0.5f, -0.5f),
		vertex(0.5f, -0.5f,  0.5f),
		vertex(0.5f, -0.5f,  0.5f),
		vertex(-0.5f, -0.5f,  0.5f),
		vertex(-0.5f, -0.5f, -0.5f),

		vertex(-0.5f,  0.5f, -0.5f),
		vertex(0.5f,  0.5f, -0.5f),
		vertex(0.5f,  0.5f,  0.5f),
		vertex(0.5f,  0.5f,  0.5f),
		vertex(-0.5f,  0.5f,  0.5f),
		vertex(-0.5f,  0.5f, -0.5f)
	};

	cube_vertices[0].normal = glm::vec3(0.0f, 0.0f, -1.0f);
	cube_vertices[1].normal = glm::vec3(0.0f, 0.0f, -1.0f);
	cube_vertices[2].normal = glm::vec3(0.0f, 0.0f, -1.0f);
	cube_vertices[3].normal = glm::vec3(0.0f, 0.0f, -1.0f);
	cube_vertices[4].normal = glm::vec3(0.0f, 0.0f, -1.0f);
	cube_vertices[5].normal = glm::vec3(0.0f, 0.0f, -1.0f);

	cube_vertices[6].normal = glm::vec3(0.0f, 0.0f, 1.0f);
	cube_vertices[7].normal = glm::vec3(0.0f, 0.0f, 1.0f);
	cube_vertices[8].normal = glm::vec3(0.0f, 0.0f, 1.0f);
	cube_vertices[9].normal = glm::vec3(0.0f, 0.0f, 1.0f);
	cube_vertices[10].normal = glm::vec3(0.0f, 0.0f, 1.0f);
	cube_vertices[11].normal = glm::vec3(0.0f, 0.0f, 1.0f);

	cube_vertices[12].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
	cube_vertices[13].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
	cube_vertices[14].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
	cube_vertices[15].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
	cube_vertices[16].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
	cube_vertices[17].normal = glm::vec3(-1.0f, 0.0f, 0.0f);

	cube_vertices[18].normal = glm::vec3(1.0f, 0.0f, 0.0f);
	cube_vertices[19].normal = glm::vec3(1.0f, 0.0f, 0.0f);
	cube_vertices[20].normal = glm::vec3(1.0f, 0.0f, 0.0f);
	cube_vertices[21].normal = glm::vec3(1.0f, 0.0f, 0.0f);
	cube_vertices[22].normal = glm::vec3(1.0f, 0.0f, 0.0f);
	cube_vertices[23].normal = glm::vec3(1.0f, 0.0f, 0.0f);

	cube_vertices[24].normal = glm::vec3(0.0f, -1.0f, 0.0f);
	cube_vertices[25].normal = glm::vec3(0.0f, -1.0f, 0.0f);
	cube_vertices[26].normal = glm::vec3(0.0f, -1.0f, 0.0f);
	cube_vertices[27].normal = glm::vec3(0.0f, -1.0f, 0.0f);
	cube_vertices[28].normal = glm::vec3(0.0f, -1.0f, 0.0f);
	cube_vertices[29].normal = glm::vec3(0.0f, -1.0f, 0.0f);

	cube_vertices[30].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	cube_vertices[31].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	cube_vertices[32].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	cube_vertices[33].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	cube_vertices[34].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	cube_vertices[35].normal = glm::vec3(0.0f, 1.0f, 0.0f);

	cube_vertices[0].tex_coord = glm::vec2(0.0f, 0.0f);
	cube_vertices[1].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[2].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[3].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[4].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[5].tex_coord = glm::vec2(0.0f, 0.0f);

	cube_vertices[6].tex_coord = glm::vec2(0.0f, 0.0f);
	cube_vertices[7].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[8].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[9].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[10].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[11].tex_coord = glm::vec2(0.0f, 0.0f);

	cube_vertices[12].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[13].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[14].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[15].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[16].tex_coord = glm::vec2(0.0f, 0.0f);
	cube_vertices[17].tex_coord = glm::vec2(1.0f, 0.0f);

	cube_vertices[18].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[19].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[20].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[21].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[22].tex_coord = glm::vec2(0.0f, 0.0f);
	cube_vertices[23].tex_coord = glm::vec2(1.0f, 0.0f);

	cube_vertices[24].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[25].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[26].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[27].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[28].tex_coord = glm::vec2(0.0f, 0.0f);
	cube_vertices[29].tex_coord = glm::vec2(0.0f, 1.0f);

	cube_vertices[30].tex_coord = glm::vec2(0.0f, 1.0f);
	cube_vertices[31].tex_coord = glm::vec2(1.0f, 1.0f);
	cube_vertices[32].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[33].tex_coord = glm::vec2(1.0f, 0.0f);
	cube_vertices[34].tex_coord = glm::vec2(0.0f, 0.0f);
	cube_vertices[35].tex_coord = glm::vec2(0.0f, 1.0f);

	mesh m = mesh(cube_vertices);
	m.is_indexed = false;
	m.should_cull_face = false;
	
	return m; // cube
}

mesh create_quad()
{
	std::vector<vertex> quad_vertices =
	{
		vertex(-1.0f, 1.0f, 0) ,
		vertex(-1.0f, -1.0f, 0),
		vertex(1.0f, -1.0f, 0),

		vertex(-1.0f, 1.0f, 0),
		vertex(1.0f, -1.0f, 0),
		vertex(1.0f, 1.0f, 0),
	};

	quad_vertices[0].tex_coord = glm::vec2(0, 1);
	quad_vertices[1].tex_coord = glm::vec2(0, 0);
	quad_vertices[2].tex_coord = glm::vec2(1, 0);
	quad_vertices[3].tex_coord = glm::vec2(0, 1);
	quad_vertices[4].tex_coord = glm::vec2(1, 0);
	quad_vertices[5].tex_coord = glm::vec2(1, 1); // tex coordinates

	quad_vertices[0].normal = glm::vec3(0, 0, 1);
	quad_vertices[1].normal = glm::vec3(0, 0, 1);
	quad_vertices[2].normal = glm::vec3(0, 0, 1);
	quad_vertices[3].normal = glm::vec3(0, 0, 1);
	quad_vertices[4].normal = glm::vec3(0, 0, 1);
	quad_vertices[5].normal = glm::vec3(0, 0, 1); // normals

	//triangle 1 *****************************************************************************

	glm::vec3 edge1 = quad_vertices[1].position - quad_vertices[0].position;
	glm::vec3 edge2 = quad_vertices[2].position - quad_vertices[0].position;
	glm::vec2 delta_uv1 = quad_vertices[1].tex_coord - quad_vertices[0].tex_coord;
	glm::vec2 delta_uv2 = quad_vertices[2].tex_coord - quad_vertices[0].tex_coord;

	float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

	quad_vertices[0].tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
	quad_vertices[0].tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
	quad_vertices[0].tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
	quad_vertices[0].tangent = glm::normalize(quad_vertices[0].tangent);

	quad_vertices[1].tangent = quad_vertices[0].tangent;
	quad_vertices[2].tangent = quad_vertices[0].tangent;

	quad_vertices[0].bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
	quad_vertices[0].bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
	quad_vertices[0].bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
	quad_vertices[0].bitangent = glm::normalize(quad_vertices[0].bitangent);

	quad_vertices[1].bitangent = quad_vertices[0].bitangent;
	quad_vertices[2].bitangent = quad_vertices[0].bitangent; //triangle 1

	// 0 == 3
	// 2 == 4
	// 

	edge1 = quad_vertices[2].position - quad_vertices[0].position;
	edge2 = quad_vertices[5].position - quad_vertices[0].position;
	delta_uv1 = quad_vertices[2].tex_coord - quad_vertices[0].tex_coord;
	delta_uv2 = quad_vertices[5].tex_coord - quad_vertices[0].tex_coord;

	f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

	quad_vertices[3].tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
	quad_vertices[3].tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
	quad_vertices[3].tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
	quad_vertices[3].tangent = glm::normalize(quad_vertices[3].tangent);

	quad_vertices[4].tangent = quad_vertices[3].tangent;
	quad_vertices[5].tangent = quad_vertices[3].tangent;

	quad_vertices[3].bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
	quad_vertices[3].bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
	quad_vertices[3].bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
	quad_vertices[3].bitangent = glm::normalize(quad_vertices[3].bitangent);

	quad_vertices[4].bitangent = quad_vertices[3].bitangent;
	quad_vertices[5].bitangent = quad_vertices[3].bitangent; //triangle 2

	mesh m = mesh(quad_vertices);
	return m; // Quad
}

mesh create_sphere()
{
	model sphere_model = model("res/models/sphere/sphere.obj", true);
	return *sphere_model.get_mesh_ptr(0);
}


mesh primitive::sphere_cache{std::vector<vertex>()};
mesh primitive::cube_cache{ std::vector<vertex>() };
mesh primitive::quad_cache{ std::vector<vertex>() };
bool primitive::is_initialized{ false };


mesh primitive::get_cube()
{
	if(!is_initialized)
	{
		cube_cache = create_cube();
		quad_cache = create_quad();
		sphere_cache = create_sphere();
		is_initialized = true;
	}
	return cube_cache;
}

mesh primitive::get_quad()
{
	if (!is_initialized)
	{
		cube_cache = create_cube();
		quad_cache = create_quad();
		sphere_cache = create_sphere();
		is_initialized = true;
	}
	return quad_cache;
}

mesh primitive::get_sphere()
{
	if (!is_initialized)
	{
		cube_cache = create_cube();
		quad_cache = create_quad();
		sphere_cache = create_sphere();
		is_initialized = true;
	}
	return sphere_cache;
}