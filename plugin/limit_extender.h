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

#ifndef _INCLUDE_LIMIT_EXTENDER_H_
#define _INCLUDE_LIMIT_EXTENDER_H_

#include "particle_fix.h"
#include "subhook.h"
#include <icvar.h>
#include <igameevents.h>

class LimitExtender
{
public:
	bool Init(char* error, size_t maxlength);
	void Enable();
	void Shutdown();
	
private:
	class CBaseClient : public IGameEventListener2, public IClient
	{
	public:
		virtual ~CBaseClient();
	};
	
	class CHLTVClient : public CBaseClient
	{
	public:
		virtual ~CHLTVClient();
	};
	
	static void SendTablesChangeCallback(IConVar* pVar, const char* pszOldValue, float flOldValue);
	static bool WIN_LINUX(__thiscall, __cdecl) CHLTVClient_SendSignonDataHook(CHLTVClient* _this);
	
	uint32_t* m_pMaxEntries;
	int32_t* m_pBits;
	uint32_t* m_pSendTableCRC;
	ConVar* m_pSendTables;
	static bf_write* m_pFullSendTables;
	subhook::Hook* m_pCHLTVClient_SendSignonData;
	static bool (WIN_LINUX(__thiscall, __cdecl) *CBaseClient_SendSignonData)(CBaseClient* _this);
};

#endif // _INCLUDE_LIMIT_EXTENDER_H_