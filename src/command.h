#pragma once

// ============================================================================
// command.h - Template helper for registering custom commands with the engine
// ============================================================================

#include <YRpp.h>
#include <CommandClass.h>

/// Creates a new command instance and adds it to the engine's command array.
/// Used during DLL initialization to register all custom hotkey commands.
template <typename T>
void MakeCommand() {
	T* command = GameCreate<T>();
	CommandClass::Array.AddItem(command);
};
