/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_VaultOfArchavon.h"

class VaultOfArchavonInstanceScript : public InstanceScript
{
public:
    explicit VaultOfArchavonInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new VaultOfArchavonInstanceScript(pMapMgr); }
};

void SetupVaultOfArchavon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_VAULT_OF_ARCHAVON, &VaultOfArchavonInstanceScript::Create);
}
