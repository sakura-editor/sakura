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

#ifndef _PROCESSTHREADSAPI_H_
#error Need to include string before ProcessPriority.h
#endif  /* _PROCESSTHREADSAPI_H_ */


//名前空間
namespace sakura {
	namespace _os {


/*!
 * @brief プロセス優先度クラス
 *   コンストラクタで優先度を設定し、デストラクタで元に戻す。
 *
 * @sa https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-setpriorityclass
 * @date 2018/07/02 berryzplus		新規作成
 */
class ProcessPriority {
private:
	HANDLE _hProcess;			//!< プロセスハンドル(キャッシュ)
	DWORD _dwDefaultPriority;	//!< 変更前のプロセス優先度

public:
	ProcessPriority(_In_ DWORD dwDesiredPriority)
		: _hProcess(::GetCurrentProcess())
		, _dwDefaultPriority(::GetPriorityClass(_hProcess)) {
		// プロセス優先度を設定する
		set(dwDesiredPriority);
	}
	virtual ~ProcessPriority() {
		reset();
	}

public:
	//!プロセス優先度を設定する
	void set(_In_ DWORD dwDesiredPriority) {
		if (dwDesiredPriority != _dwDefaultPriority) {
			BOOL bRet = ::SetPriorityClass(_hProcess, dwDesiredPriority);
			assert(bRet && "SetPriorityClass failed.");
		}
	}
	//!プロセス優先度を取得する
	DWORD get() const {
		return ::GetPriorityClass(_hProcess);
	}
	//!プロセス優先度を元に戻す
	void reset() {
		if (get() != _dwDefaultPriority) {
			BOOL bRet = ::SetPriorityClass(_hProcess, _dwDefaultPriority);
			assert(bRet && "SetPriorityClass failed.");
		}
	}
};


	}; // end of namespace _os
}; // end of namespace sakura


//名前空間を意識せずに扱えるようにusingしておく。
using sakura::_os::ProcessPriority;
