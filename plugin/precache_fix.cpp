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

#include "precache_fix.h"
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

decltype(PrecacheFix::CParticleSystemDefinition_destructor) PrecacheFix::CParticleSystemDefinition_destructor;
decltype(PrecacheFix::ParseParticleEffects) PrecacheFix::ParseParticleEffects;
decltype(PrecacheFix::m_pParticleSystemDictionary) PrecacheFix::m_pParticleSystemDictionary;
decltype(PrecacheFix::PrecacheStandardParticleSystems) PrecacheFix::PrecacheStandardParticleSystems;

bool PrecacheFix::Init(char* error, size_t maxlength)
{
#ifdef _WIN32
	const uint8_t CParticleSystemDefinitionPattern[] = "\x56\x8B\xF1\x8B\x46\x64\x85\xC0\x74\x2A\x66\x0F\x1F\x44\x00\x00\x8B\xC8\xE8\x2A\x2A\x2A\x2A\x8B\x46\x64\x85\xC0\x75";
	const uint8_t ParseParticleEffectsPattern[] = "\x55\x8B\xEC\x83\xEC\x14\x8A\x45\x08\x8D\x4D\xEC\x0F\x57\xC0\xA2\x2A\x2A\x2A\x2A\x57\x0F\x11\x45\xEC";
	const uint8_t PrecacheStandardParticleSystemsPattern[] = "\x55\x8B\xEC\x51\x56\x8B\x35\x2A\x2A\x2A\x2A\x57\x33\xFF\x0F\xB7\x46\x26\x85\xC0\x0F\x8E";
#else
	const uint8_t CParticleSystemDefinitionPattern[] = "\x55\x89\xE5\x56\x53\x8B\x5D\x08\x8B\x43\x64\x85\xC0\x74\x2A\x90\x50\xE8\x2A\x2A\x2A\x2A\x8B\x43\x64\x5E\x85\xC0";
	const uint8_t ParseParticleEffectsPattern[] = "\x55\x89\xE5\x57\x56\x8D\x7D\xD4\x53\x83\xEC\x34\x0F\xB6\x45\x08\x50\xFF\x35\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xC7\x45\xD4\x00\x00\x00\x00";
	const uint8_t PrecacheStandardParticleSystemsPattern[] = "\x55\x89\xE5\x56\x53\x31\xDB\xEB\x2A\x8D\xB4\x26\x00\x00\x00\x00\x83\xC3\x01\x83\xEC\x0C\xFF\x35\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x10";
#endif
	
	CParticleSystemDefinition_destructor = g_PatternFinderServer.Find<decltype(CParticleSystemDefinition_destructor)>(CParticleSystemDefinitionPattern, sizeof(CParticleSystemDefinitionPattern) - 1);
	if(!CParticleSystemDefinition_destructor)
	{
		V_strncpy(error, "Failed to find CParticleSystemDefinition::~CParticleSystemDefinition.", maxlength);
		
		return false;
	}
	
	ParseParticleEffects = g_PatternFinderServer.Find<decltype(ParseParticleEffects)>(ParseParticleEffectsPattern, sizeof(ParseParticleEffectsPattern) - 1);
	if(!ParseParticleEffects)
	{
		V_strncpy(error, "Failed to find ParseParticleEffects.", maxlength);
		
		return false;
	}
	
	void* pPrecacheStandardParticleSystems = g_PatternFinderServer.Find<void*>(PrecacheStandardParticleSystemsPattern, sizeof(PrecacheStandardParticleSystemsPattern) - 1);
	if(!pPrecacheStandardParticleSystems)
	{
		V_strncpy(error, "Failed to find PrecacheStandardParticleSystems.", maxlength);
		
		return false;
	}
	
	// g_pParticleSystemMgr::m_pParticleSystemDictionary
#ifdef _WIN32
	m_pParticleSystemDictionary = **reinterpret_cast<CParticleSystemDictionary***>(reinterpret_cast<uintptr_t>(pPrecacheStandardParticleSystems) + 0x7);
#else
	m_pParticleSystemDictionary = *reinterpret_cast<CParticleSystemDictionary**>(reinterpret_cast<uintptr_t>(**reinterpret_cast<void***>(reinterpret_cast<uintptr_t>(pPrecacheStandardParticleSystems) + 0x18)) + 0x8C);
#endif	
	
	m_pPrecacheStandardParticleSystemsHook = new subhook::Hook(pPrecacheStandardParticleSystems, reinterpret_cast<void*>(PrecacheStandardParticleSystemsHook));
	PrecacheStandardParticleSystems = reinterpret_cast<decltype(PrecacheStandardParticleSystems)>(m_pPrecacheStandardParticleSystemsHook->GetTrampoline());
	
	return true;
}

void PrecacheFix::Enable()
{
	m_pPrecacheStandardParticleSystemsHook->Install();
}

void PrecacheFix::Shutdown()
{
	if(m_pPrecacheStandardParticleSystemsHook)
	{
		delete m_pPrecacheStandardParticleSystemsHook;
	}
}

PrecacheFix::CParticleSystemDefinition::~CParticleSystemDefinition()
{
	CParticleSystemDefinition_destructor(this);
}

void PrecacheFix::CParticleSystemDictionary::Clear()
{
	m_ParticleNameMap.PurgeAndDeleteElements();
	m_ParticleIdMap.PurgeAndDeleteElements();
}

void PrecacheFix::PrecacheStandardParticleSystemsHook()
{
	// Removing all particles
	m_pParticleSystemDictionary->Clear();
	
	// Load particles
	ParseParticleEffects(false);
	
	// Precache standard particles
	PrecacheStandardParticleSystems();
	
	// Load and precache map particles
	char szMapManifestFilename[MAX_PATH];
	V_snprintf(szMapManifestFilename, sizeof(szMapManifestFilename), "maps/%s_particles.txt", STRING(gpGlobals->mapname));
	
	KeyValues* pManifest = new KeyValues("particles_manifest");
	if(pManifest->LoadFromFile(g_pBaseFileSystem, szMapManifestFilename, "GAME"))
	{
		for(KeyValues* sub = pManifest->GetFirstSubKey(); sub != nullptr; sub = sub->GetNextKey())
		{
			if(Q_stricmp(sub->GetName(), "file") == 0)
			{
				const char* pszPath = sub->GetString();
				if(V_stricmp(V_GetFileExtensionSafe(pszPath), "pcf") == 0)
				{
					if(pszPath[0] == '!')
					{
						pszPath++;
					}
					
					g_pEngineServer->PrecacheGeneric(pszPath, true);
				}
			}
		}
	}
	
	pManifest->deleteThis();
}