/*!	@file
	@brief Mutex管理

	@author ryoji
	@date 2007.07.05
*/
/*
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "_main/CMutex.h"

#include "basis/primitive.h"
#include "util/sync/HandleHolder.hpp"

/*!
 * @brief コンストラクタ
 *
 * 構築時にはミューテックスを作成しない。初回Lock()時に作成する。
 */
CMutex::CMutex(
	std::wstring_view name,
	LPSECURITY_ATTRIBUTES psa,
	bool bInitialOwner
)
	: m_Name{ name }
	, m_pSa{ psa }
	, m_bInitialOwner{ bInitialOwner }
{
}

/*!
 * @brief コンストラクタ
 *
 * 構築時にはミューテックスを作成しない。初回Lock()時に作成する。
 *
 * 既存コード互換用
 */
CMutex::CMutex(
	BOOL bInitialOwner,
	LPCWSTR pszName,
	LPSECURITY_ATTRIBUTES psa
)
	: CMutex(
		cxx::NullTerminatedString<WCHAR>{ pszName }.str(),
		psa,
		bInitialOwner != FALSE
	)
{
}

void CMutex::lock()
{
	Lock(INFINITE);
}

void CMutex::unlock() noexcept
{
	Unlock();
}

bool CMutex::Lock(
	DWORD dwTimeout
)
{
	if (!m_hObj) {
		m_hObj = MutexHandle::CreateMutexW(
			m_pSa,
			m_bInitialOwner,
			m_Name
		);
	}

	if (m_Lock) return true;

	bool bRet;
	try
	{
		cxx::WaitForSingleObject(m_hObj.get(), dwTimeout);

		m_Lock = m_hObj.get();

		bRet = true;
	}
	// エラーが発生したとき
	catch (const std::system_error&)
	{
		bRet = false;
	}

	return bRet;
}

void CMutex::Unlock() noexcept
{
	m_Lock = nullptr;
}
