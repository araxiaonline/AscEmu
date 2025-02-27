/*
 * AscEmu Framework based on ArcEmu MMORPG Server
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

#ifndef SPELLTARGET_H
#define SPELLTARGET_H

enum SpellImplicitTargetType
{
    SPELL_TARGET_REQUIRE_GAMEOBJECT         = 0x00000001,
    SPELL_TARGET_REQUIRE_ITEM               = 0x00000002,
    SPELL_TARGET_REQUIRE_ATTACKABLE         = 0x00000004,
    SPELL_TARGET_REQUIRE_FRIENDLY           = 0x00000008,
    SPELL_TARGET_OBJECT_SCRIPTED            = 0x00000010,   ///scripted units
    SPELL_TARGET_OBJECT_SELF                = 0x00000020,
    SPELL_TARGET_OBJECT_CURPET              = 0x00000040,
    SPELL_TARGET_OBJECT_CURCRITTER          = 0x00000080,
    SPELL_TARGET_OBJECT_PETOWNER            = 0x00000100,
    SPELL_TARGET_OBJECT_CURTOTEMS           = 0x00000200,
    SPELL_TARGET_OBJECT_TARCLASS            = 0x00000400,
    SPELL_TARGET_AREA                       = 0x00000800,
    SPELL_TARGET_AREA_SELF                  = 0x00001000,
    SPELL_TARGET_AREA_CONE                  = 0x00002000,
    SPELL_TARGET_AREA_CHAIN                 = 0x00004000,
    SPELL_TARGET_AREA_CURTARGET             = 0x00008000,
    SPELL_TARGET_AREA_RANDOM                = 0x00010000,
    SPELL_TARGET_AREA_PARTY                 = 0x00020000,
    SPELL_TARGET_AREA_RAID                  = 0x00040000,
    SPELL_TARGET_NOT_IMPLEMENTED            = 0x00080000,
    SPELL_TARGET_NO_OBJECT                  = 0x00100000,
    SPELL_TARGET_ANY_OBJECT                 = 0x00200000,

    // Helpers
    SPELL_TARGET_REQUIRE_UNIT               = SPELL_TARGET_REQUIRE_ATTACKABLE | SPELL_TARGET_REQUIRE_FRIENDLY,

    SPELL_TARGET_AREA_MASK                  = SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CONE |
        SPELL_TARGET_AREA_CHAIN | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_RANDOM |
        SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID
};

#endif // _SPELLTARGET_H
