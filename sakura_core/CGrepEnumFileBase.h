/*!	@file
	
	@brief GREP support library
	
	@author wakura
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura

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
#include <Shlwapi.h>
#include "CGrepEnumKeys.h"
#include "util/string_ex.h"

typedef std::pair< LPTSTR, DWORD > PairGrepEnumItem;
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

public:
	CGrepEnumFileBase(){
	}

	virtual ~CGrepEnumFileBase(){
		ClearItems();
	}

	void ClearItems( void ){
		for( int i = 0; i < GetCount(); i++ ){
			LPTSTR lp = m_vpItems[ i ].first;
			m_vpItems[ i ].first = NULL;
			delete [] lp;
		}
		m_vpItems.clear();
		return;
	}

	BOOL IsExist( LPCTSTR lpFileName ){
		for( int i = 0; i < GetCount(); i++ ){
			if( _tcscmp( m_vpItems[ i ].first, lpFileName ) == 0 ){
				return TRUE;
			}
		}
		return FALSE;
	}

	virtual BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL ){
		if( ! IsExist( pFile ? pFile : w32fd.cFileName ) ){
			return TRUE;
		}
		return FALSE;
	}

	int GetCount( void ){
		return (int)m_vpItems.size();
	}

	LPCTSTR GetFileName( int i ){
		if( i < 0 || i >= GetCount() ) return NULL;
		return m_vpItems[ i ].first;
	}

	DWORD GetFileSizeLow( int i ){
		if( i < 0 || i >= GetCount() ) return 0;
		return m_vpItems[ i ].second;
	}

	int Enumerates( LPCTSTR lpBaseFolder, VGrepEnumKeys& vecKeys, CGrepEnumOptions& option, CGrepEnumFileBase* pExceptItems = NULL ){
		int found = 0;

		for( int i = 0; i < (int)vecKeys.size(); i++ ){
			int baseLen = _tcslen( lpBaseFolder );
			LPTSTR lpPath = new TCHAR[ baseLen + _tcslen( vecKeys[ i ] ) + 2 ];
			if( NULL == lpPath ) break;
			auto_strcpy( lpPath, lpBaseFolder );
			auto_strcpy( lpPath + baseLen, _T("\\") );
			auto_strcpy( lpPath + baseLen + 1, vecKeys[ i ] );
			// vecKeys[ i ] ==> "subdir\*.h" 等の場合に後で(ファイル|フォルダ)名に "subdir\" を連結する
			const TCHAR* keyDirYen = _tcsrchr( vecKeys[ i ], _T('\\') );
			const TCHAR* keyDirSlash = _tcsrchr( vecKeys[ i ], _T('/') );
			const TCHAR* keyDir;
			if( keyDirYen == NULL ){
				keyDir = keyDirSlash;
			}else if( keyDirSlash == NULL ){
				keyDir = keyDirYen;
			}else if( keyDirYen < keyDirSlash ){
				keyDir = keyDirSlash;
			}else{
				keyDir = keyDirYen;
			}
			int nKeyDirLen = keyDir ? keyDir - vecKeys[ i ] + 1 : 0;

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
					LPTSTR lpName = new TCHAR[ nKeyDirLen + _tcslen( w32fd.cFileName ) + 1 ];
					_tcsncpy( lpName, vecKeys[ i ], nKeyDirLen );
					_tcscpy( lpName + nKeyDirLen, w32fd.cFileName );
					LPTSTR lpFullPath = new TCHAR[ baseLen + _tcslen(lpName) + 2 ];
					auto_strcpy( lpFullPath, lpBaseFolder );
					auto_strcpy( lpFullPath + baseLen, _T("\\") );
					auto_strcpy( lpFullPath + baseLen + 1, lpName );
					if( IsValid( w32fd, lpName ) ){
						if( pExceptItems && pExceptItems->IsExist( lpFullPath ) ){
						}else{
							m_vpItems.push_back( PairGrepEnumItem( lpName, w32fd.nFileSizeLow ) );
							found++; // 2011.11.19
							if( pExceptItems && nKeyDirLen ){
								// フォルダを含んだパスなら検索済みとして除外指定に追加する
								pExceptItems->m_vpItems.push_back( PairGrepEnumItem( lpFullPath, w32fd.nFileSizeLow ) );
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
