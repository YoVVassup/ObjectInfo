#pragma once

// ============================================================================
// GeneralUtils.h - String loading and validation utilities
// ============================================================================

#include <StringTable.h>
#include <CCINIClass.h>

/// Utility class for string validation and localized string loading.
class GeneralUtils
{
public:
	/// Returns true if the string is non-null, non-empty, and not blank.
	static bool IsValidString(const char* str);

	/// Loads a localized string by key from the game's string table.
	/// Returns defaultValue if the key is invalid or missing.
	static const wchar_t* LoadStringOrDefault(const char* key, const wchar_t* defaultValue);

	/// Loads a localized string, returning defaultValue if the result
	/// starts with "MISSING:" (indicating the key was not found).
	static const wchar_t* LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue);
};
