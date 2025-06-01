#pragma once
#include <typeindex>
#include <iostream>
#include <string>
#include <fstream>

class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	virtual const std::type_index GetType() = 0;
};