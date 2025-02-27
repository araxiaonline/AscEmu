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

#include "Server/MainServerDefines.h"
#include "Server/Master.h"
#include "Debugging/CrashHandler.h"
#include "ServerState.h"

#ifndef WIN32
#include <sys/resource.h>
#endif

#ifndef WIN32
int unix_main(int argc, char** argv)
{
    rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1)
        printf("getrlimit failed. This could be problem.\n");
    else
    {
        rl.rlim_cur = rl.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rl) == -1)
            printf("setrlimit failed. Server may not save core.dump files.\n");
    }

    if (!sMaster.Run(argc, argv))
        exit(-1);
    else
        exit(0);

    return 0; // shouldn't be reached
}

#else

int win32_main(int argc, char** argv)
{
    SetThreadName("Main Thread");

    StartCrashHandler();

    //Andy: windows only, helps fight heap allocation on allocations lower then 16KB
    unsigned long arg = 2;
    HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &arg, sizeof(arg));

    THREAD_TRY_EXECUTION
        sMaster.Run(argc, argv);
    THREAD_HANDLE_CRASH

        exit(0);
}

#endif

int main(int argc, char** argv)
{
    // Init this asap to set initTime correctly
    ServerState::instance();

#ifdef WIN32
    win32_main(argc, argv);
#else
    unix_main(argc, argv);
#endif
}
