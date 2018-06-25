/*
  Copyright (C) 2018, Sakura Editor Organization

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Sakura Editor Organization
    https://sakura-editor.github.io/ 

 */

#pragma once

#if !defined(_WINBASE_) || !defined(_WINNT_)
#error Need to include windows.h before getMessageFromSystem.h
#endif  /* !defined(_WINBASE_) || !defined(_WINNT_) */

#include <Winternl.h> // internal APIs


//ñºëOãÛä‘
namespace sakura {
	namespace _os {


struct NtQueryInformationProcessT
{
	typedef NTSTATUS(NTAPI *pfn_NtQueryInformationProcess)(
		_In_ HANDLE ProcessHandle,
		_In_ PROCESSINFOCLASS ProcessInformationClass,
		_Out_ PVOID ProcessInformation,
		_In_ ULONG ProcessInformationLength,
		_Out_opt_ PULONG ReturnLengt
		);
	pfn_NtQueryInformationProcess _pfn;

	NtQueryInformationProcessT()
		: _pfn(NULL) {
		HMODULE ntdll = (HMODULE) ::GetModuleHandle(_T("ntdll"));
		assert(ntdll);

		_pfn = (pfn_NtQueryInformationProcess)::GetProcAddress(ntdll, "NtQueryInformationProcess");
		assert(_pfn);
	}
	NTSTATUS NTAPI operator() (
		_In_ HANDLE ProcessHandle,
		_In_ PROCESSINFOCLASS ProcessInformationClass,
		_Out_ PVOID ProcessInformation,
		_In_ ULONG ProcessInformationLength,
		_Out_opt_ PULONG ReturnLength
		) const noexcept
	{
		NTSTATUS status = _pfn(
			ProcessHandle,
			ProcessInformationClass,
			ProcessInformation,
			ProcessInformationLength,
			ReturnLength);
		return status;
	}
};


	}; // end of namespace _os
}; // end of namespace sakura


   //ñºëOãÛä‘Çà”éØÇπÇ∏Ç…àµÇ¶ÇÈÇÊÇ§Ç…usingÇµÇƒÇ®Ç≠ÅB
using sakura::_os::NtQueryInformationProcessT;
