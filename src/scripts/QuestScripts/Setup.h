/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/ScriptMgr.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Item.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Management/ItemInterface.h"
#include "Movement/Spline/MoveSpline.h"
#include "Server/Script/CreatureAIScript.h"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellMgr.hpp"
#include "Movement/Spline/MoveSplineInit.h"
#include "Management/TaxiMgr.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Classes Quests
void SetupDruid(ScriptMgr* mgr);
void SetupMage(ScriptMgr* mgr);
void SetupPaladin(ScriptMgr* mgr);
void SetupWarrior(ScriptMgr* mgr);
void SetupDeathKnight(ScriptMgr* mgr);

//////////////////////////////////////////////////////////////////////////////////////////
// Proffessions Quests
void SetupFirstAid(ScriptMgr* mgr);

//////////////////////////////////////////////////////////////////////////////////////////
// Zones Quests
void SetupArathiHighlands(ScriptMgr* mgr);
void SetupAzuremystIsle(ScriptMgr* mgr);
void SetupBladeEdgeMountains(ScriptMgr* mgr);
void SetupBlastedLands(ScriptMgr* mgr);
void SetupBloodmystIsle(ScriptMgr* mgr);
void SetupBurningSteppes(ScriptMgr* mgr);
void SetupDesolace(ScriptMgr* mgr);
void SetupDragonblight(ScriptMgr* mgr);
void SetupDuskwood(ScriptMgr* mgr);
void SetupDustwallowMarsh(ScriptMgr* mgr);
void SetupEasternPlaguelands(ScriptMgr* mgr);
void SetupEversongWoods(ScriptMgr* mgr);
void SetupGhostlands(ScriptMgr* mgr);
void SetupHellfirePeninsula(ScriptMgr* mgr);
void SetupHillsbradFoothills(ScriptMgr* mgr);
void SetupHowlingFjord(ScriptMgr* mgr);
void SetupIsleOfQuelDanas(ScriptMgr* mgr);
void SetupLochModan(ScriptMgr* mgr);
void SetupMulgore(ScriptMgr* mgr);
void SetupNagrand(ScriptMgr* mgr);
void SetupNetherstorm(ScriptMgr* mgr);
void SetupRedrigeMountains(ScriptMgr* mgr);
void SetupShadowmoon(ScriptMgr* mgr);
void SetupSilithus(ScriptMgr* mgr);
void SetupSilvermoonCity(ScriptMgr* mgr);
void SetupSilverpineForest(ScriptMgr* mgr);
void SetupStonetalonMountains(ScriptMgr* mgr);
void SetupStormwind(ScriptMgr* mgr);
void SetupStranglethornVale(ScriptMgr* mgr);
void SetupTanaris(ScriptMgr* mgr);
void SetupTeldrassil(ScriptMgr* mgr);
void SetupTerrokarForest(ScriptMgr* mgr);
void SetupTheStormPeaks(ScriptMgr* mgr);
void SetupThousandNeedles(ScriptMgr* mgr);
void SetupTirisfalGlades(ScriptMgr* mgr);
void SetupUndercity(ScriptMgr* mgr);
void SetupUnGoro(ScriptMgr* mgr);
void SetupWestfall(ScriptMgr* mgr);
void SetupWetlands(ScriptMgr* mgr);
void SetupZangarmarsh(ScriptMgr* mgr);
void SetupBarrens(ScriptMgr* mgr);
void SetupBoreanTundra(ScriptMgr* mgr);
void SetupSholazarBasin(ScriptMgr* mgr);

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void SetupQuestGossip(ScriptMgr* mgr);
void SetupQuestHooks(ScriptMgr* mgr);
void SetupUnsorted(ScriptMgr* mgr);
