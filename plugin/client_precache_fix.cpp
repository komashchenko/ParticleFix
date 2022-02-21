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

#include "client_precache_fix.h"
#include <algorithm>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

decltype(ClientPrecacheFix::m_TablesOverride) ClientPrecacheFix::m_TablesOverride;
decltype(ClientPrecacheFix::m_TablesAwaitingSend) ClientPrecacheFix::m_TablesAwaitingSend;
decltype(ClientPrecacheFix::CNetworkStringTable_WriteUpdate) ClientPrecacheFix::CNetworkStringTable_WriteUpdate;

bool ClientPrecacheFix::Init(char* error, size_t maxlength)
{
#ifdef _WIN32
	const uint8_t WriteBaselinesPattern[] = "\x8B\x06\x8B\xCE\x53\xFF\x50\x10\x8B\x7D\x0C\x8B\xF0\xFF\x75\x10\x8B\xCE\x8B\x47\x0C\x83\xC0\x07\xC1";
	const uint8_t CNetworkStringTablePattern[] = "\x55\x8B\xEC\x51\x8B\x45\x1C\x53\x8B\x5D\x10\x56\x8B\xF1\x8B\x4D\x14\x57\x80\x66\x20\xFB\x89\x46\x24";
	const uint8_t WriteUpdatePattern[] = "\x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x58\x05\x00\x00";
#else
	const uint8_t WriteBaselinesPattern[] = "\x8B\x55\xA4\x8B\x06\x89\x34\x24\x89\x54\x24\x04\xFF\x50\x14\x89\xC3\x8B\x47\x0C\x8B\x55\x14\x89\x1C";
	const uint8_t CNetworkStringTablePattern[] = "\x55\x89\xE5\x57\x56\x53\x83\xEC\x2C\x8B\x5D\x08\x8B\x75\x18\x8B\x7D\x14\x80\x63\x20\xFB\xC7\x03";
	const uint8_t WriteUpdatePattern[] = "\x55\x89\xE5\x57\x56\x53\x81\xEC\x9C\x04\x00\x00\x8B\x45\x18\x8B\x75\x08\x8B\x5D\x10\x39\x45\x14\x7E\x2A\x8B\x46\x18\x39\x45\x18";
#endif

	void* pWriteBaselines = g_PatternFinderEngine.Find<void*>(WriteBaselinesPattern, sizeof(WriteBaselinesPattern) - 1);
	if(!pWriteBaselines)
	{
		V_strncpy(error, "Failed to find CNetworkStringTableContainer::WriteBaselines [GetTable].", maxlength);
		
		return false;
	}
	
	CNetworkStringTable_CNetworkStringTable = g_PatternFinderEngine.Find<decltype(CNetworkStringTable_CNetworkStringTable)>(CNetworkStringTablePattern, sizeof(CNetworkStringTablePattern) - 1);
	if(!CNetworkStringTable_CNetworkStringTable)
	{
		V_strncpy(error, "Failed to find CNetworkStringTable::CNetworkStringTable.", maxlength);
		
		return false;
	}
	
	void* pCNetworkStringTable_WriteUpdate = g_PatternFinderEngine.Find<void*>(WriteUpdatePattern, sizeof(WriteUpdatePattern) - 1);
	if(!pCNetworkStringTable_WriteUpdate)
	{
		V_strncpy(error, "Failed to find CNetworkStringTable::WriteUpdate.", maxlength);
		
		return false;
	}
	
	m_pWriteBaselines_GetTableHook = new subhook::Hook(pWriteBaselines, reinterpret_cast<void*>(WriteBaselines_GetTableHook));
	
	m_pCNetworkStringTable_WriteUpdate = new subhook::Hook(pCNetworkStringTable_WriteUpdate, reinterpret_cast<void*>(CNetworkStringTable_WriteUpdateHook));
	CNetworkStringTable_WriteUpdate = reinterpret_cast<decltype(CNetworkStringTable_WriteUpdate)>(m_pCNetworkStringTable_WriteUpdate->GetTrampoline());
	
	uintptr_t pJmpBack = reinterpret_cast<uintptr_t>(WriteBaselines_GetTableHook) + WIN_LINUX(0x1C, 0x1A);
	
	// Set jmp address [pWriteBaselines + 0x8 | 0xF]
	UnprotectMem(pJmpBack, 4);
	*reinterpret_cast<uint32_t*>(pJmpBack) = reinterpret_cast<uintptr_t>(pWriteBaselines) - pJmpBack + WIN_LINUX(0x8, 0xF) - 0x4;
	
	return true;
}

void ClientPrecacheFix::Enable()
{
	m_pWriteBaselines_GetTableHook->Install();
	m_pCNetworkStringTable_WriteUpdate->Install();
}

void ClientPrecacheFix::Shutdown()
{
	if(m_pWriteBaselines_GetTableHook)
	{
		delete m_pWriteBaselines_GetTableHook;
	}
	
	if(m_pCNetworkStringTable_WriteUpdate)
	{
		delete m_pCNetworkStringTable_WriteUpdate;
	}
	
	for(const auto& it : m_TablesOverride)
	{
		delete it.second;
	}
}

void ClientPrecacheFix::LevelInit()
{
	// It is enough to create a table for override once
	if(m_TablesOverride.empty())
	{
		const char* szOverrideTables[] = {"genericprecache", "ExtraParticleFilesTable", "ParticleEffectNames"};
		for(int i = 0; i < ARRAYSIZE(szOverrideTables); i++)
		{
			INetworkStringTable* pTable = g_pNetworkStringTable->FindTable(szOverrideTables[i]);
			m_TablesOverride[pTable->GetTableId()] = CopyStringTable(pTable);
		}
	}
}

void ClientPrecacheFix::ClientPutInServer(edict_t* pEdict)
{
	auto& fullSend = m_TablesAwaitingSend[pEdict - gpGlobals->pEdicts - 1];
	
	for(const auto& it : m_TablesOverride)
	{
		fullSend.push_back(it.first);
	}
}

INetworkStringTable* ClientPrecacheFix::CopyStringTable(INetworkStringTable* pTable)
{
	uintptr_t upTable = reinterpret_cast<uintptr_t>(pTable);
	
	INetworkStringTable* pNewTable = reinterpret_cast<INetworkStringTable*>(g_pMemAlloc->Alloc(0x48));
	CNetworkStringTable_CNetworkStringTable(pNewTable, *reinterpret_cast<TABLEID*>(upTable + 0x4), *reinterpret_cast<const char**>(upTable + 0x8), *reinterpret_cast<int*>(upTable + 0xC), *reinterpret_cast<int*>(upTable + 0x28), *reinterpret_cast<int*>(upTable + 0x2C), *reinterpret_cast<int*>(upTable + 0x24));
	
	return pNewTable;
}

WIN_LINUX(__declspec(naked), __attribute__((naked))) void ClientPrecacheFix::WriteBaselines_GetTableHook()
{
	// React call only from CBaseClient::SendServerInfo
#ifdef _WIN32
	__asm mov edx, [ebp+0x0C] // bf_write& buf
	__asm mov edx, [edx+0x14] // buf::m_pDebugName
#else
	__asm mov edx, [edi+0x14] // buf::m_pDebugName
#endif
	__asm xor eax, eax
	__asm cmp dword ptr [edx], 0x535F5653 // S_SV (SV_SendServerinfo->msg)
	__asm sete al
	
	// Call GetTableOverride
	__asm push eax
#ifdef _WIN32
	__asm mov edx, ebx // i
#else
	__asm mov edx, [ebp-0x5C] // i
#endif
	__asm mov ecx, esi // this
	__asm call WriteBaselines_GetTableOverride
	
	// Go back
	__asm _emit 0xE9
	__asm _emit 0x0
	__asm _emit 0x0
	__asm _emit 0x0
	__asm _emit 0x0
}

INetworkStringTable* ClientPrecacheFix::WriteBaselines_GetTableOverride(INetworkStringTableContainer* pStringTables, TABLEID id, bool bSendServerInfo)
{
	if(bSendServerInfo)
	{
		const auto& it = m_TablesOverride.find(id);
		if (it != m_TablesOverride.end())
		{
			return it->second;
		}
	}
	
	return pStringTables->GetTable(id);
}

int ClientPrecacheFix::CNetworkStringTable_WriteUpdateHook(INetworkStringTable* _this, CBaseClient* client, bf_write& buf, int tick_changed, int tick_ack, int entries, int unk1)
{
	if(client)
	{
		auto& fullSend = m_TablesAwaitingSend[client->GetPlayerSlot()];
		if(!fullSend.empty())
		{
			auto tableId = std::find(fullSend.begin(), fullSend.end(), _this->GetTableId());
			if(tableId != fullSend.end())
			{
				fullSend.erase(tableId);
				
				tick_ack = -1;
				entries = _this->GetNumStrings();
			}
		}
	}
	
	return CNetworkStringTable_WriteUpdate(_this, client, buf, tick_changed, tick_ack, entries, unk1);
}