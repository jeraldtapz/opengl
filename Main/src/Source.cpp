// ReSharper disable CppClangTidyCppcoreguidelinesNarrowingConversions
// ReSharper disable CppClangTidyBugproneNarrowingConversions
#define FB frame_buffer  // NOLINT(cppcoreguidelines-macro-usage)
#define TEX_T texture_type // NOLINT(cppcoreguidelines-macro-usage)

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
#include "data/primitive.h"
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
#include "rendering/uniform_buffer_object.h"
#include "utils/config.h"

#pragma region function declarations

void set_vp_from_camera();
std::string get_tex(const std::string& path);

void render_light_sources(model &m, const shader_program &light_shader_program);
void render_model(model& m, const shader_program& program);
void render_model(model& m, const tiling_and_offset &tiling_and_offset, const shader_program& program);
void render_asteroids(model& m, const shader_program& program);
void render_model_outline(model& m, const shader_program& program);
void render_transparent_quads(const std::vector<game_object> &quads, const renderer& rend, const shader_program& program);
void render_skybox(const renderer& rend, const shader_program& program);
void render_pp_quad(const renderer& rend, const shader_program& program);
void render_forward(const shader_program& program);
void render_directional_shadow_map(std::vector<model> &models, const shader_program& program);
void render_omnidirectional_shadow_map(std::vector<model>& models, const shader_program& program);
void bloom_postprocess(const frame_buffer& fb, const renderer& rend, const shader_program& bloom_brightness, const shader_program& blur);

void render_debug_point_lights(model& m, shader_program& program);

void render_ds_geometry(const shader_program& program);
void render_ds_dir_light_pass(const shader_program& program, model& quad);
void render_ds_point_light_pass(const shader_program& stencil_program, const shader_program& point_light_program, model& sphere);
void render_shadow_maps(std::vector<model>& models, const shader_program& dir_program, const shader_program& point_program);

void send_dir_light_to_shader(const shader_program& program);
void send_point_lights_to_shader(const shader_program& program);
void send_point_light_to_shader(const shader_program& program, const point_light& light);
void send_spot_light_to_shader(const shader_program& program);
void send_material_data_to_shader(const shader_program& program);
float calculate_point_light_radius(point_light& light);

void deallocate();
void init_imgui();
void destroy_imgui();

static void glfw_error_callback(int error, const char* description);
static void frame_buffer_resize_callback(GLFWwindow*, int, int);
static void process_input(GLFWwindow* window);
static void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
static void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);

void render_debug_windows();

#pragma endregion

#pragma region utility variables

#pragma region CONSTANTS

static const unsigned int ASTEROID_COUNT = 1000;
static const unsigned int SHADOW_RESOLUTION = 2048;
static const unsigned int ENV_MAP_RES = 512;
static const unsigned int IRRADIANCE_RES = 32;
static const unsigned int PREFILTER_RES = 128;
static const unsigned int WIDTH = 1280;
static const unsigned int HEIGHT = 720;
static const unsigned int SAMPLES = 8;
static const float RADIUS = 25.0f;

#pragma endregion

#pragma region Movement and Framing

camera cam = camera(45.0f, 0.1f, 200.0f);
float last_frame = 0.0f;
float delta_time = 0.0f;
double last_x = WIDTH * 0.5;
double last_y = HEIGHT * 0.5;
float key_press_cooldown = 0.25f;
float last_cursor_swap = 0.0f;
float last_flash_light_swap = 0.0f;

int cursor_mode = GLFW_CURSOR_NORMAL;
bool first_mouse;
bool is_flash_light_on = false;
bool is_open;

#pragma endregion


float hdr_exposure = 1.0f;
float brightness_threshold = 2.0f;
float skybox_lod = 0;
bool use_shadow = true;
bool use_normal_maps = true;
bool use_parallax = false;
bool use_gamma_correction = true;
bool use_hdr = true;
bool use_bloom = false;
bool use_deferred = false;
bool use_light_debug = false;
bool use_pbr = false;
bool use_ibl = true; // debug values

color ambient_color;
std::map<float, transform> sorted;
mvp mvp_matrix;
mvp dir_shadow_map_mvp_matrix;

//glfw
GLFWwindow* window;

//Game Related Instances
point_light point_lights[4];
directional_light dir_light;
spot_light spotlight;
material cube_mat;

std::vector<light*> lights;
std::vector<game_object*> game_objects;
std::vector<model> game_models;


FB shadow_fb = FB();
FB point_shadow_fb = FB();
FB bloom_fb = FB();
FB ms_fb = FB();
FB hdr_fb = FB();
FB destination_fb = FB();
FB debug_fb = FB();
FB pingpong_fb[] = { FB(), FB() };
FB geometry_fb = FB();
FB ds_light_fb = FB();
FB precompute_fb = FB();
//FB conv_fb = FB();

uniform_buffer_object vp_ubo;

float values[9] = { 1.0f,1.0f,1.0f,1.0f, -9.0f, 1.0f, 1.0f, 1.0f, 1.0f };
kernel3 kernel = kernel3(values, 0.0033f);

#pragma region Data

glm::vec3 capture_forward_directions[] =
{
	glm::vec3(1.0f,  0.0f,  0.0f),
	glm::vec3(-1.0f,  0.0f,  0.0f),
	glm::vec3(0.0f,  1.0f,  0.0f),
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f,  0.0f,  1.0f),
	glm::vec3(0.0f,  0.0f, -1.0f)
};

glm::vec3 capture_up_directions[] =
{
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f,  0.0f,  1.0f),
	glm::vec3(0.0f,  0.0f, -1.0f),
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f, -1.0f,  0.0f)
};

glm::mat4 origin_capture_views[] =
{
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[0], capture_up_directions[0]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[1], capture_up_directions[1]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[2], capture_up_directions[2]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[3], capture_up_directions[3]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[4], capture_up_directions[4]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[5], capture_up_directions[5])
};


#pragma endregion

#pragma endregion 

int main()  // NOLINT(bugprone-exception-escape)
{
	#pragma region Init GLFW

	glfwSetErrorCallback(glfw_error_callback);

	const int init_result = glfwInit();

	if (!init_result)
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Main", nullptr, nullptr);

	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}


	glfwMakeContextCurrent(window);

	#pragma endregion

	#pragma region GLAD Load

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	#pragma endregion

	#pragma region Viewport and Callbacks

	glViewport(0, 0, WIDTH, HEIGHT);
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

	
	glm::vec3 point_light_positions[] = {
		glm::vec3(4.0f,  7.0f,  -4.0f),
		glm::vec3(4.0f,  7.0f,  4.0f),
		glm::vec3(-4.0f,  7.0f, -4.0f),
		glm::vec3(-4.0f,  7.0f, 4.0f)
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

	shader ds_geometry_vertex = shader("ds_geometry_v", GL_VERTEX_SHADER);
	shader ds_geometry_pixel = shader("ds_geometry_p", GL_FRAGMENT_SHADER);

	shader ds_dir_light_vertex = shader("ds_dir_light_v", GL_VERTEX_SHADER);
	shader ds_dir_light_pixel = shader("ds_dir_light_p", GL_FRAGMENT_SHADER);

	shader ds_point_light_vertex = shader("ds_point_light_v", GL_VERTEX_SHADER);
	shader ds_point_light_pixel = shader("ds_point_light_p", GL_FRAGMENT_SHADER);
	
	shader ds_point_light_stcl_vertex = shader("ds_point_light_stcl_v", GL_VERTEX_SHADER);
	shader ds_point_light_stcl_pixel = shader("ds_point_light_stcl_p", GL_FRAGMENT_SHADER);

	shader point_light_debug_vertex = shader("ds_point_light_v", GL_VERTEX_SHADER);
	shader point_light_debug_pixel = shader("light_debug_p", GL_FRAGMENT_SHADER);


	shader pbr_forward_vertex = shader("pbr/pbr_forward_v", GL_VERTEX_SHADER);
	shader pbr_forward_pixel = shader("pbr/pbr_forward_P", GL_FRAGMENT_SHADER);

	shader eq_to_cube_vertex = shader("pbr/equirectangular_to_cube_v", GL_VERTEX_SHADER);
	shader eq_to_cube_pixel = shader("pbr/equirectangular_to_cube_p", GL_FRAGMENT_SHADER);

	shader irradiance_vertex = shader("pbr/irradiance_v", GL_VERTEX_SHADER);
	shader irradiance_pixel = shader("pbr/irradiance_2_p", GL_FRAGMENT_SHADER);

	shader prefilter_vertex = shader("pbr/prefilter_v", GL_VERTEX_SHADER);
	shader prefilter_pixel = shader("pbr/prefilter_p", GL_FRAGMENT_SHADER);

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

	
	shader_program ds_geometry_shader_program = shader_program(&ds_geometry_vertex, &ds_geometry_pixel);
	shader_program ds_dir_light_shader_program = shader_program(&ds_dir_light_vertex, &ds_dir_light_pixel);
	shader_program ds_point_light_shader_program = shader_program(&ds_point_light_vertex, &ds_point_light_pixel);
	shader_program ds_point_light_stcl_shader_program = shader_program(&ds_point_light_stcl_vertex, &ds_point_light_stcl_pixel);
	shader_program debug_light_shader_program = shader_program(&point_light_debug_vertex, &point_light_debug_pixel);

	shader_program pbr_forward_shader_program = shader_program(&pbr_forward_vertex, &pbr_forward_pixel);
	shader_program eq_to_cube_shader_program = shader_program(&eq_to_cube_vertex, &eq_to_cube_pixel);
	shader_program irradiance_diffuse_shader_program = shader_program(&irradiance_vertex, &irradiance_pixel);
	shader_program prefilter_shader_program = shader_program(&prefilter_vertex, &prefilter_pixel);
	
	#pragma endregion

	#pragma region Colors, Textures, Materials & Meshes

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	ambient_color = color(0.0f, 0.0f, 0.0f, 1.0f);
	cube_mat = material(color::WHITE, color::WHITE);

	#pragma region Loaded Textures
	
	const texture floor_tex = texture(get_tex("pavement/pavement_color.jpg"), TEX_T::diffuse, GL_UNSIGNED_BYTE, true);
	const texture floor_normal_tex = texture(get_tex("pavement/pavement_normal.jpg"), TEX_T::normal, GL_UNSIGNED_BYTE, true);
	const texture floor_mask_tex = texture(get_tex("pavement/pavement_mask.jpg"), TEX_T::mask, GL_UNSIGNED_BYTE, true);

	const texture canon_lens_mask_tex = texture("res/models/len_canon/textures/len_low_lambert2SG_metallicRoughness.png", TEX_T::mask, GL_UNSIGNED_BYTE, true);

	const texture viking_shield_mask_tex = texture("res/models/viking_shield/textures/lambert1_metallicRoughness.png", TEX_T::mask, GL_UNSIGNED_BYTE, true);

	texture hdri_map = texture(get_tex("hdr/ballroom_4k.hdr"), TEX_T::hdr, GL_RGB, GL_RGB16F, GL_FLOAT, false);

	std::vector<std::string> skybox_texture_paths = {
		"res/textures/skybox_5/px.png",
		"res/textures/skybox_5/nx.png" ,
		"res/textures/skybox_5/ny.png",
		"res/textures/skybox_5/py.png",
		"res/textures/skybox_5/pz.png",
		"res/textures/skybox_5/nz.png"
	};
	const texture skybox_tex = texture(skybox_texture_paths, TEX_T::cube, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 2048, false, GL_LINEAR); // textures

	texture hdri_cube_map = texture({}, TEX_T::cube, GL_RGB16F, GL_RGB, GL_FLOAT, ENV_MAP_RES, false , GL_LINEAR_MIPMAP_LINEAR);
	texture irradiance_map = texture({}, TEX_T::cube, GL_RGB16F, GL_RGB, GL_FLOAT, IRRADIANCE_RES, false, GL_LINEAR);
	texture prefilter_map = texture({}, TEX_T::cube, GL_RGB16F, GL_RGB, GL_FLOAT, PREFILTER_RES, true, GL_LINEAR_MIPMAP_LINEAR);

	#pragma endregion

	#pragma region Texture Buffers
	
	//basically unnecessary
	texture bloom_fb_color_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	texture bloom_fb_brightness_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);

	texture pingpong_color_tex[2];
	for (auto& i : pingpong_color_tex)
		i = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, true);

	texture debug_fb_color_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	texture debug_fb_depth_tex = texture(TEX_T::depth, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, false);

	texture ms_color_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, false, SAMPLES);
	texture ms_depth_tex = texture(TEX_T::depth, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, false, SAMPLES);

	texture hdr_color_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);

	texture shadow_depth_tex = texture(TEX_T::depth, SHADOW_RESOLUTION, SHADOW_RESOLUTION, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false);

	texture point_shadow_depth_tex = texture(TEX_T::cube, SHADOW_RESOLUTION, SHADOW_RESOLUTION, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false);
	
	texture destination_fb_color_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	texture destination_fb_depth_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false);



	//deferred
	texture geometry_pos_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGB, GL_RGB16F, GL_FLOAT, true);
	texture geometry_norm_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGB, GL_RGB16F, GL_FLOAT, true);
	texture geometry_diff_spec_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true);
	texture geometry_depth_stencil_tex = texture(TEX_T::depth, WIDTH, HEIGHT, GL_DEPTH_STENCIL, GL_DEPTH32F_STENCIL8, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, false);
	
	texture ds_light_color_tex = texture(TEX_T::color, WIDTH, HEIGHT, GL_RGBA, GL_RGBA16F, GL_FLOAT, false);
	texture ds_light_depth_stencil_tex = texture(TEX_T::depth, WIDTH, HEIGHT, GL_DEPTH_STENCIL, GL_DEPTH32F_STENCIL8, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, false);
	
	#pragma endregion
	
	#pragma region Meshes
	
	std::vector<texture> skybox_textures = { prefilter_map };
	mesh skybox_cube_mesh = primitive::get_cube();
	skybox_cube_mesh.replace_textures(skybox_textures);
	skybox_cube_mesh.is_indexed = false;
	skybox_cube_mesh.should_cull_face = false;
	

	std::vector<texture> floor_textures = { floor_tex, floor_mask_tex, floor_normal_tex , irradiance_map};
	mesh floor_mesh = primitive::get_quad();
	floor_mesh.replace_textures(floor_textures);
	floor_mesh.is_indexed = false;
	floor_mesh.should_cull_face = false;

	std::vector<texture> destination_quad_textures = { destination_fb_color_tex };
	mesh destination_quad_mesh = primitive::get_quad();
	destination_quad_mesh.replace_textures(destination_quad_textures);
	destination_quad_mesh.is_indexed = false;
	destination_quad_mesh.should_cull_face = false;

	mesh bloom_quad_mesh = primitive::get_quad();
	bloom_quad_mesh.is_indexed = false;
	bloom_quad_mesh.should_cull_face = true;

	mesh ds_dir_light_quad_mesh = primitive::get_quad();
	ds_dir_light_quad_mesh.is_indexed = false;
	ds_dir_light_quad_mesh.should_cull_face = true;
	ds_dir_light_quad_mesh.is_transparent = true;

	mesh ds_point_light_sphere_mesh = primitive::get_sphere();
	ds_point_light_sphere_mesh.is_indexed = true;
	ds_point_light_sphere_mesh.is_transparent = true;
	ds_point_light_sphere_mesh.should_cull_face = true;
	ds_point_light_sphere_mesh.cull_face = GL_FRONT;

	mesh debug_eq_to_cube_mesh = primitive::get_cube();
	debug_eq_to_cube_mesh.is_indexed = false;
	#pragma endregion
	
	#pragma endregion

	#pragma region Renderers and  Model

	renderer skybox_renderer = renderer(std::make_shared<mesh>(skybox_cube_mesh));
	renderer screen_space_quad_renderer = renderer(std::make_shared<mesh>(destination_quad_mesh));
	renderer screen_space_raw_quad_renderer = renderer(std::make_shared<mesh>(bloom_quad_mesh));

	model viking_shield = model("res/models/viking_shield/scene.gltf", true);
	viking_shield.set_name("Viking Shield");

	mesh* vk = viking_shield.get_mesh_ptr(0);
	if(vk)
	{
		vk->insert_texture(viking_shield_mask_tex);
		vk->insert_texture(irradiance_map);
	}
	
	
	model canon_lens = model("res/models/len_canon/scene.gltf", true);
	canon_lens.set_name("Canon Lens");

	mesh* cl = canon_lens.get_mesh_ptr(0);
	if(cl)
	{
		cl->insert_texture(canon_lens_mask_tex);
		cl->insert_texture(irradiance_map);
	}
	
	std::vector<mesh> floor_meshes = { floor_mesh };
	model floor_model = model(floor_meshes); //generated models
	floor_model.set_name("Floor");
	
	model ds_dir_light_quad_model = model({ ds_dir_light_quad_mesh });
	model ds_point_light_sphere_model = model({ ds_point_light_sphere_mesh });

	model debug_cube = model({ debug_eq_to_cube_mesh });
	debug_cube.set_name("Debug Cube");

	game_models.push_back(floor_model);
	game_models.push_back(canon_lens);
	game_models.push_back(viking_shield);

	#pragma endregion

	#pragma region FrameBuffers and RenderBuffers and Uniform Buffer Objects

	precompute_fb.generate();
	precompute_fb.bind();
	
	render_buffer precompute_rb = render_buffer(GL_DEPTH_COMPONENT24, ENV_MAP_RES, ENV_MAP_RES);
	precompute_fb.attach_render_buffer(precompute_rb, GL_DEPTH_ATTACHMENT);

	std::cout << "HDR to Cube Map Frame Buffer " << FB::validate() << std::endl;
	FB::unbind();

	/*conv_fb.generate();
	conv_fb.bind();

	render_buffer conv_rb = render_buffer(GL_DEPTH_COMPONENT24, IBL_MAP_RES, IBL_MAP_RES);
	conv_fb.attach_render_buffer(conv_rb, GL_DEPTH_ATTACHMENT);

	std::cout << "Convolution Frame Buffer " << FB::validate() << std::endl;
	FB::unbind();*/
	
	geometry_fb.generate();
	geometry_fb.bind();
	geometry_fb.attach_texture_2d_color(geometry_pos_tex, GL_COLOR_ATTACHMENT0);
	geometry_fb.attach_texture_2d_color(geometry_norm_tex, GL_COLOR_ATTACHMENT1);
	geometry_fb.attach_texture_2d_color(geometry_diff_spec_tex, GL_COLOR_ATTACHMENT2);
	geometry_fb.attach_texture_2d_depth_stencil(geometry_depth_stencil_tex, GL_DEPTH_STENCIL_ATTACHMENT);
	geometry_fb.set_draw_buffers(3, nullptr);

	std::cout << "Geometry Frame buffer " << FB::validate() << std::endl;

	FB::unbind(); // geometry fb
	
	ds_light_fb.generate();
	ds_light_fb.bind();
	ds_light_fb.attach_texture_2d_color(ds_light_color_tex, GL_COLOR_ATTACHMENT0);
	ds_light_fb.attach_texture_2d_depth_stencil(ds_light_depth_stencil_tex, GL_DEPTH_STENCIL_ATTACHMENT);
	//ds_light_fb.attach_render_buffer(ds_light_rb, GL_DEPTH_STENCIL_ATTACHMENT);

	std::cout << "Deferred Shading Frame Buffer " << FB::validate() << std::endl;
	FB::unbind(); // DS Light pass fb
	
	ms_fb.generate();
	ms_fb.bind();
	ms_fb.attach_texture_2d_color(ms_color_tex, GL_COLOR_ATTACHMENT0);
	ms_fb.attach_texture_2d_depth(ms_depth_tex, GL_DEPTH_ATTACHMENT);
	std::cout << "frame buffer with multi sampled color and depth texture " << FB::validate() << std::endl;;
	FB::unbind(); // Multi sampled fb
	
	bloom_fb.generate();
	bloom_fb.bind();
	bloom_fb.attach_texture_2d_color(bloom_fb_color_tex, GL_COLOR_ATTACHMENT0);
	bloom_fb.attach_texture_2d_color(bloom_fb_brightness_tex, GL_COLOR_ATTACHMENT1);
	bloom_fb.set_draw_buffers(2, nullptr);

	std::cout << "Bloom Frame buffer " << FB::validate() << std::endl;
	
	FB::unbind(); // bloom fb

	for(int i = 0; i < 2; i++)
	{
		pingpong_fb[i].generate();
		pingpong_fb[i].bind();

		pingpong_fb[i].attach_texture_2d_color(pingpong_color_tex[i], GL_COLOR_ATTACHMENT0);

		std::cout << "Pingpong texture " << std::to_string(i) << ", " << FB::validate() << std::endl;

		FB::unbind();
	} // pingpong fb

	shadow_fb.generate();
	shadow_fb.bind();
	shadow_fb.attach_texture_2d_depth(shadow_depth_tex, GL_DEPTH_ATTACHMENT);
	shadow_fb.set_draw_buffer(GL_NONE);
	shadow_fb.set_read_buffer(GL_NONE);
	std::cout << "shadow frame buffer with depth texture " << FB::validate() << std::endl;;
	FB::unbind(); // shadow fb

	point_shadow_fb.generate();
	point_shadow_fb.bind();
	point_shadow_fb.attach_texture(point_shadow_depth_tex, GL_DEPTH_ATTACHMENT);
	point_shadow_fb.set_draw_buffer(GL_NONE);
	point_shadow_fb.set_read_buffer(GL_NONE);

	std::cout << "frame buffer for point shadow " << FB::validate() << std::endl;
	FB::unbind(); // point shadow fb

	hdr_fb.generate();
	hdr_fb.bind();
	hdr_fb.attach_texture_2d_color(hdr_color_tex, GL_COLOR_ATTACHMENT0);
	
	render_buffer hdr_rb = render_buffer(GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	hdr_fb.attach_render_buffer(hdr_rb, GL_DEPTH_STENCIL_ATTACHMENT);

	std::cout << "HDR frame buffer " << FB::validate() << std::endl;

	FB::unbind(); // hdr fb

	debug_fb.generate();
	debug_fb.bind();

	debug_fb.attach_texture_2d_color(debug_fb_color_tex, GL_COLOR_ATTACHMENT0);
	//debug_fb.attach_texture_2d_stencil(debug_fb_stencil_tex, GL_STENCIL_ATTACHMENT);
	debug_fb.attach_texture_2d_depth(debug_fb_depth_tex, GL_DEPTH_ATTACHMENT);

	std::cout << "Debug Frame Buffer " << FB::validate() << std::endl;
	FB::unbind(); // debug fb

	destination_fb.generate();
	destination_fb.bind();

	destination_fb.attach_texture_2d_color(destination_fb_color_tex, GL_COLOR_ATTACHMENT0);
	destination_fb.attach_texture_2d_depth(destination_fb_depth_tex, GL_DEPTH_ATTACHMENT);

	std::cout << "Destination Frame Buffer " << FB::validate() << std::endl;

	FB::unbind(); //destination fb

	vp_ubo = uniform_buffer_object(2 * sizeof(glm::mat4), GL_STATIC_DRAW);

	//bind ubo to binding point 1
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, vp_ubo.get_id());

	//in opengl 4.x binding point can be specified in shader
	//bind shader program to binding point 1
	unsigned int vp_index = glGetUniformBlockIndex(basic_shader_program.id, "VP");
	glUniformBlockBinding(basic_shader_program.id, vp_index, 1); // UBOs
	glUniformBlockBinding(basic_shader_program_2.id, vp_index, 1); // UBOs
	glUniformBlockBinding(basic_shader_program_3.id, vp_index, 1); // UBOs
	
	#pragma endregion

	#pragma region Camera Initialization

	cam.get_transform()->set_rotation(glm::vec3(-45, 90,0));
	cam.get_transform()->set_position(glm::vec3(0, 4, -2));
	
	#pragma endregion

	#pragma region Initialize Game Objects and Lights
	
	dir_light = directional_light();
	dir_light.set_name("directional_light");
	dir_light.get_transform()->set_position(glm::vec3(0, 1, 4));
	dir_light.diff_intensity = 10.0f;
	game_objects.push_back(&dir_light);
	lights.push_back(&dir_light);
	dir_shadow_map_mvp_matrix.projection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 1.0f, 200.f);
	
	for (size_t i = 0; i < 4 ; i++)
	{
		point_lights[i].get_transform()->set_position(point_light_positions[i]);
		point_lights[i].get_transform()->set_scale(glm::vec3(5.0f));
		point_lights[i].linear = 0.0f;
		point_lights[i].set_uniform_scale();
		point_lights[i].set_radius_from_scale();
		point_lights[i].diff_intensity = 150.0f;
		point_lights[i].set_name(std::string("point_light_").append(std::to_string(i)));
		game_objects.push_back(&point_lights[i]);
		lights.push_back(&point_lights[i]);
	}

	spotlight = spot_light();
	spotlight.diffuse = color(250 / 255.0f, 1.0f, 107 / 255.0f, 1.0f);
	spotlight.diffuse = color(250 / 255.0f, 1.0f, 107 / 255.0f, 1.0f);
	spotlight.diff_intensity = 50.0f;
	
	spotlight.set_name("spot_light");
	lights.push_back(&spotlight);
	game_objects.push_back(&spotlight);
	

	floor_model.get_transform()->set_rotation(glm::vec3(-90.0f, 0.0f, 0.0f));
	floor_model.get_transform()->set_scale(glm::vec3(15, 15, 1));

	canon_lens.get_transform()->set_position(glm::vec3(3, 5, 0));

	viking_shield.get_transform()->set_position(glm::vec3(4, 2, 4));
	viking_shield.get_transform()->set_rotation(glm::vec3(90, 0, 0));
	viking_shield.get_transform()->set_scale(glm::vec3(0.1f));

	#pragma endregion

	#pragma region Environment Map Precompute

	glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	precompute_fb.bind();
	
	glViewport(0, 0, 512, 512);
	
	eq_to_cube_shader_program.use();
	eq_to_cube_shader_program.set_int("equirectangularMap", 0);
	eq_to_cube_shader_program.set_proj(capture_proj);

	texture::activate(GL_TEXTURE0);
	hdri_map.bind();
	
	for(int i = 0; i < 6; i++)
	{
		eq_to_cube_shader_program.set_view(origin_capture_views[i]);
		precompute_fb.attach_texture_2d_color(hdri_cube_map, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0);

		std::cout << "Converting HDR to cube face " << i << " " << FB::validate() << std::endl;
		FB::clear_frame();

		//renderCube();
		debug_cube.draw(eq_to_cube_shader_program);
	} // convert .hdr to cubemap

	hdri_cube_map.bind();
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // hdr to cube map
	
	
	glViewport(0, 0, IRRADIANCE_RES, IRRADIANCE_RES);
	precompute_rb.reallocate(GL_DEPTH_COMPONENT24, IRRADIANCE_RES, IRRADIANCE_RES);
	
	irradiance_diffuse_shader_program.use();
	irradiance_diffuse_shader_program.set_int("environmentMap", 0);
	irradiance_diffuse_shader_program.set_proj(capture_proj);

	texture::activate(GL_TEXTURE0);
	hdri_cube_map.bind();

	for(int i = 0; i < 6; i++)
	{
		irradiance_diffuse_shader_program.set_view(origin_capture_views[i]);
		precompute_fb.attach_texture_2d_color(irradiance_map, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0);
		std::cout << "Convoluting cube face " << i << " " << FB::validate() << std::endl;
		FB::clear_frame();

		debug_cube.draw(irradiance_diffuse_shader_program);
	}
	// precompute irradiance

	
	prefilter_shader_program.use();
	prefilter_shader_program.set_int("environmentMap", 0);
	prefilter_shader_program.set_proj(capture_proj);
	texture::activate(GL_TEXTURE0);
	hdri_cube_map.bind();
	
	precompute_fb.bind();
	unsigned int max_mip_level = 5;

	for(unsigned int mip = 0; mip < max_mip_level; mip++)
	{
		unsigned int mip_res = PREFILTER_RES * std::pow(0.5, mip);
		precompute_rb.reallocate(GL_DEPTH_COMPONENT24, mip_res, mip_res);
		glViewport(0, 0, mip_res, mip_res);

		float roughness = static_cast<float>(mip) / static_cast<float>(max_mip_level - 1);
		prefilter_shader_program.set_float("roughness", roughness);

		for(unsigned int i = 0; i < 6; i++)
		{
			prefilter_shader_program.set_view(origin_capture_views[i]);
			precompute_fb.attach_texture_2d_color(prefilter_map, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map.get_id(), mip);

			std::cout << "Convoluting Prefilter, MIP:  " << mip << ", Face: " << i << " " << FB::validate() << std::endl;
			
			FB::clear_color_buffer();
			FB::clear_depth_buffer();

			debug_cube.draw(prefilter_shader_program);
		}
	}
	// prefilter env map

	
	FB::unbind();
	glViewport(0, 0, WIDTH, HEIGHT);
	#pragma endregion

	#pragma region Loop

	while(!glfwWindowShouldClose(window))
	{
		process_input(window);

		set_vp_from_camera();

		render_shadow_maps(game_models, shadow_shader_program, point_shadow_shader_program);
		
		if(use_deferred)
		{
			render_ds_geometry(ds_geometry_shader_program);
			
			ds_light_fb.bind();
			glBlendFunc(GL_ONE, GL_ONE);
			FB::clear_color_buffer();

			render_ds_dir_light_pass(ds_dir_light_shader_program, ds_dir_light_quad_model);
			render_ds_point_light_pass(ds_point_light_stcl_shader_program, ds_point_light_shader_program, ds_point_light_sphere_model);
			render_skybox(skybox_renderer, skybox_shader_program);
			
			render_debug_point_lights(ds_point_light_sphere_model, debug_light_shader_program);
			FB::set_depth_testing(true);
			FB::set_depth_writing(true);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			render_forward(use_pbr ? pbr_forward_shader_program : basic_shader_program);
			render_skybox(skybox_renderer, skybox_shader_program);
			render_debug_point_lights(ds_point_light_sphere_model, debug_light_shader_program);
		}

		//blit to bloom buffer
		bloom_fb.bind_draw();
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		
		destination_fb.bind_draw();
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

		debug_fb.bind_draw();
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		FB::unbind(); // blit

		if (use_bloom)
		{
			if(use_deferred)
				bloom_postprocess(ds_light_fb, screen_space_raw_quad_renderer, bloom_brightness_shader_program, blur_shader_program);
			else
				bloom_postprocess(use_hdr ? hdr_fb : ms_fb, screen_space_raw_quad_renderer, bloom_brightness_shader_program, blur_shader_program);
		}
		
		FB::clear_frame();
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

#pragma region Render Functions

void render_light_sources(model& m, const shader_program& light_shader_program)
{
	light_shader_program.use();

 	for (auto& i : lights)
    {
		light& l = (*i);
 		
		if (l.get_name() == "spot_light" || !l.is_active)
			continue;

		light_shader_program.set_vec3("color", l.diffuse.to_vec3() * l.diff_intensity);
		m.get_transform()->set_position(l.get_transform()->position());

		render_model(m, light_shader_program);
	}
}

void render_model(model &m, const shader_program &program)
{
	FB::set_depth_writing(true);
	FB::set_depth_testing(true);
	FB::set_stencil_writing(false);
	FB::set_stencil_testing(false);
	
	program.use();
	program.set_float("useShadow", use_shadow ? 1 : 0);
	program.set_float("useNormalMaps", use_normal_maps ? 1 : 0);
	program.set_float("useParallax", use_parallax ? 1 : 0);
	program.set_float("useIBL", use_ibl ? 1 : 0);
	program.set_vec3("viewPos", cam.get_transform()->position());
	
	send_point_lights_to_shader(program);
	send_dir_light_to_shader(program);
	send_spot_light_to_shader(program);
	send_material_data_to_shader(program);
	

	program.set_model(m.get_transform()->get_model_matrix());

	program.set_matrix("lightView", dir_shadow_map_mvp_matrix.view);
	program.set_matrix("lightProjection", dir_shadow_map_mvp_matrix.projection);

	/*glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.get_id());
	program.set_int("mat.reflectionTexture0", 8);*/

	if(use_shadow)
	{
		texture::activate(GL_TEXTURE7);
		shadow_fb.get_depth_attachment_tex()->bind();
		program.set_int("mat.shadowMap0", 7);

		texture::activate(GL_TEXTURE8);
		point_shadow_fb.get_depth_attachment_tex()->bind();
		program.set_int("pointShadowMap", 8);
		
		//program.set_vec3("pointLights[0].lightPos", point_lights[0].get_transform()->position());
	}
	program.set_float("farPlane", RADIUS);

	/*FB::set_stencil_writing(true);
	FB::set_stencil_op(GL_KEEP, GL_KEEP, GL_REPLACE);
	FB::set_stencil_func(GL_ALWAYS, 1, 0xFF);*/
	
	m.draw(program);

	/*FB::set_stencil_writing(false);
	FB::set_stencil_op(GL_KEEP, GL_KEEP, GL_KEEP);
	FB::set_stencil_func(GL_ALWAYS, 1, 0xFF);*/
}

void render_model(model& m, const tiling_and_offset& tiling_and_offset, const shader_program& program)
{
	program.use();
	program.set_tiling_and_offset(tiling_and_offset);
	render_model(m, program);
}

void render_asteroids(model& m, const shader_program& program)
{
	FB::set_stencil_testing(false);
	FB::set_stencil_writing(false);

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

void render_transparent_quads(const std::vector<game_object> &quads, const renderer& rend, const shader_program& program)
{
	FB::set_stencil_testing(false);
	FB::set_stencil_writing(false);
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

	FB::set_stencil_testing(true);
	FB::set_stencil_writing(true);
}

void render_skybox(const renderer& rend, const shader_program& program)
{
	FB::set_depth_testing(true);
	FB::set_depth_writing(false);
	
	glDepthFunc(GL_LEQUAL);
	mvp_matrix.view = glm::mat4(glm::mat3(cam.get_view_matrix()));
	program.use();
	program.set_mvp(mvp_matrix);
	program.set_float("lod", skybox_lod);
	rend.draw_cube_map(program);
	
	FB::set_depth_writing(true);
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
	program.set_float("useBloom", use_bloom);

	if(use_bloom)
	{
		texture::activate(GL_TEXTURE1);
		pingpong_fb[1].get_color_attachment_tex(GL_COLOR_ATTACHMENT0)->bind();
		//glActiveTexture(GL_TEXTURE1);
		//pingpong_color_tex[1].bind();
		program.set_int("bloomBlur", 1);
	}
	else
	{
		texture::activate(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	rend.draw(program);
}

void render_forward(const shader_program& program)
{
	if (use_hdr)
		hdr_fb.bind();
	else
		ms_fb.bind();

	FB::clear_frame();
	
	for (auto && game_model : game_models)
	{
		auto model_name = game_model.get_name();

		tiling_and_offset t;

		if (model_name == "Floor")
		{
			t.tiling = glm::vec2(game_model.get_transform()->scale());
			t.offset = glm::vec2(0);
		}
		else
			t = tiling_and_offset();

		if (glm::abs(t.tiling.x) < 1)
			t.tiling.x = 1;

		if (glm::abs(t.tiling.y) < 1)
			t.tiling.y = 1;
		
		render_model(game_model, t, program);
	}
}

void render_shadow_maps(std::vector<model>& models, const shader_program& dir_program, const shader_program& point_program)
{
	if(use_shadow)
	{
		render_directional_shadow_map(models, dir_program);
		render_omnidirectional_shadow_map(models, point_program);
	}
}

void render_directional_shadow_map(std::vector<model>& models, const shader_program& program)
{
	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	shadow_fb.bind();

	FB::clear_depth_buffer();

	dir_shadow_map_mvp_matrix.view = glm::lookAt(dir_light.get_transform()->position(), glm::vec3(0), glm::vec3(0, 1, 0));

	program.use();
	program.set_view(dir_shadow_map_mvp_matrix.view);
	program.set_proj(dir_shadow_map_mvp_matrix.projection);

	glCullFace(GL_FRONT);

	for (auto value : models)
	{
		program.set_model(value.get_transform()->get_model_matrix());
		value.draw_shadow(program);
	}

	glCullFace(GL_BACK);

	FB::unbind();

	glViewport(0, 0, WIDTH, HEIGHT);

}

void render_omnidirectional_shadow_map(std::vector<model>& models, const shader_program &program)
{
	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);

	point_shadow_fb.bind();
	FB::clear_depth_buffer();
	FB::clear_color_buffer();

	const glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, RADIUS);

	const glm::vec3 pos = point_lights[0].get_transform()->position();
	std::vector<glm::mat4> shadow_view_matrices;
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + capture_forward_directions[0], capture_up_directions[0]));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + capture_forward_directions[1], capture_up_directions[1]));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + capture_forward_directions[2], capture_up_directions[2]));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + capture_forward_directions[3], capture_up_directions[3]));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + capture_forward_directions[4], capture_up_directions[4]));
	shadow_view_matrices.push_back(glm::lookAt(pos, pos + capture_forward_directions[5], capture_up_directions[5]));

	program.use();
	program.set_float("farPlane",RADIUS);
	program.set_vec3("lightPos", pos);
	program.set_matrix("lightProj", proj);
	
	for (size_t i = 0; i < shadow_view_matrices.size(); i++)
	{
		program.set_matrix(std::string("lightView[").append(std::to_string(i).append("]")), shadow_view_matrices[i]);
	}

	glCullFace(GL_FRONT);
	for (std::vector<model>::value_type& value : models)
	{
		program.set_model(value.get_transform()->get_model_matrix());
		value.draw_shadow(program);
		
	}
	glCullFace(GL_BACK);
	
	FB::unbind();

	glViewport(0, 0, WIDTH, HEIGHT);
}

void bloom_postprocess(const frame_buffer& fb, const renderer& rend, const shader_program &bloom_brightness, const shader_program &blur)
{
	bloom_fb.bind();
	bloom_brightness.use();

	texture* pre_bloom_color_tex = fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT0);

	texture::activate(GL_TEXTURE0);
	if (pre_bloom_color_tex)
		pre_bloom_color_tex->bind();
	
	bloom_brightness.set_float("useBloom", use_bloom);
	bloom_brightness.set_float("brightnessThreshold", brightness_threshold);
	rend.draw(bloom_brightness); // render quad to extract bright pixels

	//copy extracted bright pixels to first pingpong fbo
	bloom_fb.bind_read();
	pingpong_fb[0].bind_draw();
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	FB::unbind();

	bool horizontal = true, first_iteration = true;
	const int amount = 10;

	blur.use();

	for(int i = 0; i < amount; i++)
	{
		pingpong_fb[horizontal].bind();

		//redundant but here to be more explicit
		texture::activate(GL_TEXTURE0);

		if (first_iteration)
			pingpong_fb[0].get_color_attachment_tex(GL_COLOR_ATTACHMENT0)->bind();
		else
			pingpong_fb[!horizontal].get_color_attachment_tex(GL_COLOR_ATTACHMENT0)->bind();

		blur.set_float("isHorizontal", horizontal);
		blur.set_int("image", 0);

		rend.draw(blur);
		
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}

	FB::unbind();
}

void render_debug_point_lights(model& m, shader_program& program)
{
	if (!use_light_debug)
		return;
	
	FB::enable_depth_testing();
	FB::disable_stencil_testing();
	
	program.use();
	
	m.get_mesh_ptr(0)->should_cull_face = false;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (auto& point_light : point_lights)
	{
		if(point_light.is_active)
		{
			m.get_transform()->set_position(point_light.get_transform()->position());
			m.get_transform()->set_scale(glm::vec3(point_light.radius));
			program.set_model(m.get_transform()->get_model_matrix());
			program.set_vec3("color", point_light.diffuse.to_vec3());
			m.draw(program);
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void render_debug_windows()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow(&is_open);

	ImGui::Begin("Debug Textures");

	const ImTextureID color_tex_id = reinterpret_cast<void*>(debug_fb.get_color_attachment(GL_COLOR_ATTACHMENT0));  // NOLINT(misc-misplaced-const)
	const ImTextureID depth_tex_id = reinterpret_cast<void*>(debug_fb.get_depth_attachment()); // NOLINT(misc-misplaced-const)
	const ImTextureID bloom_color_tex_id = reinterpret_cast<void*>(bloom_fb.get_color_attachment(GL_COLOR_ATTACHMENT0)); // NOLINT(misc-misplaced-const)

	const ImTextureID g_pos = reinterpret_cast<void*>(geometry_fb.get_color_attachment(GL_COLOR_ATTACHMENT0));// NOLINT(misc-misplaced-const)
	const ImTextureID g_normal = reinterpret_cast<void*>(geometry_fb.get_color_attachment(GL_COLOR_ATTACHMENT1));// NOLINT(misc-misplaced-const)
	const ImTextureID g_diff_spec = reinterpret_cast<void*>(geometry_fb.get_color_attachment(GL_COLOR_ATTACHMENT2));// NOLINT(misc-misplaced-const)

	const ImTextureID g_depth = reinterpret_cast<void*>(geometry_fb.get_depth_attachment());// NOLINT(misc-misplaced-const)

	const ImTextureID ds_dir_light = reinterpret_cast<void*>(ds_light_fb.get_color_attachment(GL_COLOR_ATTACHMENT0));// NOLINT(misc-misplaced-const)

	const ImTextureID ds_light_depth = reinterpret_cast<void*>(ds_light_fb.get_depth_attachment());// NOLINT(misc-misplaced-const)

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
	
	if(ImGui::TreeNode("Bloom FB Color"))
	{
		ImGui::Image(bloom_color_tex_id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Deferred Shading"))
	{
		ImGui::Image(ds_dir_light, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Deferred Shading Depth"))
	{
		ImGui::Image(ds_light_depth, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if(ImGui::TreeNode("G Position"))
	{
		ImGui::Image(g_pos, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("G Normal"))
	{
		ImGui::Image(g_normal, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("G DiffSpec"))
	{
		ImGui::Image(g_diff_spec, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("G Depth"))
	{
		ImGui::Image(g_depth, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
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
	ImGui::Checkbox("Use Deferred", &use_deferred);
	ImGui::Checkbox("Use Light Debug", &use_light_debug);
	ImGui::Checkbox("Use PBR", &use_pbr);
	ImGui::Checkbox("Use IBL", &use_ibl);
	

	ImGui::Spacing();

	ImGui::SliderFloat("Skybox LOD", &skybox_lod, 0, 5, "%.3f", 1.0f);
	ImGui::SliderFloat("HDR Exposure", &hdr_exposure, 0, 10, "%.3f", 1.0f);
	ImGui::SliderFloat("Brightness Threshold", &brightness_threshold, 1, 100, "%.3f", 1.0f);
	
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
				
				ImGui::DragFloat3("scale", &(l.get_transform()->scale_ptr()->x), 0.1f);
				
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

	for (auto& point_light : point_lights)
	{
		point_light.set_uniform_scale();
		point_light.set_radius_from_scale();
	}
	
	ImGui::End();

	ImGui::Begin("Game Objects");

	for (auto &game_model : game_models)
	{
		if (ImGui::TreeNode(game_model.get_name().c_str()))
		{
			ImGui::Checkbox("Enabled", &(game_model.is_active));
			ImGui::DragFloat3("position", &(game_model.get_transform()->position_ptr()->x), 0.1f);
			ImGui::DragFloat3("rotation", &(game_model.get_transform()->rotation_ptr()->x), 0.1f);
			ImGui::DragFloat3("scale", &(game_model.get_transform()->scale_ptr()->x), 0.1f);

			ImGui::Spacing();

			ImGui::TreePop();
		}
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
			program.set_vec3(std::string(str + "specularColor"), point_lights[j].specular.to_vec3());
			program.set_vec3(std::string(str + "diffuseColor"), point_lights[j].diffuse.to_vec3());

			program.set_float(std::string(str + "diffuseIntensity"), point_lights[j].diff_intensity);

			program.set_float(std::string(str + "specularIntensity"), point_lights[j].spec_intensity);

			program.set_float(std::string(str + "linear"), point_lights[j].linear);

			program.set_float(std::string(str + "quadratic"), point_lights[j].quadratic);
		}
		else
		{
			std::string  str = "pointLights[";
			str.append(std::to_string(j));
			str.append("].");

			program.set_vec3(std::string(str + "specularColor"), point_lights[j].specular.to_vec3() * 0.0f);
			program.set_vec3(std::string(str + "diffuseColor"), point_lights[j].diffuse.to_vec3() * 0.0f);
		}
	}
}

void send_point_light_to_shader(const shader_program& program, const point_light& light)
{
	program.use();
	if (light.is_active)
	{
		const std::string  str = "pointLight.";

		program.set_vec3(std::string(str + "lightPos"), light.get_transform()->position());
		program.set_vec3(std::string(str + "specularColor"), light.specular.to_vec3());
		program.set_vec3(std::string(str + "diffuseColor"), light.diffuse.to_vec3());

		program.set_float(std::string(str + "diffuseIntensity"), light.diff_intensity);

		program.set_float(std::string(str + "specularIntensity"), light.spec_intensity);

		program.set_float(std::string(str + "linear"), light.linear);

		program.set_float(std::string(str + "quadratic"), light.quadratic);
	}
	else
	{
		const std::string  str = "pointLight.";

		program.set_vec3(std::string(str + "specularColor"), light.specular.to_vec3() * 0.0f);
		program.set_vec3(std::string(str + "diffuseColor"), light.diffuse.to_vec3() * 0.0f);
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

float calculate_point_light_radius(point_light& light)
{
	const float max_channel = fmax(fmax(light.diffuse.r, light.diffuse.g), light.diffuse.b);

	const float ret = (-light.linear + sqrtf(light.linear * light.linear -
		4.0f * light.quadratic * (1.0f - (256.0f/0.5f) * max_channel * light.diff_intensity)))
		/
		(2.0f * light.quadratic);
	return ret;
}

#pragma endregion

#pragma region Deferred Shading

void render_ds_geometry(const shader_program& program)
{
	geometry_fb.bind();
	FB::clear_frame();

	program.use();
	program.set_float("useNormalMaps", use_normal_maps);
	program.set_float("useParallax", use_parallax);
	program.set_vec3("viewPos", cam.get_transform()->position());
	
	for (auto& game_model : game_models)
	{
		tiling_and_offset t = tiling_and_offset{ glm::vec2(game_model.get_transform()->scale()), glm::vec2(0) };

		if (glm::abs(t.tiling.x) < 1)
			t.tiling.x = 1;

		if (glm::abs(t.tiling.y) < 1)
			t.tiling.y = 1;
		
		program.set_model(game_model.get_transform()->get_model_matrix());
		program.set_tiling_and_offset(t);
		game_model.draw(program);
	}
	
	/*ds_geometry_shader_program.set_model(barrel_model.get_transform()->get_model_matrix());
	barrel_model.draw(ds_geometry_shader_program);

	ds_geometry_shader_program.set_model(pistol_model.get_transform()->get_model_matrix());
	pistol_model.draw(ds_geometry_shader_program);

	ds_geometry_shader_program.set_model(floor_model.get_transform()->get_model_matrix());
	ds_geometry_shader_program.set_tiling_and_offset(floor_tiling_and_offset);
	floor_model.draw(ds_geometry_shader_program);*/

	debug_fb.bind_draw();
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ds_light_fb.bind_draw();
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

	FB::unbind();
}

void render_ds_dir_light_pass(const shader_program& program, model& quad)
{
	if (dir_light.is_active)
	{
		FB::set_depth_testing(false);
		FB::set_depth_writing(false);
		program.use();

		send_dir_light_to_shader(program);
		program.set_vec3("viewPos", cam.get_transform()->position());
		program.set_matrix("lightView", dir_shadow_map_mvp_matrix.view);
		program.set_matrix("lightProjection", dir_shadow_map_mvp_matrix.projection);
		program.set_int("gPos", 0);
		program.set_int("gNormal", 1);
		program.set_int("gDiffSpec", 2);
		program.set_int("shadowMap", 3);
		program.set_float("useShadow", use_shadow);

		texture::activate(GL_TEXTURE0);
		geometry_fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT0)->bind();

		texture::activate(GL_TEXTURE1);
		geometry_fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT1)->bind();

		texture::activate(GL_TEXTURE2);
		geometry_fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT2)->bind();

		texture::activate(GL_TEXTURE3);
		shadow_fb.get_depth_attachment_tex()->bind();

		quad.draw(program);
	}

}

void render_ds_point_light_pass(const shader_program& stencil_program, const shader_program& point_light_program, model& sphere)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		if (point_lights[i].is_active)
		{
			FB::clear_stencil_buffer();
			FB::enable_depth_testing();
			FB::set_depth_writing(false);
			FB::enable_stencil_testing();
			FB::set_stencil_writing(true);

			sphere.get_mesh_ptr(0)->should_cull_face = false;

			FB::set_stencil_func(GL_ALWAYS, 0, 0);
			FB::set_stencil_op_sep(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			FB::set_stencil_op_sep(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);


			stencil_program.use();
			sphere.get_transform()->set_position(point_lights[i].get_transform()->position());
			sphere.get_transform()->set_scale(point_lights[i].get_transform()->scale());
			stencil_program.set_model(sphere.get_transform()->get_model_matrix());
			sphere.draw(stencil_program); // stencil pass

			FB::enable_stencil_testing();
			FB::disable_depth_testing();
			FB::set_stencil_func(GL_NOTEQUAL, 0, 0xFF);
			FB::set_stencil_writing(false);
			FB::set_depth_writing(false);
			sphere.get_mesh_ptr(0)->should_cull_face = true;
			sphere.get_mesh_ptr(0)->cull_face = GL_FRONT;


			point_light_program.use();
			send_point_light_to_shader(point_light_program, point_lights[i]);
			point_light_program.set_vec3("viewPos", cam.get_transform()->position());
			point_light_program.set_int("gPos", 0);
			point_light_program.set_int("gNormal", 1);
			point_light_program.set_int("gDiffSpec", 2);
			point_light_program.set_int("pointShadowMap", 3);
			point_light_program.set_float("useShadow", i == 0);
			point_light_program.set_float("farPlane", RADIUS);
			point_light_program.set_float("useDebug", false);


			point_light_program.set_model(sphere.get_transform()->get_model_matrix());

			texture::activate(GL_TEXTURE0);
			geometry_fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT0)->bind();

			texture::activate(GL_TEXTURE1);
			geometry_fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT1)->bind();

			texture::activate(GL_TEXTURE2);
			geometry_fb.get_color_attachment_tex(GL_COLOR_ATTACHMENT2)->bind();

			texture::activate(GL_TEXTURE3);
			point_shadow_fb.get_depth_attachment_tex()->bind();

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphere.draw(point_light_program); // lighting pass

			FB::clear_stencil_buffer();
			FB::disable_stencil_testing();
			FB::set_depth_writing(false);
			FB::enable_depth_testing();


			//if(use_light_debug)
			//{
			//	sphere.get_mesh_ptr(0)->should_cull_face = false;
			//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			//	point_light_program.set_float("useDebug", true);
			//	sphere.draw(point_light_program);
			//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//} // debug visual pass
			
			FB::set_depth_writing(true);
			FB::set_stencil_writing(false);
		}
	}
}

#pragma endregion

#pragma region Other functions

void set_vp_from_camera()
{
	mvp_matrix.view = cam.get_view_matrix();
	mvp_matrix.projection = cam.get_proj_matrix();
	vp_ubo.buffer_data_range(0, sizeof(glm::mat4), glm::value_ptr(mvp_matrix.view));
	vp_ubo.buffer_data_range(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(mvp_matrix.projection)); // view projection
}

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