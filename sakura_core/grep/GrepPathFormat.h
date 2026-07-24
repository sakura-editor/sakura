/*!	@file
	@brief Grep パスリスト整形ヘルパ

	';' を含むパスの引用符付けと、パスリストの ';' 区切り連結を共通化する。
	（旧: CGrepAgent.cpp の FormatPathList / CDlgGrep.cpp 内の個別実装）
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_GREPPATHFORMAT_3C2B1A9E_5D47_4F06_9A31_7B8E2D4C6F10_H_
#define SAKURA_GREPPATHFORMAT_3C2B1A9E_5D47_4F06_9A31_7B8E2D4C6F10_H_
#pragma once

#include <string>
#include <string_view>

//! ';' を含むパスを引用符 '"' で囲んで返す（含まなければそのまま返す）
inline std::wstring QuotePathIfNeeded( std::wstring_view path )
{
	if( std::wstring_view::npos != path.find( L';' ) ){
		std::wstring quoted;
		quoted.reserve( path.size() + 2 );
		quoted += L'"';
		quoted += path;
		quoted += L'"';
		return quoted;
	}
	return std::wstring( path );
}

//! パスリストを ';' 区切りで連結して返す（';' を含む要素は引用符で囲む）
template<class ContainerType>
std::wstring FormatPathList( const ContainerType& container )
{
	std::wstring strPatterns;
	bool firstItem = true;
	for( const auto& pattern : container ){
		// パスリストは ';' で区切る(2つ目以降の前に付加する)
		if( firstItem ){
			firstItem = false;
		}else{
			strPatterns += L';';
		}
		strPatterns += QuotePathIfNeeded( std::wstring_view( pattern ) );
	}
	return strPatterns;
}

#endif /* SAKURA_GREPPATHFORMAT_3C2B1A9E_5D47_4F06_9A31_7B8E2D4C6F10_H_ */
