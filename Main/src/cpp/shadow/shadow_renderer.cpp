#include "shadow/shadow_renderer.h"

shadow_renderer::shadow_renderer() = default;


shadow_renderer::shadow_renderer(std::shared_ptr<mesh> mesh)
{
	mesh_ptr = std::move(mesh);
	vbo = 0;
	vao = 0;
	ebo = 0;

	setup();
}

void shadow_renderer::setup()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	if (mesh_ptr->is_indexed)
		glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_ptr->vertices.size() * sizeof(vertex), &mesh_ptr->vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex), nullptr);
	glEnableVertexAttribArray(0);

	if (mesh_ptr->is_indexed)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh_ptr->indices.size(), &mesh_ptr->indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}

void shadow_renderer::draw(const shader_program& program) const
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
		case texture_type::height:
		case texture_type::stencil:
		{
			number = "0";
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
		draw_with_indices();
	else
		draw_with_raw_vertices();

	if (mesh_ptr->should_cull_face)
		glDisable(GL_CULL_FACE);
}

void shadow_renderer::draw_with_indices() const
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

void shadow_renderer::draw_with_raw_vertices() const
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


