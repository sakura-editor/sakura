//	$Id$
/*!	@file
	@brief プロセス間共有データのファイルへ／からの入出力

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee, jepro, mik, asa-o, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK

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

	@param bRead [in] true: 読み込み / false: 書き込み
*/
BOOL CShareData::ShareData_IO_2( BOOL bRead )
{
	MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	char			szKey[256];
	char			szKeyData[1024];
	int				i, j;
	char			szKeyName[64];
	FileInfo		fiInit;
	FileInfo*		pfiWork;
	int				nZero = 0;
	int				nMemLen;
	char*			pszMem;
	char			szIniFileName[_MAX_PATH + 1];
	CProfile		cProfile;
	const char*		pszSecName;

	char			szPath[_MAX_PATH + 1];
	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFname[_MAX_FNAME];
	char			szExt[_MAX_EXT];
	const char*		pszKeyName;
	const char*		pszForm;

	/* exeのあるフォルダ */
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof(szPath)
	);
	_splitpath( szPath, szDrive, szDir, szFname, szExt );
	_makepath( szIniFileName, szDrive, szDir, szFname, "ini" );


//	MYTRACE( "Iniファイル処理-1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );


	if( bRead ){
		if( FALSE == cProfile.ReadProfile( szIniFileName ) ){
			/* 設定ファイルが存在しない */
			return FALSE;
		}
#ifdef _DEBUG
//@@@ 2001.12.26 YAZAKI デバッグ版が正常に起動しないため。
//		cProfile.DUMP();
#endif

	}
//	MYTRACE( "Iniファイル処理 0 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );

	{//	MRU
		pszSecName = "MRU";
		cProfile.IOProfileData( bRead, pszSecName, "_MRU_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nMRUArrNum, 0 );
		char	szKeyNameTop[64];

		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
			pfiWork = &m_pShareData->m_fiMRUArr[i];
			wsprintf( szKeyNameTop, "MRU[%02d].", i );

			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "nViewTopLine" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&pfiWork->m_nViewTopLine, 0 );
			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "nViewLeftCol" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&pfiWork->m_nViewLeftCol, 0 );
			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "nX" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&pfiWork->m_nX, 0 );
			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "nY" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&pfiWork->m_nY, 0 );
			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "nCharCode" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&pfiWork->m_nCharCode, 0 );
			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "szPath" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
			// 2002.01.16 hor
			strcpy( szKeyName, szKeyNameTop );
			strcat( szKeyName, "szMark" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
		}
		//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
		if ( bRead ){
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
			}
		}

		cProfile.IOProfileData( bRead, pszSecName, "_MRUFOLDER_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nOPENFOLDERArrNum, 0 );

		for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
			wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szOPENFOLDERArr[i],
				sizeof( m_pShareData->m_szOPENFOLDERArr[0] ));
		}
		if ( bRead ){
			for (; i< MAX_OPENFOLDER; ++i){
				strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
			}
		}
	}//	MRU

	{//	Keys
		pszSecName = "Keys";
		cProfile.IOProfileData( bRead, pszSecName, "_SEARCHKEY_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nSEARCHKEYArrNum, 0 );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
			if( i >= m_pShareData->m_nSEARCHKEYArrNum ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_szSEARCHKEYArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "_REPLACEKEY_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nREPLACEKEYArrNum, 0 );
		for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
			wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
			if( i >= m_pShareData->m_nREPLACEKEYArrNum ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_szREPLACEKEYArr[0] ) );
		}
	}// Keys

	{// Grep
		pszSecName = "Grep";
		cProfile.IOProfileData( bRead, pszSecName, "_GREPFILE_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nGREPFILEArrNum, 0 );
		for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
			wsprintf( szKeyName, "GREPFILE[%02d]", i );
			if( i >= m_pShareData->m_nGREPFILEArrNum ){
				strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szGREPFILEArr[i], sizeof( m_pShareData->m_szGREPFILEArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "_GREPFOLDER_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nGREPFOLDERArrNum, 0 );
		for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
			wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
			if( i >= m_pShareData->m_nGREPFOLDERArrNum ){
				strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_szGREPFOLDERArr[0] ));
		}
	}// Grep

	{//	Folders
		pszSecName = "Folders";
		/* マクロ用フォルダ */
		cProfile.IOProfileData( bRead, pszSecName, "szMACROFOLDER", REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_szMACROFOLDER, sizeof( m_pShareData->m_szMACROFOLDER ));
		/* 設定インポート用フォルダ */
		cProfile.IOProfileData( bRead, pszSecName, "szIMPORTFOLDER", REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_szIMPORTFOLDER, sizeof( m_pShareData->m_szIMPORTFOLDER ));
	}//	Folders

	{//	Cmd
		pszSecName = "Cmd";
		cProfile.IOProfileData( bRead, pszSecName, "nCmdArrNum", REGCNV_INT2SZ, (char*)&m_pShareData->m_nCmdArrNum, 0 );
		for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
			wsprintf( szKeyName, "szCmdArr[%02d]", i );
			if( i >= m_pShareData->m_nCmdArrNum ){
				strcpy( m_pShareData->m_szCmdArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szCmdArr[i], sizeof( m_pShareData->m_szCmdArr[0] ));
		}
	}//	Cmd

	{//	Nickname
		pszSecName = "Nickname";
		cProfile.IOProfileData( bRead, pszSecName, "ArrNum", REGCNV_INT2SZ, (char*)&m_pShareData->m_nTransformFileNameArrNum, 0 );
		for( i = 0; i < m_pShareData->m_nTransformFileNameArrNum; ++i ){
			wsprintf( szKeyName, "From%02d", i );
			if( i >= m_pShareData->m_nTransformFileNameArrNum ){
				strcpy( m_pShareData->m_szTransformFileNameFrom[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_szTransformFileNameFrom[0] ));

			wsprintf( szKeyName, "To%02d", i );
			if( i >= m_pShareData->m_nTransformFileNameArrNum ){
				strcpy( m_pShareData->m_szTransformFileNameFrom[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_szTransformFileNameTo[i], sizeof( m_pShareData->m_szTransformFileNameTo[0] ));
		}
		// 読み込み時，残りをNULLで再初期化
		if( bRead ){
			for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
				m_pShareData->m_szTransformFileNameFrom[i][0] = '\0';
				m_pShareData->m_szTransformFileNameTo[i][0]   = '\0';
			}
		}
	}//	Nickname

	/* 共通設定 */
	{
		pszSecName = "Common";

		cProfile.IOProfileData( bRead, pszSecName, "nCaretType"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nCaretType, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bIsINSMode"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bIsINSMode, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bIsFreeCursorMode"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bIsFreeCursorMode, 0 );

		cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchWord"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bStopsBothEndsWhenSearchWord, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchParagraph"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph, 0 );
		//	Oct. 27, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreCurPosition"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bRestoreCurPosition, 0 );
		// 2002.01.16 hor
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreBookmarks"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bRestoreBookmarks, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bAddCRLFWhenCopy"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAddCRLFWhenCopy, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScrollLineNum"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRepeatedScrollLineNum, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScroll_Smooth"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRepeatedScroll_Smooth, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bExitConfirm"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bExitConfirm, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchRegularExp"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bRegularExp, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchLoHiCase"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bLoHiCase, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchWordOnly"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bWordOnly, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchNOTIFYNOTFOUND"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bNOTIFYNOTFOUND, 0 );
		// 2002.01.26 hor
		cProfile.IOProfileData( bRead, pszSecName, "bSearchAll"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSearchAll, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchSelectedArea"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSelectedArea, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepSubFolder"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepSubFolder, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepOutputLine"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepOutputLine, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nGrepOutputStyle"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nGrepOutputStyle, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepDefaultFolder"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepDefaultFolder, 0 );
		// 2002/09/21 Moca 追加
		cProfile.IOProfileData( bRead, pszSecName, "nGrepCharSet"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nGrepCharSet, 0 );

		cProfile.IOProfileData( bRead, pszSecName, "bGTJW_RETURN"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGTJW_RETURN, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGTJW_LDBLCLK"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGTJW_LDBLCLK, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUp"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bBackUp, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDialog"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bBackUpDialog, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpFolder"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bBackUpFolder, 0 );

		if( !bRead ){
			int	nDummy;
			int	nCharChars;
			nDummy = strlen( m_pShareData->m_Common.m_szBackUpFolder );
			/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
			nCharChars = &m_pShareData->m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_pShareData->m_Common.m_szBackUpFolder, nDummy, &m_pShareData->m_Common.m_szBackUpFolder[nDummy] );
			if( 1 == nCharChars && m_pShareData->m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
			}else{
				strcat( m_pShareData->m_Common.m_szBackUpFolder, "\\" );
			}
		}
		cProfile.IOProfileData( bRead, pszSecName, "szBackUpFolder", REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Common.m_szBackUpFolder, sizeof( m_pShareData->m_Common.m_szBackUpFolder ));
		if( bRead ){
			int	nDummy;
			int	nCharChars;
			nDummy = strlen( m_pShareData->m_Common.m_szBackUpFolder );
			/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
			nCharChars = &m_pShareData->m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_pShareData->m_Common.m_szBackUpFolder, nDummy, &m_pShareData->m_Common.m_szBackUpFolder[nDummy] );
			if( 1 == nCharChars && m_pShareData->m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
			}else{
				strcat( m_pShareData->m_Common.m_szBackUpFolder, "\\" );
			}
		}


		cProfile.IOProfileData( bRead, pszSecName, "nBackUpType"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt1"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType_Opt1, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt2"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType_Opt2, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt3"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType_Opt3, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDustBox"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bBackUpDustBox, 0 );	//@@@ 2001.12.11 add MIK
		cProfile.IOProfileData( bRead, pszSecName, "nFileShareMode"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nFileShareMode, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp"					, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Common.m_szExtHelp, sizeof( m_pShareData->m_Common.m_szExtHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp"				, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Common.m_szExtHtmlHelp, sizeof( m_pShareData->m_Common.m_szExtHtmlHelp ));

		// ai 02/05/23 Add S
		{// Keword Help Font
			pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, "khlf", REGCNV_SZ2SZ,
					(char*)szKeyData, sizeof( szKeyData )) ){
					sscanf( szKeyData, pszForm,
						&m_pShareData->m_Common.m_lf_kh.lfHeight,
						&m_pShareData->m_Common.m_lf_kh.lfWidth,
						&m_pShareData->m_Common.m_lf_kh.lfEscapement,
						&m_pShareData->m_Common.m_lf_kh.lfOrientation,
						&m_pShareData->m_Common.m_lf_kh.lfWeight,
						&m_pShareData->m_Common.m_lf_kh.lfItalic,
						&m_pShareData->m_Common.m_lf_kh.lfUnderline,
						&m_pShareData->m_Common.m_lf_kh.lfStrikeOut,
						&m_pShareData->m_Common.m_lf_kh.lfCharSet,
						&m_pShareData->m_Common.m_lf_kh.lfOutPrecision,
						&m_pShareData->m_Common.m_lf_kh.lfClipPrecision,
						&m_pShareData->m_Common.m_lf_kh.lfQuality,
						&m_pShareData->m_Common.m_lf_kh.lfPitchAndFamily
					);
				}
			}else{
				wsprintf( szKeyData, pszForm,
					m_pShareData->m_Common.m_lf_kh.lfHeight,
					m_pShareData->m_Common.m_lf_kh.lfWidth,
					m_pShareData->m_Common.m_lf_kh.lfEscapement,
					m_pShareData->m_Common.m_lf_kh.lfOrientation,
					m_pShareData->m_Common.m_lf_kh.lfWeight,
					m_pShareData->m_Common.m_lf_kh.lfItalic,
					m_pShareData->m_Common.m_lf_kh.lfUnderline,
					m_pShareData->m_Common.m_lf_kh.lfStrikeOut,
					m_pShareData->m_Common.m_lf_kh.lfCharSet,
					m_pShareData->m_Common.m_lf_kh.lfOutPrecision,
					m_pShareData->m_Common.m_lf_kh.lfClipPrecision,
					m_pShareData->m_Common.m_lf_kh.lfQuality,
					m_pShareData->m_Common.m_lf_kh.lfPitchAndFamily
				);
				cProfile.IOProfileData( bRead, pszSecName, "khlf", REGCNV_SZ2SZ, (char*)szKeyData, 0 );
			}

			cProfile.IOProfileData( bRead, pszSecName, "khlfFaceName"			, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_lf_kh.lfFaceName, 0 );

		}// Keword Help Font
		// ai 02/05/23 Add S


		cProfile.IOProfileData( bRead, pszSecName, "nMRUArrNum_MAX"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nMRUArrNum_MAX, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nOPENFOLDERArrNum_MAX"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispTOOLBAR"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispTOOLBAR, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispSTATUSBAR"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispSTATUSBAR, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispFUNCKEYWND"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispFUNCKEYWND, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_Place"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nFUNCKEYWND_Place, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_GroupNum"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nFUNCKEYWND_GroupNum, 0 );		// 2002/11/04 Moca ファンクションキーのグループボタン数
		// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndHScroll"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSplitterWndHScroll, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndVScroll"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSplitterWndVScroll, 0 );

		cProfile.IOProfileData( bRead, pszSecName, "szMidashiKigou"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szMidashiKigou, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szInyouKigou"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szInyouKigou, 0 );

		// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokan"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseHokan, 0 );
		// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
		// 2001/06/19 asa-o タイプ別に移動したので削除：1行
		cProfile.IOProfileData( bRead, pszSecName, "bSaveWindowSize"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSaveWindowSize, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeType"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nWinSizeType, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCX"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nWinSizeCX, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCY"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nWinSizeCY, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayUse"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseTaskTray, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayStay"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bStayTaskTray, 0 );
//@@@ 2002.01.08 YAZAKI タスクトレイを「使わない」にしても常駐がチェックが残っていると再起動で「使う・常駐」になるバグ修正
#if 0
		if( bRead ){
			/* タスクトレイに常駐するときは、必ずタスクトレイアイコンを使う */
			if( m_pShareData->m_Common.m_bStayTaskTray ){
				m_pShareData->m_Common.m_bUseTaskTray = TRUE;
			}
		}
#endif
		cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyCode"		, REGCNV_WORD2SZ, (char*)&m_pShareData->m_Common.m_wTrayMenuHotKeyCode, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyMods"		, REGCNV_WORD2SZ, (char*)&m_pShareData->m_Common.m_wTrayMenuHotKeyMods, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DragDrop"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseOLE_DragDrop, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DropSource"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseOLE_DropSource, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispExitingDialog"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispExitingDialog, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bEnableUnmodifiedOverwrite"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSelectClickedURL"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSelectClickedURL, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepExitConfirm"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepExitConfirm, 0 );/* Grepモードで保存確認するか */
//		cProfile.IOProfileData( bRead, pszSecName, "bRulerDisp"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bRulerDisp, 0 );/* ルーラー表示 */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerHeight"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRulerHeight, 0 );/* ルーラー高さ */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerBottomSpace"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRulerBottomSpace, 0 );/* ルーラーとテキストの隙間 */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerType"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRulerType, 0 );/* ルーラーのタイプ */
		//	Sep. 18, 2002 genta 追加
		cProfile.IOProfileData( bRead, pszSecName, "nLineNumberRightSpace"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nLineNumRightSpace, 0 );/* 行番号の右側の隙間 */
		cProfile.IOProfileData( bRead, pszSecName, "bCopyAndDisablSelection"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bCopyAndDisablSelection, 0 );/* コピーしたら選択解除 */
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bHtmlHelpIsSingle, 0 );/* HtmlHelpビューアはひとつ */
		cProfile.IOProfileData( bRead, pszSecName, "bCompareAndTileHorz"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bCompareAndTileHorz, 0 );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
		cProfile.IOProfileData( bRead, pszSecName, "bDropFileAndClose"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDropFileAndClose, 0 );/* ファイルをドロップしたときは閉じて開く */
		cProfile.IOProfileData( bRead, pszSecName, "nDropFileNumMax"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nDropFileNumMax, 0 );/* 一度にドロップ可能なファイル数 */
		cProfile.IOProfileData( bRead, pszSecName, "bCheckFileTimeStamp"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bCheckFileTimeStamp, 0 );/* 更新の監視 */
		cProfile.IOProfileData( bRead, pszSecName, "bNotOverWriteCRLF"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bNotOverWriteCRLF, 0 );/* 改行は上書きしない */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFind"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoCloseDlgFind, 0 );/* 検索ダイアログを自動的に閉じる */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFuncList"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoCloseDlgFuncList, 0 );/* アウトライン ダイアログを自動的に閉じる */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgReplace"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoCloseDlgReplace, 0 );/* 置換 ダイアログを自動的に閉じる */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoColmnPaste"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoColmnPaste, 0 );/* 矩形コピーのテキストは常に矩形貼り付け */
		cProfile.IOProfileData( bRead, pszSecName, "bScrollBarHorz"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bScrollBarHorz, 0 );/* 水平スクロールバーを使う */

		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RETURN"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bHokanKey_RETURN, 0 );/* VK_RETURN 補完決定キーが有効/無効 */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_TAB"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bHokanKey_TAB, 0 );/* VK_TAB    補完決定キーが有効/無効 */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RIGHT"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bHokanKey_RIGHT, 0 );/* VK_RIGHT  補完決定キーが有効/無効 */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_SPACE"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bHokanKey_SPACE, 0 );/* VK_SPACE  補完決定キーが有効/無効 */

		cProfile.IOProfileData( bRead, pszSecName, "nDateFormatType"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nDateFormatType, 0 );/* 日付書式のタイプ */
		cProfile.IOProfileData( bRead, pszSecName, "szDateFormat"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szDateFormat, 0 );//日付書式
		cProfile.IOProfileData( bRead, pszSecName, "nTimeFormatType"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nTimeFormatType, 0 );/* 時刻書式のタイプ */
		cProfile.IOProfileData( bRead, pszSecName, "szTimeFormat"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szTimeFormat, 0 );//時刻書式

		cProfile.IOProfileData( bRead, pszSecName, "bMenuIcon"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bMenuIcon, 0 );//メニューにアイコンを表示する
		cProfile.IOProfileData( bRead, pszSecName, "bAutoMIMEdecode"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoMIMEdecode, 0 );//ファイル読み込み時にMIMEのdecodeを行うか



		/* 「開く」ダイアログのサイズと位置 */
		pszKeyName = "rcOpenDialog";
		pszForm = "%d,%d,%d,%d";
		if( bRead ){
			if( TRUE == cProfile.IOProfileData( bRead, pszSecName, pszKeyName, REGCNV_SZ2SZ,
				(char*)szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Common.m_rcOpenDialog.left,
					&m_pShareData->m_Common.m_rcOpenDialog.top,
					&m_pShareData->m_Common.m_rcOpenDialog.right,
					&m_pShareData->m_Common.m_rcOpenDialog.bottom
				);
			}
		}else{
			wsprintf( szKeyData, pszForm,
				m_pShareData->m_Common.m_rcOpenDialog.left,
				m_pShareData->m_Common.m_rcOpenDialog.top,
				m_pShareData->m_Common.m_rcOpenDialog.right,
				m_pShareData->m_Common.m_rcOpenDialog.bottom
			);
			cProfile.IOProfileData( bRead, pszSecName, pszKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
		}

		//2002.02.08 aroka,hor
		cProfile.IOProfileData( bRead, pszSecName, "bMarkUpBlankLineEnable"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bMarkUpBlankLineEnable, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bFunclistSetFocusOnJump"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bFunclistSetFocusOnJump, 0 );

	}// Common
	
	/* ツールバー */
	{
		pszSecName = "Toolbar";
		cProfile.IOProfileData( bRead, pszSecName, "nToolBarButtonNum", REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nToolBarButtonNum, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bToolBarIsFlat", REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bToolBarIsFlat, 0 );
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			wsprintf( szKeyName, "nTBB[%03d]", i );
			if( i < m_pShareData->m_Common.m_nToolBarButtonNum ){
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nToolBarButtonIdxArr[i], 0 );
			}else{
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&nZero, 0 );
			}
		}
	}// Tool bar

	/* カスタムメニュー */
	{
		pszSecName = "CustMenu";
		for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( szKeyName, "szCMN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta 最大長指定
			wsprintf( szKeyName, "nCMIN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nCustMenuItemNumArr[i], 0 );
			for( j = 0; j < m_pShareData->m_Common.m_nCustMenuItemNumArr[i]; ++j ){
				wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j], 0 );
				wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_CHAR2SZ, (char*)&m_pShareData->m_Common.m_nCustMenuItemKeyArr[i][j], 0 );
			}
		}
	}// Custom menu

	{// Font
		pszSecName = "Font";
		pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( bRead ){
			if( TRUE == cProfile.IOProfileData( bRead, pszSecName, "lf", REGCNV_SZ2SZ,
				(char*)szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Common.m_lf.lfHeight,
					&m_pShareData->m_Common.m_lf.lfWidth,
					&m_pShareData->m_Common.m_lf.lfEscapement,
					&m_pShareData->m_Common.m_lf.lfOrientation,
					&m_pShareData->m_Common.m_lf.lfWeight,
					&m_pShareData->m_Common.m_lf.lfItalic,
					&m_pShareData->m_Common.m_lf.lfUnderline,
					&m_pShareData->m_Common.m_lf.lfStrikeOut,
					&m_pShareData->m_Common.m_lf.lfCharSet,
					&m_pShareData->m_Common.m_lf.lfOutPrecision,
					&m_pShareData->m_Common.m_lf.lfClipPrecision,
					&m_pShareData->m_Common.m_lf.lfQuality,
					&m_pShareData->m_Common.m_lf.lfPitchAndFamily
				);
			}
		}else{
			wsprintf( szKeyData, pszForm,
				m_pShareData->m_Common.m_lf.lfHeight,
				m_pShareData->m_Common.m_lf.lfWidth,
				m_pShareData->m_Common.m_lf.lfEscapement,
				m_pShareData->m_Common.m_lf.lfOrientation,
				m_pShareData->m_Common.m_lf.lfWeight,
				m_pShareData->m_Common.m_lf.lfItalic,
				m_pShareData->m_Common.m_lf.lfUnderline,
				m_pShareData->m_Common.m_lf.lfStrikeOut,
				m_pShareData->m_Common.m_lf.lfCharSet,
				m_pShareData->m_Common.m_lf.lfOutPrecision,
				m_pShareData->m_Common.m_lf.lfClipPrecision,
				m_pShareData->m_Common.m_lf.lfQuality,
				m_pShareData->m_Common.m_lf.lfPitchAndFamily
			);
			cProfile.IOProfileData( bRead, pszSecName, "lf", REGCNV_SZ2SZ, (char*)szKeyData, 0 );
		}


		cProfile.IOProfileData( bRead, pszSecName, "lfFaceName"			, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Common.m_lf.lfFaceName, sizeof( m_pShareData->m_Common.m_lf.lfFaceName ));

		cProfile.IOProfileData( bRead, pszSecName, "bFontIs_FIXED_PITCH", REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bFontIs_FIXED_PITCH, 0 );
	}//	Font

	/* キー割り当て */
	{
		pszSecName = "KeyBind";

		for( i = 0; i < m_pShareData->m_nKeyNameArrNum; ++i ){

			strcpy( szKeyName, m_pShareData->m_pKeyNameArr[i].m_szKeyName );

			if( bRead ){
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
					(char*)szKeyData, sizeof( szKeyData )) ){
					sscanf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[0],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[1],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[2],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[3],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[4],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[5],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[6],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[7]
					 );
				}
			}else{
				wsprintf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[0],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[1],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[2],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[3],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[4],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[5],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[6],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[7]
				);
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
			}
		}
	}// KeyBind

	/* 印刷設定 */
	{
		pszSecName = "Print";
		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			wsprintf( szKeyName, "PS[%02d].nInts", i );
			static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
					(char*)szKeyData, sizeof( szKeyData ) ) ){
					sscanf( szKeyData, pszForm,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu			,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize		,
						&m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap		,
						&m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber		,
						&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0]			,
						&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1]			,
						&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2]			,
						&m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0]			,
						&m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1]			,
						&m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2]

					 );
				}
			}else{
				wsprintf( szKeyData, pszForm,
					m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth		,
					m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight		,
					m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing		,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation	,
					m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize		,
					m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap			,
					m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber		,
					m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0]			,
					m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1]			,
					m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2]			,
					m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0]			,
					m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1]			,
					m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2]
				);
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
			}


			wsprintf( szKeyName, "PS[%02d].szSName"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
			wsprintf( szKeyName, "PS[%02d].szFF"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
			wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
			for( j = 0; j < 3; ++j ){
				wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ ,
					(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[j],
					sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
				wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ ,
					(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szFooterForm[j],
					sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
			}
			wsprintf( szKeyName, "PS[%02d].szDriver", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDriverName,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
			wsprintf( szKeyName, "PS[%02d].szDevice", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDeviceName, 
				sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
			wsprintf( szKeyName, "PS[%02d].szOutput", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName,
				sizeof( m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName ));

			// 2002.02.16 hor とりあえず旧設定を変換しとく
			if(0==strcmp(m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0],"&f") &&
			   0==strcmp(m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0],"&C- &P -")
			){
				strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0], "$f" );
				strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0], "" );
				strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[1], "- $p -" );
			}

			//禁則	//@@@ 2002.04.09 MIK
			wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuHead, 0 );
			wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuTail, 0 );
			wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuRet,  0 );	//@@@ 2002.04.13 MIK
			wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuKuto, 0 );	//@@@ 2002.04.17 MIK

		}
	}// Print

	/* タイプ別設定 */
	for( i = 0; i < MAX_TYPES; ++i ){
		wsprintf( szKey, "Types(%d)", i );
		pszSecName = szKey;
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( bRead ){
			if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Types[i].m_nIdx,
					&m_pShareData->m_Types[i].m_nMaxLineSize,
					&m_pShareData->m_Types[i].m_nColmSpace,
					&m_pShareData->m_Types[i].m_nTabSpace,
					&m_pShareData->m_Types[i].m_nKeyWordSetIdx,
					&m_pShareData->m_Types[i].m_nKeyWordSetIdx2,	//MIK
					&m_pShareData->m_Types[i].m_nStringType,
					&m_pShareData->m_Types[i].m_bLineNumIsCRLF,
					&m_pShareData->m_Types[i].m_nLineTermType,
					&m_pShareData->m_Types[i].m_bWordWrap,
					&m_pShareData->m_Types[i].m_nCurrentPrintSetting
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				m_pShareData->m_Types[i].m_nIdx,
				m_pShareData->m_Types[i].m_nMaxLineSize,
				m_pShareData->m_Types[i].m_nColmSpace,
				m_pShareData->m_Types[i].m_nTabSpace,
				m_pShareData->m_Types[i].m_nKeyWordSetIdx,
				m_pShareData->m_Types[i].m_nKeyWordSetIdx2,	//MIK
				m_pShareData->m_Types[i].m_nStringType,
				m_pShareData->m_Types[i].m_bLineNumIsCRLF,
				m_pShareData->m_Types[i].m_nLineTermType,
				m_pShareData->m_Types[i].m_bWordWrap,
				m_pShareData->m_Types[i].m_nCurrentPrintSetting
			);
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
		}
		/* 行間のすきま */
		if( !bRead ){
			if( m_pShareData->m_Types[i].m_nLineSpace < /* 1 */ 0 ){
				m_pShareData->m_Types[i].m_nLineSpace = /* 1 */ 0;
			}
			if( m_pShareData->m_Types[i].m_nLineSpace > 16 ){
				m_pShareData->m_Types[i].m_nLineSpace = 16;
			}
		}
		cProfile.IOProfileData( bRead, pszSecName, "nLineSpace", REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nLineSpace, 0 );
		if( bRead ){
			if( m_pShareData->m_Types[i].m_nLineSpace < /* 1 */ 0 ){
				m_pShareData->m_Types[i].m_nLineSpace = /* 1 */ 0;
			}
			if( m_pShareData->m_Types[i].m_nLineSpace > 16 ){
				m_pShareData->m_Types[i].m_nLineSpace = 16;
			}
		}


		cProfile.IOProfileData( bRead, pszSecName, "szTypeName"			, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_szTypeName,
			sizeof( m_pShareData->m_Types[0].m_szTypeName ));
		cProfile.IOProfileData( bRead, pszSecName, "szTypeExts"			, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_szTypeExts,
			sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		cProfile.IOProfileData( bRead, pszSecName, "szTabViewString"	, REGCNV_SZ2SZ,
		(char*)/*&*/m_pShareData->m_Types[i].m_szTabViewString,
		sizeof( m_pShareData->m_Types[i].m_szTabViewString ));
//#endif
		cProfile.IOProfileData( bRead, pszSecName, "bInsSpace"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bInsSpace, 0 );	// 2001.12.03 hor

		// From Here Sep. 28, 2002 genta / YAZAKI
		if( bRead ){
			//	Block Comment
			char buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
			
			buffer[0][0] = buffer[1][0] = '\0';
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	, REGCNV_SZ2SZ,
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );			
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	, REGCNV_SZ2SZ,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			m_pShareData->m_Types[i].m_cBlockComment.CopyTo( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = '\0';
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2", REGCNV_SZ2SZ,
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	, REGCNV_SZ2SZ,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			m_pShareData->m_Types[i].m_cBlockComment.CopyTo( 1, buffer[0], buffer[1] );
			
			//	Line Comment
			char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
			int  pos;

			lbuf[0] = '\0'; pos = -1;
			cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		, REGCNV_SZ2SZ,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, REGCNV_INT2SZ,
				(char*)&pos, 0 );
			m_pShareData->m_Types[i].m_cLineComment.CopyTo( 0, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		, REGCNV_SZ2SZ,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2"	, REGCNV_INT2SZ,
				(char*)&pos, 0 );
			m_pShareData->m_Types[i].m_cLineComment.CopyTo( 1, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		, REGCNV_SZ2SZ,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO 追加
			cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3"	, REGCNV_INT2SZ,
				(char*)&pos, 0 );	//Jun. 01, 2001 JEPRO 追加
			m_pShareData->m_Types[i].m_cLineComment.CopyTo( 2, lbuf, pos );
		}
		else { // write
			//	Block Comment
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentFrom( 0 ), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentTo( 0 ), 0 );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2", REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentFrom( 1 ), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentTo(1), 0 );

			//	Line Comment
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_cLineComment.getLineComment( 0 ), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_cLineComment.getLineComment( 1 ), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_cLineComment.getLineComment( 2 ), 0 );	//Jun. 01, 2001 JEPRO 追加

		//	From here May 12, 2001 genta
		int pos;
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, REGCNV_INT2SZ, (char*)&pos, 0 );
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 1 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2"	, REGCNV_INT2SZ, (char*)&pos, 0 );
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 2 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3"	, REGCNV_INT2SZ, (char*)&pos, 0 );	//Jun. 01, 2001 JEPRO 追加
		//	To here May 12, 2001 genta

		}
		// To Here Sep. 28, 2002 genta / YAZAKI

		cProfile.IOProfileData( bRead, pszSecName, "szIndentChars"		, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_szIndentChars,
			sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
		cProfile.IOProfileData( bRead, pszSecName, "cLineTermChar"		, REG_BINARY, (char*)&m_pShareData->m_Types[i].m_cLineTermChar, sizeof( m_pShareData->m_Types[i].m_cLineTermChar ) );

		cProfile.IOProfileData( bRead, pszSecName, "nDefaultOutline"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nDefaultOutline, 0 );/* アウトライン解析方法 */
		cProfile.IOProfileData( bRead, pszSecName, "szOutlineRuleFilename"	, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_szOutlineRuleFilename,
			sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* アウトライン解析ルールファイル */
		cProfile.IOProfileData( bRead, pszSecName, "nSmartIndent"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nSmartIndent, 0 );/* スマートインデント種別 */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "nImeState"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nImeState, 0 );	//	IME制御

		//	2001/06/14 Start By asa-o: タイプ別の補完ファイルとキーワードヘルプ
		//	Oct. 5, 2002 genta sizeof()で誤ってポインタのサイズを取得していたのを修正
		cProfile.IOProfileData( bRead, pszSecName, "szHokanFile"		, REGCNV_SZ2SZ,
			(char*)&m_pShareData->m_Types[i].m_szHokanFile,
			sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	補完ファイル
		cProfile.IOProfileData( bRead, pszSecName, "bUseKeyWordHelp"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bUseKeyWordHelp, 0 );	//	キーワードヘルプを使用する
		cProfile.IOProfileData( bRead, pszSecName, "szKeyWordHelpFile"	, REGCNV_SZ2SZ,
			(char*)&m_pShareData->m_Types[i].m_szKeyWordHelpFile,
			sizeof( m_pShareData->m_Types[0].m_szKeyWordHelpFile ));	//	キーワードヘルプ 辞書ファイル
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( bRead, pszSecName, "bHokanLoHiCase"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bHokanLoHiCase, 0 );

		//@@@ 2002.2.4 YAZAKI
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp"			, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_szExtHelp,
			sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
			
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp"		, REGCNV_SZ2SZ,
			(char*)/*&*/m_pShareData->m_Types[i].m_szExtHtmlHelp,
			sizeof( m_pShareData->m_Types[i].m_szExtHtmlHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bHokanLoHiCase, 0 );

		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bAutoIndent, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent_ZENSPACE"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bAutoIndent_ZENSPACE, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nIndentLayout"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nIndentLayout, 0 );

		/* 色設定 I/O */
		IO_ColorSet( &cProfile, bRead, pszSecName, m_pShareData->m_Types[i].m_ColorInfoArr  );


//@@@ 2001.11.17 add start MIK
		{	//正規表現キーワード
			char	*p;
			cProfile.IOProfileData( bRead, pszSecName, "bUseRegexKeyword", REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bUseRegexKeyword, 0 );/* 正規表現キーワード使用するか？ */
			for(j = 0; j < 100; j++)
			{
				wsprintf( szKeyName, "RxKey[%03d]", j );
				if( bRead )
				{
					m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
					m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
					if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
						(char*)szKeyData, sizeof( szKeyData )) )
					{
						p = strchr(szKeyData, ',');
						if( p )
						{
							*p = '\0';
							m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(szKeyData);	//@@@ 2002.04.30
							if( m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex == -1 )	//名前でない
								m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = atoi(szKeyData);
							p++;
							strcpy(m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword, p);
							if( m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex < 0
							 || m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
							{
								m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
							}
						}
					}
				}
				else
				// 2002.02.08 hor 未定義値を無視
				if(lstrlen(m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword))
				{
					//wsprintf( szKeyData, "%d,%s",
					//	m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex,
					//	m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword);
					wsprintf( szKeyData, "%s,%s",
						GetColorNameByIndex( m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex ),
						m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword);
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
				}
			}
		}
//@@@ 2001.11.17 add end MIK

		/* 禁則 */
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuHead"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bKinsokuHead, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuTail"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bKinsokuTail, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuRet"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bKinsokuRet,  0 );	//@@@ 2002.04.13 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuKuto"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bKinsokuKuto, 0 );	//@@@ 2002.04.17 MIK
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuHead"	, REGCNV_SZ2SZ,
			(char*)&m_pShareData->m_Types[i].m_szKinsokuHead,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuTail"	, REGCNV_SZ2SZ,
			(char*)&m_pShareData->m_Types[i].m_szKinsokuTail,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
		cProfile.IOProfileData( bRead, pszSecName, "bUseDocumentIcon"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bUseDocumentIcon, 0 );	// Sep. 19 ,2002 genta 変数名誤り修正

	}// Types ( for loop)

	/* 強調キーワード */
	{
		pszSecName = "KeyWords";
		cProfile.IOProfileData( bRead, pszSecName, "nCurrentKeyWordSetIdx"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nKeyWordSetNum"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum, 0 );


		for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			wsprintf( szKeyName, "szSN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)/*&*/m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[i],
				sizeof( m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[0] ));
			wsprintf( szKeyName, "nCASE[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_CKeyWordSetMgr.m_nKEYWORDCASEArr[i], 0 );
			wsprintf( szKeyName, "nKWN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i], 0 );
		}
		if( bRead ){
			pszMem = new char[MAX_SETNUM * MAX_KEYWORDNUM * ( MAX_KEYWORDLEN ) + 1];
			for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
				wsprintf( szKeyName, "szKW[%02d].Size", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&nMemLen, 0 );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REG_MULTI_SZ, (char*)pszMem, nMemLen ) ){
					nMemLen = 0;
					for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
						strcpy( m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j], &pszMem[nMemLen] );
						nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
						nMemLen ++;
					}
				}
			}
			delete [] pszMem;
		}else{
			for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
				nMemLen = 0;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
					nMemLen ++;
				}
				nMemLen ++;
				wsprintf( szKeyName, "szKW[%02d].Size", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&nMemLen, 0 );
				pszMem = new char[nMemLen];
				nMemLen = 0;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					strcpy( &pszMem[nMemLen], m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
					nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
					nMemLen ++;
				}
				pszMem[nMemLen] = '\0';
				wsprintf( szKeyName, "szKW[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REG_MULTI_SZ, (char*)pszMem, nMemLen );
				delete [] pszMem;
			}
		}
	}// Keywords
	
	//	From Here Sep. 14, 2001 genta
	//	Macro
	{
		pszSecName = "Macro";
		
		for( i = 0; i < MAX_CUSTMACRO; ++i ){
			//	Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
			// 2002.02.08 hor 未定義値を無視
			if( !bRead && !lstrlen(m_pShareData->m_MacroTable[i].m_szName) && !lstrlen(m_pShareData->m_MacroTable[i].m_szFile) ) continue;
			wsprintf( szKeyName, "Name[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)m_pShareData->m_MacroTable[i].m_szName, MACRONAME_MAX - 1 );
			wsprintf( szKeyName, "File[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)m_pShareData->m_MacroTable[i].m_szFile, _MAX_PATH );
			wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ,
				(char*)&m_pShareData->m_MacroTable[i].m_bReloadWhenExecute, 0 );
		}
	}// Macro
	//	To Here  Sep. 14, 2001 genta

//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
	/* **** その他のダイアログ **** */
	/* 外部コマンド実行の「標準出力を得る」 */
	cProfile.IOProfileData( bRead, pszSecName, "bGetStdout"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_bGetStdout, 0 );
	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
	cProfile.IOProfileData( bRead, pszSecName, "bLineNumIsCRLF"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_bLineNumIsCRLF, 0 );

	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( bRead, pszSecName, "nDiffFlgOpt"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_nDiffFlgOpt, 0 );
	
	if( !bRead ){
		cProfile.WriteProfile( szIniFileName, " sakura.ini テキストエディタ設定ファイル" );
	}

//	MYTRACE( "Iniファイル処理 8 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	MYTRACE( "Iniファイル処理 所要時間(ミリ秒) = %d\n", cRunningTimerStart.Read() );

	return TRUE;
}


/*!
	@brief 色設定 I/O

	指定された色設定を指定されたセクションに書き込む。または
	指定されたセクションからいろ設定を読み込む。

	@param pcProfile [i/o] 書き出し、読み込み先Profile object (入出力方向はbReadに依存)
	@param bRead [in] true: 読み出し / false: 書き込み
	@param pszSecName [in] セクション名
	@param pColorInfoArr [i/o] 書き出し、読み込み対象の色設定へのポインタ (入出力方向はbReadに依存)
*/
void CShareData::IO_ColorSet( CProfile* pcProfile, BOOL bRead, const char* pszSecName, ColorInfo* pColorInfoArr )
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
			if( TRUE == pcProfile->IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)szKeyData, sizeof( szKeyData )) ){
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
			pcProfile->IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
		}
	}
	return;
}


/*[EOF]*/
