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

#ifndef _STRING_
#error Need to include string before getMessageFromSystem.h
#endif  /* _STRING_ */

#if !defined(_WINBASE_) || !defined(_WINNT_)
#error Need to include windows.h before getMessageFromSystem.h
#endif  /* !defined(_WINBASE_) || !defined(_WINNT_) */


namespace _os {


/*!
 * @brief システムメッセージを取得する
 *
 *		Windows API呼出失敗時の定型処理を抽出したもの。
 *
 * @param [in]		dwMessageCode	メッセージコード。
 *									::GetLastError()の戻り値を指定する。
 * @param [in,opt]	dwLanguageId	言語識別子。
 *									省略した場合、ニュートラル言語。
 *									詳細は::FormatMessage()のマニュアルを参照。
 * @retval msg						システムメッセージ。
 *									該当するメッセージがない場合、空文字列を返す。
 * @sa https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-formatmessage
 * @date 2018/06/25 berryzplus		新規作成
 */
inline
std::wstring getMessageFromSystem(
	_In_ DWORD dwMessageCode,
	_In_opt_ DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
)
{
	HLOCAL pMsg = NULL;
	DWORD length = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwMessageCode,
		dwLanguageId,
		(LPWSTR)&pMsg,
		0,
		NULL
	);
	std::wstring msg((LPCWSTR)pMsg, length);
	::LocalFree(pMsg);
	return std::move(msg);
}


}; // end of namespace _os
