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
// 2004-01-11 D.S.Koba CProfile変更によるコード簡略化

/*!
	共有データの読み込み/保存 2

	@param bRead [in] true: 読み込み / false: 書き込み
*/
BOOL CShareData::ShareData_IO_2( bool bRead )
{
	MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	char			szKey[256];
	char			szKeyData[1024];
	int				i, j;
	char			szKeyName[64];
	FileInfo		fiInit;
	FileInfo*		pfiWork;
	int				nZero = 0;
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

	{//	MRU
		pszSecName = "MRU";
		cProfile.IOProfileData( bRead, pszSecName, "_MRU_Counts", m_pShareData->m_nMRUArrNum );

		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
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

		for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
			wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_szOPENFOLDERArr[i],
				sizeof( m_pShareData->m_szOPENFOLDERArr[0] ));

			//お気に入り	//@@@ 2003.04.08 MIK
			strcat( szKeyName, ".bFavorite" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_bOPENFOLDERArrFavorite[i] );
		}
		if ( bRead ){
			for (; i< MAX_OPENFOLDER; ++i){
				strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
				m_pShareData->m_bOPENFOLDERArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
			}
		}
	}//	MRU

	{//	Keys
		pszSecName = "Keys";
		cProfile.IOProfileData( bRead, pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_nSEARCHKEYArrNum );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
			if( i >= m_pShareData->m_nSEARCHKEYArrNum ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_szSEARCHKEYArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_nREPLACEKEYArrNum );
		for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
			wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
			if( i >= m_pShareData->m_nREPLACEKEYArrNum ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_szREPLACEKEYArr[0] ) );
		}
	}// Keys

	{// Grep
		pszSecName = "Grep";
		cProfile.IOProfileData( bRead, pszSecName, "_GREPFILE_Counts", m_pShareData->m_nGREPFILEArrNum );
		for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
			wsprintf( szKeyName, "GREPFILE[%02d]", i );
			if( i >= m_pShareData->m_nGREPFILEArrNum ){
				strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szGREPFILEArr[i], sizeof( m_pShareData->m_szGREPFILEArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_nGREPFOLDERArrNum );
		for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
			wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
			if( i >= m_pShareData->m_nGREPFOLDERArrNum ){
				strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_szGREPFOLDERArr[0] ));
		}
	}// Grep

	{//	Folders
		pszSecName = "Folders";
		/* マクロ用フォルダ */
		cProfile.IOProfileData( bRead, pszSecName, "szMACROFOLDER",
			m_pShareData->m_szMACROFOLDER, sizeof( m_pShareData->m_szMACROFOLDER ));
		/* 設定インポート用フォルダ */
		cProfile.IOProfileData( bRead, pszSecName, "szIMPORTFOLDER",
			m_pShareData->m_szIMPORTFOLDER, sizeof( m_pShareData->m_szIMPORTFOLDER ));
	}//	Folders

	{//	Cmd
		pszSecName = "Cmd";
		cProfile.IOProfileData( bRead, pszSecName, "nCmdArrNum", m_pShareData->m_nCmdArrNum );
		for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
			wsprintf( szKeyName, "szCmdArr[%02d]", i );
			if( i >= m_pShareData->m_nCmdArrNum ){
				strcpy( m_pShareData->m_szCmdArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szCmdArr[i], sizeof( m_pShareData->m_szCmdArr[0] ));
		}
	}//	Cmd

	{//	Nickname
		pszSecName = "Nickname";
		cProfile.IOProfileData( bRead, pszSecName, "ArrNum", m_pShareData->m_nTransformFileNameArrNum );
		for( i = 0; i < m_pShareData->m_nTransformFileNameArrNum; ++i ){
			wsprintf( szKeyName, "From%02d", i );
			if( i >= m_pShareData->m_nTransformFileNameArrNum ){
				strcpy( m_pShareData->m_szTransformFileNameFrom[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_szTransformFileNameFrom[0] ));

			wsprintf( szKeyName, "To%02d", i );
			if( i >= m_pShareData->m_nTransformFileNameArrNum ){
				strcpy( m_pShareData->m_szTransformFileNameFrom[i], "" );
			}
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
	}//	Nickname

	/* 共通設定 */
	{
		pszSecName = "Common";

		cProfile.IOProfileData( bRead, pszSecName, "nCaretType"				, m_pShareData->m_Common.m_nCaretType );
		cProfile.IOProfileData( bRead, pszSecName, "bIsINSMode"				, m_pShareData->m_Common.m_bIsINSMode );
		cProfile.IOProfileData( bRead, pszSecName, "bIsFreeCursorMode"		, m_pShareData->m_Common.m_bIsFreeCursorMode );

		cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchWord"	, m_pShareData->m_Common.m_bStopsBothEndsWhenSearchWord );
		cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchParagraph"	, m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph );
		//	Oct. 27, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreCurPosition"	, m_pShareData->m_Common.m_bRestoreCurPosition );
		// 2002.01.16 hor
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreBookmarks"	, m_pShareData->m_Common.m_bRestoreBookmarks );
		cProfile.IOProfileData( bRead, pszSecName, "bAddCRLFWhenCopy"		, m_pShareData->m_Common.m_bAddCRLFWhenCopy );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScrollLineNum"	, m_pShareData->m_Common.m_nRepeatedScrollLineNum );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScroll_Smooth"	, m_pShareData->m_Common.m_nRepeatedScroll_Smooth );
		cProfile.IOProfileData( bRead, pszSecName, "bExitConfirm"			, m_pShareData->m_Common.m_bExitConfirm );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchRegularExp"		, m_pShareData->m_Common.m_bRegularExp );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchLoHiCase"		, m_pShareData->m_Common.m_bLoHiCase );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchWordOnly"		, m_pShareData->m_Common.m_bWordOnly );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchNOTIFYNOTFOUND"	, m_pShareData->m_Common.m_bNOTIFYNOTFOUND );
		// 2002.01.26 hor
		cProfile.IOProfileData( bRead, pszSecName, "bSearchAll"				, m_pShareData->m_Common.m_bSearchAll );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchSelectedArea"	, m_pShareData->m_Common.m_bSelectedArea );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepSubFolder"			, m_pShareData->m_Common.m_bGrepSubFolder );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepOutputLine"		, m_pShareData->m_Common.m_bGrepOutputLine );
		cProfile.IOProfileData( bRead, pszSecName, "nGrepOutputStyle"		, m_pShareData->m_Common.m_nGrepOutputStyle );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepDefaultFolder"		, m_pShareData->m_Common.m_bGrepDefaultFolder );
		// 2002/09/21 Moca 追加
		cProfile.IOProfileData( bRead, pszSecName, "nGrepCharSet"			, m_pShareData->m_Common.m_nGrepCharSet );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepRealTime"			, m_pShareData->m_Common.m_bGrepRealTimeView ); // 2003.06.16 Moca

		cProfile.IOProfileData( bRead, pszSecName, "bGTJW_RETURN"			, m_pShareData->m_Common.m_bGTJW_RETURN );
		cProfile.IOProfileData( bRead, pszSecName, "bGTJW_LDBLCLK"			, m_pShareData->m_Common.m_bGTJW_LDBLCLK );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUp"				, m_pShareData->m_Common.m_bBackUp );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDialog"			, m_pShareData->m_Common.m_bBackUpDialog );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpFolder"			, m_pShareData->m_Common.m_bBackUpFolder );

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
		cProfile.IOProfileData( bRead, pszSecName, "szBackUpFolder",
			m_pShareData->m_Common.m_szBackUpFolder, sizeof( m_pShareData->m_Common.m_szBackUpFolder ));
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


		cProfile.IOProfileData( bRead, pszSecName, "nBackUpType"				, m_pShareData->m_Common.m_nBackUpType );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt1"			, m_pShareData->m_Common.m_nBackUpType_Opt1 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt2"			, m_pShareData->m_Common.m_nBackUpType_Opt2 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt3"			, m_pShareData->m_Common.m_nBackUpType_Opt3 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDustBox"				, m_pShareData->m_Common.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
		cProfile.IOProfileData( bRead, pszSecName, "nFileShareMode"				, m_pShareData->m_Common.m_nFileShareMode );
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp",
			m_pShareData->m_Common.m_szExtHelp, sizeof( m_pShareData->m_Common.m_szExtHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp",
			m_pShareData->m_Common.m_szExtHtmlHelp, sizeof( m_pShareData->m_Common.m_szExtHtmlHelp ));
	
		cProfile.IOProfileData( bRead, pszSecName, "szMigemoDll",
			m_pShareData->m_Common.m_szMigemoDll, sizeof( m_pShareData->m_Common.m_szMigemoDll ));
		cProfile.IOProfileData( bRead, pszSecName, "szMigemoDict",
			m_pShareData->m_Common.m_szMigemoDict, sizeof( m_pShareData->m_Common.m_szMigemoDict ));

		// ai 02/05/23 Add S
		{// Keword Help Font
			pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( true == cProfile.IOProfileData( bRead, pszSecName, "khlf", szKeyData, sizeof( szKeyData )) ){
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
				cProfile.IOProfileData( bRead, pszSecName, "khlf", szKeyData, 0 );
			}

			cProfile.IOProfileData( bRead, pszSecName, "khlfFaceName"			, m_pShareData->m_Common.m_lf_kh.lfFaceName, 0 );

		}// Keword Help Font
		// ai 02/05/23 Add S


		cProfile.IOProfileData( bRead, pszSecName, "nMRUArrNum_MAX"				, m_pShareData->m_Common.m_nMRUArrNum_MAX );
		cProfile.IOProfileData( bRead, pszSecName, "nOPENFOLDERArrNum_MAX"		, m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX );
		cProfile.IOProfileData( bRead, pszSecName, "bDispTOOLBAR"				, m_pShareData->m_Common.m_bDispTOOLBAR );
		cProfile.IOProfileData( bRead, pszSecName, "bDispSTATUSBAR"				, m_pShareData->m_Common.m_bDispSTATUSBAR );
		cProfile.IOProfileData( bRead, pszSecName, "bDispFUNCKEYWND"			, m_pShareData->m_Common.m_bDispFUNCKEYWND );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_Place"			, m_pShareData->m_Common.m_nFUNCKEYWND_Place );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_GroupNum"		, m_pShareData->m_Common.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca ファンクションキーのグループボタン数

		cProfile.IOProfileData( bRead, pszSecName, "bDispTabWnd"				, m_pShareData->m_Common.m_bDispTabWnd );	//タブウインドウ	//@@@ 2003.05.31 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bDispTabWndMultiWin"		, m_pShareData->m_Common.m_bDispTabWndMultiWin );	//タブウインドウ	//@@@ 2003.05.31 MIK
		cProfile.IOProfileData( bRead, pszSecName, "szTabWndCaption"			, m_pShareData->m_Common.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK

		// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndHScroll"		, m_pShareData->m_Common.m_bSplitterWndHScroll );
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndVScroll"		, m_pShareData->m_Common.m_bSplitterWndVScroll );

		cProfile.IOProfileData( bRead, pszSecName, "szMidashiKigou"				, m_pShareData->m_Common.m_szMidashiKigou, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szInyouKigou"				, m_pShareData->m_Common.m_szInyouKigou, 0 );

		// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokan"					, m_pShareData->m_Common.m_bUseHokan );
		// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
		// 2001/06/19 asa-o タイプ別に移動したので削除：1行
		cProfile.IOProfileData( bRead, pszSecName, "bSaveWindowSize"			, m_pShareData->m_Common.m_nSaveWindowSize );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeType"				, m_pShareData->m_Common.m_nWinSizeType );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCX"					, m_pShareData->m_Common.m_nWinSizeCX );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCY"					, m_pShareData->m_Common.m_nWinSizeCY );
		// 2004.03.30 Moca *nWinPos*を追加
		cProfile.IOProfileData( bRead, pszSecName, "nSaveWindowPos"			, m_pShareData->m_Common.m_nSaveWindowPos );
		cProfile.IOProfileData( bRead, pszSecName, "nWinPosX"				, m_pShareData->m_Common.m_nWinPosX );
		cProfile.IOProfileData( bRead, pszSecName, "nWinPosY"				, m_pShareData->m_Common.m_nWinPosY );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayUse"				, m_pShareData->m_Common.m_bUseTaskTray );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayStay"				, m_pShareData->m_Common.m_bStayTaskTray );
//@@@ 2002.01.08 YAZAKI タスクトレイを「使わない」にしても常駐がチェックが残っていると再起動で「使う・常駐」になるバグ修正
#if 0
		if( bRead ){
			/* タスクトレイに常駐するときは、必ずタスクトレイアイコンを使う */
			if( m_pShareData->m_Common.m_bStayTaskTray ){
				m_pShareData->m_Common.m_bUseTaskTray = TRUE;
			}
		}
#endif
		cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyCode"		, m_pShareData->m_Common.m_wTrayMenuHotKeyCode );
		cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyMods"		, m_pShareData->m_Common.m_wTrayMenuHotKeyMods );
		cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DragDrop"			, m_pShareData->m_Common.m_bUseOLE_DragDrop );
		cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DropSource"			, m_pShareData->m_Common.m_bUseOLE_DropSource );
		cProfile.IOProfileData( bRead, pszSecName, "bDispExitingDialog"			, m_pShareData->m_Common.m_bDispExitingDialog );
		cProfile.IOProfileData( bRead, pszSecName, "bEnableUnmodifiedOverwrite"	, m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite );
		cProfile.IOProfileData( bRead, pszSecName, "bSelectClickedURL"			, m_pShareData->m_Common.m_bSelectClickedURL );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepExitConfirm"			, m_pShareData->m_Common.m_bGrepExitConfirm );/* Grepモードで保存確認するか */
//		cProfile.IOProfileData( bRead, pszSecName, "bRulerDisp"					, m_pShareData->m_Common.m_bRulerDisp );/* ルーラー表示 */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerHeight"				, m_pShareData->m_Common.m_nRulerHeight );/* ルーラー高さ */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerBottomSpace"			, m_pShareData->m_Common.m_nRulerBottomSpace );/* ルーラーとテキストの隙間 */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerType"					, m_pShareData->m_Common.m_nRulerType );/* ルーラーのタイプ */
		//	Sep. 18, 2002 genta 追加
		cProfile.IOProfileData( bRead, pszSecName, "nLineNumberRightSpace"		, m_pShareData->m_Common.m_nLineNumRightSpace );/* 行番号の右側の隙間 */
		cProfile.IOProfileData( bRead, pszSecName, "bCopyAndDisablSelection"	, m_pShareData->m_Common.m_bCopyAndDisablSelection );/* コピーしたら選択解除 */
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"			, m_pShareData->m_Common.m_bHtmlHelpIsSingle );/* HtmlHelpビューアはひとつ */
		cProfile.IOProfileData( bRead, pszSecName, "bCompareAndTileHorz"		, m_pShareData->m_Common.m_bCompareAndTileHorz );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
		cProfile.IOProfileData( bRead, pszSecName, "bDropFileAndClose"			, m_pShareData->m_Common.m_bDropFileAndClose );/* ファイルをドロップしたときは閉じて開く */
		cProfile.IOProfileData( bRead, pszSecName, "nDropFileNumMax"			, m_pShareData->m_Common.m_nDropFileNumMax );/* 一度にドロップ可能なファイル数 */
		cProfile.IOProfileData( bRead, pszSecName, "bCheckFileTimeStamp"		, m_pShareData->m_Common.m_bCheckFileTimeStamp );/* 更新の監視 */
		cProfile.IOProfileData( bRead, pszSecName, "bNotOverWriteCRLF"			, m_pShareData->m_Common.m_bNotOverWriteCRLF );/* 改行は上書きしない */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFind"			, m_pShareData->m_Common.m_bAutoCloseDlgFind );/* 検索ダイアログを自動的に閉じる */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFuncList"		, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );/* アウトライン ダイアログを自動的に閉じる */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgReplace"		, m_pShareData->m_Common.m_bAutoCloseDlgReplace );/* 置換 ダイアログを自動的に閉じる */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoColmnPaste"			, m_pShareData->m_Common.m_bAutoColmnPaste );/* 矩形コピーのテキストは常に矩形貼り付け */
		cProfile.IOProfileData( bRead, pszSecName, "bScrollBarHorz"				, m_pShareData->m_Common.m_bScrollBarHorz );/* 水平スクロールバーを使う */

		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RETURN"			, m_pShareData->m_Common.m_bHokanKey_RETURN );/* VK_RETURN 補完決定キーが有効/無効 */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_TAB"				, m_pShareData->m_Common.m_bHokanKey_TAB );/* VK_TAB    補完決定キーが有効/無効 */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RIGHT"			, m_pShareData->m_Common.m_bHokanKey_RIGHT );/* VK_RIGHT  補完決定キーが有効/無効 */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_SPACE"			, m_pShareData->m_Common.m_bHokanKey_SPACE );/* VK_SPACE  補完決定キーが有効/無効 */

		cProfile.IOProfileData( bRead, pszSecName, "nDateFormatType"			, m_pShareData->m_Common.m_nDateFormatType );/* 日付書式のタイプ */
		cProfile.IOProfileData( bRead, pszSecName, "szDateFormat"				, m_pShareData->m_Common.m_szDateFormat, 0 );//日付書式
		cProfile.IOProfileData( bRead, pszSecName, "nTimeFormatType"			, m_pShareData->m_Common.m_nTimeFormatType );/* 時刻書式のタイプ */
		cProfile.IOProfileData( bRead, pszSecName, "szTimeFormat"				, m_pShareData->m_Common.m_szTimeFormat, 0 );//時刻書式

		cProfile.IOProfileData( bRead, pszSecName, "bMenuIcon"					, m_pShareData->m_Common.m_bMenuIcon );//メニューにアイコンを表示する
		cProfile.IOProfileData( bRead, pszSecName, "bAutoMIMEdecode"			, m_pShareData->m_Common.m_bAutoMIMEdecode );//ファイル読み込み時にMIMEのdecodeを行うか
		cProfile.IOProfileData( bRead, pszSecName, "bQueryIfCodeChange"			, m_pShareData->m_Common.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
		cProfile.IOProfileData( bRead, pszSecName, "bAlertIfFileNotExist"		, m_pShareData->m_Common.m_bQueryIfCodeChange );// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する



		/* 「開く」ダイアログのサイズと位置 */
		pszKeyName = "rcOpenDialog";
		pszForm = "%d,%d,%d,%d";
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
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
			cProfile.IOProfileData( bRead, pszSecName, pszKeyName, szKeyData, 0 );
		}

		//2002.02.08 aroka,hor
		cProfile.IOProfileData( bRead, pszSecName, "bMarkUpBlankLineEnable"		, m_pShareData->m_Common.m_bMarkUpBlankLineEnable );
		cProfile.IOProfileData( bRead, pszSecName, "bFunclistSetFocusOnJump"	, m_pShareData->m_Common.m_bFunclistSetFocusOnJump );

		//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
		cProfile.IOProfileData( bRead, pszSecName, "szWinCaptionActive"
			, m_pShareData->m_Common.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
		cProfile.IOProfileData( bRead, pszSecName, "szWinCaptionInactive"
			, m_pShareData->m_Common.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );

	}// Common
	
	/* ツールバー */
	{
		pszSecName = "Toolbar";
		cProfile.IOProfileData( bRead, pszSecName, "nToolBarButtonNum", m_pShareData->m_Common.m_nToolBarButtonNum );
		cProfile.IOProfileData( bRead, pszSecName, "bToolBarIsFlat", m_pShareData->m_Common.m_bToolBarIsFlat );
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			wsprintf( szKeyName, "nTBB[%03d]", i );
			if( i < m_pShareData->m_Common.m_nToolBarButtonNum ){
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nToolBarButtonIdxArr[i] );
			}else{
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nZero );
			}
		}
	}// Tool bar

	/* カスタムメニュー */
	{
		pszSecName = "CustMenu";
		for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( szKeyName, "szCMN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta 最大長指定
			wsprintf( szKeyName, "nCMIN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nCustMenuItemNumArr[i] );
			for( j = 0; j < m_pShareData->m_Common.m_nCustMenuItemNumArr[i]; ++j ){
				wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j] );
				wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nCustMenuItemKeyArr[i][j] );
			}
		}
	}// Custom menu

	{// Font
		pszSecName = "Font";
		pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, "lf", szKeyData, sizeof( szKeyData )) ){
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
			cProfile.IOProfileData( bRead, pszSecName, "lf", szKeyData, 0 );
		}


		cProfile.IOProfileData( bRead, pszSecName, "lfFaceName",
			m_pShareData->m_Common.m_lf.lfFaceName, sizeof( m_pShareData->m_Common.m_lf.lfFaceName ));

		cProfile.IOProfileData( bRead, pszSecName, "bFontIs_FIXED_PITCH", m_pShareData->m_Common.m_bFontIs_FIXED_PITCH );
	}//	Font

	/* キー割り当て */
	{
		pszSecName = "KeyBind";

		for( i = 0; i < m_pShareData->m_nKeyNameArrNum; ++i ){

			strcpy( szKeyName, m_pShareData->m_pKeyNameArr[i].m_szKeyName );

			if( bRead ){
				if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					szKeyData, sizeof( szKeyData )) ){
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
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
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
				if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					szKeyData, sizeof( szKeyData ) ) ){
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
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
			}


			wsprintf( szKeyName, "PS[%02d].szSName"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
			wsprintf( szKeyName, "PS[%02d].szFF"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
			wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
			for( j = 0; j < 3; ++j ){
				wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[j],
					sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
				wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					m_pShareData->m_PrintSettingArr[i].m_szFooterForm[j],
					sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
			}
			wsprintf( szKeyName, "PS[%02d].szDriver", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDriverName,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
			wsprintf( szKeyName, "PS[%02d].szDevice", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDeviceName, 
				sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
			wsprintf( szKeyName, "PS[%02d].szOutput", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName,
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
			wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuHead );
			wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuTail );
			wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
			wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK

		}
	}// Print

	/* タイプ別設定 */
	for( i = 0; i < MAX_TYPES; ++i ){
		wsprintf( szKey, "Types(%d)", i );
		pszSecName = szKey;
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Types[i].m_nIdx,
					&m_pShareData->m_Types[i].m_nMaxLineSize,
					&m_pShareData->m_Types[i].m_nColmSpace,
					&m_pShareData->m_Types[i].m_nTabSpace,
					&m_pShareData->m_Types[i].m_nKeyWordSetIdx[0],
					&m_pShareData->m_Types[i].m_nKeyWordSetIdx[1],	//MIK
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
				m_pShareData->m_Types[i].m_nKeyWordSetIdx[0],
				m_pShareData->m_Types[i].m_nKeyWordSetIdx[1],	//MIK
				m_pShareData->m_Types[i].m_nStringType,
				m_pShareData->m_Types[i].m_bLineNumIsCRLF,
				m_pShareData->m_Types[i].m_nLineTermType,
				m_pShareData->m_Types[i].m_bWordWrap,
				m_pShareData->m_Types[i].m_nCurrentPrintSetting
			);
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
		}
		// 2005.01.13 MIK Keywordset 3-10
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect3",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[2] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect4",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[3] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect5",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[4] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect6",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[5] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect7",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[6] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect8",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[7] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect9",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[8] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect10", m_pShareData->m_Types[i].m_nKeyWordSetIdx[9] );

		/* 行間のすきま */
		if( !bRead ){
			if( m_pShareData->m_Types[i].m_nLineSpace < /* 1 */ 0 ){
				m_pShareData->m_Types[i].m_nLineSpace = /* 1 */ 0;
			}
			if( m_pShareData->m_Types[i].m_nLineSpace > LINESPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				m_pShareData->m_Types[i].m_nLineSpace = LINESPACE_MAX;
			}
		}
		cProfile.IOProfileData( bRead, pszSecName, "nLineSpace", m_pShareData->m_Types[i].m_nLineSpace );
		if( bRead ){
			if( m_pShareData->m_Types[i].m_nLineSpace < /* 1 */ 0 ){
				m_pShareData->m_Types[i].m_nLineSpace = /* 1 */ 0;
			}
			if( m_pShareData->m_Types[i].m_nLineSpace > 16 ){
				m_pShareData->m_Types[i].m_nLineSpace = 16;
			}
		}


		cProfile.IOProfileData( bRead, pszSecName, "szTypeName",
			m_pShareData->m_Types[i].m_szTypeName,
			sizeof( m_pShareData->m_Types[0].m_szTypeName ));
		cProfile.IOProfileData( bRead, pszSecName, "szTypeExts",
			m_pShareData->m_Types[i].m_szTypeExts,
			sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		cProfile.IOProfileData( bRead, pszSecName, "szTabViewString",
		/*&*/m_pShareData->m_Types[i].m_szTabViewString,
		sizeof( m_pShareData->m_Types[i].m_szTabViewString ));
//#endif
		cProfile.IOProfileData( bRead, pszSecName, "bTabArrow"			, m_pShareData->m_Types[i].m_bTabArrow );	//@@@ 2003.03.26 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bInsSpace"			, m_pShareData->m_Types[i].m_bInsSpace );	// 2001.12.03 hor

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
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cBlockComment.CopyTo( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2",
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cBlockComment.CopyTo( 1, buffer[0], buffer[1] );
			
			//	Line Comment
			char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
			int  pos;

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, pos );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cLineComment.CopyTo( 0, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2", pos );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cLineComment.CopyTo( 1, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO 追加
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO 追加
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cLineComment.CopyTo( 2, lbuf, pos );
		}
		else { // write
			//	Block Comment
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	,
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentFrom( 0 )), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	,
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentTo( 0 )), 0 );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2",
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentFrom( 1 )), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentTo(1)), 0 );

			//	Line Comment
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		,
			const_cast<char*>(m_pShareData->m_Types[i].m_cLineComment.getLineComment( 0 )), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		,
			const_cast<char*>(m_pShareData->m_Types[i].m_cLineComment.getLineComment( 1 )), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		,
			const_cast<char*>(m_pShareData->m_Types[i].m_cLineComment.getLineComment( 2 )), 0 );	//Jun. 01, 2001 JEPRO 追加

		//	From here May 12, 2001 genta
		int pos;
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, pos );
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 1 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2", pos );
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 2 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO 追加
		//	To here May 12, 2001 genta

		}
		// To Here Sep. 28, 2002 genta / YAZAKI

		cProfile.IOProfileData( bRead, pszSecName, "szIndentChars"		,
			m_pShareData->m_Types[i].m_szIndentChars,
			sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
		cProfile.IOProfileData( bRead, pszSecName, "cLineTermChar"		, m_pShareData->m_Types[i].m_cLineTermChar );

		cProfile.IOProfileData( bRead, pszSecName, "nDefaultOutline"	, m_pShareData->m_Types[i].m_nDefaultOutline );/* アウトライン解析方法 */
		cProfile.IOProfileData( bRead, pszSecName, "szOutlineRuleFilename"	,
			m_pShareData->m_Types[i].m_szOutlineRuleFilename,
			sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* アウトライン解析ルールファイル */
		cProfile.IOProfileData( bRead, pszSecName, "nSmartIndent"		, m_pShareData->m_Types[i].m_nSmartIndent );/* スマートインデント種別 */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "nImeState"			, m_pShareData->m_Types[i].m_nImeState );	//	IME制御

		//	2001/06/14 Start By asa-o: タイプ別の補完ファイルとキーワードヘルプ
		//	Oct. 5, 2002 genta sizeof()で誤ってポインタのサイズを取得していたのを修正
		cProfile.IOProfileData( bRead, pszSecName, "szHokanFile"		,
			m_pShareData->m_Types[i].m_szHokanFile,
			sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	補完ファイル
		cProfile.IOProfileData( bRead, pszSecName, "bUseKeyWordHelp"	, m_pShareData->m_Types[i].m_bUseKeyWordHelp );	//	キーワードヘルプを使用する
		cProfile.IOProfileData( bRead, pszSecName, "szKeyWordHelpFile"	, 
			m_pShareData->m_Types[i].m_szKeyWordHelpFile,
			sizeof( m_pShareData->m_Types[0].m_szKeyWordHelpFile ));	//	キーワードヘルプ 辞書ファイル
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( bRead, pszSecName, "bHokanLoHiCase"		, m_pShareData->m_Types[i].m_bHokanLoHiCase );

		//	2003.06.23 Moca ファイル内からの入力補完機能
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokanByFile"		, m_pShareData->m_Types[i].m_bUseHokanByFile );

		//@@@ 2002.2.4 YAZAKI
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp"			,
			m_pShareData->m_Types[i].m_szExtHelp,
			sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
			
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp"		,
			m_pShareData->m_Types[i].m_szExtHtmlHelp,
			sizeof( m_pShareData->m_Types[i].m_szExtHtmlHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"	, m_pShareData->m_Types[i].m_bHokanLoHiCase );

		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent"			, m_pShareData->m_Types[i].m_bAutoIndent );
		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent_ZENSPACE"	, m_pShareData->m_Types[i].m_bAutoIndent_ZENSPACE );
		cProfile.IOProfileData( bRead, pszSecName, "nIndentLayout"			, m_pShareData->m_Types[i].m_nIndentLayout );

		/* 色設定 I/O */
		IO_ColorSet( &cProfile, bRead, pszSecName, m_pShareData->m_Types[i].m_ColorInfoArr  );


//@@@ 2001.11.17 add start MIK
		{	//正規表現キーワード
			char	*p;
			cProfile.IOProfileData( bRead, pszSecName, "bUseRegexKeyword", m_pShareData->m_Types[i].m_bUseRegexKeyword );/* 正規表現キーワード使用するか？ */
			for(j = 0; j < 100; j++)
			{
				wsprintf( szKeyName, "RxKey[%03d]", j );
				if( bRead )
				{
					m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
					m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
					if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
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
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
				}
			}
		}
//@@@ 2001.11.17 add end MIK

		/* 禁則 */
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuHead"	, m_pShareData->m_Types[i].m_bKinsokuHead );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuTail"	, m_pShareData->m_Types[i].m_bKinsokuTail );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuRet"	, m_pShareData->m_Types[i].m_bKinsokuRet );	//@@@ 2002.04.13 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuKuto"	, m_pShareData->m_Types[i].m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuHead"	,
			m_pShareData->m_Types[i].m_szKinsokuHead,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuTail"	,
			m_pShareData->m_Types[i].m_szKinsokuTail,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
		cProfile.IOProfileData( bRead, pszSecName, "bUseDocumentIcon"	, m_pShareData->m_Types[i].m_bUseDocumentIcon );	// Sep. 19 ,2002 genta 変数名誤り修正

	}// Types ( for loop)

	/* 強調キーワード */
	{
		pszSecName = "KeyWords";
		int nKeyWordSetNum = m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum;
		bool bIOSuccess;
		cProfile.IOProfileData( bRead, pszSecName, "nCurrentKeyWordSetIdx"	, m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
		bIOSuccess = cProfile.IOProfileData( bRead, pszSecName, "nKeyWordSetNum"			, nKeyWordSetNum );
		if( bRead ){
			// nKeyWordSetNum が読み込めていれば、すべての情報がそろっていると仮定して処理を進める
			if( bIOSuccess ){
				// 2004.11.25 Moca キーワードセットの情報は、直接書き換えないで関数を利用する
				// 初期設定されているため、先に削除しないと固定メモリの確保に失敗する可能性がある
				int  nMemLen = MAX_KEYWORDNUM * ( MAX_KEYWORDLEN + 1 ) + 1;
				char *pszMem = new char[nMemLen];
				m_pShareData->m_CKeyWordSetMgr.ResetAllKeyWordSet();
				for( i = 0; i < nKeyWordSetNum; ++i ){
					int nKEYWORDCASE = 0;
					int nKeyWordNum = 0;
					wsprintf( szKeyName, "szSN[%02d]", i );
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
					wsprintf( szKeyName, "nCASE[%02d]", i );
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, nKEYWORDCASE );
					wsprintf( szKeyName, "nKWN[%02d]", i );
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, nKeyWordNum );
					m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( szKeyData, nKEYWORDCASE, nKeyWordNum );
					// 2004.11.25 Moca szKW[%02d].Size の情報は利用する意味がない。
//					wsprintf( szKeyName, "szKW[%02d].Size", i );
//					cProfile.IOProfileData( bRead, pszSecName, szKeyName, nMemLen );
					wsprintf( szKeyName, "szKW[%02d]", i );
					if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, pszMem, nMemLen ) ){
						m_pShareData->m_CKeyWordSetMgr.SetKeyWordArr( i, nKeyWordNum, pszMem );
					}
				}
				delete [] pszMem;
			}
		}else{
			for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
				wsprintf( szKeyName, "szSN[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[i],
					sizeof( m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[0] ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_CKeyWordSetMgr.m_nKEYWORDCASEArr[i] );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i] );

				int nMemLen = 0;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.GetKeyWord( i, j ) );
					nMemLen ++;
				}
				nMemLen ++;
				wsprintf( szKeyName, "szKW[%02d].Size", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nMemLen );
				char* pszMem = new char[nMemLen + 1];	//	May 25, 2003 genta 区切りをTABに変更したので，最後の\0の分を追加
				char* pMem = pszMem;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					//	May 25, 2003 genta 区切りをTABに変更
					int kwlen = strlen( m_pShareData->m_CKeyWordSetMgr.GetKeyWord( i, j ) );
					memcpy( pMem, m_pShareData->m_CKeyWordSetMgr.GetKeyWord( i, j ), kwlen );
					pMem += kwlen;
					*pMem++ = '\t';
				}
				*pMem = '\0';
				wsprintf( szKeyName, "szKW[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, pszMem, nMemLen );
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
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_MacroTable[i].m_szName, MACRONAME_MAX - 1 );
			wsprintf( szKeyName, "File[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_MacroTable[i].m_szFile, _MAX_PATH );
			wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_MacroTable[i].m_bReloadWhenExecute );
		}
	}// Macro
	//	To Here  Sep. 14, 2001 genta

//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
	{
		pszSecName = "Other";	//セクションを1個作成した。2003.05.12 MIK
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

	@param  pTagJump [in] 保存するタグジャンプ情報
	@return true : 保存成功 false : 保存失敗

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

	@param  pTagJump [out] 参照するタグジャンプ情報
	@return true : 参照成功 false : 参照失敗

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
