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

	const auto dwRet = ::WaitForSingleObject(m_hObj.get(), dwTimeout);

	const auto bRet = dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED;
	if (bRet) {
		m_Lock = m_hObj.get();
	}

	return bRet;
}

void CMutex::Unlock() noexcept
{
	m_Lock = nullptr;
}
