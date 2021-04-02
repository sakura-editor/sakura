﻿/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CDOCTYPEMANAGER_ACE5AE64_5C6A_4A70_BACF_99F9A448360D_H_
#define SAKURA_CDOCTYPEMANAGER_ACE5AE64_5C6A_4A70_BACF_99F9A448360D_H_
#pragma once

#include "types/CType.h"

struct DLLSHAREDATA;
DLLSHAREDATA& GetDllShareData();

//! ドキュメントタイプ管理
class CDocTypeManager{
public:
	CDocTypeManager()
	{
		m_pShareData = &GetDllShareData();
	}
	CTypeConfig GetDocumentTypeOfPath( const WCHAR* pszFilePath );	/* ファイルパスを渡して、ドキュメントタイプ（数値）を取得する */
	CTypeConfig GetDocumentTypeOfExt( const WCHAR* pszExt );		/* 拡張子を渡して、ドキュメントタイプ（数値）を取得する */
	CTypeConfig GetDocumentTypeOfId( int id );

	bool GetTypeConfig(CTypeConfig cDocumentType, STypeConfig& type);
	bool SetTypeConfig(CTypeConfig cDocumentType, const STypeConfig& type);
	[[nodiscard]] bool GetTypeConfigMini(CTypeConfig cDocumentType, const STypeConfigMini** type);
	bool AddTypeConfig(CTypeConfig cDocumentType);
	bool DelTypeConfig(CTypeConfig cDocumentType);

	static bool IsFileNameMatch(const WCHAR* pszTypeExts, const WCHAR* pszFileName);	// タイプ別拡張子にファイル名がマッチするか
	static void GetFirstExt(const WCHAR* pszTypeExts, WCHAR szFirstExt[], int nBuffSize);	// タイプ別拡張子の先頭拡張子を取得する
	static std::wstring ConvertTypesExtToDlgExt(const WCHAR *pszSrcExt, const WCHAR* szExt);	// タイプ別設定の拡張子リストをダイアログ用リストに変換する

	static const WCHAR* m_typeExtSeps;			// タイプ別拡張子の区切り文字
	static const WCHAR* m_typeExtWildcards;		// タイプ別拡張子のワイルドカード

private:
	DLLSHAREDATA* m_pShareData;
};
#endif /* SAKURA_CDOCTYPEMANAGER_ACE5AE64_5C6A_4A70_BACF_99F9A448360D_H_ */
