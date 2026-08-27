#pragma once

#include <YRPP.h>
#include <Helpers/Macro.h>
#include <TagTypeClass.h>
#include <TriggerTypeClass.h>
#include <AITriggerTypeClass.h>
#include <HouseClass.h>
#include "GeneralUtils.h"
#include "Rules.h"
#include "command.h"
#include <map>
#include <cmath>
#include <string>
#include <cwchar>
#include <cwctype>
#include <algorithm>
#include <variant>
#include <vector>

inline void DrawTextOutline(const wchar_t* text, int x, int y, int color)
{
	static constexpr int offsets[][2] = {
		{-2,0},{2,0},{0,-2},{0,2},{-2,-2},{2,-2},{-2,2},{2,2},
		{-1,-2},{0,-2},{1,-2},{-2,-1},{2,-1},{-2,1},{2,1},{-1,2},{0,2},{1,2}
	};
	for (auto& off : offsets)
		DSurface::Composite->DrawText(text, x + off[0], y + off[1], COLOR_BLACK);
	DSurface::Composite->DrawText(text, x, y, color);
}

#define STACK_OFFS(cur_offset, wanted_offset) STACK_OFFSET(cur_offset, -(wanted_offset))

#define RECT_COUNT 256
#define TASKFORCE_MAX_ENTRIES 6
#define SCREEN_EDGE_MARGIN 80
#define SCREEN_BOTTOM_THRESHOLD 150

enum CurrentMode : int
{
	ForceRun = 0,
	Enable,
	Disable,
	Destroy,
	ChangeTimer,
	ModeCount
};

enum TriggerSort : int
{
	Raw = 0,
	ByID,
	ByName,
	ByTimeLeft,
	ByLastExecuted,
	ByDestroyed,
	end
};

class TriggerClassExt
{
public:
	int LastExecutedFrame = -1;
	int ExecutedCount = 0;
	std::vector<bool> OccuredEvents;
	bool Destroyed = false;
	int DestroyedFrame = -1;
	TriggerTypeClass* Type = nullptr;
	int ResetTimer = -1;
};

extern bool bObjectInfo;
extern bool bTriggerDebug;
extern bool bPressedInButtonsLayer;
extern bool bTriggerDebugPageEnd;
extern bool bTriggerDebugDetailed;
extern bool bTriggerDebugEdited;
extern bool bTriggerDebugTimerEdited;

extern char FinalStringBuffer[0x1000];
extern wchar_t FinalStringBufferW[0x1000];
extern wchar_t SearchPattern[0x200];

extern int TriggerDebugStartX;
extern int TriggerDebugStartY;
extern int AITriggerDebugStartX;
extern int AITriggerDebugStartY;
extern int HoveredTriggerIndex;
extern int PageTriggerCount;
extern int CurrentPage;
extern CurrentMode Mode;
extern TriggerSort Sort;
extern int ModeIndex;
extern int ChangedTimer;
extern RectangleStruct TriggerDebugRect[RECT_COUNT];
extern RectangleStruct TriggerDebugMode[ModeCount];
extern RectangleStruct TriggerDebugPageDown;
extern RectangleStruct TriggerDebugPageUp;
extern RectangleStruct TriggerDebugDetailed;
extern RectangleStruct TriggerDebugSort;
extern RectangleStruct TriggerDebugSearch;
extern RectangleStruct TriggerDebugEnableModified;

#define AI_TRIGGER_RECT_COUNT 256

extern bool bAITriggerDebug;
extern int AITriggerDebugHoveredIndex;
extern int AITriggerDebugPage;
extern int AITriggerDebugSelectedHouse;
extern int AITriggerDebugPageItemCount;
extern RectangleStruct AITriggerDebugRect[AI_TRIGGER_RECT_COUNT];
extern RectangleStruct AITriggerDebugPageUp;
extern RectangleStruct AITriggerDebugPageDown;
extern RectangleStruct AITriggerDebugHouseLeft;

extern std::vector<TriggerClass*> SortedTriggerArray;
extern std::vector<TriggerClassExt> DestroyedTriggers;
extern std::vector<TriggerClassExt> SortedDestroyedTriggers;
extern std::map<TriggerClass*, TriggerClassExt> TriggerExtMap;

using TriggerVariant = std::variant<TriggerClass**, TriggerClassExt*>;
struct ComparableTrigger {
	TriggerVariant item;

	auto getName() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				return (*obj)->Type->Name;
			}
			else
				return obj->Type->Name;
			}, item);
	}
	auto getID() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				return (*obj)->Type->ID;
			}
			else
				return obj->Type->ID;
			}, item);
	}
	auto getTimeLeft() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				int timeLeft = (*obj)->Enabled ? (*obj)->Timer.GetTimeLeft() : (*obj)->Timer.TimeLeft;
				if (timeLeft <= 0)
					timeLeft = INT_MAX;
				return timeLeft;
			}
			else
				return INT_MAX;
			}, item);
	}
	auto getLastExecuted() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				auto it = TriggerExtMap.find(*obj);
				return it != TriggerExtMap.end() ? it->second.LastExecutedFrame : -1;
			}
			else
				return obj->LastExecutedFrame;
			}, item);
	}
	auto getDestroyed() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				return -1;
			}
			else
				return obj->DestroyedFrame;
			}, item);
	}
};

extern std::vector<ComparableTrigger> SortedAllTriggers;

inline RectangleStruct GetTextDimensionsCompat(const wchar_t* text)
{
	return Drawing::GetTextDimensions(text, { 0, 0 }, 0);
}

inline int RGB8882RGB565(int r, int g, int b)
{
	return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
}

inline std::wstring A2W(const char* str)
{
	if (!str || !*str) return {};
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
	if (len <= 0) return {};
	std::wstring result(len - 1, L'\0');
	MultiByteToWideChar(CP_ACP, 0, str, -1, &result[0], len);
	return result;
}

inline void LogGame(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

inline void Log(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(FinalStringBuffer, pFormat, args);
	LogGame(FinalStringBuffer);
	va_end(args);
}

inline const char* Format(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(FinalStringBuffer, pFormat, args);
	va_end(args);
	return FinalStringBuffer;
}

inline void Message(const wchar_t* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vswprintf_s(FinalStringBufferW, pFormat, args);
	MessageListClass::Instance.PrintMessage(FinalStringBufferW);
	va_end(args);
}

std::wstring to_lower(const wchar_t* s);
void GetEventList(TEventClass* pEvent, std::vector<int>& List);
void GetEventCount(TEventClass* pEvent, int& count);
void SortTriggerArray(TriggerSort sortType);
const char* GetMissionName(int mID);
void DrawTriggerDebug();
void DrawAITriggerDebug();
void HandleAITriggerDebugClick();
void HandleAITriggerDebugNumpad();

class TriggerInfoClass;

void ProcessTriggers(TriggerClass* pTrigger);

class TriggerDebugClass;
class TriggerDebugPageUpClass;
class TriggerDebugPageDownClass;

class TriggerInfoClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Dump Trigger Info";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_TRIGGER_INFO", L"Dump Trigger Info");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_TRIGGER_INFO_DESC", L"Dump Trigger Info to debug.log.");
	}
	virtual void Execute(WWKey eInput) const override;
};

class TriggerDebugClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Trigger Debug Mode";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_TRIGGER_DEBUG_MODE", L"Trigger Debug Mode");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_TRIGGER_DEBUG_MODE_DESC", L"Enable Trigger Debug Mode.");
	}
	virtual void Execute(WWKey eInput) const override;
};

class TriggerDebugPageUpClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Trigger Debug Page Up";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_TRIGGER_DEBUG_PAGEUP", L"Trigger Debug Page Up");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_TRIGGER_DEBUG_PAGEUP_DESC", L"Trigger Debug Page Up.");
	}
	virtual void Execute(WWKey eInput) const override;
};

class TriggerDebugPageDownClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Trigger Debug Page Down";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_TRIGGER_DEBUG_PAGEDOWN", L"Trigger Debug Page Down");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_TRIGGER_DEBUG_PAGEDOWN_DESC", L"Trigger Debug Page Down.");
	}
	virtual void Execute(WWKey eInput) const override;
};

class AITriggerDebugClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "AI Trigger Debug Mode";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_AI_TRIGGER_DEBUG_MODE", L"AI Trigger Debug Mode");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_AI_TRIGGER_DEBUG_MODE_DESC", L"Display AI Trigger Types decision-making status for each house.");
	}
	virtual void Execute(WWKey eInput) const override;
};

class AITriggerDebugPageUpClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "AI Trigger Debug Page Up";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_AI_TRIGGER_DEBUG_PAGEUP", L"AI Trigger Debug Page Up");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_AI_TRIGGER_DEBUG_PAGEUP_DESC", L"AI Trigger Debug Page Up.");
	}
	virtual void Execute(WWKey eInput) const override;
};

class AITriggerDebugPageDownClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "AI Trigger Debug Page Down";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_AI_TRIGGER_DEBUG_PAGEDOWN", L"AI Trigger Debug Page Down");
	}
	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}
	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_AI_TRIGGER_DEBUG_PAGEDOWN_DESC", L"AI Trigger Debug Page Down.");
	}
	virtual void Execute(WWKey eInput) const override;
};

template<typename AppendFn, typename DisplayFn, typename ToolTipFn>
void PrintCommonTechnoInfo(TechnoClass* pTechno, const std::string& name, bool allDisplay,
	AppendFn& append, DisplayFn& display, ToolTipFn& displayToolTip)
{
	auto pType = pTechno->GetTechnoType();

	if (ObjectInfoDisplay::CanDisplay("uiname", name) || allDisplay)
	{
		char missing[0x100] = { 0 };
		snprintf(missing, sizeof(missing), "MISSING:'%s'", pType->UINameLabel);
		auto wDefault = A2W(missing);
		displayToolTip(L"%s", GeneralUtils::LoadStringUnlessMissing(pType->UINameLabel, wDefault.c_str()));
	}
	if (ObjectInfoDisplay::CanDisplay("id", name) || allDisplay)
	{
		append("ID = %s", pType->ID);
		display();
	}
	if (ObjectInfoDisplay::CanDisplay("uid", name) || allDisplay)
	{
		append("UID = %d", (int)pTechno->UniqueID);
		display();
	}
	if (ObjectInfoDisplay::CanDisplay("hp", name) || allDisplay)
	{
		append("HP = (%d / %d)", pTechno->Health, pType->Strength);
		display();
	}
	if (ObjectInfoDisplay::CanDisplay("owner", name) || allDisplay)
	{
		if (pTechno->Owner)
		{
			append("Owner = %s (%s)", pTechno->Owner->get_ID(), pTechno->Owner->PlainName);
			display();
		}
	}
	if (ObjectInfoDisplay::CanDisplay("location", name) || allDisplay)
	{
		append("Location = (%d, %d)", (int)pTechno->GetMapCoords().X, (int)pTechno->GetMapCoords().Y);
		display();
	}
	if (ObjectInfoDisplay::CanDisplay("link", name) || allDisplay)
	{
		if (pTechno->HasAnyLink())
		{
			for (auto i = 0; i < pTechno->RadioLinks.Capacity; ++i)
			{
				if (auto const pLink = pTechno->GetNthLink(i))
				{
					auto pLinkType = pLink->GetType();
					append("Link %d: UID = %d, ID = %s, Location = (%d, %d)",
						i,
						(int)pLink->UniqueID,
						pLinkType->ID,
						(int)pLink->GetMapCoords().X, (int)pLink->GetMapCoords().Y
					);
					display();
				}
			}
		}
	}
	if (ObjectInfoDisplay::CanDisplay("target", name) || allDisplay)
	{
		if (pTechno->Target)
		{
			auto mapCoords = CellStruct::Empty;
			auto ID = "N/A";

			if (auto const pObject = abstract_cast<ObjectClass*>(pTechno->Target))
			{
				mapCoords = pObject->GetMapCoords();
				ID = pObject->GetType()->get_ID();
			}
			else if (auto const pCell = abstract_cast<CellClass*>(pTechno->Target))
			{
				mapCoords = pCell->MapCoords;
				ID = "Cell";
			}

			append("Target = %s, Distance = %d, Location = (%d, %d)", ID, (pTechno->DistanceFrom(pTechno->Target) / 256), mapCoords.X, mapCoords.Y);
			display();
		}
	}
	if (ObjectInfoDisplay::CanDisplay("ammo", name) || allDisplay)
	{
		if (pType->Ammo > 0)
		{
			append("Ammo = (%d / %d)", pTechno->Ammo, pType->Ammo);
			display();
		}
	}
	if (ObjectInfoDisplay::CanDisplay("currentmission", name) || allDisplay)
	{
		append("Current Mission = %d (%s)", pTechno->CurrentMission, GetMissionName((int)pTechno->CurrentMission));
		display();
	}
	if (ObjectInfoDisplay::CanDisplay("group", name) || allDisplay)
	{
		append("Group = %d", pTechno->Group);
		display();
	}
	if (ObjectInfoDisplay::CanDisplay("veterancy", name) || allDisplay)
	{
		if (pType->Trainable)
		{
			const char* veterancy = "N/A";
			if (pTechno->Veterancy.IsRookie())
				veterancy = "Rookie";
			else if (pTechno->Veterancy.IsVeteran())
				veterancy = "Veteran";
			else if (pTechno->Veterancy.IsElite())
				veterancy = "Elite";
			double truncatedVet = floor(pTechno->Veterancy.Veterancy * 100) / 100.0;
			append("Veterancy = %s (%.2f)", veterancy, truncatedVet);
			display();
		}
	}
	if (ObjectInfoDisplay::CanDisplay("tag", name) || allDisplay)
	{
		if (pTechno->AttachedTag)
		{
			append("Tag = %s, InstanceCount = %d", pTechno->AttachedTag->Type->get_ID(), pTechno->AttachedTag->InstanceCount);
			display();
		}
	}
}

template<typename AppendFn, typename DisplayFn, typename ToolTipFn, typename TypeFn>
void PrintObjectInfo(TechnoClass* pTechno, AppendFn& append, DisplayFn& display, ToolTipFn& displayToolTip, TypeFn& typeFields)
{
	for (const auto& name : ObjectInfoDisplay::GetList())
	{
		bool allDisplay = (name == "NONEALL");

		PrintCommonTechnoInfo(pTechno, name, allDisplay, append, display, displayToolTip);
		typeFields(pTechno, name, allDisplay);
	}
}
