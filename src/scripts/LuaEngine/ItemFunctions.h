/*
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEMFUNCTIONS_H
#define ITEMFUNCTIONS_H

#include "Objects/Item.hpp"
#include "Objects/Container.hpp"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"

namespace luaItem
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // GOSSIP
    int GossipCreateMenu(lua_State* L, Item* ptr)
    {
        uint32_t text_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        auto player = CHECK_PLAYER(L, 2);
        int autosend = static_cast<int>(luaL_checkinteger(L, 3));

        if (player == nullptr)
            return 0;

        if (LuaGlobal::instance()->m_menu != nullptr)
            delete LuaGlobal::instance()->m_menu;

        LuaGlobal::instance()->m_menu = new GossipMenu(ptr->getGuid(), text_id);

        if (autosend != 0)
            LuaGlobal::instance()->m_menu->sendGossipPacket(player);

        return 1;
    }

    int GossipMenuAddItem(lua_State* L, Item* /*ptr*/)
    {
        uint8_t icon = static_cast<uint8_t>(luaL_checkinteger(L, 1));
        const char* menu_text = luaL_checkstring(L, 2);
        uint32_t IntId = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        bool coded = (luaL_checkinteger(L, 4)) ? true : false;
        const char* boxmessage = luaL_optstring(L, 5, "");
        uint32_t boxmoney = static_cast<uint32_t>(luaL_optinteger(L, 6, 0));

        if (LuaGlobal::instance()->m_menu == NULL)
        {
            DLLLogDetail("There is no menu to add items to!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->addItem(icon, 0, IntId, menu_text,boxmoney, boxmessage, coded);

        return 0;
    }

    int GossipSendMenu(lua_State* L, Item* /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);

        if (LuaGlobal::instance()->m_menu == NULL)
        {
            DLLLogDetail("There is no menu to send!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->sendGossipPacket(plr);

        return 1;
    }

    int GossipComplete(lua_State* L, Item* /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);

        if (LuaGlobal::instance()->m_menu == NULL)
        {
            DLLLogDetail("There is no menu to complete!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->senGossipComplete(plr);

        return 1;
    }

    int GossipSendPOI(lua_State* L, Item* /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);
        float x = CHECK_FLOAT(L, 2);
        float y = CHECK_FLOAT(L, 3);
        uint32_t icon = static_cast<uint32_t>(luaL_checkinteger(L, 4));
        uint32_t flags = static_cast<uint32_t>(luaL_checkinteger(L, 5));
        uint32_t data = static_cast<uint32_t>(luaL_checkinteger(L, 6));
        const char* name = luaL_checkstring(L, 7);

        plr->sendGossipPoiPacket(x, y, icon, flags, data, name);
        return 1;
    }

    static int GossipSendQuickMenu(lua_State *L, Item *ptr)
    {
        if (ptr == NULL)
            return 0;

        uint32_t text_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        Player *player = CHECK_PLAYER(L, 2);
        uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        uint8_t itemicon = CHECK_UINT8(L, 4);
        const char *itemtext = luaL_checkstring(L, 5);
        uint32_t requiredmoney = CHECK_ULONG(L, 6);
        const char *moneytext = luaL_checkstring(L, 7);
        uint8_t extra = CHECK_UINT8(L, 8);

        if (player == NULL)
            return 0;

        GossipMenu::sendQuickMenu(ptr->getGuid(), text_id, player, itemid, itemicon, itemtext, requiredmoney, moneytext, extra);

        return 0;
    }


    int GetOwner(lua_State* L, Item* ptr)
    {
        Player* owner = ptr->getOwner();
        if (owner != NULL)
            PUSH_UNIT(L, owner);
        else
            lua_pushnil(L);
        return 1;
    }

    int AddEnchantment(lua_State* L, Item* ptr)
    {
        uint32_t entry = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        auto slot = static_cast<EnchantmentSlot>(luaL_checkinteger(L, 2));
        uint32_t duration = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        bool temp = (luaL_checkinteger(L, 4) == 1) ? true : false;

        lua_pushinteger(L, ptr->addEnchantment(entry, slot, duration, temp));
        return 1;
    }

    int GetGUID(lua_State* L, Item* ptr)
    {
        PUSH_GUID(L, ptr->getGuid());
        return 1;
    }

    int RemoveEnchantment(lua_State* L, Item* ptr)
    {
        int slot = static_cast<int>(luaL_checkinteger(L, 1));
        bool temp = CHECK_BOOL(L, 2);

        if (slot == -1)
            ptr->removeAllEnchantments(temp);
        else if (slot == -2)
            ptr->removeEnchantment(PERM_ENCHANTMENT_SLOT);
        else if (slot == -3)
            ptr->removeSocketBonusEnchant();
        else if (slot >= 0)
            ptr->removeEnchantment(static_cast<EnchantmentSlot>(slot));

        return 0;
    }

    int GetEntryId(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        ItemProperties const* proto = ptr->getItemProperties();
        lua_pushnumber(L, proto->ItemId);
        return 1;
    }

    int GetName(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;

        ItemProperties const* proto = ptr->getItemProperties();
        lua_pushstring(L, proto->Name.c_str());
        return 1;
    }

    int GetSpellId(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32_t index = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        if (index >= 5)
            return 0;

        ItemProperties const* proto = ptr->getItemProperties();
        lua_pushnumber(L, proto->Spells[index].Id);
        return 1;
    }

    int GetSpellTrigger(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32_t index = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        if (index >= 5)
            return 0;

        ItemProperties const* proto = ptr->getItemProperties();
        lua_pushnumber(L, proto->Spells[index].Trigger);
        /*
            USE                = 0,
            ON_EQUIP           = 1,
            CHANCE_ON_HIT      = 2,
            SOULSTONE          = 4,
            LEARNING           = 6,
            */
        return 1;
    }

    int AddLoot(lua_State* L, Item* ptr)
    {
        //if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature()) { return 0; }
        if ((lua_gettop(L) != 3) || (lua_gettop(L) != 5))
            return 0;
        uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint32_t mincount = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        uint32_t maxcount = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        std::vector<float> ichance;

        float chance = CHECK_FLOAT(L, 5);

        for (uint8_t i = 0; i == 3; i++)
            ichance.push_back(chance);

        bool perm = ((luaL_optinteger(L, 4, 0) == 1) ? true : false);
        if (perm)
        {
            QueryResult* result = WorldDatabase.Query("SELECT * FROM loot_items WHERE entryid = %u, itemid = %u", ptr->getEntry(), itemid);
            if (!result)
                WorldDatabase.Execute("REPLACE INTO loot_items VALUES (%u, %u, %f, 0, 0, 0, %u, %u )", ptr->getEntry(), itemid, chance, mincount, maxcount);
            delete result;
        }
        sLootMgr.addLoot(ptr->m_loot, itemid, ichance, mincount, maxcount, ptr->getWorldMap()->getDifficulty());
        return 1;
    }

    int GetItemLink(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32_t lang = static_cast<uint32_t>(luaL_optinteger(L, 1, LANG_UNIVERSAL));
        lua_pushstring(L, sMySQLStore.getItemLinkByProto(ptr->getItemProperties(), lang).c_str());
        return 1;
    }
    int SetByteValue(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint8_t index1 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
        uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 3));
        ptr->setByteValue(index, index1, value);*/
        return 1;
    }

    int GetByteValue(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint8_t index1 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
        lua_pushinteger(L, ptr->getByteValue(index, index1));*/
        return 1;
    }

    int GetItemLevel(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->getItemProperties()->ItemLevel);
        return 1;
    }

    int GetRequiredLevel(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->getItemProperties()->RequiredLevel);
        return 1;
    }

    int GetBuyPrice(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->getItemProperties()->BuyPrice);
        return 1;
    }

    int GetSellPrice(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->getItemProperties()->SellPrice);
        return 1;
    }

    int RepairItem(lua_State* /*L*/, Item* ptr)
    {
        if (!ptr)
            return 0;
        ptr->setDurabilityToMax();
        return 1;
    }

    int GetMaxDurability(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        lua_pushnumber(L, ptr->getMaxDurability());
        return 1;
    }

    int GetDurability(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        lua_pushnumber(L, ptr->getDurability());
        return 1;
    }

    int HasEnchantment(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        if (ptr->hasEnchantments())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    int ModifyEnchantmentTime(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        auto slot = static_cast<EnchantmentSlot>(luaL_checkinteger(L, 1));
        uint32_t duration = static_cast<uint32_t>(luaL_checkinteger(L, 2));

        ptr->modifyEnchantmentTime(slot, duration);
        return 1;
    }

    int SetStackCount(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32_t count = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        if (!count || count > 1000)
            return 0;
        ptr->setStackCount(count);
        return 1;
    }

    int HasFlag(lua_State* /*L*/, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint32_t flag = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        lua_pushboolean(L, ptr->HasFlag(index, flag) ? 1 : 0);*/
        return 1;
    }

    int IsSoulbound(lua_State* L, Item* ptr)
    {
        ptr->isSoulbound() ? lua_pushboolean(L, 1) : lua_pushboolean(L, 0);
        return 1;
    }

    int IsAccountbound(lua_State* L, Item* ptr)
    {
        ptr->isAccountbound() ? lua_pushboolean(L, 1) : lua_pushboolean(L, 0);
        return 1;
    }

    int IsContainer(lua_State* L, Item* ptr)
    {
        ptr->isContainer() ? lua_pushboolean(L, 1) : lua_pushboolean(L, 0);
        return 1;
    }

    int GetContainerItemCount(lua_State* L, Item* ptr)
    {
        uint32_t itemid = CHECK_ULONG(L, 1);
        if (!ptr->isContainer() || !itemid) return 0;
        Container* pCont = static_cast< Container* >(ptr);
        int16_t TotalSlots = static_cast<int16_t>(pCont->getSlotCount());
        int cnt = 0;
        for (int16_t i = 0; i < TotalSlots; i++)
        {
            Item* item = pCont->getItem(i);
            if (item)
            {
                if (item->getEntry() == itemid && item->m_wrappedItemId == 0)
                {
                    cnt += item->getStackCount() ? item->getStackCount() : 1;
                }
            }
        }
        lua_pushinteger(L, cnt);
        return 1;
    }

    int GetEquippedSlot(lua_State* L, Item* ptr)
    {
        if (!ptr) return 0;
        lua_pushinteger(L, ptr->getOwner()->getItemInterface()->GetInventorySlotById(ptr->getEntry()));
        return 1;
    }

    int GetObjectType(lua_State* L, Item* ptr)
    {
        if (!ptr) { lua_pushnil(L); return 1; }
        lua_pushstring(L, "Item");
        return 1;
    }

    int Remove(lua_State* /*L*/, Item* ptr)
    {
        if (ptr == NULL || !ptr->IsInWorld() || !ptr->isItem())
        {
            return 0;
        }
        ptr->removeFromWorld();
        return 0;
    }

    int Create(lua_State* L, Item* /*ptr*/)
    {
        uint32_t id = CHECK_ULONG(L, 1);
        uint32_t stackcount = CHECK_ULONG(L, 2);
        Item* pItem = sObjectMgr.createItem(id, NULL);
        if (!pItem)
        {
            lua_pushnil(L);
            return 1;
        }
        pItem->setStackCount(stackcount);
        pItem->saveToDB(0, 0, true, NULL);
        PUSH_ITEM(L, pItem);
        return 1;
    }

    int ModUInt32Value(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        int32_t value = static_cast<int32_t>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->modInt32Value(field, value);*/
        return 0;
    }

    int ModFloatValue(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        float value = CHECK_FLOAT(L, 2);
        if (ptr)
            ptr->modFloatValue(field, value);*/
        return 0;
    }

    int SetUInt32Value(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->setUInt32Value(field, value);*/
        return 0;
    }

    int SetUInt64Value(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(CHECK_ULONG(L, 1));
        uint64_t guid = static_cast<uint64_t>(CHECK_GUID(L, 2));
        if (ptr)
            ptr->setUInt64Value(field, guid);*/
        return 0;
    }

    int RemoveFlag(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->RemoveFlag(field, value);*/
        return 0;
    }

    int SetFlag(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->SetFlag(field, value);*/
        return 0;
    }

    int SetFloatValue(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        float value = CHECK_FLOAT(L, 2);
        if (ptr)
            ptr->setFloatValue(field, value);*/
        return 0;
    }

    int GetUInt32Value(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            lua_pushnumber(L, ptr->getUInt32Value(field));*/
        return 1;
    }

    int GetUInt64Value(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            PUSH_GUID(L, ptr->getUInt64Value(field));*/
        return 1;
    }

    int GetFloatValue(lua_State* /*L*/, Item* /*ptr*/)
    {
        /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            lua_pushnumber(L, ptr->getFloatValue(field));*/
        return 1;
    }
}

#endif      // ITEMFUNCTIONS_H
