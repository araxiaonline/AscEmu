/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Grim_Batol.h"

class GrimBatolInstanceScript : public InstanceScript
{
public:
    explicit GrimBatolInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new GrimBatolInstanceScript(pMapMgr); }
};

void SetupGrimBatol(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GRIM_BATOL, &GrimBatolInstanceScript::Create);
}
