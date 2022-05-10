/**
 * =============================================================================
 * ParticleFix
 * Copyright (C) 2022 Phoenix (˙·٠●Феникс●٠·˙)
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "particle_fix.h"
#include "limit_extender.h"
#include "precache_fix.h"
#include "client_precache_fix.h"
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

EXPOSE_SINGLE_INTERFACE(ParticleFix, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS);

IVEngineServer* g_pEngineServer;
IBaseFileSystem* g_pBaseFileSystem;
CGlobalVars* gpGlobals;
IServerGameDLL* g_pServerGameDLL;
ICvar* g_pCVar;
INetworkStringTableContainer* g_pNetworkStringTable;

PatternFinder g_PatternFinderServer;
PatternFinder g_PatternFinderEngine;
LimitExtender g_LimitExtender;
PrecacheFix g_PrecacheFix;
ClientPrecacheFix g_ClientPrecacheFix;

bool ParticleFix::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	g_pEngineServer = reinterpret_cast<IVEngineServer*>(interfaceFactory(INTERFACEVERSION_VENGINESERVER, nullptr));
	g_pBaseFileSystem = reinterpret_cast<IBaseFileSystem*>(interfaceFactory(BASEFILESYSTEM_INTERFACE_VERSION, nullptr));	
	gpGlobals = reinterpret_cast<IPlayerInfoManager*>(gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER, nullptr))->GetGlobalVars();
	g_pServerGameDLL = reinterpret_cast<IServerGameDLL*>(gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, nullptr));
	g_pCVar = reinterpret_cast<ICvar*>(interfaceFactory(CVAR_INTERFACE_VERSION, nullptr));
	g_pNetworkStringTable = reinterpret_cast<INetworkStringTableContainer*>(interfaceFactory(INTERFACENAME_NETWORKSTRINGTABLESERVER, nullptr));
	
	g_PatternFinderServer.SetLibrary(g_pServerGameDLL);
	g_PatternFinderEngine.SetLibrary(g_pEngineServer);
	
	char szError[128];
	if(!g_LimitExtender.Init(szError, sizeof(szError)))
	{
		FatalError("LimitExtender: %s\n", szError);
		
		return false;
	}
	
	if(!g_PrecacheFix.Init(szError, sizeof(szError)))
	{
		FatalError("PrecacheFix: %s\n", szError);
		
		return false;
	}
	
	if(!g_ClientPrecacheFix.Init(szError, sizeof(szError)))
	{
		FatalError("ClientPrecacheFix: %s\n", szError);
		
		return false;
	}
	
	g_LimitExtender.Enable();
	g_PrecacheFix.Enable();
	g_ClientPrecacheFix.Enable();
	
	return true;
}

void ParticleFix::Unload()
{
	g_LimitExtender.Shutdown();
	g_PrecacheFix.Shutdown();
	g_ClientPrecacheFix.Shutdown();
}

void ParticleFix::ClientPutInServer(edict_t* pEdict, char const* pszPlayerName)
{
	g_ClientPrecacheFix.ClientPutInServer(pEdict);
}

void ParticleFix::LevelInit(char const* pszMapName)
{
	g_ClientPrecacheFix.LevelInit();
}

const char* ParticleFix::GetPluginDescription()
{
	return "ParticleFix (1.0.1) by Phoenix (˙·٠●Феникс●٠·˙)";
}