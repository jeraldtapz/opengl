#include "data/model.h"
#include <assimp/postprocess.h>

model::model(const std::string &path, const bool auto_load)
{
	is_model_loaded = false;
	this->is_instanced = false;
	
	if(auto_load)
		load_model(path);
}

model::model(const std::string& path, const bool auto_load, void* data, const unsigned int buffer_size)
{
	is_model_loaded = false;
	this->is_instanced = true;
	this->data = data;
	this->buffer_size = buffer_size;
	
	if (auto_load)
		load_model(path);
}

model::model(const std::string& path, const bool auto_load, const bool is_shadow)
{
	is_model_loaded = false;
	this->is_shadow = is_shadow;

	if (auto_load)
		load_model(path);
}


void model::load(const std::string &path)
{
	if (is_model_loaded)
		return;

	load_model(path);
}


void model::draw(const shader_program &program)
{
	for (auto& renderer : renderers)
		renderer.draw(program);
}

void model::draw_instanced(const shader_program& program, const unsigned int count)
{
	for (auto& renderer : instanced_renderers)
		renderer.draw_instanced(program, count);
}

void model::draw_shadow(const shader_program& program)
{
	for (auto& renderer : shadow_renderers)
		renderer.draw(program);
}


void model::load_model(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |  aiProcess_CalcTangentSpace);

	//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString();
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	process_node(scene->mRootNode, scene);
	is_model_loaded = true;
}

void model::process_node(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		//node->mMeshes contains indices of meshes on the global meshes collection scene->mMeshes
		aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];

		mesh m = process_mesh(ai_mesh, scene);
		m.should_cull_face = true;
		m.is_indexed = true;
		meshes.push_back(m);

		if(is_instanced)
		{
			instanced_renderer instanced_rnd = instanced_renderer(std::make_shared<mesh>(m), this->data, this->buffer_size);
			instanced_renderers.push_back(instanced_rnd);
		}
		
		shadow_renderer shadow_rnd = shadow_renderer(std::make_shared<mesh>(m));
		shadow_renderers.push_back(shadow_rnd);
		
		renderers.emplace_back(std::make_shared<mesh>(m));
	}

	for (unsigned int i = 0; i< node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene);
	}
}

mesh model::process_mesh(aiMesh* m, const aiScene* scene)
{
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<texture> textures;

	//process vertices
	for (unsigned int i = 0; i < m->mNumVertices; i++)
	{
		vertex v;
		v.position.x = m->mVertices[i].x;
		v.position.y = m->mVertices[i].y;
		v.position.z = m->mVertices[i].z;

		v.normal.x = m->mNormals[i].x;
		v.normal.y = m->mNormals[i].y;
		v.normal.z = m->mNormals[i].z;

		if (m->mTextureCoords[0])
		{
			v.tex_coord.x = m->mTextureCoords[0][i].x;
			v.tex_coord.y = m->mTextureCoords[0][i].y;
		}
		else
			v.tex_coord = glm::vec2(0);
		
		vertices.push_back(v);
	}

	for (unsigned int i = 0; i < m->mNumFaces; i++)
	{
		const aiFace face = m->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if(m->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[m->mMaterialIndex];
		
		std::vector<texture> diffuse_textures = load_material_textures(material, aiTextureType_DIFFUSE, texture_type::diffuse);
		textures.insert(textures.end(), diffuse_textures.begin(), diffuse_textures.end());

		std::vector<texture> specular_textures = load_material_textures(material, aiTextureType_SPECULAR, texture_type::specular);
		textures.insert(textures.end(), specular_textures.begin(), specular_textures.end());

		/*std::vector<texture> normal_textures = load_material_textures(material, aiTextureType_HEIGHT, texture_type::normal);
		textures.insert(textures.end(), normal_textures.begin(), normal_textures.end());

		std::vector<texture> reflection_textures = load_material_textures(material, aiTextureType_AMBIENT, texture_type::reflection);
		textures.insert(textures.end(), reflection_textures.begin(), reflection_textures.end());*/
	}

	return mesh(vertices, indices, textures);
}

std::vector<texture> model::load_material_textures(aiMaterial* mat, aiTextureType type, texture_type tex_type)
{
	std::vector<texture> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		if (is_texture_loaded(str.C_Str()))
			continue;

		std::string path = directory;
		path.append("/").append(str.C_Str());
		texture tex = texture(path, tex_type, GL_UNSIGNED_BYTE, true);

		textures.push_back(tex);
		textures_loaded.insert(std::pair<std::string, texture>(str.C_Str(), tex));
	}

	return textures;
}

bool model::is_texture_loaded(const std::string& path)
{
	return textures_loaded.find(path) != textures_loaded.end();
}

void model::deallocate()
{
	if(is_instanced)
		for (auto& rend : instanced_renderers)
			rend.deallocate();
	else
		for(auto &rend: renderers)
			rend.deallocate();
}