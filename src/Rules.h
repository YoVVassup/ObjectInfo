#pragma once
#include <YRPP.h>
#include <Helpers/Macro.h>
#include <vector>

class Ares {
public:
	static const size_t readLength = 2048;
	static char readBuffer[readLength];
};
char Ares::readBuffer[];

class ObjectInfoDisplay {
public:
	static std::vector<std::vector<std::string>> DisplayLists;
	static int DisplayListIndex;
	static int DisplayOffsetX;
	static int DisplayOffsetY;
	static void ChangeNextList()
	{
		DisplayListIndex++;
		if (DisplayListIndex >= DisplayLists.size())
			DisplayListIndex = 0;
	}
	static std::vector<std::string> GetList()
	{
		if (!ObjectInfoDisplay::DisplayLists.empty())
			return DisplayLists[DisplayListIndex];

		std::vector<std::string> empty;
		std::string str1 = "NONEALL";
		empty.push_back(str1);
		return empty;
	}
	static bool CanDisplay(char name[], std::string required = "")
	{
		if (ObjectInfoDisplay::DisplayLists.size() == 0)
			return true;

		std::string str2 = name;

		if (required != "")
			if (str2 != required)
				return false;

		auto list = GetList();
		for (auto item : list)
		{
			if (item == str2)
				return true;
		}
		return false;
	}
};

std::vector<std::vector<std::string>> ObjectInfoDisplay::DisplayLists;
int ObjectInfoDisplay::DisplayListIndex;
int ObjectInfoDisplay::DisplayOffsetX;
int ObjectInfoDisplay::DisplayOffsetY;


CCINIClass* OpenConfig(const char* file) {
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

void CloseConfig(CCINIClass*& pINI) {
	if (pINI) {
		GameDelete(pINI);
		pINI = nullptr;
	}
}


DEFINE_HOOK(0x5FACDF, _Options_LoadFromINI, 5)
{
	CCINIClass* pINI = OpenConfig("objectinfo.ini");

	constexpr auto readDelims = ",";
	
	ObjectInfoDisplay::DisplayLists.clear();
	ObjectInfoDisplay::DisplayListIndex = 0;
	
	const char* section = "ObjectInfoDisplayLists";
	int itemsCount = pINI->GetKeyCount(section);
	for (int i = 0; i < itemsCount; ++i)
	{
		std::vector<std::string> objectsList;
		char* context = nullptr;
		pINI->ReadString(section, pINI->GetKeyName(section, i), "", Ares::readBuffer);
	
		for (char* cur = strtok_s(Ares::readBuffer, readDelims, &context); cur; cur = strtok_s(nullptr, readDelims, &context))
		{
			objectsList.emplace_back(cur);
		}
		ObjectInfoDisplay::DisplayLists.push_back(objectsList);
	}

	ObjectInfoDisplay::DisplayOffsetX = pINI->ReadInteger("ObjectInfoDisplayOffset", "X", 0);
	ObjectInfoDisplay::DisplayOffsetY = pINI->ReadInteger("ObjectInfoDisplayOffset", "Y", 0);

	CloseConfig(pINI);
	return 0;
}


