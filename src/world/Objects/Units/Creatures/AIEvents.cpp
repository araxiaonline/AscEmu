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

#include "AIEvents.h"
#include "AIInterface.h"

pAIEvent AIEventHandlers[NUM_AI_EVENTS] =
{
    &AIInterface::eventEnterCombat,
    &AIInterface::eventLeaveCombat,
    &AIInterface::eventDamageTaken,
    &AIInterface::eventFear,
    &AIInterface::eventUnfear,
    &AIInterface::eventFollowOwner,
    &AIInterface::eventWander,
    &AIInterface::eventUnwander,
    &AIInterface::eventUnitDied,
    &AIInterface::eventHostileAction,
    &AIInterface::eventForceRedirected,
};
