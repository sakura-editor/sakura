//	$Id$
/*!	@file
	@brief プロセス間共有データのファイルへ／からの入出力

	@author Norio Nakatani
	$Revision$

	@date 2005.04.07 D.S.Koba 各セクションの入出力を関数として分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee, jepro, mik, asa-o, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

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
	
	ShareData_IO_Mru( bRead, cProfile );
	ShareData_IO_Keys( bRead, cProfile );
	ShareData_IO_Grep( bRead, cProfile );
	ShareData_IO_Folders( bRead, cProfile );
	ShareData_IO_Cmd( bRead, cProfile );
	ShareData_IO_Nickname( bRead, cProfile );
	ShareData_IO_Common( bRead, cProfile );
	ShareData_IO_Toolbar( bRead, cProfile );
	ShareData_IO_CustMenu( bRead, cProfile );
	ShareData_IO_Font( bRead, cProfile );
	ShareData_IO_KeyBind( bRead, cProfile );
	ShareData_IO_Print( bRead, cProfile );
	ShareData_IO_Types( bRead, cProfile );
	ShareData_IO_KeyWords( bRead, cProfile );
	ShareData_IO_Macro( bRead, cProfile );
	ShareData_IO_Other( bRead, cProfile );
	
	if( !bRead ){
		cProfile.WriteProfile( szIniFileName, " sakura.ini テキストエディタ設定ファイル" );
	}

//	MYTRACE( "Iniファイル処理 8 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	MYTRACE( "Iniファイル処理 所要時間(ミリ秒) = %d\n", cRunningTimerStart.Read() );

	return TRUE;
}

/*!
	@brief 共有データのMruセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Mru( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "MRU";
	int			i;
	int			nSize;
	FileInfo*	pfiWork;
	char		szKeyName[64];

	cProfile.IOProfileData( bRead, pszSecName, "_MRU_Counts", m_pShareData->m_nMRUArrNum );
	nSize = m_pShareData->m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &m_pShareData->m_fiMRUArr[i];
		wsprintf( szKeyName, "MRU[%02d].nViewTopLine", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nViewTopLine );
		wsprintf( szKeyName, "MRU[%02d].nViewLeftCol", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
		wsprintf( szKeyName, "MRU[%02d].nX", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nX );
		wsprintf( szKeyName, "MRU[%02d].nY", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nY );
		wsprintf( szKeyName, "MRU[%02d].nCharCode", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nCharCode );
		wsprintf( szKeyName, "MRU[%02d].szPath", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
		wsprintf( szKeyName, "MRU[%02d].szMark", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
		//お気に入り	//@@@ 2003.04.08 MIK
		wsprintf( szKeyName, "MRU[%02d].bFavorite", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
	if ( bRead ){
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

	cProfile.IOProfileData( bRead, pszSecName, "_MRUFOLDER_Counts", m_pShareData->m_nOPENFOLDERArrNum );
	nSize = m_pShareData->m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_szOPENFOLDERArr[i],
			sizeof( m_pShareData->m_szOPENFOLDERArr[0] ));
		//お気に入り	//@@@ 2003.04.08 MIK
		strcat( szKeyName, ".bFavorite" );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_bOPENFOLDERArrFavorite[i] );
	}
	//読み込み時は残りを初期化
	if ( bRead ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			m_pShareData->m_szOPENFOLDERArr[i][0] = '\0';
			m_pShareData->m_bOPENFOLDERArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}
}

/*!
	@brief 共有データのKeysセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Keys( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Keys";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( bRead, pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_nSEARCHKEYArrNum );
	nSize = m_pShareData->m_nSEARCHKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_szSEARCHKEYArr[0] ));
	}
	//読み込み時は残りを初期化
	if( bRead ){
		for(; i < MAX_SEARCHKEY; ++i){
			m_pShareData->m_szSEARCHKEYArr[i][0] = '\0';
		}
	}

	cProfile.IOProfileData( bRead, pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_nREPLACEKEYArrNum );
	nSize = m_pShareData->m_nREPLACEKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_szREPLACEKEYArr[0] ) );
	}
	//読み込み時は残りを初期化
	if( bRead ){
		for(; i < MAX_REPLACEKEY; ++i){
			m_pShareData->m_szREPLACEKEYArr[i][0] = '\0';
		}
	}
}

/*!
	@brief 共有データのGrepセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Grep( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Grep";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( bRead, pszSecName, "_GREPFILE_Counts", m_pShareData->m_nGREPFILEArrNum );
	nSize = m_pShareData->m_nGREPFILEArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szGREPFILEArr[i], sizeof( m_pShareData->m_szGREPFILEArr[0] ));
	}
	//読み込み時は残りを初期化
	if( bRead ){
		for(; i < MAX_GREPFILE; ++i){
			m_pShareData->m_szGREPFILEArr[i][0] = '\0';
		}
	}

	cProfile.IOProfileData( bRead, pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_nGREPFOLDERArrNum );
	nSize = m_pShareData->m_nGREPFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_szGREPFOLDERArr[0] ));
	}
	//読み込み時は残りを初期化
	if( bRead ){
		for(; i < MAX_GREPFOLDER; ++i){
			m_pShareData->m_szGREPFOLDERArr[i][0] = '\0';
		}
	}
}

/*!
	@brief 共有データのFoldersセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Folders( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Folders";
	/* マクロ用フォルダ */
	cProfile.IOProfileData( bRead, pszSecName, "szMACROFOLDER",
		m_pShareData->m_szMACROFOLDER, sizeof( m_pShareData->m_szMACROFOLDER ));
	/* 設定インポート用フォルダ */
	cProfile.IOProfileData( bRead, pszSecName, "szIMPORTFOLDER",
		m_pShareData->m_szIMPORTFOLDER, sizeof( m_pShareData->m_szIMPORTFOLDER ));
}

/*!
	@brief 共有データのCmdセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Cmd( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Cmd";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( bRead, pszSecName, "nCmdArrNum", m_pShareData->m_nCmdArrNum );
	int nSize = m_pShareData->m_nCmdArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "szCmdArr[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szCmdArr[i], sizeof( m_pShareData->m_szCmdArr[0] ));
	}
	//読み込み時は残りを初期化
	if( bRead ){
		for(; i < MAX_CMDARR; ++i){
			m_pShareData->m_szCmdArr[i][0] = '\0';
		}
	}
}

/*!
	@brief 共有データのNicknameセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Nickname( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Nickname";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( bRead, pszSecName, "ArrNum", m_pShareData->m_nTransformFileNameArrNum );
	int nSize = m_pShareData->m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "From%02d", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_szTransformFileNameFrom[0] ));
		wsprintf( szKeyName, "To%02d", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szTransformFileNameTo[i], sizeof( m_pShareData->m_szTransformFileNameTo[0] ));
	}
	// 読み込み時，残りをNULLで再初期化
	if( bRead ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			m_pShareData->m_szTransformFileNameFrom[i][0] = '\0';
			m_pShareData->m_szTransformFileNameTo[i][0]   = '\0';
		}
	}
}

/*!
	@brief 共有データのCommonセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Common( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Common";
	// 2005.04.07 D.S.Koba
	Common& common = m_pShareData->m_Common;

	cProfile.IOProfileData( bRead, pszSecName, "nCaretType"				, common.m_nCaretType );
	cProfile.IOProfileData( bRead, pszSecName, "bIsINSMode"				, common.m_bIsINSMode );
	cProfile.IOProfileData( bRead, pszSecName, "bIsFreeCursorMode"		, common.m_bIsFreeCursorMode );
	
	cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchWord"	, common.m_bStopsBothEndsWhenSearchWord );
	cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchParagraph"	, common.m_bStopsBothEndsWhenSearchParagraph );
	//	Oct. 27, 2000 genta
	cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreCurPosition"	, common.m_bRestoreCurPosition );
	// 2002.01.16 hor
	cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreBookmarks"	, common.m_bRestoreBookmarks );
	cProfile.IOProfileData( bRead, pszSecName, "bAddCRLFWhenCopy"		, common.m_bAddCRLFWhenCopy );
	cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScrollLineNum"	, common.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScroll_Smooth"	, common.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( bRead, pszSecName, "bExitConfirm"			, common.m_bExitConfirm );
	cProfile.IOProfileData( bRead, pszSecName, "bSearchRegularExp"		, common.m_bRegularExp );
	cProfile.IOProfileData( bRead, pszSecName, "bSearchLoHiCase"		, common.m_bLoHiCase );
	cProfile.IOProfileData( bRead, pszSecName, "bSearchWordOnly"		, common.m_bWordOnly );
	cProfile.IOProfileData( bRead, pszSecName, "bSearchNOTIFYNOTFOUND"	, common.m_bNOTIFYNOTFOUND );
	// 2002.01.26 hor
	cProfile.IOProfileData( bRead, pszSecName, "bSearchAll"				, common.m_bSearchAll );
	cProfile.IOProfileData( bRead, pszSecName, "bSearchSelectedArea"	, common.m_bSelectedArea );
	cProfile.IOProfileData( bRead, pszSecName, "bGrepSubFolder"			, common.m_bGrepSubFolder );
	cProfile.IOProfileData( bRead, pszSecName, "bGrepOutputLine"		, common.m_bGrepOutputLine );
	cProfile.IOProfileData( bRead, pszSecName, "nGrepOutputStyle"		, common.m_nGrepOutputStyle );
	cProfile.IOProfileData( bRead, pszSecName, "bGrepDefaultFolder"		, common.m_bGrepDefaultFolder );
	// 2002/09/21 Moca 追加
	cProfile.IOProfileData( bRead, pszSecName, "nGrepCharSet"			, common.m_nGrepCharSet );
	cProfile.IOProfileData( bRead, pszSecName, "bGrepRealTime"			, common.m_bGrepRealTimeView ); // 2003.06.16 Moca
	
	cProfile.IOProfileData( bRead, pszSecName, "bGTJW_RETURN"			, common.m_bGTJW_RETURN );
	cProfile.IOProfileData( bRead, pszSecName, "bGTJW_LDBLCLK"			, common.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUp"				, common.m_bBackUp );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUpDialog"			, common.m_bBackUpDialog );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUpFolder"			, common.m_bBackUpFolder );
	
	if( !bRead ){
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
	cProfile.IOProfileData( bRead, pszSecName, "szBackUpFolder",
		common.m_szBackUpFolder, sizeof( common.m_szBackUpFolder ));
	if( bRead ){
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
	
	
	cProfile.IOProfileData( bRead, pszSecName, "nBackUpType"			, common.m_nBackUpType );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt1"		, common.m_nBackUpType_Opt1 );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt2"		, common.m_nBackUpType_Opt2 );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt3"		, common.m_nBackUpType_Opt3 );
	cProfile.IOProfileData( bRead, pszSecName, "bBackUpDustBox"			, common.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
	cProfile.IOProfileData( bRead, pszSecName, "nFileShareMode"			, common.m_nFileShareMode );
	cProfile.IOProfileData( bRead, pszSecName, "szExtHelp",
		common.m_szExtHelp, sizeof( common.m_szExtHelp ));
	cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp",
		common.m_szExtHtmlHelp, sizeof( common.m_szExtHtmlHelp ));
	
	cProfile.IOProfileData( bRead, pszSecName, "szMigemoDll",
		common.m_szMigemoDll, sizeof( common.m_szMigemoDll ));
	cProfile.IOProfileData( bRead, pszSecName, "szMigemoDict",
		common.m_szMigemoDict, sizeof( common.m_szMigemoDict ));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		const char*	pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		char		szKeyData[1024];
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, "khlf", szKeyData, sizeof( szKeyData )) ){
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
			cProfile.IOProfileData( bRead, pszSecName, "khlf", szKeyData, 0 );
		}
	
		cProfile.IOProfileData( bRead, pszSecName, "khlfFaceName"		, common.m_lf_kh.lfFaceName, 0 );
	
	}// Keword Help Font
	// ai 02/05/23 Add S
	
	
	cProfile.IOProfileData( bRead, pszSecName, "nMRUArrNum_MAX"			, common.m_nMRUArrNum_MAX );
	cProfile.IOProfileData( bRead, pszSecName, "nOPENFOLDERArrNum_MAX"	, common.m_nOPENFOLDERArrNum_MAX );
	cProfile.IOProfileData( bRead, pszSecName, "bDispTOOLBAR"			, common.m_bDispTOOLBAR );
	cProfile.IOProfileData( bRead, pszSecName, "bDispSTATUSBAR"			, common.m_bDispSTATUSBAR );
	cProfile.IOProfileData( bRead, pszSecName, "bDispFUNCKEYWND"		, common.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_Place"		, common.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_GroupNum"	, common.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca ファンクションキーのグループボタン数
	
	cProfile.IOProfileData( bRead, pszSecName, "bDispTabWnd"			, common.m_bDispTabWnd );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( bRead, pszSecName, "bDispTabWndMultiWin"	, common.m_bDispTabWndMultiWin );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( bRead, pszSecName, "szTabWndCaption"		, common.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK
	
	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndHScroll"	, common.m_bSplitterWndHScroll );
	cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndVScroll"	, common.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( bRead, pszSecName, "szMidashiKigou"			, common.m_szMidashiKigou, 0 );
	cProfile.IOProfileData( bRead, pszSecName, "szInyouKigou"			, common.m_szInyouKigou, 0 );
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	cProfile.IOProfileData( bRead, pszSecName, "bUseHokan"				, common.m_bUseHokan );
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	cProfile.IOProfileData( bRead, pszSecName, "bSaveWindowSize"		, common.m_nSaveWindowSize );
	cProfile.IOProfileData( bRead, pszSecName, "nWinSizeType"			, common.m_nWinSizeType );
	cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCX"				, common.m_nWinSizeCX );
	cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCY"				, common.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*を追加
	cProfile.IOProfileData( bRead, pszSecName, "nSaveWindowPos"			, common.m_nSaveWindowPos );
	cProfile.IOProfileData( bRead, pszSecName, "nWinPosX"				, common.m_nWinPosX );
	cProfile.IOProfileData( bRead, pszSecName, "nWinPosY"				, common.m_nWinPosY );
	cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayUse"			, common.m_bUseTaskTray );
	cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayStay"			, common.m_bStayTaskTray );
//@@@ 2002.01.08 YAZAKI タスクトレイを「使わない」にしても常駐がチェックが残っていると再起動で「使う・常駐」になるバグ修正
#if 0
	if( bRead ){
		/* タスクトレイに常駐するときは、必ずタスクトレイアイコンを使う */
		if( common.m_bStayTaskTray ){
			common.m_bUseTaskTray = TRUE;
		}
	}
#endif
	cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyCode"		, common.m_wTrayMenuHotKeyCode );
	cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyMods"		, common.m_wTrayMenuHotKeyMods );
	cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DragDrop"			, common.m_bUseOLE_DragDrop );
	cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DropSource"			, common.m_bUseOLE_DropSource );
	cProfile.IOProfileData( bRead, pszSecName, "bDispExitingDialog"			, common.m_bDispExitingDialog );
	cProfile.IOProfileData( bRead, pszSecName, "bEnableUnmodifiedOverwrite"	, common.m_bEnableUnmodifiedOverwrite );
	cProfile.IOProfileData( bRead, pszSecName, "bSelectClickedURL"			, common.m_bSelectClickedURL );
	cProfile.IOProfileData( bRead, pszSecName, "bGrepExitConfirm"			, common.m_bGrepExitConfirm );/* Grepモードで保存確認するか */
//	cProfile.IOProfileData( bRead, pszSecName, "bRulerDisp"					, common.m_bRulerDisp );/* ルーラー表示 */
	cProfile.IOProfileData( bRead, pszSecName, "nRulerHeight"				, common.m_nRulerHeight );/* ルーラー高さ */
	cProfile.IOProfileData( bRead, pszSecName, "nRulerBottomSpace"			, common.m_nRulerBottomSpace );/* ルーラーとテキストの隙間 */
	cProfile.IOProfileData( bRead, pszSecName, "nRulerType"					, common.m_nRulerType );/* ルーラーのタイプ */
	//	Sep. 18, 2002 genta 追加
	cProfile.IOProfileData( bRead, pszSecName, "nLineNumberRightSpace"		, common.m_nLineNumRightSpace );/* 行番号の右側の隙間 */
	cProfile.IOProfileData( bRead, pszSecName, "bCopyAndDisablSelection"	, common.m_bCopyAndDisablSelection );/* コピーしたら選択解除 */
	cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"			, common.m_bHtmlHelpIsSingle );/* HtmlHelpビューアはひとつ */
	cProfile.IOProfileData( bRead, pszSecName, "bCompareAndTileHorz"		, common.m_bCompareAndTileHorz );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	cProfile.IOProfileData( bRead, pszSecName, "bDropFileAndClose"			, common.m_bDropFileAndClose );/* ファイルをドロップしたときは閉じて開く */
	cProfile.IOProfileData( bRead, pszSecName, "nDropFileNumMax"			, common.m_nDropFileNumMax );/* 一度にドロップ可能なファイル数 */
	cProfile.IOProfileData( bRead, pszSecName, "bCheckFileTimeStamp"		, common.m_bCheckFileTimeStamp );/* 更新の監視 */
	cProfile.IOProfileData( bRead, pszSecName, "bNotOverWriteCRLF"			, common.m_bNotOverWriteCRLF );/* 改行は上書きしない */
	cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFind"			, common.m_bAutoCloseDlgFind );/* 検索ダイアログを自動的に閉じる */
	cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFuncList"		, common.m_bAutoCloseDlgFuncList );/* アウトライン ダイアログを自動的に閉じる */
	cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgReplace"		, common.m_bAutoCloseDlgReplace );/* 置換 ダイアログを自動的に閉じる */
	cProfile.IOProfileData( bRead, pszSecName, "bAutoColmnPaste"			, common.m_bAutoColmnPaste );/* 矩形コピーのテキストは常に矩形貼り付け */
	cProfile.IOProfileData( bRead, pszSecName, "bScrollBarHorz"				, common.m_bScrollBarHorz );/* 水平スクロールバーを使う */
	
	cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RETURN"			, common.m_bHokanKey_RETURN );/* VK_RETURN 補完決定キーが有効/無効 */
	cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_TAB"				, common.m_bHokanKey_TAB );/* VK_TAB    補完決定キーが有効/無効 */
	cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RIGHT"			, common.m_bHokanKey_RIGHT );/* VK_RIGHT  補完決定キーが有効/無効 */
	cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_SPACE"			, common.m_bHokanKey_SPACE );/* VK_SPACE  補完決定キーが有効/無効 */
	
	cProfile.IOProfileData( bRead, pszSecName, "nDateFormatType"			, common.m_nDateFormatType );/* 日付書式のタイプ */
	cProfile.IOProfileData( bRead, pszSecName, "szDateFormat"				, common.m_szDateFormat, 0 );//日付書式
	cProfile.IOProfileData( bRead, pszSecName, "nTimeFormatType"			, common.m_nTimeFormatType );/* 時刻書式のタイプ */
	cProfile.IOProfileData( bRead, pszSecName, "szTimeFormat"				, common.m_szTimeFormat, 0 );//時刻書式
	
	cProfile.IOProfileData( bRead, pszSecName, "bMenuIcon"					, common.m_bMenuIcon );//メニューにアイコンを表示する
	cProfile.IOProfileData( bRead, pszSecName, "bAutoMIMEdecode"			, common.m_bAutoMIMEdecode );//ファイル読み込み時にMIMEのdecodeを行うか
	cProfile.IOProfileData( bRead, pszSecName, "bQueryIfCodeChange"			, common.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
	cProfile.IOProfileData( bRead, pszSecName, "bAlertIfFileNotExist"		, common.m_bAlertIfFileNotExist );// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	
	
	/* 「開く」ダイアログのサイズと位置 */
	const char* pszKeyName = "rcOpenDialog";
	const char* pszForm = "%d,%d,%d,%d";
	char		szKeyData[1024];
	if( bRead ){
		if( true == cProfile.IOProfileData( bRead, pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
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
		cProfile.IOProfileData( bRead, pszSecName, pszKeyName, szKeyData, 0 );
	}
	
	//2002.02.08 aroka,hor
	cProfile.IOProfileData( bRead, pszSecName, "bMarkUpBlankLineEnable"		, common.m_bMarkUpBlankLineEnable );
	cProfile.IOProfileData( bRead, pszSecName, "bFunclistSetFocusOnJump"	, common.m_bFunclistSetFocusOnJump );
	
	//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	cProfile.IOProfileData( bRead, pszSecName, "szWinCaptionActive"
		, common.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
	cProfile.IOProfileData( bRead, pszSecName, "szWinCaptionInactive"
		, common.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );
}

/*!
	@brief 共有データのToolbarセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData::ShareData_IO_Toolbar( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Toolbar";
	int		i;
	char	szKeyName[64];
	Common& common = m_pShareData->m_Common;

	cProfile.IOProfileData( bRead, pszSecName, "nToolBarButtonNum", common.m_nToolBarButtonNum );
	cProfile.IOProfileData( bRead, pszSecName, "bToolBarIsFlat", common.m_bToolBarIsFlat );
	int	nSize = common.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "nTBB[%03d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, common.m_nToolBarButtonIdxArr[i] );
	}
	//読み込み時は残りを初期化
	if( bRead ){
		for(; i< MAX_TOOLBARBUTTONS; ++i){
			common.m_nToolBarButtonIdxArr[i] = 0;
		}
	}
}

/*!
	@brief 共有データのCustMenuセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_CustMenu( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "CustMenu";
	int		i, j;
	char	szKeyName[64];
	Common& common = m_pShareData->m_Common;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		wsprintf( szKeyName, "szCMN[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta 最大長指定
		wsprintf( szKeyName, "nCMIN[%02d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, common.m_nCustMenuItemNumArr[i] );
		int nSize = common.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, common.m_nCustMenuItemFuncArr[i][j] );
			wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, common.m_nCustMenuItemKeyArr[i][j] );
		}
	}
}

/*!
	@brief 共有データのFontセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Font( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Font";
	const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
	char		szKeyData[1024];
	Common& common = m_pShareData->m_Common;
	if( bRead ){
		if( true == cProfile.IOProfileData( bRead, pszSecName, "lf", szKeyData, sizeof( szKeyData )) ){
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
		cProfile.IOProfileData( bRead, pszSecName, "lf", szKeyData, 0 );
	}
	
	cProfile.IOProfileData( bRead, pszSecName, "lfFaceName",
		common.m_lf.lfFaceName, sizeof( common.m_lf.lfFaceName ));
	
	cProfile.IOProfileData( bRead, pszSecName, "bFontIs_FIXED_PITCH", common.m_bFontIs_FIXED_PITCH );
}

/*!
	@brief 共有データのKeyBindセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_KeyBind( const bool bRead, CProfile& cProfile )
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
		
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName,
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
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
		}
	}
}

/*!
	@brief 共有データのPrintセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Print( const bool bRead, CProfile& cProfile )
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
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName,
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
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
		}
		
		wsprintf( szKeyName, "PS[%02d].szSName"	, i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			printsetting.m_szPrintSettingName	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
		wsprintf( szKeyName, "PS[%02d].szFF"	, i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceHan	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
		wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceZen	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
		for( j = 0; j < 3; ++j ){
			wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				printsetting.m_szHeaderForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
			wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				printsetting.m_szFooterForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
		}
		wsprintf( szKeyName, "PS[%02d].szDriver", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDriverName,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
		wsprintf( szKeyName, "PS[%02d].szDevice", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDeviceName, 
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
		wsprintf( szKeyName, "PS[%02d].szOutput", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
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
		wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, printsetting.m_bPrintKinsokuHead );
		wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, printsetting.m_bPrintKinsokuTail );
		wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, printsetting.m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, printsetting.m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK
	}
}

/*!
	@brief 共有データのTypesセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Types( const bool bRead, CProfile& cProfile )
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
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
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
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
		}
		// 2005.01.13 MIK Keywordset 3-10
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect3",  types.m_nKeyWordSetIdx[2] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect4",  types.m_nKeyWordSetIdx[3] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect5",  types.m_nKeyWordSetIdx[4] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect6",  types.m_nKeyWordSetIdx[5] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect7",  types.m_nKeyWordSetIdx[6] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect8",  types.m_nKeyWordSetIdx[7] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect9",  types.m_nKeyWordSetIdx[8] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect10", types.m_nKeyWordSetIdx[9] );

		/* 行間のすきま */
		if( !bRead ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > LINESPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				types.m_nLineSpace = LINESPACE_MAX;
			}
		}
		cProfile.IOProfileData( bRead, pszSecName, "nLineSpace", types.m_nLineSpace );
		if( bRead ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > 16 ){
				types.m_nLineSpace = 16;
			}
		}


		cProfile.IOProfileData( bRead, pszSecName, "szTypeName",
			types.m_szTypeName,
			sizeof( m_pShareData->m_Types[0].m_szTypeName ));
		cProfile.IOProfileData( bRead, pszSecName, "szTypeExts",
			types.m_szTypeExts,
			sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		cProfile.IOProfileData( bRead, pszSecName, "szTabViewString",
		/*&*/types.m_szTabViewString,
		sizeof( types.m_szTabViewString ));
//#endif
		cProfile.IOProfileData( bRead, pszSecName, "bTabArrow"			, types.m_bTabArrow );	//@@@ 2003.03.26 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bInsSpace"			, types.m_bInsSpace );	// 2001.12.03 hor

		// From Here Sep. 28, 2002 genta / YAZAKI
		if( bRead ){
			//	Block Comment
			char buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
			//	2004.10.02 Moca 対になるコメント設定がともに読み込まれたときだけ有効な設定と見なす．
			//	ブロックコメントの始まりと終わり．行コメントの記号と桁位置
			bool bRet1, bRet2;
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	,
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );			
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) types.m_cBlockComment.CopyTo( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2",
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) types.m_cBlockComment.CopyTo( 1, buffer[0], buffer[1] );
			
			//	Line Comment
			char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
			int  pos;

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, pos );
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 0, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2", pos );
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 1, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO 追加
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO 追加
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 2, lbuf, pos );
		}
		else { // write
			//	Block Comment
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentFrom( 0 )), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentTo( 0 )), 0 );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2",
				const_cast<char*>(types.m_cBlockComment.getBlockCommentFrom( 1 )), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentTo(1)), 0 );

			//	Line Comment
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		,
			const_cast<char*>(types.m_cLineComment.getLineComment( 0 )), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		,
			const_cast<char*>(types.m_cLineComment.getLineComment( 1 )), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		,
			const_cast<char*>(types.m_cLineComment.getLineComment( 2 )), 0 );	//Jun. 01, 2001 JEPRO 追加

		//	From here May 12, 2001 genta
		int pos;
		pos = types.m_cLineComment.getLineCommentPos( 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, pos );
		pos = types.m_cLineComment.getLineCommentPos( 1 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2", pos );
		pos = types.m_cLineComment.getLineCommentPos( 2 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO 追加
		//	To here May 12, 2001 genta

		}
		// To Here Sep. 28, 2002 genta / YAZAKI

		cProfile.IOProfileData( bRead, pszSecName, "szIndentChars"		,
			types.m_szIndentChars,
			sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
		cProfile.IOProfileData( bRead, pszSecName, "cLineTermChar"		, types.m_cLineTermChar );

		cProfile.IOProfileData( bRead, pszSecName, "nDefaultOutline"	, types.m_nDefaultOutline );/* アウトライン解析方法 */
		cProfile.IOProfileData( bRead, pszSecName, "szOutlineRuleFilename"	,
			types.m_szOutlineRuleFilename,
			sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* アウトライン解析ルールファイル */
		cProfile.IOProfileData( bRead, pszSecName, "nSmartIndent"		, types.m_nSmartIndent );/* スマートインデント種別 */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "nImeState"			, types.m_nImeState );	//	IME制御

		//	2001/06/14 Start By asa-o: タイプ別の補完ファイルとキーワードヘルプ
		//	Oct. 5, 2002 genta sizeof()で誤ってポインタのサイズを取得していたのを修正
		cProfile.IOProfileData( bRead, pszSecName, "szHokanFile"		,
			types.m_szHokanFile,
			sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	補完ファイル
		cProfile.IOProfileData( bRead, pszSecName, "bUseKeyWordHelp"	, types.m_bUseKeyWordHelp );	//	キーワードヘルプを使用する
		cProfile.IOProfileData( bRead, pszSecName, "szKeyWordHelpFile"	, 
			types.m_szKeyWordHelpFile,
			sizeof( m_pShareData->m_Types[0].m_szKeyWordHelpFile ));	//	キーワードヘルプ 辞書ファイル
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( bRead, pszSecName, "bHokanLoHiCase"		, types.m_bHokanLoHiCase );

		//	2003.06.23 Moca ファイル内からの入力補完機能
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokanByFile"		, types.m_bUseHokanByFile );

		//@@@ 2002.2.4 YAZAKI
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp"			,
			types.m_szExtHelp,
			sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
			
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp"		,
			types.m_szExtHtmlHelp,
			sizeof( types.m_szExtHtmlHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"	, types.m_bHokanLoHiCase );

		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent"			, types.m_bAutoIndent );
		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent_ZENSPACE"	, types.m_bAutoIndent_ZENSPACE );
		cProfile.IOProfileData( bRead, pszSecName, "nIndentLayout"			, types.m_nIndentLayout );

		/* 色設定 I/O */
		IO_ColorSet( &cProfile, bRead, pszSecName, types.m_ColorInfoArr  );


//@@@ 2001.11.17 add start MIK
		{	//正規表現キーワード
			char	*p;
			cProfile.IOProfileData( bRead, pszSecName, "bUseRegexKeyword", types.m_bUseRegexKeyword );/* 正規表現キーワード使用するか？ */
			for(j = 0; j < 100; j++)
			{
				wsprintf( szKeyName, "RxKey[%03d]", j );
				if( bRead )
				{
					types.m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
					types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
					if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
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
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
				}
			}
		}
//@@@ 2001.11.17 add end MIK

		/* 禁則 */
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuHead"	, types.m_bKinsokuHead );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuTail"	, types.m_bKinsokuTail );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuRet"	, types.m_bKinsokuRet );	//@@@ 2002.04.13 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuKuto"	, types.m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuHead"	,
			types.m_szKinsokuHead,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuTail"	,
			types.m_szKinsokuTail,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
		cProfile.IOProfileData( bRead, pszSecName, "bUseDocumentIcon"	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta 変数名誤り修正

	}
}

/*!
	@brief 共有データのKeyWordsセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_KeyWords( const bool bRead, CProfile& cProfile )
{
	const char*		pszSecName = "KeyWords";
	int				i, j;
	char			szKeyName[64];
	char			szKeyData[1024];
	CKeyWordSetMgr*	pCKeyWordSetMgr = &m_pShareData->m_CKeyWordSetMgr;
	int				nKeyWordSetNum = pCKeyWordSetMgr->m_nKeyWordSetNum;

	cProfile.IOProfileData( bRead, pszSecName, "nCurrentKeyWordSetIdx"	, pCKeyWordSetMgr->m_nCurrentKeyWordSetIdx );
	bool bIOSuccess = cProfile.IOProfileData( bRead, pszSecName, "nKeyWordSetNum", nKeyWordSetNum );
	if( bRead ){
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
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nKEYWORDCASE );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nKeyWordNum );
				pCKeyWordSetMgr->AddKeyWordSet( szKeyData, nKEYWORDCASE, nKeyWordNum );
				// 2004.11.25 Moca szKW[%02d].Size の情報は利用する意味がない。
//				wsprintf( szKeyName, "szKW[%02d].Size", i );
//				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nMemLen );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, pszMem, nMemLen ) ){
					pCKeyWordSetMgr->SetKeyWordArr( i, nKeyWordNum, pszMem );
				}
			}
			delete [] pszMem;
		}
	}else{
		int nSize = pCKeyWordSetMgr->m_nKeyWordSetNum;
		for( i = 0; i < nSize; ++i ){
			wsprintf( szKeyName, "szSN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				pCKeyWordSetMgr->m_szSetNameArr[i],
				sizeof( pCKeyWordSetMgr->m_szSetNameArr[0] ));
			wsprintf( szKeyName, "nCASE[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pCKeyWordSetMgr->m_nKEYWORDCASEArr[i] );
			wsprintf( szKeyName, "nKWN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pCKeyWordSetMgr->m_nKeyWordNumArr[i] );
			
			int nMemLen = 0;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				nMemLen += strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				nMemLen ++;
			}
			nMemLen ++;
			wsprintf( szKeyName, "szKW[%02d].Size", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, nMemLen );
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
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pszMem, nMemLen );
			delete [] pszMem;
		}
	}
}

/*!
	@brief 共有データのMacroセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Macro( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Macro";
	int		i;	
	char	szKeyName[64];
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		MacroRec& macrorec = m_pShareData->m_MacroTable[i];
		//	Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if( !bRead && !lstrlen(macrorec.m_szName) && !lstrlen(macrorec.m_szFile) ) continue;
		wsprintf( szKeyName, "Name[%03d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, macrorec.m_szName, MACRONAME_MAX - 1 );
		wsprintf( szKeyName, "File[%03d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, macrorec.m_szFile, _MAX_PATH );
		wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
}

/*!
	@brief 共有データのOtherセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData::ShareData_IO_Other( const bool bRead, CProfile& cProfile )
{
	const char* pszSecName = "Other";	//セクションを1個作成した。2003.05.12 MIK
	int		i;	
	char	szKeyName[64];

	/* **** その他のダイアログ **** */
	/* 外部コマンド実行の「標準出力を得る」 */
	cProfile.IOProfileData( bRead, pszSecName, "bGetStdout"		, m_pShareData->m_bGetStdout );
	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
	cProfile.IOProfileData( bRead, pszSecName, "bLineNumIsCRLF"	, m_pShareData->m_bLineNumIsCRLF );
	
	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( bRead, pszSecName, "nDiffFlgOpt"	, m_pShareData->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( bRead, pszSecName, "nTagsOpt"		, m_pShareData->m_nTagsOpt );
	cProfile.IOProfileData( bRead, pszSecName, "szTagsCmdLine"	, m_pShareData->m_szTagsCmdLine, sizeof( m_pShareData->m_szTagsCmdLine ) );
	
	//From Here 2005.04.03 MIK キーワード指定タグジャンプ
	cProfile.IOProfileData( bRead, pszSecName, "_TagJumpKeyword_Counts", m_pShareData->m_nTagJumpKeywordArrNum );
	for( i = 0; i < m_pShareData->m_nTagJumpKeywordArrNum; ++i ){
		wsprintf( szKeyName, "TagJumpKeyword[%02d]", i );
		if( i >= m_pShareData->m_nTagJumpKeywordArrNum ){
			strcpy( m_pShareData->m_szTagJumpKeywordArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName,
			m_pShareData->m_szTagJumpKeywordArr[i], sizeof( m_pShareData->m_szTagJumpKeywordArr[0] ));
	}
	cProfile.IOProfileData( bRead, pszSecName, "m_bTagJumpICase"		, m_pShareData->m_bTagJumpICase );
	cProfile.IOProfileData( bRead, pszSecName, "m_bTagJumpAnyWhere"		, m_pShareData->m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK キーワード指定タグジャンプの

	//	MIK バージョン情報（書き込みのみ）
	if( ! bRead ){
		TCHAR	iniVer[256];
		wsprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( m_pShareData->m_dwProductVersionMS ),
					LOWORD( m_pShareData->m_dwProductVersionMS ),
					HIWORD( m_pShareData->m_dwProductVersionLS ),
					LOWORD( m_pShareData->m_dwProductVersionLS ) );
		cProfile.IOProfileData( bRead, pszSecName, _T("szVersion")	, iniVer, sizeof( iniVer ) );
	}
}

/*!
	@brief 色設定 I/O

	指定された色設定を指定されたセクションに書き込む。または
	指定されたセクションからいろ設定を読み込む。

	@param[in,out]	pcProfile		書き出し、読み込み先Profile object (入出力方向はbReadに依存)
	@param[in]		bRead			true: 読み出し / false: 書き込み
	@param[in]		pszSecName		セクション名
	@param[in,out]	pColorInfoArr	書き出し、読み込み対象の色設定へのポインタ (入出力方向はbReadに依存)
*/
void CShareData::IO_ColorSet( CProfile* pcProfile, bool bRead, const char* pszSecName, ColorInfo* pColorInfoArr )
{
	char	szKeyName[256];
	char	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const char* pszForm = "%d,%d,%06x,%06x,%d";
#ifndef STR_COLORDATA_HEAD3
		wsprintf( szKeyName, "CI[%02d]", j );
#else
		wsprintf( szKeyName, "C[%s]", colorIDXKeyName[j] );	//Stonee, 2001/01/12, 2001/01/15
#endif
		if( bRead ){
			if( true == pcProfile->IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				pColorInfoArr[j].m_bUnderLine = FALSE;
				sscanf( szKeyData, pszForm,
					&pColorInfoArr[j].m_bDisp,
					&pColorInfoArr[j].m_bFatFont,
					&pColorInfoArr[j].m_colTEXT,
					&pColorInfoArr[j].m_colBACK,
					&pColorInfoArr[j].m_bUnderLine
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp,
				pColorInfoArr[j].m_bFatFont,
				pColorInfoArr[j].m_colTEXT,
				pColorInfoArr[j].m_colBACK,
				pColorInfoArr[j].m_bUnderLine
			);
			pcProfile->IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
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
