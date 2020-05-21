#include "rendering/instanced_renderer.h"

instanced_renderer::instanced_renderer() : renderer()
{
	this->buffer_size = 0;
	instanced_data = nullptr;
}

instanced_renderer::instanced_renderer(std::shared_ptr<mesh> m, void* instanced_data, const unsigned int buffer_size)
{
	mesh_ptr = std::move(m);
	vbo = 0;
	vao = 0;
	ebo = 0;
	this->instanced_data = instanced_data;
	this->buffer_size = buffer_size;
	
	setup();
}

void instanced_renderer::setup()
{
	//generate buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &matrices_vbo);

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

	glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, instanced_data, GL_STATIC_DRAW);

	const size_t size = sizeof(glm::vec4);
	glVertexAttribPointer(5, 4, GL_FLOAT, false, 4 * size, nullptr);
	glVertexAttribPointer(6, 4, GL_FLOAT, false, 4 * size, reinterpret_cast<void*>(1 * size));
	glVertexAttribPointer(7, 4, GL_FLOAT, false, 4 * size, reinterpret_cast<void*>(2 * size));
	glVertexAttribPointer(8, 4, GL_FLOAT, false, 4 * size, reinterpret_cast<void*>(3 * size));
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);

	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);

	if (mesh_ptr->is_indexed)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh_ptr->indices.size(), &mesh_ptr->indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}

