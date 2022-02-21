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

#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#ifdef _WIN32 // Windows
#include <Windows.h>
#elif __linux__ // Linux
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include <libgen.h>
#else // Unknown
#error Unsupported operating system
#endif

#include <cstdint>
#include <time.h>
#include <strtools.h>
#include <dbg.h>

#ifdef _WIN32
#define WIN_LINUX(win, linux) win
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#else
#define WIN_LINUX(win, linux) linux
#endif

template<typename T>
inline void UnprotectMem(T address, size_t size)
{
#ifdef _WIN32
	DWORD old_flags;
	VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &old_flags);
#else
	uintptr_t addr = reinterpret_cast<uintptr_t>(address);
	uintptr_t alignAddr = addr & ~(sysconf(_SC_PAGESIZE) - 1);
	size_t fullSize = addr - alignAddr + size;
	mprotect(reinterpret_cast<void*>(alignAddr), fullSize, PROT_READ|PROT_WRITE|PROT_EXEC);
#endif
}

inline void GetExecutablePath(char* path, size_t size)
{
#ifdef _WIN32
	GetModuleFileNameA(reinterpret_cast<HINSTANCE>(&__ImageBase), path, size);
#else
	Dl_info info;
	dladdr(reinterpret_cast<void*>(GetExecutablePath), &info);
	V_strncpy(path, info.dli_fname, size);
#endif
}

inline void FatalError(const char* fmt, ...)
{
	va_list argptr;
	char msg[1024];
	
	va_start(argptr, fmt);
	Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);
	
	Warning("%s\n", msg);
	
	char path[MAX_PATH];
	GetExecutablePath(path, sizeof(path));
	V_StripExtension(path, path, sizeof(path));
	V_strcat_safe(path, "-fatal.log");
	
	FILE* file = fopen(path, "a");
	if(file)
	{
		char header[32];
		
		time_t rawtime = time(nullptr);
		strftime(header, sizeof(header), "%m/%d/%Y - %H:%M:%S", localtime(&rawtime));
		
		fprintf(file, "%s: %s\n", header, msg);
		
		fclose(file);
	}
}

#endif // _UTILS_HPP_