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

#include <tchar.h>
#include <windows.h>

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iomanip>

#include "util/string_ex.h"
#include "util/file.h"

typedef std::vector< LPCTSTR > VGrepEnumKeys;

//! リストの区切りを示すリテラル
constexpr const TCHAR GREP_WILDCARD_DELIMITERS[] = _T(", ;");

//! サブフォルダ探索用パターンのリテラル
constexpr const TCHAR GREP_WILDCARD_ANY[] = _T("*.*");

//! 空の除外パターンを示すリテラル
constexpr const TCHAR GREP_ACCEPT_EVERYTHING[] = _T("");

class CGrepEnumKeys {
public:
	VGrepEnumKeys m_vecSearchFileKeys;
	VGrepEnumKeys m_vecSearchFolderKeys;
	VGrepEnumKeys m_vecExceptFileKeys;
	VGrepEnumKeys m_vecExceptFolderKeys;

//	VGrepEnumKeys m_vecSearchAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFolderKeys;

	typedef std::basic_string<TCHAR> tstring;

public:
	CGrepEnumKeys(){
	}

	~CGrepEnumKeys(){
		ClearItems();
	}

	int SetFileKeys( LPCTSTR lpKeys ){
		ClearItems();
		
		std::vector< tstring > patterns = SplitPattern(lpKeys);
		for (size_t i = 0; i < patterns.size(); i++) {
			const tstring& element = patterns[i];
			const TCHAR* token = element.c_str();

			//フィルタを種類ごとに振り分ける
			enum KeyFilterType{
				FILTER_SEARCH,
				FILTER_EXCEPT_FILE,
				FILTER_EXCEPT_FOLDER,
			};
			KeyFilterType keyType = FILTER_SEARCH;
			if( token[0] == _T('!') ){
				token++;
				keyType = FILTER_EXCEPT_FILE;
			}else if( token[0] == _T('#') ){
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
			push_back_unique( m_vecSearchFileKeys, GREP_WILDCARD_ANY );
		}
		if( m_vecSearchFolderKeys.size() == 0 ){
			push_back_unique( m_vecSearchFolderKeys, GREP_WILDCARD_ANY );
		}
		return 0;
	}

	/*
		@brief パターンをエスケープする必要があるか判断する
		@param[in]     pattern チェックするパターン
		@return        true  エスケープする必要がある
		@return        false エスケープする必要がない
	*/
	bool CheckMetaChars( const tstring & pattern ) const
	{
		// 先頭が!または#の場合、エスケープ要
		if ( 1 <= pattern.length()
			&& (pattern[0] == '!' || pattern[0] == '#') )
		{
			return true;
		}
		// 先頭と末尾が"(ダブルクォート)の場合、エスケープ不要
		if ( 2 <= pattern.length()
			&& pattern[0] == '"'
			&& pattern[pattern.length() - 1] == '"' )
		{
			return false;
		}
		// 区切り記号(, ;)を含む場合、エスケープ要
		if ( ::_tcscspn( pattern.c_str(), GREP_WILDCARD_DELIMITERS ) < pattern.length() )
		{
			return true;
		}
		// 上記以外はエスケープ不要
		return false;
	}

	/*!
		@brief ファイルパターン、除外ファイルパターンを追加する
		@param[in]	includeFiles	ファイルパターン
		@param[in]	excludeFiles	除外ファイルパターン
		@param[in]	excludeDirs		除外フォルダパターン
	*/
	int SetFileKeys(
		const tstring &includeFiles,
		const tstring &excludeFiles,
		const tstring &excludeDirs )
	{
		int validationResult = SetFileKeys( includeFiles.c_str() );
		if ( validationResult ) return validationResult;
		validationResult = SplitAndClassify( excludeFiles.c_str(), m_vecExceptFileKeys, m_vecExceptAbsFileKeys );
		if ( validationResult ) return validationResult;
		validationResult = SplitAndClassify( excludeDirs.c_str(), m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys );
		return validationResult;
	}

	tstring GetTextForIncludeFiles( void ) const
	{
		return rangeToString( m_vecSearchFileKeys.cbegin(), m_vecSearchFileKeys.cend(), _T(""), GREP_WILDCARD_ANY );
	}

	tstring GetTextForExcludeFiles( const tstring &prefixStr = _T("") ) const
	{
		std::list<VGrepEnumKeys::value_type> list;
		std::copy( m_vecExceptFileKeys.cbegin(), m_vecExceptFileKeys.cend(), std::back_inserter( list ) );
		std::copy( m_vecExceptAbsFileKeys.cbegin(), m_vecExceptAbsFileKeys.cend(), std::back_inserter( list ) );
		return rangeToString( list.cbegin(), list.cend(), prefixStr, GREP_ACCEPT_EVERYTHING );
	}

	tstring GetTextForExcludeDirs( const tstring &prefixStr = _T("") ) const
	{
		std::list<VGrepEnumKeys::value_type> list;
		std::copy( m_vecExceptFolderKeys.cbegin(), m_vecExceptFolderKeys.cend(), std::back_inserter( list ) );
		std::copy( m_vecExceptAbsFolderKeys.cbegin(), m_vecExceptAbsFolderKeys.cend(), std::back_inserter( list ) );
		return rangeToString( list.cbegin(), list.cend(), prefixStr, GREP_ACCEPT_EVERYTHING );
	}

	tstring GetCmdOptionForGFile( void ) const
	{
		std::basic_ostringstream<TCHAR> out;
		if ( 0 < m_vecSearchFileKeys.size() ) {
			out << GetTextForIncludeFiles() << _T(';');
		}
		if ( 0 < m_vecExceptFileKeys.size() + m_vecExceptAbsFileKeys.size() ) {
			out << GetTextForExcludeFiles(_T("!")) << _T(';');
		}
		if ( 0 < m_vecExceptFolderKeys.size() + m_vecExceptAbsFolderKeys.size() ) {
			out << GetTextForExcludeDirs(_T("#")) << _T(';');
		}
		return trimLastCharFromSStream( std::move( out ) );
	}

	/*!
		@brief ファイルパターンを解析して、要素ごとに分離して返す
		@param[in]		lpKeys					ファイルパターン
	*/
	static std::vector< tstring > SplitPattern(LPCTSTR lpKeys)
	{
		std::vector< tstring > patterns;

		int nWildCardLen = _tcslen(lpKeys);
		TCHAR* pWildCard = new TCHAR[nWildCardLen + 1];
		if (!pWildCard) {
			return patterns;
		}
		_tcscpy(pWildCard, lpKeys);

		int nPos = 0;
		TCHAR*	token;
		while (NULL != (token = my_strtok<TCHAR>(pWildCard, nWildCardLen, &nPos, GREP_WILDCARD_DELIMITERS))) {	//トークン毎に繰り返す。
			// "を取り除いて左に詰める
			TCHAR* p;
			TCHAR* q;
			p = q = token;
			while (*p) {
				if (*p != _T('"')) {
					if (p != q) {
						*q = *p;
					}
					q++;
				}
				p++;
			}
			*q = _T('\0');

			tstring element(token);
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

	void push_back_unique( VGrepEnumKeys& keys, LPCTSTR addKey ){
		if( ! IsExist( keys, addKey) ){
			TCHAR* newKey = new TCHAR[ _tcslen( addKey ) + 1 ];
			_tcscpy( newKey, addKey );
			keys.push_back( newKey );
		}
	}

	BOOL IsExist( VGrepEnumKeys& keys, LPCTSTR addKey ){
		for( int i = 0; i < (int)keys.size(); i++ ){
			if( _tcscmp( keys[ i ], addKey ) == 0 ){
				return TRUE;
			}
		}
		return FALSE;
	}

	/*
		@retval 0 正常終了
		@retval 1 *\file.exe などのフォルダ部分でのワイルドカードはエラー
	*/
	int ValidateKey( LPCTSTR key ){
		// 
		bool wildcard = false;
		for( int i = 0; key[i]; i++ ){
			if( !wildcard && (key[i] == _T('*') || key[i] == _T('?')) ){
				wildcard = true;
			}else if( wildcard && (key[i] == _T('\\') || key[i] == _T('/')) ){
				return 1;
			}
		}
		return 0;
	}

	/*!
		@brief パターンを分割して仕訳する
		@param[in]		pszPattern		パターン
		@param[in,out]	outRelKeys		相対パスのパターンの解析結果を追加する
		@param[in,out]	outAbsKeys		絶対パスのパターンの解析結果を追加する
	*/
	int SplitAndClassify( LPCTSTR pszPattern, VGrepEnumKeys &outRelKeys, VGrepEnumKeys &outAbsKeys) {
		std::vector< tstring > patterns = SplitPattern(pszPattern);

		for (size_t i = 0; i < patterns.size(); i++) {
			const tstring& element = patterns[i];
			const TCHAR* token = element.c_str();

			bool bRelPath = _IS_REL_PATH(token);
			int nValidStatus = ValidateKey(token);
			if (0 != nValidStatus) {
				return nValidStatus;
			}
			if (bRelPath) {
				push_back_unique(outRelKeys, token);
			}
			else {
				push_back_unique(outAbsKeys, token);
			}
		}
		return 0;
	}

	/*
		@brief コレクションを文字列化する
	*/
	template<typename Iter>
	tstring rangeToString( Iter && _First, Iter && _Last,
		const tstring &prefixStr, const tstring &valueForEmpty ) const
	{
		if ( _First == _Last ) return valueForEmpty;

		std::basic_ostringstream<TCHAR> out;
		for ( ; _First != _Last; ++_First ) {
			tstring value( *_First );
			if ( CheckMetaChars( value ) ) {
				value.insert( value.cbegin(), _T('\"') );
				value.insert( value.cend(), _T('\"') );
			}
			out << prefixStr << value << _T(';');
		}

		return trimLastCharFromSStream( std::move( out ) );
	}

	/*
		@brief ostringstreamの末尾1文字を削って文字列化する
	*/
	inline tstring trimLastCharFromSStream( std::basic_ostringstream<TCHAR> &&out ) const
	{
		out.seekp( -1, std::ios_base::cur );
		if ( !out.fail() ) {
			out << _T('\0');
		}
		return tstring( out.str().c_str() );
	}
};
