/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LUAEngine.h"
#include "Server/Script/ScriptMgr.h"
#include "Server/Script/ScriptSetup.h"

#ifdef __APPLE__
#undef check
#endif

#ifndef _WIN32
#include <dirent.h>
#endif

ScriptMgr* m_scriptMgr = nullptr;

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32_t _exp_get_script_type()
{
    return SCRIPT_TYPE_SCRIPT_ENGINE;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
    m_scriptMgr = mgr;
    LuaGlobal::instance()->luaEngine()->Startup();
}

extern "C" SCRIPT_DECL void _exp_engine_unload()
{
    DLLLogDetail("exp_engine_unload was called");
}

extern "C" SCRIPT_DECL void _export_engine_reload()
{
    LuaGlobal::instance()->luaEngine()->Restart();
}

void report(lua_State* L)
{
    int count = lua_gettop(L);

    while (count > 0)
    {
        const char* msg = lua_tostring(L, -1);
        DLLLogDetail(msg);
        lua_pop(L, 1);
        count--;
    }
}

LuaEngine::LuaEngine() : lu(nullptr) {}

void LuaEngine::ScriptLoadDir(const std::string Dirname, LUALoadScripts* pak)
{
    DLLLogDetail("LuaEngine : Scanning Directory %s", Dirname.c_str());

    if (!fs::exists(Dirname))
    {
        DLLLogDetail("LuaEngine : Directory \"scripts\" does not exist.");
        return;
    }

    auto luaScripts = Util::getDirectoryContent(Dirname, ".lua", true);
    for (auto& luaScript : luaScripts)
    {
        std::string fileName = luaScript.second;
        pak->luaFiles.insert(fileName);
    }
}

void LuaEngine::LoadScripts()
{
    DLLLogDetail("LuaEngine : Scanning Script-Directories...");

    LUALoadScripts rtn;
    ScriptLoadDir("scripts", &rtn);

    luaL_openlibs(lu);

    RegisterCoreFunctions();

    DLLLogDetail("LuaEngine : Loading Scripts...");

    unsigned int cntUncomp = 0;
    for (auto& itr : rtn.luaFiles)
    {
        const auto errorCode = luaL_loadfile(lu, itr.c_str());
        if (errorCode)
        {
            DLLLogDetail("loading %s failed.(could not load). Error code %i", itr.c_str(), errorCode);
            report(lu);
        }
        else
        {
            if (errorCode != lua_pcall(lu, 0, 0, 0))
            {
                DLLLogDetail("%s failed.(could not run). Error code %i", itr.c_str(), errorCode);
                report(lu);
            }
            else
            {
                DLLLogDetail("LuaEngine : loaded %s", itr.c_str());
            }
        }
        cntUncomp++;
    }
    DLLLogDetail("LuaEngine : Loaded %u Lua scripts.", cntUncomp);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function call methods
void LuaEngine::BeginCall(uint16_t fReference)
{
    lua_settop(lu, 0); //stack should be empty
    lua_rawgeti(lu, LUA_REGISTRYINDEX, fReference);
}

bool LuaEngine::ExecuteCall(uint8_t params, uint8_t res)
{
    bool ret = true;
    int top = lua_gettop(lu);
    if (strcmp(luaL_typename(lu, top - params), "function") != 0)
    {
        ret = false;
        //Paroxysm : Stack Cleaning here, not sure what causes that crash in luaH_getstr, maybe due to lack of stack space. Anyway, experimental.
        if (params > 0)
        {
            for (int i = top; i >= (top - params); i--)
                if (!lua_isnone(lu, i))
                    lua_remove(lu, i);
        }
    }
    else
    {
        if (lua_pcall(lu, params, res, 0))
        {
            report(lu);
            ret = false;
        }
    }
    return ret;
}
void LuaEngine::EndCall(uint8_t res)
{
    for (int i = res; i > 0; i--)
        if (!lua_isnone(lu, res))
            lua_remove(lu, res);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Push methods
void LuaEngine::PushUnit(Object* unit, lua_State* L)
{
    Unit* pUnit = nullptr;
    if (unit != nullptr && unit->isCreatureOrPlayer())
        pUnit = static_cast<Unit*>(unit);

    if (L == nullptr)
        ArcLuna<Unit>::push(lu, pUnit);
    else
        ArcLuna<Unit>::push(L, pUnit);
}

void LuaEngine::PushGo(Object* go, lua_State* L)
{
    GameObject* pGo = nullptr;
    if (go != nullptr && go->isGameObject())
        pGo = static_cast< GameObject* >(go);

    if (L == nullptr)
        ArcLuna<GameObject>::push(lu, pGo);
    else
        ArcLuna<GameObject>::push(L, pGo);
}

void LuaEngine::PushItem(Object* item, lua_State* L)
{
    Item* pItem = nullptr;
    if (item != nullptr && (item->isItem() || item->isContainer()))
        pItem = static_cast< Item* >(item);

    if (L == nullptr)
        ArcLuna<Item>::push(lu, pItem);
    else
        ArcLuna<Item>::push(L, pItem);
}

void LuaEngine::PushGuid(uint64_t guid, lua_State* L)
{
    if (L == nullptr)
        GUID_MGR::push(lu, guid);
    else
        GUID_MGR::push(L, guid);
}

void LuaEngine::PushPacket(WorldPacket* pack, lua_State* L)
{
    if (L == nullptr)
        ArcLuna<WorldPacket>::push(lu, pack, true);
    else
        ArcLuna<WorldPacket>::push(L, pack, true);
}

void LuaEngine::PushTaxiPath(TaxiPath tp, lua_State* L)
{
    if (L == nullptr)
        ArcLuna<TaxiPath>::push(lu, &tp, true);
    else
        ArcLuna<TaxiPath>::push(L, &tp, true);
}

void LuaEngine::PushSpell(Spell* sp, lua_State* L)
{
    if (L == nullptr)
        ArcLuna<Spell>::push(lu, sp);
    else
        ArcLuna<Spell>::push(L, sp);
}

void LuaEngine::PushSqlField(Field* field, lua_State* L)
{
    if (L == nullptr)
        ArcLuna<Field>::push(lu, field);
    else
        ArcLuna<Field>::push(L, field);
}

void LuaEngine::PushSqlResult(QueryResult* res, lua_State* L)
{
    if (L == nullptr)
        ArcLuna<QueryResult>::push(lu, res, true);
    else
        ArcLuna<QueryResult>::push(L, res, true);
}

void LuaEngine::PushAura(Aura* aura, lua_State* L)
{
    if (L == nullptr)
        ArcLuna<Aura>::push(lu, aura);
    else
        ArcLuna<Aura>::push(L, aura);
}
// End push methods
//////////////////////////////////////////////////////////////////////////////////////////

void LuaEngine::HyperCallFunction(const char* FuncName, int ref)  //hyper as in hypersniper :3
{
    GET_LOCK
    std::string sFuncName = std::string(FuncName);
    char* copy = strdup(FuncName);
    char* token = strtok(copy, ".:");
    int top = 1;
    bool colon = false;

    //REMEMBER: top is always 1
    lua_settop(lu, 0); //stack should be empty
    if (strpbrk(FuncName, ".:") == nullptr) //stack: empty
    {
        lua_getglobal(lu, FuncName);    //stack: function
    }
    else
    {
        lua_getglobal(lu, "_G"); //start out with the global table.  //stack: _G
        while (token != nullptr)
        {
            lua_getfield(lu, -1, token); //get the (hopefully) table/func  //stack: _G, subtable/func/nil
            if ((int)sFuncName.find(token) - 1 > 0) //if it isn't the first token
            {
                if (sFuncName.at(sFuncName.find(token) - 1) == ':')
                    colon = true;
            }

            if (lua_isfunction(lu, -1) && !lua_iscfunction(lu, -1)) //if it's a Lua function //stack: _G/subt, func
            {
                if (colon) //stack: subt, func
                {
                    lua_pushvalue(lu, -2); //make the table the first arg //stack: subt, func, subt
                    lua_remove(lu, top); //stack: func, subt
                }
                else
                {
                    lua_replace(lu, top);    //stack: func
                }

                break; //we don't need anything else
            }

            if (lua_istable(lu, -1)) //stack: _G/subt, subtable
            {
                token = strtok(nullptr, ".:"); //stack: _G/subt, subtable
                lua_replace(lu, top); //stack: subtable
            }
        }
    }

    lua_rawgeti(lu, LUA_REGISTRYINDEX, ref);
    lua_State* M = lua_tothread(lu, -1);  //repeats, args
    int thread = lua_gettop(lu);
    int repeats = static_cast<int>(luaL_checkinteger(M, 1)); //repeats, args
    int nargs = lua_gettop(M) - 1;
    if (nargs != 0) //if we HAVE args...
    {
        for (int i = 2; i <= nargs + 1; i++)
            lua_pushvalue(M, i);

        lua_xmove(M, lu, nargs);
    }
    if (repeats != 0)
    {
        if (--repeats == 0) //free stuff, then
        {
            free((void*)FuncName);
            luaL_unref(lu, LUA_REGISTRYINDEX, ref);
            const auto itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.find(ref);
            LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.erase(itr);
        }
        else
        {
            lua_remove(M, 1); //args
            lua_pushinteger(M, repeats); //args, repeats
            lua_insert(M, 1); //repeats, args
        }
    }
    lua_remove(lu, thread); //now we can remove the thread object
    int r = lua_pcall(lu, nargs + (colon ? 1 : 0), 0, 0);
    if (r)
        report(lu);

    free((void*)copy);
    lua_settop(lu, 0);
    RELEASE_LOCK
}

/*
This version only accepts actual Lua functions and no arguments. See LCF_Extra:CreateClosure(...) to pass arguments to this function.
*/
static int CreateLuaEvent(lua_State* L)
{
    GET_LOCK
    int delay = static_cast<int>(luaL_checkinteger(L, 2));
    int repeats = static_cast<int>(luaL_checkinteger(L, 3));
    if (!strcmp(luaL_typename(L, 1), "function") || delay > 0)
    {
        lua_settop(L, 1);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        TimedEvent* ev = TimedEvent::Allocate(&sWorld, new CallbackP1<LuaEngine, int>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::CallFunctionByReference, functionRef), 0, delay, repeats);
        ev->eventType = LUA_EVENTS_END + functionRef; //Create custom reference by adding the ref number to the max lua event type to get a unique reference for every function.
        sWorld.event_AddEvent(ev);
        LuaGlobal::instance()->luaEngine()->getFunctionRefs().insert(functionRef);
        lua_pushinteger(L, functionRef);
    }
    else
    {
        lua_pushnil(L);
    }
    RELEASE_LOCK

    return 1;
}

void LuaEngine::CallFunctionByReference(int ref)
{
    GET_LOCK

    lua_rawgeti(lu, LUA_REGISTRYINDEX, ref);
    if (lua_pcall(lu, 0, 0, 0))
        report(lu);

    RELEASE_LOCK
}

void LuaEngine::DestroyAllLuaEvents()
{
    GET_LOCK

    //Clean up for all events.
    for (auto itr = m_functionRefs.begin(); itr != m_functionRefs.end(); ++itr)
    {
        sEventMgr.RemoveEvents(&sWorld, (*itr) + LUA_EVENTS_END);
        luaL_unref(lu, LUA_REGISTRYINDEX, (*itr));
    }
    m_functionRefs.clear();

    RELEASE_LOCK
}

static int ModifyLuaEventInterval(lua_State* L)
{
    GET_LOCK

    int ref = static_cast<int>(luaL_checkinteger(L, 1));
    int newinterval = static_cast<int>(luaL_checkinteger(L, 2));
    ref += LUA_EVENTS_END;
    //Easy interval modification.
    sEventMgr.ModifyEventTime(&sWorld, ref, newinterval);

    RELEASE_LOCK

    return 0;
}

static int DestroyLuaEvent(lua_State* L)
{
    //Simply remove the reference, CallFunctionByReference will find the reference has been freed and skip any processing.
    GET_LOCK

    int ref = static_cast<int>(luaL_checkinteger(L, 1));
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
    LuaGlobal::instance()->luaEngine()->getFunctionRefs().erase(ref);
    sEventMgr.RemoveEvents(&sWorld, ref + LUA_EVENTS_END);

    RELEASE_LOCK

    return 0;
}

void LuaEngine::RegisterCoreFunctions()
{
    lua_register(lu, "RegisterUnitEvent", RegisterUnitEvent);
    lua_register(lu, "RegisterGameObjectEvent", RegisterGameObjectEvent);
    lua_register(lu, "RegisterQuestEvent", RegisterQuestEvent);
    lua_register(lu, "RegisterUnitGossipEvent", RegisterUnitGossipEvent);
    lua_register(lu, "RegisterItemGossipEvent", RegisterItemGossipEvent);
    lua_register(lu, "RegisterGOGossipEvent", RegisterGOGossipEvent);
    lua_register(lu, "RegisterServerHook", RegisterServerHook);
    lua_register(lu, "SuspendThread", &SuspendLuaThread);
    lua_register(lu, "RegisterTimedEvent", &RegisterTimedEvent);
    lua_register(lu, "RemoveTimedEvents", &RemoveTimedEvents);
    lua_register(lu, "RegisterDummySpell", &RegisterDummySpell);
    lua_register(lu, "RegisterInstanceEvent", &RegisterInstanceEvent);

    lua_register(lu, "CreateLuaEvent", &CreateLuaEvent);
    lua_register(lu, "ModifyLuaEventInterval", &ModifyLuaEventInterval);
    lua_register(lu, "DestroyLuaEvent", &DestroyLuaEvent);

    RegisterGlobalFunctions(lu);

    ArcLuna<Unit>::Register(lu);
    ArcLuna<Item>::Register(lu);
    ArcLuna<GameObject>::Register(lu);
    ArcLuna<WorldPacket>::Register(lu);
    ArcLuna<TaxiPath>::Register(lu);
    ArcLuna<Spell>::Register(lu);
    ArcLuna<Field>::Register(lu);
    ArcLuna<QueryResult>::Register(lu);
    ArcLuna<Aura>::Register(lu);

    GUID_MGR::Register(lu);

    //set the suspendluathread a coroutine function
    lua_getglobal(lu, "coroutine");
    if (lua_istable(lu, -1))
    {
        lua_pushcfunction(lu, SuspendLuaThread);
        lua_setfield(lu, -2, "wait");
        lua_pushcfunction(lu, SuspendLuaThread);
        lua_setfield(lu, -2, "WAIT");
    }
    lua_pop(lu, 1);
}

static int RegisterServerHook(lua_State* L)
{
    uint16_t functionRef = 0;
    //Maximum passed in arguments, consider rest as garbage
    lua_settop(L, 2);
    uint32_t ev = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const char* typeName = luaL_typename(L, 2);
    if (!ev || typeName == nullptr)
        return 0;

    //For functions directly passed in, skip all that code and register the reference.
    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 2)));
    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_SERVHOOK, 0, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterDummySpell(lua_State* L)
{
    uint16_t functionRef = 0;
    uint32_t entry = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const char* typeName = luaL_typename(L, 2);
    lua_settop(L, 2);

    if (!entry || typeName == nullptr)
        return 0;

    if (LuaGlobal::instance()->m_luaDummySpells.find(entry) != LuaGlobal::instance()->m_luaDummySpells.end())
        luaL_error(L, "LuaEngineMgr : RegisterDummySpell failed! Spell %d already has a registered Lua function!", entry);
    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 2)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_DUMMYSPELL, entry, 0, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterUnitEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int entry = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!entry || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_UNIT, entry, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterInstanceEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int map = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!map || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_INSTANCE, map, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterQuestEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int entry = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!entry || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_QUEST, entry, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterGameObjectEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int entry = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!entry || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_GO, entry, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterUnitGossipEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int entry = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!entry || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_UNIT_GOSSIP, entry, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}
static int RegisterItemGossipEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int entry = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!entry || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_ITEM_GOSSIP, entry, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int RegisterGOGossipEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16_t functionRef = 0;
    int entry = static_cast<int>(luaL_checkinteger(L, 1));
    int ev = static_cast<int>(luaL_checkinteger(L, 2));
    const char* typeName = luaL_typename(L, 3);

    if (!entry || !ev || typeName == nullptr)
        return 0;

    if (!strcmp(typeName, "function"))
        functionRef = static_cast<uint16_t>(luaL_ref(L, LUA_REGISTRYINDEX));
    else if (!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = static_cast<uint16_t>(LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 3)));

    if (functionRef > 0)
        LuaGlobal::instance()->luaEngine()->RegisterEvent(REGTYPE_GO_GOSSIP, entry, ev, functionRef);

    lua_pop(L, lua_gettop(L));

    return 0;
}

static int SuspendLuaThread(lua_State* L)
{
    lua_State* thread = (lua_isthread(L, 1)) ? lua_tothread(L, 1) : nullptr;
    if (thread == nullptr)
        return luaL_error(L, "LuaEngineMgr", "SuspendLuaThread expected Lua coroutine, got nullptr. \n");

    int waitime = static_cast<int>(luaL_checkinteger(L, 2));
    if (waitime <= 0)
        return luaL_error(L, "LuaEngineMgr", "SuspendLuaThread expected timer > 0 instead got (%d) \n", waitime);

    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL || ref == LUA_NOREF)
        return luaL_error(L, "Error in SuspendLuaThread! Failed to create a valid reference.");

    TimedEvent* evt = TimedEvent::Allocate(thread, new CallbackP1<LuaEngine, int>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::ResumeLuaThread, ref), 0, waitime, 1);
    sWorld.event_AddEvent(evt);
    lua_remove(L, 1); // remove thread object
    lua_remove(L, 1); // remove timer.
                      //All that remains now are the extra arguments passed to this function.
    lua_xmove(L, thread, lua_gettop(L));
    LuaGlobal::instance()->luaEngine()->getThreadRefs().insert(ref);

    return lua_yield(thread, lua_gettop(L));
}

static int RegisterTimedEvent(lua_State* L)  //in this case, L == lu
{
    const char* funcName = strdup(luaL_checkstring(L, 1));
    int delay = static_cast<int>(luaL_checkinteger(L, 2));
    int repeats = static_cast<int>(luaL_checkinteger(L, 3));
    if (!delay || repeats < 0 || !funcName)
    {
        lua_pushnumber(L, LUA_REFNIL);
        free((void*)funcName);
        return 1;
    }

    lua_remove(L, 1);
    lua_remove(L, 1);//repeats, args
    lua_State* thread = lua_newthread(L);  //repeats, args, thread
    lua_insert(L, 1); //thread, repeats, args
    lua_xmove(L, thread, lua_gettop(L) - 1); //thread

    int ref = luaL_ref(L, LUA_REGISTRYINDEX); //empty
    if (ref == LUA_REFNIL || ref == LUA_NOREF)
    {
        free((void*)funcName);
        return luaL_error(L, "Error in RegisterTimedEvent! Failed to create a valid reference.");
    }

    TimedEvent* te = TimedEvent::Allocate(LuaGlobal::instance()->luaEngine().get(), new CallbackP2<LuaEngine, const char*, int>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::HyperCallFunction, funcName, ref), EVENT_LUA_TIMED, delay, repeats);
    EventInfoHolder* ek = new EventInfoHolder;
    ek->funcName = funcName;
    ek->te = te;
    LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.insert(std::pair<int, EventInfoHolder*>(ref, ek));
    LuaGlobal::instance()->luaEngine()->LuaEventMgr.event_AddEvent(te);
    lua_settop(L, 0);
    lua_pushnumber(L, ref);
    return 1;
}

static int RemoveTimedEvents(lua_State* /*L*/)  //in this case, L == lu
{
    LuaGlobal::instance()->luaEngine()->LuaEventMgr.RemoveEvents();
    return 0;
}

//all of these run similarly, they execute OnServerHook for all the functions in their respective event's list.
bool LuaHookOnNewCharacter(uint32_t Race, uint32_t Class, WorldSession* /*Session*/, const char* Name)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_NEW_CHARACTER])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_NEW_CHARACTER);
        LuaGlobal::instance()->luaEngine()->PUSH_STRING(Name);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Race);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Class);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(4, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }
    RELEASE_LOCK

    return result;
}

void LuaHookOnKillPlayer(Player* pPlayer, Player* pVictim)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_KILL_PLAYER])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_KILL_PLAYER);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PushUnit(pVictim);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnFirstEnterWorld(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

void LuaHookOnEnterWorld(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_ENTER_WORLD])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_ENTER_WORLD);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

void LuaHookOnGuildJoin(Player* pPlayer, Guild* pGuild)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_GUILD_JOIN])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_GUILD_JOIN);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_STRING(pGuild->getNameChar());
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnDeath(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_DEATH])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_DEATH);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

bool LuaHookOnRepop(Player* pPlayer)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_REPOP])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_REPOP);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(2, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }
    RELEASE_LOCK

    return result;
}

void LuaHookOnEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_EMOTE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_EMOTE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PushUnit(pUnit);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Emote);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);
    }

    RELEASE_LOCK
}

void LuaHookOnEnterCombat(Player* pPlayer, Unit* pTarget)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_ENTER_COMBAT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_ENTER_COMBAT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PushUnit(pTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

bool LuaHookOnCastSpell(Player* pPlayer, SpellInfo const* pSpell, Spell* spell)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_CAST_SPELL])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_CAST_SPELL);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(pSpell->getId());
        LuaGlobal::instance()->luaEngine()->PushSpell(spell);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(4, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }

    RELEASE_LOCK

    return result;
}

void LuaHookOnTick()
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_TICK])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->ExecuteCall();
    }

    RELEASE_LOCK
}

bool LuaHookOnLogoutRequest(Player* pPlayer)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(2, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }

    RELEASE_LOCK

    return result;
}

void LuaHookOnLogout(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOGOUT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_LOGOUT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

void LuaHookOnQuestAccept(Player* pPlayer, QuestProperties* pQuest, Object* pQuestGiver)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_ACCEPT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(pQuest->id);

        if (!pQuestGiver)
            LuaGlobal::instance()->luaEngine()->PUSH_NIL();
        else if (pQuestGiver->isCreatureOrPlayer())
            LuaGlobal::instance()->luaEngine()->PushUnit(pQuestGiver);
        else if (pQuestGiver->isGameObject())
            LuaGlobal::instance()->luaEngine()->PushGo(pQuestGiver);
        else if (pQuestGiver->isItem())
            LuaGlobal::instance()->luaEngine()->PushItem(pQuestGiver);
        else
            LuaGlobal::instance()->luaEngine()->PUSH_NIL();

        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);
    }

    RELEASE_LOCK
}

void LuaHookOnZone(Player* pPlayer, uint32_t Zone, uint32_t oldZone)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_ZONE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_ZONE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Zone);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(oldZone);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);
    }

    RELEASE_LOCK
}

bool LuaHookOnChat(Player* pPlayer, uint32_t Type, uint32_t Lang, const char* Message, const char* Misc)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_CHAT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_CHAT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_STRING(Message);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Type);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Lang);
        LuaGlobal::instance()->luaEngine()->PUSH_STRING(Misc);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(6, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }

    RELEASE_LOCK

    return result;
}

void LuaHookOnLoot(Player* pPlayer, Unit* pTarget, uint32_t Money, uint32_t ItemId)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOOT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_LOOT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PushUnit(pTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Money);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(ItemId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(5);
    }

    RELEASE_LOCK
}

void LuaHookOnGuildCreate(Player* pLeader, Guild* pGuild)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_GUILD_CREATE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_GUILD_CREATE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pLeader);
        LuaGlobal::instance()->luaEngine()->PUSH_STRING(pGuild->getNameChar());
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnEnterWorld2(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_FULL_LOGIN])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_FULL_LOGIN);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

void LuaHookOnCharacterCreate(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_CHARACTER_CREATE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_CHARACTER_CREATE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

void LuaHookOnQuestCancelled(Player* pPlayer, QuestProperties* pQuest)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_CANCELLED])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(pQuest->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnQuestFinished(Player* pPlayer, QuestProperties* pQuest, Object* pQuestGiver)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_FINISHED])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_QUEST_FINISHED);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(pQuest->id);

        if (!pQuestGiver)
            LuaGlobal::instance()->luaEngine()->PUSH_NIL();
        else if (pQuestGiver->isCreatureOrPlayer())
            LuaGlobal::instance()->luaEngine()->PushUnit(pQuestGiver);
        else if (pQuestGiver->isGameObject())
            LuaGlobal::instance()->luaEngine()->PushGo(pQuestGiver);
        else if (pQuestGiver->isItem())
            LuaGlobal::instance()->luaEngine()->PushItem(pQuestGiver);
        else
            LuaGlobal::instance()->luaEngine()->PUSH_NIL();

        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);
    }

    RELEASE_LOCK
}

void LuaHookOnHonorableKill(Player* pPlayer, Player* pKilled)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_HONORABLE_KILL])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_HONORABLE_KILL);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PushUnit(pKilled);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_ARENA_FINISH])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_ARENA_FINISH);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_STRING(pTeam->m_name.c_str());
        LuaGlobal::instance()->luaEngine()->PUSH_BOOL(victory);
        LuaGlobal::instance()->luaEngine()->PUSH_BOOL(rated);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(5);
    }

    RELEASE_LOCK
}

void LuaHookOnObjectLoot(Player* pPlayer, Object* pTarget, uint32_t Money, uint32_t ItemId)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_OBJECTLOOT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_OBJECTLOOT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PushUnit(pTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Money);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(ItemId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(5);
    }

    RELEASE_LOCK
}

void LuaHookOnAreaTrigger(Player* pPlayer, uint32_t areaTrigger)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_AREATRIGGER])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_AREATRIGGER);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(areaTrigger);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnPostLevelUp(Player* pPlayer)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_POST_LEVELUP])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_POST_LEVELUP);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

bool LuaHookOnPreUnitDie(Unit* Killer, Unit* Victim)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_PRE_DIE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_PRE_DIE);
        LuaGlobal::instance()->luaEngine()->PushUnit(Killer);
        LuaGlobal::instance()->luaEngine()->PushUnit(Victim);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(3, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }

    RELEASE_LOCK

    return result;
}

void LuaHookOnAdvanceSkillLine(Player* pPlayer, uint32_t SkillLine, uint32_t Current)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(SkillLine);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(Current);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);
    }

    RELEASE_LOCK
}

void LuaHookOnDuelFinished(Player* pWinner, Player* pLoser)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_DUEL_FINISHED])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_DUEL_FINISHED);
        LuaGlobal::instance()->luaEngine()->PushUnit(pWinner);
        LuaGlobal::instance()->luaEngine()->PushUnit(pLoser);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
    }

    RELEASE_LOCK
}

void LuaHookOnAuraRemove(Aura* aura)
{
    GET_LOCK

    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_AURA_REMOVE])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_AURA_REMOVE);
        LuaGlobal::instance()->luaEngine()->PushAura(aura);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);
    }

    RELEASE_LOCK
}

bool LuaHookOnResurrect(Player* pPlayer)
{
    GET_LOCK

    bool result = true;
    for (auto itr : LuaGlobal::instance()->EventAsToFuncName[SERVER_HOOK_EVENT_ON_RESURRECT])
    {
        LuaGlobal::instance()->luaEngine()->BeginCall(itr);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(SERVER_HOOK_EVENT_ON_RESURRECT);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        if (LuaGlobal::instance()->luaEngine()->ExecuteCall(2, 1))
        {
            lua_State* L = LuaGlobal::instance()->luaEngine()->getluState();
            if (!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;

            LuaGlobal::instance()->luaEngine()->EndCall(1);
        }
    }

    RELEASE_LOCK

    return result;
}

bool LuaOnDummySpell(uint8_t effectIndex, Spell* pSpell)
{
    GET_LOCK

    LuaGlobal::instance()->luaEngine()->BeginCall(LuaGlobal::instance()->m_luaDummySpells[pSpell->getSpellInfo()->getId()]);
    LuaGlobal::instance()->luaEngine()->PUSH_UINT(effectIndex);
    LuaGlobal::instance()->luaEngine()->PushSpell(pSpell);
    LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

    RELEASE_LOCK

    return true;
}

class LuaCreature : public CreatureAIScript
{
public:
    LuaCreature(Creature* creature) : CreatureAIScript(creature), m_binding(nullptr) {}
    ~LuaCreature()
    {}

    void OnCombatStart(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_ENTER_COMBAT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_ENTER_COMBAT);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnCombatStop(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_LEAVE_COMBAT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_LEAVE_COMBAT);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnTargetDied(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_TARGET_DIED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_TARGET_DIED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnDied(Unit* mKiller)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_DIED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_DIED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mKiller);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnTargetParried(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_TARGET_PARRIED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_TARGET_PARRIED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnTargetDodged(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_TARGET_DODGED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_TARGET_DODGED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnTargetBlocked(Unit* mTarget, int32_t iAmount)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_TARGET_BLOCKED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_TARGET_BLOCKED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(iAmount);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnTargetCritHit(Unit* mTarget, int32_t fAmount)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_TARGET_CRIT_HIT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_TARGET_CRIT_HIT);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(fAmount);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnParried(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_PARRY]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_PARRY);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnDodged(Unit* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_DODGED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_DODGED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnBlocked(Unit* mTarget, int32_t iAmount)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_BLOCKED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_BLOCKED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(iAmount);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnCritHit(Unit* mTarget, int32_t fAmount)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_CRIT_HIT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_CRIT_HIT);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_INT(fAmount);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnHit(Unit* mTarget, float fAmount)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_HIT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_HIT);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_FLOAT(fAmount);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnAssistTargetDied(Unit* mAssistTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_ASSIST_TARGET_DIED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_ASSIST_TARGET_DIED);
        LuaGlobal::instance()->luaEngine()->PushUnit(mAssistTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnFear(Unit* mFeared, uint32_t iSpellId)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_FEAR]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_FEAR);
        LuaGlobal::instance()->luaEngine()->PushUnit(mFeared);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(iSpellId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnFlee(Unit* mFlee)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_FLEE]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_FLEE);
        LuaGlobal::instance()->luaEngine()->PushUnit(mFlee);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnCallForHelp()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_CALL_FOR_HELP]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_CALL_FOR_HELP);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    void OnLoad()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_LOAD]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_LOAD);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
        uint32_t iid = getCreature()->GetInstanceID();
        if (getCreature()->getWorldMap() == nullptr || getCreature()->getWorldMap()->getBaseMap()->getMapInfo()->isNonInstanceMap())
            iid = 0;

        WoWGuid wowGuid;
        wowGuid.Init(getCreature()->getGuid());

        LuaGlobal::instance()->m_onLoadInfo.push_back(getCreature()->GetMapId());
        LuaGlobal::instance()->m_onLoadInfo.push_back(iid);
        LuaGlobal::instance()->m_onLoadInfo.push_back(wowGuid.getGuidLowPart());
    }

    void OnReachWP(uint32_t iWaypointId, bool bForwards)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_REACH_WP]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_REACH_WP);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(iWaypointId);
        LuaGlobal::instance()->luaEngine()->PUSH_BOOL(bForwards);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnLootTaken(Player* pPlayer, ItemProperties const* pItemPrototype)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_LOOT_TAKEN]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_LOOT_TAKEN);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(pItemPrototype->ItemId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void AIUpdate()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_AIUPDATE]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_AIUPDATE);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    void OnEmote(Player* pPlayer, EmoteType Emote)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_EMOTE]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_EMOTE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_INT((int32_t)Emote);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_DAMAGE_TAKEN]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PUSH_INT(CREATURE_EVENT_ON_DAMAGE_TAKEN);
        LuaGlobal::instance()->luaEngine()->PushUnit(mAttacker);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(fAmount);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnEnterVehicle()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_ENTER_VEHICLE]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK;
    }

    void OnExitVehicle()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_EXIT_VEHICLE]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK;
    }

    void OnFirstPassengerEntered(Unit* passenger)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_FIRST_PASSENGER_ENTERED]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PushUnit(passenger);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK;
    }

    void OnVehicleFull()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_VEHICLE_FULL]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK;
    }

    void OnLastPassengerLeft(Unit* passenger)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[CREATURE_EVENT_ON_LAST_PASSENGER_LEFT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->PushUnit(passenger);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK;
    }

    void StringFunctionCall(int fRef)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(static_cast<uint16_t>(fRef));
        LuaGlobal::instance()->luaEngine()->PushUnit(getCreature());
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void Destroy()
    {
        {
            typedef std::multimap<uint32_t, LuaCreature*> CMAP;
            CMAP& cMap = LuaGlobal::instance()->luaEngine()->getLuCreatureMap();
            auto itr = cMap.find(getCreature()->getEntry());
            auto itend = cMap.upper_bound(getCreature()->getEntry());
            CMAP::iterator it;
            for (; itr != cMap.end() && itr != itend;)
            {
                it = itr++;
                if (it->second != nullptr && it->second == this)
                    cMap.erase(it);
            }
        }

        {
            //Function Ref clean up
            std::map< uint64_t, std::set<int> >& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
            auto itr = objRefs.find(getCreature()->getGuid());
            if (itr != objRefs.end())
            {
                std::set<int>& refs = itr->second;
                for (auto it = refs.begin(); it != refs.end(); ++it)
                {
                    luaL_unref(LuaGlobal::instance()->luaEngine()->getluState(), LUA_REGISTRYINDEX, (*it));
                    sEventMgr.RemoveEvents(getCreature(), (*it) + EVENT_LUA_CREATURE_EVENTS);
                }
                refs.clear();
            }
        }
        delete this;
    }
    LuaObjectBinding* m_binding;
};

class LuaGameObjectScript : public GameObjectAIScript
{
public:
    explicit LuaGameObjectScript(GameObject* go) : GameObjectAIScript(go), m_binding(nullptr) {}
    ~LuaGameObjectScript() {}

    GameObject* getGO()
    {
        return _gameobject;
    }

    void OnCreate()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_CREATE]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void OnSpawn()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_SPAWN]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void OnDespawn()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_DESPAWN]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void OnLootTaken(Player* pLooter, ItemProperties const* pItemInfo)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_LOOT_TAKEN]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(GAMEOBJECT_EVENT_ON_LOOT_TAKEN);
        LuaGlobal::instance()->luaEngine()->PushUnit(pLooter);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(pItemInfo->ItemId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    void OnActivate(Player* pPlayer)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_USE]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(GAMEOBJECT_EVENT_ON_USE);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void AIUpdate()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_AIUPDATE]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void OnDamaged(uint32_t damage)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_DAMAGED]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(damage);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK;
    }

    void OnDestroyed()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[GAMEOBJECT_EVENT_ON_DESTROYED]);
        LuaGlobal::instance()->luaEngine()->PushGo(_gameobject);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK;
    }

    void Destroy()
    {
        typedef std::multimap<uint32_t, LuaGameObjectScript*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getLuGameObjectMap();
        auto itr = gMap.find(_gameobject->getEntry());
        auto itend = gMap.upper_bound(_gameobject->getEntry());
        GMAP::iterator it;

        //uint64_t guid = _gameobject->getGuid(); Unused?
        for (; itr != itend;)
        {
            it = itr++;
            if (it->second != nullptr && it->second == this)
                gMap.erase(it);
        }

        std::map< uint64_t, std::set<int> >& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
        auto itr2 = objRefs.find(_gameobject->getGuid());
        if (itr2 != objRefs.end())
        {
            std::set<int>& refs = itr2->second;
            for (std::set<int>::iterator it2 = refs.begin(); it2 != refs.end(); ++it2)
                luaL_unref(LuaGlobal::instance()->luaEngine()->getluState(), LUA_REGISTRYINDEX, (*it2));

            refs.clear();
        }
        delete this;
    }
    LuaObjectBinding* m_binding;
};

class LuaGossip : public GossipScript
{
public:
    LuaGossip() : GossipScript(), m_unit_gossip_binding(nullptr), m_item_gossip_binding(nullptr), m_go_gossip_binding(nullptr) {}
    ~LuaGossip()
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> MapType;
        MapType gMap;
        if (this->m_go_gossip_binding != nullptr)
        {
            gMap = LuaGlobal::instance()->luaEngine()->getGameObjectGossipInterfaceMap();
            for (auto itr = gMap.begin(); itr != gMap.end(); ++itr)
            {
                if (itr->second == this)
                {
                    gMap.erase(itr);
                    break;
                }
            }
        }
        else if (this->m_unit_gossip_binding != nullptr)
        {
            gMap = LuaGlobal::instance()->luaEngine()->getUnitGossipInterfaceMap();
            for (auto itr = gMap.begin(); itr != gMap.end(); ++itr)
            {
                if (itr->second == this)
                {
                    gMap.erase(itr);
                    break;
                }
            }
        }
        else if (this->m_item_gossip_binding != nullptr)
        {
            gMap = LuaGlobal::instance()->luaEngine()->getItemGossipInterfaceMap();
            for (auto itr = gMap.begin(); itr != gMap.end(); ++itr)
            {
                if (itr->second == this)
                {
                    gMap.erase(itr);
                    break;
                }
            }
        }
    }

    void onHello(Object* pObject, Player* plr) override
    {
        GET_LOCK

        if (pObject->isCreature())
        {
            if (m_unit_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }

            LuaGlobal::instance()->luaEngine()->BeginCall(m_unit_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_TALK]);
            LuaGlobal::instance()->luaEngine()->PushUnit(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_TALK);
            LuaGlobal::instance()->luaEngine()->PushUnit(plr);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
        }
        else if (pObject->isItem())
        {
            if (m_item_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }

            LuaGlobal::instance()->luaEngine()->BeginCall(m_item_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_TALK]);
            LuaGlobal::instance()->luaEngine()->PushItem(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_TALK);
            LuaGlobal::instance()->luaEngine()->PushUnit(plr);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
        }
        else if (pObject->isGameObject())
        {
            if (m_go_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }

            LuaGlobal::instance()->luaEngine()->BeginCall(m_go_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_TALK]);
            LuaGlobal::instance()->luaEngine()->PushGo(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_TALK);
            LuaGlobal::instance()->luaEngine()->PushUnit(plr);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
        }

        RELEASE_LOCK
    }

    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* EnteredCode, uint32_t /*gossipId*/) override
    {
        GET_LOCK

        if (pObject->isCreature())
        {
            if (m_unit_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }

            LuaGlobal::instance()->luaEngine()->BeginCall(m_unit_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_SELECT_OPTION]);
            LuaGlobal::instance()->luaEngine()->PushUnit(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_SELECT_OPTION);
            LuaGlobal::instance()->luaEngine()->PushUnit(Plr);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(Id);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(Id); // used to be IntId
            LuaGlobal::instance()->luaEngine()->PUSH_STRING(EnteredCode);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(6);
        }
        else if (pObject->isItem())
        {
            if (m_item_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }
            LuaGlobal::instance()->luaEngine()->BeginCall(m_item_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_SELECT_OPTION]);
            LuaGlobal::instance()->luaEngine()->PushItem(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_SELECT_OPTION);
            LuaGlobal::instance()->luaEngine()->PushUnit(Plr);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(Id);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(Id); // used to be IntId
            LuaGlobal::instance()->luaEngine()->PUSH_STRING(EnteredCode);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(6);
        }
        else if (pObject->isGameObject())
        {
            if (m_go_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }
            LuaGlobal::instance()->luaEngine()->BeginCall(m_go_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_SELECT_OPTION]);
            LuaGlobal::instance()->luaEngine()->PushGo(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_SELECT_OPTION);
            LuaGlobal::instance()->luaEngine()->PushUnit(Plr);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(Id);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(Id); // used to be IntId
            LuaGlobal::instance()->luaEngine()->PUSH_STRING(EnteredCode);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(6);
        }

        RELEASE_LOCK
    }

    void onEnd(Object* pObject, Player* Plr) override
    {
        GET_LOCK

        if (pObject->isCreature())
        {
            if (m_unit_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }
            LuaGlobal::instance()->luaEngine()->BeginCall(m_unit_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_END]);
            LuaGlobal::instance()->luaEngine()->PushUnit(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_END);
            LuaGlobal::instance()->luaEngine()->PushUnit(Plr);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
        }
        else if (pObject->isItem())
        {
            if (m_item_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }
            LuaGlobal::instance()->luaEngine()->BeginCall(m_item_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_END]);
            LuaGlobal::instance()->luaEngine()->PushItem(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_END);
            LuaGlobal::instance()->luaEngine()->PushUnit(Plr);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
        }
        else if (pObject->isGameObject())
        {
            if (m_go_gossip_binding == nullptr)
            {
                RELEASE_LOCK;
                return;
            }
            LuaGlobal::instance()->luaEngine()->BeginCall(m_go_gossip_binding->m_functionReferences[GOSSIP_EVENT_ON_END]);
            LuaGlobal::instance()->luaEngine()->PushGo(pObject);
            LuaGlobal::instance()->luaEngine()->PUSH_UINT(GOSSIP_EVENT_ON_END);
            LuaGlobal::instance()->luaEngine()->PushUnit(Plr);
            LuaGlobal::instance()->luaEngine()->ExecuteCall(3);
        }

        RELEASE_LOCK
    }

    LuaObjectBinding* m_unit_gossip_binding;
    LuaObjectBinding* m_item_gossip_binding;
    LuaObjectBinding* m_go_gossip_binding;
};

class LuaQuest : public QuestScript
{
public:
    LuaQuest() : QuestScript()
    {
        m_binding = nullptr;
    }

    ~LuaQuest()
    {
        typedef std::unordered_map<uint32_t, LuaQuest*> QuestType;
        QuestType qMap = LuaGlobal::instance()->luaEngine()->getLuQuestMap();
        for (auto itr = qMap.begin(); itr != qMap.end(); ++itr)
        {
            if (itr->second == this)
            {
                qMap.erase(itr);
                break;
            }
        }
    }

    void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_ON_ACCEPT]);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(qLogEntry->getQuestProperties()->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    void OnQuestComplete(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_ON_COMPLETE]);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(qLogEntry->getQuestProperties()->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    void OnQuestCancel(Player* mTarget)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_ON_CANCEL]);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void OnGameObjectActivate(uint32_t entry, Player* mTarget, QuestLogEntry* qLogEntry)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_GAMEOBJECT_ACTIVATE]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(entry);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(qLogEntry->getQuestProperties()->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnCreatureKill(uint32_t entry, Player* mTarget, QuestLogEntry* qLogEntry)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_ON_CREATURE_KILL]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(entry);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(qLogEntry->getQuestProperties()->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnExploreArea(uint32_t areaId, Player* mTarget, QuestLogEntry* qLogEntry)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_ON_EXPLORE_AREA]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(areaId);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(qLogEntry->getQuestProperties()->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnPlayerItemPickup(uint32_t itemId, uint32_t totalCount, Player* mTarget, QuestLogEntry* qLogEntry)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[QUEST_EVENT_ON_PLAYER_ITEMPICKUP]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(itemId);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(totalCount);
        LuaGlobal::instance()->luaEngine()->PushUnit(mTarget);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(qLogEntry->getQuestProperties()->id);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }
    LuaObjectBinding* m_binding;
};

class LuaInstance : public InstanceScript
{
public:
    explicit LuaInstance(WorldMap* pMapMgr) : InstanceScript(pMapMgr), m_instanceId(pMapMgr->getInstanceId()), m_binding(nullptr) {}
    ~LuaInstance() {}

    // Player
    void OnPlayerDeath(Player* pVictim, Unit* pKiller)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_PLAYER_DEATH]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushUnit(pVictim);
        LuaGlobal::instance()->luaEngine()->PushUnit(pKiller);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnPlayerEnter(Player* pPlayer)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_PLAYER_ENTER]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    void OnAreaTrigger(Player* pPlayer, uint32_t uAreaId)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_AREA_TRIGGER]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(uAreaId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnZoneChange(Player* pPlayer, uint32_t uNewZone, uint32_t uOldZone)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_ZONE_CHANGE]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(uNewZone);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(uOldZone);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(4);

        RELEASE_LOCK
    }

    // Creature / GameObject - part of it is simple reimplementation for easier use Creature / GO < --- > Script
    void OnCreatureDeath(Creature* pVictim, Unit* pKiller)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_CREATURE_DEATH]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushUnit(pVictim);
        LuaGlobal::instance()->luaEngine()->PushUnit(pKiller);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnCreaturePushToWorld(Creature* pCreature)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_CREATURE_PUSH]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushUnit(pCreature);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_GO_ACTIVATE]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushGo(pGameObject);
        LuaGlobal::instance()->luaEngine()->PushUnit(pPlayer);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(3);

        RELEASE_LOCK
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject)
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ON_GO_PUSH]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->PushGo(pGameObject);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(2);

        RELEASE_LOCK
    }

    // Standard virtual methods
    void OnLoad()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_ONLOAD]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK
    }

    void Destroy()
    {
        CHECK_BINDING_ACQUIRELOCK

        LuaGlobal::instance()->luaEngine()->BeginCall(m_binding->m_functionReferences[INSTANCE_EVENT_DESTROY]);
        LuaGlobal::instance()->luaEngine()->PUSH_UINT(m_instanceId);
        LuaGlobal::instance()->luaEngine()->ExecuteCall(1);

        RELEASE_LOCK

        typedef std::unordered_map<uint32_t, LuaInstance*> IMAP;
        IMAP& iMap = LuaGlobal::instance()->luaEngine()->getLuInstanceMap();
        for (auto itr = iMap.begin(); itr != iMap.end(); ++itr)
        {
            if (itr->second == this)
            {
                iMap.erase(itr);
                break;
            }
        }
        delete this;
    }

    uint32_t m_instanceId;
    LuaObjectBinding* m_binding;
};

CreatureAIScript* CreateLuaCreature(Creature* src)
{
    LuaCreature* script = nullptr;
    if (src != nullptr)
    {
        uint32_t id = src->getEntry();
        LuaObjectBinding* pBinding = LuaGlobal::instance()->luaEngine()->getUnitBinding(id);
        if (pBinding != nullptr)
        {
            typedef std::multimap<uint32_t, LuaCreature*> CRCMAP;
            CRCMAP& cMap = LuaGlobal::instance()->luaEngine()->getLuCreatureMap();
            script = new LuaCreature(src);
            cMap.emplace(std::make_pair(id, script));
            script->m_binding = pBinding;
        }
    }
    return script;
}

GameObjectAIScript* CreateLuaGameObjectScript(GameObject* src)
{
    LuaGameObjectScript* script = nullptr;
    if (src != nullptr)
    {
        uint32_t id = src->GetGameObjectProperties()->entry;
        LuaObjectBinding* pBinding = nullptr;
        pBinding = LuaGlobal::instance()->luaEngine()->getGameObjectBinding(id);
        if (pBinding != nullptr)
        {
            typedef std::multimap<uint32_t, LuaGameObjectScript*> GMAP;
            GMAP& gMap = LuaGlobal::instance()->luaEngine()->getLuGameObjectMap();
            script = new LuaGameObjectScript(src);
            gMap.emplace(std::make_pair(id, script));
            script->m_binding = pBinding;
        }
    }
    return script;
}

QuestScript* CreateLuaQuestScript(uint32_t id)
{
    LuaQuest* pLua = nullptr;
    LuaObjectBinding* pBinding = LuaGlobal::instance()->luaEngine()->getQuestBinding(id);
    if (pBinding != nullptr)
    {
        typedef std::unordered_map<uint32_t, LuaQuest*> QMAP;
        QMAP& qMap = LuaGlobal::instance()->luaEngine()->getLuQuestMap();
        QMAP::iterator itr = qMap.find(id);
        if (itr != qMap.end())
        {
            if (itr->second == nullptr)
                pLua = itr->second = new LuaQuest();
            else
                pLua = itr->second;
        }
        else
        {
            pLua = new LuaQuest();
            qMap.insert(std::make_pair(id, pLua));
        }
        pLua->m_binding = pBinding;
    }
    return pLua;
}

InstanceScript* CreateLuaInstance(WorldMap* pMapMgr)
{
    LuaInstance* pLua = nullptr;
    uint32_t id = pMapMgr->getBaseMap()->getMapId();
    LuaObjectBinding* pBinding = LuaGlobal::instance()->luaEngine()->getInstanceBinding(id);
    if (pBinding != nullptr)
    {
        typedef std::unordered_map<uint32_t, LuaInstance*> IMAP;
        IMAP& iMap = LuaGlobal::instance()->luaEngine()->getLuInstanceMap();
        auto itr = iMap.find(id);
        if (itr != iMap.end())
        {
            if (itr->second == nullptr)
                pLua = itr->second = new LuaInstance(pMapMgr);
            else
                pLua = itr->second;
        }
        else
        {
            pLua = new LuaInstance(pMapMgr);
            iMap.insert(std::make_pair(id, pLua));
        }
        pLua->m_binding = pBinding;
    }
    return pLua;
}

GossipScript* CreateLuaUnitGossipScript(uint32_t id)
{
    LuaGossip* pLua = nullptr;
    LuaObjectBinding* pBinding = LuaGlobal::instance()->luaEngine()->getLuaUnitGossipBinding(id);
    if (pBinding != nullptr)
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getUnitGossipInterfaceMap();
        auto itr = gMap.find(id);
        if (itr != gMap.end())
        {
            if (itr->second == nullptr)
                pLua = itr->second = new LuaGossip();
            else
                pLua = itr->second;
        }
        else
        {
            pLua = new LuaGossip();
            gMap.insert(std::make_pair(id, pLua));
        }
        pLua->m_unit_gossip_binding = pBinding;
    }
    return pLua;
}
GossipScript* CreateLuaItemGossipScript(uint32_t id)
{
    LuaGossip* pLua = nullptr;
    LuaObjectBinding* pBinding = LuaGlobal::instance()->luaEngine()->getLuaItemGossipBinding(id);
    if (pBinding != nullptr)
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getItemGossipInterfaceMap();
        auto itr = gMap.find(id);
        if (itr != gMap.end())
        {
            if (itr->second == nullptr)
                pLua = itr->second = new LuaGossip();
            else
                pLua = itr->second;
        }
        else
        {
            pLua = new LuaGossip();
            gMap.insert(std::make_pair(id, pLua));

        }
        pLua->m_item_gossip_binding = pBinding;
    }
    return pLua;
}

GossipScript* CreateLuaGOGossipScript(uint32_t id)
{
    LuaGossip* pLua = nullptr;
    LuaObjectBinding* pBinding = LuaGlobal::instance()->luaEngine()->getLuaGOGossipBinding(id);
    if (pBinding != nullptr)
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getGameObjectGossipInterfaceMap();
        auto itr = gMap.find(id);
        if (itr != gMap.end())
        {
            if (itr->second == nullptr)
                pLua = itr->second = new LuaGossip();
            else
                pLua = itr->second;
        }
        else
        {
            pLua = new LuaGossip();
            gMap.insert(std::make_pair(id, pLua));
        }
        pLua->m_go_gossip_binding = pBinding;
    }
    return pLua;
}

void LuaEngine::Startup()
{
    DLLLogDetail("LuaEngineMgr : Loaded ALE (AscEmu Lua Engine)");
    //Create a new global state that will server as the lua universe.
    lu = luaL_newstate();

    LoadScripts();

    // stuff is registered, so lets go ahead and make our emulated C++ scripted lua classes.
    for (auto& itr : m_unitBinding)
    {
        m_scriptMgr->register_creature_script(itr.first, CreateLuaCreature);
        LuaGlobal::instance()->luaEngine()->getLuCreatureMap().emplace(std::make_pair(itr.first, (LuaCreature*)nullptr));
    }

    for (auto& itr : m_gameobjectBinding)
    {
        m_scriptMgr->register_gameobject_script(itr.first, CreateLuaGameObjectScript);
        LuaGlobal::instance()->luaEngine()->getLuGameObjectMap().emplace(std::make_pair(itr.first, (LuaGameObjectScript*)nullptr));
    }

    for (auto& itr : m_questBinding)
    {
        QuestScript* qs = CreateLuaQuestScript(itr.first);
        if (qs != nullptr)
        {
            m_scriptMgr->register_quest_script(itr.first, qs);
            LuaGlobal::instance()->luaEngine()->getLuQuestMap().insert(std::make_pair(itr.first, (LuaQuest*)nullptr));
        }
    }

    for (auto& itr : m_instanceBinding)
    {
        m_scriptMgr->register_instance_script(itr.first, CreateLuaInstance);
        LuaGlobal::instance()->luaEngine()->getLuInstanceMap().insert(std::make_pair(itr.first, (LuaInstance*)nullptr));
    }

    for (auto& itr : m_unit_gossipBinding)
    {
        GossipScript* gs = CreateLuaUnitGossipScript(itr.first);
        if (gs != nullptr)
        {
            m_scriptMgr->register_creature_gossip(itr.first, gs);
            LuaGlobal::instance()->luaEngine()->getUnitGossipInterfaceMap().insert(std::make_pair(itr.first, (LuaGossip*)nullptr));
        }
    }

    for (auto& itr : m_item_gossipBinding)
    {
        GossipScript* gs = CreateLuaItemGossipScript(itr.first);
        if (gs != nullptr)
        {
            m_scriptMgr->register_item_gossip(itr.first, gs);
            LuaGlobal::instance()->luaEngine()->getItemGossipInterfaceMap().insert(std::make_pair(itr.first, (LuaGossip*)nullptr));
        }
    }

    for (auto& itr : m_go_gossipBinding)
    {
        GossipScript* gs = CreateLuaGOGossipScript(itr.first);
        if (gs != nullptr)
        {
            m_scriptMgr->register_go_gossip(itr.first, gs);
            LuaGlobal::instance()->luaEngine()->getGameObjectGossipInterfaceMap().insert(std::make_pair(itr.first, (LuaGossip*)nullptr));
        }
    }

    //big server hook chunk. it only hooks if there are functions present to save on unnecessary processing.

    RegisterHook(SERVER_HOOK_EVENT_ON_NEW_CHARACTER, (void*)LuaHookOnNewCharacter)
    RegisterHook(SERVER_HOOK_EVENT_ON_KILL_PLAYER, (void*)LuaHookOnKillPlayer)
    RegisterHook(SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD, (void*)LuaHookOnFirstEnterWorld)
    RegisterHook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)LuaHookOnEnterWorld)
    RegisterHook(SERVER_HOOK_EVENT_ON_GUILD_JOIN, (void*)LuaHookOnGuildJoin)
    RegisterHook(SERVER_HOOK_EVENT_ON_DEATH, (void*)LuaHookOnDeath)
    RegisterHook(SERVER_HOOK_EVENT_ON_REPOP, (void*)LuaHookOnRepop)
    RegisterHook(SERVER_HOOK_EVENT_ON_EMOTE, (void*)LuaHookOnEmote)
    RegisterHook(SERVER_HOOK_EVENT_ON_ENTER_COMBAT, (void*)LuaHookOnEnterCombat)
    RegisterHook(SERVER_HOOK_EVENT_ON_CAST_SPELL, (void*)LuaHookOnCastSpell)
    RegisterHook(SERVER_HOOK_EVENT_ON_TICK, (void*)LuaHookOnTick)
    RegisterHook(SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST, (void*)LuaHookOnLogoutRequest)
    RegisterHook(SERVER_HOOK_EVENT_ON_LOGOUT, (void*)LuaHookOnLogout)
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT, (void*)LuaHookOnQuestAccept)
    RegisterHook(SERVER_HOOK_EVENT_ON_ZONE, (void*)LuaHookOnZone)
    RegisterHook(SERVER_HOOK_EVENT_ON_CHAT, (void*)LuaHookOnChat)
    RegisterHook(SERVER_HOOK_EVENT_ON_LOOT, (void*)LuaHookOnLoot)
    RegisterHook(SERVER_HOOK_EVENT_ON_GUILD_CREATE, (void*)LuaHookOnGuildCreate)
    RegisterHook(SERVER_HOOK_EVENT_ON_FULL_LOGIN, (void*)LuaHookOnEnterWorld2)
    RegisterHook(SERVER_HOOK_EVENT_ON_CHARACTER_CREATE, (void*)LuaHookOnCharacterCreate)
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED, (void*)LuaHookOnQuestCancelled)
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_FINISHED, (void*)LuaHookOnQuestFinished)
    RegisterHook(SERVER_HOOK_EVENT_ON_HONORABLE_KILL, (void*)LuaHookOnHonorableKill)
    RegisterHook(SERVER_HOOK_EVENT_ON_ARENA_FINISH, (void*)LuaHookOnArenaFinish)
    RegisterHook(SERVER_HOOK_EVENT_ON_OBJECTLOOT, (void*)LuaHookOnObjectLoot)
    RegisterHook(SERVER_HOOK_EVENT_ON_AREATRIGGER, (void*)LuaHookOnAreaTrigger)
    RegisterHook(SERVER_HOOK_EVENT_ON_POST_LEVELUP, (void*)LuaHookOnPostLevelUp)
    RegisterHook(SERVER_HOOK_EVENT_ON_PRE_DIE, (void*)LuaHookOnPreUnitDie)
    RegisterHook(SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE, (void*)LuaHookOnAdvanceSkillLine)
    RegisterHook(SERVER_HOOK_EVENT_ON_DUEL_FINISHED, (void*)LuaHookOnDuelFinished)
    RegisterHook(SERVER_HOOK_EVENT_ON_AURA_REMOVE, (void*)LuaHookOnAuraRemove)
    RegisterHook(SERVER_HOOK_EVENT_ON_RESURRECT, (void*)LuaHookOnResurrect)

    for (const auto& dummySpell : LuaGlobal::instance()->m_luaDummySpells)
    {
        auto dummyHook = LuaGlobal::instance()->luaEngine()->HookInfo.dummyHooks;
        if (std::find(dummyHook.begin(), dummyHook.end(), dummySpell.first) == dummyHook.end())
        {
            m_scriptMgr->register_dummy_spell(dummySpell.first, &LuaOnDummySpell);
            LuaGlobal::instance()->luaEngine()->HookInfo.dummyHooks.push_back(dummySpell.first);
        }
    }
}
void LuaEngine::RegisterEvent(uint8_t regtype, uint32_t id, uint32_t evt, uint16_t functionRef)
{
    switch (regtype)
    {
        case REGTYPE_UNIT:
        {
            if (id && evt && evt < CREATURE_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getUnitBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_unitBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
        case REGTYPE_GO:
        {
            if (id && evt && evt < GAMEOBJECT_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getGameObjectBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_gameobjectBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
        case REGTYPE_QUEST:
        {
            if (id && evt && evt < QUEST_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getQuestBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_questBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
        case REGTYPE_SERVHOOK:
        {
            if (evt < NUM_SERVER_HOOKS)
                LuaGlobal::instance()->EventAsToFuncName[evt].push_back(functionRef);
        }
        break;
        case REGTYPE_DUMMYSPELL:
        {
            if (id)
                LuaGlobal::instance()->m_luaDummySpells.insert(std::pair<uint32_t, uint16_t>(id, functionRef));
        }
        break;
        case REGTYPE_INSTANCE:
        {
            if (id && evt && evt < INSTANCE_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getInstanceBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_instanceBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
        case REGTYPE_UNIT_GOSSIP:
        {
            if (id && evt && evt < GOSSIP_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getLuaUnitGossipBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_unit_gossipBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
        case REGTYPE_ITEM_GOSSIP:
        {
            if (id && evt && evt < GOSSIP_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getLuaItemGossipBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_item_gossipBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
        case REGTYPE_GO_GOSSIP:
        {
            if (id && evt && evt < GOSSIP_EVENT_COUNT)
            {
                LuaObjectBinding* bind = getLuaGOGossipBinding(id);
                if (bind == nullptr)
                {
                    LuaObjectBinding nbind;
                    memset(&nbind, 0, sizeof(LuaObjectBinding));
                    nbind.m_functionReferences[evt] = functionRef;
                    m_go_gossipBinding.insert(std::make_pair(id, nbind));
                }
                else
                {
                    if (bind->m_functionReferences[evt] > 0)
                        luaL_unref(lu, LUA_REGISTRYINDEX, bind->m_functionReferences[evt]);

                    bind->m_functionReferences[evt] = functionRef;
                }
            }
        }
        break;
    }
}

void LuaEngine::Unload()
{
    RemoveTimedEvents(lu);

    DestroyAllLuaEvents(); // stop all pending events.

    // clean up the engine of any existing defined variables
    for (auto& itr : m_unitBinding)
    {
        for (int i = 0; i < CREATURE_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_unitBinding.clear();

    for (auto& itr : m_gameobjectBinding)
    {
        for (int i = 0; i < GAMEOBJECT_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_gameobjectBinding.clear();

    for (auto& itr : m_questBinding)
    {
        for (int i = 0; i < QUEST_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_questBinding.clear();

    for (auto& itr : m_instanceBinding)
    {
        for (int i = 0; i < INSTANCE_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_instanceBinding.clear();

    for (auto& itr : m_unit_gossipBinding)
    {
        for (int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_unit_gossipBinding.clear();

    for (auto& itr : m_item_gossipBinding)
    {
        for (int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_item_gossipBinding.clear();

    for (auto& itr : m_go_gossipBinding)
    {
        for (int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
            if (itr.second.m_functionReferences[i] > 0)
                luaL_unref(lu, LUA_REGISTRYINDEX, itr.second.m_functionReferences[i]);
    }
    m_go_gossipBinding.clear();

    //Serv hooks : had forgotten these.
    for (auto& next : LuaGlobal::instance()->EventAsToFuncName)
    {
        for (auto itr = next.begin(); itr != next.end(); ++itr)
            luaL_unref(lu, LUA_REGISTRYINDEX, (*itr));

        next.clear();
    }

    for (auto& m_luaDummySpell : LuaGlobal::instance()->m_luaDummySpells)
    {
        luaL_unref(lu, LUA_REGISTRYINDEX, m_luaDummySpell.second);
    }
    LuaGlobal::instance()->m_luaDummySpells.clear();

    for (auto itr : m_pendingThreads)
    {
        luaL_unref(lu, LUA_REGISTRYINDEX, itr);
    }
    m_pendingThreads.clear();
    m_functionRefs.clear();

    lua_close(lu);
}

void LuaEngine::Restart()
{
    DLLLogDetail("LuaEngineMgr : Restarting Engine.");
    GET_LOCK
    getcoLock().Acquire();
    Unload();
    lu = luaL_newstate();
    LoadScripts();
    for (auto& itr : m_unitBinding)
    {
        typedef std::multimap<uint32_t, LuaCreature*> CMAP;
        CMAP& cMap = LuaGlobal::instance()->luaEngine()->getLuCreatureMap();
        auto it = cMap.find(itr.first);
        auto itend = cMap.upper_bound(itr.first);
        if (it == cMap.end())
        {
            m_scriptMgr->register_creature_script(itr.first, CreateLuaCreature);
            cMap.emplace(std::make_pair(itr.first, (LuaCreature*)nullptr));
        }
        else
        {
            for (; it != itend; ++it)
                if (it->second != nullptr)
                    it->second->m_binding = &itr.second;
        }
    }

    for (auto& itr : m_gameobjectBinding)
    {
        typedef std::multimap<uint32_t, LuaGameObjectScript*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getLuGameObjectMap();
        auto it = gMap.find(itr.first);
        auto itend = gMap.upper_bound(itr.first);
        if (it == gMap.end())
        {
            m_scriptMgr->register_gameobject_script(itr.first, CreateLuaGameObjectScript);
            gMap.emplace(std::make_pair(itr.first, (LuaGameObjectScript*)nullptr));
        }
        else
        {
            for (; it != itend; ++it)
                if (it->second != nullptr)
                    it->second->m_binding = &itr.second;
        }
    }

    for (auto& itr : m_questBinding)
    {
        typedef std::unordered_map<uint32_t, LuaQuest*> QMAP;
        QMAP& qMap = LuaGlobal::instance()->luaEngine()->getLuQuestMap();
        auto it = qMap.find(itr.first);
        if (it == qMap.end())
        {
            m_scriptMgr->register_quest_script(itr.first, CreateLuaQuestScript(itr.first));
            qMap.insert(std::make_pair(itr.first, (LuaQuest*)nullptr));
        }
        else
        {
            LuaQuest* q_interface = it->second;
            if (q_interface != nullptr)
                q_interface->m_binding = &itr.second;
        }
    }

    for (auto& itr : m_instanceBinding)
    {
        typedef std::unordered_map<uint32_t, LuaInstance*> IMAP;
        IMAP& iMap = LuaGlobal::instance()->luaEngine()->getLuInstanceMap();
        auto it = iMap.find(itr.first);
        if (it == iMap.end())
        {
            m_scriptMgr->register_instance_script(itr.first, CreateLuaInstance);
            iMap.insert(std::make_pair(itr.first, (LuaInstance*)nullptr));
        }
        else
        {
            if (it->second != nullptr)
                it->second->m_binding = &itr.second;
        }
    }

    for (auto itr = m_unit_gossipBinding.begin(); itr != m_unit_gossipBinding.end(); ++itr)
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getUnitGossipInterfaceMap();
        auto it = gMap.find(itr->first);
        if (it == gMap.end())
        {
            GossipScript* gs = CreateLuaUnitGossipScript(itr->first);
            if (gs != nullptr)
            {
                m_scriptMgr->register_creature_gossip(itr->first, gs);
                gMap.insert(std::make_pair(itr->first, (LuaGossip*)nullptr));
            }
        }
        else
        {
            LuaGossip* u_gossip = it->second;
            if (u_gossip != nullptr)
                u_gossip->m_unit_gossip_binding = &itr->second;
        }
    }

    for (auto itr = m_item_gossipBinding.begin(); itr != m_item_gossipBinding.end(); ++itr)
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getItemGossipInterfaceMap();
        auto it = gMap.find(itr->first);
        if (it == gMap.end())
        {
            GossipScript* gs = CreateLuaItemGossipScript(itr->first);
            if (gs != nullptr)
            {
                m_scriptMgr->register_item_gossip(itr->first, gs);
                gMap.insert(std::make_pair(itr->first, (LuaGossip*)nullptr));
            }
        }
        else
        {
            LuaGossip* i_gossip = it->second;
            if (i_gossip != nullptr)
                i_gossip->m_item_gossip_binding = &itr->second;
        }
    }

    for (auto itr = m_go_gossipBinding.begin(); itr != m_go_gossipBinding.end(); ++itr)
    {
        typedef std::unordered_map<uint32_t, LuaGossip*> GMAP;
        GMAP& gMap = LuaGlobal::instance()->luaEngine()->getGameObjectGossipInterfaceMap();
        auto it = gMap.find(itr->first);
        if (it == gMap.end())
        {
            GossipScript* gs = CreateLuaGOGossipScript(itr->first);
            if (gs != nullptr)
            {
                m_scriptMgr->register_go_gossip(itr->first, gs);
                gMap.insert(std::make_pair(itr->first, (LuaGossip*)nullptr));
            }
        }
        else
        {
            LuaGossip* g_gossip = it->second;
            if (g_gossip != nullptr)
                g_gossip->m_go_gossip_binding = &itr->second;
        }
    }

    /*
    BIG SERV HOOK CHUNK EEK
    */
    RegisterHook(SERVER_HOOK_EVENT_ON_NEW_CHARACTER, (void*)LuaHookOnNewCharacter)
    RegisterHook(SERVER_HOOK_EVENT_ON_KILL_PLAYER, (void*)LuaHookOnKillPlayer)
    RegisterHook(SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD, (void*)LuaHookOnFirstEnterWorld)
    RegisterHook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)LuaHookOnEnterWorld)
    RegisterHook(SERVER_HOOK_EVENT_ON_GUILD_JOIN, (void*)LuaHookOnGuildJoin)
    RegisterHook(SERVER_HOOK_EVENT_ON_DEATH, (void*)LuaHookOnDeath)
    RegisterHook(SERVER_HOOK_EVENT_ON_REPOP, (void*)LuaHookOnRepop)
    RegisterHook(SERVER_HOOK_EVENT_ON_EMOTE, (void*)LuaHookOnEmote)
    RegisterHook(SERVER_HOOK_EVENT_ON_ENTER_COMBAT, (void*)LuaHookOnEnterCombat)
    RegisterHook(SERVER_HOOK_EVENT_ON_CAST_SPELL, (void*)LuaHookOnCastSpell)
    RegisterHook(SERVER_HOOK_EVENT_ON_TICK, (void*)LuaHookOnTick)
    RegisterHook(SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST, (void*)LuaHookOnLogoutRequest)
    RegisterHook(SERVER_HOOK_EVENT_ON_LOGOUT, (void*)LuaHookOnLogout)
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT, (void*)LuaHookOnQuestAccept)
    RegisterHook(SERVER_HOOK_EVENT_ON_ZONE, (void*)LuaHookOnZone)
    RegisterHook(SERVER_HOOK_EVENT_ON_CHAT, (void*)LuaHookOnChat)
    RegisterHook(SERVER_HOOK_EVENT_ON_LOOT, (void*)LuaHookOnLoot)
    RegisterHook(SERVER_HOOK_EVENT_ON_GUILD_CREATE, (void*)LuaHookOnGuildCreate)
    RegisterHook(SERVER_HOOK_EVENT_ON_FULL_LOGIN, (void*)LuaHookOnEnterWorld2)
    RegisterHook(SERVER_HOOK_EVENT_ON_CHARACTER_CREATE, (void*)LuaHookOnCharacterCreate)
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED, (void*)LuaHookOnQuestCancelled)
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_FINISHED, (void*)LuaHookOnQuestFinished)
    RegisterHook(SERVER_HOOK_EVENT_ON_HONORABLE_KILL, (void*)LuaHookOnHonorableKill)
    RegisterHook(SERVER_HOOK_EVENT_ON_ARENA_FINISH, (void*)LuaHookOnArenaFinish)
    RegisterHook(SERVER_HOOK_EVENT_ON_OBJECTLOOT, (void*)LuaHookOnObjectLoot)
    RegisterHook(SERVER_HOOK_EVENT_ON_AREATRIGGER, (void*)LuaHookOnAreaTrigger)
    RegisterHook(SERVER_HOOK_EVENT_ON_POST_LEVELUP, (void*)LuaHookOnPostLevelUp)
    RegisterHook(SERVER_HOOK_EVENT_ON_PRE_DIE, (void*)LuaHookOnPreUnitDie)
    RegisterHook(SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE, (void*)LuaHookOnAdvanceSkillLine)
    RegisterHook(SERVER_HOOK_EVENT_ON_DUEL_FINISHED, (void*)LuaHookOnDuelFinished)
    RegisterHook(SERVER_HOOK_EVENT_ON_AURA_REMOVE, (void*)LuaHookOnAuraRemove)
    RegisterHook(SERVER_HOOK_EVENT_ON_RESURRECT, (void*)LuaHookOnResurrect)

    for (const auto& dummySpell : LuaGlobal::instance()->m_luaDummySpells)
    {
        auto dummyHook = LuaGlobal::instance()->luaEngine()->HookInfo.dummyHooks;
        if (std::find(dummyHook.begin(), dummyHook.end(), dummySpell.first) == dummyHook.end())
        {
            m_scriptMgr->register_dummy_spell(dummySpell.first, &LuaOnDummySpell);
            LuaGlobal::instance()->luaEngine()->HookInfo.dummyHooks.push_back(dummySpell.first);
        }
    }
    RELEASE_LOCK
    getcoLock().Release();

    //hyper: do OnSpawns for spawned creatures.
    std::vector<uint32_t> temp = LuaGlobal::instance()->m_onLoadInfo;
    LuaGlobal::instance()->m_onLoadInfo.clear();
    for (auto itr = temp.begin(); itr != temp.end(); itr += 3)
    {
        //*itr = mapid; *(itr+1) = iid; *(itr+2) = lowguid
        WorldMap* mgr = nullptr;
        if (*(itr + 1) == 0) //no instance
        {
            mgr = sMapMgr.findWorldMap(*itr);
        }
        else
        {
            InstanceMap* inst = sMapMgr.findInstanceMap(*(itr + 1));
            if (inst != nullptr)
                mgr = inst;
        }

        if (mgr != nullptr)
        {
            Creature* unit = mgr->getCreature(*(itr + 2));
            if (unit != nullptr && unit->IsInWorld() && unit->GetScript() != nullptr)
                unit->GetScript()->OnLoad();
        }
    }
    temp.clear();

    DLLLogDetail("LuaEngineMgr : Done restarting engine.");
}

void LuaEngine::ResumeLuaThread(int ref)
{
    getcoLock().Acquire();
    lua_State* expectedThread = nullptr;
    lua_rawgeti(lu, LUA_REGISTRYINDEX, ref);
    if (lua_isthread(lu, -1))
        expectedThread = lua_tothread(lu, -1);

    if (expectedThread != nullptr)
    {
        //push ourself on the stack
        lua_pushthread(expectedThread);
        //move the thread to the main lu state(and pop it off)
        lua_xmove(expectedThread, lu, 1);
        if (lua_rawequal(lu, -1, -2))
        {
            lua_pop(lu, 2);
            int res = lua_resume(expectedThread, expectedThread, lua_gettop(expectedThread), nullptr);
            if (res && res != LUA_YIELD)
                report(expectedThread);
        }
        else
        {
            lua_pop(lu, 2);
        }
        luaL_unref(lu, LUA_REGISTRYINDEX, ref);
    }
    getcoLock().Release();
}

//I know its not a good idea to do it like that BUT it is the easiest way. I will make it better in steps:
#include "FunctionTables.h"
#include "LUAFunctions.h"
