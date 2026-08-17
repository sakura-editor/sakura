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

#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <Shlwapi.h>
#include "grep/CGrepEnumKeys.h"
#include "util/string_ex.h"

using PairGrepEnumItem = std::pair< std::wstring, DWORD >;
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
	//! 透過的ハッシュ（LPCWSTR/wstring_view から一時 std::wstring を生成せずに検索するため）
	struct SWStringHash {
		using is_transparent = void;
		[[nodiscard]] size_t operator()( std::wstring_view sv ) const noexcept {
			return std::hash<std::wstring_view>{}( sv );
		}
	};
	std::unordered_set< std::wstring, SWStringHash, std::equal_to<> > m_setItemNames;	//!< IsExist 用の名前索引（m_vpItems と同期して維持する）

	using Me = CGrepEnumFileBase;

	//! FindFirstFile ハンドルの RAII クローザ
	struct FindHandleCloser {
		void operator()( HANDLE h ) const {
			// FindFirstFile 失敗時（INVALID_HANDLE_VALUE）はクローズ対象外
			if( h && h != INVALID_HANDLE_VALUE ) ::FindClose( h );
		}
	};

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
		m_vpItems.clear();
		m_setItemNames.clear();
		return;
	}

	BOOL IsExist( LPCWSTR lpFileName ) const {
		return m_setItemNames.contains( std::wstring_view( lpFileName ) ) ? TRUE : FALSE;
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

	LPCWSTR GetFileName( int i ){
		if( i < 0 || i >= GetCount() ) return nullptr;
		return m_vpItems[ i ].first.c_str();
	}

	DWORD GetFileSizeLow( int i ){
		if( i < 0 || i >= GetCount() ) return 0;
		return m_vpItems[ i ].second;
	}

	//! 属性フラグによる列挙除外判定（Enumerates の CC 削減用）
	static bool IsIgnoredByAttribute( const WIN32_FIND_DATA& w32fd, const CGrepEnumOptions& option ){
		if( option.m_bIgnoreHidden && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ){
			return true;
		}
		if( option.m_bIgnoreReadOnly && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ){
			return true;
		}
		if( option.m_bIgnoreSystem && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ){
			return true;
		}
		return false;
	}

	int Enumerates( LPCWSTR lpBaseFolder, const VGrepEnumKeys& vecKeys, const CGrepEnumOptions& option, CGrepEnumFileBase* pExceptItems = nullptr ){
		int found = 0;

		const std::wstring baseFolder = lpBaseFolder ? lpBaseFolder : L"";
		for( const auto& key : vecKeys ){
			const std::wstring path = baseFolder + L"\\" + key;
			// key ==> "subdir\*.h" 等の場合に後で(ファイル|フォルダー)名に "subdir\" を連結する
			const size_t keyDirPos = key.find_last_of( L"\\/" );
			const size_t nKeyDirLen = ( keyDirPos != std::wstring::npos ) ? keyDirPos + 1 : 0;

			WIN32_FIND_DATA w32fd;
			// ハンドルは RAII で管理し、例外発生時も FindClose を保証する
			std::unique_ptr< void, FindHandleCloser > handle( ::FindFirstFile( path.c_str(), &w32fd ) );
			if( INVALID_HANDLE_VALUE == handle.get() ){
				continue;
			}
			do{
				// 検索キーのワイルドカードにマッチしない名前はスキップ
				if( !::PathMatchSpec(w32fd.cFileName, key.c_str() + nKeyDirLen) ){
					continue;
				}
				if( IsIgnoredByAttribute( w32fd, option ) ){
					continue;
				}
				std::wstring name = key.substr( 0, nKeyDirLen ) + w32fd.cFileName;
				// 派生クラスの判定（ファイル列挙/フォルダー列挙の種別・"." ".." 除外等）で対象外ならスキップ
				if( !IsValid( w32fd, name.c_str() ) ){
					continue;
				}
				const std::wstring fullPath = baseFolder + L"\\" + name;
				if( pExceptItems && pExceptItems->IsExist( fullPath.c_str() ) ){
					continue;	// 除外指定に一致（従来の空ブロック if/else 構造を反転）
				}
				if( pExceptItems && nKeyDirLen ){
					// フォルダーを含んだパスなら検索済みとして除外指定に追加する
					pExceptItems->AddItem( fullPath, w32fd.nFileSizeLow );
				}
				AddItem( std::move( name ), w32fd.nFileSizeLow );
				found++; // 2011.11.19
			}while( ::FindNextFile( handle.get(), &w32fd ) );
		}
		return found;
	}

private:
	//! 列挙結果へ 1 件追加する（IsExist 用の名前索引も同期して更新する）
	void AddItem( std::wstring name, DWORD dwFileSizeLow ){
		m_setItemNames.insert( name );
		m_vpItems.emplace_back( std::move( name ), dwFileSizeLow );
	}
};

#endif /* SAKURA_CGREPENUMFILEBASE_6B85547E_13E4_4183_AE06_B4D6395ABC88_H_ */
