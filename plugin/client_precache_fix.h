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

#ifndef _INCLUDE_CLIENT_PRECACHE_FIX_H_
#define _INCLUDE_CLIENT_PRECACHE_FIX_H_

#include "particle_fix.h"
#include "subhook.h"
#include <igameevents.h>
#include <map>
#include <list>

#define MAX_PLAYERS 64

class bf_write;

struct ClientPrecacheFix
{
public:
	bool Init(char* error, size_t maxlength);
	void Enable();
	void Shutdown();
	void LevelInit();
	void ClientPutInServer(edict_t* pEdict);
	
private:
	class CBaseClient : public IGameEventListener2, public IClient
	{
	public:
		virtual ~CBaseClient();
	};
	
	INetworkStringTable* CopyStringTable(INetworkStringTable* pTable);
	static void WriteBaselines_GetTableHook();
	static INetworkStringTable* __fastcall WriteBaselines_GetTableOverride(INetworkStringTableContainer* pStringTables, TABLEID id, bool bSendServerInfo);
	static int WIN_LINUX(__thiscall, __cdecl) CNetworkStringTable_WriteUpdateHook(INetworkStringTable* _this, CBaseClient* client, bf_write& buf, int tick_changed, int tick_ack, int entries, int unk1);
	
	void (WIN_LINUX(__thiscall, __cdecl) *CNetworkStringTable_CNetworkStringTable)(INetworkStringTable* pMem, TABLEID id, const char* tableName, int maxentries, int userdatafixedsize, int userdatanetworkbits, int flags);
	subhook::Hook* m_pWriteBaselines_GetTableHook;
	subhook::Hook* m_pCNetworkStringTable_WriteUpdate;
	static int (WIN_LINUX(__thiscall, __cdecl) *CNetworkStringTable_WriteUpdate)(INetworkStringTable* _this, CBaseClient* client, bf_write& buf, int tick_changed, int tick_ack, int entries, int unk1);
	static std::map<TABLEID, INetworkStringTable*> m_TablesOverride;
	static std::list<TABLEID> m_TablesAwaitingSend[MAX_PLAYERS];
};

#endif // _INCLUDE_CLIENT_PRECACHE_FIX_H_