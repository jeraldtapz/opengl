#pragma once
#include <memory>
#include <string>

#include "data/transform.h"

class game_object
{
protected:
	std::string name;
	std::shared_ptr<transform> _transform;

public:

	bool is_active;
	
	explicit game_object(const transform &t_form);
	explicit game_object();

	transform* get_transform() const;
	std::string get_name() const;
	void set_name(const std::string& name);
	
};
