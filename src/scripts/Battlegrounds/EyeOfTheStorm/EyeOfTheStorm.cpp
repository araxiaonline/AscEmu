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
#include "EyeOfTheStorm.h"

static float EOTSBuffCoordinates[4][4] =
{
    { 2050.54f, 1372.66f, 1194.56f, 1.67f },  // BE Tower
    { 2047.72f, 1749.73f, 1190.19f, 4.76f },  // Fel Reaver Ruins
    { 2283.39f, 1748.89f, 1189.77f, 4.88f },  // Mage Tower
    { 2302.68f, 1391.27f, 1197.79f, 2.05f },  // Draenei Ruins
};

static float EOTSBuffRotations[4][2] =
{
    { 0.681998f, -0.731354f },
    { 0.771625f, 0.636078f },
    { 0.422618f, -0.906308f },
    { 0.743145f, 0.669131f },
};

uint32_t EOTSbuffentrys[3] = { 184971, 184978, 184973 };

const float EOTSGraveyardLocations[EOTS_TOWER_COUNT][3] =
{
    { 2012.403442f, 1455.412354f, 1172.201782f },      // BE Tower
    { 2013.061890f, 1677.238037f, 1182.125732f },      // Fel Reaver Ruins
    { 2355.297852f, 1683.713989f, 1173.153687f },      // Mage Tower
    { 2351.785400f, 1455.399048f, 1185.333374f },      // Draenei Ruins
};

const float EOTSCPLocations[EOTS_TOWER_COUNT][4] =
{
    { 2047.19f, 1349.19f, 1189.11f, 1.5f },            // BE Tower 1
    { 2057.46f, 1735.07f, 1188.51f, 1.5f },            // Fel Reaver Ruins 1
    { 2270.84f, 1784.08f, 1186.76f, 1.5f },            // Mage Tower 1
    { 2276.81f, 1400.41f, 1196.74f, 2.8f },            // Draenei Ruins 1
};

const float EOTSCPLocations2[EOTS_TOWER_COUNT][4] =
{
    { 2074.07f, 1383.77f, 1195.16f, 0.4f },            // BE Tower 2
    { 2032.25f, 1729.53f, 1191.48f, 1.5f },            // Fel Reaver Ruins 2
    { 2269.13f, 1737.71f, 1186.66f, 4.2f },            // Mage Tower 2
    { 2305.78f, 1404.56f, 1199.73f, 1.5f },            // Draenei Ruins 2
};

const float EOTSCPLocations3[EOTS_TOWER_COUNT][4] =
{
    { 2025.13f, 1386.12f, 1192.81f, 2.2f },            // BE Tower 3
    { 2092.35f, 1775.46f, 1187.71f, 5.8f },            // Fel Reaver Ruins 3
    { 2300.86f, 1741.25f, 1187.96f, 5.3f },            // Mage Tower 3
    { 2245.42f, 1366.41f, 1195.49f, 2.8f },            // Draenei Ruins 3
};

const float EOTSTCLocations[EOTS_TOWER_COUNT][3] =
{
    { 2050.49f, 1372.23f, 1194.56f },                  // BE Tower
    { 2024.24f, 1742.48f, 1195.15f },                  // Fel Reaver Ruins
    { 2282.12f, 1755.00f, 1189.70f },                  // Mage Tower
    { 2301.01f, 1386.93f, 1197.18f },                  // Draenei Ruins
};

const float EOTSFlagLocation[3] = { 2174.718750f, 1568.766113f, 1159.958740f };
const float EOTSStartLocations[2][4] =
{
    { 2523.686035f, 1596.597290f, 1269.347656f, 3.191859f },
    { 1807.735962f, 1539.415649f, 1267.627319f, 0.0f },
};

const float EOTSBubbleLocations[2][5] =
{
    { 184719, 1803.21f, 1539.49f, 1261.09f, M_PI_FLOAT },
    { 184720, 2527.6f, 1596.91f, 1262.13f, -3.12414f },
};

const float EOTSBubbleRotations[2][4] =
{
    { -0.173642f, -0.001515f, 0.984770f, -0.008594f },
    { -0.173642f, -0.001515f, 0.984770f, -0.008594f },
};

static const char* EOTSControlPointNames[EOTS_TOWER_COUNT] =
{
    "Blood Elf Tower",
    "Fel Reaver Ruins",
    "Mage Tower",
    "Draenei Ruins"
};

static uint32_t resourcesToGainBH = 330;

const uint32_t EOTSTowerIds[EOTS_TOWER_COUNT] = { EOTS_GO_BE_TOWER, EOTS_GO_FELREAVER, EOTS_GO_MAGE_TOWER, EOTS_GO_DRAENEI_TOWER };

const uint32_t m_iconsStates[EOTS_TOWER_COUNT][3] =
{
    {2722, 2723, 2724},
    {2725, 2726, 2727},
    {2728, 2730, 2729},
    {2731, 2732, 2733}
};

EyeOfTheStorm::EyeOfTheStorm(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        EOTSm_buffs[i] = nullptr;
        m_CPStatus[i] = 50;
        m_CPBanner[i] = nullptr;
        m_CPBanner2[i] = nullptr;
        m_CPBanner3[i] = nullptr;
        m_CPStatusGO[i] = nullptr;

        m_spiritGuides[i] = nullptr;
    }

    m_flagHolder = 0;
    m_points[0] = m_points[1] = 0;
    m_lastHonorGainPoints[0] = m_lastHonorGainPoints[1] = 0;
    m_standFlag = nullptr;
    m_dropFlag = nullptr;

    m_zoneId = 3820;
    for (uint8_t i = 0; i < 2; ++i)
        m_bubbles[i] = nullptr;
}

EyeOfTheStorm::~EyeOfTheStorm()
{
    if (m_standFlag != nullptr)
    {
        if (!m_standFlag->IsInWorld())
            delete m_standFlag;
    }

    if (m_dropFlag != nullptr)
    {
        if (!m_dropFlag->IsInWorld())
            delete m_dropFlag;
    }

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        if (m_CPStatusGO[i] != nullptr)
        {
            if (!m_CPStatusGO[i]->IsInWorld())
                delete m_CPStatusGO[i];
        }
        if (m_CPBanner[i] != nullptr)
        {
            if (!m_CPBanner[i]->IsInWorld())
                delete m_CPBanner[i];
        }
        if (m_CPBanner2[i] != nullptr)
        {
            if (!m_CPBanner2[i]->IsInWorld())
                delete m_CPBanner2[i];
        }
        if (m_CPBanner3[i] != nullptr)
        {
            if (!m_CPBanner3[i]->IsInWorld())
                delete m_CPBanner3[i];
        }
        if (EOTSm_buffs[i] != nullptr)
        {
            if (!EOTSm_buffs[i]->IsInWorld())
                delete EOTSm_buffs[i];
        }

        if (m_spiritGuides[i])
        {
            if (!m_spiritGuides[i]->IsInWorld())
                delete m_spiritGuides[i];
        }
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (m_bubbles[i] != nullptr)
        {
            if (!m_bubbles[i]->IsInWorld())
                delete m_bubbles[i];
        }
    }

    m_resurrectMap.clear();

}

/*! Handles end of battleground rewards (marks etc)
*  \param winningTeam Team that won the battleground
*  \returns True if Battleground class should finish applying rewards, false if we handled it fully */
bool EyeOfTheStorm::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    castSpellOnTeam(winningTeam, 43477);
    castSpellOnTeam(winningTeam, 69156);
    return true;
}

void EyeOfTheStorm::RepopPlayersOfTeam(int32_t team, Creature* sh)
{
    std::map<Creature*, std::set<uint32_t> >::iterator itr = m_resurrectMap.find(sh);
    if (itr != m_resurrectMap.end())
    {
        for (std::set<uint32_t>::iterator it2 = itr->second.begin(); it2 != itr->second.end(); ++it2)
        {
            Player* r_plr = m_mapMgr->getPlayer(*it2);
            if (r_plr != nullptr && (team < 0 || (int32_t)r_plr->getTeam() == team) && r_plr->isDead())
                HookHandleRepop(r_plr);
        }
    }
}

bool EyeOfTheStorm::HookHandleRepop(Player* plr)
{
    uint32_t t = plr->getTeam();
    float dist = 999999.0f;
    float distcur;
    LocationVector dest;

    dest.ChangeCoords({ EOTSStartLocations[t][0], EOTSStartLocations[t][1], EOTSStartLocations[t][2] });

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        if (m_CPBanner[i] &&
            (((m_CPBanner[i]->getEntry() == EOTS_BANNER_ALLIANCE) && (t == TEAM_ALLIANCE)) ||
                ((m_CPBanner[i]->getEntry() == EOTS_BANNER_HORDE) && (t == TEAM_HORDE))))
        {
            distcur = plr->GetPositionNC().Distance2DSq({ EOTSGraveyardLocations[i][0], EOTSGraveyardLocations[i][1] });
            if (distcur < dist)
            {
                dist = distcur;
                dest.ChangeCoords({ EOTSGraveyardLocations[i][0], EOTSGraveyardLocations[i][1], EOTSGraveyardLocations[i][2] });
            }
        }
    }

    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void EyeOfTheStorm::HookOnAreaTrigger(Player* plr, uint32_t id)
{
    int32_t tid = -1;
    int32_t bonusid = -1;
    switch (id)
    {
        case 4476:            // BE Tower
            tid = EOTS_TOWER_BE;
            break;
        case 4568:            // BE Tower bonus
            bonusid = EOTS_TOWER_BE;
            break;
        case 4514:            // Fel Reaver Tower
            tid = EOTS_TOWER_FELREAVER;
            break;
        case 4569:            // Fel Reaver Tower bonus
            bonusid = EOTS_TOWER_FELREAVER;
            break;
        case 4518:            // Draenei Tower
            tid = EOTS_TOWER_DRAENEI;
            break;
        case 4571:            // Draenei Tower bonus
            bonusid = EOTS_TOWER_DRAENEI;
            break;
        case 4516:            // Mage Tower
            tid = EOTS_TOWER_MAGE;
            break;
        case 4570:            // Mage Tower bonus
            bonusid = EOTS_TOWER_MAGE;
            break;
        case 4512:
        case 4515:
        case 4517:
        case 4519:
        case 4530:
        case 4531:
        case 5866:
            break;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id %u", id);
            return;
            break;
    }

    if (plr->isDead())        // on ne buff pas les joueurs morts ;)
        return;

    if (bonusid > -1)
    {
        uint32_t spellid = 0;
        uint32_t x = (uint32_t)bonusid;
        if (EOTSm_buffs[x] && EOTSm_buffs[x]->IsInWorld())
        {
            spellid = EOTSm_buffs[x]->GetGameObjectProperties()->raw.parameter_3;
            SpellInfo const* sp = sSpellMgr.getSpellInfo(spellid);
            if (sp)
            {
                Spell* pSpell = sSpellMgr.newSpell(plr, sp, true, nullptr);
                SpellCastTargets targets(plr->getGuid());
                pSpell->prepare(&targets);
            }
            EOTSm_buffs[x]->despawn(0, EOTS_BUFF_RESPAWN_TIME);
        }
    }

    if (tid < 0)
        return;

    if (!m_hasStarted)
    {
        Anticheat_Log->writefromsession(plr->getSession(), "%s tried to hook the flag in eye of the storm before battleground (ID %u) started.", plr->getName().c_str(), this->m_id);
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, plr->getGuid(), "%s will be removed from the game for cheating.", plr->getName().c_str());
        // Remove player from battleground.
        this->removePlayer(plr, false);
        // Kick    player from server.
        plr->kickFromServer(TimeVarsMs::Second * 6);
        return;
    }

    const auto team = plr->getTeam();
    if (plr->getGuidLow() != m_flagHolder)
        return;

    int32_t val;

    uint32_t towers = 0;
    if (team == TEAM_ALLIANCE)
        val = EOTS_BANNER_ALLIANCE;
    else
        val = EOTS_BANNER_HORDE;

    if (!m_CPBanner[tid] || m_CPBanner[tid]->getEntry() != static_cast<uint32_t>(val))
        return;            // not captured by our team

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        if (m_CPBanner[i] && m_CPBanner[i]->getEntry() == static_cast<uint32_t>(val))
            towers++;
    }

    /*
    Points from flag captures
    * 1 towers controlled = 75 points
    * 2 towers controlled = 85 points
    * 3 towers controlled = 100 points
    * 4 towers controlled = 500 points
    */

    // 25 is guessed
    const static uint32_t points[5] = { 25, 75, 85, 100, 500 };
    const char* msgs[2] = { "The Alliance have captured the flag.", "The Horde have captured the flag." };

    sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE + team, 0, msgs[team]);
    GivePoints(team, points[towers]);

    DropFlag2(plr, id);
    setWorldState(EOTS_NETHERWING_FLAG_READY, 1);

    plr->removeAllAurasById(EOTS_NETHERWING_FLAG_SPELL);
    plr->m_bgScore.MiscData[BattlegroundDef::EOTS_FLAGS_CAPTURED]++;
    updatePvPData();
}

void EyeOfTheStorm::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;

    if (m_flagHolder == plr->getGuidLow())
        HookOnFlagDrop(plr);

    updatePvPData();
}

void EyeOfTheStorm::HookFlagDrop(Player* plr, GameObject* /*obj*/)
{
    if (!m_dropFlag->IsInWorld())
        return;

    std::map<uint32_t, uint32_t>::iterator itr = plr->m_forcedReactions.find(1059);
    if (itr != plr->m_forcedReactions.end())
    {
        return;
    }

    m_dropFlag->RemoveFromWorld(false);
    plr->castSpell(plr->getGuid(), EOTS_NETHERWING_FLAG_SPELL, true);

    setWorldState(EOTS_NETHERWING_FLAG_READY, 0);
    playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);
    sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE + plr->getTeam(), plr->getGuid(), "%s has taken the flag!", plr->getName().c_str());
    m_flagHolder = plr->getGuidLow();

    event_RemoveEvents(EVENT_EOTS_RESET_FLAG);
}

void EyeOfTheStorm::HookFlagStand(Player* /*plr*/, GameObject* /*obj*/)
{
}

bool EyeOfTheStorm::HookSlowLockOpen(GameObject* /*pGo*/, Player* pPlayer, Spell* /*pSpell*/)
{
    if (m_flagHolder != 0)
        return false;

    m_standFlag->RemoveFromWorld(false);
    pPlayer->castSpell(pPlayer->getGuid(), EOTS_NETHERWING_FLAG_SPELL, true);

    setWorldState(EOTS_NETHERWING_FLAG_READY, 0);
    playSoundToAll(pPlayer->isTeamHorde() ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);
    sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE + pPlayer->getTeam(), pPlayer->getGuid(), "%s has taken the flag!", pPlayer->getName().c_str());
    m_flagHolder = pPlayer->getGuidLow();
    return true;
}

void EyeOfTheStorm::HookOnMount(Player* plr)
{
    if (m_flagHolder == plr->getGuidLow())
    {
        HookOnFlagDrop(plr);
    }
}

void EyeOfTheStorm::OnAddPlayer(Player* plr)
{
    if (!m_hasStarted && plr->IsInWorld())
    {
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
        plr->m_bgScore.MiscData[BattlegroundDef::EOTS_FLAGS_CAPTURED] = 0;
    }
    updatePvPData();
}

void EyeOfTheStorm::OnRemovePlayer(Player* plr)
{
    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        m_CPDisplay[i].erase(plr);
    }

    if (m_flagHolder == plr->getGuidLow())
    {
        HookOnFlagDrop(plr);
    }

    if (!m_hasStarted)
        plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

void EyeOfTheStorm::DropFlag2(Player* plr, uint32_t id)
{
    if (m_flagHolder != plr->getGuidLow())
        return;

    switch (id)
    {
        case 4476:            // Blood Elf Tower
            m_dropFlag->SetPosition(LocationVector(2048.49f, 1393.64f, 1194.36f, 0.1641f));
            break;
        case 4514:            // Fel Reaver Tower
            m_dropFlag->SetPosition(LocationVector(2044.05f, 1729.86f, 1189.85f, 0.1641f));
            break;
        case 4518:            // Draenei Tower
            m_dropFlag->SetPosition(LocationVector(2286.41f, 1402.44f, 1197.11f, 3.3483f));
            break;
        case 4516:            // Mage Tower
            m_dropFlag->SetPosition(LocationVector(2284.48f, 1731.16f, 1189.87f, 3.3483f));
            break;
        default:
            m_dropFlag->SetPosition(plr->GetPosition());
            break;
    }

    plr->castSpell(plr, EOTS_RECENTLY_DROPPED_FLAG, true);
    playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_SCORES : BattlegroundDef::ALLIANCE_SCORES);
    m_dropFlag->setFlags(GO_FLAG_NONSELECTABLE);
    m_dropFlag->PushToWorld(m_mapMgr);
    m_flagHolder = 0;
    sEventMgr.AddEvent(this, &EyeOfTheStorm::EventResetFlag, EVENT_EOTS_RESET_FLAG, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void EyeOfTheStorm::HookOnFlagDrop(Player* plr)
{
    if (m_flagHolder != plr->getGuidLow())
        return;

    plr->removeAllAurasById(EOTS_NETHERWING_FLAG_SPELL);
    plr->castSpell(plr, EOTS_RECENTLY_DROPPED_FLAG, true);

    m_dropFlag->SetPosition(plr->GetPosition());
    m_dropFlag->PushToWorld(m_mapMgr);
    m_flagHolder = 0;
    playSoundToAll(BattlegroundDef::FLAG_RETURNED);
    sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE + plr->getTeam(), plr->getGuid(), "%s has dropped the flag!", plr->getName().c_str());

    sEventMgr.AddEvent(this, &EyeOfTheStorm::EventResetFlag, EVENT_EOTS_RESET_FLAG, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void EyeOfTheStorm::EventResetFlag()
{
    if (!m_dropFlag->IsInWorld())
        return;

    m_dropFlag->RemoveFromWorld(false);
    m_dropFlag->setFlags(GO_FLAG_NONE);
    m_standFlag->PushToWorld(m_mapMgr);

    setWorldState(EOTS_NETHERWING_FLAG_READY, 1);
    playSoundToAll(BattlegroundDef::FLAG_RESPAWN);
    sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The flag has been reset.");
    m_flagHolder = 0;
}

void EyeOfTheStorm::OnCreate()
{
    GameObjectProperties const* gameobject_info;

    // create gameobjects
    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        gameobject_info = sMySQLStore.getGameObjectProperties(EOTSTowerIds[i]);
        if (gameobject_info == nullptr)
        {
            DLLLogDetail("EOTS is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        m_CPStatusGO[i] = m_mapMgr->createGameObject(gameobject_info->entry);
        m_CPStatusGO[i]->create(gameobject_info->entry, m_mapMgr, 0, LocationVector(EOTSTCLocations[i][0], EOTSTCLocations[i][1], EOTSTCLocations[i][2]), QuaternionData(), GO_STATE_CLOSED);
        m_CPStatusGO[i]->PushToWorld(m_mapMgr);

        gameobject_info = sMySQLStore.getGameObjectProperties(EOTS_BANNER_NEUTRAL);
        if (gameobject_info == nullptr)
        {
            DLLLogDetail("EOTS is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        m_CPBanner[i] = m_mapMgr->createGameObject(gameobject_info->entry);
        m_CPBanner[i]->create(gameobject_info->entry, m_mapMgr, 0, LocationVector(EOTSCPLocations[i][0], EOTSCPLocations[i][1], EOTSCPLocations[i][2], EOTSCPLocations[i][3]), QuaternionData(), GO_STATE_CLOSED);
        m_CPBanner[i]->setScale(1.7f);
        m_CPBanner[i]->PushToWorld(m_mapMgr);

        m_CPBanner2[i] = m_mapMgr->createGameObject(gameobject_info->entry);
        m_CPBanner2[i]->create(gameobject_info->entry, m_mapMgr, 0, LocationVector(EOTSCPLocations2[i][0], EOTSCPLocations2[i][1], EOTSCPLocations2[i][2], EOTSCPLocations2[i][3]), QuaternionData(), GO_STATE_CLOSED);
        m_CPBanner2[i]->setScale(1.7f);
        m_CPBanner2[i]->PushToWorld(m_mapMgr);

        m_CPBanner3[i] = m_mapMgr->createGameObject(gameobject_info->entry);
        m_CPBanner3[i]->create(gameobject_info->entry, m_mapMgr, 0, LocationVector(EOTSCPLocations3[i][0], EOTSCPLocations3[i][1], EOTSCPLocations3[i][2], EOTSCPLocations3[i][3]), QuaternionData(), GO_STATE_CLOSED);
        m_CPBanner3[i]->setScale(1.7f);
        m_CPBanner3[i]->PushToWorld(m_mapMgr);
    }

    // BUBBLES
    for (uint8_t i = 0; i < 2; ++i)
    {
        m_bubbles[i] = m_mapMgr->createGameObject((uint32_t)EOTSBubbleLocations[i][0]);
        if (!m_bubbles[i]->create((uint32_t)EOTSBubbleLocations[i][0], m_mapMgr, 0, LocationVector(EOTSBubbleLocations[i][1], EOTSBubbleLocations[i][2], EOTSBubbleLocations[i][3], EOTSBubbleLocations[i][4]), QuaternionData(), GO_STATE_CLOSED))
        {
            DLLLogDetail("EOTS is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        m_bubbles[i]->setScale(0.1f);
        m_bubbles[i]->setState(GO_STATE_CLOSED);
        m_bubbles[i]->setFlags(GO_FLAG_NEVER_DESPAWN);
        m_bubbles[i]->SetFaction(114);
        m_bubbles[i]->setAnimationProgress(100);

        m_bubbles[i]->PushToWorld(m_mapMgr);
    }

    SpawnBuff(EOTS_TOWER_DRAENEI);
    SpawnBuff(EOTS_TOWER_MAGE);
    SpawnBuff(EOTS_TOWER_FELREAVER);
    SpawnBuff(EOTS_TOWER_BE);

    // Flag
    m_standFlag = m_mapMgr->createGameObject(184141);
    m_standFlag->create(184141, m_mapMgr, 0, LocationVector(2174.284912f, 1569.466919f, 1159.960083f, 4.4892f), QuaternionData(), GO_STATE_CLOSED);
    m_standFlag->setScale(2.0f);
    m_standFlag->PushToWorld(m_mapMgr);

    m_dropFlag = m_mapMgr->createGameObject(184142);
    m_dropFlag->create(184142, m_mapMgr, 0, LocationVector(2174.284912f, 1569.466919f, 1159.960083f, 0.1641f), QuaternionData(), GO_STATE_CLOSED);
    m_dropFlag->setScale(2.0f);
}

void EyeOfTheStorm::RespawnCPFlag(uint32_t i, uint32_t id)
{
    m_CPBanner[i]->RemoveFromWorld(false);
    m_CPBanner[i]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
    m_CPBanner[i]->create(id, m_mapMgr, 0, LocationVector(m_CPBanner[i]->GetPositionX(), m_CPBanner[i]->GetPositionY(), m_CPBanner[i]->GetPositionZ(), m_CPBanner[i]->GetOrientation()), QuaternionData(), GO_STATE_CLOSED);
    m_CPBanner[i]->PushToWorld(m_mapMgr);

    m_CPBanner2[i]->RemoveFromWorld(false);
    m_CPBanner2[i]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
    m_CPBanner2[i]->create(id, m_mapMgr, 0, LocationVector(m_CPBanner2[i]->GetPositionX(), m_CPBanner2[i]->GetPositionY(), m_CPBanner2[i]->GetPositionZ(), m_CPBanner2[i]->GetOrientation()), QuaternionData(), GO_STATE_CLOSED);
    m_CPBanner2[i]->PushToWorld(m_mapMgr);

    m_CPBanner3[i]->RemoveFromWorld(false);
    m_CPBanner3[i]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
    m_CPBanner3[i]->create(id, m_mapMgr, 0, LocationVector(m_CPBanner3[i]->GetPositionX(), m_CPBanner3[i]->GetPositionY(), m_CPBanner3[i]->GetPositionZ(), m_CPBanner3[i]->GetOrientation()), QuaternionData(), GO_STATE_CLOSED);
    m_CPBanner3[i]->PushToWorld(m_mapMgr);
}

void EyeOfTheStorm::UpdateCPs()
{
    GameObject* go;
    int32_t delta = 0;
    uint32_t playercounts[2];
    uint32_t towers[2] = { 0, 0 };
    EOTSCaptureDisplayList::iterator eitr, eitr2, eitrend;
    EOTSCaptureDisplayList* disp;

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        // loop players in range, add any that aren't in the set to the set
        playercounts[0] = playercounts[1] = 0;
        go = m_CPStatusGO[i];
        disp = &m_CPDisplay[i];

        for (const auto& itr : go->getInRangePlayersSet())
        {
            Player* plr = static_cast<Player*>(itr);
            if (plr && plr->isAlive() && !(plr->isStealthed()) && !plr->isInvisible() && !(plr->m_schoolImmunityList[0]) && plr->GetDistance2dSq(go) <= EOTS_CAPTURE_DISTANCE)
            {
                playercounts[plr->getTeam()]++;

                if (disp->find(plr) == disp->end())
                {
                    disp->insert(plr);
                    plr->sendWorldStateUpdate(WORLDSTATE_EOTS_DISPLAYON, 1);
                }
            }
        }

        // score diff calculation
        //printf("EOTS: Playercounts = %u %u\n", playercounts[0], playercounts[1]);
        if (playercounts[0] != playercounts[1])
        {
            if (playercounts[0] > playercounts[1])
                delta = playercounts[0];
            else if (playercounts[1] > playercounts[0])
                delta = -(int32_t)playercounts[1];

            delta *= EOTS_CAPTURE_RATE;
            m_CPStatus[i] += delta;
            if (m_CPStatus[i] > 100)
                m_CPStatus[i] = 100;
            else if (m_CPStatus[i] < 0)
                m_CPStatus[i] = 0;

            // change the flag depending on cp status
            if (m_CPStatus[i] <= 30)
            {
                if (m_CPBanner[i] && m_CPBanner[i]->getEntry() != EOTS_BANNER_HORDE)
                {
                    RespawnCPFlag(i, EOTS_BANNER_HORDE);
                    if (m_spiritGuides[i] != nullptr)
                    {
                        RepopPlayersOfTeam(0, m_spiritGuides[i]);
                        m_spiritGuides[i]->Despawn(0, 0);
                        removeSpiritGuide(m_spiritGuides[i]);
                        m_spiritGuides[i] = nullptr;
                    }
                    m_spiritGuides[i] = spawnSpiritGuide(EOTSGraveyardLocations[i][0], EOTSGraveyardLocations[i][1], EOTSGraveyardLocations[i][2], 0, 1);
                    addSpiritGuide(m_spiritGuides[i]);

                    setWorldState(m_iconsStates[i][0], 0);
                    setWorldState(m_iconsStates[i][1], 0);
                    setWorldState(m_iconsStates[i][2], 1);
                    sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, 0, "The Horde has taken the %s !", EOTSControlPointNames[i]);
                    playSoundToAll(BattlegroundDef::HORDE_CAPTURE);
                }
            }
            else if (m_CPStatus[i] >= 70)
            {
                if (m_CPBanner[i] && m_CPBanner[i]->getEntry() != EOTS_BANNER_ALLIANCE)
                {
                    RespawnCPFlag(i, EOTS_BANNER_ALLIANCE);
                    if (m_spiritGuides[i] != nullptr)
                    {
                        RepopPlayersOfTeam(1, m_spiritGuides[i]);
                        m_spiritGuides[i]->Despawn(0, 0);
                        removeSpiritGuide(m_spiritGuides[i]);
                        m_spiritGuides[i] = nullptr;
                    }

                    m_spiritGuides[i] = spawnSpiritGuide(EOTSGraveyardLocations[i][0], EOTSGraveyardLocations[i][1], EOTSGraveyardLocations[i][2], 0, 0);
                    addSpiritGuide(m_spiritGuides[i]);

                    setWorldState(m_iconsStates[i][0], 0);
                    setWorldState(m_iconsStates[i][1], 1);
                    setWorldState(m_iconsStates[i][2], 0);
                    sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, 0, "The Alliance has taken the %s", EOTSControlPointNames[i]);
                    playSoundToAll(BattlegroundDef::ALLIANCE_CAPTURE);
                }
            }
            else
            {
                if (m_CPBanner[i]->getEntry() != EOTS_BANNER_NEUTRAL)
                {
                    if (m_CPBanner[i]->getEntry() == EOTS_BANNER_ALLIANCE)
                    {
                        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The Alliance has lost the control of the %s.", EOTSControlPointNames[i]);
                    }
                    else if (m_CPBanner[i]->getEntry() == EOTS_BANNER_HORDE)
                    {
                        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The Horde has lost the control of the %s.", EOTSControlPointNames[i]);
                    }
                    RespawnCPFlag(i, EOTS_BANNER_NEUTRAL);
                    if (m_spiritGuides[i] != nullptr)
                    {
                        RepopPlayersOfTeam(-1, m_spiritGuides[i]);
                        m_spiritGuides[i]->Despawn(0, 0);
                        removeSpiritGuide(m_spiritGuides[i]);
                        m_spiritGuides[i] = nullptr;
                    }
                    setWorldState(m_iconsStates[i][0], 1);
                    setWorldState(m_iconsStates[i][1], 0);
                    setWorldState(m_iconsStates[i][2], 0);
                }
            }
        }

        // update the players with the new value
        eitr = disp->begin();
        eitrend = disp->end();

        for (; eitr != eitrend;)
        {
            Player* plr = *eitr;
            eitr2 = eitr;
            ++eitr;

            if (plr->GetDistance2dSq(go) > EOTS_CAPTURE_DISTANCE)
            {
                disp->erase(eitr2);
                plr->sendWorldStateUpdate(WORLDSTATE_EOTS_DISPLAYON, 0);            // hide the cp bar
            }
            else
                plr->sendWorldStateUpdate(WORLDSTATE_EOTS_DISPLAYVALUE, m_CPStatus[i]);
        }
    }

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        if (m_CPBanner[i] && m_CPBanner[i]->getEntry() == EOTS_BANNER_ALLIANCE)
            towers[0]++;
        else if (m_CPBanner[i] && m_CPBanner[i]->getEntry() == EOTS_BANNER_HORDE)
            towers[1]++;
    }

    setWorldState(WORLDSTATE_EOTS_ALLIANCE_BASES, towers[0]);
    setWorldState(WORLDSTATE_EOTS_HORDE_BASES, towers[1]);
}

void EyeOfTheStorm::GeneratePoints()
{
    uint32_t towers[2] = { 0, 0 };

    /*
    #  Unlike Arathi Basin, points are always generated in 2 seconds intervals no matter how many towers are controlled by both teams.
    # Each claimed tower generates victory points for the controlling team. The more towers your team owns, the faster your team gains points

    * 1 tower controlled = 1 point/tick (0.5 points per second)
    * 2 towers controlled = 2 points/tick (1 point per second)
    * 3 towers controlled = 5 points/tick (2.5 points per second)
    * 4 towers controlled = 10 points/tick (5 points per second)

    */
    uint32_t pointspertick[5] = { 0, 1, 2, 5, 10 };

    for (uint8_t i = 0; i < EOTS_TOWER_COUNT; ++i)
    {
        if (m_CPBanner[i] && m_CPBanner[i]->getEntry() == EOTS_BANNER_ALLIANCE)
            towers[0]++;
        else if (m_CPBanner[i] && m_CPBanner[i]->getEntry() == EOTS_BANNER_HORDE)
            towers[1]++;
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (towers[i] == 0)
        {
            //printf("EOTS: No points on team %u\n", i);
            continue;
        }

        if (GivePoints(i, pointspertick[towers[i]]))
            return;
    }
}

bool EyeOfTheStorm::GivePoints(uint32_t team, uint32_t points)
{
    //printf("EOTS: Give team %u %u points.\n", team, points);

    m_points[team] += points;
    if ((m_points[team] - m_lastHonorGainPoints[team]) >= resourcesToGainBH)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        uint32_t honorToAdd = m_honorPerKill;
        for (std::set<Player*>::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
        {
            (*itr)->m_bgScore.BonusHonor += honorToAdd;
            HonorHandler::AddHonorPointsToPlayer((*itr), honorToAdd);
        }

        updatePvPData();
        m_lastHonorGainPoints[team] += resourcesToGainBH;
    }

    if (m_points[team] >= 1600)
    {
        m_points[team] = 1600;

        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        setWorldState(WORLDSTATE_EOTS_ALLIANCE_VICTORYPOINTS + team, m_points[team]);

        this->endBattleground(team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
        return true;
    }

    setWorldState(WORLDSTATE_EOTS_ALLIANCE_VICTORYPOINTS + team, m_points[team]);
    return false;
}

void EyeOfTheStorm::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    updatePvPData();
}

void EyeOfTheStorm::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void EyeOfTheStorm::SpawnBuff(uint32_t x)
{
    uint32_t chosen_buffid = EOTSbuffentrys[Util::getRandomUInt(2)];
    GameObjectProperties const* goi = sMySQLStore.getGameObjectProperties(chosen_buffid);
    if (goi == nullptr)
        return;

    if (EOTSm_buffs[x] == nullptr)
    {
        EOTSm_buffs[x] = spawnGameObject(chosen_buffid, LocationVector(EOTSBuffCoordinates[x][0], EOTSBuffCoordinates[x][1], EOTSBuffCoordinates[x][2], EOTSBuffCoordinates[x][3]), 0, 114, 1);

        EOTSm_buffs[x]->setLocalRotation(0.f, 0.f, EOTSBuffRotations[x][0], EOTSBuffRotations[x][1]);
        EOTSm_buffs[x]->setState(GO_STATE_CLOSED);
        EOTSm_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
        EOTSm_buffs[x]->setAnimationProgress(100);
        EOTSm_buffs[x]->PushToWorld(m_mapMgr);
    }
    else
    {
        if (EOTSm_buffs[x]->IsInWorld())
            EOTSm_buffs[x]->RemoveFromWorld(false);

        if (chosen_buffid != EOTSm_buffs[x]->getEntry())
        {
            EOTSm_buffs[x]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
            EOTSm_buffs[x]->setEntry(chosen_buffid);
            EOTSm_buffs[x]->SetGameObjectProperties(goi);
        }

        EOTSm_buffs[x]->PushToWorld(m_mapMgr);
    }
}

LocationVector EyeOfTheStorm::GetStartingCoords(uint32_t Team)
{
    return LocationVector(EOTSStartLocations[Team][0],
                          EOTSStartLocations[Team][1],
                          EOTSStartLocations[Team][2],
                          EOTSStartLocations[Team][3]);
}

void EyeOfTheStorm::OnStart()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            (*itr)->removeAllAurasById(BattlegroundDef::PREPARATION);
        }
    }

    // start the events
    sEventMgr.AddEvent(this, &EyeOfTheStorm::GeneratePoints, EVENT_EOTS_GIVE_POINTS, 2000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    sEventMgr.AddEvent(this, &EyeOfTheStorm::UpdateCPs, EVENT_EOTS_CHECK_CAPTURE_POINT_STATUS, 5000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    // spirit guides
    addSpiritGuide(spawnSpiritGuide(EOTSStartLocations[0][0], EOTSStartLocations[0][1], EOTSStartLocations[0][2], 0, 0));
    addSpiritGuide(spawnSpiritGuide(EOTSStartLocations[1][0], EOTSStartLocations[1][1], EOTSStartLocations[1][2], 0, 1));

    // remove the bubbles
    for (uint8_t i = 0; i < 2; ++i)
    {
        m_bubbles[i]->RemoveFromWorld(false);
        delete m_bubbles[i];
        m_bubbles[i] = nullptr;
    }

    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);
    m_hasStarted = true;
}

void EyeOfTheStorm::HookOnShadowSight()
{}

void EyeOfTheStorm::HookGenerateLoot(Player* /*plr*/, Object* /*pOCorpse*/)
{}

void EyeOfTheStorm::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void EyeOfTheStorm::SetIsWeekend(bool isweekend)
{
    if (isweekend)
    {
        resourcesToGainBH = 200;
    }
    else
    {
        resourcesToGainBH = 330;
    }
}

