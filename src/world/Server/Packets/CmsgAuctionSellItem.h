/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "Management/AuctionHouse.h"

namespace AscEmu::Packets
{
    class CmsgAuctionSellItem : public ManagedPacket
    {
    public:
        WoWGuid auctioneerGuid;
#if VERSION_STRING >= Cata
        uint64_t bidMoney;
        uint64_t buyoutPrice;
#else
        uint32_t bidMoney;
        uint32_t buyoutPrice;
#endif
        uint32_t itemsCount;
        uint32_t expireTime;

        uint64_t itemGuids[MAX_AUCTION_ITEMS];
        uint32_t count[MAX_AUCTION_ITEMS];

        CmsgAuctionSellItem() : CmsgAuctionSellItem(0, 0, 0, 0, 0)
        {
        }

#if VERSION_STRING >= Cata
        CmsgAuctionSellItem(uint64_t auctioneerGuid, uint64_t bidMoney, uint64_t buyoutPrice, uint32_t itemsCount, uint32_t expireTime) :
            ManagedPacket(CMSG_AUCTION_SELL_ITEM, 0),
            auctioneerGuid(auctioneerGuid),
            bidMoney(bidMoney),
            buyoutPrice(buyoutPrice),
            itemsCount(itemsCount),
            expireTime(expireTime)
#else
        CmsgAuctionSellItem(uint64_t auctioneerGuid, uint32_t bidMoney, uint32_t buyoutPrice, uint32_t itemsCount, uint32_t expireTime) :
            ManagedPacket(CMSG_AUCTION_SELL_ITEM, 0),
            auctioneerGuid(auctioneerGuid),
            bidMoney(bidMoney),
            buyoutPrice(buyoutPrice),
            itemsCount(itemsCount),
            expireTime(expireTime)
#endif
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> itemsCount;
            auctioneerGuid.Init(unpacked_guid);

            for (uint32_t i = 0; i < itemsCount; ++i)
            {
                packet >> itemGuids[i];
                packet >> count[i];

                if (!itemGuids[i] || !count[i] || count[i] > 1000)
                    return false;
            }

            packet >> bidMoney >> buyoutPrice >> expireTime;

            return true;
        }
    };
}
