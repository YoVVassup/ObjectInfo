#pragma once
#include <StringTable.h>
#include <CCINIClass.h>

class GeneralUtils
{
public:
	static bool IsValidString(const char* str);
	static const wchar_t* LoadStringOrDefault(const char* key, const wchar_t* defaultValue);
	static const wchar_t* LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue);
};
