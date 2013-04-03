/*!	@file
	@brief プロセス間共有データのファイルへ／からの入出力

	@author Norio Nakatani

	@date 2005.04.07 D.S.Koba 各セクションの入出力を関数として分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, Stonee, jepro, MIK, asa-o, YAZAKI, hor
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, genta, Moca
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, MIK, Moca, D.S.Koba, genta
	Copyright (C) 2005, MIK, genta, D.S.Koba, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon, maru
	Copyright (C) 2007, ryoji, genta, maru, Moca, nasukoji
	Copyright (C) 2008, nasukoji, novice
	Copyright (C) 2009, nasukoji, ryoji, salarm

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <imagehlp.h>
#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "mymessage.h"
#include "Debug.h"
#include "global.h"
#include "CRunningTimer.h"
#include "CProfile.h"
#include "etc_uty.h"
#include "COsVersionInfo.h"
#include "CCommandLine.h"

#ifndef CSIDL_PROFILE
#define CSIDL_PROFILE                   0x0028        // USERPROFILE
#endif

#define STR_COLORDATA_HEAD3		" テキストエディタ色設定 Ver3"	//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした

void ShareData_IO_Sub_LogFont( CProfile& cProfile, const char* pszSecName,
	const char* pszKeyLf, const char* pszKeyPointSize, const char* pszKeyFaceName, LOGFONT& lf, int& pointSize );

template <typename T>
void SetValueLimit(T& target, int minval, int maxval)
{
	target = t_max<T>(minval, t_min<T>(maxval, target));
}

template <typename T>
void SetValueLimit(T& target, int maxval)
{
	SetValueLimit( target, 0, maxval );
}

/* static */ TCHAR CShareData::GetAccessKeyByIndex(int index, bool bZeroOrigin)
{
	if( index < 0 ) return 0;
	int accKeyIndex = ((bZeroOrigin? index: index+1) % 36);
	TCHAR c = (TCHAR)((accKeyIndex < 10) ? (_T('0') + accKeyIndex) : (_T('A') + accKeyIndex - 10));
	return c;
}

static void GetAccessKeyLabelByIndex(TCHAR* pszLabel, bool bEspaceAmp, int index, bool bZeroOrigin)
{
	TCHAR c = CShareData::GetAccessKeyByIndex(index, bZeroOrigin);
	if( c ){
		if( bEspaceAmp ){
			pszLabel[0] = _T('&');
			pszLabel[1] = c;
			pszLabel[2] = _T(' ');
			pszLabel[3] = _T('\0');
		}else{
			pszLabel[0] = c;
			pszLabel[1] = _T(' ');
			pszLabel[2] = _T('\0');
		}
	}else{
		pszLabel[0] = _T('\0');
	}
}

/*
	@param editInfo      ウィンドウ情報。NULで不明扱い
	@param index         いつも0originで指定。 -1で非表示
	@param bZeroOrigin   アクセスキーを0から振る
*/
bool CShareData::GetMenuFullLabel(
	TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const EditInfo* editInfo, int nId, bool bFavorite,
	int index, bool bAccKeyZeroOrigin
){
	const EditInfo* pfi = editInfo;
	TCHAR szAccKey[4];
	int ret = 0;
	if( NULL == pfi ){
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		ret = _stprintf( pszOutput, _T("%s 不明(応答なし)"), szAccKey );
		return true; // trueにしておく
	}else if( pfi->m_bIsGrep ){
		
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		//pfi->m_szGrepKeyShort → cmemDes
		CMemory	cmemDes;
		int nGrepKeyLen = _tcslen(pfi->m_szGrepKey);
		const int GREPKEY_LIMIT_LEN = 64;
		// CEditDoc::ExpandParameter では 32文字制限
		// メニューは 64文字制限
		LimitStringLengthB( pfi->m_szGrepKey, nGrepKeyLen, GREPKEY_LIMIT_LEN, cmemDes );
		
		const TCHAR* pszKey;
		TCHAR szMenu2[GREPKEY_LIMIT_LEN*2*2+1]; // WCHAR=>ACHARで2倍、&で2倍
		if( bEspaceAmp ){
			dupamp( cmemDes.GetStringPtr(), szMenu2 );
			pszKey = szMenu2;
		}else{
			pszKey = cmemDes.GetStringPtr();
		}

		//szMenuを作る
		//	Jan. 19, 2002 genta
		//	&の重複処理を追加したため継続判定を若干変更
		//	20100729 ExpandParameterにあわせて、・・・を...に変更
		ret = _stprintf( pszOutput, _T("%s 【Grep】\"%s%s\""),
			szAccKey, pszKey,
			( nGrepKeyLen > cmemDes.GetStringLength() ) ? _T("..."):_T("")
		);
	}else if( pfi->m_bIsDebug ){
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		ret = _stprintf( pszOutput, _T("%s アウトプット"), szAccKey );
	}else{
		return GetMenuFullLabel(pszOutput, nBuffSize, bEspaceAmp, pfi->m_szPath, nId, pfi->m_bIsModified, pfi->m_nCharCode, bFavorite,
			 index, bAccKeyZeroOrigin);
	}
	return 0 < ret;
}

bool CShareData::GetMenuFullLabel(
	TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const TCHAR* pszFile, int nId, bool bModified, ECodeType nCharCode, bool bFavorite,
	int index, bool bAccKeyZeroOrigin
){
	TCHAR szAccKey[4];
	TCHAR szFileName[_MAX_PATH];
	TCHAR szMenu2[_MAX_PATH * 2];
	const TCHAR* pszName;

	GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
	if( pszFile[0] ){
		this->GetTransformFileNameFast( pszFile, szFileName, _MAX_PATH );

		// szFileName → szMenu2
		//	Jan. 19, 2002 genta
		//	メニュー文字列の&を考慮
		if( bEspaceAmp ){
			dupamp( szFileName, szMenu2 );
			pszName = szMenu2;
		}else{
			pszName = szFileName;
		}
	}else{
		wsprintf( szFileName, _T("(無題)%d"), nId);
		pszName = szFileName;
	}
	const TCHAR* pszCharset = _T("");
	if( IsValidCodeTypeExceptSJIS(nCharCode)){
		pszCharset = gm_pszCodeNameArr_3[nCharCode];
	}
	
	int ret = _stprintf( pszOutput, _T("%s%s%s %s%s"),
		szAccKey, (bFavorite ? _T("★ ") : _T("")), pszName,
		(bModified ? _T("*"):_T(" ")), pszCharset
	);
	return 0 < ret;
}


/**
	構成設定ファイルからiniファイル名を取得する

	sakura.exe.iniからsakura.iniの格納フォルダを取得し、フルパス名を返す

	@param[out] pszPrivateIniFile マルチユーザ用のiniファイルパス
	@param[out] pszIniFile EXE基準のiniファイルパス

	@author ryoji
	@date 2007.09.04 ryoji 新規作成
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
void CShareData::GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile )
{
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(
		NULL,
		szPath, _countof(szPath)
	);
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );

	_snprintf( pszIniFile, _MAX_PATH - 1, _T("%s%s%s%s"), szDrive, szDir, szFname, _T(".ini") );
	pszIniFile[_MAX_PATH - 1] = _T('\0');

	// マルチユーザ用のiniファイルパス
	//		exeと同じフォルダに置かれたマルチユーザ構成設定ファイル（sakura.exe.ini）の内容
	//		に従ってマルチユーザ用のiniファイルパスを決める
	pszPrivateIniFile[0] = _T('\0');
	if( IsWin2000_or_later() ){
		_snprintf( szPath, _MAX_PATH - 1, _T("%s%s%s%s"), szDrive, szDir, szFname, _T(".exe.ini") );
		szPath[_MAX_PATH - 1] = _T('\0');
		int nEnable = ::GetPrivateProfileInt(_T("Settings"), _T("MultiUser"), 0, szPath );
		if( nEnable ){
			int nFolder = ::GetPrivateProfileInt(_T("Settings"), _T("UserRootFolder"), 0, szPath );
			switch( nFolder ){
			case 1:
				nFolder = CSIDL_PROFILE;			// ユーザのルートフォルダ
				break;
			case 2:
				nFolder = CSIDL_PERSONAL;			// ユーザのドキュメントフォルダ
				break;
			case 3:
				nFolder = CSIDL_DESKTOPDIRECTORY;	// ユーザのデスクトップフォルダ
				break;
			default:
				nFolder = CSIDL_APPDATA;			// ユーザのアプリケーションデータフォルダ
				break;
			}
			::GetPrivateProfileString(_T("Settings"), _T("UserSubFolder"), _T("sakura"), szDir, _MAX_DIR, szPath );
			if( szDir[0] == _T('\0') )
				::lstrcpy( szDir, _T("sakura") );
			if( GetSpecialFolderPath( nFolder, szPath ) ){
				_snprintf( pszPrivateIniFile, _MAX_PATH - 1, _T("%s\\%s\\%s%s"), szPath, szDir, szFname, _T(".ini") );
				pszPrivateIniFile[_MAX_PATH - 1] = _T('\0');
			}
		}
	}
	return;
}

/**
	iniファイル名の取得

	共有データからsakura.iniの格納フォルダを取得し、フルパス名を返す
	（共有データ未設定のときは共有データ設定を行う）

	@param[out] pszIniFileName iniファイル名（フルパス）
	@param[in] bRead true: 読み込み / false: 書き込み

	@author ryoji
	@date 2007.05.19 ryoji 新規作成
*/
void CShareData::GetIniFileName( LPTSTR pszIniFileName, BOOL bRead/*=FALSE*/ )
{
	if( !m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit ){
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit = true;			// 初期化済フラグ
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate = false;	// マルチユーザ用iniからの読み出しフラグ
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = false;	// マルチユーザ用iniへの書き込みフラグ

		GetIniFileNameDirect( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile, m_pShareData->m_sFileNameManagement.m_IniFolder.m_szIniFile );
		if( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile[0] != _T('\0') ){
			m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate = true;
			m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = true;
			if( CCommandLine::getInstance()->IsNoWindow() && CCommandLine::getInstance()->IsWriteQuit() )
				m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = false;

			// マルチユーザ用のiniフォルダを作成しておく
			if( m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate ){
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile, szDrive, szDir, NULL, NULL );
				_snprintf( szPath, _MAX_PATH - 1, _T("%s\\%s"), szDrive, szDir );
				szPath[_MAX_PATH - 1] = _T('\0');
				::MakeSureDirectoryPathExists( szPath );
			}
		}
	}

	bool bPrivate = bRead? m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate: m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate;
	::lstrcpy( pszIniFileName, bPrivate? m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile: m_pShareData->m_sFileNameManagement.m_IniFolder.m_szIniFile );
}

/*!
	共有データの読み込み/保存 2

	@param[in] bRead true: 読み込み / false: 書き込み

	@date 2004-01-11 D.S.Koba CProfile変更によるコード簡略化
	@date 2005-04-05 D.S.Koba 各セクションの入出力を関数として分離
*/
bool CShareData::ShareData_IO_2( bool bRead )
{
	//MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	CProfile	cProfile;

	// Feb. 12, 2006 D.S.Koba
	if( bRead ){
		cProfile.SetReadingMode();
	} else {
		cProfile.SetWritingMode();
	}

	TCHAR		szIniFileName[_MAX_PATH + 1];
	GetIniFileName( szIniFileName, bRead );	// 2007.05.19 ryoji iniファイル名を取得する

//	MYTRACE_A( "Iniファイル処理-1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );


	if( bRead ){
		if( !cProfile.ReadProfile( szIniFileName ) ){
			/* 設定ファイルが存在しない */
			return false;
		}
	}
//	MYTRACE_A( "Iniファイル処理 0 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
	
	// Feb. 12, 2006 D.S.Koba
	ShareData_IO_Mru( cProfile );
	ShareData_IO_Keys( cProfile );
	ShareData_IO_Grep( cProfile );
	ShareData_IO_Folders( cProfile );
	ShareData_IO_Cmd( cProfile );
	ShareData_IO_Nickname( cProfile );
	ShareData_IO_Common( cProfile );
	ShareData_IO_Toolbar( cProfile );
	ShareData_IO_CustMenu( cProfile );
	ShareData_IO_Font( cProfile );
	ShareData_IO_KeyBind( cProfile );
	ShareData_IO_Print( cProfile );
	ShareData_IO_Types( cProfile );
	ShareData_IO_KeyWords( cProfile );
	ShareData_IO_Macro( cProfile );
	ShareData_IO_Other( cProfile );
	
	if( !bRead ){
		cProfile.WriteProfile( szIniFileName, _T(" sakura.ini テキストエディタ設定ファイル") );
	}

//	MYTRACE_A( "Iniファイル処理 8 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	MYTRACE_A( "Iniファイル処理 所要時間(ミリ秒) = %d\n", cRunningTimerStart.Read() );

	return true;
}

/*!
	@brief 共有データのMruセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Mru( CProfile& cProfile )
{
	const char* pszSecName = "MRU";
	int			i;
	int			nSize;
	EditInfo*	pfiWork;
	char		szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_MRU_Counts", m_pShareData->m_sHistory.m_nMRUArrNum );
	SetValueLimit( m_pShareData->m_sHistory.m_nMRUArrNum, MAX_MRU );
	nSize = m_pShareData->m_sHistory.m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &m_pShareData->m_sHistory.m_fiMRUArr[i];
		if( cProfile.IsReadingMode() ){
			pfiWork->m_nType = -1;
		}
		wsprintf( szKeyName, "MRU[%02d].nViewTopLine", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewTopLine );
		wsprintf( szKeyName, "MRU[%02d].nViewLeftCol", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
		wsprintf( szKeyName, "MRU[%02d].nX", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nX );
		wsprintf( szKeyName, "MRU[%02d].nY", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nY );
		wsprintf( szKeyName, "MRU[%02d].nCharCode", i );
		cProfile.IOProfileData( pszSecName, szKeyName, (int&)pfiWork->m_nCharCode );
		wsprintf( szKeyName, "MRU[%02d].szPath", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
		wsprintf( szKeyName, "MRU[%02d].szMark", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
		wsprintf( szKeyName, "MRU[%02d].nType", i );
		int nType = pfiWork->m_nType;
		cProfile.IOProfileData( pszSecName, szKeyName, nType );
		pfiWork->m_nType = nType;
		//お気に入り	//@@@ 2003.04.08 MIK
		wsprintf( szKeyName, "MRU[%02d].bFavorite", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_sHistory.m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
	if ( cProfile.IsReadingMode() ){
		EditInfo	fiInit;
		//	残りをfiInitで初期化しておく。
		fiInit.m_nCharCode = CODE_DEFAULT;
		fiInit.m_nViewLeftCol = 0;
		fiInit.m_nViewTopLine = 0;
		fiInit.m_nX = 0;
		fiInit.m_nY = 0;
		_tcscpy( fiInit.m_szPath, _T("") );
		strcpy( fiInit.m_szMarkLines, "" );	// 2002.01.16 hor
		for( ; i < MAX_MRU; ++i){
			m_pShareData->m_sHistory.m_fiMRUArr[i] = fiInit;
			m_pShareData->m_sHistory.m_bMRUArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}

	cProfile.IOProfileData( pszSecName, "_MRUFOLDER_Counts", m_pShareData->m_sHistory.m_nOPENFOLDERArrNum );
	SetValueLimit( m_pShareData->m_sHistory.m_nOPENFOLDERArrNum, MAX_OPENFOLDER );
	nSize = m_pShareData->m_sHistory.m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_sHistory.m_szOPENFOLDERArr[i],
			sizeof( m_pShareData->m_sHistory.m_szOPENFOLDERArr[0] ));
		//お気に入り	//@@@ 2003.04.08 MIK
		strcat( szKeyName, ".bFavorite" );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_sHistory.m_bOPENFOLDERArrFavorite[i] );
	}
	//読み込み時は残りを初期化
	if ( cProfile.IsReadingMode() ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			m_pShareData->m_sHistory.m_szOPENFOLDERArr[i][0] = '\0';
			m_pShareData->m_sHistory.m_bOPENFOLDERArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}
}

/*!
	@brief 共有データのKeysセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Keys( CProfile& cProfile )
{
	const char* pszSecName = "Keys";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum, MAX_SEARCHKEY );
	nSize = m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[0] ));
	}

	cProfile.IOProfileData( pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum, MAX_REPLACEKEY );
	nSize = m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[0] ) );
	}
}

/*!
	@brief 共有データのGrepセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Grep( CProfile& cProfile )
{
	const char* pszSecName = "Grep";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_GREPFILE_Counts", m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum, MAX_GREPFILE );
	nSize = m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[0] ));
	}

	cProfile.IOProfileData( pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum );
	SetValueLimit( m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum, MAX_GREPFOLDER );
	nSize = m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sSearchKeywords.m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_sSearchKeywords.m_szGREPFOLDERArr[0] ));
	}
}

/*!
	@brief 共有データのFoldersセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Folders( CProfile& cProfile )
{
	const char* pszSecName = "Folders";
	/* マクロ用フォルダ */
	cProfile.IOProfileData( pszSecName, "szMACROFOLDER",
		m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, sizeof( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ));
	/* 設定インポート用フォルダ */
	cProfile.IOProfileData( pszSecName, "szIMPORTFOLDER",
		m_pShareData->m_sHistory.m_szIMPORTFOLDER, sizeof( m_pShareData->m_sHistory.m_szIMPORTFOLDER ));
}

/*!
	@brief 共有データのCmdセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Cmd( CProfile& cProfile )
{
	const char* pszSecName = "Cmd";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "nCmdArrNum", m_pShareData->m_sHistory.m_nCmdArrNum );
	SetValueLimit( m_pShareData->m_sHistory.m_nCmdArrNum, MAX_CMDARR );
	int nSize = m_pShareData->m_sHistory.m_nCmdArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "szCmdArr[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sHistory.m_szCmdArr[i], sizeof( m_pShareData->m_sHistory.m_szCmdArr[0] ));
	}
}

/*!
	@brief 共有データのNicknameセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Nickname( CProfile& cProfile )
{
	const char* pszSecName = "Nickname";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "ArrNum", m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum );
	SetValueLimit( m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum, MAX_TRANSFORM_FILENAME );
	int nSize = m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "From%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[0] ));
		wsprintf( szKeyName, "To%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[i], sizeof( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[0] ));
	}
	// 読み込み時，残りをNULLで再初期化
	if( cProfile.IsReadingMode() ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] = '\0';
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[i][0]   = '\0';
		}
	}
}

/*!
	@brief 共有データのCommonセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Common( CProfile& cProfile )
{
	const char* pszSecName = "Common";
	// 2005.04.07 D.S.Koba
	CommonSetting& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, "nCaretType"				, common.m_sGeneral.m_nCaretType );
	//	Oct. 2, 2005 genta
	//	初期値を挿入モードに固定するため，設定の読み書きをやめる
	//cProfile.IOProfileData( pszSecName, "bIsINSMode"			, common.m_sGeneral.m_bIsINSMode );
	cProfile.IOProfileData( pszSecName, "bIsFreeCursorMode"		, common.m_sGeneral.m_bIsFreeCursorMode );
	
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchWord"	, common.m_sGeneral.m_bStopsBothEndsWhenSearchWord );
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchParagraph"	, common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph );
	//	Oct. 27, 2000 genta
	cProfile.IOProfileData( pszSecName, "m_bRestoreCurPosition"	, common.m_sFile.m_bRestoreCurPosition );
	// 2002.01.16 hor
	cProfile.IOProfileData( pszSecName, "m_bRestoreBookmarks"	, common.m_sFile.m_bRestoreBookmarks );
	cProfile.IOProfileData( pszSecName, "bAddCRLFWhenCopy"		, common.m_sEdit.m_bAddCRLFWhenCopy );
	cProfile.IOProfileData( pszSecName, "eOpenDialogDir"		, (int&)common.m_sEdit.m_eOpenDialogDir );
	cProfile.IOProfileData( pszSecName, "szOpenDialogSelDir"		, common.m_sEdit.m_OpenDialogSelDir, sizeof(common.m_sEdit.m_OpenDialogSelDir) );
	cProfile.IOProfileData( pszSecName, "nRepeatedScrollLineNum"	, common.m_sGeneral.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( pszSecName, "nRepeatedScroll_Smooth"	, common.m_sGeneral.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( pszSecName, "nPageScrollByWheel"	, common.m_sGeneral.m_nPageScrollByWheel );	// 2009.01.12 nasukoji
	cProfile.IOProfileData( pszSecName, "nHorizontalScrollByWheel"	, common.m_sGeneral.m_nHorizontalScrollByWheel );	// 2009.01.12 nasukoji
	cProfile.IOProfileData( pszSecName, "bCloseAllConfirm"		, common.m_sGeneral.m_bCloseAllConfirm );	/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
	cProfile.IOProfileData( pszSecName, "bExitConfirm"			, common.m_sGeneral.m_bExitConfirm );
	cProfile.IOProfileData( pszSecName, "bSearchRegularExp"		, common.m_sSearch.m_sSearchOption.bRegularExp );
	cProfile.IOProfileData( pszSecName, "bSearchLoHiCase"		, common.m_sSearch.m_sSearchOption.bLoHiCase );
	cProfile.IOProfileData( pszSecName, "bSearchWordOnly"		, common.m_sSearch.m_sSearchOption.bWordOnly );
	cProfile.IOProfileData( pszSecName, "bSearchConsecutiveAll"	, common.m_sSearch.m_bConsecutiveAll );	// 2007.01.16 ryoji
	cProfile.IOProfileData( pszSecName, "bSearchNOTIFYNOTFOUND"	, common.m_sSearch.m_bNOTIFYNOTFOUND );
	// 2002.01.26 hor
	cProfile.IOProfileData( pszSecName, "bSearchAll"			, common.m_sSearch.m_bSearchAll );
	cProfile.IOProfileData( pszSecName, "bSearchSelectedArea"	, common.m_sSearch.m_bSelectedArea );
	cProfile.IOProfileData( pszSecName, "bGrepSubFolder"		, common.m_sSearch.m_bGrepSubFolder );
	cProfile.IOProfileData( pszSecName, "bGrepOutputLine"		, common.m_sSearch.m_bGrepOutputLine );
	cProfile.IOProfileData( pszSecName, "nGrepOutputStyle"		, common.m_sSearch.m_nGrepOutputStyle );
	cProfile.IOProfileData( pszSecName, "bGrepDefaultFolder"	, common.m_sSearch.m_bGrepDefaultFolder );
	// 2002/09/21 Moca 追加
	cProfile.IOProfileData( pszSecName, "nGrepCharSet"			, (int&)common.m_sSearch.m_nGrepCharSet );
	cProfile.IOProfileData( pszSecName, "bGrepRealTime"			, common.m_sSearch.m_bGrepRealTimeView ); // 2003.06.16 Moca
	cProfile.IOProfileData( pszSecName, "bCaretTextForSearch"	, common.m_sSearch.m_bCaretTextForSearch );	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	
	/* 正規表現DLL 2007.08.12 genta */
	cProfile.IOProfileData( pszSecName, "szRegexpLib"			, common.m_sSearch.m_szRegexpLib, sizeof( common.m_sSearch.m_szRegexpLib ) );
	
	cProfile.IOProfileData( pszSecName, "bGTJW_RETURN"			, common.m_sSearch.m_bGTJW_RETURN );
	cProfile.IOProfileData( pszSecName, "bGTJW_LDBLCLK"			, common.m_sSearch.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( pszSecName, "bBackUp"				, common.m_sBackup.m_bBackUp );
	cProfile.IOProfileData( pszSecName, "bBackUpDialog"			, common.m_sBackup.m_bBackUpDialog );
	cProfile.IOProfileData( pszSecName, "bBackUpFolder"			, common.m_sBackup.m_bBackUpFolder );
	
	if( !cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen( common.m_sBackup.m_szBackUpFolder );
		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
		nCharChars = &common.m_sBackup.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_sBackup.m_szBackUpFolder, nDummy, &common.m_sBackup.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_sBackup.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			_tcscat( common.m_sBackup.m_szBackUpFolder, _T("\\") );
		}
	}
	cProfile.IOProfileData( pszSecName, "szBackUpFolder",
		common.m_sBackup.m_szBackUpFolder, sizeof( common.m_sBackup.m_szBackUpFolder ));
	if( cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen( common.m_sBackup.m_szBackUpFolder );
		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
		nCharChars = &common.m_sBackup.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_sBackup.m_szBackUpFolder, nDummy, &common.m_sBackup.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_sBackup.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			_tcscat( common.m_sBackup.m_szBackUpFolder, _T("\\") );
		}
	}
	
	
	cProfile.IOProfileData( pszSecName, "nBackUpType"			, common.m_sBackup.m_nBackUpType );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt1"		, common.m_sBackup.m_nBackUpType_Opt1 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt2"		, common.m_sBackup.m_nBackUpType_Opt2 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt3"		, common.m_sBackup.m_nBackUpType_Opt3 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt4"		, common.m_sBackup.m_nBackUpType_Opt4 );
	cProfile.IOProfileData( pszSecName, "bBackUpDustBox"		, common.m_sBackup.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
	cProfile.IOProfileData( pszSecName, "bBackUpPathAdvanced"	, common.m_sBackup.m_bBackUpPathAdvanced );	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "szBackUpPathAdvanced"	,
		common.m_sBackup.m_szBackUpPathAdvanced, sizeof( common.m_sBackup.m_szBackUpPathAdvanced ));	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "nFileShareMode"		, common.m_sFile.m_nFileShareMode );
	cProfile.IOProfileData( pszSecName, "szExtHelp",
		common.m_sHelper.m_szExtHelp, sizeof( common.m_sHelper.m_szExtHelp ));
	cProfile.IOProfileData( pszSecName, "szExtHtmlHelp",
		common.m_sHelper.m_szExtHtmlHelp, sizeof( common.m_sHelper.m_szExtHtmlHelp ));
	
	cProfile.IOProfileData( pszSecName, "szMigemoDll",
		common.m_sHelper.m_szMigemoDll, sizeof( common.m_sHelper.m_szMigemoDll ));
	cProfile.IOProfileData( pszSecName, "szMigemoDict",
		common.m_sHelper.m_szMigemoDict, sizeof( common.m_sHelper.m_szMigemoDict ));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		ShareData_IO_Sub_LogFont( cProfile, pszSecName, "khlf", "khps", "khlfFaceName",
			common.m_sHelper.m_lf_kh, common.m_sHelper.m_ps_kh );
	}// Keword Help Font
	

	cProfile.IOProfileData( pszSecName, "nMRUArrNum_MAX"		, common.m_sGeneral.m_nMRUArrNum_MAX );
	SetValueLimit( common.m_sGeneral.m_nMRUArrNum_MAX, MAX_MRU );
	cProfile.IOProfileData( pszSecName, "nOPENFOLDERArrNum_MAX"	, common.m_sGeneral.m_nOPENFOLDERArrNum_MAX );
	SetValueLimit( common.m_sGeneral.m_nOPENFOLDERArrNum_MAX, MAX_OPENFOLDER );
	cProfile.IOProfileData( pszSecName, "bDispTOOLBAR"			, common.m_sWindow.m_bDispTOOLBAR );
	cProfile.IOProfileData( pszSecName, "bDispSTATUSBAR"		, common.m_sWindow.m_bDispSTATUSBAR );
	cProfile.IOProfileData( pszSecName, "bDispFUNCKEYWND"		, common.m_sWindow.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_Place"		, common.m_sWindow.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_GroupNum"	, common.m_sWindow.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca ファンクションキーのグループボタン数
	
	cProfile.IOProfileData( pszSecName, "bDispTabWnd"			, common.m_sTabBar.m_bDispTabWnd );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "bDispTabWndMultiWin"	, common.m_sTabBar.m_bDispTabWndMultiWin );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "szTabWndCaption"		, common.m_sTabBar.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK
	cProfile.IOProfileData( pszSecName, "bSameTabWidth"			, common.m_sTabBar.m_bSameTabWidth );	// 2006.01.28 ryoji タブを等幅にする
	cProfile.IOProfileData( pszSecName, "bDispTabIcon"			, common.m_sTabBar.m_bDispTabIcon );	// 2006.01.28 ryoji タブにアイコンを表示する
	cProfile.IOProfileData( pszSecName, "bSortTabList"			, common.m_sTabBar.m_bSortTabList );	// 2006.05.10 ryoji タブ一覧をソートする
	cProfile.IOProfileData( pszSecName, "bTab_RetainEmptyWin"	, common.m_sTabBar.m_bTab_RetainEmptyWin );	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, "bTab_CloseOneWin"		, common.m_sTabBar.m_bTab_CloseOneWin );	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, "bTab_ListFull"			, common.m_sTabBar.m_bTab_ListFull );	// タブ一覧をフルパス表示する	// 2007.02.28 ryoji
	cProfile.IOProfileData( pszSecName, "bChgWndByWheel"		, common.m_sTabBar.m_bChgWndByWheel );	// 2006.03.26 ryoji マウスホイールでウィンドウ切り替え
	cProfile.IOProfileData( pszSecName, "bNewWindow"			, common.m_sTabBar.m_bNewWindow );	// 外部から起動するときは新しいウインドウで開く

	ShareData_IO_Sub_LogFont( cProfile, pszSecName, "lfTabFont", "lfTabFontPs", "lfTabFaceName",
		common.m_sTabBar.m_tabFont, common.m_sTabBar.m_tabFontPs );
	
	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	cProfile.IOProfileData( pszSecName, "bSplitterWndHScroll"	, common.m_sWindow.m_bSplitterWndHScroll );
	cProfile.IOProfileData( pszSecName, "bSplitterWndVScroll"	, common.m_sWindow.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( pszSecName, "szMidashiKigou"		, common.m_sFormat.m_szMidashiKigou, 0 );
	cProfile.IOProfileData( pszSecName, "szInyouKigou"			, common.m_sFormat.m_szInyouKigou, 0 );
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	cProfile.IOProfileData( pszSecName, "bUseHokan"				, common.m_sHelper.m_bUseHokan );
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	cProfile.IOProfileData( pszSecName, "bSaveWindowSize"		, (int&)common.m_sWindow.m_eSaveWindowSize );
	cProfile.IOProfileData( pszSecName, "nWinSizeType"			, common.m_sWindow.m_nWinSizeType );
	cProfile.IOProfileData( pszSecName, "nWinSizeCX"			, common.m_sWindow.m_nWinSizeCX );
	cProfile.IOProfileData( pszSecName, "nWinSizeCY"			, common.m_sWindow.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*を追加
	cProfile.IOProfileData( pszSecName, "nSaveWindowPos"		, (int&)common.m_sWindow.m_eSaveWindowPos );
	cProfile.IOProfileData( pszSecName, "nWinPosX"				, common.m_sWindow.m_nWinPosX );
	cProfile.IOProfileData( pszSecName, "nWinPosY"				, common.m_sWindow.m_nWinPosY );
	cProfile.IOProfileData( pszSecName, "bTaskTrayUse"			, common.m_sGeneral.m_bUseTaskTray );
	cProfile.IOProfileData( pszSecName, "bTaskTrayStay"			, common.m_sGeneral.m_bStayTaskTray );

	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyCode"		, common.m_sGeneral.m_wTrayMenuHotKeyCode );
	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyMods"		, common.m_sGeneral.m_wTrayMenuHotKeyMods );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DragDrop"			, common.m_sEdit.m_bUseOLE_DragDrop );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DropSource"			, common.m_sEdit.m_bUseOLE_DropSource );
	cProfile.IOProfileData( pszSecName, "bDispExitingDialog"			, common.m_sGeneral.m_bDispExitingDialog );
	cProfile.IOProfileData( pszSecName, "bEnableUnmodifiedOverwrite"	, common.m_sFile.m_bEnableUnmodifiedOverwrite );
	cProfile.IOProfileData( pszSecName, "bSelectClickedURL"			, common.m_sEdit.m_bSelectClickedURL );
	cProfile.IOProfileData( pszSecName, "bGrepExitConfirm"			, common.m_sSearch.m_bGrepExitConfirm );/* Grepモードで保存確認するか */
//	cProfile.IOProfileData( pszSecName, "bRulerDisp"				, common.m_bRulerDisp );/* ルーラー表示 */
	cProfile.IOProfileData( pszSecName, "nRulerHeight"				, common.m_sWindow.m_nRulerHeight );/* ルーラー高さ */
	cProfile.IOProfileData( pszSecName, "nRulerBottomSpace"			, common.m_sWindow.m_nRulerBottomSpace );/* ルーラーとテキストの隙間 */
	cProfile.IOProfileData( pszSecName, "nRulerType"				, common.m_sWindow.m_nRulerType );/* ルーラーのタイプ */
	//	Sep. 18, 2002 genta 追加
	cProfile.IOProfileData( pszSecName, "nLineNumberRightSpace"		, common.m_sWindow.m_nLineNumRightSpace );/* 行番号の右側の隙間 */
	cProfile.IOProfileData( pszSecName, "nVertLineOffset"			, common.m_sWindow.m_nVertLineOffset ); // 2005.11.10 Moca
	cProfile.IOProfileData( pszSecName, "bUseCompotibleBMP"			, common.m_sWindow.m_bUseCompatibleBMP ); // 2007.09.09 Moca
	cProfile.IOProfileData( pszSecName, "bCopyAndDisablSelection"	, common.m_sEdit.m_bCopyAndDisablSelection );/* コピーしたら選択解除 */
	cProfile.IOProfileData( pszSecName, "bEnableNoSelectCopy"		, common.m_sEdit.m_bEnableNoSelectCopy );/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
	cProfile.IOProfileData( pszSecName, "bEnableLineModePaste"		, common.m_sEdit.m_bEnableLineModePaste );/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
	cProfile.IOProfileData( pszSecName, "bConvertEOLPaste"			, common.m_sEdit.m_bConvertEOLPaste );	/* 改行コードを変換して貼り付ける */	// 2009.02.28 salarm
	cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"			, common.m_sHelper.m_bHtmlHelpIsSingle );/* HtmlHelpビューアはひとつ */
	cProfile.IOProfileData( pszSecName, "bCompareAndTileHorz"		, common.m_sCompare.m_bCompareAndTileHorz );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	cProfile.IOProfileData( pszSecName, "bDropFileAndClose"			, common.m_sFile.m_bDropFileAndClose );/* ファイルをドロップしたときは閉じて開く */
	cProfile.IOProfileData( pszSecName, "nDropFileNumMax"			, common.m_sFile.m_nDropFileNumMax );/* 一度にドロップ可能なファイル数 */
	cProfile.IOProfileData( pszSecName, "bCheckFileTimeStamp"		, common.m_sFile.m_bCheckFileTimeStamp );/* 更新の監視 */
	cProfile.IOProfileData( pszSecName, "bNotOverWriteCRLF"			, common.m_sEdit.m_bNotOverWriteCRLF );/* 改行は上書きしない */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFind"			, common.m_sSearch.m_bAutoCloseDlgFind );/* 検索ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFuncList"		, common.m_sOutline.m_bAutoCloseDlgFuncList );/* アウトライン ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgReplace"		, common.m_sSearch.m_bAutoCloseDlgReplace );/* 置換 ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, "bAutoColmnPaste"			, common.m_sEdit.m_bAutoColmnPaste );/* 矩形コピーのテキストは常に矩形貼り付け */
	cProfile.IOProfileData( pszSecName, "NoCaretMoveByActivation"	, common.m_sGeneral.m_bNoCaretMoveByActivation );/* マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta) */
	cProfile.IOProfileData( pszSecName, "bScrollBarHorz"			, common.m_sWindow.m_bScrollBarHorz );/* 水平スクロールバーを使う */
	
	cProfile.IOProfileData( pszSecName, "bHokanKey_RETURN"			, common.m_sHelper.m_bHokanKey_RETURN );/* VK_RETURN 補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, "bHokanKey_TAB"				, common.m_sHelper.m_bHokanKey_TAB );/* VK_TAB    補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, "bHokanKey_RIGHT"			, common.m_sHelper.m_bHokanKey_RIGHT );/* VK_RIGHT  補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, "bHokanKey_SPACE"			, common.m_sHelper.m_bHokanKey_SPACE );/* VK_SPACE  補完決定キーが有効/無効 */
	
	cProfile.IOProfileData( pszSecName, "nDateFormatType"			, common.m_sFormat.m_nDateFormatType );/* 日付書式のタイプ */
	cProfile.IOProfileData( pszSecName, "szDateFormat"				, common.m_sFormat.m_szDateFormat, 0 );//日付書式
	cProfile.IOProfileData( pszSecName, "nTimeFormatType"			, common.m_sFormat.m_nTimeFormatType );/* 時刻書式のタイプ */
	cProfile.IOProfileData( pszSecName, "szTimeFormat"				, common.m_sFormat.m_szTimeFormat, 0 );//時刻書式
	
	cProfile.IOProfileData( pszSecName, "bMenuIcon"					, common.m_sWindow.m_bMenuIcon );//メニューにアイコンを表示する
	cProfile.IOProfileData( pszSecName, "bMenuWChar"				, common.m_sWindow.m_bMenuWChar );//メニューの字化け対策
	cProfile.IOProfileData( pszSecName, "bAutoMIMEdecode"			, common.m_sFile.m_bAutoMIMEdecode );//ファイル読み込み時にMIMEのdecodeを行うか
	cProfile.IOProfileData( pszSecName, "bQueryIfCodeChange"		, common.m_sFile.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
	cProfile.IOProfileData( pszSecName, "bAlertIfFileNotExist"		, common.m_sFile.m_bAlertIfFileNotExist );// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveNew"			, common.m_sFile.m_bNoFilterSaveNew );	// 新規から保存時は全ファイル表示	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveFile"			, common.m_sFile.m_bNoFilterSaveFile );	// 新規以外から保存時は全ファイル表示	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, "bAlertIfLargeFile"			, common.m_sFile.m_bAlertIfLargeFile ); // 開こうとしたファイルが大きい場合に警告する
	cProfile.IOProfileData( pszSecName, "nAlertFileSize"   			, common.m_sFile.m_nAlertFileSize );    // 警告を開始するファイルサイズ(MB単位)

	cProfile.IOProfileData( pszSecName, "bCreateAccelTblEachWin"	, common.m_sKeyBind.m_bCreateAccelTblEachWin);	// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)	// 2009.08.15 nasukoji
	
	/* 「開く」ダイアログのサイズと位置 */
	const char* pszKeyName = "rcOpenDialog";
	const char* pszForm = "%d,%d,%d,%d";
	char		szKeyData[1024];
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&common.m_sOthers.m_rcOpenDialog.left,
				&common.m_sOthers.m_rcOpenDialog.top,
				&common.m_sOthers.m_rcOpenDialog.right,
				&common.m_sOthers.m_rcOpenDialog.bottom
			);
		}
	}else{
		wsprintf(
			szKeyData,
			pszForm,
			common.m_sOthers.m_rcOpenDialog.left,
			common.m_sOthers.m_rcOpenDialog.top,
			common.m_sOthers.m_rcOpenDialog.right,
			common.m_sOthers.m_rcOpenDialog.bottom
		);
		cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, 0 );
	}
	
	//2002.02.08 aroka,hor
	cProfile.IOProfileData( pszSecName, "bMarkUpBlankLineEnable"	, common.m_sOutline.m_bMarkUpBlankLineEnable );
	cProfile.IOProfileData( pszSecName, "bFunclistSetFocusOnJump"	, common.m_sOutline.m_bFunclistSetFocusOnJump );
	
	//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	cProfile.IOProfileData( pszSecName, "szWinCaptionActive"
		, common.m_sWindow.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
	cProfile.IOProfileData( pszSecName, "szWinCaptionInactive"
		, common.m_sWindow.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );
	
	// アウトライン/トピックリスト の位置とサイズを記憶  20060201 aroka
	cProfile.IOProfileData( pszSecName, "bRememberOutlineWindowPos", common.m_sOutline.m_bRememberOutlineWindowPos);
	if( common.m_sOutline.m_bRememberOutlineWindowPos ){
		cProfile.IOProfileData( pszSecName, "widthOutlineWindow"	, common.m_sOutline.m_widthOutlineWindow);
		cProfile.IOProfileData( pszSecName, "heightOutlineWindow", common.m_sOutline.m_heightOutlineWindow);
		cProfile.IOProfileData( pszSecName, "xOutlineWindowPos"	, common.m_sOutline.m_xOutlineWindowPos);
		cProfile.IOProfileData( pszSecName, "yOutlineWindowPos"	, common.m_sOutline.m_yOutlineWindowPos);
	}
	
}

/*!
	@brief 共有データのToolbarセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Toolbar( CProfile& cProfile )
{
	const char* pszSecName = "Toolbar";
	int		i;
	char	szKeyName[64];
	CommonSetting_ToolBar& toolbar = m_pShareData->m_Common.m_sToolBar;

	cProfile.IOProfileData( pszSecName, "bToolBarIsFlat", toolbar.m_bToolBarIsFlat );

	cProfile.IOProfileData( pszSecName, "nToolBarButtonNum", toolbar.m_nToolBarButtonNum );
	SetValueLimit( toolbar.m_nToolBarButtonNum, MAX_TOOLBAR_BUTTON_ITEMS );
	int	nSize = toolbar.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "nTBB[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i< MAX_TOOLBAR_BUTTON_ITEMS; ++i){
			toolbar.m_nToolBarButtonIdxArr[i] = 0;
		}
	}
}

/*!
	@brief 共有データのCustMenuセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_CustMenu( CProfile& cProfile )
{
	const char* pszSecName = "CustMenu";
	int		i, j;
	char	szKeyName[64];
	CommonSetting_CustomMenu& menu = m_pShareData->m_Common.m_sCustomMenu;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		wsprintf( szKeyName, "szCMN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta 最大長指定
		wsprintf( szKeyName, "nCMIN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemNumArr[i] );
		SetValueLimit( menu.m_nCustMenuItemNumArr[i], _countof(menu.m_nCustMenuItemFuncArr[0]) );
		int nSize = menu.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemFuncArr[i][j] );
			wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemKeyArr[i][j] );
		}
	}
}

/*!
	@brief 共有データのFontセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Font( CProfile& cProfile )
{
	const char* pszSecName = "Font";
	CommonSetting_View& view = m_pShareData->m_Common.m_sView;
	ShareData_IO_Sub_LogFont( cProfile, pszSecName, "lf", "nPointSize", "lfFaceName",
		view.m_lf, view.m_nPointSize );

	cProfile.IOProfileData( pszSecName, "bFontIs_FIXED_PITCH", view.m_bFontIs_FIXED_PITCH );
}

/*!
	@brief 共有データのKeyBindセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_KeyBind( CProfile& cProfile )
{
	const char* pszSecName = "KeyBind";
	int		i;
	char	szKeyName[64];
	char	szKeyData[1024];
	int		nSize = m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum;
	for( i = 0; i < nSize; ++i ){
		// 2005.04.07 D.S.Koba
		KEYDATA& keydata = m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[i];
		strcpy( szKeyName, keydata.m_szKeyName );
		
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
					&keydata.m_nFuncCodeArr[0],
					&keydata.m_nFuncCodeArr[1],
					&keydata.m_nFuncCodeArr[2],
					&keydata.m_nFuncCodeArr[3],
					&keydata.m_nFuncCodeArr[4],
					&keydata.m_nFuncCodeArr[5],
					&keydata.m_nFuncCodeArr[6],
					&keydata.m_nFuncCodeArr[7]
				 );
			}
		}else{
			wsprintf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
				keydata.m_nFuncCodeArr[0],
				keydata.m_nFuncCodeArr[1],
				keydata.m_nFuncCodeArr[2],
				keydata.m_nFuncCodeArr[3],
				keydata.m_nFuncCodeArr[4],
				keydata.m_nFuncCodeArr[5],
				keydata.m_nFuncCodeArr[6],
				keydata.m_nFuncCodeArr[7]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
	}
}

/*!
	@brief 共有データのPrintセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Print( CProfile& cProfile )
{
	const char* pszSecName = "Print";
	int		i, j;
	char	szKeyName[64];
	char	szKeyData[1024];
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		// 2005.04.07 D.S.Koba
		PRINTSETTING& printsetting = m_pShareData->m_PrintSettingArr[i];
		wsprintf( szKeyName, "PS[%02d].nInts", i );
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData ) ) ){
				sscanf( szKeyData, pszForm,
					&printsetting.m_nPrintFontWidth		,
					&printsetting.m_nPrintFontHeight		,
					&printsetting.m_nPrintDansuu			,
					&printsetting.m_nPrintDanSpace		,
					&printsetting.m_nPrintLineSpacing		,
					&printsetting.m_nPrintMarginTY		,
					&printsetting.m_nPrintMarginBY		,
					&printsetting.m_nPrintMarginLX		,
					&printsetting.m_nPrintMarginRX		,
					&printsetting.m_nPrintPaperOrientation,
					&printsetting.m_nPrintPaperSize		,
					&printsetting.m_bPrintWordWrap		,
					&printsetting.m_bPrintLineNumber		,
					&printsetting.m_bHeaderUse[0]			,
					&printsetting.m_bHeaderUse[1]			,
					&printsetting.m_bHeaderUse[2]			,
					&printsetting.m_bFooterUse[0]			,
					&printsetting.m_bFooterUse[1]			,
					&printsetting.m_bFooterUse[2]
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				printsetting.m_nPrintFontWidth		,
				printsetting.m_nPrintFontHeight		,
				printsetting.m_nPrintDansuu			,
				printsetting.m_nPrintDanSpace			,
				printsetting.m_nPrintLineSpacing		,
				printsetting.m_nPrintMarginTY			,
				printsetting.m_nPrintMarginBY			,
				printsetting.m_nPrintMarginLX			,
				printsetting.m_nPrintMarginRX			,
				printsetting.m_nPrintPaperOrientation	,
				printsetting.m_nPrintPaperSize		,
				printsetting.m_bPrintWordWrap			,
				printsetting.m_bPrintLineNumber		,
				printsetting.m_bHeaderUse[0]			,
				printsetting.m_bHeaderUse[1]			,
				printsetting.m_bHeaderUse[2]			,
				printsetting.m_bFooterUse[0]			,
				printsetting.m_bFooterUse[1]			,
				printsetting.m_bFooterUse[2]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
		
		wsprintf( szKeyName, "PS[%02d].szSName"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintSettingName	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
		wsprintf( szKeyName, "PS[%02d].szFF"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceHan	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
		wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceZen	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
		for( j = 0; j < 3; ++j ){
			wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
			cProfile.IOProfileData( pszSecName, szKeyName,
				printsetting.m_szHeaderForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
			wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName,
				printsetting.m_szFooterForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
		}
		wsprintf( szKeyName, "PS[%02d].szDriver", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDriverName,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
		wsprintf( szKeyName, "PS[%02d].szDevice", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDeviceName, 
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
		wsprintf( szKeyName, "PS[%02d].szOutput", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterOutputName,
			sizeof( printsetting.m_mdmDevMode.m_szPrinterOutputName ));
		
		// 2002.02.16 hor とりあえず旧設定を変換しとく
		if(0==strcmp(printsetting.m_szHeaderForm[0],"&f") &&
		   0==strcmp(printsetting.m_szFooterForm[0],"&C- &P -")
		){
			strcpy( printsetting.m_szHeaderForm[0], "$f" );
			strcpy( printsetting.m_szFooterForm[0], "" );
			strcpy( printsetting.m_szFooterForm[1], "- $p -" );
		}
		
		//禁則	//@@@ 2002.04.09 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuHead );
		wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuTail );
		wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK
	}
}

/*!
	@brief 共有データのSTypeConfigセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
	@date 2010/04/17 Uchi ループ内をShareData_IO_Type_Oneに分離。
*/
void CShareData::ShareData_IO_Types( CProfile& cProfile )
{
	int		i;
	char	szKey[32];

	for( i = 0; i < MAX_TYPES; ++i ){
		wsprintf( szKey, "Types(%d)", i );
		ShareData_IO_Type_One( cProfile, i, szKey);
	}
}

/*!
@brief 共有データのSTypeConfigセクションの入出力(１個分)
	@param[in,out]	cProfile	INIファイル入出力クラス
	@param[in]		nType		STypeConfigセクション番号
	@param[in]		pszSecName	セクション名

	@date 2010/04/17 Uchi ShareData_IO_TypesOneから分離。
*/
void CShareData::ShareData_IO_Type_One( CProfile& cProfile, int nType, const char* pszSecName)
{
	int		j;
	char	szKeyName[64];
	char	szKeyData[1024];

	// 2005.04.07 D.S.Koba
	STypeConfig& types = m_pShareData->m_Types[nType];
	static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
	strcpy( szKeyName, "nInts" );
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&types.m_nIdx,
				&types.m_nMaxLineKetas,
				&types.m_nColmSpace,
				&types.m_nTabSpace,
				&types.m_nKeyWordSetIdx[0],
				&types.m_nKeyWordSetIdx[1],	//MIK
				&types.m_nStringType,
				&types.m_bLineNumIsCRLF,
				&types.m_nLineTermType,
				&types.m_bWordWrap,
				&types.m_nCurrentPrintSetting
			 );
		}
		// 折り返し幅の最小値は10。少なくとも４ないとハングアップする。 // 20050818 aroka
		if( types.m_nMaxLineKetas < MINLINEKETAS ){
			types.m_nMaxLineKetas = MINLINEKETAS;
		}
	}
	else{
		wsprintf( szKeyData, pszForm,
			types.m_nIdx,
			types.m_nMaxLineKetas,
			types.m_nColmSpace,
			types.m_nTabSpace,
			types.m_nKeyWordSetIdx[0],
			types.m_nKeyWordSetIdx[1],	//MIK
			types.m_nStringType,
			types.m_bLineNumIsCRLF,
			types.m_nLineTermType,
			types.m_bWordWrap,
			types.m_nCurrentPrintSetting
		);
		cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
	}
	// 2005.01.13 MIK Keywordset 3-10
	cProfile.IOProfileData( pszSecName, "nKeywordSelect3",  types.m_nKeyWordSetIdx[2] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect4",  types.m_nKeyWordSetIdx[3] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect5",  types.m_nKeyWordSetIdx[4] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect6",  types.m_nKeyWordSetIdx[5] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect7",  types.m_nKeyWordSetIdx[6] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect8",  types.m_nKeyWordSetIdx[7] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect9",  types.m_nKeyWordSetIdx[8] );
	cProfile.IOProfileData( pszSecName, "nKeywordSelect10", types.m_nKeyWordSetIdx[9] );

	/* 行間のすきま */
	cProfile.IOProfileData( pszSecName, "nLineSpace", types.m_nLineSpace );
	if( cProfile.IsReadingMode() ){
		if( types.m_nLineSpace < /* 1 */ 0 ){
			types.m_nLineSpace = /* 1 */ 0;
		}
		if( types.m_nLineSpace > LINESPACE_MAX ){
			types.m_nLineSpace = LINESPACE_MAX;
		}
	}


	cProfile.IOProfileData( pszSecName, "szTypeName",
		types.m_szTypeName,
		sizeof( m_pShareData->m_Types[0].m_szTypeName ));
	cProfile.IOProfileData( pszSecName, "szTypeExts",
		types.m_szTypeExts,
		sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	cProfile.IOProfileData( pszSecName, "szTabViewString",
		/*&*/types.m_szTabViewString,
		sizeof( types.m_szTabViewString ));
//#endif
	cProfile.IOProfileData( pszSecName, "bTabArrow"			, types.m_bTabArrow );	//@@@ 2003.03.26 MIK
	cProfile.IOProfileData( pszSecName, "bInsSpace"			, types.m_bInsSpace );	// 2001.12.03 hor

	cProfile.IOProfileData( pszSecName, "nTextWrapMethod", types.m_nTextWrapMethod );		// 2008.05.30 nasukoji

	// From Here Sep. 28, 2002 genta / YAZAKI
	if( cProfile.IsReadingMode() ){
		//	Block Comment
		char buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
		//	2004.10.02 Moca 対になるコメント設定がともに読み込まれたときだけ有効な設定と見なす．
		//	ブロックコメントの始まりと終わり．行コメントの記号と桁位置
		bool bRet1, bRet2;
		buffer[0][0] = buffer[1][0] = '\0';
		bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom"	,
			buffer[0], BLOCKCOMMENT_BUFFERSIZE );			
		bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
			buffer[1], BLOCKCOMMENT_BUFFERSIZE );
		if( bRet1 && bRet2 ) types.m_cBlockComments[0].SetBlockCommentRule(buffer[0], buffer[1] );

		//@@@ 2001.03.10 by MIK
		buffer[0][0] = buffer[1][0] = '\0';
		bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
			buffer[0], BLOCKCOMMENT_BUFFERSIZE );
		bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
			buffer[1], BLOCKCOMMENT_BUFFERSIZE );
		if( bRet1 && bRet2 ) types.m_cBlockComments[1].SetBlockCommentRule(buffer[0], buffer[1] );
		
		//	Line Comment
		char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
		int  pos;

		lbuf[0] = '\0'; pos = -1;
		bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment"		,
			lbuf, COMMENT_DELIMITER_BUFFERSIZE );
		bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn"	, pos );
		if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 0, lbuf, pos );

		lbuf[0] = '\0'; pos = -1;
		bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment2"		,
			lbuf, COMMENT_DELIMITER_BUFFERSIZE );
		bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn2", pos );
		if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 1, lbuf, pos );

		lbuf[0] = '\0'; pos = -1;
		bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment3"		,
			lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO 追加
		bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO 追加
		if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 2, lbuf, pos );
	}
	else { // write
		//	Block Comment
		cProfile.IOProfileData( pszSecName, "szBlockCommentFrom"	,
			const_cast<char*>(types.m_cBlockComments[0].getBlockCommentFrom()), 0 );
		cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
			const_cast<char*>(types.m_cBlockComments[0].getBlockCommentTo()), 0 );

		//@@@ 2001.03.10 by MIK
		cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
			const_cast<char*>(types.m_cBlockComments[1].getBlockCommentFrom()), 0 );
		cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
			const_cast<char*>(types.m_cBlockComments[1].getBlockCommentTo()), 0 );

		//	Line Comment
	cProfile.IOProfileData( pszSecName, "szLineComment"		,
		const_cast<char*>(types.m_cLineComment.getLineComment( 0 )), 0 );
	cProfile.IOProfileData( pszSecName, "szLineComment2"		,
		const_cast<char*>(types.m_cLineComment.getLineComment( 1 )), 0 );
	cProfile.IOProfileData( pszSecName, "szLineComment3"		,
		const_cast<char*>(types.m_cLineComment.getLineComment( 2 )), 0 );	//Jun. 01, 2001 JEPRO 追加

	//	From here May 12, 2001 genta
	int pos;
	pos = types.m_cLineComment.getLineCommentPos( 0 );
	cProfile.IOProfileData( pszSecName, "nLineCommentColumn"	, pos );
	pos = types.m_cLineComment.getLineCommentPos( 1 );
	cProfile.IOProfileData( pszSecName, "nLineCommentColumn2", pos );
	pos = types.m_cLineComment.getLineCommentPos( 2 );
	cProfile.IOProfileData( pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO 追加
	//	To here May 12, 2001 genta

	}
	// To Here Sep. 28, 2002 genta / YAZAKI

	cProfile.IOProfileData( pszSecName, "szIndentChars"		,
		types.m_szIndentChars,
		sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
	cProfile.IOProfileData( pszSecName, "cLineTermChar"		, types.m_cLineTermChar );

	cProfile.IOProfileData( pszSecName, "nDefaultOutline"	, types.m_nDefaultOutline );/* アウトライン解析方法 */
	cProfile.IOProfileData( pszSecName, "szOutlineRuleFilename"	,
		types.m_szOutlineRuleFilename,
		sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* アウトライン解析ルールファイル */
	cProfile.IOProfileData( pszSecName, "nSmartIndent"		, types.m_nSmartIndent );/* スマートインデント種別 */
	//	Nov. 20, 2000 genta
	cProfile.IOProfileData( pszSecName, "nImeState"			, types.m_nImeState );	//	IME制御

	//	2001/06/14 Start By asa-o: タイプ別の補完ファイル
	//	Oct. 5, 2002 genta sizeof()で誤ってポインタのサイズを取得していたのを修正
	cProfile.IOProfileData( pszSecName, "szHokanFile"		,
		types.m_szHokanFile,
		sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	補完ファイル
	//	2001/06/14 End

	//	2001/06/19 asa-o
	cProfile.IOProfileData( pszSecName, "bHokanLoHiCase"		, types.m_bHokanLoHiCase );

	//	2003.06.23 Moca ファイル内からの入力補完機能
	cProfile.IOProfileData( pszSecName, "bUseHokanByFile"		, types.m_bUseHokanByFile );

	//@@@ 2002.2.4 YAZAKI
	cProfile.IOProfileData( pszSecName, "szExtHelp"			,
		types.m_szExtHelp,
		sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
		
	cProfile.IOProfileData( pszSecName, "szExtHtmlHelp"		,
		types.m_szExtHtmlHelp,
		sizeof( types.m_szExtHtmlHelp ));
	cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"	, types.m_bHtmlHelpIsSingle ); // 2012.06.30 Fix m_bHokanLoHiCase -> m_bHtmlHelpIsSingle

	cProfile.IOProfileData( pszSecName, "eDefaultCodetype"		, (int&)types.m_eDefaultCodetype );
	cProfile.IOProfileData( pszSecName, "eDefaultEoltype"		, types.m_eDefaultEoltype );
	cProfile.IOProfileData( pszSecName, "bDefaultBom"			, types.m_bDefaultBom );

	cProfile.IOProfileData( pszSecName, "bAutoIndent"			, types.m_bAutoIndent );
	cProfile.IOProfileData( pszSecName, "bAutoIndent_ZENSPACE"	, types.m_bAutoIndent_ZENSPACE );
	cProfile.IOProfileData( pszSecName, "bRTrimPrevLine"		, types.m_bRTrimPrevLine );			// 2005.10.08 ryoji
	cProfile.IOProfileData( pszSecName, "nIndentLayout"			, types.m_nIndentLayout );

	/* 色設定 I/O */
	IO_ColorSet( &cProfile, pszSecName, types.m_ColorInfoArr  );

	// 2005.11.08 Moca 指定桁縦線
	for(j = 0; j < MAX_VERTLINES; j++ ){
		wsprintf( szKeyName, "nVertLineIdx%d", j + 1 );
		cProfile.IOProfileData( pszSecName, szKeyName, types.m_nVertLineIdx[j] );
		if( types.m_nVertLineIdx[j] == 0 ){
			break;
		}
	}

//@@@ 2001.11.17 add start MIK
	{	//正規表現キーワード
		char	*p;
		cProfile.IOProfileData( pszSecName, "bUseRegexKeyword", types.m_bUseRegexKeyword );/* 正規表現キーワード使用するか？ */
		for(j = 0; j < 100; j++)
		{
			wsprintf( szKeyName, "RxKey[%03d]", j );
			if( cProfile.IsReadingMode() )
			{
				types.m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
				types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
				if( cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
				{
					p = strchr(szKeyData, ',');
					if( p )
					{
						*p = '\0';
						types.m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(szKeyData);	//@@@ 2002.04.30
						if( types.m_RegexKeywordArr[j].m_nColorIndex == -1 )	//名前でない
							types.m_RegexKeywordArr[j].m_nColorIndex = atoi(szKeyData);
						p++;
						strcpy(types.m_RegexKeywordArr[j].m_szKeyword, p);
						if( types.m_RegexKeywordArr[j].m_nColorIndex < 0
						 || types.m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
						{
							types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
						}
					}
				}
			}
			// 2002.02.08 hor 未定義値を無視
			else if(lstrlen(types.m_RegexKeywordArr[j].m_szKeyword))
			{
				wsprintf( szKeyData, "%s,%s",
					GetColorNameByIndex( types.m_RegexKeywordArr[j].m_nColorIndex ),
					types.m_RegexKeywordArr[j].m_szKeyword);
				cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
			}
		}
	}
//@@@ 2001.11.17 add end MIK

	/* 禁則 */
	cProfile.IOProfileData( pszSecName, "bKinsokuHead"	, types.m_bKinsokuHead );
	cProfile.IOProfileData( pszSecName, "bKinsokuTail"	, types.m_bKinsokuTail );
	cProfile.IOProfileData( pszSecName, "bKinsokuRet"	, types.m_bKinsokuRet );	//@@@ 2002.04.13 MIK
	cProfile.IOProfileData( pszSecName, "bKinsokuKuto"	, types.m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
	cProfile.IOProfileData( pszSecName, "szKinsokuHead"	,
		types.m_szKinsokuHead,
		sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
	cProfile.IOProfileData( pszSecName, "szKinsokuTail"	,
		types.m_szKinsokuTail,
		sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
	cProfile.IOProfileData( pszSecName, "szKinsokuKuto"	,
		types.m_szKinsokuKuto,
		sizeof( m_pShareData->m_Types[0].m_szKinsokuKuto ));	// 2009.08.07 ryoji
	cProfile.IOProfileData( pszSecName, "bUseDocumentIcon"	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta 変数名誤り修正

//@@@ 2006.04.10 fon ADD-start
	{	/* キーワード辞書 */
		static const char* pszForm = "%d,%s,%s";
		char	*pH, *pT;	/* <pH>keyword<pT> */
		cProfile.IOProfileData( pszSecName, "bUseKeyWordHelp", types.m_bUseKeyWordHelp );	/* キーワード辞書選択を使用するか？ */
//		cProfile.IOProfileData( pszSecName, "nKeyHelpNum", types.m_nKeyHelpNum );				/* 登録辞書数 */
		cProfile.IOProfileData( pszSecName, "bUseKeyHelpAllSearch", types.m_bUseKeyHelpAllSearch );	/* ヒットした次の辞書も検索(&A) */
		cProfile.IOProfileData( pszSecName, "bUseKeyHelpKeyDisp", types.m_bUseKeyHelpKeyDisp );		/* 1行目にキーワードも表示する(&W) */
		cProfile.IOProfileData( pszSecName, "bUseKeyHelpPrefix", types.m_bUseKeyHelpPrefix );		/* 選択範囲で前方一致検索(&P) */
		for(j = 0; j < MAX_KEYHELP_FILE; j++){
			wsprintf( szKeyName, "KDct[%02d]", j );
			/* 読み出し */
			if( cProfile.IsReadingMode() ){
				types.m_KeyHelpArr[j].m_nUse = 0;
				types.m_KeyHelpArr[j].m_szAbout[0] = '\0';
				types.m_KeyHelpArr[j].m_szPath[0] = '\0';
				if( cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
					pH = szKeyData;
					if( NULL != (pT=strchr(pH, ',')) ){
						*pT = '\0';
						types.m_KeyHelpArr[j].m_nUse = atoi( pH );
						pH = pT+1;
						if( NULL != (pT=strchr(pH, ',')) ){
							*pT = '\0';
							strcpy( types.m_KeyHelpArr[j].m_szAbout, pH );
							pH = pT+1;
							if( '\0' != (*pH) ){
								strcpy( types.m_KeyHelpArr[j].m_szPath, pH );
								types.m_nKeyHelpNum = j+1;	// iniに保存せずに、読み出せたファイル分を辞書数とする
							}
						}
					}
				}
			}/* 書き込み */
			else{
				if(_tcslen(types.m_KeyHelpArr[j].m_szPath)){
					wsprintf( szKeyData, pszForm,
						types.m_KeyHelpArr[j].m_nUse,
						types.m_KeyHelpArr[j].m_szAbout,
						types.m_KeyHelpArr[j].m_szPath
					);
					cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
				}
			}
		}
		/* 旧バージョンiniファイルの読み出しサポート */
		if( cProfile.IsReadingMode() ){
			cProfile.IOProfileData( pszSecName, "szKeyWordHelpFile",
			types.m_KeyHelpArr[0].m_szPath, sizeof( types.m_KeyHelpArr[0].m_szPath ) );
		}
	}
//@@@ 2006.04.10 fon ADD-end
}

/*!
	@brief 共有データのKeyWordsセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_KeyWords( CProfile& cProfile )
{
	const char*		pszSecName = "KeyWords";
	int				i, j;
	char			szKeyName[64];
	char			szKeyData[1024];
	CKeyWordSetMgr*	pCKeyWordSetMgr = &m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	int				nKeyWordSetNum = pCKeyWordSetMgr->m_nKeyWordSetNum;

	cProfile.IOProfileData( pszSecName, "nCurrentKeyWordSetIdx"	, pCKeyWordSetMgr->m_nCurrentKeyWordSetIdx );
	bool bIOSuccess = cProfile.IOProfileData( pszSecName, "nKeyWordSetNum", nKeyWordSetNum );
	if( cProfile.IsReadingMode() ){
		// nKeyWordSetNum が読み込めていれば、すべての情報がそろっていると仮定して処理を進める
		if( bIOSuccess ){
			// 2004.11.25 Moca キーワードセットの情報は、直接書き換えないで関数を利用する
			// 初期設定されているため、先に削除しないと固定メモリの確保に失敗する可能性がある
			int  nMemLen = MAX_KEYWORDNUM * ( MAX_KEYWORDLEN + 1 ) + 1;
			char *pszMem = new char[nMemLen];
			pCKeyWordSetMgr->ResetAllKeyWordSet();
			for( i = 0; i < nKeyWordSetNum; ++i ){
				bool bKEYWORDCASE = false;
				int nKeyWordNum = 0;
				//値の取得
				wsprintf( szKeyName, "szSN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, bKEYWORDCASE );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKeyWordNum );

				//追加
				pCKeyWordSetMgr->AddKeyWordSet( szKeyData, bKEYWORDCASE, nKeyWordNum );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen ) ){
					pCKeyWordSetMgr->SetKeyWordArr( i, nKeyWordNum, pszMem );
				}
			}
			delete [] pszMem;
		}
	}else{
		int nSize = pCKeyWordSetMgr->m_nKeyWordSetNum;
		for( i = 0; i < nSize; ++i ){
			wsprintf( szKeyName, "szSN[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName,
				pCKeyWordSetMgr->m_szSetNameArr[i],
				sizeof( pCKeyWordSetMgr->m_szSetNameArr[0] ));
			wsprintf( szKeyName, "nCASE[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_bKEYWORDCASEArr[i] );
			wsprintf( szKeyName, "nKWN[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_nKeyWordNumArr[i] );
			
			int nMemLen = 0;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				nMemLen += strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				nMemLen ++;
			}
			nMemLen ++;
			wsprintf( szKeyName, "szKW[%02d].Size", i );
			cProfile.IOProfileData( pszSecName, szKeyName, nMemLen );
			char* pszMem = new char[nMemLen + 1];	//	May 25, 2003 genta 区切りをTABに変更したので，最後の\0の分を追加
			char* pMem = pszMem;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				//	May 25, 2003 genta 区切りをTABに変更
				int kwlen = strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				memcpy( pMem, pCKeyWordSetMgr->GetKeyWord( i, j ), kwlen );
				pMem += kwlen;
				*pMem++ = '\t';
			}
			*pMem = '\0';
			wsprintf( szKeyName, "szKW[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen );
			delete [] pszMem;
		}
	}
}

/*!
	@brief 共有データのMacroセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Macro( CProfile& cProfile )
{
	const char* pszSecName = "Macro";
	int		i;	
	char	szKeyName[64];
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		MacroRec& macrorec = m_pShareData->m_Common.m_sMacro.m_MacroTable[i];
		//	Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if( !cProfile.IsReadingMode() && !_tcslen(macrorec.m_szName) && !_tcslen(macrorec.m_szFile) ) continue;
		wsprintf( szKeyName, "Name[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szName, MACRONAME_MAX - 1 );
		wsprintf( szKeyName, "File[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szFile, _MAX_PATH );
		wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
	cProfile.IOProfileData( pszSecName, "nMacroOnOpened", m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );	/* オープン後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, "nMacroOnTypeChanged", m_pShareData->m_Common.m_sMacro.m_nMacroOnTypeChanged );	/* タイプ変更後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, "nMacroOnSave", m_pShareData->m_Common.m_sMacro.m_nMacroOnSave );	/* 保存前自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
}

/*!
	@brief 共有データのOtherセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Other( CProfile& cProfile )
{
	const char* pszSecName = "Other";	//セクションを1個作成した。2003.05.12 MIK
	int		i;	
	char	szKeyName[64];

	/* **** その他のダイアログ **** */
	/* 外部コマンド実行の「標準出力を得る」 */
	if(!cProfile.IOProfileData( pszSecName, "nExecFlgOpt"	, m_pShareData->m_nExecFlgOpt ) ){ //	2006.12.03 maru オプション拡張
		cProfile.IOProfileData( pszSecName, "bGetStdout"		, m_pShareData->m_nExecFlgOpt );
	}

	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
	cProfile.IOProfileData( pszSecName, "bLineNumIsCRLF"	, m_pShareData->m_bLineNumIsCRLF );
	
	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( pszSecName, "nDiffFlgOpt"	, m_pShareData->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( pszSecName, "nTagsOpt"		, m_pShareData->m_nTagsOpt );
	cProfile.IOProfileData( pszSecName, "szTagsCmdLine"	, m_pShareData->m_szTagsCmdLine, sizeof( m_pShareData->m_szTagsCmdLine ) );
	
	//From Here 2005.04.03 MIK キーワード指定タグジャンプ
	cProfile.IOProfileData( pszSecName, "_TagJumpKeyword_Counts", m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum );
	for( i = 0; i < m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum; ++i ){
		wsprintf( szKeyName, "TagJumpKeyword[%02d]", i );
		if( i >= m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum ){
			strcpy( m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[i], "" );
		}
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[i], sizeof( m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[0] ));
	}
	cProfile.IOProfileData( pszSecName, "m_bTagJumpICase"		, m_pShareData->m_sTagJump.m_bTagJumpICase );
	cProfile.IOProfileData( pszSecName, "m_bTagJumpAnyWhere"		, m_pShareData->m_sTagJump.m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK キーワード指定タグジャンプの

	//	MIK バージョン情報（書き込みのみ）
	if( ! cProfile.IsReadingMode() ){
		TCHAR	iniVer[256];
		wsprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( m_pShareData->m_sVersion.m_dwProductVersionMS ),
					LOWORD( m_pShareData->m_sVersion.m_dwProductVersionMS ),
					HIWORD( m_pShareData->m_sVersion.m_dwProductVersionLS ),
					LOWORD( m_pShareData->m_sVersion.m_dwProductVersionLS ) );
		cProfile.IOProfileData( pszSecName, _T("szVersion")	, iniVer, sizeof( iniVer ) );
	}
}

/*!
	@brief 色設定 I/O

	指定された色設定を指定されたセクションに書き込む。または
	指定されたセクションからいろ設定を読み込む。

	@param[in,out]	pcProfile		書き出し、読み込み先Profile object (入出力方向はbReadに依存)
	@param[in]		pszSecName		セクション名
	@param[in,out]	pColorInfoArr	書き出し、読み込み対象の色設定へのポインタ (入出力方向はbReadに依存)
*/
void CShareData::IO_ColorSet( CProfile* pcProfile, const char* pszSecName, ColorInfo* pColorInfoArr )
{
	char	szKeyName[256];
	char	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const char* pszForm = "%d,%d,%06x,%06x,%d";
		wsprintf( szKeyName, "C[%s]", g_ColorAttributeArr[j].szName );	//Stonee, 2001/01/12, 2001/01/15
		if( pcProfile->IsReadingMode() ){
			if( pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				pColorInfoArr[j].m_bUnderLine = FALSE;
				sscanf( szKeyData, pszForm,
					&pColorInfoArr[j].m_bDisp,
					&pColorInfoArr[j].m_bFatFont,
					&pColorInfoArr[j].m_colTEXT,
					&pColorInfoArr[j].m_colBACK,
					&pColorInfoArr[j].m_bUnderLine
				 );
			}
			else{
				// 2006.12.07 ryoji
				// sakura Ver1.5.13.1 以前のiniファイルを読んだときにキャレットがテキスト背景色と同じになると
				// ちょっと困るのでキャレット色が読めないときはキャレット色をテキスト色と同じにする
				if( COLORIDX_CARET == j )
					pColorInfoArr[j].m_colTEXT = pColorInfoArr[COLORIDX_TEXT].m_colTEXT;
			}
			// 2006.12.18 ryoji
			// 矛盾設定があれば修復する
			unsigned int fAttribute = g_ColorAttributeArr[j].fAttribute;
			if( 0 != (fAttribute & COLOR_ATTRIB_FORCE_DISP) )
				pColorInfoArr[j].m_bDisp = TRUE;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_BOLD) )
				pColorInfoArr[j].m_bFatFont = FALSE;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )
				pColorInfoArr[j].m_bUnderLine = FALSE;
		}
		else{
			wsprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp,
				pColorInfoArr[j].m_bFatFont,
				pColorInfoArr[j].m_colTEXT,
				pColorInfoArr[j].m_colBACK,
				pColorInfoArr[j].m_bUnderLine
			);
			pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
	}
}


/*!
	@brief タグジャンプ情報の保存

	タグジャンプするときに、タグジャンプ先の情報を保存する。

	@param[in] pTagJump 保存するタグジャンプ情報
	@retval true	保存成功
	@retval false	保存失敗

	@date 2004/06/21 新規作成
	@date 2004/06/22 Moca 一杯になったら一番古い情報を削除しそこに新しい情報を入れる
*/
void CShareData::PushTagJump(const TagJump *pTagJump)
{
	int i = m_pShareData->m_sTagJump.m_TagJumpTop + 1;
	if( MAX_TAGJUMPNUM <= i ){
		i = 0;
	}
	if( m_pShareData->m_sTagJump.m_TagJumpNum < MAX_TAGJUMPNUM ){
		m_pShareData->m_sTagJump.m_TagJumpNum++;
	}
	m_pShareData->m_sTagJump.m_TagJump[i] = *pTagJump;
	m_pShareData->m_sTagJump.m_TagJumpTop = i;
}


/*!
	@brief タグジャンプ情報の参照

	タグジャンプバックするときに、タグジャンプ元の情報を参照する。

	@param[out] pTagJump 参照するタグジャンプ情報
	@retval true	参照成功
	@retval false	参照失敗

	@date 2004/06/21 新規作成
	@date 2004/06/22 Moca SetTagJump変更による修正
*/
bool CShareData::PopTagJump(TagJump *pTagJump)
{
	if( 0 < m_pShareData->m_sTagJump.m_TagJumpNum ){
		*pTagJump = m_pShareData->m_sTagJump.m_TagJump[m_pShareData->m_sTagJump.m_TagJumpTop--];
		if( m_pShareData->m_sTagJump.m_TagJumpTop < 0 ){
			m_pShareData->m_sTagJump.m_TagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		m_pShareData->m_sTagJump.m_TagJumpNum--;
		return true;
	}
	return false;
}

void ShareData_IO_Sub_LogFont( CProfile& cProfile, const char* pszSecName,
	const char* pszKeyLf, const char* pszKeyPointSize, const char* pszKeyFaceName, LOGFONT& lf, int& pointSize )
{
	const TCHAR* pszForm = _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
	TCHAR		szKeyData[1024];

	cProfile.IOProfileData( pszSecName, pszKeyPointSize, pointSize );	// 2009.10.01 ryoji
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, pszKeyLf, szKeyData, sizeof(szKeyData) ) ){
			sscanf( szKeyData, pszForm,
				&lf.lfHeight,
				&lf.lfWidth,
				&lf.lfEscapement,
				&lf.lfOrientation,
				&lf.lfWeight,
				&lf.lfItalic,
				&lf.lfUnderline,
				&lf.lfStrikeOut,
				&lf.lfCharSet,
				&lf.lfOutPrecision,
				&lf.lfClipPrecision,
				&lf.lfQuality,
				&lf.lfPitchAndFamily
			);
			if( pointSize != 0 ){
				// DPI変更してもフォントのポイントサイズが変わらないように
				// ポイント数からピクセル数に変換する
				lf.lfHeight = -DpiPointsToPixels( abs(pointSize), 10 );	// pointSize: 1/10ポイント単位のサイズ
			}else{
				// 初回または古いバージョンからの更新時はポイント数をピクセル数から逆算して仮設定
				pointSize = DpiPixelsToPoints( abs(lf.lfHeight) ) * 10;	// 小数点部分はゼロの扱い（従来フォントダイアログで小数点は指定不可）
			}
		}
	}else{
		wsprintf( szKeyData, pszForm,
			lf.lfHeight,
			lf.lfWidth,
			lf.lfEscapement,
			lf.lfOrientation,
			lf.lfWeight,
			lf.lfItalic,
			lf.lfUnderline,
			lf.lfStrikeOut,
			lf.lfCharSet,
			lf.lfOutPrecision,
			lf.lfClipPrecision,
			lf.lfQuality,
			lf.lfPitchAndFamily
		);
		cProfile.IOProfileData( pszSecName, pszKeyLf, szKeyData, 0 );
	}
	
	cProfile.IOProfileData( pszSecName, pszKeyFaceName, lf.lfFaceName, sizeof(lf.lfFaceName) );
}

/*[EOF]*/
