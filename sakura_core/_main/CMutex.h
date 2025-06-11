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

#include <Windows.h>

/** ミューテックスを扱うクラス
	@date 2007.07.05 ryoji 新規作成
*/
class CMutex
{
	using Me = CMutex;

public:
	CMutex( BOOL bInitialOwner, LPCWSTR pszName, LPSECURITY_ATTRIBUTES psa = NULL )
	{
		m_hObj = ::CreateMutex( psa, bInitialOwner, pszName );
	}
	CMutex(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CMutex(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CMutex()
	{
		if( NULL != m_hObj )
		{
			::CloseHandle( m_hObj );
			m_hObj = NULL;
		}
	}
	BOOL Lock( DWORD dwTimeout = INFINITE )
	{
		DWORD dwRet = ::WaitForSingleObject( m_hObj, dwTimeout );
		if( dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED )
			return TRUE;
		else
			return FALSE;
	}
	BOOL Unlock()
	{
		return ::ReleaseMutex( m_hObj );
	}
	operator HANDLE() const { return m_hObj; }
protected:
	HANDLE m_hObj;
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
template<class EXCLUSIVE_OBJECT>
class LockGuard {
	using Me = LockGuard< EXCLUSIVE_OBJECT>;

	EXCLUSIVE_OBJECT& o_;
public:
	LockGuard(EXCLUSIVE_OBJECT& ex) : o_( ex ){
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
