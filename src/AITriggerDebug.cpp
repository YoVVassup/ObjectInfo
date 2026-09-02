#include "Common.h"
#include <WWMouseClass.h>

bool bAITriggerDebug = false;
int AITriggerDebugHoveredIndex = -100;
int AITriggerDebugPage = 0;
int AITriggerDebugSelectedHouse = -1;
int AITriggerDebugPageItemCount = 0;

RectangleStruct AITriggerDebugRect[AI_TRIGGER_RECT_COUNT]{0};
RectangleStruct AITriggerDebugPageUp{0};
RectangleStruct AITriggerDebugPageDown{0};
RectangleStruct AITriggerDebugHouseLeft{0};

int AITriggerDebugStartX = -780;
int AITriggerDebugStartY = 180;
static const int AITriggerDebugPerPage = 30;

// Cached filtered triggers to avoid O(N) rescan on every click
static std::vector<AITriggerTypeClass*> sFilteredTriggers;
static int sFilteredHouseIndex = -2; // invalid sentinel to force initial rebuild

// Column widths
static const int COL_STATUS = 15;
static const int COL_ID = 90;
static const int COL_NAME = 190;
static const int COL_CONDITION = 180;
static const int COL_WEIGHT = 90;
static const int COL_TL = 30;
static const int COL_TEAM = 185;
static const int TOTAL_WIDTH = COL_STATUS + COL_ID + COL_NAME + COL_CONDITION + COL_WEIGHT + COL_TL + COL_TEAM;

// Marquee state
static const int MARQUEE_SPEED = 3;
static const int BOTTOM_MARGIN = 10;
static int AITriggerDebugMarqueeOffset = 0;
static int AITriggerDebugMarqueeTimer = 0;

static void GetAIHouses(std::vector<HouseClass*>& out);
static int GetTotalRelevantTriggers(const std::vector<HouseClass*>& aiHouses, int selectedHouse);

void AITriggerDebugClass::Execute(WWKey eInput) const
{
	bAITriggerDebug = !bAITriggerDebug;
	if (bAITriggerDebug)
	{
		AITriggerDebugPage = 0;
		AITriggerDebugSelectedHouse = -1;
		sFilteredHouseIndex = -2;
		AITriggerDebugMarqueeOffset = 0;
		AITriggerDebugMarqueeTimer = 0;
	}
}

void AITriggerDebugPageUpClass::Execute(WWKey eInput) const
{
	if (bAITriggerDebug)
	{
		AITriggerDebugMarqueeOffset = 0;
		if (AITriggerDebugPage > 0)
			AITriggerDebugPage--;
		else
		{
			std::vector<HouseClass*> aiHouses;
			GetAIHouses(aiHouses);
			int total = GetTotalRelevantTriggers(aiHouses, AITriggerDebugSelectedHouse);
			int maxPage = (total + AITriggerDebugPerPage - 1) / AITriggerDebugPerPage - 1;
			if (maxPage > 0)
				AITriggerDebugPage = maxPage;
		}
	}
}

void AITriggerDebugPageDownClass::Execute(WWKey eInput) const
{
	if (bAITriggerDebug)
	{
		AITriggerDebugMarqueeOffset = 0;
		std::vector<HouseClass*> aiHouses;
		GetAIHouses(aiHouses);
		int total = GetTotalRelevantTriggers(aiHouses, AITriggerDebugSelectedHouse);
		int maxPage = (total + AITriggerDebugPerPage - 1) / AITriggerDebugPerPage - 1;
		if (maxPage < 0) maxPage = 0;

		AITriggerDebugPage++;
		if (AITriggerDebugPage > maxPage)
			AITriggerDebugPage = 0;
	}
}

static const char* GetConditionTypeName(AITriggerCondition type)
{
	switch (type)
	{
	case AITriggerCondition::Pool:             return "Pool";
	case AITriggerCondition::AIOwns:           return "AI Owns";
	case AITriggerCondition::EnemyOwns:        return "Enemy Owns";
	case AITriggerCondition::EnemyYellowPowe:  return "Enemy Y.Pwr";
	case AITriggerCondition::EnemyRedPower:    return "Enemy R.Pwr";
	case AITriggerCondition::EnemyCashExceeds: return "Enemy Cash>";
	case AITriggerCondition::IronCharged:      return "Iron Charged";
	case AITriggerCondition::ChronoCharged:    return "Chrono Charged";
	case AITriggerCondition::NeutralOwns:      return "Neutral Owns";
	default:                                   return "Unknown";
	}
}

static const char* GetHouseTypeName(AITriggerHouseType type)
{
	switch (type)
	{
	case AITriggerHouseType::None:   return "Any AI";
	case AITriggerHouseType::Single: return "Single";
	case AITriggerHouseType::Any:    return "Any of Side";
	default:                         return "Unknown";
	}
}

static const char* GetSideName(int sideIndex)
{
	if (sideIndex == 0)
		return "All";
	int arrayIdx = sideIndex - 1;
	if (arrayIdx >= 0 && arrayIdx < SideClass::Array.Count)
		return SideClass::Array.GetItem(arrayIdx)->get_ID();
	return "Unknown";
}

static const char* GetComparatorOperator(AITriggerConditionComparator cond)
{
	switch (cond.ComparatorType)
	{
	case AITriggerConditionComparatorType::Less:          return "<";
	case AITriggerConditionComparatorType::LessOrEqual:   return "<=";
	case AITriggerConditionComparatorType::Equal:         return "=";
	case AITriggerConditionComparatorType::GreaterOrEqual: return ">=";
	case AITriggerConditionComparatorType::Greater:       return ">";
	case AITriggerConditionComparatorType::NotEqual:      return "!=";
	default:                                              return "?";
	}
}

static int GetComparatorValue(AITriggerConditionComparator cond)
{
	return cond.ComparatorOperand;
}

static HouseClass* FindEnemyHouse(HouseClass* pHouse)
{
	if (!pHouse || pHouse->Defeated)
		return nullptr;

	int bestAnger = -1;
	HouseClass* pBestEnemy = nullptr;

	for (int i = 0; i < HouseClass::Array.Count; i++)
	{
		auto pOther = HouseClass::Array.GetItem(i);
		if (!pOther || pOther == pHouse || pOther->Defeated || pOther->IsObserver())
			continue;
		if (pOther->Type->MultiplayPassive)
			continue;
		if (pHouse->IsAlliedWith(pOther))
			continue;

		for (auto& node : pHouse->AngerNodes)
		{
			if (node.House == pOther && node.AngerLevel > bestAnger)
			{
				bestAnger = node.AngerLevel;
				pBestEnemy = pOther;
			}
		}
	}

	return pBestEnemy;
}

static HouseClass* FindCivilianHouse()
{
	for (int i = 0; i < HouseClass::Array.Count; i++)
	{
		auto pHouse = HouseClass::Array.GetItem(i);
		if (pHouse && pHouse->Type && pHouse->Type->MultiplayPassive && !pHouse->Defeated)
			return pHouse;
	}
	return nullptr;
}

static bool IsConditionMetForHouse(AITriggerTypeClass* pTrigger, HouseClass* pHouse)
{
	if (pTrigger->ConditionType == AITriggerCondition::Pool)
		return true;

	HouseClass* pTargetHouse = nullptr;

	if (pTrigger->ConditionType == AITriggerCondition::AIOwns
		|| pTrigger->ConditionType == AITriggerCondition::EnemyYellowPowe
		|| pTrigger->ConditionType == AITriggerCondition::EnemyRedPower
		|| pTrigger->ConditionType == AITriggerCondition::EnemyCashExceeds)
		pTargetHouse = FindEnemyHouse(pHouse);
	else if (pTrigger->ConditionType == AITriggerCondition::EnemyOwns
		|| pTrigger->ConditionType == AITriggerCondition::IronCharged
		|| pTrigger->ConditionType == AITriggerCondition::ChronoCharged)
		pTargetHouse = pHouse;
	else if (pTrigger->ConditionType == AITriggerCondition::NeutralOwns)
		pTargetHouse = FindCivilianHouse();

	if (!pTargetHouse && pTrigger->ConditionType != AITriggerCondition::Pool)
		return false;

	return pTrigger->ConditionMet(pHouse, pTargetHouse, false);
}

static bool IsTriggerRelevantForHouse(AITriggerTypeClass* pTrigger, HouseClass* pHouse)
{
	if (pTrigger->OwnerHouseType == AITriggerHouseType::Single)
	{
		if (pTrigger->HouseIndex == -1)
			return false;
		if (pTrigger->HouseIndex < 0 || pTrigger->HouseIndex >= HouseTypeClass::Array.Count)
			return false;
		auto pHouseType = HouseTypeClass::Array.GetItem(pTrigger->HouseIndex);
		if (!pHouseType)
			return false;
		if (pHouse->Type != pHouseType)
			return false;
	}
	else if (pTrigger->OwnerHouseType == AITriggerHouseType::Any)
	{
		if (pTrigger->SideIndex != 0 && pTrigger->SideIndex - 1 != pHouse->Type->SideIndex)
			return false;
	}

	return true;
}

static bool IsHouseAIControlled(HouseClass* pHouse)
{
	if (!pHouse || pHouse->Defeated)
		return false;
	return !pHouse->IsControlledByHuman();
}

static void GetAIHouses(std::vector<HouseClass*>& out)
{
	for (int i = 0; i < HouseClass::Array.Count; i++)
	{
		auto pHouse = HouseClass::Array.GetItem(i);
		if (IsHouseAIControlled(pHouse))
			out.push_back(pHouse);
	}
}

static void RebuildFilteredCache(const std::vector<HouseClass*>& aiHouses, int selectedHouse)
{
	if (selectedHouse == sFilteredHouseIndex)
		return;

	sFilteredTriggers.clear();
	sFilteredHouseIndex = selectedHouse;

	for (int i = 0; i < AITriggerTypeClass::Array.Count; i++)
	{
		auto pTrigger = AITriggerTypeClass::Array.GetItem(i);
		if (!pTrigger)
			continue;

		bool relevant = true;
		if (selectedHouse >= 0)
		{
			if (selectedHouse < (int)aiHouses.size())
				relevant = IsTriggerRelevantForHouse(pTrigger, aiHouses[selectedHouse]);
		}
		else
		{
			relevant = false;
			for (auto pHouse : aiHouses)
			{
				if (IsTriggerRelevantForHouse(pTrigger, pHouse))
				{
					relevant = true;
					break;
				}
			}
		}

		if (relevant)
			sFilteredTriggers.push_back(pTrigger);
	}
}

static int GetTotalRelevantTriggers(const std::vector<HouseClass*>& aiHouses, int selectedHouse)
{
	RebuildFilteredCache(aiHouses, selectedHouse);
	return (int)sFilteredTriggers.size();
}

static AITriggerTypeClass* GetAITriggerAtIndex(int hoveredIndex)
{
	if (hoveredIndex < 0 || hoveredIndex >= (int)sFilteredTriggers.size())
		return nullptr;

	return sFilteredTriggers[hoveredIndex];
}

static int DrawTextInColumn(const wchar_t* text, int x, int y, int maxWidth, int color, bool marquee = false)
{
	auto wanted = GetTextDimensionsCompat(text);
	int textWidth = wanted.Width;

	if (textWidth <= maxWidth)
	{
		DrawTextOutline(text, x, y, color);
		return textWidth;
	}

	if (!marquee)
	{
		// Truncate with ellipsis
		std::wstring truncated(text);
		while (!truncated.empty())
		{
			std::wstring test = truncated + L"...";
			if (GetTextDimensionsCompat(test.c_str()).Width <= maxWidth)
			{
				DrawTextOutline(test.c_str(), x, y, color);
				return maxWidth;
			}
			truncated.pop_back();
		}
		DrawTextOutline(text, x, y, color);
		return textWidth;
	}

	// Marquee mode - scroll text within column bounds
	int scrollPos = AITriggerDebugMarqueeOffset % (textWidth + 20);

	// Create text with padding for seamless loop
	std::wstring padded = std::wstring(text) + L"                    " + std::wstring(text);

	// Calculate visible portion
	if (scrollPos < (int)padded.size())
	{
		// Find how many characters fit
		std::wstring visible;
		int accWidth = 0;
		for (size_t i = scrollPos; i < padded.size() && accWidth < maxWidth; i++)
		{
			wchar_t buf[2] = { padded[i], 0 };
			int charW = GetTextDimensionsCompat(buf).Width;
			if (accWidth + charW > maxWidth)
				break;
			visible += padded[i];
			accWidth += charW;
		}
		if (!visible.empty())
			DrawTextOutline(visible.c_str(), x, y, color);
	}

	return maxWidth;
}

void DrawAITriggerDebug()
{
	if (!bAITriggerDebug)
		return;

	const int perPage = AITriggerDebugPerPage;
	int displayX = AITriggerDebugStartX;
	int displayY = AITriggerDebugStartY;

	if (displayX < 0)
	{
		auto w = DSurface::Composite->GetWidth();
		displayX = w + displayX;
	}

	for (int i = 0; i < AI_TRIGGER_RECT_COUNT; ++i)
		AITriggerDebugRect[i] = { 0, 0, 0, 0 };

	// Update marquee timer
	AITriggerDebugMarqueeTimer++;
	if (AITriggerDebugMarqueeTimer >= MARQUEE_SPEED)
	{
		AITriggerDebugMarqueeTimer = 0;
		AITriggerDebugMarqueeOffset++;
	}

	std::vector<HouseClass*> aiHouses;
	GetAIHouses(aiHouses);

	if (aiHouses.empty())
	{
		DrawTextOutline(L"No AI houses found", displayX, displayY, COLOR_WHITE);
		return;
	}

	// House navigation
	std::wstring houseNav;
	if (AITriggerDebugSelectedHouse < 0)
	{
		houseNav = L"< All AI Houses >";
	}
	else
	{
		if (AITriggerDebugSelectedHouse < (int)aiHouses.size())
		{
			auto pHouse = aiHouses[AITriggerDebugSelectedHouse];
			const char* sideName = GetSideName(pHouse->Type->SideIndex + 1);
			const char* parentName = pHouse->Type->ParentCountry;
			if (parentName && strcmp(parentName, pHouse->get_ID()) != 0)
				houseNav = A2W(Format("< %s [%s] (%s) -> %s >", pHouse->get_ID(), sideName, pHouse->PlainName, parentName).c_str());
			else
				houseNav = A2W(Format("< %s [%s] (%s) >", pHouse->get_ID(), sideName, pHouse->PlainName).c_str());
		}
		else
		{
			AITriggerDebugSelectedHouse = -1;
			houseNav = L"< All AI Houses >";
		}
	}

	{
		auto wanted = GetTextDimensionsCompat(houseNav.c_str());
		wanted.Height = TEXT_LINE_HEIGHT;
		AITriggerDebugHouseLeft = { displayX, displayY, wanted.Width, wanted.Height };
		DrawTextOutline(houseNav.c_str(), displayX, displayY, COLOR_WHITE);
		displayY += wanted.Height;
	}

	// Page Up/Down
	{
		int upRight = DrawTextButton(L"Page Up", displayX, displayY, COLOR_WHITE, AITriggerDebugPageUp);
		upRight = DrawTextButton(L"Page Down", upRight + 10, displayY, COLOR_WHITE, AITriggerDebugPageDown);

		// Page indicator
		int totalPages = (GetTotalRelevantTriggers(aiHouses, AITriggerDebugSelectedHouse) + perPage - 1) / perPage;
		if (totalPages < 1) totalPages = 1;
		int maxPage = totalPages - 1;
		if (AITriggerDebugPage > maxPage)
			AITriggerDebugPage = maxPage;
		if (AITriggerDebugPage < 0)
			AITriggerDebugPage = 0;
		std::wstring pageInfo = A2W(Format("Page %d/%d", AITriggerDebugPage + 1, totalPages).c_str());
		DrawTextOutline(pageInfo.c_str(), upRight + 10, displayY, COLOR_PAGE_INFO);

		displayY += TEXT_LINE_HEIGHT;
	}

	// Column header
	{
		struct ColumnDef { const wchar_t* name; int width; };
		static const ColumnDef columns[] = {
			{ L"St", COL_STATUS },
			{ L"ID", COL_ID },
			{ L"Name", COL_NAME },
			{ L"Condition", COL_CONDITION },
			{ L"Weight", COL_WEIGHT },
			{ L"TL", COL_TL },
			{ L"Team", COL_TEAM },
		};

		int colX = displayX;
		for (const auto& col : columns)
		{
			DrawTextOutline(col.name, colX + 2, displayY, COLOR_HEADER_DIM);
			colX += col.width;
		}

		displayY += TEXT_LINE_HEIGHT;
	}

	// Use cached filtered triggers
	RebuildFilteredCache(aiHouses, AITriggerDebugSelectedHouse);
	int row = 0;
	int skipped = 0;

	for (int i = 0; i < (int)sFilteredTriggers.size(); i++)
	{
		auto pTrigger = sFilteredTriggers[i];

		if (skipped < AITriggerDebugPage * perPage)
		{
			skipped++;
			continue;
		}

		if (row >= perPage)
			continue;

		auto h = DSurface::Composite->GetHeight();
		if (displayY + TEXT_LINE_HEIGHT >= h - BOTTOM_MARGIN)
			break;

		bool conditionMetAny = false;
		if (AITriggerDebugSelectedHouse >= 0)
		{
			if (AITriggerDebugSelectedHouse < (int)aiHouses.size())
				conditionMetAny = IsConditionMetForHouse(pTrigger, aiHouses[AITriggerDebugSelectedHouse]);
		}
		else
		{
			for (auto pHouse : aiHouses)
			{
				if (IsConditionMetForHouse(pTrigger, pHouse))
				{
					conditionMetAny = true;
					break;
				}
			}
		}

		int color;
		const char* statusIcon;
		if (!pTrigger->IsEnabled)
		{
			color = COLOR_TRIGGER_DESTROYED;
			statusIcon = "x";
		}
		else if (conditionMetAny)
		{
			color = COLOR_TRIGGER_ENABLED;
			statusIcon = "+";
		}
		else
		{
			color = COLOR_TRIGGER_DISABLED;
			statusIcon = "o";
		}

		if (pTrigger->IsEnabled && pTrigger->Weight_Maximum > 0)
		{
			double ratio = pTrigger->Weight_Current / pTrigger->Weight_Maximum;
			if (ratio < AI_TRIGGER_LOW_WEIGHT_RATIO && ratio >= 0.0)
				color = COLOR_TRIGGER_LOW_WEIGHT;
		}

		// Row rectangle (for click detection, no background)
		AITriggerDebugRect[row] = { displayX, displayY, TOTAL_WIDTH, 14 };

		// Draw each column
		int colX = displayX;

		// Status
		wchar_t statusW[2] = { (wchar_t)statusIcon[0], 0 };
		DrawTextOutline(statusW, colX + 4, displayY, color);
		colX += COL_STATUS;

		// ID (truncated)
		char idBuf[32] = { 0 };
		strncpy_s(idBuf, pTrigger->get_ID(), 12);
		idBuf[12] = '\0';
		DrawTextInColumn(A2W(idBuf).c_str(), colX + 2, displayY, COL_ID - 4, color);
		colX += COL_ID;

		// Name (marquee if too long)
		std::wstring name = pTrigger->Name ? A2W(pTrigger->Name) : std::wstring(L"");
		DrawTextInColumn(name.c_str(), colX + 2, displayY, COL_NAME - 4, color, true);
		colX += COL_NAME;

		// Condition: readable format
		const char* condName = GetConditionTypeName(pTrigger->ConditionType);
		const char* compOp = GetComparatorOperator(pTrigger->Conditions[0]);
		int compVal = GetComparatorValue(pTrigger->Conditions[0]);
		const char* objName = pTrigger->ConditionObject ? pTrigger->ConditionObject->get_ID() : nullptr;

		std::string condStr;
		if (objName)
			condStr = Format("%s %s%d [%s]", condName, compOp, compVal, objName);
		else if (compVal != 0 || pTrigger->Conditions[0].ComparatorType != AITriggerConditionComparatorType::Equal)
			condStr = Format("%s %s%d", condName, compOp, compVal);
		else
			condStr = Format("%s", condName);
		DrawTextInColumn(A2W(condStr.c_str()).c_str(), colX + 2, displayY, COL_CONDITION - 4, color, true);
		colX += COL_CONDITION;

		// Weight: min/current/max
		std::wstring weightStr;
		if (pTrigger->Weight_Current >= AI_TRIGGER_FIRE_IMMEDIATELY_WEIGHT)
			weightStr = A2W(Format("%.0f MAX!", pTrigger->Weight_Current).c_str());
		else
			weightStr = A2W(Format("%.0f/%.0f/%.0f", pTrigger->Weight_Minimum, pTrigger->Weight_Current, pTrigger->Weight_Maximum).c_str());
		DrawTextOutline(weightStr.c_str(), colX + 2, displayY, color);
		colX += COL_WEIGHT;

		// TechLevel
		std::wstring tlStr = A2W(Format("%d", pTrigger->TechLevel).c_str());
		DrawTextOutline(tlStr.c_str(), colX + 2, displayY, color);
		colX += COL_TL;

		// Team1/Team2
		std::wstring t1 = pTrigger->Team1 ? A2W(pTrigger->Team1->get_ID()) : std::wstring(L"-");
		std::wstring t2 = pTrigger->Team2 ? A2W(pTrigger->Team2->get_ID()) : std::wstring(L"-");
		std::wstring teamStr = t1 + L"/" + t2;
		DrawTextInColumn(teamStr.c_str(), colX + 2, displayY, COL_TEAM - 4, color);

		displayY += TEXT_LINE_HEIGHT;
		row++;
	}

	AITriggerDebugPageItemCount = row;

	if (row == 0)
	{
		DrawTextOutline(L"No matching AITriggerTypes", displayX, displayY, COLOR_WHITE);
	}
}

void HandleAITriggerDebugClick()
{
	if (AITriggerDebugHoveredIndex >= 0)
	{
		AITriggerTypeClass* pTrigger = GetAITriggerAtIndex(AITriggerDebugHoveredIndex);
		if (!pTrigger)
			return;

		// Check if click is on status column
		RectangleStruct rect = AITriggerDebugRect[AITriggerDebugHoveredIndex];
		int mouseX = WWMouseClass::Instance->XY1.X;

		if (mouseX < rect.X + COL_STATUS)
		{
			pTrigger->IsEnabled = !pTrigger->IsEnabled;
			Message(L"%s: %s", A2W(pTrigger->get_ID()).c_str(),
				pTrigger->IsEnabled ? L"Enabled" : L"Disabled");
		}
		else
		{
			std::string detail = Format("=== %s (%s) ===\n", pTrigger->get_ID(), pTrigger->Name ? pTrigger->Name : "N/A");
			detail += Format("Condition: %s", GetConditionTypeName(pTrigger->ConditionType));
			if (pTrigger->ConditionObject)
				detail += Format(" [%s]", pTrigger->ConditionObject->get_ID());
			detail += "\nConditions:";
			for (int c = 0; c < 4; c++)
				detail += Format("\n  [%d] %s %d", c, GetComparatorOperator(pTrigger->Conditions[c]), GetComparatorValue(pTrigger->Conditions[c]));
			detail += "\n";
			detail += Format("Owner: %s", GetHouseTypeName(pTrigger->OwnerHouseType));
			if (pTrigger->OwnerHouseType == AITriggerHouseType::Single && pTrigger->HouseIndex >= 0 && pTrigger->HouseIndex < HouseTypeClass::Array.Count)
				detail += Format(" (%s)", HouseTypeClass::Array.GetItem(pTrigger->HouseIndex)->get_ID());
			detail += "\n";
			detail += Format("Side: %s (%d), TechLevel: %d\n", GetSideName(pTrigger->SideIndex), pTrigger->SideIndex, pTrigger->TechLevel);
			detail += Format("Weight: %.0f / %.0f / %.0f%s\n",
				pTrigger->Weight_Minimum, pTrigger->Weight_Current, pTrigger->Weight_Maximum,
				pTrigger->Weight_Current >= AI_TRIGGER_FIRE_IMMEDIATELY_WEIGHT ? " [OVERRIDE - FIRE IMMEDIATELY]" : "");
			if (pTrigger->Team1)
				detail += Format("Team1: %s", pTrigger->Team1->get_ID());
			if (pTrigger->Team2)
				detail += Format(", Team2: %s", pTrigger->Team2->get_ID());
			detail += "\n";
			detail += Format("Executed: %d, Completed: %d\n", pTrigger->TimesExecuted, pTrigger->TimesCompleted);
			detail += Format("Difficulty: Easy=%d Normal=%d Hard=%d, Skirmish=%d, Global=%d\n",
				pTrigger->Enabled_Easy, pTrigger->Enabled_Normal, pTrigger->Enabled_Hard,
				pTrigger->IsForSkirmish, pTrigger->IsGlobal);
			detail += "Numpad +/-: weight +/-1, Ctrl: x10, Shift: /10";

			Message(L"%s", A2W(detail.c_str()).c_str());
		}
	}
	else if (AITriggerDebugHoveredIndex > -100)
	{
		switch (AITriggerDebugHoveredIndex)
		{
		case -1:
			if (AITriggerDebugPage > 0)
				AITriggerDebugPage--;
			break;
		case -2:
			AITriggerDebugPage++;
			break;
		case -3:
		{
			std::vector<HouseClass*> aiHouses;
			GetAIHouses(aiHouses);
			if (!aiHouses.empty())
			{
				AITriggerDebugSelectedHouse++;
				if (AITriggerDebugSelectedHouse >= (int)aiHouses.size())
					AITriggerDebugSelectedHouse = -1;
				sFilteredHouseIndex = -2;
				AITriggerDebugPage = 0;
				AITriggerDebugMarqueeOffset = 0;
			}
			break;
		}
		default:
			break;
		}
	}
}

void HandleAITriggerDebugNumpad()
{
	if (!bAITriggerDebug || AITriggerDebugHoveredIndex < 0)
		return;

	AITriggerTypeClass* pTrigger = GetAITriggerAtIndex(AITriggerDebugHoveredIndex);
	if (!pTrigger)
		return;

	bool numpadPlus = GetAsyncKeyState(VK_ADD) & 1;
	bool numpadMinus = GetAsyncKeyState(VK_SUBTRACT) & 1;

	if (!numpadPlus && !numpadMinus)
		return;

	double step = 1.0;
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		step = 10.0;
	else if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		step = 0.1;

	if (numpadPlus)
		pTrigger->Weight_Current += step;
	else
		pTrigger->Weight_Current -= step;

	if (pTrigger->Weight_Current < pTrigger->Weight_Minimum)
		pTrigger->Weight_Current = pTrigger->Weight_Minimum;
	if (pTrigger->Weight_Current > pTrigger->Weight_Maximum)
		pTrigger->Weight_Current = pTrigger->Weight_Maximum;

	Message(L"%s weight: %.1f (Numpad+/-: 1, Ctrl: 10, Shift: 0.1)", A2W(pTrigger->get_ID()).c_str(), pTrigger->Weight_Current);
}
