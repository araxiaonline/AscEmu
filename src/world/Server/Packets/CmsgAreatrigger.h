/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgAreatrigger : public ManagedPacket
    {
    public:
        uint32_t triggerId;

        CmsgAreatrigger() : CmsgAreatrigger(0)
        {
        }

        CmsgAreatrigger(uint32_t triggerId) :
            ManagedPacket(CMSG_AREATRIGGER, 4),
            triggerId(triggerId)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> triggerId;
            return true;
        }
    };
}
