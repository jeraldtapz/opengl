#include "rendering/texture.h"
#include <GLFW/glfw3.h>
#include "engine/camera.h"
#include "rendering/frame_buffer.h"
#include "rendering/shader.h"
#include "rendering/shader_program.h"


static const unsigned int ENV_MAP_RES = 512;
static const unsigned int IRRADIANCE_RES = 32;
static const unsigned int PREFILTER_RES = 128;
static const unsigned int WIDTH = 1280;
static const unsigned int HEIGHT = 720;

static void render_cube();
static void glfw_error_callback(int error, const char* description);
static void frame_buffer_resize_callback(GLFWwindow*, int, int);
static void process_input(GLFWwindow* window);
static void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
static void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);

#pragma region Movement and Framing

static camera cam = camera(45.0f, 0.1f, 200.0f);
static float last_frame = 0.0f;
static float delta_time = 0.0f;
static double last_x = WIDTH * 0.5;
static double last_y = HEIGHT * 0.5;
static float key_press_cooldown = 0.25f;
static float last_cursor_swap = 0.0f;
static float last_flash_light_swap = 0.0f;
static int cursor_mode = GLFW_CURSOR_NORMAL;
static bool first_mouse;
static bool is_flash_light_on = false;

#pragma endregion

static std::string get_tex(const std::string& path)
{
	return std::string("res/textures/").append(path);
}

static glm::vec3 capture_forward_directions[] =
{
	glm::vec3(1.0f,  0.0f,  0.0f),
	glm::vec3(-1.0f,  0.0f,  0.0f),
	glm::vec3(0.0f,  1.0f,  0.0f),
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f,  0.0f,  1.0f),
	glm::vec3(0.0f,  0.0f, -1.0f)
};

static glm::vec3 capture_up_directions[] =
{
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f,  0.0f,  1.0f),
	glm::vec3(0.0f,  0.0f, -1.0f),
	glm::vec3(0.0f, -1.0f,  0.0f),
	glm::vec3(0.0f, -1.0f,  0.0f)
};

static glm::mat4 origin_capture_views[] =
{
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[0], capture_up_directions[0]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[1], capture_up_directions[1]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[2], capture_up_directions[2]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[3], capture_up_directions[3]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[4], capture_up_directions[4]),
   glm::lookAt(glm::vec3(0.0f), capture_forward_directions[5], capture_up_directions[5])
};

static glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
static GLFWwindow* window;

int mainz()
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
	
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	shader eq_to_cube_vertex = shader("pbr/equirectangular_to_cube_v", GL_VERTEX_SHADER);
	shader eq_to_cube_pixel = shader("pbr/equirectangular_to_cube_p", GL_FRAGMENT_SHADER);
	
	shader irradiance_vertex = shader("pbr/irradiance_v", GL_VERTEX_SHADER);
	shader irradiance_pixel = shader("pbr/irradiance_2_p", GL_FRAGMENT_SHADER);

	shader prefilter_vertex = shader("pbr/prefilter_v", GL_VERTEX_SHADER);
	shader prefilter_pixel = shader("src/testing/2.2.1.prefilter.fs", GL_FRAGMENT_SHADER, false);
	//shader prefilter_pixel = shader("pbr/prefilter_p", GL_FRAGMENT_SHADER);

	shader bg_vertex = shader("src/testing/2.2.1.background.vs", GL_VERTEX_SHADER, false);
	shader bg_pixel = shader("src/testing/2.2.1.background.fs", GL_FRAGMENT_SHADER, false);

	shader_program eq_to_cube_shader_program = shader_program(&eq_to_cube_vertex, &eq_to_cube_pixel);
	shader_program irradiance_shader_program = shader_program(&irradiance_vertex, &irradiance_pixel);
	shader_program prefilter_shader_program = shader_program(&prefilter_vertex, &prefilter_pixel);

	shader_program bg = shader_program(&bg_vertex, &bg_pixel);

	unsigned int fbo;
	unsigned int rbo;
	unsigned int hdr_texture;
	unsigned int env_map;
	unsigned int irradiance_map;
	unsigned int prefilter_map;

	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ENV_MAP_RES, ENV_MAP_RES);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	stbi_set_flip_vertically_on_load(true);
	int width, height, number_components;
	void* data = stbi_loadf("res/textures/hdr/ballroom_4k.hdr", &width, &height, &number_components, 0);
	if(data)
	{
		glGenTextures(1, &hdr_texture);
		glBindTexture(GL_TEXTURE_2D, hdr_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}


	glGenTextures(1, &env_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
	for(unsigned int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, ENV_MAP_RES, ENV_MAP_RES, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	eq_to_cube_shader_program.use();
	eq_to_cube_shader_program.set_int("equirectangularMap", 0);
	eq_to_cube_shader_program.set_proj(capture_projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdr_texture);

	glViewport(0, 0, ENV_MAP_RES, ENV_MAP_RES);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	for(int i = 0; i < 6; i++)
	{
		eq_to_cube_shader_program.set_view(origin_capture_views[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_map, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_cube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	unsigned int prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	prefilter_shader_program.use();
	prefilter_shader_program.set_int("environmentMap", 0);
	prefilter_shader_program.set_matrix("projection", capture_projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilter_shader_program.set_float("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilter_shader_program.set_matrix("view", origin_capture_views[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render_cube();
		}
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, WIDTH, HEIGHT);
	// initialize static shader uniforms before rendering
	// --------------------------------------------------
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);
	bg.use();
	bg.set_matrix("projection", cam.get_proj_matrix());

	while(!glfwWindowShouldClose(window))
	{
		process_input(window);

		glClearColor(1, 0, 0, 1);
		frame_buffer::clear_color_buffer();

		bg.use();
		bg.set_matrix("view", cam.get_view_matrix());
		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
		render_cube();
		
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
}

static void glfw_error_callback(int error, const char* description)
{
	std::cout << description << std::endl;
}

static void frame_buffer_resize_callback(GLFWwindow* window, const int width, int const height)
{
	glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow* window, const double x_pos, const double y_pos)
{
	if (cursor_mode == GLFW_CURSOR_NORMAL)
		return;

	if (first_mouse)
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

static void scroll_callback(GLFWwindow* window, const double x_offset, const double y_offset)
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

static void process_input(GLFWwindow* window)
{
	const float this_frame = static_cast<float>(glfwGetTime());
	delta_time = this_frame - last_frame;
	last_frame = this_frame;

	const float actual_speed = config::MOVE_SPEED * delta_time;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (cursor_mode == GLFW_CURSOR_DISABLED)
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


	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		const float time = static_cast<float>(glfwGetTime());
		if (time - last_cursor_swap <= key_press_cooldown)
			return;
		last_cursor_swap = time;

		cursor_mode = (cursor_mode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
		first_mouse = true;
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		const float time = static_cast<float>(glfwGetTime());
		if (time - last_flash_light_swap <= key_press_cooldown)
			return;

		last_flash_light_swap = time;

		is_flash_light_on = !is_flash_light_on;
	}
}


static unsigned int cubeVAO = 0;
static unsigned int cubeVBO = 0;
static void render_cube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}