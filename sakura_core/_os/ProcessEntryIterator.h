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

#ifndef _APISETHANDLE_
#error Need to include windows.h before ProcessEntryIterator.h
#endif  /* _APISETHANDLE_ */

#include <iterator>
#include <TlHelp32.h>


//名前空間
namespace sakura {
	namespace _os {


/*!
 * @brief ProcessEntryIterator
 *
 * 構築した瞬間のスナップショットを元にプロセス情報を検索する単方向イテレータ。
 *
 * 後置インクリメントは提供しない。
 * 比較演算子はループの終了判定以外の目的に使用しないこと。
 *
 * @author	berryzplus
 * @date	2018/06/25 berryzplus	新規作成
 */
class ProcessEntryIterator : public std::iterator<std::input_iterator_tag, PROCESSENTRY32>
{
	typedef std::iterator<std::input_iterator_tag, PROCESSENTRY32>	_Mybase;
	typedef ProcessEntryIterator									_Myt;

private:
	HANDLE				_hSnapshot;
	PROCESSENTRY32		_sFoundData;

public:
	/*!
	 * @brief 引数ありコンストラクタ(検索開始用)
	 *
	 * @param [in]		dwProcessId		プロセスID。
	 */
	ProcessEntryIterator(
		_In_ DWORD dwProcessId
	) noexcept
		: _hSnapshot(INVALID_HANDLE_VALUE)
	{
		// プロセスのスナップショットを作成する
		// これはwin xp以降のみ使える関数。32と付いてるが64兼用。
		// https://msdn.microsoft.com/en-us/library/ms682489(v=vs.85).aspx
		_hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwProcessId);
		if (_hSnapshot == INVALID_HANDLE_VALUE) {
			return;
		}

		// 最初の検索を実行する
		_sFoundData.dwSize = sizeof(decltype(_sFoundData));
		if (!::Process32First(_hSnapshot, &_sFoundData)) {
			Close();
		}
	}
	//引数なしコンストラクタ(end構築用)
	ProcessEntryIterator(void) noexcept
		: _hSnapshot(INVALID_HANDLE_VALUE)
	{
	}
	//デストラクタ
	virtual ~ProcessEntryIterator(void) noexcept
	{
		Close();
	}

public:
	//前置インクリメント
	_Myt & operator++(void) noexcept
	{
		//検索ハンドルが有効な場合のみ
		if (_hSnapshot != INVALID_HANDLE_VALUE)
		{
			//次を検索
			if (!::Process32Next(_hSnapshot, &_sFoundData))
			{
				Close();
			}
		}
		return (*this);
	}
	pointer   operator->(void) const noexcept { return const_cast< pointer >(&_sFoundData); }
	reference operator* (void) const noexcept { return const_cast< reference >(_sFoundData); }
	bool      operator==(const _Myt& rhs) const noexcept { return _hSnapshot == rhs._hSnapshot; }
	bool      operator!=(const _Myt& rhs) const noexcept { return !(*this == rhs); }

	ProcessEntryIterator(const _Myt& rhs) = delete;
	_Myt& operator = (const _Myt& rhs) = delete;
	_Myt operator++( int ) = delete;

protected:
	//検索が終わったときと、デストラクタが呼ばれたときに呼ばれる
	void Close(void) noexcept
	{
		//検索ハンドルが無効なら何もしない
		if (_hSnapshot == INVALID_HANDLE_VALUE)
		{
			return;
		}

		//有効な検索ハンドルが残っていれば閉じる
		::CloseHandle(_hSnapshot);
		_hSnapshot = INVALID_HANDLE_VALUE;
	}
};


	}; // end of namespace _os
}; // end of namespace sakura


   //名前空間を意識せずに扱えるようにusingしておく。
using sakura::_os::ProcessEntryIterator;
