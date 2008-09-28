/*
	2008.05.18 kobake CShareData から分離
*/

#pragma once

#include "util/design_template.h"

//! iniフォルダ設定	// 2007.05.31 ryoji
struct IniFolder {
	bool m_bInit;							// 初期化済フラグ
	bool m_bReadPrivate;					// マルチユーザ用iniからの読み出しフラグ
	bool m_bWritePrivate;					// マルチユーザ用iniへの書き込みフラグ
	TCHAR m_szIniFile[_MAX_PATH];			// EXE基準のiniファイルパス
	TCHAR m_szPrivateIniFile[_MAX_PATH];	// マルチユーザ用のiniファイルパス
};	/* iniフォルダ設定 */


//共有メモリ内構造体
struct SShare_FileNameManagement{
	IniFolder			m_IniFolder;	/**** iniフォルダ設定 ****/
};


//!ファイル名管理
class CFileNameManager : public TSingleton<CFileNameManager>{
public:
	friend class TSingleton<CFileNameManager>;
protected:
	CFileNameManager()
	{
		m_pShareData = &GetDllShareData();
		m_nTransformFileNameCount = -1;
	}
public:
	//ファイル名関連
	LPTSTR GetTransformFileNameFast( LPCTSTR, LPTSTR, int );	// 2002.11.24 Moca Add
	int TransformFileName_MakeCache( void );
	static LPCTSTR GetFilePathFormat( LPCTSTR, LPTSTR, int, LPCTSTR, LPCTSTR );
	static bool ExpandMetaToFolder( LPCTSTR, LPTSTR, int );
	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile );	/* 構成設定ファイルからiniファイル名を取得する */	// 2007.09.04 ryoji
	void GetIniFileName( LPTSTR pszIniFileName, BOOL bRead = FALSE );	/* iniファイル名の取得 */	// 2007.05.19 ryoji

private:
	DLLSHAREDATA* m_pShareData;

	// ファイル名簡易表示用キャッシュ
	int		m_nTransformFileNameCount; // 有効数
	TCHAR	m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int		m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};
