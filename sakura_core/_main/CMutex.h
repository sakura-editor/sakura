/*!	@file
	@brief Mutex管理

	@author ryoji
	@date 2007.07.05
*/
/*
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMUTEX_51EDDE78_F635_419A_9E10_159485D0F710_H_
#define SAKURA_CMUTEX_51EDDE78_F635_419A_9E10_159485D0F710_H_
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "util/sync/MutexHandle.hpp"

#include <Windows.h>

/*!
 * @brief ミューテックスを扱うクラス
 *
 * @date 2007/07/05 ryoji 新規作成
 */
class CMutex
{
private:
	using MutexHandle = cxx::MutexHandle;
	using LockHolder = cxx::ResourceHolder<&::ReleaseMutex>;

	using Me = CMutex;

public:
	explicit CMutex(
		std::wstring_view name,
		LPSECURITY_ATTRIBUTES psa = nullptr,
		bool bInitialOwner = false
	);

	CMutex(
		BOOL bInitialOwner,
		LPCWSTR pszName,
		LPSECURITY_ATTRIBUTES psa = nullptr
	);

	CMutex(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	void lock();
	void unlock() noexcept;

	bool Lock(
		DWORD dwTimeout = INFINITE
	);

	void Unlock() noexcept;

	operator HANDLE() const { return m_hObj; }

private:
	std::wstring_view m_Name;
	LPSECURITY_ATTRIBUTES m_pSa;
	bool m_bInitialOwner;

	MutexHandle m_hObj{};
	LockHolder	m_Lock{};
};

/**	スコープから抜けると同時にロックを解除する．

	@date 2007.07.07 genta 新規作成

	@code
	CMutex aMutex;
	
    void function()
    {
        //  other processing
        {
            LockGuard<CMutex> aGuard(aMutex);
            //  aMutex is locked
            //  do something protected by "aMutex"

        } // aMutex is automatically released
        //  other processing
    }
	@endcode
*/
//TODO: いつかコメント外す
//[[deprecated( "Use std::lock_guard or std::unique_lock instead" )]] ]
template<class EXCLUSIVE_OBJECT>
class LockGuard {
	using Me = LockGuard< EXCLUSIVE_OBJECT>;

	EXCLUSIVE_OBJECT& o_;

public:
	explicit LockGuard(EXCLUSIVE_OBJECT& ex) : o_( ex ){
		o_.Lock();
	}
	template<class PARAM>
	LockGuard(EXCLUSIVE_OBJECT& ex, PARAM p) : o_( ex ){
		o_.Lock(p);
	}
	LockGuard(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	LockGuard(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~LockGuard() {
		o_.Unlock();
	}
};

#endif /* SAKURA_CMUTEX_51EDDE78_F635_419A_9E10_159485D0F710_H_ */
