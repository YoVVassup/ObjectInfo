#pragma once
#include <YRPP.h>
#include <Helpers/Macro.h>
#include <vector>

class Ares {
public:
	static const size_t readLength = 2048;
	inline static char readBuffer[readLength];
};

class ObjectInfoDisplay {
public:
	inline static std::vector<std::vector<std::string>> DisplayLists;
	inline static int DisplayListIndex = 0;
	inline static int DisplayOffsetX = 0;
	inline static int DisplayOffsetY = 0;
	static void ChangeNextList()
	{
		DisplayListIndex++;
		if ((size_t)DisplayListIndex >= DisplayLists.size())
			DisplayListIndex = 0;
	}
	static const std::vector<std::string>& GetList()
	{
		if (!ObjectInfoDisplay::DisplayLists.empty())
			return DisplayLists[DisplayListIndex];

		static const std::vector<std::string> empty = { "NONEALL" };
		return empty;
	}
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

inline void CloseConfig(CCINIClass*& pINI) {
	if (pINI) {
		GameDelete(pINI);
		pINI = nullptr;
	}
}
