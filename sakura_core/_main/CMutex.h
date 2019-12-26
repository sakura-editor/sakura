/*!	@file
	@brief Mutex管理

	@author ryoji
	@date 2007.07.05
*/
/*
	Copyright (C) 2007, ryoji, genta

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

#pragma once

#include <Windows.h>
#include <string>
#include <string_view>

/*!
 * ミューテックスを扱うクラス
 * 
 * @date 2007/07/05 ryoji 新規作成
 */
class CMutex
{
public:
	/*!
	 * コンストラクタ
	 *
	 * @param [in] name mutexの名前
	 */
	CMutex( std::wstring_view name ) noexcept
		: m_strName( name )
		, m_hObj( NULL )
		, m_bOwnership( false )
	{
	}

	/*!
	 * デストラクタ
	 * 
	 * 確保済みのすべてのリソースを開放する
	 */
	~CMutex() noexcept
	{
		Unlock();
		if( m_hObj )
		{
			::CloseHandle( m_hObj );
			m_hObj = NULL;
		}
	}

	/*!
	 * mutexの所有権を獲得する
	 *
	 * @param [in] dwTimeout タイムアウトまでの時間。単位はミリ秒。
	 */
	BOOL Lock( DWORD dwTimeout = INFINITE ) noexcept
	{
		// 所有権獲得済みなら直ちに抜ける
		if( owning() ){
			return TRUE;
		}

		// mutex未作成なら作成する
		if( !m_hObj ){
			// 引数は、handleを継承しない、初期所有権を要求しない、NULLでない名前。
			m_hObj = ::CreateMutex( NULL, FALSE, GetName() );
		}

		// mutexが取得できるまで待機。
		BOOL bRet = FALSE;
		if( m_hObj ){
			DWORD dwRet = ::WaitForSingleObject( m_hObj, dwTimeout );
			if( dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED ){
				bRet = TRUE;
				m_bOwnership = true;
			}
		}
		return bRet;
	}

	/*!
	 * mutexの所有権を手放す
	 */
	BOOL Unlock() noexcept
	{
		BOOL bRet = FALSE;
		if( m_hObj && owning() ){
			bRet = ::ReleaseMutex( m_hObj );
			m_bOwnership = false;
		}
		return bRet;
	}

	//! mutexの名前を取得する
	std::wstring_view name() const noexcept { return m_strName.data(); }
	//! mutexのハンドルがあるか判定する
	bool valid() const noexcept { return (m_hObj != NULL); }
	//! mutexの所有権があるか判定する
	bool owning() const noexcept { return m_bOwnership; }

	//! mutexの名前を取得する(C-Style)
	const wchar_t* GetName() const noexcept { return name().data(); }

protected:
	std::wstring	m_strName;		//!< mutexの名前
	HANDLE			m_hObj;			//!< mutexのhandle
	bool			m_bOwnership;	//!< mutexの所有権を持っているかどうか
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
	EXCLUSIVE_OBJECT& o_;
public:
	LockGuard(EXCLUSIVE_OBJECT& ex) : o_( ex ){
		o_.Lock();
	}
	template<class PARAM>
	LockGuard(EXCLUSIVE_OBJECT& ex, PARAM p) : o_( ex ){
		o_.Lock(p);
	}
	
	~LockGuard() {
		o_.Unlock();
	}
};

