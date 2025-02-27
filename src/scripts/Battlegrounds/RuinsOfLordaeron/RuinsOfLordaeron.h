/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class RuinsOfLordaeron : public Arena
{
public:
    RuinsOfLordaeron(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side);
    ~RuinsOfLordaeron();

    static Battleground* Create(BattlegroundMap* m, uint32_t i, uint32_t l, uint32_t t, uint32_t players_per_side)
    {
        return new RuinsOfLordaeron(m, i, l, t, players_per_side);
    }

    void OnCreate() override;
    void HookOnShadowSight() override;
    LocationVector GetStartingCoords(uint32_t Team) override;
    void HookOnAreaTrigger(Player* plr, uint32_t trigger) override;
    bool HookHandleRepop(Player* plr) override;
};
