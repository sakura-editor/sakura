/*!	@file
	
	@brief GREP support library
	
	@author wakura
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CGREPENUMFILTERFILES_48762BD9_D3E4_4D32_8C3A_502A7A6AE85A_H_
#define SAKURA_CGREPENUMFILTERFILES_48762BD9_D3E4_4D32_8C3A_502A7A6AE85A_H_
#pragma once

#include "grep/CGrepEnumFiles.h"

#include <string>
#include <string_view>

class CGrepEnumFilterFiles final : public CGrepEnumFiles {
private:
	CGrepEnumFiles m_cGrepEnumExceptFiles;

	//! 除外ファイル(正規表現)の照合に使う。Enumerates() で設定する
	const CGrepEnumKeys* m_pGrepEnumKeys = nullptr;

	std::wstring m_strBaseFolder;	//!< Enumerates() の基準フォルダー。除外ファイル(正規表現)のフルパスを作るのに使う
	std::wstring m_strFullPath;		//!< フルパスの作業用バッファ(ファイルごとに確保しないため)

	/*!
		@brief 基準フォルダーとファイル名からフルパスを作る
		@param[in]	name	キーのフォルダー部分を含むファイル名(Enumerates() の strName)
		@note CGrepEnumFileBase::Enumerates() の strFullPath と同じ組み立て方にする
	*/
	std::wstring_view MakeFullPath( std::wstring_view name ){
		m_strFullPath.assign( m_strBaseFolder );
		if( !m_strBaseFolder.empty() ){
			m_strFullPath.append( L"\\" );
		}
		m_strFullPath.append( name );
		return m_strFullPath;
	}

public:
	CGrepEnumFilterFiles(){
	}

	virtual ~CGrepEnumFilterFiles(){
	}

	BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCWSTR pFile = nullptr  ) override {
		if( CGrepEnumFiles::IsValid( w32fd, pFile ) ){
			if( m_cGrepEnumExceptFiles.IsValid( w32fd, pFile ) ){
				// 除外ファイル(正規表現)はフルパスで照合する
				if( m_pGrepEnumKeys && m_pGrepEnumKeys->IsExceptFilePath( MakeFullPath( pFile ? pFile : w32fd.cFileName ) ) ){
					return FALSE;
				}
				return TRUE;
			}
		}
		return FALSE;
	}

	int Enumerates( LPCWSTR lpBaseFolder, CGrepEnumKeys& cGrepEnumKeys, CGrepEnumOptions option, CGrepEnumFiles& pExcept ){
		m_pGrepEnumKeys = &cGrepEnumKeys;
		m_strBaseFolder = lpBaseFolder ? lpBaseFolder : L"";
		m_cGrepEnumExceptFiles.Enumerates( lpBaseFolder, cGrepEnumKeys.m_vecExceptFileKeys, option, nullptr );
		return CGrepEnumFiles::Enumerates( lpBaseFolder, cGrepEnumKeys.m_vecSearchFileKeys, option, &pExcept );
	}
};
#endif /* SAKURA_CGREPENUMFILTERFILES_48762BD9_D3E4_4D32_8C3A_502A7A6AE85A_H_ */
