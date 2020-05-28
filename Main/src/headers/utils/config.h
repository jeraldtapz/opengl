// jerald   Jerald Tapalla
// OpenGL  OpenGL  Config.h
// 29 04 2019

#pragma once
#include <string>
#include "rendering/color.h"

class config
{
public:
	static const int WIDTH{ 1280 };
	static const int HEIGHT{ 720 };
	static const std::string WINDOW_NAME;
	static const color CLEAR_COLOR;
	static const std::string VERTEX_SHADER_DEFAULT;
	static const std::string FRAGMENT_SHADER_DEFAULT;
	constexpr static const float MOVE_SPEED{ 5.0f };
	constexpr static const float MOUSE_SENSITIVITY{ 0.05f };
};
