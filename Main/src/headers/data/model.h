#pragma once
#include <map>
#include <string>

#include "mesh.h"
#include "rendering/renderer.h"
#include "rendering/shader_program.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>


#include "engine/game_object.h"
#include "rendering/instanced_renderer.h"
#include "shadow/shadow_renderer.h"


class model : public game_object
{
public:
	explicit model(const std::string& path, bool auto_load);
	explicit model(const std::string &path,  bool auto_load, void* data, const unsigned int buffer_size);
	explicit model(const std::string& path, bool auto_load, bool is_shadow);
	explicit model(const std::vector<mesh> &meshes);
	void draw(const shader_program& program);
	void draw_instanced(const shader_program &program, const unsigned int count);
	void draw_shadow(const shader_program& program);
	void load(const std::string &path);
	void deallocate();
	std::vector<mesh> get_meshes() const;
	mesh* get_mesh_ptr(int index);

private:
	std::vector<mesh> meshes;
	std::vector<renderer> renderers;
	std::vector<instanced_renderer> instanced_renderers;
	std::vector<shadow_renderer> shadow_renderers;
	std::string directory;
	std::map<std::string, texture> textures_loaded;

	void* data {nullptr};
	unsigned int buffer_size{0};
	
	void load_model(const std::string& path);
	void process_node(aiNode* node, const aiScene* scene);
	mesh process_mesh(aiMesh* m, const aiScene* scene);
	std::vector<texture> load_material_textures(aiMaterial* mat, aiTextureType type,texture_type tex_type);

	bool is_texture_loaded(const std::string& path);
	bool is_model_loaded{false};
	bool is_instanced{ false };
	bool is_shadow{false};
};
