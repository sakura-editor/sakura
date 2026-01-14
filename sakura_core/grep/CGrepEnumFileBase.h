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

#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <Shlwapi.h>
#include "grep/CGrepEnumKeys.h"
#include "util/string_ex.h"

typedef std::pair< LPWSTR, DWORD > PairGrepEnumItem;
typedef std::vector< PairGrepEnumItem > VPGrepEnumItem;

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
	virtual ~CGrepEnumFileBase(){
		ClearItems();
	}

	void ClearItems( void ){
		for( int i = 0; i < GetCount(); i++ ){
			LPWSTR lp = m_vpItems[ i ].first;
			m_vpItems[ i ].first = nullptr;
			delete [] lp;
		}
		m_vpItems.clear();
		return;
	}

	BOOL IsExist( LPCWSTR lpFileName ){
		for( int i = 0; i < GetCount(); i++ ){
			if( wcscmp( m_vpItems[ i ].first, lpFileName ) == 0 ){
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

	int GetCount( void ){
		return (int)m_vpItems.size();
	}

	LPCWSTR GetFileName( int i ){
		if( i < 0 || i >= GetCount() ) return nullptr;
		return m_vpItems[ i ].first;
	}

	DWORD GetFileSizeLow( int i ){
		if( i < 0 || i >= GetCount() ) return 0;
		return m_vpItems[ i ].second;
	}

	int Enumerates( LPCWSTR lpBaseFolder, VGrepEnumKeys& vecKeys, CGrepEnumOptions& option, CGrepEnumFileBase* pExceptItems = nullptr ){
		int found = 0;

		const auto cchBaseFolder = lpBaseFolder ? wcsnlen_s(lpBaseFolder, 4096 - 1) : 0; // FIXME: パス長の上限は暫定値。
		for( int i = 0; i < (int)vecKeys.size(); i++ ){
			const auto baseLen = cchBaseFolder;
			LPWSTR lpPath = new WCHAR[ baseLen + wcslen( vecKeys[ i ] ) + 2 ];
			if( nullptr == lpPath ) break;
			wcscpy( lpPath, lpBaseFolder );
			wcscpy( lpPath + baseLen, L"\\" );
			wcscpy( lpPath + baseLen + 1, vecKeys[ i ] );
			// vecKeys[ i ] ==> "subdir\*.h" 等の場合に後で(ファイル|フォルダー)名に "subdir\" を連結する
			const WCHAR* keyDirYen = wcsrchr( vecKeys[ i ], L'\\' );
			const WCHAR* keyDirSlash = wcsrchr( vecKeys[ i ], L'/' );
			const WCHAR* keyDir;
			if( keyDirYen == nullptr ){
				keyDir = keyDirSlash;
			}else if( keyDirSlash == nullptr ){
				keyDir = keyDirYen;
			}else if( keyDirYen < keyDirSlash ){
				keyDir = keyDirSlash;
			}else{
				keyDir = keyDirYen;
			}
			const auto nKeyDirLen = keyDir ? keyDir - vecKeys[ i ] + 1 : 0;

			WIN32_FIND_DATA w32fd;
			HANDLE handle = ::FindFirstFile( lpPath, &w32fd );
			if( INVALID_HANDLE_VALUE != handle ){
				do{
					if( !::PathMatchSpec(w32fd.cFileName, vecKeys[ i ] + nKeyDirLen) ){
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
					const auto cchName = nKeyDirLen + wcslen(w32fd.cFileName) + 1;
					LPWSTR lpName = new WCHAR[cchName];
					::wcsncpy_s(lpName, cchName, vecKeys[ i ], _TRUNCATE);
					::wcsncat_s(lpName, cchName, w32fd.cFileName, _TRUNCATE);
					const auto cchFullPath = baseLen + wcslen(lpName) + 2;
					LPWSTR lpFullPath = new WCHAR[cchFullPath];
					::wcsncpy_s(lpFullPath, cchFullPath, lpBaseFolder, _TRUNCATE);
					::wcsncat_s(lpFullPath, cchFullPath, L"\\", _TRUNCATE);
					::wcsncat_s(lpFullPath, cchFullPath, lpName, _TRUNCATE);
					if( IsValid( w32fd, lpName ) ){
						if( pExceptItems && pExceptItems->IsExist( lpFullPath ) ){
						}else{
							m_vpItems.emplace_back( lpName, w32fd.nFileSizeLow );
							found++; // 2011.11.19
							if( pExceptItems && nKeyDirLen ){
								// フォルダーを含んだパスなら検索済みとして除外指定に追加する
								pExceptItems->m_vpItems.emplace_back( lpFullPath, w32fd.nFileSizeLow );
							}else{
								delete [] lpFullPath;
							}
							continue;
						}
					}
					delete [] lpName;
					delete [] lpFullPath;
				}while( ::FindNextFile( handle, &w32fd ) );
				::FindClose( handle );
			}
			delete [] lpPath;
		}
		return found;
	}
};

#endif /* SAKURA_CGREPENUMFILEBASE_6B85547E_13E4_4183_AE06_B4D6395ABC88_H_ */
