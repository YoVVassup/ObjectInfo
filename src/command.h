#pragma once

#include <YRPP.h>
#include <CommandClass.h>

// will the templates ever stop? :D
template <typename T>
void MakeCommand() {
	T* command = GameCreate<T>();
	CommandClass::Array.AddItem(command);
};
