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

#include <list>
#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "util/string_ex.h"
#include "util/file.h"

typedef std::vector< LPCWSTR > VGrepEnumKeys;

class CGrepEnumKeys {
public:
	VGrepEnumKeys m_vecSearchFileKeys;
	VGrepEnumKeys m_vecSearchFolderKeys;
	VGrepEnumKeys m_vecExceptFileKeys;
	VGrepEnumKeys m_vecExceptFolderKeys;

//	VGrepEnumKeys m_vecSearchAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFolderKeys;

public:
	CGrepEnumKeys(){
	}

	~CGrepEnumKeys(){
		ClearItems();
	}

	// 除外ファイルの2つの解析済み配列から1つのリストを作る
	auto GetExcludeFiles() const ->  std::list<decltype(m_vecExceptFileKeys)::value_type> {
		std::list<decltype(m_vecExceptFileKeys)::value_type> excludeFiles;
		const auto& fileKeys = m_vecExceptFileKeys;
		excludeFiles.insert( excludeFiles.cend(), fileKeys.cbegin(), fileKeys.cend() );
		const auto& absFileKeys = m_vecExceptAbsFileKeys;
		excludeFiles.insert( excludeFiles.cend(), absFileKeys.cbegin(), absFileKeys.cend() );
		return excludeFiles;
	}

	// 除外フォルダの2つの解析済み配列から1つのリストを作る
	auto GetExcludeFolders() const ->  std::list<decltype(m_vecExceptFolderKeys)::value_type> {
		std::list<decltype(m_vecExceptFolderKeys)::value_type> excludeFolders;
		const auto& folderKeys = m_vecExceptFolderKeys;
		excludeFolders.insert( excludeFolders.cend(), folderKeys.cbegin(), folderKeys.cend() );
		const auto& absFolderKeys = m_vecExceptAbsFolderKeys;
		excludeFolders.insert( excludeFolders.cend(), absFolderKeys.cbegin(), absFolderKeys.cend() );
		return excludeFolders;
	}

	int SetFileKeys( LPCWSTR lpKeys ){
		const WCHAR* WILDCARD_ANY = L"*.*";	//サブフォルダ探索用
		ClearItems();
		
		std::vector< wstring > patterns = SplitPattern(lpKeys);
		for (size_t i = 0; i < patterns.size(); i++) {
			const wstring& element = patterns[i];
			const WCHAR* token = element.c_str();

			//フィルタを種類ごとに振り分ける
			enum KeyFilterType{
				FILTER_SEARCH,
				FILTER_EXCEPT_FILE,
				FILTER_EXCEPT_FOLDER,
			};
			KeyFilterType keyType = FILTER_SEARCH;
			if( token[0] == L'!' ){
				token++;
				keyType = FILTER_EXCEPT_FILE;
			}else if( token[0] == L'#' ){
				token++;
				keyType = FILTER_EXCEPT_FOLDER;
			}

			bool bRelPath = _IS_REL_PATH( token );
			int nValidStatus = ValidateKey( token );
			if( 0 != nValidStatus ){

				return nValidStatus;
			}
			if( keyType == FILTER_SEARCH ){
				if( bRelPath ){
					push_back_unique( m_vecSearchFileKeys, token );
				}else{
//					push_back_unique( m_vecSearchAbsFileKeys, token );
//					push_back_unique( m_vecSearchFileKeys, token );
					return 2; // 絶対パス指定は不可
				}
			}else if( keyType == FILTER_EXCEPT_FILE ){
				if( bRelPath ){
					push_back_unique( m_vecExceptFileKeys, token );
				}else{
					push_back_unique( m_vecExceptAbsFileKeys, token );
				}
			}else if( keyType == FILTER_EXCEPT_FOLDER ){
				if( bRelPath ){
					push_back_unique( m_vecExceptFolderKeys, token );
				}else{
					push_back_unique( m_vecExceptAbsFolderKeys, token );
				}
			}
		}
		if( m_vecSearchFileKeys.size() == 0 ){
			push_back_unique( m_vecSearchFileKeys, WILDCARD_ANY );
		}
		if( m_vecSearchFolderKeys.size() == 0 ){
			push_back_unique( m_vecSearchFolderKeys, WILDCARD_ANY );
		}
		return 0;
	}

	/*!
		@brief 除外ファイルパターンを追加する
		@param[in]	lpKeys	除外ファイルパターン
	*/
	int AddExceptFile(LPCWSTR lpKeys) {
		return ParseAndAddException(lpKeys, m_vecExceptFileKeys, m_vecExceptAbsFileKeys);
	}

	/*!
		@brief 除外フォルダパターンを追加する
		@param[in]	lpKeys	除外フォルダパターン
	*/
	int AddExceptFolder(LPCWSTR lpKeys) {
		return ParseAndAddException(lpKeys, m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);
	}

	typedef std::basic_string<WCHAR> wstring;

	/*!
		@brief ファイルパターンを解析して、要素ごとに分離して返す
		@param[in]		lpKeys					ファイルパターン
	*/
	static std::vector< wstring > SplitPattern(LPCWSTR lpKeys)
	{
		std::vector< wstring > patterns;

		const WCHAR* WILDCARD_DELIMITER = L" ;,";	//リストの区切り
		int nWildCardLen = wcslen(lpKeys);
		WCHAR* pWildCard = new WCHAR[nWildCardLen + 1];
		if (!pWildCard) {
			return patterns;
		}
		wcscpy(pWildCard, lpKeys);

		int nPos = 0;
		WCHAR*	token;
		while (NULL != (token = my_strtok<WCHAR>(pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER))) {	//トークン毎に繰り返す。
			// "を取り除いて左に詰める
			WCHAR* p;
			WCHAR* q;
			p = q = token;
			while (*p) {
				if (*p != L'"') {
					if (p != q) {
						*q = *p;
					}
					q++;
				}
				p++;
			}
			*q = L'\0';

			wstring element(token);
			patterns.push_back(element);
		}
		delete[] pWildCard;
		return patterns;
	}

private:
	void ClearItems( void ){
		ClearEnumKeys(m_vecExceptFileKeys);
		ClearEnumKeys(m_vecSearchFileKeys);
		ClearEnumKeys(m_vecExceptFolderKeys);
		ClearEnumKeys(m_vecSearchFolderKeys);
		return;
	}
	void ClearEnumKeys( VGrepEnumKeys& keys ){
		for( int i = 0; i < (int)keys.size(); i++ ){
			delete [] keys[ i ];
		}
		keys.clear();
	}

	void push_back_unique( VGrepEnumKeys& keys, LPCWSTR addKey ){
		if( ! IsExist( keys, addKey) ){
			WCHAR* newKey = new WCHAR[ wcslen( addKey ) + 1 ];
			wcscpy( newKey, addKey );
			keys.push_back( newKey );
		}
	}

	BOOL IsExist( VGrepEnumKeys& keys, LPCWSTR addKey ){
		for( int i = 0; i < (int)keys.size(); i++ ){
			if( wcscmp( keys[ i ], addKey ) == 0 ){
				return TRUE;
			}
		}
		return FALSE;
	}

	/*
		@retval 0 正常終了
		@retval 1 *\file.exe などのフォルダ部分でのワイルドカードはエラー
	*/
	int ValidateKey( LPCWSTR key ){
		// 
		bool wildcard = false;
		for( int i = 0; key[i]; i++ ){
			if( !wildcard && (key[i] == L'*' || key[i] == L'?') ){
				wildcard = true;
			}else if( wildcard && (key[i] == L'\\' || key[i] == L'/') ){
				return 1;
			}
		}
		return 0;
	}

	/*!
		@brief 除外ファイルパターンを追加する
		@param[in]		lpKeys					除外ファイルパターン
		@param[in,out]	exceptionKeys			除外ファイルパターンの解析結果を追加する
		@param[in,out]	exceptionAbsoluteKeys	除外ファイルパターンの絶対パスの解析結果を追加する
	*/
	int ParseAndAddException(LPCWSTR lpKeys, VGrepEnumKeys& exceptionKeys, VGrepEnumKeys & exceptionAbsoluteKeys) {
		std::vector< wstring > patterns = SplitPattern(lpKeys);

		for (size_t i = 0; i < patterns.size(); i++) {
			const wstring& element = patterns[i];
			const WCHAR* token = element.c_str();

			bool bRelPath = _IS_REL_PATH(token);
			int nValidStatus = ValidateKey(token);
			if (0 != nValidStatus) {
				return nValidStatus;
			}
			if (bRelPath) {
				push_back_unique(exceptionKeys, token);
			}
			else {
				push_back_unique(exceptionAbsoluteKeys, token);
			}
		}
		return 0;
	}
};
