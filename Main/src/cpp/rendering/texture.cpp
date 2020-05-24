#include "rendering/texture.h"

#include <iostream>

#include "stb_image.h"

texture::texture() = default;

//used for loading textures from disk
texture::texture(const std::string& absolute_path, const texture_type type, const GLenum data_format, 
                 const bool generate_mipmaps)
{
	id = 0;
	
	this->wrap_mode = GL_REPEAT;
	this->filter_mag = GL_LINEAR;
	this->filter_min = GL_LINEAR;
	
	this->type = type;
	is_multi_sampled = false;

	glGenTextures(1, &id);

	this->bind();

	int temp_width;
	int temp_height;
	int temp_number_of_channels;

	stbi_set_flip_vertically_on_load(true);
	stbi_uc* const data = stbi_load(absolute_path.c_str(), &temp_width, &temp_height, &temp_number_of_channels, 0);

	if(data)
	{
		this->width = temp_width;
		this->height = temp_height;
		this->channels = temp_number_of_channels;

		GLenum format = 0;
		GLenum internal_format = 0;

		if(channels == 3)
		{
			format = GL_RGB;
			internal_format = type == texture_type::diffuse ? GL_SRGB : GL_RGB;

		}
		else if(channels == 4)
		{
			format = GL_RGBA;
			internal_format = type == texture_type::diffuse ? GL_RGBA : GL_RGBA;
			//internal_format = type == texture_type::diffuse ? GL_SRGB : GL_RGB;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, data_format, data);
		if (generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		
		set_wrap_mode(channels == 3 ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		set_filter_mag(GL_LINEAR);
		set_filter_min(GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		std::cout << "ERROR: FAILED TO LOAD TEXTURE" << std::endl;
	}

	stbi_image_free(data);
}

texture::texture(const texture_type type, const unsigned int width, const unsigned int height, const GLenum format, const GLenum internal_format, const GLenum data_format, const bool generate_mipmaps)
{
	id = 0;

	if(type == texture_type::cube)
	{
		this->wrap_mode = GL_CLAMP_TO_EDGE;
		this->filter_mag = GL_NEAREST;
		this->filter_min = GL_NEAREST;
	}
	else
	{
		this->wrap_mode = GL_REPEAT;
		this->filter_mag = GL_LINEAR;
		this->filter_min = GL_LINEAR;
	}
	
	this->width = width;
	this->height = height;
	
	this->type = type;
	is_multi_sampled = false;

	glGenTextures(1, &id);

	bind();
	if(type == texture_type::cube)
	{
		for (int i = 0;i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0, format, data_format, nullptr);
		}
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, data_format, nullptr);
	
	if (generate_mipmaps)
		glGenerateMipmap(type == texture_type::cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D);

	switch (format)
	{
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_FLOAT:
			channels = 1;
			break;

		case GL_RG:
			channels = 2;
			break;

		case GL_RGB:
			channels = 3;
			break;

		case GL_RGBA:
			channels = 4;
			break;

		default:
			channels = 1;
			break;
	}
	
	set_wrap_mode(wrap_mode);
	set_filter_mag(filter_mag);
	set_filter_min(filter_min);
}

texture::texture(const texture_type type, const unsigned int width, const unsigned int height, const GLenum format, GLenum const internal_format, const GLenum data_format, const bool generate_mipmaps, const unsigned int samples)
{
	id = 0;
	this->wrap_mode = GL_REPEAT;
	this->filter_mag = GL_LINEAR;
	this->filter_min = GL_LINEAR;
	this->width = width;
	this->height = height;

	this->type = type;
	is_multi_sampled = true;

	glGenTextures(1, &id);

	bind();

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height, GL_TRUE);

	if (generate_mipmaps)
		glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);

	switch (format)
	{
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
		channels = 1;
		break;

	case GL_RG:
		channels = 2;
		break;

	case GL_RGB:
		channels = 3;
		break;

	case GL_RGBA:
		channels = 4;
		break;

	default:
		channels = 0;
		break;
	}

	set_wrap_mode(wrap_mode);
	set_filter_mag(filter_mag);
	set_filter_min(filter_min);
}


//used for cube_maps
texture::texture(const std::string paths[], const texture_type type, const GLenum internal_format, const GLenum format, const GLenum data_format)
{
	id = 0;
	
	this->wrap_mode = GL_CLAMP_TO_EDGE;
	this->filter_mag = GL_LINEAR;
	this->filter_min = GL_LINEAR;

	this->type = type;
	is_multi_sampled = false;

	glGenTextures(1, &id);
	bind();
	//glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	int temp_width;
	int temp_height;
	int temp_number_of_channels;
	stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < 6; i++)
	{
		stbi_uc* data = stbi_load(paths[i].c_str(), &temp_width, &temp_height, &temp_number_of_channels, 0);
		if(data)
		{
			this->width = temp_width;
			this->height = temp_height;
			this->channels = temp_number_of_channels;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, temp_width, temp_height, 0, format, data_format, data);
		}
		else
		{
			std::cout << "ERROR: FAILED TO LOAD TEXTURE" << std::endl;
		}

		stbi_image_free(data);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
}


unsigned texture::get_id() const
{
	return id;
}

unsigned texture::get_wrap_mode() const
{
	return wrap_mode;
}

bool texture::get_is_multi_sampled() const
{
	return is_multi_sampled;
}

unsigned texture::get_channels() const
{
	return channels;
}

int texture::get_filter_mag() const
{
	return filter_mag;
}

int texture::get_filter_min() const
{
	return filter_min;
}

texture_type texture::get_type() const
{
	return type;
}

unsigned texture::get_width() const
{
	return width;
}

unsigned texture::get_height() const
{
	return height;
}




void texture::bind() const
{
	if (type == texture_type::cube)
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
	else
		glBindTexture(is_multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, this->id);
}

void texture::activate(GLenum texture_location)
{
	glActiveTexture(texture_location);
}

std::string texture::type_to_string(const texture_type type)
{
	switch (type)
	{
		case texture_type::diffuse: return "diffuseTexture";
		case texture_type::normal:return "normalTexture" ;
		case texture_type::specular: return "specularTexture";
		case texture_type::reflection: return "reflectionTexture";
		case texture_type::color: return "color";
		case texture_type::depth: return "depth";
		case texture_type::stencil: return "stencil";
		case texture_type::depth_stencil: return "depthStencil";
		case texture_type::cube: return "cube";
		case texture_type::height: return "height";
	default: return "error";
	}
}

void texture::set_wrap_mode(const GLint wrap_mode)
{
	this->wrap_mode = wrap_mode;

	if(type == texture_type::cube)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, this->wrap_mode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, this->wrap_mode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, this->wrap_mode);
	}
	else
	{
		glTexParameteri(is_multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrap_mode);
		glTexParameteri(is_multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, this->wrap_mode);
		glTexParameteri(is_multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrap_mode);
	}
}

void texture::set_filter_mag(const GLint filter_mag)
{
	this->filter_mag = filter_mag;
	if(type == texture_type::cube)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter_mag);
	}
	else
		glTexParameteri(is_multi_sampled ?  GL_TEXTURE_2D_MULTISAMPLE :  GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);
}

void texture::set_filter_min(const GLint filter_min)
{
	this->filter_min = filter_min;
	if (type == texture_type::cube)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter_min);
	}
	else
	glTexParameteri(is_multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
}


