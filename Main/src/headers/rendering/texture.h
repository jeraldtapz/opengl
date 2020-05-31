#pragma once
#include <string>
#include <glad/glad.h>
#include "texture_type.h"
#include <iostream>
#include <vector>

#include "stb_image.h"
#include "utils/config.h"


class texture
{
private:
	unsigned int id{0};
	int wrap_mode{0};
	int filter_mag{0};
	int filter_min{0};
	unsigned int width{0};
	unsigned int height{0};
	unsigned int channels{0};
	texture_type type {texture_type::diffuse};
	bool is_multi_sampled{false};

public:
	unsigned int get_id() const;
	unsigned int get_wrap_mode() const;
	int get_filter_mag() const;
	int get_filter_min() const;
	unsigned int get_width() const;
	unsigned int get_height() const;
	unsigned int get_channels() const;
	texture_type get_type() const;
	bool get_is_multi_sampled() const;

	void set_wrap_mode(GLint wrap_mode);
	void set_filter_mag(GLint filter_mag);
	void set_filter_min(GLint filter_min);

	void bind() const;
	static void activate(GLenum texture_location);
	static std::string type_to_string(const texture_type type);

	texture();
	explicit  texture(const std::string& absolute_path, texture_type type, GLenum data_format, bool generate_mipmaps);
	explicit  texture(const std::string& absolute_path, texture_type type, GLenum format, GLenum internal_format, GLenum data_format, bool generate_mipmaps);
	/*explicit  texture(const std::string& absolute_path, texture_type type, GLenum data_format, bool generate_mipmaps, unsigned int samples);*/

	explicit  texture(const texture_type type, const unsigned int width, const unsigned int height,
		const GLenum format, const GLenum internal_format, const GLenum data_format, const bool generate_mipmaps);

	explicit  texture(const texture_type type, const unsigned int width, const unsigned int height,
		const GLenum format, const GLenum internal_format, const GLenum data_format, const bool generate_mipmaps, const unsigned int samples);

	explicit  texture(const std::vector<std::string>& paths, texture_type type, const GLenum internal_format, const GLenum format, GLenum data_format);
};
