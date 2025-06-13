/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFILENAMEMANAGER_2B89B426_470E_40D6_B62E_5321E383ECD6_H_
#define SAKURA_CFILENAMEMANAGER_2B89B426_470E_40D6_B62E_5321E383ECD6_H_
#pragma once

#include <string_view>

#include "util/design_template.h"
#include "config/maxdata.h"

struct DLLSHAREDATA;
struct EditInfo;
DLLSHAREDATA& GetDllShareData();

//!ファイル名管理
class CFileNameManager : public TSingleton<CFileNameManager>{
	friend class TSingleton<CFileNameManager>;
	CFileNameManager()
	{
		m_pShareData = &GetDllShareData();
		m_nTransformFileNameCount = -1;
	}

public:
	//ファイル名関連
	LPWSTR GetTransformFileNameFast( LPCWSTR, LPWSTR, int nDestLen, HDC hDC, bool bFitMode = true, int cchMaxWidth = 0 );	// 2002.11.24 Moca Add
	int TransformFileName_MakeCache( void );
	static LPCWSTR GetFilePathFormat( std::wstring_view strSrc, LPWSTR pszDest, size_t nDestLen, std::wstring_view strFrom, std::wstring_view strTo );
	static bool ExpandMetaToFolder( LPCWSTR, LPWSTR, int );

	//メニュー類のファイル名作成
	bool GetMenuFullLabel_WinList(WCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index, HDC hDC){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, false, index, false, hDC);
	}
	bool GetMenuFullLabel_MRU(WCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, bool bFavorite, int index, HDC hDC){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, bFavorite, index, true, hDC);
	}
	bool GetMenuFullLabel_WinListNoEscape(WCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index, HDC hDC){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, editInfo, id, false, index, false, hDC);
	}
	bool GetMenuFullLabel_File(WCHAR* pszOutput, int nBuffSize, const WCHAR* pszFile, int id, HDC hDC, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, pszFile, id, false, nCharCode, false, -1, false, hDC);
	}
	bool GetMenuFullLabel_FileNoEscape(WCHAR* pszOutput, int nBuffSize, const WCHAR* pszFile, int id, HDC hDC, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, pszFile, id, false, nCharCode, false, -1, false, hDC);
	}

	bool GetMenuFullLabel(WCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const EditInfo* editInfo, int id, bool bFavorite, int index, bool bAccKeyZeroOrigin, HDC hDC);
	bool GetMenuFullLabel(WCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const WCHAR* pszFile, int id, bool bModified, ECodeType nCharCode, bool bFavorite, int index, bool bAccKeyZeroOrigin, HDC hDC);
	
	static WCHAR GetAccessKeyByIndex(int index, bool bZeroOrigin);

private:
	DLLSHAREDATA* m_pShareData;

	// ファイル名簡易表示用キャッシュ
	int		m_nTransformFileNameCount; // 有効数
	WCHAR	m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int		m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};
#endif /* SAKURA_CFILENAMEMANAGER_2B89B426_470E_40D6_B62E_5321E383ECD6_H_ */
