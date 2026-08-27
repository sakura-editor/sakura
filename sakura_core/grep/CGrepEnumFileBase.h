/*!	@file
	
	@brief GREP support library
	
	@author wakura
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPENUMFILEBASE_6B85547E_13E4_4183_AE06_B4D6395ABC88_H_
#define SAKURA_CGREPENUMFILEBASE_6B85547E_13E4_4183_AE06_B4D6395ABC88_H_
#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <Shlwapi.h>
#include "cxx/ResourceHolder.hpp"
#include "grep/CGrepEnumKeys.h"
#include "util/string_ex.h"

using PairGrepEnumItem = std::pair< std::wstring, DWORD >;
using VPGrepEnumItem = std::vector< PairGrepEnumItem >;

class CGrepEnumOptions {
public:
	CGrepEnumOptions()
		:m_bIgnoreHidden(false)
		,m_bIgnoreReadOnly(false)
		,m_bIgnoreSystem(false)
	{}
	bool	m_bIgnoreHidden;
	bool	m_bIgnoreReadOnly;
	bool	m_bIgnoreSystem;
};

class CGrepEnumFileBase {
private:
	VPGrepEnumItem m_vpItems;

	using Me = CGrepEnumFileBase;

public:
	CGrepEnumFileBase() noexcept = default;
	CGrepEnumFileBase(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CGrepEnumFileBase(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CGrepEnumFileBase() = default;

	void ClearItems( void ){
		m_vpItems.clear();
		return;
	}

	BOOL IsExist( std::wstring_view svFileName ) const {
		for( const auto& item : m_vpItems ){
			if( std::wstring_view( item.first ) == svFileName ){
				return TRUE;
			}
		}
		return FALSE;
	}

	virtual BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCWSTR pFile = nullptr ){
		if( ! IsExist( pFile ? pFile : w32fd.cFileName ) ){
			return TRUE;
		}
		return FALSE;
	}

	int GetCount( void ) const {
		return (int)m_vpItems.size();
	}

	LPCWSTR GetFileName( int i ) const {
		if( i < 0 || i >= GetCount() ) return nullptr;
		return m_vpItems[ i ].first.c_str();
	}

	DWORD GetFileSizeLow( int i ) const {
		if( i < 0 || i >= GetCount() ) return 0;
		return m_vpItems[ i ].second;
	}

	int Enumerates( LPCWSTR lpBaseFolder, VGrepEnumKeys& vecKeys, CGrepEnumOptions& option, CGrepEnumFileBase* pExceptItems = nullptr ){
		int found = 0;

		// 基底フォルダーは長さの上限を設けずに扱う(固定長バッファを使わなくなったため上限が不要)
		const std::wstring_view svBaseFolder = lpBaseFolder ? std::wstring_view( lpBaseFolder ) : std::wstring_view();

		// 作業用バッファはループの外に置き、確保済みの領域を使い回す
		std::wstring strPath;
		std::wstring strName;
		std::wstring strFullPath;

		for( const auto& strKey : vecKeys ){
			strPath.assign( svBaseFolder );
			strPath.append( L"\\" );
			strPath.append( strKey );

			// strKey ==> "subdir\*.h" 等の場合に後で(ファイル|フォルダー)名に "subdir\" を連結する
			const auto keyDirPos = strKey.find_last_of( L"\\/" );
			const auto nKeyDirLen = ( std::wstring::npos != keyDirPos ) ? keyDirPos + 1 : 0;

			WIN32_FIND_DATA w32fd;
			HANDLE handle = ::FindFirstFile( strPath.c_str(), &w32fd );
			if( INVALID_HANDLE_VALUE == handle ){
				continue;
			}
			// 検索ハンドルをスマートポインタに入れる(途中で抜けても確実に閉じる)
			using FindFileHolder = cxx::ResourceHolder<&::FindClose>;
			FindFileHolder handleHolder{ handle };
			do{
				if( !::PathMatchSpec( w32fd.cFileName, strKey.c_str() + nKeyDirLen ) ){
					continue;
				}
				if( option.m_bIgnoreHidden && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ){
					continue;
				}
				if( option.m_bIgnoreReadOnly && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ){
					continue;
				}
				if( option.m_bIgnoreSystem && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ){
					continue;
				}
				strName.assign( strKey, 0, nKeyDirLen );
				strName.append( w32fd.cFileName );
				strFullPath.assign( svBaseFolder );
				strFullPath.append( L"\\" );
				strFullPath.append( strName );
				if( !IsValid( w32fd, strName.c_str() ) ){
					continue;
				}
				if( pExceptItems && pExceptItems->IsExist( strFullPath ) ){
					continue;
				}
				m_vpItems.emplace_back( strName, w32fd.nFileSizeLow );
				found++; // 2011.11.19
				if( pExceptItems && nKeyDirLen ){
					// フォルダーを含んだパスなら検索済みとして除外指定に追加する
					pExceptItems->m_vpItems.emplace_back( strFullPath, w32fd.nFileSizeLow );
				}
			}while( ::FindNextFile( handle, &w32fd ) );
		}
		return found;
	}
};

#endif /* SAKURA_CGREPENUMFILEBASE_6B85547E_13E4_4183_AE06_B4D6395ABC88_H_ */
