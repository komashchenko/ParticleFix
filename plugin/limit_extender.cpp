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

#include "limit_extender.h"
#include <server_class.h>
#include <dt_send.h>
#include <inetchannel.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

decltype(LimitExtender::m_pFullSendTables) LimitExtender::m_pFullSendTables;
decltype(LimitExtender::CBaseClient_SendSignonData) LimitExtender::CBaseClient_SendSignonData;

bool LimitExtender::Init(char* error, size_t maxlength)
{
#ifdef _WIN32
	const uint8_t CreateNetworkStringTablesPattern[] = "\x8B\x0D\x2A\x2A\x2A\x2A\x6A\x01\x6A\x00\x6A\x00\x8B\x01\x68\x00\x04\x00\x00\x68";
	const uint8_t SendTablePattern[] = "\x55\x8B\xEC\x83\xEC\x0C\x83\x3D\x2A\x2A\x2A\x2A\x00\x53\x56\x57\x8B\xFA\x8B\xD9\x74";
	const uint8_t CreateBaselinePattern[] = "\x55\x8B\xEC\x83\xEC\x5C\xB9\x2A\x2A\x2A\x2A\x53\x56\x57\xE8";
	const uint8_t CHLTVClient_SendSignonDataPattern[] = "\x55\x8B\xEC\x83\xEC\x44\x56\x8B\xF1\x8B\x86\xF8\x01\x00\x00";
	const uint8_t CBaseClient_SendSignonDataPattern[] = "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x54\x53\x56\x57\x68";
#else
	const uint8_t CreateNetworkStringTablesPattern[] = "\x55\x89\xE5\x83\xEC\x10\xA1\x2A\x2A\x2A\x2A\x8B\x10\x6A\x01\x6A\x00\x6A\x00\x68\x00\x04\x00\x00\x68\x2A\x2A\x2A\x2A\x50\xFF\x52\x08\x83\xC4\x18";
	const uint8_t SendTablePattern[] = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x1C\x8B\x1D\x2A\x2A\x2A\x2A\x85\xDB\x0F\x85";
	const uint8_t CreateBaselinePattern[] = "\x55\x89\xE5\x57\x56\x53\x81\xEC\x78\x40\x00\x00\x65\xA1\x14\x00\x00\x00\x89\x45\xE4\x31\xC0";
	const uint8_t CHLTVClient_SendSignonDataPattern[] = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x4C\x8B\x5D\x08\x8B\xB3\xE4\x01\x00\x00";
	const uint8_t CBaseClient_SendSignonDataPattern[] = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x68\x8B\x5D\x08\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x10";
#endif
	
	uintptr_t pCreateNetworkStringTables = g_PatternFinderServer.Find<uintptr_t>(CreateNetworkStringTablesPattern, sizeof(CreateNetworkStringTablesPattern) - 1);
	if(!pCreateNetworkStringTables)
	{
		V_strncpy(error, "Failed to find CServerGameDLL::CreateNetworkStringTables.", maxlength);
		
		return false;
	}
	
	uintptr_t pSendTable_Init = g_PatternFinderEngine.Find<uintptr_t>(SendTablePattern, sizeof(SendTablePattern) - 1);
	if(!pSendTable_Init)
	{
		V_strncpy(error, "Failed to find SendTable_Init.", maxlength);
		
		return false;
	}
	
	uintptr_t pCreateBaseline = g_PatternFinderEngine.Find<uintptr_t>(CreateBaselinePattern, sizeof(CreateBaselinePattern) - 1);
	if(!pCreateBaseline)
	{
		V_strncpy(error, "Failed to find SV_CreateBaseline.", maxlength);
		
		return false;
	}
	
	void* pCHLTVClient_SendSignonData = g_PatternFinderEngine.Find<void*>(CHLTVClient_SendSignonDataPattern, sizeof(CHLTVClient_SendSignonDataPattern) - 1);
	if(!pCHLTVClient_SendSignonData)
	{
		V_strncpy(error, "Failed to find CHLTVClient::SendSignonData.", maxlength);
		
		return false;
	}
	
	CBaseClient_SendSignonData = g_PatternFinderEngine.Find<decltype(CBaseClient_SendSignonData)>(CBaseClient_SendSignonDataPattern, sizeof(CBaseClient_SendSignonDataPattern) - 1);
	if(!CBaseClient_SendSignonData)
	{
		V_strncpy(error, "Failed to find CBaseClient::SendSignonData.", maxlength);
		
		return false;
	}
	
	// Maxentries parameter
	m_pMaxEntries = reinterpret_cast<uint32_t*>(pCreateNetworkStringTables + WIN_LINUX(0x2F, 0x37));
	
	// DT_ParticleSystem::m_iEffectIndex::m_nBits
	ServerClass* pClasses = g_pServerGameDLL->GetAllServerClasses();
	for (ServerClass* pClass = pClasses; pClass; pClass = pClass->m_pNext)
	{
		SendTable* pTable = pClass->m_pTable;
		if(strcmp(pTable->GetName(), "DT_ParticleSystem") == 0)
		{
			for(int i = 0; i < pTable->GetNumProps(); i++)
			{
				SendProp* pProp = pTable->GetProp(i);
				if(strcmp(pProp->GetName(), "m_iEffectIndex") == 0)
				{
					m_pBits = &pProp->m_nBits;
					
					break;
				}
			}
			
			break;
		}
	}
	
	// g_SendTableCRC
	m_pSendTableCRC = *reinterpret_cast<uint32_t**>(pSendTable_Init + WIN_LINUX(0xCD, 0xEC));
	
	// Transfer modified table to clients
	m_pSendTables = g_pCVar->FindVar("sv_sendtables");
	
	// sv.m_FullSendTables
	m_pFullSendTables = *reinterpret_cast<bf_write**>(pCreateBaseline + WIN_LINUX(0xA5, 0xBC));
	
	m_pCHLTVClient_SendSignonData = new subhook::Hook(pCHLTVClient_SendSignonData, reinterpret_cast<void*>(CHLTVClient_SendSignonDataHook));
	
	return true;
}

void LimitExtender::Enable()
{
	UnprotectMem(m_pMaxEntries, 4);
	
	// New limits
	// Increasing maximum number lines in ParticleEffectNames
	*m_pMaxEntries = 1 << MAX_PARTICLESYSTEMS_STRING_BITS;
	// Number bits allocated for network transfer of DT_ParticleSystem::m_iEffectIndex also needs increased
	*m_pBits = MAX_PARTICLESYSTEMS_STRING_BITS;
	
	// Trigger
	*m_pSendTableCRC = ~0;
	
	// Disallow changing sv_sendtables
	m_pSendTables->InstallChangeCallback(SendTablesChangeCallback);
	
	m_pCHLTVClient_SendSignonData->Install();
}

void LimitExtender::Shutdown()
{
	if(m_pSendTables)
	{
		m_pSendTables->RemoveChangeCallback(SendTablesChangeCallback);
	}
	
	if(m_pCHLTVClient_SendSignonData)
	{
		delete m_pCHLTVClient_SendSignonData;
	}
}

void LimitExtender::SendTablesChangeCallback(IConVar* pVar, const char* pszOldValue, float flOldValue)
{
	if(static_cast<ConVar*>(pVar)->GetInt() != 1)
	{
		pVar->SetValue(1);
	}
}

bool LimitExtender::CHLTVClient_SendSignonDataHook(CHLTVClient* _this)
{
	// Send tables & class infos
	_this->GetNetChannel()->SendData(*m_pFullSendTables);
	
	return CBaseClient_SendSignonData(_this);
}