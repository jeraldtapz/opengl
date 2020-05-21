#include "engine/game_object.h"

game_object::game_object()
{
	this->_transform = std::make_shared<transform>(transform());
	name = "unnamed game_object";
	is_active = true;
}

// ReSharper disable once CppParameterNamesMismatch
game_object::game_object(const transform& t_form)
{
	_transform = std::make_shared<transform>(t_form);
	name = "unnamed game_object";
	is_active = true;
}


transform* game_object::get_transform() const
{
	return this->_transform.get();
}

std::string game_object::get_name() const
{
	return name;
}

void game_object::set_name(const std::string& name)
{
	this->name = name;
}

