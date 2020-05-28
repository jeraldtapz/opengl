#include <utility>


#include "rendering/renderer.h"

renderer::renderer() = default;


renderer::renderer(std::shared_ptr<mesh> mesh)
{
	mesh_ptr = std::move(mesh);
	vbo = 0;
	vao = 0;
	ebo = 0;

	renderer::setup();
}

void renderer::draw(const shader_program &program) const
{
	if(mesh_ptr->should_cull_face)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glCullFace(mesh_ptr->cull_face);

	program.use();
	
	unsigned int diffuse_number = -1;
	unsigned int specular_number = -1;
	unsigned int reflection_number = -1;
	unsigned int normal_number = -1;

	std::string setting_name;
	std::string texture_type_str;
	std::string number;
	
	for (int i = 0; i < mesh_ptr->textures.size(); i++)
	{
		setting_name.clear();
		setting_name.append("mat.");
		const texture_type tex_type = mesh_ptr->textures[i].get_type();
		std::string tex_type_str = texture::type_to_string(tex_type);
		switch (tex_type)
		{
			case texture_type::diffuse:
			{
				number = std::to_string(++diffuse_number);
				break;
			}

			case texture_type::normal:
			{
				number = std::to_string(++normal_number);
				break;
			}
			case texture_type::specular:
			{
				number = std::to_string(++specular_number);
				break;
			}
			case texture_type::reflection:
			{
				number = std::to_string(++reflection_number);
				break;
			}
			case texture_type::color:
			case texture_type::depth:
			case texture_type::stencil:
			case texture_type::height:
			{
				number = "0";
				break;
			}
			case texture_type::depth_stencil:
			default:
				break;
		}

		texture::activate(GL_TEXTURE0 + i);
		glBindTexture(mesh_ptr->textures[i].get_is_multi_sampled() ? GL_TEXTURE_2D_MULTISAMPLE: GL_TEXTURE_2D, 0);
		mesh_ptr->textures[i].bind();
		program.set_int(setting_name.append(tex_type_str).append(number), i);
	}

	texture::activate(GL_TEXTURE0);

	if (mesh_ptr->is_indexed)
		draw_with_indices();
	else
		draw_with_raw_vertices();

	if (mesh_ptr->should_cull_face)
		glDisable(GL_CULL_FACE);
}

void renderer::draw_instanced(const shader_program& program, const unsigned int count) const
{
	if (mesh_ptr->should_cull_face)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
	glCullFace(mesh_ptr->cull_face);

	program.use();

	unsigned int diffuse_number = -1;
	unsigned int specular_number = -1;
	unsigned int reflection_number = -1;
	unsigned int normal_number = -1;
	unsigned int height_number = -1;

	std::string setting_name;
	std::string texture_type_str;
	std::string number;

	for (int i = 0; i < mesh_ptr->textures.size(); i++)
	{
		setting_name.clear();
		setting_name.append("mat.");
		const texture_type tex_type = mesh_ptr->textures[i].get_type();
		std::string tex_type_str = texture::type_to_string(tex_type);
		switch (tex_type)
		{
		case texture_type::diffuse:
		{
			number = std::to_string(++diffuse_number);
			break;
		}

		case texture_type::normal:
		{
			number = std::to_string(++normal_number);
			break;
		}
		case texture_type::specular:
		{
			number = std::to_string(++specular_number);
			break;
		}
		case texture_type::reflection:
		{
			number = std::to_string(++reflection_number);
			break;
		}
		case texture_type::height:
		{
			number = std::to_string(++height_number);
			break;
		}
		case texture_type::color:
		case texture_type::depth:

		case texture_type::stencil:
		{
			number = "";
			break;
		}
		case texture_type::depth_stencil:
		default:
			break;
		}

		texture::activate(GL_TEXTURE0 + i);
		mesh_ptr->textures[i].bind();
		program.set_int(setting_name.append(tex_type_str).append(number), i);
	}

	texture::activate(GL_TEXTURE0);

	if (mesh_ptr->is_indexed)
		draw_with_indices_instanced(count);
	else
		draw_with_raw_vertices_instanced(count);

	if (mesh_ptr->should_cull_face)
		glDisable(GL_CULL_FACE);
}

void renderer::draw_cube_map(const shader_program& program) const
{
	/*if (mesh_ptr->should_cull_face)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(mesh_ptr->cull_face);
	}*/

	program.use();

	texture::activate(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mesh_ptr->textures[0].get_id());
	program.set_int("cubeMap", 0);

	if (mesh_ptr->is_indexed)
		draw_with_indices();
	else
		draw_with_raw_vertices();

	/*if (mesh_ptr->should_cull_face)
		glDisable(GL_CULL_FACE);*/
}


void renderer::draw_with_indices() const
{
	if (mesh_ptr->is_transparent)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh_ptr->indices.size()), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void renderer::draw_with_raw_vertices() const
{
	if (mesh_ptr->is_transparent)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh_ptr->vertices.size()));
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void renderer::draw_with_indices_instanced(const unsigned int count) const
{
	if (mesh_ptr->is_transparent)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh_ptr->indices.size()), GL_UNSIGNED_INT, nullptr, count );
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void renderer::draw_with_raw_vertices_instanced(const unsigned count) const
{
	if (mesh_ptr->is_transparent)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	glBindVertexArray(vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh_ptr->vertices.size()), count);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void renderer::setup()
{
	//generate buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	if (mesh_ptr->is_indexed)
		glGenBuffers(1, &ebo);

	
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_ptr->vertices.size() * sizeof(vertex), &mesh_ptr->vertices[0], GL_STATIC_DRAW);

	
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vertex), reinterpret_cast<void*>(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(vertex), reinterpret_cast<void*>(6 * sizeof(float)));
	glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(vertex), reinterpret_cast<void*>(8 * sizeof(float)));
	glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(vertex), reinterpret_cast<void*>(11 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	if(mesh_ptr->is_indexed)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(unsigned int) * mesh_ptr->indices.size(), &mesh_ptr->indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}

std::shared_ptr<mesh> renderer::get_mesh_ptr() const
{
	return mesh_ptr;
}


renderer::~renderer() = default;

void renderer::deallocate() const
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	if(mesh_ptr->is_indexed)
		glDeleteBuffers(1, &ebo);
}



