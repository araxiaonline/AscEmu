/*
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "RuinsOfLordaeron.h"

RuinsOfLordaeron::RuinsOfLordaeron(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

RuinsOfLordaeron::~RuinsOfLordaeron()
{}

void RuinsOfLordaeron::OnCreate()
{
    GameObject* obj = nullptr;

    obj = spawnGameObject(185917, LocationVector(1278.647705f, 1730.556641f, 31.605574f, 1.684245f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.746058f, 0.665881f);
    m_gates.insert(obj);

    obj = spawnGameObject(185918, LocationVector(1293.560791f, 1601.937988f, 31.605574f, -1.457349f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, -0.665881f, 0.746058f);
    m_gates.insert(obj);

    Arena::OnCreate();
}

void RuinsOfLordaeron::HookOnShadowSight()
{
    m_buffs[0] = spawnGameObject(184664, LocationVector(1328.729268f, 1632.738403f, 34.838585f, 2.611449f), 32, 1375, 1.0f);
    m_buffs[0]->setState(GO_STATE_CLOSED);
    m_buffs[0]->setLocalRotation(0.f, 0.f, 0.904455f, -0.426569f);
    m_buffs[0]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[0]->setAnimationProgress(100);
    m_buffs[0]->PushToWorld(m_mapMgr);

    m_buffs[1] = spawnGameObject(184664, LocationVector(1243.306763f, 1699.334351f, 34.837566f, 5.713773f), 32, 1375, 1.0f);
    m_buffs[1]->setState(GO_STATE_CLOSED);
    m_buffs[1]->setLocalRotation(0.f, 0.f, 0.90445f, -0.426569f);
    m_buffs[1]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[1]->setAnimationProgress(100);
    m_buffs[1]->PushToWorld(m_mapMgr);

}

LocationVector RuinsOfLordaeron::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(1277.105103f, 1743.956177f, 31.603209f);
    else
        return LocationVector(1295.322388f, 1585.953369f, 31.605387f);

}

void RuinsOfLordaeron::HookOnAreaTrigger(Player* /*plr*/, uint32_t trigger)
{
    switch (trigger)
    {
        case 4696:
        case 4697:
            break;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id %u", trigger);
            break;
    }
}

bool RuinsOfLordaeron::HookHandleRepop(Player* plr)
{
    LocationVector dest(0, 0, 0, 0);
    dest.ChangeCoords({ 1286.112061f, 1668.334961f, 39.289127f });
    plr->safeTeleport(m_mapMgr->getBaseMap()->getMapId(), m_mapMgr->getInstanceId(), dest);
    return true;
}
