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

#ifndef _INCLUDE_PRECACHE_FIX_H_
#define _INCLUDE_PRECACHE_FIX_H_

#include "particle_fix.h"
#include "subhook.h"
#include <UtlStringMap.h>
#include <utlvector.h>

class PrecacheFix
{
public:
	bool Init(char* error, size_t maxlength);
	void Enable();
	void Shutdown();
	
private:
	class CParticleSystemDefinition
	{
	public:
		~CParticleSystemDefinition();
	};
	
	class CParticleSystemDictionary
	{
	public:
		void Clear();
		
	private:
		CUtlStringMap<CParticleSystemDefinition*> m_ParticleNameMap;
		CUtlVector<CParticleSystemDefinition*> m_ParticleIdMap;
	};
	static_assert(sizeof(CParticleSystemDictionary) == 92, "CParticleSystemDictionary - incorrect size");
	
	static void PrecacheStandardParticleSystemsHook();
	
	static void (WIN_LINUX(__thiscall, __cdecl) *CParticleSystemDefinition_destructor)(CParticleSystemDefinition*);
	static void (*ParseParticleEffects)(bool bLoadSheets);
	subhook::Hook* m_pPrecacheStandardParticleSystemsHook;
	static CParticleSystemDictionary* m_pParticleSystemDictionary;
	static void (*PrecacheStandardParticleSystems)();
};

#endif // _INCLUDE_PRECACHE_FIX_H_