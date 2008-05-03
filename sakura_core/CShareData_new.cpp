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
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji, genta, maru

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <imagehlp.h>
#include <stdio.h>
#include "CShareData.h"
#include "mymessage.h"
#include "debug/Debug.h"
#include "global.h"
#include "debug/CRunningTimer.h"
#include "CDataProfile.h"
#include "COsVersionInfo.h"
#include "CCommandLine.h"
#include "types/CPropTypes.h" //STR_COLORDATA_HEAD3
#include "util/shell.h"
#include <shlobj.h>
#include "util/string_ex2.h"


//	CShareData_new2.cppと統合
CShareData::CShareData()
{
//	m_pszAppName = GSTR_CSHAREDATA;
	m_hFileMap   = NULL;
	m_pShareData = NULL;
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動
	m_nTransformFileNameCount = -1;
}

// レジストリは使わない。
// 未使用の２関数を削除 2002/2/3 aroka


/**
	構成設定ファイルからiniファイル名を取得する

	sakura.exe.iniからsakura.iniの格納フォルダを取得し、フルパス名を返す

	@param[out] pszPrivateIniFile マルチユーザ用のiniファイルパス
	@param[out] pszIniFile EXE基準のiniファイルパス

	@author ryoji
	@date 2007.09.04 ryoji 新規作成
*/
void CShareData::GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile )
{
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, _countof(szPath)
	);
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );

	auto_snprintf( pszIniFile, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".ini") );
	pszIniFile[_MAX_PATH - 1] = _T('\0');

	// マルチユーザ用のiniファイルパス
	//		exeと同じフォルダに置かれたマルチユーザ構成設定ファイル（sakura.exe.ini）の内容
	//		に従ってマルチユーザ用のiniファイルパスを決める
	pszPrivateIniFile[0] = _T('\0');
	if( COsVersionInfo().IsWin2000_or_later() ){
		auto_snprintf( szPath, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".exe.ini") );
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
				auto_snprintf( pszPrivateIniFile, _MAX_PATH - 1, _T("%ts\\%ts\\%ts%ts"), szPath, szDir, szFname, _T(".ini") );
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
	if( !m_pShareData->m_IniFolder.m_bInit ){
		m_pShareData->m_IniFolder.m_bInit = true;			// 初期化済フラグ
		m_pShareData->m_IniFolder.m_bReadPrivate = false;	// マルチユーザ用iniからの読み出しフラグ
		m_pShareData->m_IniFolder.m_bWritePrivate = false;	// マルチユーザ用iniへの書き込みフラグ

		GetIniFileNameDirect( m_pShareData->m_IniFolder.m_szPrivateIniFile, m_pShareData->m_IniFolder.m_szIniFile );
		if( m_pShareData->m_IniFolder.m_szPrivateIniFile[0] != _T('\0') ){
			m_pShareData->m_IniFolder.m_bReadPrivate = true;
			m_pShareData->m_IniFolder.m_bWritePrivate = true;
			if( CCommandLine::Instance()->IsNoWindow() && CCommandLine::Instance()->IsWriteQuit() )
				m_pShareData->m_IniFolder.m_bWritePrivate = false;

			// マルチユーザ用のiniフォルダを作成しておく
			if( m_pShareData->m_IniFolder.m_bWritePrivate ){
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( m_pShareData->m_IniFolder.m_szPrivateIniFile, szDrive, szDir, NULL, NULL );
				auto_snprintf( szPath, _MAX_PATH - 1, _T("%ts\\%ts"), szDrive, szDir );
				szPath[_MAX_PATH - 1] = _T('\0');
				MakeSureDirectoryPathExistsT( szPath );
			}
		}
	}

	bool bPrivate = bRead? m_pShareData->m_IniFolder.m_bReadPrivate: m_pShareData->m_IniFolder.m_bWritePrivate;
	::lstrcpy( pszIniFileName, bPrivate? m_pShareData->m_IniFolder.m_szPrivateIniFile: m_pShareData->m_IniFolder.m_szIniFile );
}

/*!
	共有データの読み込み/保存 2

	@param[in] bRead true: 読み込み / false: 書き込み

	@date 2004-01-11 D.S.Koba CProfile変更によるコード簡略化
	@date 2005-04-05 D.S.Koba 各セクションの入出力を関数として分離
*/
BOOL CShareData::ShareData_IO_2( bool bRead )
{
	MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	CDataProfile	cProfile;

	// Feb. 12, 2006 D.S.Koba
	if( bRead ){
		cProfile.SetReadingMode();
	} else {
		cProfile.SetWritingMode();
	}

	TCHAR	szIniFileName[_MAX_PATH + 1];
	GetIniFileName( szIniFileName, bRead );	// 2007.05.19 ryoji iniファイル名を取得する

//	MYTRACE_A( "Iniファイル処理-1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );


	if( bRead ){
		if( !cProfile.ReadProfile( szIniFileName ) ){
			/* 設定ファイルが存在しない */
			return FALSE;
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
		cProfile.WriteProfile( szIniFileName, LTEXT(" sakura.ini テキストエディタ設定ファイル") );
	}

//	MYTRACE_A( "Iniファイル処理 8 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	MYTRACE_A( "Iniファイル処理 所要時間(ミリ秒) = %d\n", cRunningTimerStart.Read() );

	return TRUE;
}

/*!
	@brief 共有データのMruセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Mru( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("MRU");
	int			i;
	int			nSize;
	EditInfo*	pfiWork;
	WCHAR		szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("_MRU_Counts"), m_pShareData->m_nMRUArrNum );
	nSize = m_pShareData->m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &m_pShareData->m_fiMRUArr[i];
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nViewTopLine"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewTopLine );
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nViewLeftCol"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nX"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_ptCursor.x );
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nY"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_ptCursor.y );
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nCharCode"), i );
		cProfile.IOProfileData_WrapInt( pszSecName, szKeyName, pfiWork->m_nCharCode );
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szPath"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(pfiWork->m_szPath) );
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pfiWork->m_szMarkLines) );
		//お気に入り	//@@@ 2003.04.08 MIK
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].bFavorite"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
	if ( cProfile.IsReadingMode() ){
		EditInfo	fiInit;
		//	残りをfiInitで初期化しておく。
		fiInit.m_nCharCode = CODE_DEFAULT;
		fiInit.m_nViewLeftCol = CLayoutInt(0);
		fiInit.m_nViewTopLine = CLayoutInt(0);
		fiInit.m_ptCursor.Set(CLogicInt(0), CLogicInt(0));
		_tcscpy( fiInit.m_szPath, _T("") );
		wcscpy( fiInit.m_szMarkLines, L"" );	// 2002.01.16 hor
		for( ; i < MAX_MRU; ++i){
			m_pShareData->m_fiMRUArr[i] = fiInit;
			m_pShareData->m_bMRUArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}

	cProfile.IOProfileData( pszSecName, LTEXT("_MRUFOLDER_Counts"), m_pShareData->m_nOPENFOLDERArrNum );
	nSize = m_pShareData->m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("MRUFOLDER[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_szOPENFOLDERArr[i] );
		//お気に入り	//@@@ 2003.04.08 MIK
		wcscat( szKeyName, LTEXT(".bFavorite") );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_bOPENFOLDERArrFavorite[i] );
	}
	//読み込み時は残りを初期化
	if ( cProfile.IsReadingMode() ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			m_pShareData->m_szOPENFOLDERArr[i][0] = L'\0';
			m_pShareData->m_bOPENFOLDERArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}
}

/*!
	@brief 共有データのKeysセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Keys( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Keys");
	int		i;
	int		nSize;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("_SEARCHKEY_Counts"), m_pShareData->m_aSearchKeys._GetSizeRef() );
	nSize = m_pShareData->m_aSearchKeys.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("SEARCHKEY[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_aSearchKeys[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_SEARCHKEY; ++i){
			m_pShareData->m_aSearchKeys[i][0] = L'\0';
		}
	}

	cProfile.IOProfileData( pszSecName, LTEXT("_REPLACEKEY_Counts"), m_pShareData->m_aReplaceKeys._GetSizeRef() );
	nSize = m_pShareData->m_aReplaceKeys.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("REPLACEKEY[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_aReplaceKeys[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_REPLACEKEY; ++i){
			m_pShareData->m_aReplaceKeys[i][0] = L'\0';
		}
	}
}

/*!
	@brief 共有データのGrepセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Grep( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Grep");
	int		i;
	int		nSize;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("_GREPFILE_Counts"), m_pShareData->m_aGrepFiles._GetSizeRef() );
	nSize = m_pShareData->m_aGrepFiles.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("GREPFILE[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_aGrepFiles[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_GREPFILE; ++i){
			m_pShareData->m_aGrepFiles[i][0] = L'\0';
		}
	}

	cProfile.IOProfileData( pszSecName, LTEXT("_GREPFOLDER_Counts"), m_pShareData->m_aGrepFolders._GetSizeRef() );
	nSize = m_pShareData->m_aGrepFolders.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("GREPFOLDER[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_aGrepFolders[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_GREPFOLDER; ++i){
			m_pShareData->m_aGrepFolders[i][0] = L'\0';
		}
	}
}

/*!
	@brief 共有データのFoldersセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Folders( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Folders");
	/* マクロ用フォルダ */
	cProfile.IOProfileData( pszSecName, LTEXT("szMACROFOLDER"), m_pShareData->m_szMACROFOLDER );
	/* 設定インポート用フォルダ */
	cProfile.IOProfileData( pszSecName, LTEXT("szIMPORTFOLDER"), m_pShareData->m_szIMPORTFOLDER );
}

/*!
	@brief 共有データのCmdセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Cmd( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Cmd");
	int		i;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("nCmdArrNum"), m_pShareData->m_aCommands._GetSizeRef() );
	int nSize = m_pShareData->m_aCommands.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("szCmdArr[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_aCommands[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_CMDARR; ++i){
			m_pShareData->m_aCommands[i][0] = L'\0';
		}
	}
}

/*!
	@brief 共有データのNicknameセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Nickname( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Nickname");
	int		i;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("ArrNum"), m_pShareData->m_nTransformFileNameArrNum );
	int nSize = m_pShareData->m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("From%02d"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(m_pShareData->m_szTransformFileNameFrom[i]) );
		auto_sprintf( szKeyName, LTEXT("To%02d"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(m_pShareData->m_szTransformFileNameTo[i]) );
	}
	// 読み込み時，残りをNULLで再初期化
	if( cProfile.IsReadingMode() ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			m_pShareData->m_szTransformFileNameFrom[i][0] = L'\0';
			m_pShareData->m_szTransformFileNameTo[i][0]   = L'\0';
		}
	}
}

/*!
	@brief 共有データのCommonセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Common( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Common");
	// 2005.04.07 D.S.Koba
	CommonSetting& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, LTEXT("nCaretType")				, common.m_sGeneral.m_nCaretType );
	//	Oct. 2, 2005 genta
	//	初期値を挿入モードに固定するため，設定の読み書きをやめる
	//cProfile.IOProfileData( pszSecName, LTEXT("bIsINSMode")				, common.m_bIsINSMode );
	cProfile.IOProfileData( pszSecName, LTEXT("bIsFreeCursorMode")		, common.m_sGeneral.m_bIsFreeCursorMode );
	
	cProfile.IOProfileData( pszSecName, LTEXT("bStopsBothEndsWhenSearchWord")	, common.m_sGeneral.m_bStopsBothEndsWhenSearchWord );
	cProfile.IOProfileData( pszSecName, LTEXT("bStopsBothEndsWhenSearchParagraph")	, common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph );
	//	Oct. 27, 2000 genta
	cProfile.IOProfileData( pszSecName, LTEXT("m_bRestoreCurPosition")	, common.m_sFile.m_bRestoreCurPosition );
	// 2002.01.16 hor
	cProfile.IOProfileData( pszSecName, LTEXT("m_bRestoreBookmarks")	, common.m_sFile.m_bRestoreBookmarks );
	cProfile.IOProfileData( pszSecName, LTEXT("bAddCRLFWhenCopy")		, common.m_sEdit.m_bAddCRLFWhenCopy );
	cProfile.IOProfileData( pszSecName, LTEXT("nRepeatedScrollLineNum")	, common.m_sGeneral.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( pszSecName, LTEXT("nRepeatedScroll_Smooth")	, common.m_sGeneral.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( pszSecName, LTEXT("bCloseAllConfirm")		, common.m_sGeneral.m_bCloseAllConfirm );	/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bExitConfirm")			, common.m_sGeneral.m_bExitConfirm );
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchRegularExp")	, common.m_sSearch.m_sSearchOption.bRegularExp );
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchLoHiCase")		, common.m_sSearch.m_sSearchOption.bLoHiCase );
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchWordOnly")		, common.m_sSearch.m_sSearchOption.bWordOnly );
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchConsecutiveAll")		, common.m_sSearch.m_bConsecutiveAll );	// 2007.01.16 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchNOTIFYNOTFOUND")	, common.m_sSearch.m_bNOTIFYNOTFOUND );
	// 2002.01.26 hor
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchAll")				, common.m_sSearch.m_bSearchAll );
	cProfile.IOProfileData( pszSecName, LTEXT("bSearchSelectedArea")	, common.m_sSearch.m_bSelectedArea );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepSubFolder")			, common.m_sSearch.m_bGrepSubFolder );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepOutputLine")		, common.m_sSearch.m_bGrepOutputLine );
	cProfile.IOProfileData( pszSecName, LTEXT("nGrepOutputStyle")		, common.m_sSearch.m_nGrepOutputStyle );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepDefaultFolder")		, common.m_sSearch.m_bGrepDefaultFolder );
	// 2002/09/21 Moca 追加
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nGrepCharSet")	, common.m_sSearch.m_nGrepCharSet );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepRealTime")			, common.m_sSearch.m_bGrepRealTimeView ); // 2003.06.16 Moca
	cProfile.IOProfileData( pszSecName, LTEXT("bCaretTextForSearch")	, common.m_sSearch.m_bCaretTextForSearch );	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	
	/* 正規表現DLL 2007.08.12 genta */
	cProfile.IOProfileData( pszSecName, LTEXT("szRegexpLib")			, MakeStringBufferT(common.m_sSearch.m_szRegexpLib) );
	cProfile.IOProfileData( pszSecName, LTEXT("bGTJW_RETURN")			, common.m_sSearch.m_bGTJW_RETURN );
	cProfile.IOProfileData( pszSecName, LTEXT("bGTJW_LDBLCLK")			, common.m_sSearch.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUp")				, common.m_sBackup.m_bBackUp );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpDialog")			, common.m_sBackup.m_bBackUpDialog );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpFolder")			, common.m_sBackup.m_bBackUpFolder );
	
	if( !cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen( common.m_sBackup.m_szBackUpFolder );
		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
		nCharChars = &common.m_sBackup.m_szBackUpFolder[nDummy]
			- CNativeT::GetCharPrev( common.m_sBackup.m_szBackUpFolder, nDummy, &common.m_sBackup.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_sBackup.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			_tcscat( common.m_sBackup.m_szBackUpFolder, _T("\\") );
		}
	}
	cProfile.IOProfileData( pszSecName, LTEXT("szBackUpFolder"), common.m_sBackup.m_szBackUpFolder );
	if( cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen( common.m_sBackup.m_szBackUpFolder );
		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
		nCharChars = &common.m_sBackup.m_szBackUpFolder[nDummy]
			- CNativeT::GetCharPrev( common.m_sBackup.m_szBackUpFolder, nDummy, &common.m_sBackup.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_sBackup.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			_tcscat( common.m_sBackup.m_szBackUpFolder, _T("\\") );
		}
	}
	
	
	cProfile.IOProfileData( pszSecName, LTEXT("nBackUpType")			, common.m_sBackup.m_nBackUpType );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpType2_Opt1")		, common.m_sBackup.m_nBackUpType_Opt1 );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpType2_Opt2")		, common.m_sBackup.m_nBackUpType_Opt2 );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpType2_Opt3")		, common.m_sBackup.m_nBackUpType_Opt3 );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpType2_Opt4")		, common.m_sBackup.m_nBackUpType_Opt4 );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpDustBox")			, common.m_sBackup.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpPathAdvanced")	, common.m_sBackup.m_bBackUpPathAdvanced );	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, LTEXT("szBackUpPathAdvanced")	, common.m_sBackup.m_szBackUpPathAdvanced );	/* 20051107 aroka */
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nFileShareMode")			, common.m_sFile.m_nFileShareMode );
	cProfile.IOProfileData( pszSecName, LTEXT("szExtHelp"), MakeStringBufferT(common.m_sHelper.m_szExtHelp) );
	cProfile.IOProfileData( pszSecName, LTEXT("szExtHtmlHelp"), MakeStringBufferT(common.m_sHelper.m_szExtHtmlHelp) );
	
	cProfile.IOProfileData( pszSecName, LTEXT("szMigemoDll"), MakeStringBufferT(common.m_sHelper.m_szMigemoDll) );
	cProfile.IOProfileData( pszSecName, LTEXT("szMigemoDict"), MakeStringBufferT(common.m_sHelper.m_szMigemoDict) );
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		const TCHAR*	pszForm = _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
		TCHAR		szKeyData[1024];
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, LTEXT("khlf"), MakeStringBufferT(szKeyData) ) ){
				//##########################危険
				_stscanf( szKeyData, pszForm,
					&common.m_sHelper.m_lf_kh.lfHeight,
					&common.m_sHelper.m_lf_kh.lfWidth,
					&common.m_sHelper.m_lf_kh.lfEscapement,
					&common.m_sHelper.m_lf_kh.lfOrientation,
					&common.m_sHelper.m_lf_kh.lfWeight,
					&common.m_sHelper.m_lf_kh.lfItalic,
					&common.m_sHelper.m_lf_kh.lfUnderline,
					&common.m_sHelper.m_lf_kh.lfStrikeOut,
					&common.m_sHelper.m_lf_kh.lfCharSet,
					&common.m_sHelper.m_lf_kh.lfOutPrecision,
					&common.m_sHelper.m_lf_kh.lfClipPrecision,
					&common.m_sHelper.m_lf_kh.lfQuality,
					&common.m_sHelper.m_lf_kh.lfPitchAndFamily
				);
			}
		}else{
			auto_sprintf( szKeyData, pszForm,
				common.m_sHelper.m_lf_kh.lfHeight,
				common.m_sHelper.m_lf_kh.lfWidth,
				common.m_sHelper.m_lf_kh.lfEscapement,
				common.m_sHelper.m_lf_kh.lfOrientation,
				common.m_sHelper.m_lf_kh.lfWeight,
				common.m_sHelper.m_lf_kh.lfItalic,
				common.m_sHelper.m_lf_kh.lfUnderline,
				common.m_sHelper.m_lf_kh.lfStrikeOut,
				common.m_sHelper.m_lf_kh.lfCharSet,
				common.m_sHelper.m_lf_kh.lfOutPrecision,
				common.m_sHelper.m_lf_kh.lfClipPrecision,
				common.m_sHelper.m_lf_kh.lfQuality,
				common.m_sHelper.m_lf_kh.lfPitchAndFamily
			);
			cProfile.IOProfileData( pszSecName, LTEXT("khlf"), MakeStringBufferT(szKeyData) );
		}
	
		cProfile.IOProfileData( pszSecName, LTEXT("khlfFaceName")		, MakeStringBufferT(common.m_sHelper.m_lf_kh.lfFaceName) );
	
	}// Keword Help Font
	
	
	cProfile.IOProfileData( pszSecName, LTEXT("nMRUArrNum_MAX")			, common.m_sGeneral.m_nMRUArrNum_MAX );
	cProfile.IOProfileData( pszSecName, LTEXT("nOPENFOLDERArrNum_MAX")	, common.m_sGeneral.m_nOPENFOLDERArrNum_MAX );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTOOLBAR")			, common.m_sWindow.m_bDispTOOLBAR );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispSTATUSBAR")			, common.m_sWindow.m_bDispSTATUSBAR );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispFUNCKEYWND")		, common.m_sWindow.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( pszSecName, LTEXT("nFUNCKEYWND_Place")		, common.m_sWindow.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( pszSecName, LTEXT("nFUNCKEYWND_GroupNum")	, common.m_sWindow.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca ファンクションキーのグループボタン数
	
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTabWnd")			, common.m_sTabBar.m_bDispTabWnd );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTabWndMultiWin")	, common.m_sTabBar.m_bDispTabWndMultiWin );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("szTabWndCaption")		, MakeStringBufferW(common.m_sTabBar.m_szTabWndCaption) );	//@@@ 2003.06.13 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bSameTabWidth")			, common.m_sTabBar.m_bSameTabWidth );	// 2006.01.28 ryoji タブを等幅にする
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTabIcon")			, common.m_sTabBar.m_bDispTabIcon );	// 2006.01.28 ryoji タブにアイコンを表示する
	cProfile.IOProfileData( pszSecName, LTEXT("bSortTabList")			, common.m_sTabBar.m_bSortTabList );	// 2006.05.10 ryoji タブ一覧をソートする
	cProfile.IOProfileData( pszSecName, LTEXT("bTab_RetainEmptyWin")	, common.m_sTabBar.m_bTab_RetainEmptyWin );	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, LTEXT("bTab_CloseOneWin")	, common.m_sTabBar.m_bTab_CloseOneWin );	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, LTEXT("bTab_ListFull")			, common.m_sTabBar.m_bTab_ListFull );	// タブ一覧をフルパス表示する	// 2007.02.28 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bChgWndByWheel")		, common.m_sTabBar.m_bChgWndByWheel );	// 2006.03.26 ryoji マウスホイールでウィンドウ切り替え
	
	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	cProfile.IOProfileData( pszSecName, LTEXT("bSplitterWndHScroll")	, common.m_sWindow.m_bSplitterWndHScroll );
	cProfile.IOProfileData( pszSecName, LTEXT("bSplitterWndVScroll")	, common.m_sWindow.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( pszSecName, LTEXT("szMidashiKigou")		, MakeStringBufferW(common.m_sFormat.m_szMidashiKigou) );
	cProfile.IOProfileData( pszSecName, LTEXT("szInyouKigou")			, MakeStringBufferW(common.m_sFormat.m_szInyouKigou) );
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	cProfile.IOProfileData( pszSecName, LTEXT("bUseHokan")				, common.m_sHelper.m_bUseHokan );
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	cProfile.IOProfileData( pszSecName, LTEXT("bSaveWindowSize")		, common.m_sWindow.m_nSaveWindowSize );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinSizeType")			, common.m_sWindow.m_nWinSizeType );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinSizeCX")				, common.m_sWindow.m_nWinSizeCX );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinSizeCY")				, common.m_sWindow.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*を追加
	cProfile.IOProfileData( pszSecName, LTEXT("nSaveWindowPos")			, common.m_sWindow.m_nSaveWindowPos );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinPosX")				, common.m_sWindow.m_nWinPosX );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinPosY")				, common.m_sWindow.m_nWinPosY );
	cProfile.IOProfileData( pszSecName, LTEXT("bTaskTrayUse")			, common.m_sGeneral.m_bUseTaskTray );
	cProfile.IOProfileData( pszSecName, LTEXT("bTaskTrayStay")			, common.m_sGeneral.m_bStayTaskTray );

	cProfile.IOProfileData( pszSecName, LTEXT("wTrayMenuHotKeyCode")		, common.m_sGeneral.m_wTrayMenuHotKeyCode );
	cProfile.IOProfileData( pszSecName, LTEXT("wTrayMenuHotKeyMods")		, common.m_sGeneral.m_wTrayMenuHotKeyMods );
	cProfile.IOProfileData( pszSecName, LTEXT("bUseOLE_DragDrop")			, common.m_sEdit.m_bUseOLE_DragDrop );
	cProfile.IOProfileData( pszSecName, LTEXT("bUseOLE_DropSource")			, common.m_sEdit.m_bUseOLE_DropSource );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispExitingDialog")			, common.m_sGeneral.m_bDispExitingDialog );
	cProfile.IOProfileData( pszSecName, LTEXT("bEnableUnmodifiedOverwrite")	, common.m_sFile.m_bEnableUnmodifiedOverwrite );
	cProfile.IOProfileData( pszSecName, LTEXT("bSelectClickedURL")			, common.m_sEdit.m_bSelectClickedURL );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepExitConfirm")			, common.m_sSearch.m_bGrepExitConfirm );/* Grepモードで保存確認するか */
//	cProfile.IOProfileData( pszSecName, LTEXT("bRulerDisp")					, common.m_bRulerDisp );/* ルーラー表示 */
	cProfile.IOProfileData( pszSecName, LTEXT("nRulerHeight")				, common.m_sWindow.m_nRulerHeight );/* ルーラー高さ */
	cProfile.IOProfileData( pszSecName, LTEXT("nRulerBottomSpace")			, common.m_sWindow.m_nRulerBottomSpace );/* ルーラーとテキストの隙間 */
	cProfile.IOProfileData( pszSecName, LTEXT("nRulerType")					, common.m_sWindow.m_nRulerType );/* ルーラーのタイプ */
	//	Sep. 18, 2002 genta 追加
	cProfile.IOProfileData( pszSecName, LTEXT("nLineNumberRightSpace")		, common.m_sWindow.m_nLineNumRightSpace );/* 行番号の右側の隙間 */
	cProfile.IOProfileData( pszSecName, LTEXT("nVertLineOffset")			, common.m_sWindow.m_nVertLineOffset ); // 2005.11.10 Moca
	cProfile.IOProfileData( pszSecName, LTEXT("bUseCompotibleBMP")			, common.m_sWindow.m_bUseCompotibleBMP ); // 2007.09.09 Moca
	cProfile.IOProfileData( pszSecName, LTEXT("bCopyAndDisablSelection")	, common.m_sEdit.m_bCopyAndDisablSelection );/* コピーしたら選択解除 */
	cProfile.IOProfileData( pszSecName, LTEXT("bEnableNoSelectCopy")		, common.m_sEdit.m_bEnableNoSelectCopy );/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bEnableLineModePaste")		, common.m_sEdit.m_bEnableLineModePaste );/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bHtmlHelpIsSingle")			, common.m_sHelper.m_bHtmlHelpIsSingle );/* HtmlHelpビューアはひとつ */
	cProfile.IOProfileData( pszSecName, LTEXT("bCompareAndTileHorz")		, common.m_sCompare.m_bCompareAndTileHorz );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	cProfile.IOProfileData( pszSecName, LTEXT("bDropFileAndClose")			, common.m_sFile.m_bDropFileAndClose );/* ファイルをドロップしたときは閉じて開く */
	cProfile.IOProfileData( pszSecName, LTEXT("nDropFileNumMax")			, common.m_sFile.m_nDropFileNumMax );/* 一度にドロップ可能なファイル数 */
	cProfile.IOProfileData( pszSecName, LTEXT("bCheckFileTimeStamp")		, common.m_sFile.m_bCheckFileTimeStamp );/* 更新の監視 */
	cProfile.IOProfileData( pszSecName, LTEXT("bNotOverWriteCRLF")			, common.m_sEdit.m_bNotOverWriteCRLF );/* 改行は上書きしない */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoCloseDlgFind")			, common.m_sSearch.m_bAutoCloseDlgFind );/* 検索ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoCloseDlgFuncList")		, common.m_sOutline.m_bAutoCloseDlgFuncList );/* アウトライン ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoCloseDlgReplace")		, common.m_sSearch.m_bAutoCloseDlgReplace );/* 置換 ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoColmnPaste")			, common.m_sEdit.m_bAutoColmnPaste );/* 矩形コピーのテキストは常に矩形貼り付け */
	cProfile.IOProfileData( pszSecName, LTEXT("NoCaretMoveByActivation")	, common.m_sGeneral.m_bNoCaretMoveByActivation );/* マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta) */
	cProfile.IOProfileData( pszSecName, LTEXT("bScrollBarHorz")				, common.m_sWindow.m_bScrollBarHorz );/* 水平スクロールバーを使う */

	cProfile.IOProfileData( pszSecName, LTEXT("bHokanKey_RETURN")			, common.m_sHelper.m_bHokanKey_RETURN );/* VK_RETURN 補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, LTEXT("bHokanKey_TAB")				, common.m_sHelper.m_bHokanKey_TAB );/* VK_TAB    補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, LTEXT("bHokanKey_RIGHT")			, common.m_sHelper.m_bHokanKey_RIGHT );/* VK_RIGHT  補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, LTEXT("bHokanKey_SPACE")			, common.m_sHelper.m_bHokanKey_SPACE );/* VK_SPACE  補完決定キーが有効/無効 */
	
	cProfile.IOProfileData( pszSecName, LTEXT("nDateFormatType")			, common.m_sFormat.m_nDateFormatType );/* 日付書式のタイプ */
	cProfile.IOProfileData( pszSecName, LTEXT("szDateFormat")				, MakeStringBufferT(common.m_sFormat.m_szDateFormat) );//日付書式
	cProfile.IOProfileData( pszSecName, LTEXT("nTimeFormatType")			, common.m_sFormat.m_nTimeFormatType );/* 時刻書式のタイプ */
	cProfile.IOProfileData( pszSecName, LTEXT("szTimeFormat")				, MakeStringBufferT(common.m_sFormat.m_szTimeFormat) );//時刻書式
	
	cProfile.IOProfileData( pszSecName, LTEXT("bMenuIcon")					, common.m_sWindow.m_bMenuIcon );//メニューにアイコンを表示する
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoMIMEdecode")			, common.m_sFile.m_bAutoMIMEdecode );//ファイル読み込み時にMIMEのdecodeを行うか
	cProfile.IOProfileData( pszSecName, LTEXT("bQueryIfCodeChange")			, common.m_sFile.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
	cProfile.IOProfileData( pszSecName, LTEXT("bAlertIfFileNotExist")		, common.m_sFile.m_bAlertIfFileNotExist );// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	
	cProfile.IOProfileData( pszSecName, LTEXT("bNoFilterSaveNew")			, common.m_sFile.m_bNoFilterSaveNew );	// 新規から保存時は全ファイル表示	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bNoFilterSaveFile")			, common.m_sFile.m_bNoFilterSaveFile );	// 新規以外から保存時は全ファイル表示	// 2006.11.16 ryoji
	
	/* 「開く」ダイアログのサイズと位置 */
	const WCHAR* pszKeyName = LTEXT("rcOpenDialog");
	const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
	WCHAR		szKeyData[1024];
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData)) ){
			int buf[4];
			scan_ints( szKeyData, pszForm, buf );
			common.m_sOthers.m_rcOpenDialog.left	= buf[0];
			common.m_sOthers.m_rcOpenDialog.top		= buf[1];
			common.m_sOthers.m_rcOpenDialog.right	= buf[2];
			common.m_sOthers.m_rcOpenDialog.bottom	= buf[3];
		}
	}else{
		auto_sprintf(
			szKeyData,
			pszForm,
			common.m_sOthers.m_rcOpenDialog.left,
			common.m_sOthers.m_rcOpenDialog.top,
			common.m_sOthers.m_rcOpenDialog.right,
			common.m_sOthers.m_rcOpenDialog.bottom
		);
		cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData) );
	}
	
	//2002.02.08 aroka,hor
	cProfile.IOProfileData( pszSecName, LTEXT("bMarkUpBlankLineEnable")	, common.m_sOutline.m_bMarkUpBlankLineEnable );
	cProfile.IOProfileData( pszSecName, LTEXT("bFunclistSetFocusOnJump")	, common.m_sOutline.m_bFunclistSetFocusOnJump );
	
	//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	cProfile.IOProfileData( pszSecName, LTEXT("szWinCaptionActive") , MakeStringBufferT(common.m_sWindow.m_szWindowCaptionActive) );
	cProfile.IOProfileData( pszSecName, LTEXT("szWinCaptionInactive"), MakeStringBufferT(common.m_sWindow.m_szWindowCaptionInactive) );
	
	// アウトライン/トピックリスト の位置とサイズを記憶  20060201 aroka
	cProfile.IOProfileData( pszSecName, LTEXT("bRememberOutlineWindowPos"), common.m_sOutline.m_bRememberOutlineWindowPos);
	if( common.m_sOutline.m_bRememberOutlineWindowPos ){
		cProfile.IOProfileData( pszSecName, LTEXT("widthOutlineWindow")	, common.m_sOutline.m_widthOutlineWindow);
		cProfile.IOProfileData( pszSecName, LTEXT("heightOutlineWindow"), common.m_sOutline.m_heightOutlineWindow);
		cProfile.IOProfileData( pszSecName, LTEXT("xOutlineWindowPos")	, common.m_sOutline.m_xOutlineWindowPos);
		cProfile.IOProfileData( pszSecName, LTEXT("yOutlineWindowPos")	, common.m_sOutline.m_yOutlineWindowPos);
	}
	
}

/*!
	@brief 共有データのToolbarセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Toolbar( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Toolbar");
	int		i;
	WCHAR	szKeyName[64];
	CommonSetting_ToolBar& toolbar = m_pShareData->m_Common.m_sToolBar;

	cProfile.IOProfileData( pszSecName, LTEXT("nToolBarButtonNum"), toolbar.m_nToolBarButtonNum );
	cProfile.IOProfileData( pszSecName, LTEXT("bToolBarIsFlat"), toolbar.m_bToolBarIsFlat );
	int	nSize = toolbar.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("nTBB[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i< MAX_TOOLBARBUTTONS; ++i){
			toolbar.m_nToolBarButtonIdxArr[i] = 0;
		}
	}
}

/*!
	@brief 共有データのCustMenuセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_CustMenu( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("CustMenu");
	int		i, j;
	WCHAR	szKeyName[64];
	CommonSetting_CustomMenu& menu = m_pShareData->m_Common.m_sCustomMenu;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		auto_sprintf( szKeyName, LTEXT("szCMN[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(menu.m_szCustMenuNameArr[i]) );	//	Oct. 15, 2001 genta 最大長指定
		auto_sprintf( szKeyName, LTEXT("nCMIN[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemNumArr[i] );
		int nSize = menu.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			auto_sprintf( szKeyName, LTEXT("nCMIF[%02d][%02d]"), i, j );
			cProfile.IOProfileData_WrapInt( pszSecName, szKeyName, menu.m_nCustMenuItemFuncArr[i][j] );
			auto_sprintf( szKeyName, LTEXT("nCMIK[%02d][%02d]"), i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemKeyArr[i][j] );
		}
	}
}

/*!
	@brief 共有データのFontセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Font( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Font");
	const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
	WCHAR		szKeyData[1024];
	CommonSetting_View& view = m_pShareData->m_Common.m_sView;
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, LTEXT("lf"), MakeStringBufferW(szKeyData) ) ){
			int buf[13];
			scan_ints( szKeyData, pszForm, buf );
			view.m_lf.lfHeight			= buf[ 0];
			view.m_lf.lfWidth			= buf[ 1];
			view.m_lf.lfEscapement		= buf[ 2];
			view.m_lf.lfOrientation		= buf[ 3];
			view.m_lf.lfWeight			= buf[ 4];
			view.m_lf.lfItalic			= buf[ 5];
			view.m_lf.lfUnderline		= buf[ 6];
			view.m_lf.lfStrikeOut		= buf[ 7];
			view.m_lf.lfCharSet			= buf[ 8];
			view.m_lf.lfOutPrecision	= buf[ 9];
			view.m_lf.lfClipPrecision	= buf[10];
			view.m_lf.lfQuality			= buf[11];
			view.m_lf.lfPitchAndFamily	= buf[12];
		}
	}else{
		auto_sprintf( szKeyData, pszForm,
			view.m_lf.lfHeight,
			view.m_lf.lfWidth,
			view.m_lf.lfEscapement,
			view.m_lf.lfOrientation,
			view.m_lf.lfWeight,
			view.m_lf.lfItalic,
			view.m_lf.lfUnderline,
			view.m_lf.lfStrikeOut,
			view.m_lf.lfCharSet,
			view.m_lf.lfOutPrecision,
			view.m_lf.lfClipPrecision,
			view.m_lf.lfQuality,
			view.m_lf.lfPitchAndFamily
		);
		cProfile.IOProfileData( pszSecName, LTEXT("lf"), MakeStringBufferW(szKeyData) );
	}
	
	cProfile.IOProfileData( pszSecName, LTEXT("lfFaceName"), MakeStringBufferT(view.m_lf.lfFaceName) );
	
	cProfile.IOProfileData( pszSecName, LTEXT("bFontIs_FIXED_PITCH"), view.m_bFontIs_FIXED_PITCH );
}

/*!
	@brief 共有データのKeyBindセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_KeyBind( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("KeyBind");
	int		i;
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
	int		nSize = m_pShareData->m_nKeyNameArrNum;
	for( i = 0; i < nSize; ++i ){
		// 2005.04.07 D.S.Koba
		KEYDATA& keydata = m_pShareData->m_pKeyNameArr[i];
		_tcstowcs( szKeyName, keydata.m_szKeyName, _countof(szKeyName) );
		
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
				int buf[8];
				scan_ints( szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"), buf );
				keydata.m_nFuncCodeArr[0]	= (EFunctionCode)buf[0];
				keydata.m_nFuncCodeArr[1]	= (EFunctionCode)buf[1];
				keydata.m_nFuncCodeArr[2]	= (EFunctionCode)buf[2];
				keydata.m_nFuncCodeArr[3]	= (EFunctionCode)buf[3];
				keydata.m_nFuncCodeArr[4]	= (EFunctionCode)buf[4];
				keydata.m_nFuncCodeArr[5]	= (EFunctionCode)buf[5];
				keydata.m_nFuncCodeArr[6]	= (EFunctionCode)buf[6];
				keydata.m_nFuncCodeArr[7]	= (EFunctionCode)buf[7];
			}
		}else{
			auto_sprintf( szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"),
				keydata.m_nFuncCodeArr[0],
				keydata.m_nFuncCodeArr[1],
				keydata.m_nFuncCodeArr[2],
				keydata.m_nFuncCodeArr[3],
				keydata.m_nFuncCodeArr[4],
				keydata.m_nFuncCodeArr[5],
				keydata.m_nFuncCodeArr[6],
				keydata.m_nFuncCodeArr[7]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
		}
	}
}

/*!
	@brief 共有データのPrintセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Print( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Print");
	int		i, j;
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		// 2005.04.07 D.S.Koba
		PRINTSETTING& printsetting = m_pShareData->m_PrintSettingArr[i];
		auto_sprintf( szKeyName, LTEXT("PS[%02d].nInts"), i );
		static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
				int buf[19];
				scan_ints( szKeyData, pszForm, buf );
				printsetting.m_nPrintFontWidth			= buf[ 0];
				printsetting.m_nPrintFontHeight			= buf[ 1];
				printsetting.m_nPrintDansuu				= buf[ 2];
				printsetting.m_nPrintDanSpace			= buf[ 3];
				printsetting.m_nPrintLineSpacing		= buf[ 4];
				printsetting.m_nPrintMarginTY			= buf[ 5];
				printsetting.m_nPrintMarginBY			= buf[ 6];
				printsetting.m_nPrintMarginLX			= buf[ 7];
				printsetting.m_nPrintMarginRX			= buf[ 8];
				printsetting.m_nPrintPaperOrientation	= buf[ 9];
				printsetting.m_nPrintPaperSize			= buf[10];
				printsetting.m_bPrintWordWrap			= buf[11];
				printsetting.m_bPrintLineNumber			= buf[12];
				printsetting.m_bHeaderUse[0]			= buf[13];
				printsetting.m_bHeaderUse[1]			= buf[14];
				printsetting.m_bHeaderUse[2]			= buf[15];
				printsetting.m_bFooterUse[0]			= buf[16];
				printsetting.m_bFooterUse[1]			= buf[17];
				printsetting.m_bFooterUse[2]			= buf[18];
			}
		}else{
			auto_sprintf( szKeyData, pszForm,
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
				printsetting.m_bPrintWordWrap?1:0,
				printsetting.m_bPrintLineNumber?1:0,
				printsetting.m_bHeaderUse[0]?1:0,
				printsetting.m_bHeaderUse[1]?1:0,
				printsetting.m_bHeaderUse[2]?1:0,
				printsetting.m_bFooterUse[0]?1:0,
				printsetting.m_bFooterUse[1]?1:0,
				printsetting.m_bFooterUse[2]?1:0
			);
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
		}
		
		auto_sprintf( szKeyName, LTEXT("PS[%02d].szSName")	, i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(printsetting.m_szPrintSettingName) );
		auto_sprintf( szKeyName, LTEXT("PS[%02d].szFF")	, i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(printsetting.m_szPrintFontFaceHan) );
		auto_sprintf( szKeyName, LTEXT("PS[%02d].szFFZ")	, i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(printsetting.m_szPrintFontFaceZen) );
		for( j = 0; j < 3; ++j ){
			auto_sprintf( szKeyName, LTEXT("PS[%02d].szHF[%d]") , i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(printsetting.m_szHeaderForm[j]) );
			auto_sprintf( szKeyName, LTEXT("PS[%02d].szFTF[%d]"), i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(printsetting.m_szFooterForm[j]) );
		}
		auto_sprintf( szKeyName, LTEXT("PS[%02d].szDriver"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(printsetting.m_mdmDevMode.m_szPrinterDriverName) );
		auto_sprintf( szKeyName, LTEXT("PS[%02d].szDevice"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(printsetting.m_mdmDevMode.m_szPrinterDeviceName) );
		auto_sprintf( szKeyName, LTEXT("PS[%02d].szOutput"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(printsetting.m_mdmDevMode.m_szPrinterOutputName) );
		
		// 2002.02.16 hor とりあえず旧設定を変換しとく
		if(0==wcscmp(printsetting.m_szHeaderForm[0],_EDITL("&f")) &&
		   0==wcscmp(printsetting.m_szFooterForm[0],_EDITL("&C- &P -"))
		){
			auto_strcpy( printsetting.m_szHeaderForm[0], _EDITL("$f") );
			auto_strcpy( printsetting.m_szFooterForm[0], _EDITL("") );
			auto_strcpy( printsetting.m_szFooterForm[1], _EDITL("- $p -") );
		}
		
		//禁則	//@@@ 2002.04.09 MIK
		auto_sprintf( szKeyName, LTEXT("PS[%02d].bKinsokuHead"), i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuHead );
		auto_sprintf( szKeyName, LTEXT("PS[%02d].bKinsokuTail"), i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuTail );
		auto_sprintf( szKeyName, LTEXT("PS[%02d].bKinsokuRet"),  i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
		auto_sprintf( szKeyName, LTEXT("PS[%02d].bKinsokuKuto"), i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK
	}
}

/*!
	@brief 共有データのSTypeConfigセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Types( CDataProfile& cProfile )
{
	const WCHAR* pszSecName;
	int		i, j;
	WCHAR	szKey[256];
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];

	for( i = 0; i < MAX_TYPES; ++i ){
		// 2005.04.07 D.S.Koba
		STypeConfig& types = m_pShareData->GetTypeSetting(CTypeConfig(i));
		auto_sprintf( szKey, LTEXT("Types(%d)"), i );
		pszSecName = szKey;
		static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");	//MIK
		auto_strcpy( szKeyName, LTEXT("nInts") );
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
				int buf[11];
				scan_ints( szKeyData, pszForm, buf );
				types.m_nIdx					= buf[ 0];
				types.m_nMaxLineKetas			= buf[ 1];
				types.m_nColmSpace				= buf[ 2];
				types.m_nTabSpace				= buf[ 3];
				types.m_nKeyWordSetIdx[0]		= buf[ 4];
				types.m_nKeyWordSetIdx[1]		= buf[ 5];
				types.m_nStringType				= buf[ 6];
				types.m_bLineNumIsCRLF			= buf[ 7];
				types.m_nLineTermType			= buf[ 8];
				types.m_bWordWrap				= buf[ 9];
				types.m_nCurrentPrintSetting	= buf[10];
			}
			// 折り返し幅の最小値は10。少なくとも４ないとハングアップする。 // 20050818 aroka
			if( types.m_nMaxLineKetas < CLayoutInt(MINLINEKETAS) ){
				types.m_nMaxLineKetas = CLayoutInt(MINLINEKETAS);
			}
		}
		else{
			auto_sprintf( szKeyData, pszForm,
				types.m_nIdx,
				types.m_nMaxLineKetas,
				types.m_nColmSpace,
				types.m_nTabSpace,
				types.m_nKeyWordSetIdx[0],
				types.m_nKeyWordSetIdx[1],
				types.m_nStringType,
				types.m_bLineNumIsCRLF?1:0,
				types.m_nLineTermType,
				types.m_bWordWrap?1:0,
				types.m_nCurrentPrintSetting
			);
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
		}
		// 2005.01.13 MIK Keywordset 3-10
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect3"),  types.m_nKeyWordSetIdx[2] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect4"),  types.m_nKeyWordSetIdx[3] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect5"),  types.m_nKeyWordSetIdx[4] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect6"),  types.m_nKeyWordSetIdx[5] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect7"),  types.m_nKeyWordSetIdx[6] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect8"),  types.m_nKeyWordSetIdx[7] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect9"),  types.m_nKeyWordSetIdx[8] );
		cProfile.IOProfileData( pszSecName, LTEXT("nKeywordSelect10"), types.m_nKeyWordSetIdx[9] );

		/* 行間のすきま */
		if( !cProfile.IsReadingMode() ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > LINESPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				types.m_nLineSpace = LINESPACE_MAX;
			}
		}
		cProfile.IOProfileData( pszSecName, LTEXT("nLineSpace"), types.m_nLineSpace );
		if( cProfile.IsReadingMode() ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > 16 ){
				types.m_nLineSpace = 16;
			}
		}


		cProfile.IOProfileData( pszSecName, LTEXT("szTypeName"), MakeStringBufferT(types.m_szTypeName) );
		cProfile.IOProfileData( pszSecName, LTEXT("szTypeExts"), MakeStringBufferT(types.m_szTypeExts) );
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		cProfile.IOProfileData( pszSecName, LTEXT("szTabViewString"), MakeStringBufferW(types.m_szTabViewString) );
//#endif
		cProfile.IOProfileData( pszSecName, LTEXT("bTabArrow")			, types.m_bTabArrow );	//@@@ 2003.03.26 MIK
		cProfile.IOProfileData( pszSecName, LTEXT("bInsSpace")			, types.m_bInsSpace );	// 2001.12.03 hor

		// From Here Sep. 28, 2002 genta / YAZAKI
		if( cProfile.IsReadingMode() ){
			//	Block Comment
			wchar_t buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
			//	2004.10.02 Moca 対になるコメント設定がともに読み込まれたときだけ有効な設定と見なす．
			//	ブロックコメントの始まりと終わり．行コメントの記号と桁位置
			bool bRet1, bRet2;
			buffer[0][0] = buffer[1][0] = L'\0';
			bRet1 = cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentFrom"), MakeStringBufferW(buffer[0]) );			
			bRet2 = cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo"), MakeStringBufferW(buffer[1]) );
			if( bRet1 && bRet2 ) types.m_cBlockComment.SetBlockCommentRule( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = L'\0';
			bRet1 = cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentFrom2"), MakeStringBufferW(buffer[0]) );
			bRet2 = cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo2")	, MakeStringBufferW(buffer[1]) );
			if( bRet1 && bRet2 ) types.m_cBlockComment.SetBlockCommentRule( 1, buffer[0], buffer[1] );
			
			//	Line Comment
			wchar_t lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
			int  pos;

			lbuf[0] = L'\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( pszSecName, LTEXT("szLineComment")		, MakeStringBufferW(lbuf) );
			bRet2 = cProfile.IOProfileData( pszSecName, LTEXT("nLineCommentColumn")	, pos );
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 0, lbuf, pos );

			lbuf[0] = L'\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( pszSecName, LTEXT("szLineComment2")		, MakeStringBufferW(lbuf) );
			bRet2 = cProfile.IOProfileData( pszSecName, LTEXT("nLineCommentColumn2"), pos );
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 1, lbuf, pos );

			lbuf[0] = L'\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( pszSecName, LTEXT("szLineComment3")		, MakeStringBufferW(lbuf) );	//Jun. 01, 2001 JEPRO 追加
			bRet2 = cProfile.IOProfileData( pszSecName, LTEXT("nLineCommentColumn3"), pos );	//Jun. 01, 2001 JEPRO 追加
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 2, lbuf, pos );
		}
		else { // write
			//	Block Comment
			cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentFrom")	,
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComment.getBlockCommentFrom(0))) );
			cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo")	,
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComment.getBlockCommentTo(0))) );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentFrom2"),
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComment.getBlockCommentFrom(1))) );
			cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo2")	,
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComment.getBlockCommentTo(1))) );

			//	Line Comment
			cProfile.IOProfileData( pszSecName, LTEXT("szLineComment")		,
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cLineComment.getLineComment(0))) );
			cProfile.IOProfileData( pszSecName, LTEXT("szLineComment2")		,
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cLineComment.getLineComment(1))) );
			cProfile.IOProfileData( pszSecName, LTEXT("szLineComment3")		,
				MakeStringBufferW0(const_cast<wchar_t*>(types.m_cLineComment.getLineComment(2))) );	//Jun. 01, 2001 JEPRO 追加

			//	From here May 12, 2001 genta
			int pos;
			pos = types.m_cLineComment.getLineCommentPos( 0 );
			cProfile.IOProfileData( pszSecName, LTEXT("nLineCommentColumn")	, pos );
			pos = types.m_cLineComment.getLineCommentPos( 1 );
			cProfile.IOProfileData( pszSecName, LTEXT("nLineCommentColumn2"), pos );
			pos = types.m_cLineComment.getLineCommentPos( 2 );
			cProfile.IOProfileData( pszSecName, LTEXT("nLineCommentColumn3"), pos );	//Jun. 01, 2001 JEPRO 追加
			//	To here May 12, 2001 genta

		}
		// To Here Sep. 28, 2002 genta / YAZAKI

		cProfile.IOProfileData( pszSecName, LTEXT("szIndentChars")		, MakeStringBufferW(types.m_szIndentChars) );
		cProfile.IOProfileData( pszSecName, LTEXT("cLineTermChar")		, types.m_cLineTermChar );

		cProfile.IOProfileData( pszSecName, LTEXT("nDefaultOutline")	, types.m_nDefaultOutline );/* アウトライン解析方法 */
		cProfile.IOProfileData( pszSecName, LTEXT("szOutlineRuleFilename")	, types.m_szOutlineRuleFilename );/* アウトライン解析ルールファイル */
		cProfile.IOProfileData( pszSecName, LTEXT("nSmartIndent")		, types.m_nSmartIndent );/* スマートインデント種別 */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( pszSecName, LTEXT("nImeState")			, types.m_nImeState );	//	IME制御

		//	2001/06/14 Start By asa-o: タイプ別の補完ファイル
		//	Oct. 5, 2002 genta _countof()で誤ってポインタのサイズを取得していたのを修正
		cProfile.IOProfileData( pszSecName, LTEXT("szHokanFile")		, types.m_szHokanFile );		//	補完ファイル
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( pszSecName, LTEXT("bHokanLoHiCase")		, types.m_bHokanLoHiCase );

		//	2003.06.23 Moca ファイル内からの入力補完機能
		cProfile.IOProfileData( pszSecName, LTEXT("bUseHokanByFile")		, types.m_bUseHokanByFile );

		//@@@ 2002.2.4 YAZAKI
		cProfile.IOProfileData( pszSecName, LTEXT("szExtHelp")			, types.m_szExtHelp );
			
		cProfile.IOProfileData( pszSecName, LTEXT("szExtHtmlHelp")		, types.m_szExtHtmlHelp );
		cProfile.IOProfileData( pszSecName, LTEXT("bHtmlHelpIsSingle")	, types.m_bHokanLoHiCase );

		cProfile.IOProfileData( pszSecName, LTEXT("bAutoIndent")			, types.m_bAutoIndent );
		cProfile.IOProfileData( pszSecName, LTEXT("bAutoIndent_ZENSPACE")	, types.m_bAutoIndent_ZENSPACE );
		cProfile.IOProfileData( pszSecName, LTEXT("bRTrimPrevLine")			, types.m_bRTrimPrevLine );			// 2005.10.08 ryoji
		cProfile.IOProfileData( pszSecName, LTEXT("nIndentLayout")			, types.m_nIndentLayout );

		/* 色設定 I/O */
		IO_ColorSet( &cProfile, pszSecName, types.m_ColorInfoArr  );

		// 2005.11.08 Moca 指定桁縦線
		for(j = 0; j < MAX_VERTLINES; j++ ){
			auto_sprintf( szKeyName, LTEXT("nVertLineIdx%d"), j + 1 );
			cProfile.IOProfileData( pszSecName, szKeyName, types.m_nVertLineIdx[j] );
			if( types.m_nVertLineIdx[j] == 0 ){
				break;
			}
		}

//@@@ 2001.11.17 add start MIK
		{	//正規表現キーワード
			WCHAR	*p;
			cProfile.IOProfileData( pszSecName, LTEXT("bUseRegexKeyword"), types.m_bUseRegexKeyword );/* 正規表現キーワード使用するか？ */
			for(j = 0; j < 100; j++)
			{
				auto_sprintf( szKeyName, LTEXT("RxKey[%03d]"), j );
				if( cProfile.IsReadingMode() )
				{
					types.m_RegexKeywordArr[j].m_szKeyword[0] = L'\0';
					types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
					if( cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData)) )
					{
						p = wcschr(szKeyData, LTEXT(','));
						if( p )
						{
							*p = LTEXT('\0');
							types.m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(to_tchar(szKeyData));	//@@@ 2002.04.30
							if( types.m_RegexKeywordArr[j].m_nColorIndex == -1 )	//名前でない
								types.m_RegexKeywordArr[j].m_nColorIndex = _wtoi(szKeyData);
							p++;
							wcscpy(types.m_RegexKeywordArr[j].m_szKeyword, p);
							if( types.m_RegexKeywordArr[j].m_nColorIndex < 0
							 || types.m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
							{
								types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
							}
						}
					}
				}
				// 2002.02.08 hor 未定義値を無視
				else if(wcslen(types.m_RegexKeywordArr[j].m_szKeyword))
				{
					auto_sprintf( szKeyData, LTEXT("%ls,%ls"),
						GetColorNameByIndex( types.m_RegexKeywordArr[j].m_nColorIndex ),
						types.m_RegexKeywordArr[j].m_szKeyword);
					cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
				}
			}
		}
//@@@ 2001.11.17 add end MIK

		/* 禁則 */
		cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuHead")	, types.m_bKinsokuHead );
		cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuTail")	, types.m_bKinsokuTail );
		cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuRet")	, types.m_bKinsokuRet );	//@@@ 2002.04.13 MIK
		cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuKuto")	, types.m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
		cProfile.IOProfileData( pszSecName, LTEXT("szKinsokuHead")	, MakeStringBufferW(types.m_szKinsokuHead) );
		cProfile.IOProfileData( pszSecName, LTEXT("szKinsokuTail")	, MakeStringBufferW(types.m_szKinsokuTail) );
		cProfile.IOProfileData( pszSecName, LTEXT("bUseDocumentIcon")	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta 変数名誤り修正

//@@@ 2006.04.10 fon ADD-start
		{	/* キーワード辞書 */
			WCHAR	*pH, *pT;	/* <pH>keyword<pT> */
			cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyWordHelp"), types.m_bUseKeyWordHelp );	/* キーワード辞書選択を使用するか？ */
//			cProfile.IOProfileData( pszSecName, LTEXT("nKeyHelpNum"), types.m_nKeyHelpNum );				/* 登録辞書数 */
			cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyHelpAllSearch"), types.m_bUseKeyHelpAllSearch );	/* ヒットした次の辞書も検索(&A) */
			cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyHelpKeyDisp"), types.m_bUseKeyHelpKeyDisp );		/* 1行目にキーワードも表示する(&W) */
			cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyHelpPrefix"), types.m_bUseKeyHelpPrefix );		/* 選択範囲で前方一致検索(&P) */
			for(j = 0; j < MAX_KEYHELP_FILE; j++){
				auto_sprintf( szKeyName, LTEXT("KDct[%02d]"), j );
				/* 読み出し */
				if( cProfile.IsReadingMode() ){
					types.m_KeyHelpArr[j].m_bUse = false;
					types.m_KeyHelpArr[j].m_szAbout[0] = _T('\0');
					types.m_KeyHelpArr[j].m_szPath[0] = _T('\0');
					if( cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData)) ){
						pH = szKeyData;
						if( NULL != (pT=wcschr(pH, L',')) ){
							*pT = L'\0';
							types.m_KeyHelpArr[j].m_bUse = (_wtoi( pH )!=0);
							pH = pT+1;
							if( NULL != (pT=wcschr(pH, L',')) ){
								*pT = L'\0';
								_wcstotcs( types.m_KeyHelpArr[j].m_szAbout, pH, _countof(types.m_KeyHelpArr[j].m_szAbout) );
								pH = pT+1;
								if( NULL != (*pH) ){
									_wcstotcs( types.m_KeyHelpArr[j].m_szPath, pH, _countof2(types.m_KeyHelpArr[j].m_szPath) );
									types.m_nKeyHelpNum = j+1;	// iniに保存せずに、読み出せたファイル分を辞書数とする
								}
							}
						}
					}
				}/* 書き込み */
				else{
					if(_tcslen(types.m_KeyHelpArr[j].m_szPath)){
						auto_sprintf( szKeyData, LTEXT("%d,%ts,%ts"),
							types.m_KeyHelpArr[j].m_bUse?1:0,
							types.m_KeyHelpArr[j].m_szAbout,
							types.m_KeyHelpArr[j].m_szPath.c_str()
						);
						cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
					}
				}
			}
			/* 旧バージョンiniファイルの読み出しサポート */
			if( cProfile.IsReadingMode() ){
				SFilePath tmp;
				if(cProfile.IOProfileData( pszSecName, LTEXT("szKeyWordHelpFile"), tmp )){
					types.m_KeyHelpArr[0].m_szPath = tmp;
				}
			}
		}
//@@@ 2006.04.10 fon ADD-end

	}/* for */
}

/*!
	@brief 共有データのKeyWordsセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_KeyWords( CDataProfile& cProfile )
{
	const WCHAR*		pszSecName = LTEXT("KeyWords");
	int				i, j;
	WCHAR			szKeyName[64];
	WCHAR			szKeyData[1024];
	CKeyWordSetMgr*	pCKeyWordSetMgr = &m_pShareData->m_CKeyWordSetMgr;
	int				nKeyWordSetNum = pCKeyWordSetMgr->m_nKeyWordSetNum;

	cProfile.IOProfileData( pszSecName, LTEXT("nCurrentKeyWordSetIdx")	, pCKeyWordSetMgr->m_nCurrentKeyWordSetIdx );
	bool bIOSuccess = cProfile.IOProfileData( pszSecName, LTEXT("nKeyWordSetNum"), nKeyWordSetNum );
	if( cProfile.IsReadingMode() ){
		// nKeyWordSetNum が読み込めていれば、すべての情報がそろっていると仮定して処理を進める
		if( bIOSuccess ){
			// 2004.11.25 Moca キーワードセットの情報は、直接書き換えないで関数を利用する
			// 初期設定されているため、先に削除しないと固定メモリの確保に失敗する可能性がある
			int  nMemLen = MAX_KEYWORDNUM * ( MAX_KEYWORDLEN + 1 ) + 1;
			wchar_t *pszMem = new wchar_t[nMemLen];
			pCKeyWordSetMgr->ResetAllKeyWordSet();
			for( i = 0; i < nKeyWordSetNum; ++i ){
				bool bKEYWORDCASE = false;
				int nKeyWordNum = 0;
				//値の取得
				auto_sprintf( szKeyName, LTEXT("szSN[%02d]"), i );
				cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
				auto_sprintf( szKeyName, LTEXT("nCASE[%02d]"), i );
				cProfile.IOProfileData( pszSecName, szKeyName, bKEYWORDCASE );
				auto_sprintf( szKeyName, LTEXT("nKWN[%02d]"), i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKeyWordNum );

				//追加
				pCKeyWordSetMgr->AddKeyWordSet( szKeyData, bKEYWORDCASE, nKeyWordNum );
				auto_sprintf( szKeyName, LTEXT("szKW[%02d]"), i );
				if( cProfile.IOProfileData( pszSecName, szKeyName, StringBufferW(pszMem,nMemLen)) ){
					pCKeyWordSetMgr->SetKeyWordArr( i, nKeyWordNum, pszMem );
				}
			}
			delete [] pszMem;
		}
	}else{
		int nSize = pCKeyWordSetMgr->m_nKeyWordSetNum;
		for( i = 0; i < nSize; ++i ){
			auto_sprintf( szKeyName, LTEXT("szSN[%02d]"), i );
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pCKeyWordSetMgr->m_szSetNameArr[i]) );
			auto_sprintf( szKeyName, LTEXT("nCASE[%02d]"), i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_bKEYWORDCASEArr[i] );
			auto_sprintf( szKeyName, LTEXT("nKWN[%02d]"), i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_nKeyWordNumArr[i] );
			
			int nMemLen = 0;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				nMemLen += wcslen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				nMemLen ++;
			}
			nMemLen ++;
			auto_sprintf( szKeyName, LTEXT("szKW[%02d].Size"), i );
			cProfile.IOProfileData( pszSecName, szKeyName, nMemLen );
			wchar_t* pszMem = new wchar_t[nMemLen + 1];	//	May 25, 2003 genta 区切りをTABに変更したので，最後の\0の分を追加
			wchar_t* pMem = pszMem;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				//	May 25, 2003 genta 区切りをTABに変更
				int kwlen = wcslen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				auto_memcpy( pMem, pCKeyWordSetMgr->GetKeyWord( i, j ), kwlen );
				pMem += kwlen;
				*pMem++ = L'\t';
			}
			*pMem = L'\0';
			auto_sprintf( szKeyName, LTEXT("szKW[%02d]"), i );
			cProfile.IOProfileData( pszSecName, szKeyName, StringBufferW(pszMem,nMemLen) );
			delete [] pszMem;
		}
	}
}

/*!
	@brief 共有データのMacroセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Macro( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Macro");
	int		i;	
	WCHAR	szKeyName[64];
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		MacroRec& macrorec = m_pShareData->m_MacroTable[i];
		//	Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if( !cProfile.IsReadingMode() && !_tcslen(macrorec.m_szName) && !_tcslen(macrorec.m_szFile) ) continue;
		auto_sprintf( szKeyName, LTEXT("Name[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(macrorec.m_szName) );
		auto_sprintf( szKeyName, LTEXT("File[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(macrorec.m_szFile) );
		auto_sprintf( szKeyName, LTEXT("ReloadWhenExecute[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
}

/*!
	@brief 共有データのOtherセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Other( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Other");	//セクションを1個作成した。2003.05.12 MIK
	int		i;	
	WCHAR	szKeyName[64];

	/* **** その他のダイアログ **** */
	/* 外部コマンド実行の「標準出力を得る」 */
	if(!cProfile.IOProfileData( pszSecName, LTEXT("nExecFlgOpt")	, m_pShareData->m_nExecFlgOpt ) ){ //	2006.12.03 maru オプション拡張
		cProfile.IOProfileData( pszSecName, LTEXT("bGetStdout")		, m_pShareData->m_nExecFlgOpt );
	}

	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
	cProfile.IOProfileData( pszSecName, LTEXT("bLineNumIsCRLF")	, m_pShareData->m_bLineNumIsCRLF );
	
	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("nDiffFlgOpt")	, m_pShareData->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("nTagsOpt")		, m_pShareData->m_nTagsOpt );
	cProfile.IOProfileData( pszSecName, LTEXT("szTagsCmdLine")	, MakeStringBufferT(m_pShareData->m_szTagsCmdLine) );
	
	//From Here 2005.04.03 MIK キーワード指定タグジャンプ
	cProfile.IOProfileData( pszSecName, LTEXT("_TagJumpKeyword_Counts"), m_pShareData->m_aTagJumpKeywords._GetSizeRef() );
	for( i = 0; i < m_pShareData->m_aTagJumpKeywords.size(); ++i ){
		auto_sprintf( szKeyName, LTEXT("TagJumpKeyword[%02d]"), i );
		if( i >= m_pShareData->m_aTagJumpKeywords.size() ){
			wcscpy( m_pShareData->m_aTagJumpKeywords[i], LTEXT("") );
		}
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_aTagJumpKeywords[i] );
	}
	cProfile.IOProfileData( pszSecName, LTEXT("m_bTagJumpICase")		, m_pShareData->m_bTagJumpICase );
	cProfile.IOProfileData( pszSecName, LTEXT("m_bTagJumpAnyWhere")		, m_pShareData->m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK キーワード指定タグジャンプの

	//	MIK バージョン情報（書き込みのみ）
	if( ! cProfile.IsReadingMode() ){
		TCHAR	iniVer[256];
		auto_sprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( m_pShareData->m_dwProductVersionMS ),
					LOWORD( m_pShareData->m_dwProductVersionMS ),
					HIWORD( m_pShareData->m_dwProductVersionLS ),
					LOWORD( m_pShareData->m_dwProductVersionLS ) );
		cProfile.IOProfileData( pszSecName, LTEXT("szVersion"), MakeStringBufferT(iniVer) );
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
void CShareData::IO_ColorSet( CDataProfile* pcProfile, const WCHAR* pszSecName, ColorInfo* pColorInfoArr )
{
	WCHAR	szKeyName[256];
	WCHAR	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const WCHAR* pszForm = LTEXT("%d,%d,%06x,%06x,%d");
		auto_sprintf( szKeyName, LTEXT("C[%ts]"), g_ColorAttributeArr[j].szName );	//Stonee, 2001/01/12, 2001/01/15
		if( pcProfile->IsReadingMode() ){
			if( pcProfile->IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
				pColorInfoArr[j].m_bUnderLine = false;
				int buf[5];
				scan_ints( szKeyData, pszForm, buf);
				pColorInfoArr[j].m_bDisp      = (buf[0]!=0);
				pColorInfoArr[j].m_bFatFont   = (buf[1]!=0);
				pColorInfoArr[j].m_colTEXT    = buf[2];
				pColorInfoArr[j].m_colBACK    = buf[3];
				pColorInfoArr[j].m_bUnderLine = (buf[4]!=0);
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
				pColorInfoArr[j].m_bDisp = true;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_BOLD) )
				pColorInfoArr[j].m_bFatFont = false;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )
				pColorInfoArr[j].m_bUnderLine = false;
		}
		else{
			auto_sprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp?1:0,
				pColorInfoArr[j].m_bFatFont?1:0,
				pColorInfoArr[j].m_colTEXT,
				pColorInfoArr[j].m_colBACK,
				pColorInfoArr[j].m_bUnderLine?1:0
			);
			pcProfile->IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
		}
	}
	return;
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
	int i = m_pShareData->m_TagJumpTop + 1;
	if( MAX_TAGJUMPNUM <= i ){
		i = 0;
	}
	if( m_pShareData->m_TagJumpNum < MAX_TAGJUMPNUM ){
		m_pShareData->m_TagJumpNum++;
	}
	m_pShareData->m_TagJump[i] = *pTagJump;
	m_pShareData->m_TagJumpTop = i;
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
	if( 0 < m_pShareData->m_TagJumpNum ){
		*pTagJump = m_pShareData->m_TagJump[m_pShareData->m_TagJumpTop--];
		if( m_pShareData->m_TagJumpTop < 0 ){
			m_pShareData->m_TagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		m_pShareData->m_TagJumpNum--;
		return true;
	}
	return false;
}




