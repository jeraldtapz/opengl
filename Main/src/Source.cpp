#pragma region includes

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "rendering/shader.h"
#include "rendering/shader_program.h"
#include "rendering/texture.h"
#include "engine/camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include "data/kernel3.h"
#include "data/tiling_and_offset.h"
#include "data/mesh.h"
#include "data/model.h"
#include "data/mvp.h"
#include "light/light.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "light/directional_light.h"
#include "light/point_light.h"
#include "light/spot_light.h"
#include "rendering/frame_buffer.h"
#include "rendering/material.h"
#include "rendering/renderer.h"
#include "rendering/render_buffer.h"
#include "rendering/transparent_renderer.h"
#include "rendering/uniform_buffer_object.h"
#include "shadow/shadow_renderer.h"
#include "utils/config.h"

#pragma endregion

#pragma region function declarations

std::string get_tex(const std::string& path);

void clear_color_buffer();
void clear_depth_buffer();
void clear_stencil_buffer();
void clear_frame();

void set_depth_testing(const bool flag);
void set_stencil_testing(const bool flag);
void set_depth_writing(const bool flag);
void set_stencil_writing(const bool flag);
void enable_depth_testing();
void enable_stencil_testing();
void disable_depth_testing();
void disable_stencil_testing();



void render_light_sources(model &m, const shader_program &light_shader_program);
void render_model(model& m, const shader_program& program);
void render_model(model& m, const tiling_and_offset &tiling_and_offset, const shader_program& program);
void render_planet(model& m, const shader_program& program);
void render_asteroids(model& m, const shader_program& program);
void render_model_outline(model& m, const shader_program& program);
void render_floor(const renderer& rend, const shader_program& program);
void render_transparent_quads(const renderer& rend, const shader_program& program);
void render_skybox(const renderer& rend, const shader_program& program);
void render_pp_quad(const renderer& rend, const shader_program& program);
void render_directional_shadow_map(std::vector<model> &models, const shader_program& program);
void render_omnidirectional_shadow_map(std::vector<model>& models, const shader_program& program);
void bloom_postprocess(const renderer& rend, unsigned int attachments[], const shader_program& bloom_brightness, const shader_program& blur);

void send_dir_light_to_shader(const shader_program& program);
void send_point_lights_to_shader(const shader_program& program);
void send_spot_light_to_shader(const shader_program& program);
void send_material_data_to_shader(const shader_program& program);

void deallocate();
void init_imgui();
void destroy_imgui();

void glfw_error_callback(int error, const char* description);
void frame_buffer_resize_callback(GLFWwindow*, int, int);
void process_input(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);

void render_debug_windows();

#pragma endregion

#pragma region utility variables

#pragma region CONSTANTS

const unsigned int ASTEROID_COUNT = 1000;
const unsigned int SHADOW_RESOLUTION = 2048;

#pragma endregion

#pragma region Movement and Framing

camera cam = camera(45.0f, 0.1f, 200.0f);
float last_frame = 0.0f;
float delta_time = 0.0f;
double last_x = config::WIDTH * 0.5;
double last_y = config::HEIGHT * 0.5;
float key_press_cooldown = 0.25f;
float last_cursor_swap = 0.0f;
float last_flash_light_swap = 0.0f;

#pragma endregion


float hdr_exposure = 1.0f;
float brightness_threshold = 2.0f;

int cursor_mode = GLFW_CURSOR_NORMAL;
bool first_mouse;
bool is_flash_light_on = false;
bool is_open;
bool use_shadow = true;
bool use_normal_maps = true;
bool use_parallax = true;
bool use_gamma_correction = true;
bool use_hdr = true;
bool use_bloom = true;

color ambient_color;
std::map<float, transform> sorted;
mvp mvp_matrix;
mvp mvp_loaded_model;
glm::mat4* asteroid_model_matrices = new glm::mat4[ASTEROID_COUNT];

//glfw
GLFWwindow* window;


//Game Related Instances
game_object cubes[10];
game_object quads[5];
point_light point_lights[4];
directional_light dir_light;
spot_light spotlight;

std::vector<light*> lights;
std::vector<game_object*> game_objects;

material cube_mat;


//textures
const unsigned int SAMPLES = 8;
texture ms_color_tex, ms_depth_tex, shadow_depth_tex, point_shadow_depth_tex;
texture hdr_color_tex;

texture debug_fb_color_tex, debug_fb_depth_tex, debug_fb_stencil_tex;
texture bloom_fb_color_tex, bloom_fb_brightness_tex, pingpong_color_tex[2];
texture destination_fb_color_tex, destination_fb_depth_tex, destination_fb_stencil_tex;

texture container_diff_tex;
texture skybox_tex;
texture floor_normal_tex;


//models
std::vector<model> game_models;

renderer cube_renderer, screen_space_quad_renderer, rear_quad_renderer, skybox_renderer, floor_renderer;
renderer screen_space_raw_quad_renderer;
transparent_renderer quad_renderer;

shadow_renderer floor_shadow_renderer, cube_shadow_renderer;


frame_buffer shadow_fb = frame_buffer();
frame_buffer point_shadow_fb = frame_buffer();
frame_buffer bloom_fb = frame_buffer();
frame_buffer ms_fb = frame_buffer();
frame_buffer hdr_fb = frame_buffer();
frame_buffer destination_fb = frame_buffer();
frame_buffer debug_fb = frame_buffer();
frame_buffer pingpong_fb[] = { frame_buffer(), frame_buffer() };

uniform_buffer_object vp_ubo;

float values[9] = { 1.0f,1.0f,1.0f,1.0f, -9.0f, 1.0f, 1.0f, 1.0f, 1.0f };
kernel3 kernel = kernel3(values, 0.0033f);

std::string skybox_texture_paths[] =
{
	"res/textures/skybox_5/px.png",
	"res/textures/skybox_5/nx.png" ,
	"res/textures/skybox_5/ny.png",
	"res/textures/skybox_5/py.png",
	"res/textures/skybox_5/pz.png",
	"res/textures/skybox_5/nz.png"
};

#pragma endregion 

int main()
{
	
	#pragma region Init GLFW
	
	glfwSetErrorCallback(glfw_error_callback);
	
	const int init_result = glfwInit();

	if(!init_result)
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(config::WIDTH, config::HEIGHT, "Main", nullptr, nullptr);

	if(!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	
	glfwMakeContextCurrent(window);

	#pragma endregion

	#pragma region GLAD Load
	
	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

#pragma endregion

	#pragma region Viewport and Callbacks
	
	glViewport(0, 0, config::WIDTH, config::HEIGHT);
	glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);

	glfwSwapInterval(0);
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetScrollCallback(window, scroll_callback);
	
	#pragma endregion

	#pragma region ImGui Init

	init_imgui();
	
	#pragma endregion

	#pragma region Data

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
	quad_vertices[5].tex_coord = glm::vec2(1, 1);
	

	quad_vertices[0].normal = glm::vec3(0, 0, 1);
	quad_vertices[1].normal = glm::vec3(0, 0, 1);
	quad_vertices[2].normal = glm::vec3(0, 0, 1);
	quad_vertices[3].normal = glm::vec3(0, 0, 1);
	quad_vertices[4].normal = glm::vec3(0, 0, 1);
	quad_vertices[5].normal = glm::vec3(0, 0, 1);

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
	quad_vertices[2].bitangent = quad_vertices[0].bitangent;
	

	//triangle 1 *****************************************************************************

	//triangle 2 *****************************************************************************

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
	quad_vertices[5].bitangent = quad_vertices[3].bitangent;

	//triangle 2 *****************************************************************************

	std::vector<vertex> rear_quad_vertices =
	{
		vertex(-0.2f, 1.0f, 0) , // top left
		vertex(-0.2f, 0.7f, 0), // bottom left
		vertex(0.2f, 0.7f, 0), // bottom right
		vertex(0.2f, 1.0f, 0) // top right
	};

	rear_quad_vertices[0].tex_coord = glm::vec2(0, 1);
	rear_quad_vertices[1].tex_coord = glm::vec2(0, 0);
	rear_quad_vertices[2].tex_coord = glm::vec2(1, 0);
	rear_quad_vertices[3].tex_coord = glm::vec2(1, 1);


	std::vector<unsigned int> quad_indices
	{
		0, 1, 2,
		0, 2, 3
	};

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	std::vector<vertex> cube_vertices;

	for (int i = 0; i < 36; i++)
	{
		vertex v;
		v.position.x = vertices[i * 8];
		v.position.y = vertices[i * 8 + 1];
		v.position.z = vertices[i * 8 + 2];

		v.normal.x = vertices[i * 8 + 3];
		v.normal.y = vertices[i * 8 + 4];
		v.normal.z = vertices[i * 8 + 5];

		v.tex_coord.x = vertices[i * 8 + 6];
		v.tex_coord.y = vertices[i * 8 + 7];

		cube_vertices.push_back(v);
	}


	glm::vec3 cube_positions[] =
	{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 point_light_positions[] = 
	{
		glm::vec3(0.0f,  4.5f,  1.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	std::vector<glm::vec3> grass_positions
	{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3(1.5f, 0.0f, 0.51f),
		glm::vec3(0.0f, 0.0f, 0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3(0.5f, 0.0f, -0.6f)
	};

#pragma endregion

	#pragma region Shaders

	// *********** shaders *****************
	shader basic_shader_vertex = shader("basic_v", GL_VERTEX_SHADER);
	shader basic_shader_pixel = shader("basic_p", GL_FRAGMENT_SHADER);

	shader light_shader_vertex = shader("light_v", GL_VERTEX_SHADER);
	shader light_shader_pixel =  shader("light_p", GL_FRAGMENT_SHADER);

	shader outline_shader_vertex = shader("outline_v", GL_VERTEX_SHADER);
	shader outline_shader_pixel =  shader("outline_p", GL_FRAGMENT_SHADER);

	shader transparent_shader_vertex = shader("transparent_v", GL_VERTEX_SHADER);
	shader transparent_shader_pixel =  shader("transparent_p", GL_FRAGMENT_SHADER);

	shader screen_space_shader_vertex = shader("screen_space_v", GL_VERTEX_SHADER);
	shader screen_space_shader_pixel =  shader("screen_space_p", GL_FRAGMENT_SHADER);

	shader skybox_shader_vertex = shader("skybox_v", GL_VERTEX_SHADER);
	shader skybox_shader_pixel =  shader("skybox_p", GL_FRAGMENT_SHADER);

	shader planet_shader_vertex = shader("planet_v", GL_VERTEX_SHADER);
	shader planet_shader_pixel =  shader("planet_p", GL_FRAGMENT_SHADER);

	shader asteroid_shader_vertex = shader("asteroid_v", GL_VERTEX_SHADER);
	shader asteroid_shader_pixel =  shader("asteroid_p", GL_FRAGMENT_SHADER);

	shader shadow_shader_vertex = shader("simple_depth_v", GL_VERTEX_SHADER);
	shader shadow_shader_pixel = shader("simple_depth_p", GL_FRAGMENT_SHADER);

	shader point_shadow_shader_vertex = shader("point_shadow_v", GL_VERTEX_SHADER);
	shader point_shadow_shader_pixel = shader("point_shadow_p", GL_FRAGMENT_SHADER);
	shader point_shadow_shader_geometry = shader("point_shadow_g", GL_GEOMETRY_SHADER);

	shader bloom_brightness_shader_vertex = shader("bloom_brightness_v", GL_VERTEX_SHADER);
	shader bloom_brightness_shader_pixel = shader("bloom_brightness_p", GL_FRAGMENT_SHADER);

	shader blur_shader_vertex = shader("blur_v", GL_VERTEX_SHADER);
	shader blur_shader_pixel = shader("blur_p", GL_FRAGMENT_SHADER);

	// ************** shader programs **************
	shader_program basic_shader_program = shader_program(&basic_shader_vertex, &basic_shader_pixel);
	shader_program basic_shader_program_2 = shader_program(&basic_shader_vertex, &basic_shader_pixel);
	shader_program basic_shader_program_3 = shader_program(&basic_shader_vertex, &basic_shader_pixel);
	shader_program light_shader_program = shader_program(&light_shader_vertex, &light_shader_pixel);
	shader_program outline_shader_program = shader_program(&outline_shader_vertex, &outline_shader_pixel);
	shader_program transparent_shader_program = shader_program(&transparent_shader_vertex, &transparent_shader_pixel);
	shader_program screen_space_shader_program = shader_program(&screen_space_shader_vertex, &screen_space_shader_pixel);
	shader_program skybox_shader_program = shader_program(&skybox_shader_vertex, &skybox_shader_pixel);
	shader_program planet_shader_program = shader_program(&planet_shader_vertex, &planet_shader_pixel);
	shader_program asteroid_shader_program = shader_program(&asteroid_shader_vertex, &asteroid_shader_pixel);
	shader_program shadow_shader_program = shader_program(&shadow_shader_vertex, &shadow_shader_pixel);
	shader_program point_shadow_shader_program = shader_program(&point_shadow_shader_vertex, &point_shadow_shader_pixel, &point_shadow_shader_geometry);
	shader_program bloom_brightness_shader_program = shader_program(&bloom_brightness_shader_vertex, &bloom_brightness_shader_pixel);
	shader_program blur_shader_program = shader_program(&blur_shader_vertex, &blur_shader_pixel);
	
	#pragma endregion

	#pragma region Colors, Textures, Materials & Meshes

	ambient_color = color(0.1f, 0.1f, 0.1f, 1.0f);

	container_diff_tex = texture(get_tex("container2_diffuse.png"), texture_type::diffuse, GL_UNSIGNED_BYTE, true);
	const texture wall_tex = texture(get_tex("liza.jpg"), texture_type::diffuse, GL_UNSIGNED_BYTE, true);

	const texture container_spec_tex = texture(get_tex("container2_specular.png"), texture_type::specular, GL_UNSIGNED_BYTE, true);

	const texture matrix_tex = texture(get_tex("matrix.jpg"), texture_type::diffuse, GL_UNSIGNED_BYTE, true);

	texture grass_tex = texture(get_tex("grass.png"), texture_type::diffuse, GL_UNSIGNED_BYTE, true);

	texture window_tex = texture(get_tex("window.png"), texture_type::diffuse, GL_UNSIGNED_BYTE, true);

	texture floor_tex = texture(get_tex("floor/bricks_col.jpg"), texture_type::diffuse, GL_UNSIGNED_BYTE, true);
	floor_normal_tex = texture(get_tex("floor/bricks_normal.jpg"), texture_type::normal, GL_UNSIGNED_BYTE, true);
	texture floor_height_tex = texture(get_tex("floor/bricks_displacement.jpg"), texture_type::height, GL_UNSIGNED_BYTE, true);
	
	texture floor_spec_tex = texture(get_tex("floor/bricks_rough.jpg"), texture_type::specular, GL_UNSIGNED_BYTE, true);

	//basically unnecessary
	bloom_fb_color_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	bloom_fb_brightness_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);

	for(int i = 0; i < 2; i++)
		pingpong_color_tex[i] = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, true);

	debug_fb_color_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	debug_fb_depth_tex = texture(texture_type::depth, config::WIDTH, config::HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, false);
	/*debug_fb_stencil_tex = texture(texture_type::stencil, config::WIDTH, config::HEIGHT, GL_STENCIL_INDEX, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, false);*/

	ms_color_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, false, SAMPLES);
	ms_depth_tex = texture(texture_type::depth, config::WIDTH, config::HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, false, SAMPLES);

	hdr_color_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);

	shadow_depth_tex = texture(texture_type::depth, SHADOW_RESOLUTION, SHADOW_RESOLUTION, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false);

	point_shadow_depth_tex = texture(texture_type::cube, SHADOW_RESOLUTION, SHADOW_RESOLUTION, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false);
	
	destination_fb_color_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	destination_fb_depth_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false);
	/*destination_fb_stencil_tex = texture(texture_type::color, config::WIDTH, config::HEIGHT, GL_STENCIL_INDEX, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, false);*/

	skybox_tex = texture(skybox_texture_paths, texture_type::cube, GL_RGB, GL_RGBA, GL_UNSIGNED_BYTE); // textures
	
	//materials
	cube_mat = material(color::WHITE, color::WHITE);


	std::vector<texture> cube_textures = { container_diff_tex, container_spec_tex };
	mesh cube_mesh = mesh(cube_vertices, cube_textures);
	cube_mesh.is_indexed = false;
	cube_mesh.should_cull_face = false;

	std::vector<texture> skybox_textures = { skybox_tex };
	mesh skybox_cube_mesh = mesh(cube_vertices, skybox_textures);
	skybox_cube_mesh.is_indexed = false;
	skybox_cube_mesh.should_cull_face = false;


	std::vector<texture> quad_textures = { window_tex };
	mesh transparent_quad_mesh = mesh(quad_vertices, quad_indices, quad_textures);
	transparent_quad_mesh.is_transparent = true;
	transparent_quad_mesh.is_indexed = false;
	transparent_quad_mesh.should_cull_face = false;

	std::vector<texture> floor_textures = { floor_tex, floor_spec_tex, floor_normal_tex , floor_height_tex };
	mesh floor_mesh = mesh(quad_vertices, floor_textures);
	floor_mesh.is_indexed = false;
	floor_mesh.should_cull_face = false;

	std::vector<texture> screen_space_quad_textures = { destination_fb_color_tex };
	mesh ss_quad_mesh = mesh{ quad_vertices, quad_indices, screen_space_quad_textures };
	ss_quad_mesh.is_indexed = false;
	ss_quad_mesh.should_cull_face = false;

	mesh ss_raw_quad_mesh = mesh{ quad_vertices };
	ss_quad_mesh.is_indexed = false;
	ss_quad_mesh.should_cull_face = true;
	
	mesh rear_quad_mesh = mesh{ rear_quad_vertices, quad_indices, screen_space_quad_textures };
	rear_quad_mesh.is_indexed = false;
	rear_quad_mesh.should_cull_face = false; //meshes
	
	
	#pragma endregion

	#pragma region Renderers and  Model

	cube_renderer = renderer(std::make_shared<mesh>(cube_mesh));
	cube_shadow_renderer = shadow_renderer(std::make_shared<mesh>(cube_mesh));
	skybox_renderer = renderer(std::make_shared<mesh>(skybox_cube_mesh));
	quad_renderer = transparent_renderer(std::make_shared<mesh>(transparent_quad_mesh));
	screen_space_quad_renderer = renderer(std::make_shared<mesh>(ss_quad_mesh));
	screen_space_raw_quad_renderer = renderer(std::make_shared<mesh>(ss_raw_quad_mesh));
	rear_quad_renderer = renderer(std::make_shared<mesh>(rear_quad_mesh));
	floor_renderer = renderer(std::make_shared<mesh>(floor_mesh));

	floor_shadow_renderer = shadow_renderer(std::make_shared<mesh>(floor_mesh));

	//loaded models
	model barrel_model = model("res/models/barrel/scene.gltf", true);
	model sphere_model = model("res/models/sphere/sphere.obj", true);
	//model cabin_model = model("res/models/cabin_gltf/scene.gltf", true);
	//nanosuit.load("res/models/nanosuit2/scene.gltf");

	//generated models
	std::vector<mesh> cube_meshes = { cube_mesh };
	model cube_model = model(cube_meshes);

	std::vector<mesh> floor_meshes = { floor_mesh };
	model floor_model = model(floor_meshes);

	srand(static_cast<unsigned int>(glfwGetTime()));
	float radius = 50.0f;
	float offset = 70.f;

	for (unsigned int i = 0; i < ASTEROID_COUNT; i++)
	{
		glm::mat4 m = glm::mat4(1.0f);
		float angle = static_cast<float>(i) / static_cast<float>(ASTEROID_COUNT) * 360.0f;
		float displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		m = glm::translate(m, glm::vec3(x, y, z));

		float scale = rand() % 20 / 100.0f + 0.05f;
		m = glm::scale(m, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rot_angle = static_cast<float>(rand() % 360);
		m = glm::rotate(m, rot_angle, glm::vec3(0.4f, 0.6f, 0.8f));

		asteroid_model_matrices[i] = m;
	}

	model asteroid = model("res/models/rock/rock.obj", true, &asteroid_model_matrices[0], 4 * ASTEROID_COUNT * sizeof(glm::vec4));
	model planet = model("res/models/planet/planet.obj", true);

	game_models.push_back(barrel_model);
	game_models.push_back(cube_model);
	
	#pragma endregion

	#pragma region FrameBuffers and RenderBuffers and Uniform Buffer Objects

	ms_fb.generate();
	ms_fb.bind();
	frame_buffer::attach_texture_2d(ms_color_tex.get_id(), GL_COLOR_ATTACHMENT0, true);
	frame_buffer::attach_texture_2d(ms_depth_tex.get_id(), GL_DEPTH_ATTACHMENT, true);
	std::cout << "frame buffer with multi sampled color and depth texture " << frame_buffer::validate() << std::endl;;
	frame_buffer::unbind(); // Multi sampled fb
	
	bloom_fb.generate();
	bloom_fb.bind();
	frame_buffer::attach_texture_2d(bloom_fb_color_tex.get_id(), GL_COLOR_ATTACHMENT0, false);
	frame_buffer::attach_texture_2d(bloom_fb_brightness_tex.get_id(), GL_COLOR_ATTACHMENT1, false);

	unsigned int bloom_attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	
	std::cout << "Bloom Frame buffer " << frame_buffer::validate() << std::endl;
	
	frame_buffer::unbind(); // bloom fb

	for(int i = 0; i < 2; i++)
	{
		pingpong_fb[i].generate();
		pingpong_fb[i].bind();

		frame_buffer::attach_texture_2d(pingpong_color_tex[i].get_id(), GL_COLOR_ATTACHMENT0, false);

		std::cout << "Pingpong texture " << std::to_string(i) << ", " << frame_buffer::validate() << std::endl;

		frame_buffer::unbind();
	} // pingpong fb

	shadow_fb.generate();
	shadow_fb.bind();
	frame_buffer::attach_texture_2d(shadow_depth_tex.get_id(), GL_DEPTH_ATTACHMENT, false);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	std::cout << "shadow frame buffer with depth texture " << frame_buffer::validate() << std::endl;;
	frame_buffer::unbind(); // shadow fb

	point_shadow_fb.generate();
	point_shadow_fb.bind();
	frame_buffer::attach_texture(point_shadow_depth_tex, GL_DEPTH_ATTACHMENT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	std::cout << "frame buffer for point shadow " << frame_buffer::validate() << std::endl;
	frame_buffer::unbind(); // point shadow fb

	hdr_fb.generate();
	hdr_fb.bind();
	frame_buffer::attach_texture_2d(hdr_color_tex.get_id(), GL_COLOR_ATTACHMENT0, false);
	
	render_buffer hdr_rb = render_buffer(GL_DEPTH24_STENCIL8, config::WIDTH, config::HEIGHT);
	frame_buffer::attach_render_buffer(hdr_rb.get_id(), GL_DEPTH_STENCIL_ATTACHMENT, false);

	std::cout << "HDR frame buffer " << frame_buffer::validate() << std::endl;

	frame_buffer::unbind(); // hdr fb

	debug_fb.generate();
	debug_fb.bind();

	frame_buffer::attach_texture_2d(debug_fb_color_tex.get_id(), GL_COLOR_ATTACHMENT0, false);
	frame_buffer::attach_texture_2d(debug_fb_depth_tex.get_id(), GL_DEPTH_ATTACHMENT, false);

	std::cout << "Debug Frame Buffer " << frame_buffer::validate() << std::endl;
	frame_buffer::unbind(); // debug fb

	destination_fb.generate();
	destination_fb.bind();

	frame_buffer::attach_texture_2d(destination_fb_color_tex.get_id(), GL_COLOR_ATTACHMENT0, false);
	frame_buffer::attach_texture_2d(destination_fb_depth_tex.get_id(), GL_DEPTH_ATTACHMENT, false);

	std::cout << "Destination Frame Buffer " << frame_buffer::validate() << std::endl;

	frame_buffer::unbind(); //destination fb

	vp_ubo = uniform_buffer_object(2 * sizeof(glm::mat4), GL_STATIC_DRAW);

	//bind ubo to binding point 1
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, vp_ubo.get_id());

	//in opengl 4.x binding point can be specified in shader
	//bind shader program to binding point 1
	/*unsigned int vp_index = glGetUniformBlockIndex(basic_shader_program.id, "VP");
	glUniformBlockBinding(basic_shader_program.id, vp_index, 1);*/ // UBOs
	
	#pragma endregion

	#pragma region Camera Initialization

	cam.get_transform()->set_rotation(glm::vec3(-45, 90,0));
	cam.get_transform()->set_position(glm::vec3(0, 4, -2));
	
	#pragma endregion

	#pragma region Initialize Game Objects and Lights
	
	for (size_t i = 0; i < 10; i++)
	{
		cubes[i] = game_object();
		cubes[i].set_name("cube_" + std::to_string(i));
		cubes[i].get_transform()->set_position(cube_positions[i]);
		game_objects.push_back(&cubes[i]);
	}

	for (int i = 0; i < 5; i++)
	{
		quads[i] = game_object();
		quads[i].set_name("grass" + std::to_string(i));
		quads[i].get_transform()->set_position(grass_positions[i]);
		game_objects.push_back(&quads[i]);
	}

	dir_light = directional_light();
	dir_light.set_name("directional_light");
	dir_light.get_transform()->set_position(glm::vec3(0, 1, 4));
	dir_light.diff_intensity = 0.5f;
	game_objects.push_back(&dir_light);
	lights.push_back(&dir_light);
	
	for (size_t i = 0; i < 4 ; i++)
	{
		point_lights[i].get_transform()->set_position(point_light_positions[i]);
		point_lights[i].get_transform()->set_scale(glm::vec3(0.1f));
		point_lights[i].linear = 0.09f;
		point_lights[i].quadratic = 0.032f;
		point_lights[i].is_active = i == 0;
		point_lights[i].set_name(std::string("point_light_").append(std::to_string(i)));
		game_objects.push_back(&point_lights[i]);
		lights.push_back(&point_lights[i]);
	}

	spotlight = spot_light();
	spotlight.diffuse = color(250 / 255.0f, 1.0f, 107 / 255.0f, 1.0f);
	spotlight.diff_intensity = 2.0f;
	spotlight.diffuse = color(250 / 255.0f, 1.0f, 107 / 255.0f, 1.0f);
	spotlight.diff_intensity = 1.0f;
	
	spotlight.set_name("spot_light");
	lights.push_back(&spotlight);
	game_objects.push_back(&spotlight);


	mvp_loaded_model.model_matrix = glm::mat4(1);
	mvp_loaded_model.model_matrix = glm::scale(mvp_loaded_model.model_matrix, glm::vec3(0.025f));
	mvp_loaded_model.model_matrix = glm::rotate(mvp_loaded_model.model_matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	mvp_loaded_model.model_matrix = glm::translate(mvp_loaded_model.model_matrix, glm::vec3(0, 1, 1));

	barrel_model.get_transform()->set_position(glm::vec3(0, 0, 1));
	barrel_model.get_transform()->set_rotation(glm::vec3(-90.0f, 0.0f, 0.0f));
	barrel_model.get_transform()->set_scale(glm::vec3(0.025f));

	cube_model.get_transform()->set_position(glm::vec3(2, 0.5f, 0));

	floor_model.get_transform()->set_rotation(glm::vec3(-90.0f, 0.0f, 0.0f));
	floor_model.get_transform()->set_scale(glm::vec3(5, 5, 1));
	tiling_and_offset floor_tiling_and_offset = tiling_and_offset{ glm::vec2(5,5), glm::vec2(0,0) };

	sphere_model.get_transform()->set_position(glm::vec3(1, 5, 0));
	sphere_model.get_transform()->set_scale(glm::vec3(0.1f));
	
	#pragma endregion

	#pragma region Loop

	while(!glfwWindowShouldClose(window))
	{
		process_input(window);
		
		render_omnidirectional_shadow_map(game_models, point_shadow_shader_program);
		render_directional_shadow_map(game_models, shadow_shader_program); // shadow passes

		glViewport(0, 0, config::WIDTH, config::HEIGHT);

		if (use_hdr)
			hdr_fb.bind();
		else
			ms_fb.bind();

		clear_frame();

		mvp_matrix.view = cam.get_view_matrix();
		mvp_matrix.projection = cam.get_proj_matrix();

		vp_ubo.buffer_data_range(0, sizeof(glm::mat4), glm::value_ptr(mvp_matrix.view));
		vp_ubo.buffer_data_range(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(mvp_matrix.projection));

		render_model(cube_model, tiling_and_offset(), planet_shader_program);
		render_light_sources(sphere_model, light_shader_program);
		render_model(barrel_model, tiling_and_offset(), basic_shader_program);
		render_model(floor_model, floor_tiling_and_offset, basic_shader_program_2);
		render_skybox(skybox_renderer, skybox_shader_program);

		
		//render_planet(planet, planet_shader_program);
		//render_asteroids(asteroid, asteroid_shader_program);
		//render_transparent_quads(quad_renderer, transparent_shader_program);
		
		frame_buffer::unbind(); // render normally, with HDR

		glBindFramebuffer(GL_READ_FRAMEBUFFER, use_hdr ? hdr_fb.get_id() : ms_fb.get_id());

		//blit to bloom buffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloom_fb.get_id());
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination_fb.get_id());
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, debug_fb.get_id());
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
		
		frame_buffer::unbind(); // blit

		bloom_postprocess(screen_space_raw_quad_renderer, bloom_attachments, bloom_brightness_shader_program, blur_shader_program);
		
		clear_frame();
		render_pp_quad(screen_space_quad_renderer, screen_space_shader_program);
		render_debug_windows();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	#pragma endregion

	#pragma region Cleanup

	deallocate();
	destroy_imgui();
	glfwTerminate();
	
	#pragma endregion 
}

#pragma region Buffer Clearing, Enable/Disable

void clear_color_buffer()
{
	const color col = color(0.15f, 0.15f, 0.15f, 1.0f);
	glClearColor(col.r, col.g, col.b, col.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void clear_depth_buffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void clear_stencil_buffer()
{
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void clear_frame()
{
	clear_color_buffer();
	clear_depth_buffer();
	clear_stencil_buffer();
}

void enable_depth_testing()
{
	glEnable(GL_DEPTH_TEST);
}

void enable_stencil_testing()
{
	glEnable(GL_STENCIL_TEST);
}

void disable_depth_testing()
{
	glDisable(GL_DEPTH_TEST);
}

void disable_stencil_testing()
{
	glDisable(GL_STENCIL_TEST);
}

void set_depth_testing(const bool flag)
{
	if (flag)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void set_depth_writing(const bool flag)
{
	glDepthMask(flag ? GL_TRUE : GL_FALSE);
}

void set_stencil_testing(const bool flag)
{
	if (flag)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void set_stencil_writing(const bool flag)
{
	glStencilMask(flag ? 0xFF : 0x00);
}

#pragma endregion

#pragma region Render Functions

void render_light_sources(model& m, const shader_program& light_shader_program)
{
	light_shader_program.use();

 	for (int i = 0; i < 4; i++)
	{
		light& l = (*lights[i]);
 		
		if (l.get_name() == "spot_light" || !l.is_active)
			continue;

		light_shader_program.set_vec3("color", l.diffuse.to_vec3() * l.diff_intensity);
		m.get_transform()->set_position(l.get_transform()->position());

		render_model(m, light_shader_program);
	}
}

void render_model(model &m, const shader_program &program)
{
	set_depth_writing(true);
	set_depth_testing(true);
	set_stencil_writing(true);
	set_stencil_testing(false);
	
	program.use();
	program.set_float("useShadow", use_shadow ? 1 : 0);
	program.set_float("useNormalMaps", use_normal_maps ? 1 : 0);
	program.set_float("useParallax", use_parallax ? 1 : 0);
	program.set_vec3("viewPos", cam.get_transform()->position());
	
	send_point_lights_to_shader(program);
	send_dir_light_to_shader(program);
	send_spot_light_to_shader(program);
	send_material_data_to_shader(program);
	

	program.set_model(m.get_transform()->get_model_matrix());

	program.set_matrix("lightView", glm::lookAt(dir_light.get_transform()->position(), glm::vec3(0), glm::vec3(0, 1, 0)));
	program.set_matrix("lightProjection", glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.f));

	/*glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.get_id());
	program.set_int("mat.reflectionTexture0", 8);*/

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_tex.get_id());
	program.set_int("mat.shadowMap0", 7);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_tex.get_id());
	program.set_int("pointShadowMap", 8);
	program.set_float("farPlane", 25.0f);
	program.set_vec3("pointLights[0].lightPos", point_lights[0].get_transform()->position());

	program.set_float("shouldReceiveShadow", 0);

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	set_stencil_writing(true);
	m.draw(program);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	//glStencilMask(0x00);
}

void render_model(model& m, const tiling_and_offset& tiling_and_offset, const shader_program& program)
{
	program.use();
	program.set_tiling_and_offset(tiling_and_offset);
	render_model(m, program);
}

void render_cabin(model& m, const shader_program& program)
{
	set_depth_writing(true);
	set_depth_testing(true);
	set_stencil_writing(true);
	//set_stencil_testing(true);

	program.use();
	program.set_float("useShadow", use_shadow ? 1 : 0);
	program.set_float("useNormalMaps", use_normal_maps ? 1 : 0);
	program.set_float("useParallax", use_parallax ? 1 : 0);
	program.set_vec3("viewPos", cam.get_transform()->position());
	program.set_vec3("tiling", glm::vec3(1));
	program.set_vec3("offset", glm::vec3(0));

	send_point_lights_to_shader(program);
	send_dir_light_to_shader(program);
	send_spot_light_to_shader(program);
	send_material_data_to_shader(program);


	mvp_matrix.model_matrix = glm::mat4(1);
	program.set_mvp(mvp_matrix);

	//needed for shadow mapping
	program.set_matrix("lightView", glm::lookAt(dir_light.get_transform()->position(), glm::vec3(0), glm::vec3(0, 1, 0)));
	program.set_matrix("lightProjection", glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.f));

	/*glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.get_id());
	program.set_int("mat.reflectionTexture0", 8);*/

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_tex.get_id());
	program.set_int("mat.shadowMap0", 7);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_tex.get_id());
	program.set_int("pointShadowMap", 8);
	program.set_float("farPlane", 25.0f);
	program.set_vec3("pointLights[0].lightPos", point_lights[0].get_transform()->position());

	program.set_float("shouldReceiveShadow", 0);

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	set_stencil_writing(true);
	m.draw(program);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	//glStencilMask(0x00);
}

void render_planet(model& m, const shader_program& program)
{
	set_stencil_testing(false);
	set_stencil_writing(false);
	
	program.use();

	glm::mat4 planet_model_matrix = glm::mat4(1);
	planet_model_matrix = glm::translate(planet_model_matrix, glm::vec3(0, -3, 0));
	planet_model_matrix = glm::scale(planet_model_matrix, glm::vec3(1));
	mvp_matrix.model_matrix = planet_model_matrix;
	program.set_mvp(mvp_matrix);

	m.draw(program);
}

void render_asteroids(model& m, const shader_program& program)
{
	set_stencil_testing(false);
	set_stencil_writing(false);

	program.use();

	//for (size_t i = 0; i < asteroid_count; i++)
	//{
	//	mvp_matrix.model_matrix = asteroid_model_matrices[i];
	//	//program.set_matrix("model", mvp_matrix.model_matrix);
	//	m.draw_instanced(program, asteroid_count);
	//	//m.draw(program);
	//}
	m.draw_instanced(program, ASTEROID_COUNT);

	//glm::mat4 planet_model_matrix = glm::mat4(1);
	//planet_model_matrix = glm::translate(planet_model_matrix, glm::vec3(0, -3, 0));
	//planet_model_matrix = glm::scale(planet_model_matrix, glm::vec3(1));
	//mvp_matrix.model_matrix = planet_model_matrix;
	////program.set_mvp(mvp_matrix);
	//program.set_matrix("model", mvp_matrix.model_matrix);

	//m.draw(program);
}

void render_model_outline(model &m, const shader_program &program)
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	program.use();

	glm::mat4 model_matrix = glm::mat4(1);
	model_matrix = glm::scale(model_matrix, glm::vec3(0.1f, 0.1f, 0.1f));
	mvp_matrix.model_matrix = model_matrix;
	
	program.set_mvp(mvp_matrix);
	program.set_vec3("outlineColor", glm::vec3(0, 0, 1));
	m.draw(program);

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glEnable(GL_DEPTH_TEST);
}

void render_floor(const renderer& rend, const shader_program& program)
{
	set_depth_writing(true);
	set_depth_testing(true);
	set_stencil_writing(true);

	program.use();
	program.set_float("useShadow", use_shadow ? 1 : 0);
	program.set_float("useNormalMaps", use_normal_maps ? 1 : 0);
	program.set_float("useParallax", use_parallax ? 1 : 0);
	program.set_vec3("viewPos", cam.get_transform()->position());
	send_point_lights_to_shader(program);
	send_dir_light_to_shader(program);
	send_spot_light_to_shader(program);
	send_material_data_to_shader(program);

	mvp_matrix.model_matrix = glm::mat4(1);
	mvp_matrix.model_matrix = glm::translate(mvp_matrix.model_matrix, glm::vec3(0, 0, 0));
	mvp_matrix.model_matrix = glm::rotate(mvp_matrix.model_matrix, glm::radians( -90.0f), glm::vec3(1, 0, 0));
	mvp_matrix.model_matrix = glm::scale(mvp_matrix.model_matrix, glm::vec3(5, 5, 1));
	program.set_mvp(mvp_matrix);
	program.set_vec3("tiling", glm::vec3(5));
	program.set_vec3("offset", glm::vec3(0));

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_tex.get_id());
	program.set_int("mat.shadowMap0", 7);

	program.set_matrix("lightView", glm::lookAt(dir_light.get_transform()->position(), glm::vec3(0), glm::vec3(0, 1, 0)));
	program.set_matrix("lightProjection", glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.f));

	program.set_float("shouldReceiveShadow", 1.0f);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_tex.get_id());
	program.set_int("pointShadowMap", 8);
	program.set_float("farPlane", 25.0f);
	program.set_vec3("pointLights[0].lightPos", point_lights[0].get_transform()->position());
	rend.draw(program);
}

void render_transparent_quads(const renderer& rend, const shader_program& program)
{
	set_stencil_testing(false);
	set_stencil_writing(false);
	sorted.clear();
	for (const auto& quad : quads)
	{
		float distance = length(cam.get_transform()->position() - quad.get_transform()->position());
		sorted[distance] = *(quad.get_transform());
	}

	program.use();

	for (std::map<float, transform>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
	{
		program.use();
		it->second.set_scale(glm::vec3(0.25f));
		mvp_matrix.model_matrix = it->second.get_model_matrix();
		program.set_mvp(mvp_matrix);
		program.set_vec3("tiling", glm::vec3(1));
		rend.draw(program);
	}

	set_stencil_testing(true);
	set_stencil_writing(true);
}

void render_skybox(const renderer& rend, const shader_program& program)
{
	set_depth_testing(true);
	set_depth_writing(false);
	
	glDepthFunc(GL_LEQUAL);
	mvp_matrix.view = glm::mat4(glm::mat3(cam.get_view_matrix()));
	program.use();
	program.set_mvp(mvp_matrix);
	rend.draw_cube_map(program);
	set_depth_writing(true);
	glDepthFunc(GL_LESS);

	mvp_matrix.view = cam.get_view_matrix();
}

void render_pp_quad(const renderer& rend, const shader_program& program)
{
	program.use();
	program.set_float_array("kernel", 9, kernel.kernel);
	program.set_vec2_array("offsets", 9, &kernel.offset[0].x);
	program.set_float("exposure", hdr_exposure);
	program.set_float("useGammaCorrection", use_gamma_correction ? 1 : 0);
	program.set_float("useHDR", use_gamma_correction ? 1 : 0);

	glActiveTexture(GL_TEXTURE1);
	pingpong_color_tex[1].bind();
	program.set_int("bloomBlur", 1);
	program.set_float("useBloom", use_bloom ? 1 : 0);
	rend.draw(program);
}

void render_directional_shadow_map(std::vector<model>& models, const shader_program& program)
{
	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	shadow_fb.bind();

	clear_depth_buffer();

	mvp_matrix.view = glm::lookAt(dir_light.get_transform()->position(), glm::vec3(0), glm::vec3(0, 1, 0));
	mvp_matrix.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.f);

	program.use();
	program.set_view(mvp_matrix.view);
	program.set_proj(mvp_matrix.projection);

	glCullFace(GL_FRONT);

	for (auto value : models)
	{
		program.set_model(value.get_transform()->get_model_matrix());
		value.draw_shadow(program);
	}

	/*for (auto shadow_renderer : rend)
	{
		
	}

	rend[0].draw(program);*/
	glCullFace(GL_BACK);

	frame_buffer::unbind(); // render scene to directional shadow map
}

void render_omnidirectional_shadow_map(std::vector<model>& models, const shader_program &program)
{
	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);

	point_shadow_fb.bind();
	clear_depth_buffer();
	clear_color_buffer();

	const glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 25.0f);

	const glm::vec3 pos = point_lights[0].get_transform()->position();
	std::vector<glm::mat4> shadow_view_matrices;
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	program.use();
	program.set_float("farPlane", 25.0f);
	program.set_vec3("lightPos", pos);
	program.set_matrix("lightProj", proj);
	
	for (size_t i = 0; i < shadow_view_matrices.size(); i++)
	{
		program.set_matrix(std::string("lightView[").append(std::to_string(i).append("]")), shadow_view_matrices[i]);
	}

	/*glm::mat4 loaded_model_matrix = glm::mat4(1);
	loaded_model_matrix = glm::translate(loaded_model_matrix, glm::vec3(0, 0, 0));
	loaded_model_matrix = glm::scale(loaded_model_matrix, glm::vec3(0.05f));
	loaded_model_matrix = glm::rotate(loaded_model_matrix, glm::radians(90.0f), glm::vec3(0, 0, 1));*/

	glCullFace(GL_FRONT);
	for (std::vector<model>::value_type& value : models)
	{
		program.set_model(value.get_transform()->get_model_matrix());
		value.draw_shadow(program);
		
	}
	glCullFace(GL_BACK);
	
	//point_shadow_shader_program.set_matrix("model", mvp_loaded_model.model_matrix);

	//texture::activate(GL_TEXTURE0);
	//point_shadow_depth_tex.bind();
	/*glCullFace(GL_FRONT);
	barrel_model.draw_shadow(point_shadow_shader_program);
	glCullFace(GL_BACK);

	glm::mat4 cube_model_matrix = glm::mat4(1);
	cube_model_matrix = glm::translate(cube_model_matrix, glm::vec3(2, 0.5f, 0));
	point_shadow_shader_program.set_matrix("model", cube_model_matrix);
	glCullFace(GL_BACK);
	set_depth_testing(true);
	set_depth_writing(true);
	cube_shadow_renderer.draw(point_shadow_shader_program);
	glCullFace(GL_FRONT);*/

	frame_buffer::unbind(); // render scene to omni directional shadow map // render scene to omnidirectional shadow map
}

void bloom_postprocess(const renderer& rend, unsigned int attachments[], const shader_program &bloom_brightness, const shader_program &blur)
{
	bloom_fb.bind();
	glDrawBuffers(2, attachments);
	
	bloom_brightness.use();
	glBindTexture(GL_TEXTURE_2D, bloom_fb_color_tex.get_id());
	bloom_brightness.set_float("useBloom", use_bloom);
	bloom_brightness.set_float("brightnessThreshold", brightness_threshold);
	rend.draw(bloom_brightness);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, bloom_fb_brightness_tex.get_id());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pingpong_fb[0].get_id());
	glBlitFramebuffer(0, 0, config::WIDTH, config::HEIGHT, 0, 0, config::WIDTH, config::HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	frame_buffer::unbind();

	bool horizontal = true, first_iteration = true;
	const int amount = 10;

	blur.use();

	for(int i = 0; i < amount; i++)
	{
		pingpong_fb[horizontal].bind();

		//redundant but here to be more explicit
		glActiveTexture(GL_TEXTURE0);

		if (first_iteration)
			pingpong_color_tex[0].bind();
		else
			pingpong_color_tex[!horizontal].bind();
		

		blur.set_float("isHorizontal", horizontal);
		blur.set_int("image", 0);

		rend.draw(blur);
		
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}

	frame_buffer::unbind();
}

void render_debug_windows()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	is_open = true;
	ImGui::ShowDemoWindow(&is_open);

	ImGui::Begin("Debug Textures");

	const ImTextureID color_tex_id = reinterpret_cast<void*>(debug_fb_color_tex.get_id());  // NOLINT(misc-misplaced-const)
	const ImTextureID depth_tex_id = reinterpret_cast<void*>(debug_fb_depth_tex.get_id()); // NOLINT(misc-misplaced-const)
	const ImTextureID shadow_tex_id = reinterpret_cast<void*>(shadow_depth_tex.get_id()); // NOLINT(misc-misplaced-const)
	const ImTextureID bloom_color_tex_id = reinterpret_cast<void*>(bloom_fb_color_tex.get_id()); // NOLINT(misc-misplaced-const)
	const ImTextureID bloom_brightness_tex_id = reinterpret_cast<void*>(bloom_fb_brightness_tex.get_id()); // NOLINT(misc-misplaced-const)
	const ImTextureID blur_brightness_tex_id_0 = reinterpret_cast<void*>(pingpong_color_tex[0].get_id());  // NOLINT(misc-misplaced-const)
	const ImTextureID blur_brightness_tex_id_1 = reinterpret_cast<void*>(pingpong_color_tex[1].get_id());  // NOLINT(misc-misplaced-const)

	const float width = 400;
	const float height = 225;

	if (ImGui::TreeNode("Color"))
	{
		ImGui::Image(color_tex_id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Depth"))
	{
		ImGui::Image(depth_tex_id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Directional Shadow"))
	{
		ImGui::Image(shadow_tex_id, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Bloom FB Color"))
	{
		ImGui::Image(bloom_color_tex_id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Bloom FB Brightness"))
	{
		ImGui::Image(bloom_brightness_tex_id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Blur Brightness 0"))
	{
		ImGui::Image(blur_brightness_tex_id_0, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Blur Brightness 1"))
	{
		ImGui::Image(blur_brightness_tex_id_1, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}
	

	ImGui::End();

	ImGui::Begin("Lights");

	ImGui::Checkbox("Use Shadows", &use_shadow);
	ImGui::Checkbox("Use Normal Maps", &use_normal_maps);
	ImGui::Checkbox("Use Parallax Mapping", &use_parallax);
	ImGui::Checkbox("Use HDR", &use_hdr);
	ImGui::Checkbox("Use Gamma Correction", &use_gamma_correction);
	ImGui::Checkbox("Use Bloom", &use_bloom);

	ImGui::Spacing();

	ImGui::SliderFloat("HDR Exposure", &hdr_exposure, 0, 10, "%.3f", 1.0f);
	ImGui::SliderFloat("Brightness Threshold", &brightness_threshold, 1, 5, "%.3f", 1.0f);
	
	ImGui::Spacing();
	
	if (ImGui::TreeNode("Basic Lights"))
	{
		for (size_t i = 0; i < lights.size(); i++)
		{
			light& l = *lights[i];
			
			if (ImGui::TreeNode(l.get_name().c_str()))
			{
				ImGui::Checkbox("Enabled", &(l.is_active));
				ImGui::DragFloat3("position", &(l.get_transform()->position_ptr()->x), 0.1f);
				ImGui::ColorEdit4("diffuse", &(l.diffuse.r));
				ImGui::ColorEdit4("specular", &(l.specular.r));

				ImGui::Spacing();

				ImGui::DragFloat("specular intensity", &(l.spec_intensity));
				ImGui::DragFloat("diffuse intensity", &(l.diff_intensity));
				ImGui::TreePop();
			}
		}

		if (ImGui::TreeNode("ambient_light"))
		{
			ImGui::ColorEdit4("ambient", &ambient_color.r);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	ImGui::End();



	ImGui::Begin("Stats");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void send_dir_light_to_shader(const shader_program& program)
{
	program.use();
	if (dir_light.is_active)
	{
		program.set_vec3("dirLight.lightDir", glm::normalize(-dir_light.get_transform()->position()));
		program.set_vec3("dirLight.specularColor", dir_light.specular.to_vec3());
		program.set_vec3("dirLight.diffuseColor", dir_light.diffuse.to_vec3());
		program.set_float("dirLight.diffuseIntensity", dir_light.diff_intensity);
		program.set_float("dirLight.specularIntensity", dir_light.spec_intensity);
	}
	else
	{
		program.set_vec3("dirLight.specularColor", dir_light.specular.to_vec3() * 0.0f);
		program.set_vec3("dirLight.diffuseColor", dir_light.diffuse.to_vec3() * 0.0f);
	}
}

void send_point_lights_to_shader(const shader_program& program)
{
	program.use();
	for (int j = 0; j < 4; j++)
	{
		if (point_lights[j].is_active)
		{
			std::string  str = "pointLights[";
			str.append(std::to_string(j));
			str.append("].");

			program.set_vec3(std::string(str + "lightPos"), point_lights[j].get_transform()->position());
			program.set_vec3(std::string(str + "specularColor", j), point_lights[j].specular.to_vec3());
			program.set_vec3(std::string(str + "diffuseColor", j), point_lights[j].diffuse.to_vec3());

			program.set_float(std::string(str + "diffuseIntensity", j), point_lights[j].diff_intensity);

			program.set_float(std::string(str + "specularIntensity", j), point_lights[j].spec_intensity);

			program.set_float(std::string(str + "linear", j), point_lights[j].linear);

			program.set_float(std::string(str + "quadratic", j), point_lights[j].quadratic);
		}
		else
		{
			std::string  str = "pointLights[";
			str.append(std::to_string(j));
			str.append("].");

			program.set_vec3(std::string(str + "specularColor", j), point_lights[j].specular.to_vec3() * 0.0f);
			program.set_vec3(std::string(str + "diffuseColor", j), point_lights[j].diffuse.to_vec3() * 0.0f);
		}
	}
}

void send_spot_light_to_shader(const shader_program& program)
{
	program.use();
	if (spotlight.is_active)
	{
		spotlight.get_transform()->set_position(cam.get_transform()->position());
		spotlight.get_transform()->set_rotation(cam.get_transform()->rotation());
		spotlight.spot_direction = cam.get_transform()->forward();
	}
	
	program.set_float("isFlashlightOn", is_flash_light_on ? 1.0f : 0.0f);
	if (is_flash_light_on)
	{
		program.set_vec3("spotLight.spotDirection", cam.get_transform()->forward());
		program.set_float("spotLight.cutOffValue", glm::cos(glm::radians(spotlight.cutoff_angle)));
		program.set_float("spotLight.innerCutOffValue",
			glm::cos(glm::radians(spotlight.inner_cutoff_angle)));
		program.set_vec3("spotLight.lightPos", cam.get_transform()->position());
		program.set_vec3("spotLight.specularColor", spotlight.specular.to_vec3());
		program.set_vec3("spotLight.diffuseColor", spotlight.diffuse.to_vec3());
		program.set_float("spotLight.diffuseIntensity", spotlight.diff_intensity);
		program.set_float("spotLight.specularIntensity", spotlight.spec_intensity);
	}
}

void send_material_data_to_shader(const shader_program& program)
{
	program.use();
	program.set_float("mat.shininess", cube_mat.shininess);
	program.set_vec3("mat.specularColor", cube_mat.specular_tint.to_vec3());
	program.set_vec3("mat.diffuseColor", cube_mat.diffuse_tint.to_vec3());
	program.set_vec3("mat.specularIntensity", cube_mat.diffuse_tint.to_vec3());

	program.set_vec3("ambientColor", ambient_color.to_vec3());
	program.set_float("time", static_cast<float>(glfwGetTime()));
}

#pragma endregion

#pragma region Other functions
std::string get_tex(const std::string& path)
{
	return std::string("res/textures/").append(path);
}

void deallocate()
{
	
}

void init_imgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui::StyleColorsDark();
}

void destroy_imgui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void process_input(GLFWwindow* window)
{
	const float this_frame =static_cast<float>(glfwGetTime());
	delta_time = this_frame - last_frame;
	last_frame = this_frame;

	const float actual_speed = config::MOVE_SPEED * delta_time;
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if(cursor_mode == GLFW_CURSOR_DISABLED)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			transform* transform = cam.get_transform();
			glm::vec3 pos = transform->position();
			pos += actual_speed * transform->forward();
			transform->set_position(pos);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			transform* transform = cam.get_transform();
			glm::vec3 pos = transform->position();
			pos -= actual_speed * transform->forward();
			transform->set_position(pos);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			transform* transform = cam.get_transform();
			glm::vec3 pos = transform->position();
			pos += actual_speed * transform->right();
			transform->set_position(pos);
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			transform* transform = cam.get_transform();
			glm::vec3 pos = transform->position();
			pos -= actual_speed * transform->right();
			transform->set_position(pos);
		}
	}


	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		const float time = static_cast<float>(glfwGetTime());
		if (time - last_cursor_swap <= key_press_cooldown)
			return;
		last_cursor_swap = time;

		cursor_mode = (cursor_mode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
		first_mouse = true;
	}

	if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		const float time = static_cast<float>(glfwGetTime());
		if (time - last_flash_light_swap <= key_press_cooldown)
			return;

		last_flash_light_swap = time;

		is_flash_light_on = !is_flash_light_on;
	}
}

#pragma endregion

#pragma region Callbacks

void glfw_error_callback(int error, const char* description)
{
	std::cout << description << std::endl;
}

void frame_buffer_resize_callback(GLFWwindow* window, const int width, int const height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, const double x_pos, const double y_pos)
{
	if (cursor_mode == GLFW_CURSOR_NORMAL)
		return;
	
	if(first_mouse)
	{
		last_x = x_pos;
		last_y = y_pos;
		first_mouse = false;
	}
	
	double x_delta = x_pos - last_x;
	double y_delta = last_y - y_pos;

	x_delta *= config::MOUSE_SENSITIVITY;
	y_delta *= config::MOUSE_SENSITIVITY;
	
	last_x = x_pos;
	last_y = y_pos;

	transform* transform = cam.get_transform();
	glm::vec3 rot = transform->rotation();
	rot.x += static_cast<float>(y_delta);
	rot.y += static_cast<float>(x_delta);

	if (rot.x > 89.0f)
		rot.x = 89.0f;
	if (rot.x < -89.0f)
		rot.x = -89.0f;

	transform->set_rotation(rot);
}

void scroll_callback(GLFWwindow* window, const double x_offset, const double y_offset)
{
	float fov = cam.fov;
	
	if (fov >= 25.0f && fov <= 45.0f)
		fov -= static_cast<float>(y_offset);
	
	if (fov <= 25.0f)
		fov = 25.0f;
	if (fov >= 45.0f)
		fov = 45.0f;

	cam.fov = fov;
}

#pragma endregion