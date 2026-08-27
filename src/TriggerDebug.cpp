#include "Common.h"
#include <WWMouseClass.h>

int TriggerDebugStartX = 10;
int TriggerDebugStartY = 180;
int HoveredTriggerIndex = -100;
int PageTriggerCount = RECT_COUNT;
int CurrentPage = 0;
CurrentMode Mode = ForceRun;
TriggerSort Sort = Raw;
int ModeIndex = -1;
int ChangedTimer = 0;
RectangleStruct TriggerDebugRect[RECT_COUNT]{0};
RectangleStruct TriggerDebugMode[ModeCount]{0};
RectangleStruct TriggerDebugPageDown{ 0 };
RectangleStruct TriggerDebugPageUp{ 0 };
RectangleStruct TriggerDebugDetailed{ 0 };
RectangleStruct TriggerDebugSort{ 0 };
RectangleStruct TriggerDebugSearch{ 0 };
RectangleStruct TriggerDebugEnableModified{ 0 };

std::vector<TriggerClass*> SortedTriggerArray;
std::vector<TriggerClassExt> DestroyedTriggers;
std::vector<TriggerClassExt> SortedDestroyedTriggers;

std::map<TriggerClass*, TriggerClassExt> TriggerExtMap;
std::vector<ComparableTrigger> SortedAllTriggers;

std::wstring to_lower(const wchar_t* s) {
	std::wstring result;
	while (*s) {
		result += std::towlower(*s++);
	}
	return result;
}

static std::vector<TriggerClass*> match_wchar_patterns(const std::vector<TriggerClass*>& items, const wchar_t* source) {
	std::vector<TriggerClass*> result;
	std::wstring lower_source = to_lower(source);
	lower_source.erase(0, lower_source.find_first_not_of(L" \t\n\r\f\v"));
	auto last = lower_source.find_last_not_of(L" \t\n\r\f\v");
	if (last != std::wstring::npos)
		lower_source.erase(last + 1);
	else
		lower_source.clear();

	if (lower_source.empty())
		return result;

	bool exclude = false;
	if (lower_source.front() == L'!' && lower_source.back() == L'!' && lower_source.length() > 2) {
		lower_source = lower_source.substr(1, lower_source.length() - 2);
		exclude = true;
	}

	for (auto item : items) {
		if (!item || !item->Type || !item->Type->ID || !item->Type->Name)
			continue;
		std::wstring pattern = to_lower(A2W(Format("%s (%s)", item->Type->get_ID(), item->Type->Name)).c_str());
		if (exclude) {
			if (pattern.find(lower_source) == std::wstring::npos)
				result.push_back(item);
		}
		else {
			if (pattern.find(lower_source) != std::wstring::npos)
				result.push_back(item);
		}
	}

	return result;
}

static std::vector<TriggerClassExt> match_wchar_patternsExt(const std::vector<TriggerClassExt>& items, const wchar_t* source) {
	std::vector<TriggerClassExt> result;
	std::wstring lower_source = to_lower(source);
	lower_source.erase(0, lower_source.find_first_not_of(L" \t\n\r\f\v"));
	auto last = lower_source.find_last_not_of(L" \t\n\r\f\v");
	if (last != std::wstring::npos)
		lower_source.erase(last + 1);
	else
		lower_source.clear();

	if (lower_source.empty())
		return result;

	bool exclude = false;
	if (lower_source.front() == L'!' && lower_source.back() == L'!' && lower_source.length() > 2) {
		lower_source = lower_source.substr(1, lower_source.length() - 2);
		exclude = true;
	}

	for (auto& item : items) {
		if (!item.Type || !item.Type->ID || !item.Type->Name)
			continue;
		std::wstring pattern = to_lower(A2W(Format("<Expired> %s (%s)", item.Type->get_ID(), item.Type->Name)).c_str());
		if (exclude) {
			if (pattern.find(lower_source) == std::wstring::npos)
				result.push_back(item);
		}
		else {
			if (pattern.find(lower_source) != std::wstring::npos)
				result.push_back(item);
		}
	}

	return result;
}

void GetEventList(TEventClass* pEvent, std::vector<int>& List)
{
	if (pEvent)
	{
		List.push_back((int)pEvent->EventKind);
		GetEventList(pEvent->NextEvent, List);
	}
}

void GetEventCount(TEventClass* pEvent, int& count)
{
	if (pEvent)
	{
		count++;
		GetEventCount(pEvent->NextEvent, count);
	}
}

void SortTriggerArray(TriggerSort sortType)
{
	SortedTriggerArray.clear();
	SortedDestroyedTriggers.clear();
	SortedAllTriggers.clear();
	for (int i = 0; i < TriggerClass::Array.Count; i++) {
		SortedTriggerArray.push_back(TriggerClass::Array.GetItem(i));
	}

	if (bTriggerDebugEdited && !MessageListClass::Instance.HasEditFocus() && wcscmp(SearchPattern, MessageListClass::Instance.GetEditBuffer()) != 0)
	{
		bTriggerDebugEdited = false;
		CurrentPage = 0;
		wcsncpy_s(SearchPattern, MessageListClass::Instance.GetEditBuffer(), _TRUNCATE);
	}
	if (wcslen(SearchPattern) > 0)
	{
		SortedTriggerArray = match_wchar_patterns(SortedTriggerArray, SearchPattern);
		SortedDestroyedTriggers = match_wchar_patternsExt(DestroyedTriggers, SearchPattern);
	}
	else
	{
		SortedDestroyedTriggers = DestroyedTriggers;
	}

	for (auto& item : SortedTriggerArray)
	{
		SortedAllTriggers.push_back(ComparableTrigger{ &item });
	}
	for (auto& item : SortedDestroyedTriggers)
	{
		SortedAllTriggers.push_back(ComparableTrigger{ &item });
	}

	switch (sortType)
	{
	case Raw:
		break;
	case ByID:
		std::sort(SortedAllTriggers.begin(), SortedAllTriggers.end(),
			[](const ComparableTrigger& lhs, const ComparableTrigger& rhs) {
			return lhs.getID() < rhs.getID();
			});
		break;
	case ByName:
		std::sort(SortedAllTriggers.begin(), SortedAllTriggers.end(),
			[](const ComparableTrigger& lhs, const ComparableTrigger& rhs) {
				return lhs.getName() < rhs.getName();
			});
		break;
	case ByTimeLeft:
		std::sort(SortedAllTriggers.begin(), SortedAllTriggers.end(),
			[](const ComparableTrigger& lhs, const ComparableTrigger& rhs) {
				return lhs.getTimeLeft() < rhs.getTimeLeft();
			});
		break;
	case ByLastExecuted:
		std::sort(SortedAllTriggers.begin(), SortedAllTriggers.end(),
			[](const ComparableTrigger& lhs, const ComparableTrigger& rhs) {
				return lhs.getLastExecuted() > rhs.getLastExecuted();
			});
		break;
	case ByDestroyed:
		std::sort(SortedAllTriggers.begin(), SortedAllTriggers.end(),
			[](const ComparableTrigger& lhs, const ComparableTrigger& rhs) {
				return lhs.getDestroyed() > rhs.getDestroyed();
			});
		break;
	default:
		break;
	}
}

void TriggerInfoClass::Execute(WWKey eInput) const
{
	auto DumpAllTrigger = [](TriggerClass* pTrigger) -> void {
		if (pTrigger) {
			Log("[Trigger Info]     Trigger: %s (%s), %s\n", pTrigger->Type->get_ID(), pTrigger->Type->Name, pTrigger->Enabled ? "Enabled" : "Disabled");
		}
		};
	auto DumpTrigger = [](auto self, TriggerClass* pTrigger, int counter) -> void {
		if (pTrigger) {
			Log("[Trigger Info]         Trigger: %s (%s), %s\n", pTrigger->Type->get_ID(), pTrigger->Type->Name, pTrigger->Enabled ? "Enabled" : "Disabled");
			self(self, pTrigger->NextTrigger, counter + 1);
		}
		};
	auto DumpTag = [DumpTrigger](TagClass* pTag) {
		if (pTag) {
			Log("[Trigger Info]     Tag: %s (%s)\n", pTag->Type->get_ID(), pTag->Type->Name);
			DumpTrigger(DumpTrigger, pTag->FirstTrigger, 0);
		}
		};
	auto DumpTags = [DumpTag](DynamicVectorClass<TagClass*>* tagArray) {
		for (int i = 0; i < tagArray->Count; i++) {
			DumpTag(tagArray->GetItem(i));
		}
		};

	Log("[Trigger Info] ================== Array_Logic ==================\n");
	DumpTags(&TagClass::Array);
	Log("[Trigger Info] ================== Array_House ==================\n");
	for (int i = 0; i < HouseClass::Array.Count; i++) {
		if (auto pHouse = HouseClass::Array.GetItem(i)) {
			if (pHouse->RelatedTags.Count > 0) {
				Log("[Trigger Info] %s (%s):\n", pHouse->get_ID(), pHouse->PlainName);
				DumpTags(&pHouse->RelatedTags);
			}
		}
	}
	Log("[Trigger Info] ================== Array_Object ==================\n");
	std::map<TagClass*, std::vector<TechnoClass*>> ObjectTags;
	for (auto pTechno : TechnoClass::Array) {
		if (pTechno->AttachedTag) {
			ObjectTags[pTechno->AttachedTag].push_back(pTechno);
		}
	}
	for (auto& objTag : ObjectTags) {
		for (auto& pTechno : objTag.second) {
			auto pType = pTechno->GetTechnoType();
			Log("[Trigger Info] %s, UID: %d", pType->ID, (int)pTechno->UniqueID);
			if (pTechno->IsOnMap && !pTechno->InLimbo && pTechno->IsAlive) {
				Log(", Location: (%d, %d):\n", (int)pTechno->GetMapCoords().X, (int)pTechno->GetMapCoords().Y);
			}
			else
				Log(":\n");
		}
		DumpTag(objTag.first);
	}
	Log("[Trigger Info] ================== Array_Cell ==================\n");
	std::map<TagClass*, std::vector<CellStruct>> CellTags;
	for (int i = 0; i < MapClass::Instance.TaggedCells.Count; i++) {
		auto mapCoord = MapClass::Instance.TaggedCells.GetItem(i);
		if (auto pCell = MapClass::Instance.TryGetCellAt(mapCoord)) {
			if (pCell->AttachedTag) {
				bool add = true;
				for (auto& mp : CellTags[pCell->AttachedTag]) {
					if (mp == pCell->MapCoords) {
						add = false;
					}
				}
				if (add) {
					CellTags[pCell->AttachedTag].push_back(pCell->MapCoords);
				}
			}
		}
	}
	for (auto& cellTag : CellTags) {
		for (auto& mapCoord : cellTag.second) {
			Log("[Trigger Info] Location: (%d, %d):\n", mapCoord.X, mapCoord.Y);
		}
		DumpTag(cellTag.first);
	}
	Log("[Trigger Info] ================== All_Triggers ==================\n");
	for (int i = 0; i < TriggerClass::Array.Count; i++) {
		if (auto trigger = TriggerClass::Array.GetItem(i)) {
			DumpAllTrigger(trigger);
		}
	}

	Message(L"Trigger Info Dumped");
}

void TriggerDebugClass::Execute(WWKey eInput) const
{
	bTriggerDebug = !bTriggerDebug;
}

void TriggerDebugPageUpClass::Execute(WWKey eInput) const
{
	if (bTriggerDebug && CurrentPage > 0)
	{
		CurrentPage--;
	}
}

void TriggerDebugPageDownClass::Execute(WWKey eInput) const
{
	if (bTriggerDebug && !bTriggerDebugPageEnd)
	{
		CurrentPage++;
	}
}

static void ProcessActions(TActionClass* pAction, TriggerClass* pTrigger, int counter)
{
	if (pAction) {
		switch (Mode)
		{
		case ForceRun:
			pAction->Execute(HouseClass::FindByCountryIndex(pTrigger->Type->House->ArrayIndex), NULL, pTrigger, CellStruct{ 0,0 });
			break;
		case Enable:
		case Disable:
		default:
			break;
		}

		ProcessActions(pAction->NextAction, pTrigger, counter + 1);
	}
}

void ProcessTriggers(TriggerClass* pTrigger)
{
	if (pTrigger) {
		switch (Mode)
		{
		case ForceRun:
			ProcessActions(pTrigger->Type->FirstAction, pTrigger, 0);
			TriggerExtMap[pTrigger].LastExecutedFrame = Unsorted::CurrentFrame;
			TriggerExtMap[pTrigger].ExecutedCount++;
			Message(L"Executed trigger <%s>", A2W(pTrigger->Type->Name).c_str());
			break;
		case Enable:
		{
			pTrigger->Enable();
			auto& ext = TriggerExtMap[pTrigger];
			if (ext.ResetTimer > -1)
			{
				pTrigger->Timer.TimeLeft = ext.ResetTimer;
				ext.ResetTimer = -1;
			}
			Message(L"Enabled trigger <%s>", A2W(pTrigger->Type->Name).c_str());
			break;
		}
		case Disable:
			pTrigger->Disable();
			Message(L"Disabled trigger <%s>", A2W(pTrigger->Type->Name).c_str());
			break;
		case Destroy:
			pTrigger->Destroy();
			Message(L"Destroyed trigger <%s>", A2W(pTrigger->Type->Name).c_str());
			break;
		case ChangeTimer:
			if (pTrigger->Enabled)
			{
				int elapsed = pTrigger->Timer.TimeLeft - pTrigger->Timer.GetTimeLeft();
				pTrigger->Timer.TimeLeft = ChangedTimer + elapsed;
			}
			else
			{
				pTrigger->Timer.TimeLeft = ChangedTimer;
				TriggerExtMap[pTrigger].ResetTimer = ChangedTimer;
			}
			Message(L"Set trigger <%s> timer to %d", A2W(pTrigger->Type->Name).c_str(), ChangedTimer);
			break;
		default:
			break;
		}
	}
}

void DrawTriggerDebug()
{
	auto DrawText = [](const wchar_t* string, int& offsetX, int& offsetY, int color, int index) {

		auto h = DSurface::Composite->GetHeight();

		auto wanted = GetTextDimensionsCompat(string);
		wanted.Height = 14;

		if (wanted.Height + offsetY >= h - 100)
			return false;

		RectangleStruct rect = { offsetX , offsetY, wanted.Width, wanted.Height };
		TriggerDebugRect[index] = rect;

		DrawTextOutline(string, rect.X, rect.Y, color);

		offsetY += wanted.Height;
		return true;
		};

	SortTriggerArray(Sort);
	int DisplayX = TriggerDebugStartX;
	int DisplayY = TriggerDebugStartY;
	for (int i = 0; i < RECT_COUNT; ++i)
	{
		TriggerDebugRect[i] = { 0,0,0,0 };
	}
	bTriggerDebugPageEnd = true;

	if (bTriggerDebugTimerEdited && !MessageListClass::Instance.HasEditFocus())
	{
		bTriggerDebugTimerEdited = false;
		ChangedTimer = _wtoi(MessageListClass::Instance.GetEditBuffer());
	}

	for (int i = CurrentPage * PageTriggerCount; i < std::min((int)SortedAllTriggers.size(), (CurrentPage + 1) * PageTriggerCount); i++) {
		std::string text;
		bool enabled = false;
		auto& obj = SortedAllTriggers[i];
		if (std::holds_alternative<TriggerClass**>(obj.item))
		{
			auto trigger = *std::get<TriggerClass**>(obj.item);
			auto& ext = TriggerExtMap[trigger];
			if (trigger->Enabled)
				enabled = true;
			text = Format("%s (%s)", trigger->Type->get_ID(), trigger->Type->Name);
			if (bTriggerDebugDetailed)
			{
				int timeLeft = trigger->Enabled ? trigger->Timer.GetTimeLeft() : trigger->Timer.TimeLeft;
				if (timeLeft > 0)
				{
					text += Format(", Frame Left(s): %d(%d)", timeLeft, timeLeft / 15);
					if (ext.ResetTimer > -1)
						text += " (Modified)";
				}
				int lastExecutedFrame = ext.LastExecutedFrame;
				if (lastExecutedFrame > -1)
				{
					text += Format(", Last Executed Frame(s): %d(%d)", lastExecutedFrame, lastExecutedFrame / 15);
				}
				int executedCount = ext.ExecutedCount;
				if (executedCount > 0)
				{
					text += Format(", Execute Count: %d", executedCount);
				}
				if (trigger->Enabled)
				{
					if (!ext.OccuredEvents.empty())
					{
						std::vector<int> eventList;
						GetEventList(trigger->Type->FirstEvent, eventList);
						text += ", Conditions:";
						for (int j = eventList.size() - 1; j >= 0; --j)
						{
							text += Format(" %d[%s]", eventList[j], ext.OccuredEvents[j] ? "@" : "  ");
						}
					}
				}
			}
		}
		else
		{
			auto& trigger = *std::get<TriggerClassExt*>(obj.item);
			text = Format("<Expired> %s (%s)", trigger.Type->get_ID(), trigger.Type->Name);
			if (bTriggerDebugDetailed)
			{
				int lastExecutedFrame = trigger.LastExecutedFrame;
				if (lastExecutedFrame > -1)
				{
					text += Format(", Last Executed Frame(s): %d(%d)", lastExecutedFrame, lastExecutedFrame / 15);
				}
				int executedCount = trigger.ExecutedCount;
				if (executedCount > 0)
				{
					text += Format(", Execute Count: %d", executedCount);
				}
				int destroyedFrame = trigger.DestroyedFrame;
				if (destroyedFrame > 0)
				{
					text += Format(", Expired Frame(s): %d(%d)", destroyedFrame, destroyedFrame / 15);
				}
			}
		}

		auto wtext = A2W(text.c_str());
		int textColor;
		if (!std::holds_alternative<TriggerClass**>(obj.item))
		{
			textColor = RGB8882RGB565(200, 60, 60);
		}
		else if (enabled)
		{
			textColor = RGB8882RGB565(0, 180, 0);
		}
		else
		{
			textColor = RGB8882RGB565(140, 140, 140);
		}
		if (!DrawText(wtext.c_str(),
			DisplayX, DisplayY, textColor, i - CurrentPage * PageTriggerCount))
		{
			if (PageTriggerCount == RECT_COUNT)
				PageTriggerCount = i - CurrentPage * PageTriggerCount;
			bTriggerDebugPageEnd = false;
			break;
		}
		if (i == (CurrentPage + 1) * PageTriggerCount - 1 && i != (int)SortedAllTriggers.size() - 1)
			bTriggerDebugPageEnd = false;
	}


	const wchar_t* pageUp = L"Page Up";
	const wchar_t* pageDown = L"Page Down";
	const wchar_t* Detail = L"Details";
	std::wstring SortType = L"Sort: ";
	switch (Sort)
	{
	case Raw:
		SortType += L"Raw";
		break;
	case ByID:
		SortType += L"ID";
		break;
	case ByName:
		SortType += L"Name";
		break;
	case ByTimeLeft:
		SortType += L"Frame Left";
		break;
	case ByLastExecuted:
		SortType += L"Last Executed";
		break;
	case ByDestroyed:
		SortType += L"Expired";
		break;
	default:
		break;
	}
	const wchar_t* Search = L"Search";
	const wchar_t* EnableChanged = L"Enable Timer-modified";
	int upRight = 0;

	{
		auto wanted = GetTextDimensionsCompat(pageUp);
		wanted.Height = 14;

		RectangleStruct rect = { TriggerDebugStartX , TriggerDebugStartY - wanted.Height, wanted.Width, wanted.Height };
		TriggerDebugPageUp = rect;
		upRight = TriggerDebugStartX + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(pageUp, rect.X, rect.Y, COLOR_WHITE);
	}
	{
		auto wanted = GetTextDimensionsCompat(pageDown);
		wanted.Height = 14;

		RectangleStruct rect = { upRight + 10 , TriggerDebugStartY - wanted.Height, wanted.Width, wanted.Height };
		TriggerDebugPageDown = rect;
		upRight = rect.X + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(pageDown, rect.X, rect.Y, COLOR_WHITE);
	}
	{
		auto wanted = GetTextDimensionsCompat(Detail);
		wanted.Height = 14;

		RectangleStruct rect = { upRight + 10 , TriggerDebugStartY - wanted.Height, wanted.Width, wanted.Height };
		TriggerDebugDetailed = rect;
		upRight = rect.X + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(Detail, rect.X, rect.Y, bTriggerDebugDetailed ? COLOR_RED : COLOR_WHITE);
	}
	{
		auto wanted = GetTextDimensionsCompat(SortType.c_str());
		wanted.Height = 14;

		RectangleStruct rect = { upRight + 10 , TriggerDebugStartY - wanted.Height, wanted.Width, wanted.Height };
		TriggerDebugSort = rect;
		upRight = rect.X + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(SortType.c_str(), rect.X, rect.Y, COLOR_WHITE);
	}
	{
		auto wanted = GetTextDimensionsCompat(Search);
		wanted.Height = 14;

		RectangleStruct rect = { upRight + 10 , TriggerDebugStartY - wanted.Height, wanted.Width, wanted.Height };
		TriggerDebugSearch = rect;
		upRight = rect.X + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(Search, rect.X, rect.Y, COLOR_WHITE);
	}
	{
		auto wanted = GetTextDimensionsCompat(EnableChanged);
		wanted.Height = 14;

		RectangleStruct rect = { upRight + 10 , TriggerDebugStartY - wanted.Height, wanted.Width, wanted.Height };
		TriggerDebugEnableModified = rect;
		upRight = rect.X + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(EnableChanged, rect.X, rect.Y, COLOR_WHITE);
	}

	const wchar_t* Modes[ModeCount] =
	{
		L"Run", L"Enable", L"Disable", L"Destroy", L"Set Timer"
	};
	int modesRight[ModeCount]{ 0 };
	for (int i = 0; i < ModeCount; ++i)
	{
		auto wanted = GetTextDimensionsCompat(Modes[i]);
		wanted.Height = 14;

		RectangleStruct rect = { TriggerDebugStartX , TriggerDebugPageDown.Y - wanted.Height, wanted.Width, wanted.Height };
		if (i > 0)
		{
			rect.X = modesRight[i - 1] + 10;
		}
		TriggerDebugMode[i] = rect;
		modesRight[i] = rect.X + wanted.Width;

		// DrawBtn removed - text only with outline
		DrawTextOutline(Modes[i], rect.X, rect.Y, (int)Mode == i ? COLOR_RED : COLOR_WHITE);
	}
}

DEFINE_HOOK(0x69300B, ScrollClass_MouseUpdate_SkipMouseActionUpdate, 6)
{
	if (!bTriggerDebug && !bAITriggerDebug)
		return 0;

	enum { SkipGameCode = 0x69301A };
	const Point2D mousePosition = WWMouseClass::Instance->XY1;

	// Handle numpad +/- for weight adjustment
	if (bAITriggerDebug)
		HandleAITriggerDebugNumpad();

	auto isInRect = [&](const RectangleStruct& rect)
		{
			return rect.X <= mousePosition.X && mousePosition.X <= rect.X + rect.Width &&
				rect.Y <= mousePosition.Y && mousePosition.Y <= rect.Y + rect.Height;
		};

	if (bTriggerDebug)
	{
		HoveredTriggerIndex = -100;
		ModeIndex = -1;
		for (int i = 0; i < PageTriggerCount; ++i)
		{
			if (isInRect(TriggerDebugRect[i]))
			{
				HoveredTriggerIndex = i + CurrentPage * PageTriggerCount;
				R->Stack(STACK_OFFS(0x30, -0x24), 0);
				R->EAX(Action::None);
				return SkipGameCode;
			}
		}
		if (isInRect(TriggerDebugPageUp))
		{
			HoveredTriggerIndex = -1;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(TriggerDebugPageDown))
		{
			HoveredTriggerIndex = -2;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(TriggerDebugDetailed))
		{
			HoveredTriggerIndex = -3;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(TriggerDebugSort))
		{
			HoveredTriggerIndex = -4;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(TriggerDebugSearch))
		{
			HoveredTriggerIndex = -5;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(TriggerDebugEnableModified))
		{
			HoveredTriggerIndex = -6;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		for (int i = 0; i < ModeCount; ++i)
		{
			if (isInRect(TriggerDebugMode[i]))
			{
				ModeIndex = i;
				R->Stack(STACK_OFFS(0x30, -0x24), 0);
				R->EAX(Action::None);
				return SkipGameCode;
			}
		}
	}

	if (bAITriggerDebug)
	{
		AITriggerDebugHoveredIndex = -100;
		for (int i = 0; i < AITriggerDebugPageItemCount; ++i)
		{
			if (isInRect(AITriggerDebugRect[i]))
			{
				AITriggerDebugHoveredIndex = i;
				R->Stack(STACK_OFFS(0x30, -0x24), 0);
				R->EAX(Action::None);
				return SkipGameCode;
			}
		}
		if (isInRect(AITriggerDebugPageUp))
		{
			AITriggerDebugHoveredIndex = -1;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(AITriggerDebugPageDown))
		{
			AITriggerDebugHoveredIndex = -2;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
		if (isInRect(AITriggerDebugHouseLeft))
		{
			AITriggerDebugHoveredIndex = -3;
			R->Stack(STACK_OFFS(0x30, -0x24), 0);
			R->EAX(Action::None);
			return SkipGameCode;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6931A5, ScrollClass_WindowsProcedure_PressLeftMouseButton, 6)
{
	enum { SkipGameCode = 0x6931B4 };

	if (bTriggerDebug)
	{
		if (HoveredTriggerIndex >= 0)
		{
			if (HoveredTriggerIndex < (int)SortedAllTriggers.size())
			{
				auto& obj = SortedAllTriggers[HoveredTriggerIndex];
				if (std::holds_alternative<TriggerClass**>(obj.item))
				{
					ProcessTriggers(*std::get<TriggerClass**>(obj.item));
				}
				else
				{
					auto& ext = *std::get<TriggerClassExt*>(obj.item);
					if (ext.Type)
					{
						switch (Mode)
						{
						case ForceRun:
						{
							auto newTrigger = TriggerClass::GetInstance(ext.Type);
							if (newTrigger)
							{
								ProcessTriggers(newTrigger);
								newTrigger->Destroy();
							}
							break;
						}
						case Enable:
						case Disable:
						case Destroy:
						case ChangeTimer:
						default:
							break;
						}
					}
				}
			}
		}
		else if (HoveredTriggerIndex > -100)
		{
			switch (HoveredTriggerIndex)
			{
			case -1:
			{
				if (CurrentPage > 0)
				{
					CurrentPage--;
				}
				break;
			}
			case -2:
			{
				if (!bTriggerDebugPageEnd)
				{
					CurrentPage++;
				}
				break;
			}
			case -3:
			{
				bTriggerDebugDetailed = !bTriggerDebugDetailed;
				break;
			}
			case -4:
			{
				Sort = TriggerSort(Sort + 1);
				if (Sort == end)
					Sort = Raw;
				break;
			}
			case -5:
			{
				if (!MessageListClass::Instance.HasEditFocus())
				{
					MessageListClass::Instance.RemoveEdit();
					MessageListClass::Instance.AddEdit(0, TextPrintType::BrightColor, L"");
					bTriggerDebugEdited = true;
				}
				break;
			}
			case -6:
			{
				for (int i = 0; i < TriggerClass::Array.Count; i++) {
					auto pTrigger = TriggerClass::Array.GetItem(i);
					auto& ext = TriggerExtMap[pTrigger];
					if (ext.ResetTimer > -1)
					{
						pTrigger->Enable();
						pTrigger->Timer.TimeLeft = ext.ResetTimer;
						ext.ResetTimer = -1;
					}
				}
				Message(L"Enabled all triggers with modified timer");
				break;
			}
			default:
				break;
			}
			bPressedInButtonsLayer = true;
			R->Stack(STACK_OFFS(0x28, 0x8), 0);
			R->EAX(Action::None);
		}
		else if (ModeIndex > -1)
		{
			Mode = CurrentMode(ModeIndex);
			bPressedInButtonsLayer = true;
			if (Mode == ChangeTimer && !MessageListClass::Instance.HasEditFocus())
			{
				MessageListClass::Instance.RemoveEdit();
				MessageListClass::Instance.AddEdit(0, TextPrintType::BrightColor, L"");
				bTriggerDebugTimerEdited = true;
			}
			R->Stack(STACK_OFFS(0x28, 0x8), 0);
			R->EAX(Action::None);
		}
	}

	if (bAITriggerDebug)
	{
		if (AITriggerDebugHoveredIndex != -100)
		{
			HandleAITriggerDebugClick();
			bPressedInButtonsLayer = true;
			R->Stack(STACK_OFFS(0x28, 0x8), 0);
			R->EAX(Action::None);
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x693268, ScrollClass_WindowsProcedure_ReleaseLeftMouseButton, 5)
{
	enum { SkipGameCode = 0x693276 };

	if (bPressedInButtonsLayer)
	{
		bPressedInButtonsLayer = false;
		R->Stack(STACK_OFFS(0x28, 0x8), 0);
		R->EAX(Action::None);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x692F85, ScrollClass_MouseUpdate_SkipMouseLongPress, 7)
{
	enum { CheckMousePress = 0x692F8E, CheckMouseNoPress = 0x692FDC };

	GET(ScrollClass*, pThis, EBX);

	return (pThis->unknown_byte_554A && !bPressedInButtonsLayer) ? CheckMousePress : CheckMouseNoPress;
}

DEFINE_HOOK(0x6851F0, Logic_Init, 5)
{
	CurrentPage = 0;
	TriggerExtMap.clear();
	SortedTriggerArray.clear();
	DestroyedTriggers.clear();
	return 0;
}

DEFINE_HOOK(0x7265D1, TriggerClass_FireActions, 5)
{
	GET(TriggerClass*, pTrigger, EDI);
	TriggerExtMap[pTrigger].LastExecutedFrame = Unsorted::CurrentFrame;
	TriggerExtMap[pTrigger].ExecutedCount++;
	return 0;
}

DEFINE_HOOK(0x7264C0, TriggerClass_RegisterEvent_Clear, 7)
{
	GET(TriggerClass*, pThis, ECX);
	TriggerExtMap[pThis].OccuredEvents.clear();

	int count = 0;
	GetEventCount(pThis->Type->FirstEvent, count);
	for (int i = 0; i < count; ++i)
	{
		TriggerExtMap[pThis].OccuredEvents.push_back(false);
	}
	return 0;
}

DEFINE_HOOK(0x726564, TriggerClass_RegisterEvent_Record, 6)
{
	GET(TriggerClass*, pThis, ESI);
	int eventBit = R->EBP();
	if (eventBit > 0)
	{
		int index = (int)log2(eventBit);
		auto& events = TriggerExtMap[pThis].OccuredEvents;
		if (index >= 0 && index < (int)events.size())
		{
			events[index] = true;
		}
	}
	return 0;
}

DEFINE_HOOK(0x726720, TriggerClass_Destroy, 7)
{
	GET(TriggerClass*, pThis, ECX);
	auto& destroyed = DestroyedTriggers.emplace_back(TriggerExtMap[pThis]);

	destroyed.Destroyed = true;
	destroyed.DestroyedFrame = Unsorted::CurrentFrame;
	destroyed.Type = pThis->Type;

	return 0;
}
