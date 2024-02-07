#include <stdio.h>
#include "antiflash.h"
#include "metamod_oslink.h"
#include "utils.hpp"
#include <utlstring.h>
#include <KeyValues.h>
#include "sdk/schemasystem.h"
#include "sdk/CBaseEntity.h"
#include "sdk/CBasePlayerPawn.h"
#include "sdk/CCSPlayerController.h"

antiflash g_antiflash;
PLUGIN_EXPOSE(antiflash, g_antiflash);
IVEngineServer2* engine = nullptr;
IGameEventManager2* gameeventmanager = nullptr;
IGameResourceServiceServer* g_pGameResourceService = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;
CSchemaSystem* g_pCSchemaSystem = nullptr;
Cplayer_blind g_player_blind;
bool g_bPistolRound;

class GameSessionConfiguration_t { };
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);
SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);

bool antiflash::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceService, IGameResourceServiceServer, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	// Get CSchemaSystem
	{
		HINSTANCE m_hModule = dlmount(WIN_LINUX("schemasystem.dll", "libschemasystem.so"));
		g_pCSchemaSystem = reinterpret_cast<CSchemaSystem*>(reinterpret_cast<CreateInterfaceFn>(dlsym(m_hModule, "CreateInterface"))(SCHEMASYSTEM_INTERFACE_VERSION, nullptr));
		dlclose(m_hModule);
	}

	SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &antiflash::StartupServer), true);
	SH_ADD_HOOK(IServerGameDLL, GameFrame, g_pSource2Server, SH_MEMBER(this, &antiflash::GameFrame), true);

	gameeventmanager = static_cast<IGameEventManager2*>(CallVFunc<IToolGameEventAPI*, 93>(g_pSource2Server));

	ConVar_Register(FCVAR_GAMEDLL);

	return true;
}

bool antiflash::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IServerGameDLL, GameFrame, g_pSource2Server, SH_MEMBER(this, &antiflash::GameFrame), true);
	SH_REMOVE_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &antiflash::StartupServer), true);

	gameeventmanager->RemoveListener(&g_player_blind);
	ConVar_Unregister();
	
	return true;
}

void antiflash::NextFrame(std::function<void()> fn)
{
	m_nextFrame.push_back(fn);
}

void antiflash::StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession*, const char*)
{
	static bool bDone = false;
	if (!bDone)
	{
		g_pGameEntitySystem = *reinterpret_cast<CGameEntitySystem**>(reinterpret_cast<uintptr_t>(g_pGameResourceService) + WIN_LINUX(0x58, 0x50));
		g_pEntitySystem = g_pGameEntitySystem;
		gameeventmanager->AddListener(&g_player_blind, "player_blind", true);
		bDone = true;
	}
}

void antiflash::GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
	while (!m_nextFrame.empty())
	{
		m_nextFrame.front()();
		m_nextFrame.pop_front();
	}
}

void Cplayer_blind::FireGameEvent(IGameEvent* event)
{
	CCSPlayerController* pPlayerController_target = static_cast<CCSPlayerController*>(event->GetPlayerController("userid"));
	if (!pPlayerController_target) // Ignore bots
		return;

	CCSPlayerController* pPlayerController_attacker = static_cast<CCSPlayerController*>(event->GetPlayerController("attacker"));
	if (!pPlayerController_attacker) // Ignore bots
		return;
		
	if(pPlayerController_attacker->m_hPlayerPawn()->m_iTeamNum() == pPlayerController_target->m_hPlayerPawn()->m_iTeamNum())
	{
		if(pPlayerController_attacker != pPlayerController_target)//maybe you should blind yourself
		{
			CCSPlayerPawnBase* pPlayerPawn = pPlayerController_target->m_hPlayerPawn();
			pPlayerPawn->m_flFlashDuration() = 0.0;
		}
	}
}


///////////////////////////////////////
const char* antiflash::GetLicense()
{
	return "GPL";
}

const char* antiflash::GetVersion()
{
	return "1.0.0";
}

const char* antiflash::GetDate()
{
	return __DATE__;
}

const char *antiflash::GetLogTag()
{
	return "antiflash";
}

const char* antiflash::GetAuthor()
{
	return "bklol";
}

const char* antiflash::GetDescription()
{
	return "antiflash";
}

const char* antiflash::GetName()
{
	return "antiflash";
}

const char* antiflash::GetURL()
{
	return "";
}
