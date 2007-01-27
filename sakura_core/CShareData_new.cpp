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
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "mymessage.h"
#include "debug.h"
#include "global.h"
#include "CRunningTimer.h"
#include "CProfile.h"
#include "etc_uty.h"

#define STR_COLORDATA_HEAD3		" テキストエディタ色設定 Ver3"	//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした



//	CShareData_new2.cppと統合
CShareData::CShareData()
{
//	m_pszAppName = GSTR_CSHAREDATA;
	m_hFileMap   = NULL;
	m_pShareData = NULL;
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動
	m_nTransformFileNameCount = -1;
	return;
}

// レジストリは使わない。
// 未使用の２関数を削除 2002/2/3 aroka

/*!
	共有データの読み込み/保存 2

	@param[in] bRead true: 読み込み / false: 書き込み

	@date 2004-01-11 D.S.Koba CProfile変更によるコード簡略化
	@date 2005-04-05 D.S.Koba 各セクションの入出力を関数として分離
*/
BOOL CShareData::ShareData_IO_2( bool bRead )
{
	MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	char		szIniFileName[_MAX_PATH + 1];
	CProfile	cProfile;
	char		szPath[_MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE];
	char		szDir[_MAX_DIR];
	char		szFname[_MAX_FNAME];
	char		szExt[_MAX_EXT];

	// Feb. 12, 2006 D.S.Koba
	if( bRead ){
		cProfile.SetReadingMode();
	} else {
		cProfile.SetWritingMode();
	}

	/* exeのあるフォルダ */
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof(szPath)
	);
	_splitpath( szPath, szDrive, szDir, szFname, szExt );
	_makepath( szIniFileName, szDrive, szDir, szFname, "ini" );


//	MYTRACE( "Iniファイル処理-1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );


	if( bRead ){
		if( false == cProfile.ReadProfile( szIniFileName ) ){
			/* 設定ファイルが存在しない */
			return FALSE;
		}
#ifdef _DEBUG
//@@@ 2001.12.26 YAZAKI デバッグ版が正常に起動しないため。
//		cProfile.DUMP();
#endif

	}
//	MYTRACE( "Iniファイル処理 0 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
	
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
		cProfile.WriteProfile( szIniFileName, " sakura.ini テキストエディタ設定ファイル" );
	}

//	MYTRACE( "Iniファイル処理 8 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	MYTRACE( "Iniファイル処理 所要時間(ミリ秒) = %d\n", cRunningTimerStart.Read() );

	return TRUE;
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
	FileInfo*	pfiWork;
	char		szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_MRU_Counts", m_pShareData->m_nMRUArrNum );
	nSize = m_pShareData->m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &m_pShareData->m_fiMRUArr[i];
		wsprintf( szKeyName, "MRU[%02d].nViewTopLine", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewTopLine );
		wsprintf( szKeyName, "MRU[%02d].nViewLeftCol", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
		wsprintf( szKeyName, "MRU[%02d].nX", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nX );
		wsprintf( szKeyName, "MRU[%02d].nY", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nY );
		wsprintf( szKeyName, "MRU[%02d].nCharCode", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nCharCode );
		wsprintf( szKeyName, "MRU[%02d].szPath", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
		wsprintf( szKeyName, "MRU[%02d].szMark", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
		//お気に入り	//@@@ 2003.04.08 MIK
		wsprintf( szKeyName, "MRU[%02d].bFavorite", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
	if ( cProfile.IsReadingMode() ){
		FileInfo	fiInit;
		//	残りをfiInitで初期化しておく。
		fiInit.m_nCharCode = 0;
		fiInit.m_nViewLeftCol = 0;
		fiInit.m_nViewTopLine = 0;
		fiInit.m_nX = 0;
		fiInit.m_nY = 0;
		strcpy( fiInit.m_szPath, "" );
		strcpy( fiInit.m_szMarkLines, "" );	// 2002.01.16 hor
		for( ; i < MAX_MRU; ++i){
			m_pShareData->m_fiMRUArr[i] = fiInit;
			m_pShareData->m_bMRUArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}

	cProfile.IOProfileData( pszSecName, "_MRUFOLDER_Counts", m_pShareData->m_nOPENFOLDERArrNum );
	nSize = m_pShareData->m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_szOPENFOLDERArr[i],
			sizeof( m_pShareData->m_szOPENFOLDERArr[0] ));
		//お気に入り	//@@@ 2003.04.08 MIK
		strcat( szKeyName, ".bFavorite" );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_bOPENFOLDERArrFavorite[i] );
	}
	//読み込み時は残りを初期化
	if ( cProfile.IsReadingMode() ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			m_pShareData->m_szOPENFOLDERArr[i][0] = '\0';
			m_pShareData->m_bOPENFOLDERArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
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

	cProfile.IOProfileData( pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_nSEARCHKEYArrNum );
	nSize = m_pShareData->m_nSEARCHKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_szSEARCHKEYArr[0] ));
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_SEARCHKEY; ++i){
			m_pShareData->m_szSEARCHKEYArr[i][0] = '\0';
		}
	}

	cProfile.IOProfileData( pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_nREPLACEKEYArrNum );
	nSize = m_pShareData->m_nREPLACEKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_szREPLACEKEYArr[0] ) );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_REPLACEKEY; ++i){
			m_pShareData->m_szREPLACEKEYArr[i][0] = '\0';
		}
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

	cProfile.IOProfileData( pszSecName, "_GREPFILE_Counts", m_pShareData->m_nGREPFILEArrNum );
	nSize = m_pShareData->m_nGREPFILEArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szGREPFILEArr[i], sizeof( m_pShareData->m_szGREPFILEArr[0] ));
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_GREPFILE; ++i){
			m_pShareData->m_szGREPFILEArr[i][0] = '\0';
		}
	}

	cProfile.IOProfileData( pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_nGREPFOLDERArrNum );
	nSize = m_pShareData->m_nGREPFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_szGREPFOLDERArr[0] ));
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_GREPFOLDER; ++i){
			m_pShareData->m_szGREPFOLDERArr[i][0] = '\0';
		}
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
		m_pShareData->m_szMACROFOLDER, sizeof( m_pShareData->m_szMACROFOLDER ));
	/* 設定インポート用フォルダ */
	cProfile.IOProfileData( pszSecName, "szIMPORTFOLDER",
		m_pShareData->m_szIMPORTFOLDER, sizeof( m_pShareData->m_szIMPORTFOLDER ));
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

	cProfile.IOProfileData( pszSecName, "nCmdArrNum", m_pShareData->m_nCmdArrNum );
	int nSize = m_pShareData->m_nCmdArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "szCmdArr[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szCmdArr[i], sizeof( m_pShareData->m_szCmdArr[0] ));
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_CMDARR; ++i){
			m_pShareData->m_szCmdArr[i][0] = '\0';
		}
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

	cProfile.IOProfileData( pszSecName, "ArrNum", m_pShareData->m_nTransformFileNameArrNum );
	int nSize = m_pShareData->m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "From%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_szTransformFileNameFrom[0] ));
		wsprintf( szKeyName, "To%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szTransformFileNameTo[i], sizeof( m_pShareData->m_szTransformFileNameTo[0] ));
	}
	// 読み込み時，残りをNULLで再初期化
	if( cProfile.IsReadingMode() ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			m_pShareData->m_szTransformFileNameFrom[i][0] = '\0';
			m_pShareData->m_szTransformFileNameTo[i][0]   = '\0';
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
	Common& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, "nCaretType"				, common.m_nCaretType );
	//	Oct. 2, 2005 genta
	//	初期値を挿入モードに固定するため，設定の読み書きをやめる
	//cProfile.IOProfileData( pszSecName, "bIsINSMode"				, common.m_bIsINSMode );
	cProfile.IOProfileData( pszSecName, "bIsFreeCursorMode"		, common.m_bIsFreeCursorMode );
	
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchWord"	, common.m_bStopsBothEndsWhenSearchWord );
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchParagraph"	, common.m_bStopsBothEndsWhenSearchParagraph );
	//	Oct. 27, 2000 genta
	cProfile.IOProfileData( pszSecName, "m_bRestoreCurPosition"	, common.m_bRestoreCurPosition );
	// 2002.01.16 hor
	cProfile.IOProfileData( pszSecName, "m_bRestoreBookmarks"	, common.m_bRestoreBookmarks );
	cProfile.IOProfileData( pszSecName, "bAddCRLFWhenCopy"		, common.m_bAddCRLFWhenCopy );
	cProfile.IOProfileData( pszSecName, "nRepeatedScrollLineNum"	, common.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( pszSecName, "nRepeatedScroll_Smooth"	, common.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( pszSecName, "bCloseAllConfirm"		, common.m_bCloseAllConfirm );	/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
	cProfile.IOProfileData( pszSecName, "bExitConfirm"			, common.m_bExitConfirm );
	cProfile.IOProfileData( pszSecName, "bSearchRegularExp"		, common.m_bRegularExp );
	cProfile.IOProfileData( pszSecName, "bSearchLoHiCase"		, common.m_bLoHiCase );
	cProfile.IOProfileData( pszSecName, "bSearchWordOnly"		, common.m_bWordOnly );
	cProfile.IOProfileData( pszSecName, "bSearchConsecutiveAll"		, common.m_bConsecutiveAll );	// 2007.01.16 ryoji
	cProfile.IOProfileData( pszSecName, "bSearchNOTIFYNOTFOUND"	, common.m_bNOTIFYNOTFOUND );
	// 2002.01.26 hor
	cProfile.IOProfileData( pszSecName, "bSearchAll"				, common.m_bSearchAll );
	cProfile.IOProfileData( pszSecName, "bSearchSelectedArea"	, common.m_bSelectedArea );
	cProfile.IOProfileData( pszSecName, "bGrepSubFolder"			, common.m_bGrepSubFolder );
	cProfile.IOProfileData( pszSecName, "bGrepOutputLine"		, common.m_bGrepOutputLine );
	cProfile.IOProfileData( pszSecName, "nGrepOutputStyle"		, common.m_nGrepOutputStyle );
	cProfile.IOProfileData( pszSecName, "bGrepDefaultFolder"		, common.m_bGrepDefaultFolder );
	// 2002/09/21 Moca 追加
	cProfile.IOProfileData( pszSecName, "nGrepCharSet"			, common.m_nGrepCharSet );
	cProfile.IOProfileData( pszSecName, "bGrepRealTime"			, common.m_bGrepRealTimeView ); // 2003.06.16 Moca
	
	cProfile.IOProfileData( pszSecName, "bGTJW_RETURN"			, common.m_bGTJW_RETURN );
	cProfile.IOProfileData( pszSecName, "bGTJW_LDBLCLK"			, common.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( pszSecName, "bBackUp"				, common.m_bBackUp );
	cProfile.IOProfileData( pszSecName, "bBackUpDialog"			, common.m_bBackUpDialog );
	cProfile.IOProfileData( pszSecName, "bBackUpFolder"			, common.m_bBackUpFolder );
	
	if( !cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = strlen( common.m_szBackUpFolder );
		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
		nCharChars = &common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_szBackUpFolder, nDummy, &common.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			strcat( common.m_szBackUpFolder, "\\" );
		}
	}
	cProfile.IOProfileData( pszSecName, "szBackUpFolder",
		common.m_szBackUpFolder, sizeof( common.m_szBackUpFolder ));
	if( cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = strlen( common.m_szBackUpFolder );
		/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
		nCharChars = &common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_szBackUpFolder, nDummy, &common.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			strcat( common.m_szBackUpFolder, "\\" );
		}
	}
	
	
	cProfile.IOProfileData( pszSecName, "nBackUpType"			, common.m_nBackUpType );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt1"		, common.m_nBackUpType_Opt1 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt2"		, common.m_nBackUpType_Opt2 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt3"		, common.m_nBackUpType_Opt3 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt4"		, common.m_nBackUpType_Opt4 );
	cProfile.IOProfileData( pszSecName, "bBackUpDustBox"			, common.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
	cProfile.IOProfileData( pszSecName, "bBackUpPathAdvanced"	, common.m_bBackUpPathAdvanced );	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "szBackUpPathAdvanced"	,
		common.m_szBackUpPathAdvanced, sizeof( common.m_szBackUpPathAdvanced ));	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "nFileShareMode"			, common.m_nFileShareMode );
	cProfile.IOProfileData( pszSecName, "szExtHelp",
		common.m_szExtHelp, sizeof( common.m_szExtHelp ));
	cProfile.IOProfileData( pszSecName, "szExtHtmlHelp",
		common.m_szExtHtmlHelp, sizeof( common.m_szExtHtmlHelp ));
	
	cProfile.IOProfileData( pszSecName, "szMigemoDll",
		common.m_szMigemoDll, sizeof( common.m_szMigemoDll ));
	cProfile.IOProfileData( pszSecName, "szMigemoDict",
		common.m_szMigemoDict, sizeof( common.m_szMigemoDict ));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		const char*	pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		char		szKeyData[1024];
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, "khlf", szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&common.m_lf_kh.lfHeight,
					&common.m_lf_kh.lfWidth,
					&common.m_lf_kh.lfEscapement,
					&common.m_lf_kh.lfOrientation,
					&common.m_lf_kh.lfWeight,
					&common.m_lf_kh.lfItalic,
					&common.m_lf_kh.lfUnderline,
					&common.m_lf_kh.lfStrikeOut,
					&common.m_lf_kh.lfCharSet,
					&common.m_lf_kh.lfOutPrecision,
					&common.m_lf_kh.lfClipPrecision,
					&common.m_lf_kh.lfQuality,
					&common.m_lf_kh.lfPitchAndFamily
				);
			}
		}else{
			wsprintf( szKeyData, pszForm,
				common.m_lf_kh.lfHeight,
				common.m_lf_kh.lfWidth,
				common.m_lf_kh.lfEscapement,
				common.m_lf_kh.lfOrientation,
				common.m_lf_kh.lfWeight,
				common.m_lf_kh.lfItalic,
				common.m_lf_kh.lfUnderline,
				common.m_lf_kh.lfStrikeOut,
				common.m_lf_kh.lfCharSet,
				common.m_lf_kh.lfOutPrecision,
				common.m_lf_kh.lfClipPrecision,
				common.m_lf_kh.lfQuality,
				common.m_lf_kh.lfPitchAndFamily
			);
			cProfile.IOProfileData( pszSecName, "khlf", szKeyData, 0 );
		}
	
		cProfile.IOProfileData( pszSecName, "khlfFaceName"		, common.m_lf_kh.lfFaceName, 0 );
	
	}// Keword Help Font
	// ai 02/05/23 Add S
//	cProfile.IOProfileData( pszSecName, "bClickKeySearch"		, common.m_bUseCaretKeyWord );	// 2006.03.24 fon sakura起動ごとFALSEとし、初期化しない
	
	
	cProfile.IOProfileData( pszSecName, "nMRUArrNum_MAX"			, common.m_nMRUArrNum_MAX );
	cProfile.IOProfileData( pszSecName, "nOPENFOLDERArrNum_MAX"	, common.m_nOPENFOLDERArrNum_MAX );
	cProfile.IOProfileData( pszSecName, "bDispTOOLBAR"			, common.m_bDispTOOLBAR );
	cProfile.IOProfileData( pszSecName, "bDispSTATUSBAR"			, common.m_bDispSTATUSBAR );
	cProfile.IOProfileData( pszSecName, "bDispFUNCKEYWND"		, common.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_Place"		, common.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_GroupNum"	, common.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca ファンクションキーのグループボタン数
	
	cProfile.IOProfileData( pszSecName, "bDispTabWnd"			, common.m_bDispTabWnd );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "bDispTabWndMultiWin"	, common.m_bDispTabWndMultiWin );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "szTabWndCaption"		, common.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK
	cProfile.IOProfileData( pszSecName, "bSameTabWidth"			, common.m_bSameTabWidth );	// 2006.01.28 ryoji タブを等幅にする
	cProfile.IOProfileData( pszSecName, "bDispTabIcon"			, common.m_bDispTabIcon );	// 2006.01.28 ryoji タブにアイコンを表示する
	cProfile.IOProfileData( pszSecName, "bSortTabList"			, common.m_bSortTabList );	// 2006.05.10 ryoji タブ一覧をソートする
	
	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	cProfile.IOProfileData( pszSecName, "bSplitterWndHScroll"	, common.m_bSplitterWndHScroll );
	cProfile.IOProfileData( pszSecName, "bSplitterWndVScroll"	, common.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( pszSecName, "szMidashiKigou"			, common.m_szMidashiKigou, 0 );
	cProfile.IOProfileData( pszSecName, "szInyouKigou"			, common.m_szInyouKigou, 0 );
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	cProfile.IOProfileData( pszSecName, "bUseHokan"				, common.m_bUseHokan );
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	cProfile.IOProfileData( pszSecName, "bSaveWindowSize"		, common.m_nSaveWindowSize );
	cProfile.IOProfileData( pszSecName, "nWinSizeType"			, common.m_nWinSizeType );
	cProfile.IOProfileData( pszSecName, "nWinSizeCX"				, common.m_nWinSizeCX );
	cProfile.IOProfileData( pszSecName, "nWinSizeCY"				, common.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*を追加
	cProfile.IOProfileData( pszSecName, "nSaveWindowPos"			, common.m_nSaveWindowPos );
	cProfile.IOProfileData( pszSecName, "nWinPosX"				, common.m_nWinPosX );
	cProfile.IOProfileData( pszSecName, "nWinPosY"				, common.m_nWinPosY );
	cProfile.IOProfileData( pszSecName, "bTaskTrayUse"			, common.m_bUseTaskTray );
	cProfile.IOProfileData( pszSecName, "bTaskTrayStay"			, common.m_bStayTaskTray );
//@@@ 2002.01.08 YAZAKI タスクトレイを「使わない」にしても常駐がチェックが残っていると再起動で「使う・常駐」になるバグ修正
#if 0
	if( bRead ){
		/* タスクトレイに常駐するときは、必ずタスクトレイアイコンを使う */
		if( common.m_bStayTaskTray ){
			common.m_bUseTaskTray = TRUE;
		}
	}
#endif
	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyCode"		, common.m_wTrayMenuHotKeyCode );
	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyMods"		, common.m_wTrayMenuHotKeyMods );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DragDrop"			, common.m_bUseOLE_DragDrop );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DropSource"			, common.m_bUseOLE_DropSource );
	cProfile.IOProfileData( pszSecName, "bDispExitingDialog"			, common.m_bDispExitingDialog );
	cProfile.IOProfileData( pszSecName, "bEnableUnmodifiedOverwrite"	, common.m_bEnableUnmodifiedOverwrite );
	cProfile.IOProfileData( pszSecName, "bSelectClickedURL"			, common.m_bSelectClickedURL );
	cProfile.IOProfileData( pszSecName, "bGrepExitConfirm"			, common.m_bGrepExitConfirm );/* Grepモードで保存確認するか */
//	cProfile.IOProfileData( pszSecName, "bRulerDisp"					, common.m_bRulerDisp );/* ルーラー表示 */
	cProfile.IOProfileData( pszSecName, "nRulerHeight"				, common.m_nRulerHeight );/* ルーラー高さ */
	cProfile.IOProfileData( pszSecName, "nRulerBottomSpace"			, common.m_nRulerBottomSpace );/* ルーラーとテキストの隙間 */
	cProfile.IOProfileData( pszSecName, "nRulerType"					, common.m_nRulerType );/* ルーラーのタイプ */
	//	Sep. 18, 2002 genta 追加
	cProfile.IOProfileData( pszSecName, "nLineNumberRightSpace"		, common.m_nLineNumRightSpace );/* 行番号の右側の隙間 */
	cProfile.IOProfileData( pszSecName, "nVertLineOffset"			, common.m_nVertLineOffset ); // 2005.11.10 Moca
	cProfile.IOProfileData( pszSecName, "bCopyAndDisablSelection"	, common.m_bCopyAndDisablSelection );/* コピーしたら選択解除 */
	cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"			, common.m_bHtmlHelpIsSingle );/* HtmlHelpビューアはひとつ */
	cProfile.IOProfileData( pszSecName, "bCompareAndTileHorz"		, common.m_bCompareAndTileHorz );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	cProfile.IOProfileData( pszSecName, "bDropFileAndClose"			, common.m_bDropFileAndClose );/* ファイルをドロップしたときは閉じて開く */
	cProfile.IOProfileData( pszSecName, "nDropFileNumMax"			, common.m_nDropFileNumMax );/* 一度にドロップ可能なファイル数 */
	cProfile.IOProfileData( pszSecName, "bCheckFileTimeStamp"		, common.m_bCheckFileTimeStamp );/* 更新の監視 */
	cProfile.IOProfileData( pszSecName, "bNotOverWriteCRLF"			, common.m_bNotOverWriteCRLF );/* 改行は上書きしない */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFind"			, common.m_bAutoCloseDlgFind );/* 検索ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFuncList"		, common.m_bAutoCloseDlgFuncList );/* アウトライン ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgReplace"		, common.m_bAutoCloseDlgReplace );/* 置換 ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, "bAutoColmnPaste"			, common.m_bAutoColmnPaste );/* 矩形コピーのテキストは常に矩形貼り付け */
	cProfile.IOProfileData( pszSecName, "bScrollBarHorz"				, common.m_bScrollBarHorz );/* 水平スクロールバーを使う */
	
	cProfile.IOProfileData( pszSecName, "bHokanKey_RETURN"			, common.m_bHokanKey_RETURN );/* VK_RETURN 補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, "bHokanKey_TAB"				, common.m_bHokanKey_TAB );/* VK_TAB    補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, "bHokanKey_RIGHT"			, common.m_bHokanKey_RIGHT );/* VK_RIGHT  補完決定キーが有効/無効 */
	cProfile.IOProfileData( pszSecName, "bHokanKey_SPACE"			, common.m_bHokanKey_SPACE );/* VK_SPACE  補完決定キーが有効/無効 */
	
	cProfile.IOProfileData( pszSecName, "nDateFormatType"			, common.m_nDateFormatType );/* 日付書式のタイプ */
	cProfile.IOProfileData( pszSecName, "szDateFormat"				, common.m_szDateFormat, 0 );//日付書式
	cProfile.IOProfileData( pszSecName, "nTimeFormatType"			, common.m_nTimeFormatType );/* 時刻書式のタイプ */
	cProfile.IOProfileData( pszSecName, "szTimeFormat"				, common.m_szTimeFormat, 0 );//時刻書式
	
	cProfile.IOProfileData( pszSecName, "bMenuIcon"					, common.m_bMenuIcon );//メニューにアイコンを表示する
	cProfile.IOProfileData( pszSecName, "bAutoMIMEdecode"			, common.m_bAutoMIMEdecode );//ファイル読み込み時にMIMEのdecodeを行うか
	cProfile.IOProfileData( pszSecName, "bQueryIfCodeChange"			, common.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
	cProfile.IOProfileData( pszSecName, "bAlertIfFileNotExist"		, common.m_bAlertIfFileNotExist );// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveNew"			, common.m_bNoFilterSaveNew );	// 新規から保存時は全ファイル表示	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveFile"			, common.m_bNoFilterSaveFile );	// 新規以外から保存時は全ファイル表示	// 2006.11.16 ryoji
	
	/* 「開く」ダイアログのサイズと位置 */
	const char* pszKeyName = "rcOpenDialog";
	const char* pszForm = "%d,%d,%d,%d";
	char		szKeyData[1024];
	if( cProfile.IsReadingMode() ){
		if( true == cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&common.m_rcOpenDialog.left,
				&common.m_rcOpenDialog.top,
				&common.m_rcOpenDialog.right,
				&common.m_rcOpenDialog.bottom
			);
		}
	}else{
		wsprintf( szKeyData, pszForm,
			common.m_rcOpenDialog.left,
			common.m_rcOpenDialog.top,
			common.m_rcOpenDialog.right,
			common.m_rcOpenDialog.bottom
		);
		cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, 0 );
	}
	
	//2002.02.08 aroka,hor
	cProfile.IOProfileData( pszSecName, "bMarkUpBlankLineEnable"	, common.m_bMarkUpBlankLineEnable );
	cProfile.IOProfileData( pszSecName, "bFunclistSetFocusOnJump"	, common.m_bFunclistSetFocusOnJump );
	
	//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	cProfile.IOProfileData( pszSecName, "szWinCaptionActive"
		, common.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
	cProfile.IOProfileData( pszSecName, "szWinCaptionInactive"
		, common.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );
	
	// アウトライン/トピックリスト の位置とサイズを記憶  20060201 aroka
	cProfile.IOProfileData( pszSecName, "bRememberOutlineWindowPos", common.m_bRememberOutlineWindowPos);
	if( common.m_bRememberOutlineWindowPos ){
		cProfile.IOProfileData( pszSecName, "widthOutlineWindow"	, common.m_widthOutlineWindow);
		cProfile.IOProfileData( pszSecName, "heightOutlineWindow", common.m_heightOutlineWindow);
		cProfile.IOProfileData( pszSecName, "xOutlineWindowPos"	, common.m_xOutlineWindowPos);
		cProfile.IOProfileData( pszSecName, "yOutlineWindowPos"	, common.m_yOutlineWindowPos);
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
	Common& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, "nToolBarButtonNum", common.m_nToolBarButtonNum );
	cProfile.IOProfileData( pszSecName, "bToolBarIsFlat", common.m_bToolBarIsFlat );
	int	nSize = common.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "nTBB[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, common.m_nToolBarButtonIdxArr[i] );
	}
	//読み込み時は残りを初期化
	if( cProfile.IsReadingMode() ){
		for(; i< MAX_TOOLBARBUTTONS; ++i){
			common.m_nToolBarButtonIdxArr[i] = 0;
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
	Common& common = m_pShareData->m_Common;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		wsprintf( szKeyName, "szCMN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta 最大長指定
		wsprintf( szKeyName, "nCMIN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, common.m_nCustMenuItemNumArr[i] );
		int nSize = common.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, common.m_nCustMenuItemFuncArr[i][j] );
			wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, common.m_nCustMenuItemKeyArr[i][j] );
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
	const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
	char		szKeyData[1024];
	Common& common = m_pShareData->m_Common;
	if( cProfile.IsReadingMode() ){
		if( true == cProfile.IOProfileData( pszSecName, "lf", szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&common.m_lf.lfHeight,
				&common.m_lf.lfWidth,
				&common.m_lf.lfEscapement,
				&common.m_lf.lfOrientation,
				&common.m_lf.lfWeight,
				&common.m_lf.lfItalic,
				&common.m_lf.lfUnderline,
				&common.m_lf.lfStrikeOut,
				&common.m_lf.lfCharSet,
				&common.m_lf.lfOutPrecision,
				&common.m_lf.lfClipPrecision,
				&common.m_lf.lfQuality,
				&common.m_lf.lfPitchAndFamily
			);
		}
	}else{
		wsprintf( szKeyData, pszForm,
			common.m_lf.lfHeight,
			common.m_lf.lfWidth,
			common.m_lf.lfEscapement,
			common.m_lf.lfOrientation,
			common.m_lf.lfWeight,
			common.m_lf.lfItalic,
			common.m_lf.lfUnderline,
			common.m_lf.lfStrikeOut,
			common.m_lf.lfCharSet,
			common.m_lf.lfOutPrecision,
			common.m_lf.lfClipPrecision,
			common.m_lf.lfQuality,
			common.m_lf.lfPitchAndFamily
		);
		cProfile.IOProfileData( pszSecName, "lf", szKeyData, 0 );
	}
	
	cProfile.IOProfileData( pszSecName, "lfFaceName",
		common.m_lf.lfFaceName, sizeof( common.m_lf.lfFaceName ));
	
	cProfile.IOProfileData( pszSecName, "bFontIs_FIXED_PITCH", common.m_bFontIs_FIXED_PITCH );
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
	int		nSize = m_pShareData->m_nKeyNameArrNum;
	for( i = 0; i < nSize; ++i ){
		// 2005.04.07 D.S.Koba
		KEYDATA& keydata = m_pShareData->m_pKeyNameArr[i];
		strcpy( szKeyName, keydata.m_szKeyName );
		
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
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
			if( true == cProfile.IOProfileData( pszSecName, szKeyName,
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
	@brief 共有データのTypesセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Types( CProfile& cProfile )
{
	const char* pszSecName;
	int		i, j;
	char	szKey[256];
	char	szKeyName[64];
	char	szKeyData[1024];

	for( i = 0; i < MAX_TYPES; ++i ){
		// 2005.04.07 D.S.Koba
		Types& types = m_pShareData->m_Types[i];
		wsprintf( szKey, "Types(%d)", i );
		pszSecName = szKey;
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&types.m_nIdx,
					&types.m_nMaxLineSize,
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
			if( types.m_nMaxLineSize < MINLINESIZE ){
				types.m_nMaxLineSize = MINLINESIZE;
			}
		}else{
			wsprintf( szKeyData, pszForm,
				types.m_nIdx,
				types.m_nMaxLineSize,
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
		if( !cProfile.IsReadingMode() ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > LINESPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				types.m_nLineSpace = LINESPACE_MAX;
			}
		}
		cProfile.IOProfileData( pszSecName, "nLineSpace", types.m_nLineSpace );
		if( cProfile.IsReadingMode() ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > 16 ){
				types.m_nLineSpace = 16;
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
			if( bRet1 && bRet2 ) types.m_cBlockComment.CopyTo( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) types.m_cBlockComment.CopyTo( 1, buffer[0], buffer[1] );
			
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
				const_cast<char*>(types.m_cBlockComment.getBlockCommentFrom( 0 )), 0 );
			cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentTo( 0 )), 0 );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
				const_cast<char*>(types.m_cBlockComment.getBlockCommentFrom( 1 )), 0 );
			cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentTo(1)), 0 );

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
		cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"	, types.m_bHokanLoHiCase );

		cProfile.IOProfileData( pszSecName, "bAutoIndent"			, types.m_bAutoIndent );
		cProfile.IOProfileData( pszSecName, "bAutoIndent_ZENSPACE"	, types.m_bAutoIndent_ZENSPACE );
		cProfile.IOProfileData( pszSecName, "bRTrimPrevLine"			, types.m_bRTrimPrevLine );			// 2005.10.08 ryoji
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
					if( true == cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
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
				else
				// 2002.02.08 hor 未定義値を無視
				if(lstrlen(types.m_RegexKeywordArr[j].m_szKeyword))
				{
					//wsprintf( szKeyData, "%d,%s",
					//	types.m_RegexKeywordArr[j].m_nColorIndex,
					//	types.m_RegexKeywordArr[j].m_szKeyword);
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
		cProfile.IOProfileData( pszSecName, "bUseDocumentIcon"	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta 変数名誤り修正

//@@@ 2006.04.10 fon ADD-start
		{	/* キーワード辞書 */
			static const char* pszForm = "%d,%s,%s";
			char	*pH, *pT;	/* <pH>keyword<pT> */
			cProfile.IOProfileData( pszSecName, "bUseKeyWordHelp", types.m_bUseKeyWordHelp );	/* キーワード辞書選択を使用するか？ */
//			cProfile.IOProfileData( pszSecName, "nKeyHelpNum", types.m_nKeyHelpNum );				/* 登録辞書数 */
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
					if( true == cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
						pH = szKeyData;
						if( NULL != (pT=strchr(pH, ',')) ){
							*pT = '\0';
							types.m_KeyHelpArr[j].m_nUse = atoi( pH );
							pH = pT+1;
							if( NULL != (pT=strchr(pH, ',')) ){
								*pT = '\0';
								strcpy( types.m_KeyHelpArr[j].m_szAbout, pH );
								pH = pT+1;
								if( NULL != (*pH) ){
									strcpy( types.m_KeyHelpArr[j].m_szPath, pH );
									types.m_nKeyHelpNum = j+1;	// iniに保存せずに、読み出せたファイル分を辞書数とする
								}
							}
						}
					}
				}/* 書き込み */
				else{
					if(lstrlen(types.m_KeyHelpArr[j].m_szPath)){
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

	}/* for */
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
	CKeyWordSetMgr*	pCKeyWordSetMgr = &m_pShareData->m_CKeyWordSetMgr;
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
				int nKEYWORDCASE = 0;
				int nKeyWordNum = 0;
				wsprintf( szKeyName, "szSN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKEYWORDCASE );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKeyWordNum );
				pCKeyWordSetMgr->AddKeyWordSet( szKeyData, nKEYWORDCASE, nKeyWordNum );
				// 2004.11.25 Moca szKW[%02d].Size の情報は利用する意味がない。
//				wsprintf( szKeyName, "szKW[%02d].Size", i );
//				cProfile.IOProfileData( pszSecName, szKeyName, nMemLen );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( true == cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen ) ){
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
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_nKEYWORDCASEArr[i] );
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
		MacroRec& macrorec = m_pShareData->m_MacroTable[i];
		//	Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if( !cProfile.IsReadingMode() && !lstrlen(macrorec.m_szName) && !lstrlen(macrorec.m_szFile) ) continue;
		wsprintf( szKeyName, "Name[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szName, MACRONAME_MAX - 1 );
		wsprintf( szKeyName, "File[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szFile, _MAX_PATH );
		wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
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
	cProfile.IOProfileData( pszSecName, "bGetStdout"		, m_pShareData->m_bGetStdout );
	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
	cProfile.IOProfileData( pszSecName, "bLineNumIsCRLF"	, m_pShareData->m_bLineNumIsCRLF );
	
	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( pszSecName, "nDiffFlgOpt"	, m_pShareData->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( pszSecName, "nTagsOpt"		, m_pShareData->m_nTagsOpt );
	cProfile.IOProfileData( pszSecName, "szTagsCmdLine"	, m_pShareData->m_szTagsCmdLine, sizeof( m_pShareData->m_szTagsCmdLine ) );
	
	//From Here 2005.04.03 MIK キーワード指定タグジャンプ
	cProfile.IOProfileData( pszSecName, "_TagJumpKeyword_Counts", m_pShareData->m_nTagJumpKeywordArrNum );
	for( i = 0; i < m_pShareData->m_nTagJumpKeywordArrNum; ++i ){
		wsprintf( szKeyName, "TagJumpKeyword[%02d]", i );
		if( i >= m_pShareData->m_nTagJumpKeywordArrNum ){
			strcpy( m_pShareData->m_szTagJumpKeywordArr[i], "" );
		}
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szTagJumpKeywordArr[i], sizeof( m_pShareData->m_szTagJumpKeywordArr[0] ));
	}
	cProfile.IOProfileData( pszSecName, "m_bTagJumpICase"		, m_pShareData->m_bTagJumpICase );
	cProfile.IOProfileData( pszSecName, "m_bTagJumpAnyWhere"		, m_pShareData->m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK キーワード指定タグジャンプの

	//	MIK バージョン情報（書き込みのみ）
	if( ! cProfile.IsReadingMode() ){
		TCHAR	iniVer[256];
		wsprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( m_pShareData->m_dwProductVersionMS ),
					LOWORD( m_pShareData->m_dwProductVersionMS ),
					HIWORD( m_pShareData->m_dwProductVersionLS ),
					LOWORD( m_pShareData->m_dwProductVersionLS ) );
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
#ifndef STR_COLORDATA_HEAD3
		wsprintf( szKeyName, "CI[%02d]", j );
#else
		wsprintf( szKeyName, "C[%s]", g_ColorAttributeArr[j].szName );	//Stonee, 2001/01/12, 2001/01/15
#endif
		if( pcProfile->IsReadingMode() ){
			if( true == pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				pColorInfoArr[j].m_bUnderLine = FALSE;
				sscanf( szKeyData, pszForm,
					&pColorInfoArr[j].m_bDisp,
					&pColorInfoArr[j].m_bFatFont,
					&pColorInfoArr[j].m_colTEXT,
					&pColorInfoArr[j].m_colBACK,
					&pColorInfoArr[j].m_bUnderLine
				 );
			}else{
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
		}else{
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


/*[EOF]*/
