/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/CmsgAttackSwing.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Management/Faction.h"

using namespace AscEmu::Packets;

void WorldSession::handleAttackSwingOpcode(WorldPacket& recvPacket)
{
    CmsgAttackSwing srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ATTACKSWING: %u (guidLow)", srlPacket.guid.getGuidLow());

    if (_player->isFeared() || _player->isStunned() || _player->isPacified() || _player->isDead())
        return;

    const auto unitTarget = _player->getWorldMap()->getUnit(srlPacket.guid.getRawGuid());
    if (unitTarget == nullptr)
        return;

    if (!isAttackable(_player, unitTarget, false) || unitTarget->isDead())
        return;

    _player->smsg_AttackStart(unitTarget);
    _player->eventAttackStart();
}

void WorldSession::handleAttackStopOpcode(WorldPacket& /*recvPacket*/)
{
    const auto unitTarget = _player->getWorldMap()->getUnit(_player->getTargetGuid());
    if (unitTarget == nullptr)
        return;

    _player->eventAttackStop();
    _player->smsg_AttackStop(unitTarget);
}
