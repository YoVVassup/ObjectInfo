#pragma once

// ============================================================================
// Rules.h - Display configuration and INI config helpers
// ============================================================================

#include <YRPP.h>
#include <Helpers/Macro.h>
#include <vector>

// ============================================================================
// Ares Compatibility Buffer
// ============================================================================

/// Static buffer used for reading INI string values, matching Ares convention.
class Ares {
public:
	static const size_t readLength = 2048;
	inline static char readBuffer[readLength];
};

// ============================================================================
// ObjectInfoDisplay - Preset-based field display system
// ============================================================================

/// Manages which fields are shown in the object info overlay.
/// Presets are loaded from [ObjectInfoDisplayLists] in objectinfo.ini.
/// Each preset is a comma-separated list of field names.
/// "NONEALL" or empty list means show all fields.
class ObjectInfoDisplay {
public:
	inline static std::vector<std::vector<std::string>> DisplayLists;
	inline static int DisplayListIndex = 0;
	inline static int DisplayOffsetX = 0;
	inline static int DisplayOffsetY = 0;

	/// Cycles to the next display preset.
	static void ChangeNextList()
	{
		DisplayListIndex++;
		if ((size_t)DisplayListIndex >= DisplayLists.size())
			DisplayListIndex = 0;
	}

	/// Returns the currently active preset list.
	static const std::vector<std::string>& GetList()
	{
		if (!ObjectInfoDisplay::DisplayLists.empty())
			return DisplayLists[DisplayListIndex];

		static const std::vector<std::string> empty = { "NONEALL" };
		return empty;
	}

	/// Checks whether a field should be displayed in the current preset.
	/// If required is non-empty, name must match it exactly.
	static bool CanDisplay(const char* name, const std::string& required = "")
	{
		if (ObjectInfoDisplay::DisplayLists.empty())
			return true;

		std::string str2 = name;

		if (!required.empty())
			if (str2 != required)
				return false;

		const auto& list = GetList();
		for (const auto& item : list)
		{
			if (item == str2)
				return true;
		}
		return false;
	}
};

// ============================================================================
// INI Config Helpers
// ============================================================================

/// Opens and reads a CCINIClass from a file. Returns nullptr on failure.
/// Caller is responsible for calling CloseConfig() on the returned pointer.
inline CCINIClass* OpenConfig(const char* file) {
	CCINIClass* pINI = GameCreate<CCINIClass>();

	if (pINI) {
		CCFileClass* cfg = GameCreate<CCFileClass>(file);

		if (cfg) {
			if (cfg->Exists()) {
				pINI->ReadCCFile(cfg);
			}
			GameDelete(cfg);
		}
	}

	return pINI;
}

/// Safely deletes a CCINIClass created by OpenConfig and nulls the pointer.
inline void CloseConfig(CCINIClass*& pINI) {
	if (pINI) {
		GameDelete(pINI);
		pINI = nullptr;
	}
}
