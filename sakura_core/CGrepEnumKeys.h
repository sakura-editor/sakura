/*!	@file
	
	@brief GREP support library
	
	@author wakura, Moca
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2011, Moca

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

#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "util/string_ex.h"
#include "util/file.h"

typedef std::vector< LPCTSTR > VGrepEnumKeys;

class CGrepEnumKeys {
	static constexpr TCHAR WILDCARD_DELIMITER[] = _T(" ;,");	//リストの区切り
	static constexpr TCHAR WILDCARD_ANY[] = _T("*.*");			//サブフォルダ探索用

public:
	VGrepEnumKeys m_vecSearchFileKeys;
	VGrepEnumKeys m_vecSearchFolderKeys;

	VGrepEnumKeys m_vecExceptFileKeys;
	VGrepEnumKeys m_vecExceptFolderKeys;

	VGrepEnumKeys m_vecExceptAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFolderKeys;

public:
	CGrepEnumKeys() = default;
	virtual ~CGrepEnumKeys() noexcept;

	/*!
	 * Grepダイアログのファイルパターンを解析して分解する
	 *
	 * @param lpKeys ファイルパターン
	 * @retval -1 不明なエラー(std::bad_alloc)
	 * @retval 1 *\file.exe などのフォルダ部分でのワイルドカードはエラー
	 * @retval 2 絶対パス指定は不可
	 */
	int SetFileKeys(_In_z_ LPCTSTR lpKeys) noexcept;

	static void AddExcludeFiles(_Inout_ SFilePathLong& szFile, _In_z_ LPCTSTR pszExcludeFile, _In_opt_ TCHAR chDelimiter = _T(';'));
	static void AddExcludeFolders(_Inout_ SFilePathLong& szFile, _In_z_ LPCTSTR pszExcludeFolder, _In_opt_ TCHAR chDelimiter = _T(';'));

	void GetSearchFile(SFilePathLong& szFile);
	void GetExcludeFile(SFilePathLong& szExcludeFile);
	void GetExcludeFolder(SFilePathLong& szExcludeFolder);

	/*!
	 * 共有メモリに保存するファイルパターン文字列を組み立てる
	 *
	 * @param chDelimiter 区切り文字
	 * @retval CShareDataのファイルパターン履歴に入れる文字列
	 */
	void GetFileKeys(SFilePathLong& szFile, _In_opt_ TCHAR chDelimiter = _T(';'));

private:
	void ClearItems() noexcept;

};


