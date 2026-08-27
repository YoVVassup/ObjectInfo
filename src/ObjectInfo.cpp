#include "Common.h"

bool bObjectInfo = false;
bool bTriggerDebug = false;
bool bPressedInButtonsLayer = false;
bool bTriggerDebugPageEnd = false;
bool bTriggerDebugDetailed = false;
bool bTriggerDebugEdited = false;
bool bTriggerDebugTimerEdited = false;

char FinalStringBuffer[0x1000];
wchar_t FinalStringBufferW[0x1000];
wchar_t SearchPattern[0x200] = L"";

const char* GetMissionName(int mID)
{
	switch (mID)
	{
	case -1:  return "None";
	case 0:   return "Sleep";
	case 1:   return "Attack";
	case 2:   return "Move";
	case 3:   return "QMove";
	case 4:   return "Retreat";
	case 5:   return "Guard";
	case 6:   return "Sticky";
	case 7:   return "Enter";
	case 8:   return "Capture";
	case 9:   return "Eaten";
	case 10:  return "Harvest";
	case 11:  return "Area_Guard";
	case 12:  return "Return";
	case 13:  return "Stop";
	case 14:  return "Ambush";
	case 15:  return "Hunt";
	case 16:  return "Unload";
	case 17:  return "Sabotage";
	case 18:  return "Construction";
	case 19:  return "Selling";
	case 20:  return "Repair";
	case 21:  return "Rescue";
	case 22:  return "Missile";
	case 23:  return "Harmless";
	case 24:  return "Open";
	case 25:  return "Patrol";
	case 26:  return "ParadropApproach";
	case 27:  return "ParadropOverfly";
	case 28:  return "Wait";
	case 29:  return "AttackMove";
	case 30:  return "SpyplaneApproach";
	case 31:  return "SpyplaneOverfly";
	default:  return "INVALID_MISSION";
	}
}

class ObjectInfoClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Display Object Info";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_OBJECT_INFO", L"Display Object Info");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_OBJECT_INFO_DESC", L"Display objects' information along with them.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		bObjectInfo = !bObjectInfo;
	}
};

class ObjectInfoChangeClass : public CommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Next Info Preset";
	}
	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_OBJECT_INFO_NEXT", L"Next Info Preset");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_OBJECT_INFO_NEXT_DESC", L"Change to next display object info preset.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		ObjectInfoDisplay::ChangeNextList();
	}
};

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 6)
{
	MakeCommand<ObjectInfoClass>();
	MakeCommand<ObjectInfoChangeClass>();
	MakeCommand<TriggerInfoClass>();
	MakeCommand<TriggerDebugClass>();
	MakeCommand<TriggerDebugPageUpClass>();
	MakeCommand<TriggerDebugPageDownClass>();
	MakeCommand<AITriggerDebugClass>();
	MakeCommand<AITriggerDebugPageUpClass>();
	MakeCommand<AITriggerDebugPageDownClass>();

	return 0;
}

DEFINE_HOOK(0x5FACDF, _Options_LoadFromINI, 5)
{
	CCINIClass* pINI = OpenConfig("objectinfo.ini");

	if (!pINI)
		return 0;

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

	TriggerDebugStartX = pINI->ReadInteger("TriggerDebugPosition", "X", TriggerDebugStartX);
	TriggerDebugStartY = pINI->ReadInteger("TriggerDebugPosition", "Y", TriggerDebugStartY);

	AITriggerDebugStartX = pINI->ReadInteger("AITriggerDebugPosition", "X", AITriggerDebugStartX);
	AITriggerDebugStartY = pINI->ReadInteger("AITriggerDebugPosition", "Y", AITriggerDebugStartY);

	CloseConfig(pINI);
	return 0;
}

DEFINE_HOOK(0x4F4583, GScreenClass_DrawOnTop_TheDarkSideOfTheMoon, 6)
{
	if (bObjectInfo)
	{
		for (auto pTechno : TechnoClass::Array)
		{
			if (pTechno->IsMouseHovering || pTechno->IsSelected)
			{
				Point2D position;
				TacticalClass::Instance->CoordsToClient(&pTechno->GetCoords(), &position);

				int offsetX = position.X + ObjectInfoDisplay::DisplayOffsetX;
				int offsetY = position.Y + ObjectInfoDisplay::DisplayOffsetY;
				bool opposite = false;
				bool draw = true;

				if (DSurface::Composite->GetHeight() - position.Y < SCREEN_BOTTOM_THRESHOLD)
				{
					opposite = true;
					offsetY = position.Y - 30;
				}
				if ((position.X < -SCREEN_EDGE_MARGIN || position.X > DSurface::Composite->GetWidth() + SCREEN_EDGE_MARGIN)
					|| (position.Y < -SCREEN_EDGE_MARGIN || position.Y > DSurface::Composite->GetHeight() + SCREEN_EDGE_MARGIN))
					draw = false;

				char displayBuffer[0x800] = { 0 };

				auto DrawText = [&opposite, &draw](const wchar_t* string, int& offsetX, int& offsetY, int color) {
					if (draw)
					{
						auto h = DSurface::Composite->GetHeight();
						auto w = DSurface::Composite->GetWidth();

						auto wanted = GetTextDimensionsCompat(string);
						wanted.Height = 14;

						int exceedX = w - offsetX - wanted.Width;
						if (exceedX >= 0)
							exceedX = 0;

						RectangleStruct rect = { offsetX + exceedX, offsetY, wanted.Width, wanted.Height };

						DrawTextOutline(string, rect.X, rect.Y, color);

						if (opposite)
							offsetY -= wanted.Height;
						else
							offsetY += wanted.Height;
					}
					};
				auto DrawToolTipText = [&opposite, &draw](const wchar_t* string, int& offsetX, int& offsetY, int color) {
					if (draw)
					{
						std::vector<std::wstring> lines;
						std::wstring input(string);
						size_t pos = 0;
						while ((pos = input.find(L'\n')) != std::wstring::npos)
						{
							lines.emplace_back(input.substr(0, pos));
							input.erase(0, pos + 1);
						}
						lines.push_back(input);

						auto h = DSurface::Composite->GetHeight();
						auto w = DSurface::Composite->GetWidth();
						RectangleStruct wanted = { 0, 0, 0, 0 };

						for (const auto& line : lines)
						{
							auto wanted2 = GetTextDimensionsCompat(line.c_str());
							wanted.Height += wanted2.Height + 2;
							if (wanted.Width < wanted2.Width)
								wanted.Width = wanted2.Width;
						}

						int exceedX = w - offsetX - wanted.Width;
						if (exceedX >= 0)
							exceedX = 0;

						RectangleStruct rect = { offsetX + exceedX + 4, offsetY , wanted.Width, wanted.Height };

						for (const auto& line : lines)
						{
							auto wanted2 = GetTextDimensionsCompat(line.c_str());
							DrawTextOutline(line.c_str(), rect.X, rect.Y, color);
							rect.Y += wanted2.Height + 2;
						}

						wanted.Height = 14;

						if (opposite)
							offsetY -= wanted.Height;
						else
							offsetY += wanted.Height;
					}
					};

				auto append = [&displayBuffer](const char* pFormat, ...)
					{
						char buffer[0x100];
						va_list args;
						va_start(args, pFormat);
						vsnprintf(buffer, sizeof(buffer), pFormat, args);
						va_end(args);
						strcat_s(displayBuffer, buffer);
					};
				auto display = [&displayBuffer, &DrawText, &offsetX, &offsetY]()
					{
						wchar_t bufferW[0x800] = { 0 };
						mbstowcs(bufferW, displayBuffer, 0x7FF);
						DrawText(bufferW, offsetX, offsetY, COLOR_WHITE);

						displayBuffer[0] = 0;
					};
				auto displayToolTip = [&DrawToolTipText, &offsetX, &offsetY, &pTechno](const wchar_t* pFormat, ...)
					{
						wchar_t buffer[0x100] = { 0 };
						va_list args;
						va_start(args, pFormat);
						vswprintf_s(buffer, 0x100, pFormat, args);
						va_end(args);

						if (pTechno->Owner)
						{
							auto color = pTechno->Owner->Color;
							DrawToolTipText(buffer, offsetX, offsetY, RGB8882RGB565(color.R, color.G, color.B));
						}
					};


				auto footFields = [&append, &display](TechnoClass* pTechno, const std::string& name, bool allDisplay)
					{
						auto pFoot = static_cast<FootClass*>(pTechno);

						if (pFoot->BelongsToATeam())
						{
							auto pTeam = pFoot->Team;
							auto pTeamType = pFoot->Team->Type;
							if (ObjectInfoDisplay::CanDisplay("aitrigger", name) || allDisplay)
							{
								bool found = false;
								for (int i = 0; i < AITriggerTypeClass::Array.Count && !found; i++)
								{
									auto pTriggerTeam1Type = AITriggerTypeClass::Array.GetItem(i)->Team1;
									auto pTriggerTeam2Type = AITriggerTypeClass::Array.GetItem(i)->Team2;

									if (pTeamType && ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType) || (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
									{
										found = true;
										auto pTriggerType = AITriggerTypeClass::Array.GetItem(i);
										append("Trigger ID = %s, weights [Current, Min, Max]: %f, %f, %f", pTriggerType->ID, pTriggerType->Weight_Current, pTriggerType->Weight_Minimum, pTriggerType->Weight_Maximum);
									}
								}
								if (found)
									display();
							}
							if (ObjectInfoDisplay::CanDisplay("team", name) || allDisplay)
							{
								if (pTeam->Type && pTeam->CurrentScript && pTeam->CurrentScript->Type && pTeam->Type->TaskForce)
								{
									append("Team ID = %s, Script ID = %s, Taskforce ID = %s",
										pTeam->Type->ID, pTeam->CurrentScript->Type->get_ID(), pTeam->Type->TaskForce->ID);
									display();
								}
							}
							if (ObjectInfoDisplay::CanDisplay("currentscript", name) || allDisplay)
							{
								if (pTeam->Type && pTeam->Type->TaskForce && pTeam->CurrentScript)
								{
									bool missingUnit = false;
									for (int i = 0; i < TASKFORCE_MAX_ENTRIES; ++i)
									{
										auto pEntry = pTeam->Type->TaskForce->Entries[i];
										if (pEntry.Type && pEntry.Amount > 0)
										{
											int missing = pEntry.Amount;

											for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
											{
												auto pUnitType = pUnit->GetTechnoType();

												if (pUnitType
													&& pUnit->IsAlive
													&& pUnit->Health > 0
													&& !pUnit->InLimbo)
												{
													if (pEntry.Type->ID == pUnitType->ID)
													{
														missing--;
													}
												}
											}

											if (missing > 0)
											{
												missingUnit = true;
											}
										}
									}
									if (pTeam->CurrentScript->CurrentMission >= 0)
									{
										ScriptActionNode sNode;
										pTeam->CurrentScript->GetCurrentAction(&sNode);
										append("Current Script [Line = Action, Argument]: %d = %d, %d", pTeam->CurrentScript->CurrentMission, sNode.Action, sNode.Argument);

									}
									else if (!missingUnit)
										append("Current Script [Line = Action, Argument]: %d", pTeam->CurrentScript->CurrentMission);
									else
									{
										append("Team Missing: %d", pTeam->CurrentScript->CurrentMission);
										for (int i = 0; i < TASKFORCE_MAX_ENTRIES; ++i)
										{
											auto pEntry = pTeam->Type->TaskForce->Entries[i];
											if (pEntry.Type && pEntry.Amount > 0)
											{
												int missing = pEntry.Amount;

												for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
												{
													auto pUnitType = pUnit->GetTechnoType();

													if (pUnitType
														&& pUnit->IsAlive
														&& pUnit->Health > 0
														&& !pUnit->InLimbo)
													{
														if (pEntry.Type->ID == pUnitType->ID)
														{
															missing--;
														}
													}
												}

												if (missing > 0)
												{
													append("(%d, %s) ", missing, pEntry.Type->ID);
												}
											}
										}
									}
									display();
								}
							}
						}
						if (ObjectInfoDisplay::CanDisplay("passenger", name) || allDisplay)
						{
							if (pFoot->Passengers.NumPassengers > 0)
							{
								FootClass* pCurrent = pFoot->Passengers.FirstPassenger;
								append("%d Passengers: %s", pFoot->Passengers.NumPassengers, pCurrent->GetTechnoType()->ID);
								for (pCurrent = abstract_cast<FootClass*>(pCurrent->NextObject); pCurrent; pCurrent = abstract_cast<FootClass*>(pCurrent->NextObject))
									append(", %s", pCurrent->GetTechnoType()->ID);
								display();
							}
						}
						if (ObjectInfoDisplay::CanDisplay("destination", name) || allDisplay)
						{
							auto pDestination = abstract_cast<TechnoClass*>(pFoot->Destination);

							if (pDestination)
							{
								append("Destination = %s, Distance = %d, Location = (%d, %d)", pDestination->GetTechnoType()->ID, (pDestination->DistanceFrom(pFoot) / 256), pDestination->GetMapCoords().X, pDestination->GetMapCoords().Y);
								display();
							}
							else
							{
								if (pFoot->Destination)
								{
									auto destCell = CellClass::Coord2Cell(pFoot->Destination->GetCoords());
									append("Destination = (%d, %d)", destCell.X, destCell.Y);
									display();
								}
							}
						}
						if (ObjectInfoDisplay::CanDisplay("focus", name) || allDisplay)
						{
							auto pFocus = abstract_cast<TechnoClass*>(pFoot->ArchiveTarget);

							if (pFocus)
							{
								append("Focus = %s, Distance = %d, Location = (%d, %d)", pFocus->GetTechnoType()->ID, (pFocus->DistanceFrom(pFoot) / 256), pFocus->GetMapCoords().X, pFocus->GetMapCoords().Y);
								display();
							}
						}
						if (ObjectInfoDisplay::CanDisplay("megamission", name) || allDisplay)
						{
							if ((int)pFoot->MegaMission > -1)
							{
								append("Mega Mission = %d (%s)", (int)pFoot->MegaMission, GetMissionName((int)pFoot->MegaMission));
								display();
							}
						}
						if (ObjectInfoDisplay::CanDisplay("megatarget", name) || allDisplay)
						{
							auto megaTarget = (AbstractClass*)pFoot->MegaTarget;
							if (megaTarget)
							{
								auto mapCoords = CellStruct::Empty;
								auto ID = "N/A";

								if (auto const pObject = abstract_cast<ObjectClass*>(megaTarget))
								{
									mapCoords = pObject->GetMapCoords();
									ID = pObject->GetType()->get_ID();
								}
								else if (auto const pCell = abstract_cast<CellClass*>(megaTarget))
								{
									mapCoords = pCell->MapCoords;
									ID = "Cell";
								}

								append("Mega Target = %s, Distance = %d, Location = (%d, %d)", ID, (pFoot->DistanceFrom(megaTarget) / 256), mapCoords.X, mapCoords.Y);
								display();
							}
						}
						if (ObjectInfoDisplay::CanDisplay("megadestination", name) || allDisplay)
						{
							auto megaDestination = (AbstractClass*)pFoot->MegaDestination;
							auto pDestination = abstract_cast<TechnoClass*>(megaDestination);
							if (pDestination)
							{
								append("Destination = %s, Distance = %d, Location = (%d, %d)", pDestination->GetTechnoType()->ID, (pDestination->DistanceFrom(pFoot) / 256), pDestination->GetMapCoords().X, pDestination->GetMapCoords().Y);
								display();
							}
							else
							{
								if (megaDestination)
								{
									auto destCell = CellClass::Coord2Cell(megaDestination->GetCoords());
									append("Mega Destination = (%d, %d)", destCell.X, destCell.Y);
									display();
								}
							}
						}
						if (ObjectInfoDisplay::CanDisplay("recruit", name) || allDisplay)
						{
							append("RecruitA = %d, RecruitB = %d", (int)pFoot->RecruitableA, (int)pFoot->RecruitableB);
							display();
						}
						if (ObjectInfoDisplay::CanDisplay("tag", name) || allDisplay)
						{
							if (pFoot->AttachedTag)
							{
								if (pFoot->AttachedTag->FirstTrigger)
								{
									auto pTrigger = pFoot->AttachedTag->FirstTrigger;

									auto displayTrigger = [append, display](auto self, TriggerClass* trigger, int counter) -> void {
										if (trigger) {
											append("Trigger %d = %s", counter, trigger->Type->get_ID());
											display();
											self(self, trigger->NextTrigger, counter + 1);
										}
										};
									displayTrigger(displayTrigger, pTrigger, 0);
								}
							}
						}
					};

				auto buildingFields = [&append, &display](TechnoClass* pTechno, const std::string& name, bool allDisplay)
					{
						auto pBuilding = static_cast<BuildingClass*>(pTechno);
						auto pType = pBuilding->GetTechnoType();

						if (ObjectInfoDisplay::CanDisplay("power", name) || allDisplay)
						{
							if (auto const pBldType = abstract_cast<BuildingTypeClass*>(pType))
							{
								int power = pBldType->PowerBonus - pBldType->PowerDrain;
								if (power > 0)
								{
									power *= ((double)pBuilding->Health / (double)pBuilding->Type->Strength);
								}
								append("Power: %d", power);
								append(", Total Power Output: %d, Total Power Drain: %d", pBuilding->Owner->PowerOutput, pBuilding->Owner->PowerDrain);
								display();
							}
						}
						if (ObjectInfoDisplay::CanDisplay("factory", name) || allDisplay)
						{
							if (pBuilding->Factory && pBuilding->Factory->Object)
							{
								auto pFactoryType = pBuilding->Factory->Object->GetTechnoType();
								if (pFactoryType)
								{
									append("Production: %s (%d%%)", pFactoryType->ID, (pBuilding->Factory->GetProgress() * 100 / 54));
									display();
								}
							}
						}
						if (ObjectInfoDisplay::CanDisplay("money", name) || allDisplay)
						{
							append("Money: %d", pBuilding->Owner->Available_Money());
							display();
						}
						if (ObjectInfoDisplay::CanDisplay("occupants", name) || allDisplay)
						{
							if (pBuilding->Occupants.Count > 0)
							{
								append("%d Occupants: %s", pBuilding->Occupants.Count, pBuilding->Occupants.GetItem(0)->Type->ID);
								for (int i = 1; i < pBuilding->Occupants.Count; i++)
								{
									append(", %s", pBuilding->Occupants.GetItem(i)->Type->ID);
								}
								display();
							}
						}
						if (ObjectInfoDisplay::CanDisplay("enemy", name) || allDisplay)
						{
							HouseClass* pEnemyHouse = nullptr;

							if (auto pHouse = pBuilding->Owner)
							{
								int angerLevel = -1;
								if (pHouse->EnemyHouseIndex >= 0)
								{
									pEnemyHouse = HouseClass::Array.GetItem(pHouse->EnemyHouseIndex);
									for (auto pNode : pHouse->AngerNodes)
									{
										if (pNode.House == pEnemyHouse)
										{
											angerLevel = pNode.AngerLevel;
										}
									}
								}
								else
								{
									for (auto pNode : pHouse->AngerNodes)
									{
										if (!pNode.House->Type->MultiplayPassive
											&& !pNode.House->Defeated
											&& !pNode.House->IsObserver()
											&& !pNode.House->IsAlliedWith(pHouse)
											&& ((pNode.AngerLevel > angerLevel
												)
												|| angerLevel < 0))
										{
											angerLevel = pNode.AngerLevel;
											pEnemyHouse = pNode.House;
										}
									}
								}

								if (pEnemyHouse)
								{
									append("Enemy = %s (%s),  AngerLevel = %d", pEnemyHouse->get_ID(), pEnemyHouse->PlainName, angerLevel);
									display();
								}
							}
						}
						if (ObjectInfoDisplay::CanDisplay("upgrades", name) || allDisplay)
						{
							if (pBuilding->Type->Upgrades)
							{
								append("Upgrades (%d / %d): ", pBuilding->UpgradeLevel, pBuilding->Type->Upgrades);
								for (int i = 0; i < 3; i++)
								{
									if (i != 0)
										append(", ");

									append("Slot %d = %s", i + 1, pBuilding->Upgrades[i] ? pBuilding->Upgrades[i]->get_ID() : "<none>");
								}
								display();
							}
						}
					};


				switch (pTechno->WhatAmI())
				{
				case AbstractType::Infantry:
				case AbstractType::Unit:
				case AbstractType::Aircraft:
					PrintObjectInfo(pTechno, append, display, displayToolTip, footFields);
					break;
				case AbstractType::Building:
					PrintObjectInfo(pTechno, append, display, displayToolTip, buildingFields);
					break;
				default:
					append("INVALID ITEM!");
					display();
					break;
				}
			}
		}
	}

	if (bTriggerDebug)
	{
		DrawTriggerDebug();
	}

	if (bAITriggerDebug)
	{
		DrawAITriggerDebug();
	}

	return 0;
}
