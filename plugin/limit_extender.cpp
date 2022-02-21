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
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

bool LimitExtender::Init(char* error, size_t maxlength)
{
#ifdef _WIN32
	const uint8_t CreateNetworkStringTablesPattern[] = "\x8B\x0D\x2A\x2A\x2A\x2A\x6A\x01\x6A\x00\x6A\x00\x8B\x01\x68\x00\x04\x00\x00\x68";
	const uint8_t SendTablePattern[] = "\x55\x8B\xEC\x83\xEC\x0C\x83\x3D\x2A\x2A\x2A\x2A\x00\x53\x56\x57\x8B\xFA\x8B\xD9\x74";
#else
	const uint8_t CreateNetworkStringTablesPattern[] = "\x55\x89\xE5\x83\xEC\x28\xA1\x2A\x2A\x2A\x2A\x8B\x10\xC7\x44\x24\x14\x01\x00\x00\x00\xC7\x44\x24\x10\x00\x00\x00\x00\xC7\x44\x24\x0C\x00\x00\x00\x00\xC7\x44\x24\x08\x00\x04\x00\x00\xC7\x44\x24\x04\x2A\x2A\x2A\x2A\x89\x04\x24\xFF\x52\x08\xA3\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x8B\x10";
	const uint8_t SendTablePattern[] = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x4C\xA1\x2A\x2A\x2A\x2A\x8B\x7D\x08\x85\xC0";
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
	
	// Maxentries parameter
	m_pMaxEntries = reinterpret_cast<uint32_t*>(pCreateNetworkStringTables + WIN_LINUX(0x2F, 0x63));
	
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
	m_pSendTableCRC = *reinterpret_cast<uint32_t**>(pSendTable_Init + WIN_LINUX(0xCD, 0x10E));
	
	// Transfer modified table to clients
	m_pSendTables = g_pCVar->FindVar("sv_sendtables");
	
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
}

void LimitExtender::Shutdown()
{
	if(m_pSendTables)
	{
		m_pSendTables->RemoveChangeCallback(SendTablesChangeCallback);
	}
}

void LimitExtender::SendTablesChangeCallback(IConVar* pVar, const char* pszOldValue, float flOldValue)
{
	if(static_cast<ConVar*>(pVar)->GetInt() != 1)
	{
		pVar->SetValue(1);
	}
}