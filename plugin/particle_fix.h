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

#ifndef _INCLUDE_PARTICLE_FIX_H_
#define _INCLUDE_PARTICLE_FIX_H_

#include <iserverplugin.h>
#include <interface.h>
#include <eiface.h>
#include <networkstringtabledefs.h>
#include <filesystem.h>
#include <iplayerinfo.h>
#include "pattern_finder.hpp"
#include "utils.hpp"

class ParticleFix: public IServerPluginCallbacks
{
public: // IServerPluginCallbacks
	// Initialize the plugin to run
	// Return false if there is an error during startup.
	bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);

	// Called when the plugin should be shutdown
	void Unload(void);

	// called when a plugins execution is stopped but the plugin is not unloaded
	void Pause(void)
	{
	}

	// called when a plugin should start executing again (sometime after a Pause() call)
	void UnPause(void)
	{
	}

	// Returns string describing current plugin.  e.g., Admin-Mod.  
	const char* GetPluginDescription(void);

	// Called any time a new level is started (after GameInit() also on level transitions within a game)
	void LevelInit(char const* pszMapName);

	// The server is about to activate
	void ServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
	{
	}

	// The server should run physics/think on all edicts
	void GameFrame(bool simulating)
	{
	}

	// Called when a level is shutdown (including changing levels)
	void LevelShutdown(void)
	{
	}

	// Client is going active
	void ClientActive(edict_t* pEntity)
	{
	}

	// Client is fully connected ( has received initial baseline of entities )
	void ClientFullyConnect(edict_t* pEntity)
	{
	}

	// Client is disconnecting from server
	void ClientDisconnect(edict_t* pEntity)
	{
	}
	
	// Client is connected and should be put in the game
	void ClientPutInServer(edict_t* pEntity, char const* playername);

	// Sets the client index for the client who typed the command into their console
	void SetCommandClient(int index)
	{
	}

	// A player changed one/several replicated cvars (name etc)
	void ClientSettingsChanged(edict_t* pEdict)
	{
	}

	// Client is connecting to server ( set retVal to false to reject the connection )
	//	You can specify a rejection message by writing it into reject
	PLUGIN_RESULT ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen)
	{
		return PLUGIN_CONTINUE;
	}

	// The client has typed a command at the console
	PLUGIN_RESULT ClientCommand(edict_t* pEntity, const CCommand& args)
	{
		return PLUGIN_CONTINUE;
	}

	// A user has had their network id setup and validated 
	PLUGIN_RESULT NetworkIDValidated(const char* pszUserName, const char* pszNetworkID)
	{
		return PLUGIN_CONTINUE;
	}

	// This is called when a query from IServerPluginHelpers::StartQueryCvarValue is finished.
	// iCookie is the value returned by IServerPluginHelpers::StartQueryCvarValue.
	// Added with version 2 of the interface.
	void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, EQueryCvarValueStatus eStatus, const char* pCvarName, const char* pCvarValue)
	{
	}

	// added with version 3 of the interface.
	void OnEdictAllocated(edict_t* edict)
	{
	}

	void OnEdictFreed(const edict_t* edict)
	{
	}

	//
	// Allow plugins to validate and configure network encryption keys (added in Version 4 of the interface)
	// Game server must run with -externalnetworkcryptkey flag, and 3rd party client software must set the
	// matching encryption key in the client game process.
	//

	// BNetworkCryptKeyCheckRequired allows the server to allow connections from clients or relays that don't have
	// an encryption key. The function must return true if the client encryption key is required, and false if the client
	// is allowed to connect without an encryption key. It is recommended that if client wants to use encryption key
	// this function should return true to require it on the server side as well.
	// Any plugin in the chain that returns true will flag connection to require encryption key for the engine and check
	// with other plugins will not be continued.
	// If no plugin returns true to require encryption key then the default implementation will require encryption key
	// if the client wants to use it.
	bool BNetworkCryptKeyCheckRequired(uint32 unFromIP, uint16 usFromPort, uint32 unAccountIdProvidedByClient, bool bClientWantsToUseCryptKey)
	{
		return false;
	}

	// BNetworkCryptKeyValidate allows the server to validate client's over the wire encrypted payload cookie and return
	// false if the client cookie is malformed to prevent connection to the server. If this function returns true then
	// the plugin allows the client to connect with the encryption key, and upon return the engine expects the plugin
	// to have copied 16-bytes of client encryption key into the buffer pointed at by pbPlainTextKeyForNetChan. That key
	// must match the plaintext key supplied by 3rd party client software to the client game process, not the client cookie
	// transmitted unencrypted over the wire as part of the connection packet.
	// Any plugin in the chain that returns true will stop evaluation of other plugins and the 16-bytes encryption key
	// copied into pbPlainTextKeyForNetchan will be used. If a plugin returns false then evaluation of other plugins will
	// continue and buffer data in pbPlainTextKeyForNetchan will be preserved from previous calls.
	// If no plugin returns true and the encryption key is required then the client connection will be rejected with
	// an invalid certificate error.
	bool BNetworkCryptKeyValidate(uint32 unFromIP, uint16 usFromPort, uint32 unAccountIdProvidedByClient, int nEncryptionKeyIndexFromClient, int numEncryptedBytesFromClient, byte* pbEncryptedBufferFromClient, byte* pbPlainTextKeyForNetchan)
	{
		return false;
	}
};

extern IVEngineServer* g_pEngineServer;
extern IBaseFileSystem* g_pBaseFileSystem;
extern CGlobalVars* gpGlobals;
extern IServerGameDLL* g_pServerGameDLL;
extern ICvar* g_pCVar;
extern INetworkStringTableContainer* g_pNetworkStringTable;

extern PatternFinder g_PatternFinderServer;
extern PatternFinder g_PatternFinderEngine;

#endif // _INCLUDE_PARTICLE_FIX_H_