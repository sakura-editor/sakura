﻿/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake

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

// 要先行定義
// #include "DLLSHAREDATA.h"

#include "config/maxdata.h"
#include "util/design_template.h"

struct EditInfo;

//! iniフォルダ設定	// 2007.05.31 ryoji
struct IniFolder
{
	bool  m_bInit;						 // 初期化済フラグ
	bool  m_bReadPrivate;				 // マルチユーザ用iniからの読み出しフラグ
	bool  m_bWritePrivate;				 // マルチユーザ用iniへの書き込みフラグ
	WCHAR m_szIniFile[_MAX_PATH];		 // EXE基準のiniファイルパス
	WCHAR m_szPrivateIniFile[_MAX_PATH]; // マルチユーザ用のiniファイルパス
};										 /* iniフォルダ設定 */

//共有メモリ内構造体
struct SShare_FileNameManagement
{
	IniFolder m_IniFolder; /**** iniフォルダ設定 ****/
};

//!ファイル名管理
class CFileNameManager : public TSingleton<CFileNameManager>
{
	friend class TSingleton<CFileNameManager>;
	CFileNameManager()
	{
		m_pShareData			  = &GetDllShareData();
		m_nTransformFileNameCount = -1;
	}

public:
	//ファイル名関連
	LPWSTR		   GetTransformFileNameFast(LPCWSTR, LPWSTR, int nDestLen, HDC hDC, bool bFitMode = true,
											int cchMaxWidth = 0); // 2002.11.24 Moca Add
	int			   TransformFileName_MakeCache(void);
	static LPCWSTR GetFilePathFormat(LPCWSTR, LPWSTR, int, LPCWSTR, LPCWSTR);
	static bool	ExpandMetaToFolder(LPCWSTR, LPWSTR, int);

	//メニュー類のファイル名作成
	bool GetMenuFullLabel_WinList(WCHAR *pszOutput, int nBuffSize, const EditInfo *editInfo, int id, int index, HDC hDC)
	{
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, false, index, false, hDC);
	}
	bool GetMenuFullLabel_MRU(WCHAR *pszOutput, int nBuffSize, const EditInfo *editInfo, int id, bool bFavorite,
							  int index, HDC hDC)
	{
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, bFavorite, index, true, hDC);
	}
	bool GetMenuFullLabel_WinListNoEscape(WCHAR *pszOutput, int nBuffSize, const EditInfo *editInfo, int id, int index,
										  HDC hDC)
	{
		return GetMenuFullLabel(pszOutput, nBuffSize, false, editInfo, id, false, index, false, hDC);
	}
	bool GetMenuFullLabel_File(WCHAR *pszOutput, int nBuffSize, const WCHAR *pszFile, int id, HDC hDC,
							   bool bModified = false, ECodeType nCharCode = CODE_NONE)
	{
		return GetMenuFullLabel(pszOutput, nBuffSize, true, pszFile, id, false, nCharCode, false, -1, false, hDC);
	}
	bool GetMenuFullLabel_FileNoEscape(WCHAR *pszOutput, int nBuffSize, const WCHAR *pszFile, int id, HDC hDC,
									   bool bModified = false, ECodeType nCharCode = CODE_NONE)
	{
		return GetMenuFullLabel(pszOutput, nBuffSize, false, pszFile, id, false, nCharCode, false, -1, false, hDC);
	}

	bool GetMenuFullLabel(WCHAR *pszOutput, int nBuffSize, bool bEspaceAmp, const EditInfo *editInfo, int id,
						  bool bFavorite, int index, bool bAccKeyZeroOrigin, HDC hDC);
	bool GetMenuFullLabel(WCHAR *pszOutput, int nBuffSize, bool bEspaceAmp, const WCHAR *pszFile, int id,
						  bool bModified, ECodeType nCharCode, bool bFavorite, int index, bool bAccKeyZeroOrigin,
						  HDC hDC);

	static WCHAR GetAccessKeyByIndex(int index, bool bZeroOrigin);

	static void GetIniFileNameDirect(LPWSTR pszPrivateIniFile, LPWSTR pszIniFile, LPCWSTR pszProfName);
	/* 構成設定ファイルからiniファイル名を取得する */ // 2007.09.04 ryoji
	void GetIniFileName(LPWSTR pszIniFileName, LPCWSTR pszProfName, BOOL bRead = FALSE);
	/* iniファイル名の取得 */ // 2007.05.19 ryoji

private:
	DLLSHAREDATA *m_pShareData;

	// ファイル名簡易表示用キャッシュ
	int   m_nTransformFileNameCount; // 有効数
	WCHAR m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int   m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};
