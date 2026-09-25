/*!	@file
	
	@brief GREP support library
	
	@author wakura, Moca
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2011, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPENUMKEYS_FCE5732F_FA0C_4CB2_90D9_D1D440841D5C_H_
#define SAKURA_CGREPENUMKEYS_FCE5732F_FA0C_4CB2_90D9_D1D440841D5C_H_
#pragma once

#include <algorithm>
#include <list>
#include <string>
#include <string_view>
#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "util/string_ex.h"
#include "util/file.h"

using VGrepEnumKeys = std::vector< std::wstring >;

class CGrepEnumKeys {

	using Me = CGrepEnumKeys;

public:
	VGrepEnumKeys m_vecSearchFileKeys;
	VGrepEnumKeys m_vecSearchFolderKeys;
	VGrepEnumKeys m_vecExceptFileKeys;
	VGrepEnumKeys m_vecExceptFolderKeys;

//	VGrepEnumKeys m_vecSearchAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFolderKeys;

public:
	CGrepEnumKeys() noexcept = default;
	CGrepEnumKeys(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CGrepEnumKeys(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CGrepEnumKeys() = default;

	// 除外ファイルの2つの解析済み配列から1つのリストを作る
	auto GetExcludeFiles() const ->  std::vector<decltype(m_vecExceptFileKeys)::value_type> {
		std::vector<decltype(m_vecExceptFileKeys)::value_type> excludeFiles;
		const auto& fileKeys = m_vecExceptFileKeys;
		excludeFiles.insert( excludeFiles.cend(), fileKeys.cbegin(), fileKeys.cend() );
		const auto& absFileKeys = m_vecExceptAbsFileKeys;
		excludeFiles.insert( excludeFiles.cend(), absFileKeys.cbegin(), absFileKeys.cend() );
		return excludeFiles;
	}

	// 除外フォルダーの2つの解析済み配列から1つのリストを作る
	auto GetExcludeFolders() const ->  std::vector<decltype(m_vecExceptFolderKeys)::value_type> {
		std::vector<decltype(m_vecExceptFolderKeys)::value_type> excludeFolders;
		const auto& folderKeys = m_vecExceptFolderKeys;
		excludeFolders.insert( excludeFolders.cend(), folderKeys.cbegin(), folderKeys.cend() );
		const auto& absFolderKeys = m_vecExceptAbsFolderKeys;
		excludeFolders.insert( excludeFolders.cend(), absFolderKeys.cbegin(), absFolderKeys.cend() );
		return excludeFolders;
	}

	int SetFileKeys( LPCWSTR lpKeys ){
		const WCHAR* WILDCARD_ANY = L"*.*";	//サブフォルダー探索用
		ClearItems();
		
		std::vector< std::wstring > patterns = SplitPattern(lpKeys);
		for (size_t i = 0; i < patterns.size(); i++) {
			const std::wstring& element = patterns[i];
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
		@brief 除外フォルダーパターンを追加する
		@param[in]	lpKeys	除外フォルダーパターン
	*/
	int AddExceptFolder(LPCWSTR lpKeys) {
		return ParseAndAddException(lpKeys, m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);
	}

	/*!
		@brief ファイルパターンを解析して、要素ごとに分離して返す
		@param[in]		lpKeys					ファイルパターン
		@return 引用符を取り除いた要素の配列
	*/
	static std::vector< std::wstring > SplitPattern(LPCWSTR lpKeys)
	{
		std::vector< std::wstring > patterns;
		for (const auto& quoted : SplitPatternKeepQuotes(lpKeys)) {
			// "を取り除く
			std::wstring& element = patterns.emplace_back(quoted);
			std::erase(element, L'"');
		}
		return patterns;
	}

	/*!
		@brief ファイルパターンを、引用符を残したまま要素ごとに分離して返す

		空白・セミコロン・カンマで区切る。引用符の内側では区切らない。
		引用符はトークンのどの位置にあっても内外を切り替える。空の要素は返さない。

		@param[in]		keys					ファイルパターン
		@return 引用符を残した要素の配列。各要素は keys の部分文字列を指す
	*/
	static std::vector< std::wstring_view > SplitPatternKeepQuotes(std::wstring_view keys)
	{
		constexpr std::wstring_view WILDCARD_DELIMITER = L" ;,";	//リストの区切り

		std::vector< std::wstring_view > patterns;
		bool bInQuote = false;	//ダブルコーテーションの中か？
		size_t nBegin = 0;
		for (size_t i = 0; i < keys.size(); ++i) {
			if (keys[i] == L'"') {
				bInQuote = !bInQuote;
			}
			else if (!bInQuote && WILDCARD_DELIMITER.find(keys[i]) != std::wstring_view::npos) {
				if (nBegin < i) {
					patterns.push_back(keys.substr(nBegin, i - nBegin));
				}
				nBegin = i + 1;
			}
		}
		if (nBegin < keys.size()) {
			patterns.push_back(keys.substr(nBegin));
		}
		return patterns;
	}

	/*!
		@brief 引用符が閉じられていないか判定する
		@param[in]		lpKeys					ファイルパターン
		@retval true	引用符の数が奇数（閉じ忘れ）
		@retval false	引用符が閉じられている、または引用符がない
	*/
	static bool HasUnclosedQuote(LPCWSTR lpKeys)
	{
		const std::wstring_view keys(lpKeys);
		return std::ranges::count(keys, L'"') % 2 != 0;
	}

private:
	void ClearItems( void ){
		m_vecExceptFileKeys.clear();
		m_vecSearchFileKeys.clear();
		m_vecExceptFolderKeys.clear();
		m_vecSearchFolderKeys.clear();
		m_vecExceptAbsFileKeys.clear();
		m_vecExceptAbsFolderKeys.clear();
		return;
	}

	void push_back_unique( VGrepEnumKeys& keys, LPCWSTR addKey ){
		if( ! IsExist( keys, addKey) ){
			keys.emplace_back( addKey );
		}
	}

	BOOL IsExist( const VGrepEnumKeys& keys, LPCWSTR addKey ) const {
		return ( keys.cend() != std::find( keys.cbegin(), keys.cend(), addKey ) ) ? TRUE : FALSE;
	}

	/*
		@retval 0 正常終了
		@retval 1 *\file.exe などのフォルダー部分でのワイルドカードはエラー
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
		std::vector< std::wstring > patterns = SplitPattern(lpKeys);

		for (size_t i = 0; i < patterns.size(); i++) {
			const std::wstring& element = patterns[i];
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
#endif /* SAKURA_CGREPENUMKEYS_FCE5732F_FA0C_4CB2_90D9_D1D440841D5C_H_ */
