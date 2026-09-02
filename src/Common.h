#pragma once

// ============================================================================
// Common.h - Shared types, globals, utilities, and templates for ObjectInfo
// ============================================================================

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
#include <cstdarg>
#include <cstdio>
#include <cstring>

// ============================================================================
// Drawing Utilities
// ============================================================================

/// Draws text with a black outline for readability against any background.
/// Renders 18 offset copies in black, then the original text in the specified color.
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

// ============================================================================
// Build Constants
// ============================================================================

#define STACK_OFFS(cur_offset, wanted_offset) STACK_OFFSET(cur_offset, -(wanted_offset))

#define RECT_COUNT 256              // Max trigger entries per page in debug overlay
#define TASKFORCE_MAX_ENTRIES 6     // Max unit types in a TaskForce definition
#define SCREEN_EDGE_MARGIN 80       // Pixels off-screen before hiding overlay
#define SCREEN_BOTTOM_THRESHOLD 150 // Min distance from bottom to flip overlay upward
#define TEXT_LINE_HEIGHT 14         // Pixel height of one text line
#define OUTLINE_RADIUS 2            // Outline thickness for DrawTextOutline
#define APPEND_BUFFER_SIZE 0x400    // 1024 bytes for append format buffer

// ============================================================================
// Color Constants (RGB565 format for game surface)
// ============================================================================

#define COLOR_TRIGGER_ENABLED    RGB8882RGB565(0, 180, 0)    // Active trigger
#define COLOR_TRIGGER_DISABLED   RGB8882RGB565(140, 140, 140) // Disabled trigger
#define COLOR_TRIGGER_DESTROYED  RGB8882RGB565(200, 60, 60)  // Expired/destroyed trigger
#define COLOR_TRIGGER_LOW_WEIGHT RGB8882RGB565(200, 200, 60)  // Weight below threshold
#define COLOR_HEADER_DIM         RGB8882RGB565(100, 100, 100) // Dimmed column headers
#define COLOR_PAGE_INFO          RGB8882RGB565(150, 150, 150) // Page indicator text

// ============================================================================
// Game Logic Constants
// ============================================================================

// RA2 runs at 15 frames per second; converts frame count to seconds
#define FRAMES_TO_SECONDS(frames) ((frames) / 15)

// Factory GetProgress() returns values up to this maximum
#define FACTORY_PROGRESS_MAX 54

// AI trigger weight that forces immediate execution
#define AI_TRIGGER_FIRE_IMMEDIATELY_WEIGHT 5000.0

// AI trigger is flagged as low weight when ratio to max is below this
#define AI_TRIGGER_LOW_WEIGHT_RATIO 0.2

// ============================================================================
// Enums
// ============================================================================

/// Action modes for the Trigger Debug panel.
enum CurrentMode : int
{
	ForceRun = 0,   // Execute trigger actions immediately
	Enable,         // Re-enable a disabled trigger
	Disable,        // Disable a trigger
	Destroy,        // Permanently destroy a trigger
	ChangeTimer,    // Set a new timer value
	ModeCount
};

/// Sort modes for the Trigger Debug panel.
enum TriggerSort : int
{
	Raw = 0,        // Default game order
	ByID,           // Ascending by Type ID
	ByName,         // Ascending alphabetical
	ByTimeLeft,     // Ascending by remaining time
	ByLastExecuted, // Descending by last execution frame
	ByDestroyed,    // Ascending by destruction frame
	end
};

// ============================================================================
// Extended Trigger Data
// ============================================================================

/// Runtime extension data tracked per TriggerClass instance.
/// Stores execution history, event states, and destruction info that the
/// engine does not expose natively.
class TriggerClassExt
{
public:
	int LastExecutedFrame = -1;    // Frame number of last execution (-1 = never)
	int ExecutedCount = 0;         // Total number of times executed
	std::vector<bool> OccuredEvents; // Per-condition satisfied state
	bool Destroyed = false;        // Whether this trigger has been destroyed
	int DestroyedFrame = -1;       // Frame when destroyed (-1 = not destroyed)
	TriggerTypeClass* Type = nullptr; // Back-pointer to type (for destroyed triggers)
	int ResetTimer = -1;           // Original timer value before manual edit (-1 = unmodified)
};

// ============================================================================
// Global State Declarations
// ============================================================================

// --- Overlay toggle flags ---
extern bool bObjectInfo;              // Object info overlay enabled
extern bool bTriggerDebug;            // Trigger debug panel enabled
extern bool bPressedInButtonsLayer;   // Mouse click was consumed by a UI button
extern bool bTriggerDebugPageEnd;     // Reached last page of trigger list
extern bool bTriggerDebugDetailed;    // Show detailed trigger info
extern bool bTriggerDebugEdited;      // Search input was just submitted
extern bool bTriggerDebugTimerEdited; // Timer input was just submitted

// --- Search state ---
extern std::wstring SearchPattern;    // Current trigger search/filter string

// --- Overlay positions (configurable via INI) ---
extern int TriggerDebugStartX;
extern int TriggerDebugStartY;
extern int AITriggerDebugStartX;
extern int AITriggerDebugStartY;

// --- Trigger debug panel state ---
extern int HoveredTriggerIndex;       // Index of trigger under mouse (-100 = none)
extern int PageTriggerCount;          // Actual number of triggers shown per page
extern int CurrentPage;               // Current page index
extern CurrentMode Mode;              // Active action mode
extern TriggerSort Sort;              // Active sort mode
extern int ModeIndex;                 // Index of mode button under mouse
extern int ChangedTimer;              // Timer value entered by user

// --- Click detection rectangles (one per trigger row) ---
extern RectangleStruct TriggerDebugRect[RECT_COUNT];
extern RectangleStruct TriggerDebugMode[ModeCount];
extern RectangleStruct TriggerDebugPageDown;
extern RectangleStruct TriggerDebugPageUp;
extern RectangleStruct TriggerDebugDetailed;
extern RectangleStruct TriggerDebugSort;
extern RectangleStruct TriggerDebugSearch;
extern RectangleStruct TriggerDebugEnableModified;

#define AI_TRIGGER_RECT_COUNT 256

// --- AI Trigger debug panel state ---
extern bool bAITriggerDebug;
extern int AITriggerDebugHoveredIndex;
extern int AITriggerDebugPage;
extern int AITriggerDebugSelectedHouse;
extern int AITriggerDebugPageItemCount;
extern RectangleStruct AITriggerDebugRect[AI_TRIGGER_RECT_COUNT];
extern RectangleStruct AITriggerDebugPageUp;
extern RectangleStruct AITriggerDebugPageDown;
extern RectangleStruct AITriggerDebugHouseLeft;

// --- Trigger sorting and extension storage ---
extern std::vector<TriggerClass*> SortedTriggerArray;
extern std::vector<TriggerClassExt> DestroyedTriggers;
extern std::vector<TriggerClassExt> SortedDestroyedTriggers;
extern std::map<TriggerClass*, TriggerClassExt> TriggerExtMap;

// ============================================================================
// ComparableTrigger - Unified wrapper for sorting active and destroyed triggers
// ============================================================================

/// Variant type that can hold either a live TriggerClass pointer or a
/// destroyed TriggerClassExt, allowing both to be sorted together.
using TriggerVariant = std::variant<TriggerClass**, TriggerClassExt*>;

/// Wrapper that provides uniform accessors over TriggerVariant.
/// Used by the sort algorithms to compare triggers regardless of their state.
struct ComparableTrigger {
	TriggerVariant item;

	/// Returns the trigger's display name from its Type.
	auto getName() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				return (*obj)->Type->Name;
			}
			else
				return obj->Type->Name;
			}, item);
	}

	/// Returns the trigger's string ID from its Type.
	auto getID() const {
		return std::visit([](auto* obj) {
			if constexpr (std::is_same_v<std::decay_t<decltype(*obj)>, TriggerClass*>) {
				return (*obj)->Type->ID;
			}
			else
				return obj->Type->ID;
			}, item);
	}

	/// Returns remaining time in frames. Returns INT_MAX for destroyed triggers.
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

	/// Returns the frame number when the trigger was last executed, or -1.
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

	/// Returns the frame when the trigger was destroyed, or -1 if still alive.
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
extern TriggerSort LastSortedType;
extern bool bTriggerCacheDirty;

// ============================================================================
// Utility Functions
// ============================================================================

/// Wrapper for Drawing::GetTextDimensions with default parameters.
inline RectangleStruct GetTextDimensionsCompat(const wchar_t* text)
{
	return Drawing::GetTextDimensions(text, { 0, 0 }, 0);
}

/// Draws a text button with outline and returns the right edge X coordinate.
/// Stores the clickable rectangle in outRect for mouse hit-testing.
inline int DrawTextButton(const wchar_t* text, int x, int y, int color, RectangleStruct& outRect)
{
	auto wanted = GetTextDimensionsCompat(text);
	wanted.Height = TEXT_LINE_HEIGHT;

	RectangleStruct rect = { x, y, wanted.Width, wanted.Height };
	outRect = rect;

	DrawTextOutline(text, rect.X, rect.Y, color);
	return rect.X + wanted.Width;
}

/// Converts 8-bit RGB components to 16-bit RGB565 format used by the game surface.
inline int RGB8882RGB565(int r, int g, int b)
{
	return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
}

/// Converts a narrow string (ANSI codepage) to a wide string.
inline std::wstring A2W(const char* str)
{
	if (!str || !*str) return {};
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
	if (len <= 0) return {};
	std::wstring result(len - 1, L'\0');
	MultiByteToWideChar(CP_ACP, 0, str, -1, &result[0], len);
	return result;
}

/// Writes a formatted message to the game's log file (gamemd.log).
inline void LogGame(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

/// Formats and writes a message to the game log using a local buffer.
inline void Log(const char* pFormat, ...)
{
	char buf[0x1000] = { 0 };
	va_list args;
	va_start(args, pFormat);
	vsnprintf(buf, sizeof(buf), pFormat, args);
	va_end(args);
	LogGame(buf);
}

/// Returns a formatted string. Uses a local buffer to avoid global state.
inline std::string Format(const char* pFormat, ...)
{
	char buf[0x1000] = { 0 };
	va_list args;
	va_start(args, pFormat);
	vsnprintf(buf, sizeof(buf), pFormat, args);
	va_end(args);
	return std::string(buf);
}

/// Displays a formatted message in the in-game message list.
inline void Message(const wchar_t* pFormat, ...)
{
	wchar_t buf[0x1000] = { 0 };
	va_list args;
	va_start(args, pFormat);
	vswprintf_s(buf, pFormat, args);
	va_end(args);
	MessageListClass::Instance.PrintMessage(buf);
}

// ============================================================================
// Forward Declarations
// ============================================================================

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

// ============================================================================
// Command Classes - Registered as hotkey commands in the game engine
// ============================================================================

/// Dumps all trigger data to debug.log for external analysis.
class TriggerInfoClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "Dump Trigger Info"; }
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

/// Toggles the interactive trigger debug panel.
class TriggerDebugClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "Trigger Debug Mode"; }
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

/// Scrolls the trigger debug list up one page.
class TriggerDebugPageUpClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "Trigger Debug Page Up"; }
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

/// Scrolls the trigger debug list down one page.
class TriggerDebugPageDownClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "Trigger Debug Page Down"; }
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

/// Toggles the AI Trigger Types decision-making panel.
class AITriggerDebugClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "AI Trigger Debug Mode"; }
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

/// Scrolls the AI trigger debug list up one page.
class AITriggerDebugPageUpClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "AI Trigger Debug Page Up"; }
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

/// Scrolls the AI trigger debug list down one page.
class AITriggerDebugPageDownClass : public CommandClass
{
public:
	virtual const char* GetName() const override { return "AI Trigger Debug Page Down"; }
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

// ============================================================================
// Display Templates
// ============================================================================

/// Prints common TechnoClass fields (ID, HP, owner, location, etc.)
/// that apply to all object types. Uses CRTP-style lambdas for append/display.
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

/// Iterates the active display preset list and calls PrintCommonTechnoInfo
/// followed by a type-specific field functor (footFields or buildingFields).
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
