/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/ScriptMgr.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Server/MainServerDefines.h"
#include "Server/Master.h"
#include "Server/Script/CreatureAIScript.h"

void SetupGoHandlers(ScriptMgr* mgr);
void SetupQDGoHandlers(ScriptMgr* mgr);
void SetupRandomScripts(ScriptMgr* mgr);
void SetupMiscCreatures(ScriptMgr* mgr);
void InitializeGameObjectTeleportTable(ScriptMgr* mgr);
void SetupCityDalaran(ScriptMgr* mgr);
