#pragma once

#include "rendering/renderer.h"

class instanced_renderer final : public renderer
{
public:
	instanced_renderer();
	instanced_renderer(std::shared_ptr<mesh>, void* instanced_data, const unsigned int buffer_size);

protected:
	void setup() override;
	void* instanced_data;
	unsigned int buffer_size;
	unsigned int matrices_vbo {0};
};