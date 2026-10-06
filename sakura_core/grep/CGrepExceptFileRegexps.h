/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPEXCEPTFILEREGEXPS_H_
#define SAKURA_CGREPEXCEPTFILEREGEXPS_H_
#pragma once

#include "grep/CGrepEnumKeys.h"
#include "extmodule/CBregexp.h"
#include "CSelectLang.h"
#include "sakura_rc.h"

#include <algorithm>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/*!
	@brief 除外ファイル(正規表現)の照合器

	CGrepEnumKeys::m_vecExceptFileRegexKeys をコンパイルし、
	CGrepEnumKeys::m_fnIsExceptFilePath に照合関数を設定する。

	@note 照合関数はこのオブジェクトを参照するので、CGrepEnumKeys より先に破棄しないこと。
	@note CBregexp::Match() は照合結果をインスタンスに持つので、複数のスレッドから同時に照合しないこと。
*/
class CGrepExceptFileRegexps {
	std::vector<std::unique_ptr<CBregexp>> m_regexps;	//!< パターンごとのコンパイル済み正規表現
	std::wstring m_strErrorMessage;						//!< 失敗したときのメッセージ

public:
	/*!
		@brief 除外ファイル(正規表現)をコンパイルし、照合関数を設定する
		@param[in,out]	cGrepEnumKeys	SetFileKeys() で解析済みのキー。照合関数を設定する
		@param[in]		pszDllName		正規表現ライブラリの DLL 名(nullptr なら既定)
		@retval true	成功(正規表現が無いときも true。照合関数は設定しない)
		@retval false	失敗。GetErrorMessage() で理由を取得できる
	*/
	bool Attach( CGrepEnumKeys& cGrepEnumKeys, LPCWSTR pszDllName )
	{
		m_regexps.clear();
		m_strErrorMessage.clear();
		for( const auto& strPattern : cGrepEnumKeys.m_vecExceptFileRegexKeys ){
			auto pRegexp = std::make_unique<CBregexp>();
			if( DLL_SUCCESS != pRegexp->InitDll( pszDllName ) ){
				m_strErrorMessage = LS( STR_BREGONIG_LOAD );
				return false;
			}
			// Windows のパスなので、既存の除外ワイルドカードと同じく大文字小文字を区別しない
			if( !pRegexp->Compile( strPattern.c_str(), CBregexp::optNothing ) ){
				m_strErrorMessage = std::format( L"{}\n{}\n{}", LS( STR_GREP_ERR_EXCLUDE_REGEXP ), strPattern, pRegexp->GetLastMessage() );
				return false;
			}
			m_regexps.push_back( std::move( pRegexp ) );
		}
		if( !m_regexps.empty() ){
			cGrepEnumKeys.m_fnIsExceptFilePath = [this]( std::wstring_view filePath ){ return IsMatch( filePath ); };
		}
		return true;
	}

	/*!
		@brief ファイルのフルパスがいずれかの正規表現に一致するか調べる
		@param[in]	filePath	ファイルのフルパス
	*/
	bool IsMatch( std::wstring_view filePath )
	{
		return std::ranges::any_of( m_regexps, [filePath]( const auto& pRegexp ){
			return pRegexp->Match( filePath.data(), int( filePath.length() ) );
		} );
	}

	//! 失敗したときのメッセージ
	const std::wstring& GetErrorMessage() const noexcept { return m_strErrorMessage; }
};

#endif /* SAKURA_CGREPEXCEPTFILEREGEXPS_H_ */
