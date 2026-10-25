/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <map>
#include <string>

#include <Windows.h>
#include <DbgHelp.h>

/*!
 * @brief DbgHelp.DLLを扱うクラス
 *
 * DbgHelp.DLLはWindows標準のデバッグモジュール。
 * 他の外部DLLと実装を変えているのは「ほぼ間違いなく存在する」ため。
 */
class DbgHelp
{
private:
	using DllHolder = cxx::ResourceHolder<&::FreeLibrary>;

	using Me = DbgHelp;

protected:
	/*!
	 * @brief コンストラクタ
	 *
	 * DbgHelp APIの関数ポインタをロードする。
	 * DLLが存在しなかったり、必要なAPI関数が存在しなかったりすると例外を投げる。
	 *
	 * @throws std::system_error DbgHelp APIのロードに失敗した場合（≒正規品Windowsでない場合）
	 */
	DbgHelp();

public:
	DllHolder m_DbgHelp{ ::LoadLibraryW(L"DbgHelp.dll") };

	std::map<std::string, FARPROC, std::less<>> m_Functions{};

	HANDLE	m_Process = ::GetCurrentProcess();
	HANDLE	m_Thread = ::GetCurrentThread();
	DWORD	m_ProcessId = ::GetCurrentProcessId();
	DWORD	m_ThreadId = ::GetCurrentThreadId();
};
