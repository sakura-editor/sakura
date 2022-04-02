/*!	@file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#include "StdAfx.h"
#include "CDbgHelp.h"

#include "util/file.h"

LPCWSTR CDbgHelp::GetDllNameImp(int nIndex)
{
	return L"Dbghelp.dll";
}

bool CDbgHelp::InitDllImp()
{
	MiniDumpWriteDump;
	const ImportTable table[] = {
		{ (void*)&m_pfnMiniDumpWriteDump,		"MiniDumpWriteDump" },
		{ nullptr, 0 }
	};

	return RegisterEntries(table);
}

/*!
	@brief クラッシュダンプ
	
	@author ryoji
	@date 2009.01.21
 */
int CDbgHelp::WriteDump( PEXCEPTION_POINTERS pExceptPtrs )
{
	if (m_pfnMiniDumpWriteDump) {
		const auto dumpFile = GetIniFileName().replace_filename(L"sakura.dmp");
		if (HANDLE hFile = ::CreateFile(
			dumpFile.c_str(),
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
			nullptr);
			hFile && hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION* pInfo = nullptr;

			MINIDUMP_EXCEPTION_INFORMATION eInfo = {};
			if (pExceptPtrs) {
				eInfo.ThreadId = GetCurrentThreadId();
				eInfo.ExceptionPointers = pExceptPtrs;
				eInfo.ClientPointers = FALSE;
				pInfo = &eInfo;
			}

			m_pfnMiniDumpWriteDump(
				::GetCurrentProcess(),
				::GetCurrentProcessId(),
				hFile,
				MiniDumpNormal,
				pInfo,
				nullptr,
				nullptr);

			::CloseHandle(hFile);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

/*
	デバッグセッションを実行します。

	発生した例外は構造化例外で処理され、呼出元に伝播します。
	例外を握りつぶすわけではないので呼出元でキャッチしなかった場合、そのまま落ちます。

	@param [in] session 実行するコードをラムダで指定します。
 */
void CDbgHelp::DbgSession(const std::function<void()>& session)
{
	__try {
		session();
	}
	__except (WriteDump(GetExceptionInformation())) {
	}
}
