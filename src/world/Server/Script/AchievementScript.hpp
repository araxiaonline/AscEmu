/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"
#include "CommonTypes.hpp"

#ifdef FT_ACHIEVEMENTS

class Object;
class Player;

class SERVER_DECL AchievementCriteriaScript
{
public:
    AchievementCriteriaScript() = default;
    virtual ~AchievementCriteriaScript() {}

    // Note; target can be nullptr!
    virtual bool canCompleteCriteria(uint32_t criteriaId, Player* player, Object* target);
};
#endif
