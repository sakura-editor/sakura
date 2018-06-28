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
#error Need to include string before SkipExeNameOfCommandLine.h
#endif  /* _STRING_ */

#ifndef _WINNT_
#error Need to include windows.h before SkipExeNameOfCommandLine.h
#endif  /* _WINNT_ */


namespace _os {


/*!
 * @brief WinMainの引数「コマンドライン文字列」を取得する
 *
 *  入力: "\"C:\Program Files (x86)\\sakura\\sakura.exe\" -NOWIN"
 *  出力: "-NOWIN"
 *  ※ポインタ位置を進めているだけ。
 *
 * @param [in] lpCmdLine GetCommandLineW()の戻り値。
 * @retval lpCmdLine wWinMain形式のコマンドライン文字列。
 */
inline
_Ret_z_ LPWSTR SkipExeNameOfCommandLine(_In_z_ LPWSTR lpCmdLine) noexcept
{
	// 内部定数(空白文字)
	const WCHAR whiteSpace[] = L"\t\x20";

	// 文字列がダブルクォーテーションで始まっているかチェック
	if (L'\x22' == lpCmdLine[0]) {
		// 文字列ポインタを進める
		lpCmdLine++;
		// 閉じクォーテーションを探す(パス文字列なのでエスケープの考慮は不要)
		WCHAR *p = ::wcschr(lpCmdLine, L'\x22');
		if (p) {
			// 文字列ポインタを進める
			lpCmdLine = ++p;
		}
	}
	else {
		// 最初のトークンをスキップする
		// ※Windows 環境で実行する場合、この部分はデッドコードになる
		//   Wine等によるエミュレータ実行を考慮して実装だけはしておく
		size_t nPos = ::wcscspn(lpCmdLine, whiteSpace);
		lpCmdLine = &lpCmdLine[nPos];
	}

	// 次のトークンまで進める
	size_t nPos = ::wcsspn(lpCmdLine, whiteSpace);
	return &lpCmdLine[nPos];
}


}; // end of namespace _os
