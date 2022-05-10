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

#ifndef _PATTERN_FINDER_HPP_
#define _PATTERN_FINDER_HPP_

#include <cstdint>
#include <string.h>

#ifdef _WIN32 // Windows
#include <Windows.h>
#elif __linux__ // Linux
#include <unistd.h>
#include <fcntl.h>
#include <link.h>
#include <sys/mman.h>
#else // Unknown
#error Unsupported operating system
#endif

class PatternFinder
{
public:
	PatternFinder() : m_baseAddr(nullptr), m_size(0)
	{}
	
	bool SetLibrary(const void* lib, bool handle = false)
	{
		m_baseAddr = nullptr;
		m_size = 0;
		
		if(!lib)
		{
			return false;
		}
		
#ifdef _WIN32
		
		MEMORY_BASIC_INFORMATION info;
		
		if(!VirtualQuery(lib, &info, sizeof(info)))
		{
			return false;
		}
		
		uintptr_t baseAddr = reinterpret_cast<uintptr_t>(info.AllocationBase);
		
		IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(baseAddr);
		IMAGE_NT_HEADERS* pe = reinterpret_cast<IMAGE_NT_HEADERS*>(baseAddr + dos->e_lfanew);
		IMAGE_FILE_HEADER* file = &pe->FileHeader;
		IMAGE_OPTIONAL_HEADER* opt = &pe->OptionalHeader;

		if(dos->e_magic != IMAGE_DOS_SIGNATURE || pe->Signature != IMAGE_NT_SIGNATURE || opt->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		{
			return false;
		}

		if(file->Machine != IMAGE_FILE_MACHINE_I386)
		{
			return false;
		}

		if((file->Characteristics & IMAGE_FILE_DLL) == 0)
		{
			return false;
		}
		
		m_size = opt->SizeOfImage;
		
#else
		uintptr_t baseAddr;
		
		if(handle)
		{
			baseAddr = reinterpret_cast<uintptr_t>(static_cast<const link_map*>(lib)->l_addr);
		}
		else
		{
			Dl_info info;
			
			if (!dladdr(lib, &info))
			{
				return false;
			}
			
			if (!info.dli_fname)
			{
				return false;
			}
			
			baseAddr = reinterpret_cast<uintptr_t>(info.dli_fbase);
		}
		
		if(!baseAddr)
		{
			return false;
		}
		
		Elf32_Ehdr* file = reinterpret_cast<Elf32_Ehdr*>(baseAddr);
		
		if(memcmp(ELFMAG, file->e_ident, SELFMAG) != 0)
		{
			return false;
		}
		
		if(file->e_ident[EI_VERSION] != EV_CURRENT)
		{
			return false;
		}
		
		if(file->e_ident[EI_CLASS] != ELFCLASS32 || file->e_machine != EM_386 || file->e_ident[EI_DATA] != ELFDATA2LSB)
		{
			return false;
		}
		
		if(file->e_type != ET_DYN)
		{
			return false;
		}
		
		uint16_t phdrCount = file->e_phnum;
		Elf32_Phdr* phdr = reinterpret_cast<Elf32_Phdr*>(baseAddr + file->e_phoff);
		
		for(uint16_t i = 0; i < phdrCount; i++)
		{
			Elf32_Phdr& hdr = phdr[i];
			
			if(hdr.p_type == PT_LOAD && hdr.p_flags == (PF_X|PF_R))
			{
				size_t pagesize = sysconf(_SC_PAGESIZE);
				m_size = (hdr.p_filesz + pagesize - 1) & ~(pagesize - 1);
				
				break;
			}
		}
		
#endif
		
		m_baseAddr = reinterpret_cast<uint8_t*>(baseAddr);
		
		return true;
	}
	
	template<typename T>
	T Find(const void* pattern, size_t size)
	{
		if(!m_baseAddr)
		{
			return T();
		}
		
		uint8_t* ptr = m_baseAddr;
		uint8_t* end = m_baseAddr + m_size - size;
		const uint8_t* _pattern = reinterpret_cast<const uint8_t*>(pattern);
		
		do
		{
			if (*ptr != _pattern[0] || *(ptr + size - 1) != _pattern[size - 1])
			{
				continue;
			}
			
			bool find = true;
			
			for(size_t i = 1; i < size - 1; i++)
			{
				if(_pattern[i] != ptr[i] && _pattern[i] != '\x2A')
				{
					find = false;
					
					break;
				}
			}
			
			if(find)
			{
				return reinterpret_cast<T>(ptr);
			}
		}
		while(++ptr < end);

		return T();
	}
	
private:
	uint8_t* m_baseAddr;
	size_t m_size;
};

#endif // _PATTERN_FINDER_HPP_