//2008.XX.XX kobake CShareDataから分離
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

#include "StdAfx.h"
#include "env/CShareData_IO.h"
#include "doc/CDocTypeSetting.h" // ColorInfo !!
#include "CShareData.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "plugin/CPlugin.h"
#include "uiparts/CMenuDrawer.h"
#include "_main/CCommandLine.h"

void ShareData_IO_Sub_LogFont( CDataProfile& cProfile, const WCHAR* pszSecName,
	const WCHAR* pszKeyLf, const WCHAR* pszKeyPointSize, const WCHAR* pszKeyFaceName, LOGFONT& lf, INT& nPointSize );

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

/* 共有データのロード */
bool CShareData_IO::LoadShareData()
{
	return ShareData_IO_2( true );
}

/* 共有データの保存 */
void CShareData_IO::SaveShareData()
{
	ShareData_IO_2( false );
}

/*!
	共有データの読み込み/保存 2

	@param[in] bRead true: 読み込み / false: 書き込み

	@date 2004-01-11 D.S.Koba CProfile変更によるコード簡略化
	@date 2005-04-05 D.S.Koba 各セクションの入出力を関数として分離
*/
bool CShareData_IO::ShareData_IO_2( bool bRead )
{
	//MY_RUNNINGTIMER( cRunningTimer, "CShareData_IO::ShareData_IO_2" );
	CShareData* pcShare = CShareData::getInstance();

	CDataProfile	cProfile;

	// Feb. 12, 2006 D.S.Koba
	if( bRead ){
		cProfile.SetReadingMode();
	} else {
		cProfile.SetWritingMode();
	}

	std::tstring strProfileName = to_tchar(CCommandLine::getInstance()->GetProfileName());
	TCHAR	szIniFileName[_MAX_PATH + 1];
	CFileNameManager::getInstance()->GetIniFileName( szIniFileName, strProfileName.c_str(), bRead );	// 2007.05.19 ryoji iniファイル名を取得する

//	MYTRACE( _T("Iniファイル処理-1 所要時間(ミリ秒) = %d\n"), cRunningTimer.Read() );


	if( bRead ){
		if( !cProfile.ReadProfile( szIniFileName ) ){
			/* 設定ファイルが存在しない */
			return false;
		}

		// バージョンアップ時はバックアップファイルを作成する	// 2011.01.28 ryoji
		TCHAR iniVer[256];
		DWORD mH, mL, lH, lL;
		mH = mL = lH = lL = 0;	// ※ 古～い ini だと "szVersion" は無い
		if( cProfile.IOProfileData( LTEXT("Other"), LTEXT("szVersion"), MakeStringBufferT(iniVer) ) )
			_stscanf( iniVer, _T("%u.%u.%u.%u"), &mH, &mL, &lH, &lL );
		DWORD dwMS = (DWORD)MAKELONG(mL, mH);
		DWORD dwLS = (DWORD)MAKELONG(lL, lH);
		DLLSHAREDATA* pShareData = &GetDllShareData();
		if( pShareData->m_sVersion.m_dwProductVersionMS > dwMS
			|| (pShareData->m_sVersion.m_dwProductVersionMS == dwMS && pShareData->m_sVersion.m_dwProductVersionLS > dwLS) )
		{
			TCHAR szBkFileName[_countof(szIniFileName) + 4];
			::lstrcpy(szBkFileName, szIniFileName);
			::lstrcat(szBkFileName, _T(".bak"));
			::CopyFile(szIniFileName, szBkFileName, FALSE);
		}
	}
//	MYTRACE( _T("Iniファイル処理 0 所要時間(ミリ秒) = %d\n"), cRunningTimer.Read() );

	CMenuDrawer* pcMenuDrawer = new CMenuDrawer; // 2010/7/4 Uchi

	if( bRead ){
		DLLSHAREDATA* pShareData = &GetDllShareData();
		cProfile.IOProfileData( L"Common", L"szLanguageDll", MakeStringBufferT( pShareData->m_Common.m_sWindow.m_szLanguageDll ) );
		CSelectLang::ChangeLang( pShareData->m_Common.m_sWindow.m_szLanguageDll );
		pcShare->RefreshString();
	}

	// Feb. 12, 2006 D.S.Koba
	ShareData_IO_Mru( cProfile );
	ShareData_IO_Keys( cProfile );
	ShareData_IO_Grep( cProfile );
	ShareData_IO_Folders( cProfile );
	ShareData_IO_Cmd( cProfile );
	ShareData_IO_Nickname( cProfile );
	ShareData_IO_Common( cProfile );
	ShareData_IO_Plugin( cProfile, pcMenuDrawer );		// Move here	2010/6/24 Uchi
	ShareData_IO_Toolbar( cProfile, pcMenuDrawer );
	ShareData_IO_CustMenu( cProfile );
	ShareData_IO_Font( cProfile );
	ShareData_IO_KeyBind( cProfile );
	ShareData_IO_Print( cProfile );
	ShareData_IO_Types( cProfile );
	ShareData_IO_KeyWords( cProfile );
	ShareData_IO_Macro( cProfile );
	ShareData_IO_Statusbar( cProfile );		// 2008/6/21 Uchi
	ShareData_IO_MainMenu( cProfile );		// 2010/5/15 Uchi
	ShareData_IO_Other( cProfile );

	delete pcMenuDrawer;					// 2010/7/4 Uchi
	pcMenuDrawer = NULL;

	if( !bRead ){
		// 2014.12.08 sakura.iniの読み取り専用
		if( !GetDllShareData().m_Common.m_sOthers.m_bIniReadOnly ){
			cProfile.WriteProfile( szIniFileName, LTEXT(" sakura.ini テキストエディタ設定ファイル") );
		}
	}

//	MYTRACE( _T("Iniファイル処理 8 所要時間(ミリ秒) = %d\n"), cRunningTimer.Read() );
//	MYTRACE( _T("Iniファイル処理 所要時間(ミリ秒) = %d\n"), cRunningTimerStart.Read() );

	return true;
}

/*!
	@brief 共有データのMruセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData_IO::ShareData_IO_Mru( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("MRU");
	int			i;
	int			nSize;
	EditInfo*	pfiWork;
	WCHAR		szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("_MRU_Counts"), pShare->m_sHistory.m_nMRUArrNum );
	SetValueLimit( pShare->m_sHistory.m_nMRUArrNum, MAX_MRU );
	nSize = pShare->m_sHistory.m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &pShare->m_sHistory.m_fiMRUArr[i];
		if( cProfile.IsReadingMode() ){
			pfiWork->m_nTypeId = -1;
		}
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
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark2"), i );
		if( !cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pfiWork->m_szMarkLines) ) ){
			if( cProfile.IsReadingMode() ){
				auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark"), i ); // 旧ver互換
				cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pfiWork->m_szMarkLines) );
			}
		}
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nType"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nTypeId );
		//お気に入り	//@@@ 2003.04.08 MIK
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].bFavorite"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sHistory.m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
	if ( cProfile.IsReadingMode() ){
		EditInfo	fiInit;
		//	残りをfiInitで初期化しておく。
		fiInit.m_nCharCode = CODE_DEFAULT;
		fiInit.m_nViewLeftCol = CLayoutInt(0);
		fiInit.m_nViewTopLine = CLayoutInt(0);
		fiInit.m_ptCursor.Set(CLogicInt(0), CLogicInt(0));
		fiInit.m_szPath[0] = _T('\0');
		fiInit.m_szMarkLines[0] = L'\0';	// 2002.01.16 hor
		for( ; i < MAX_MRU; ++i){
			pShare->m_sHistory.m_fiMRUArr[i] = fiInit;
			pShare->m_sHistory.m_bMRUArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}

	cProfile.IOProfileData( pszSecName, LTEXT("_MRUFOLDER_Counts"), pShare->m_sHistory.m_nOPENFOLDERArrNum );
	SetValueLimit( pShare->m_sHistory.m_nOPENFOLDERArrNum, MAX_OPENFOLDER );
	nSize = pShare->m_sHistory.m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("MRUFOLDER[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sHistory.m_szOPENFOLDERArr[i] );
		//お気に入り	//@@@ 2003.04.08 MIK
		wcscat( szKeyName, LTEXT(".bFavorite") );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sHistory.m_bOPENFOLDERArrFavorite[i] );
	}
	//読み込み時は残りを初期化
	if ( cProfile.IsReadingMode() ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			pShare->m_sHistory.m_szOPENFOLDERArr[i][0] = L'\0';
			pShare->m_sHistory.m_bOPENFOLDERArrFavorite[i] = false;	//お気に入り	//@@@ 2003.04.08 MIK
		}
	}
	
	cProfile.IOProfileData( pszSecName, LTEXT("_ExceptMRU_Counts"), pShare->m_sHistory.m_aExceptMRU._GetSizeRef() );
	pShare->m_sHistory.m_aExceptMRU.SetSizeLimit();
	nSize = pShare->m_sHistory.m_aExceptMRU.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("ExceptMRU[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sHistory.m_aExceptMRU[i] );
	}
}

/*!
	@brief 共有データのKeysセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData_IO::ShareData_IO_Keys( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Keys");
	int		i;
	int		nSize;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("_SEARCHKEY_Counts"), pShare->m_sSearchKeywords.m_aSearchKeys._GetSizeRef() );
	pShare->m_sSearchKeywords.m_aSearchKeys.SetSizeLimit();
	nSize = pShare->m_sSearchKeywords.m_aSearchKeys.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("SEARCHKEY[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sSearchKeywords.m_aSearchKeys[i] );
	}

	cProfile.IOProfileData( pszSecName, LTEXT("_REPLACEKEY_Counts"), pShare->m_sSearchKeywords.m_aReplaceKeys._GetSizeRef() );
	pShare->m_sSearchKeywords.m_aReplaceKeys.SetSizeLimit();
	nSize = pShare->m_sSearchKeywords.m_aReplaceKeys.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("REPLACEKEY[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sSearchKeywords.m_aReplaceKeys[i] );
	}
}

/*!
	@brief 共有データのGrepセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData_IO::ShareData_IO_Grep( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Grep");
	int		i;
	int		nSize;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("_GREPFILE_Counts"), pShare->m_sSearchKeywords.m_aGrepFiles._GetSizeRef() );
	pShare->m_sSearchKeywords.m_aGrepFiles.SetSizeLimit();
	nSize = pShare->m_sSearchKeywords.m_aGrepFiles.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("GREPFILE[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sSearchKeywords.m_aGrepFiles[i] );
	}

	cProfile.IOProfileData( pszSecName, LTEXT("_GREPFOLDER_Counts"), pShare->m_sSearchKeywords.m_aGrepFolders._GetSizeRef() );
	pShare->m_sSearchKeywords.m_aGrepFolders.SetSizeLimit();
	nSize = pShare->m_sSearchKeywords.m_aGrepFolders.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("GREPFOLDER[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sSearchKeywords.m_aGrepFolders[i] );
	}
}

/*!
	@brief 共有データのFoldersセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData_IO::ShareData_IO_Folders( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Folders");
	/* マクロ用フォルダ */
	cProfile.IOProfileData( pszSecName, LTEXT("szMACROFOLDER"), pShare->m_Common.m_sMacro.m_szMACROFOLDER );
	/* 設定インポート用フォルダ */
	cProfile.IOProfileData( pszSecName, LTEXT("szIMPORTFOLDER"), pShare->m_sHistory.m_szIMPORTFOLDER );
}

/*!
	@brief 共有データのCmdセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData_IO::ShareData_IO_Cmd( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Cmd");
	int		i;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("nCmdArrNum"), pShare->m_sHistory.m_aCommands._GetSizeRef() );
	pShare->m_sHistory.m_aCommands.SetSizeLimit();
	int nSize = pShare->m_sHistory.m_aCommands.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("szCmdArr[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sHistory.m_aCommands[i] );
	}

	cProfile.IOProfileData( pszSecName, LTEXT("nCurDirArrNum"), pShare->m_sHistory.m_aCurDirs._GetSizeRef() );
	pShare->m_sHistory.m_aCurDirs.SetSizeLimit();
	nSize = pShare->m_sHistory.m_aCurDirs.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("szCurDirArr[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sHistory.m_aCurDirs[i] );
	}
}

/*!
	@brief 共有データのNicknameセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData_IO::ShareData_IO_Nickname( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Nickname");
	int		i;
	WCHAR	szKeyName[64];

	cProfile.IOProfileData( pszSecName, LTEXT("bShortPath"), pShare->m_Common.m_sFileName.m_bTransformShortPath );
	cProfile.IOProfileData( pszSecName, LTEXT("nShortPathMaxWidth"), pShare->m_Common.m_sFileName.m_nTransformShortMaxWidth );
	cProfile.IOProfileData( pszSecName, LTEXT("ArrNum"), pShare->m_Common.m_sFileName.m_nTransformFileNameArrNum );
	SetValueLimit( pShare->m_Common.m_sFileName.m_nTransformFileNameArrNum, MAX_TRANSFORM_FILENAME );
	int nSize = pShare->m_Common.m_sFileName.m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("From%02d"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(pShare->m_Common.m_sFileName.m_szTransformFileNameFrom[i]) );
		auto_sprintf( szKeyName, LTEXT("To%02d"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(pShare->m_Common.m_sFileName.m_szTransformFileNameTo[i]) );
	}
	// 読み込み時，残りをNULLで再初期化
	if( cProfile.IsReadingMode() ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			pShare->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] = L'\0';
			pShare->m_Common.m_sFileName.m_szTransformFileNameTo[i][0]   = L'\0';
		}
	}
}

static bool ShareData_IO_RECT( CDataProfile& cProfile, const WCHAR* pszSecName, const WCHAR* pszKeyName, RECT& rcValue )
{
	const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
	WCHAR		szKeyData[100];
	bool		ret = false;
	if( cProfile.IsReadingMode() ){
		ret = cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData) );
		if( ret ){
			int buf[4];
			scan_ints( szKeyData, pszForm, buf );
			rcValue.left	= buf[0];
			rcValue.top		= buf[1];
			rcValue.right	= buf[2];
			rcValue.bottom	= buf[3];
		}
	}else{
		auto_sprintf(
			szKeyData,
			pszForm,
			rcValue.left,
			rcValue.top,
			rcValue.right,
			rcValue.bottom
		);
		ret = cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData) );
	}
	return ret;
}

/*!
	@brief 共有データのCommonセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData_IO::ShareData_IO_Common( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Common");
	// 2005.04.07 D.S.Koba
	CommonSetting& common = pShare->m_Common;

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
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("eOpenDialogDir")		, common.m_sEdit.m_eOpenDialogDir );
	cProfile.IOProfileData( pszSecName, LTEXT("szOpenDialogSelDir")		, StringBufferT(common.m_sEdit.m_OpenDialogSelDir,_countof2(common.m_sEdit.m_OpenDialogSelDir)) );
	cProfile.IOProfileData( pszSecName, LTEXT("bBoxSelectLock")	, common.m_sEdit.m_bBoxSelectLock );
	cProfile.IOProfileData( pszSecName, LTEXT("nRepeatedScrollLineNum")	, common.m_sGeneral.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( pszSecName, LTEXT("nRepeatedMoveCaretNum")	, common.m_sGeneral.m_nRepeatedMoveCaretNum );
	cProfile.IOProfileData( pszSecName, LTEXT("nRepeatedScroll_Smooth")	, common.m_sGeneral.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( pszSecName, LTEXT("nPageScrollByWheel")	, common.m_sGeneral.m_nPageScrollByWheel );					// 2009.01.17 nasukoji
	cProfile.IOProfileData( pszSecName, LTEXT("nHorizontalScrollByWheel")	, common.m_sGeneral.m_nHorizontalScrollByWheel );	// 2009.01.17 nasukoji
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
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepOutputLine")		, common.m_sSearch.m_nGrepOutputLineType );
	cProfile.IOProfileData( pszSecName, LTEXT("nGrepOutputStyle")		, common.m_sSearch.m_nGrepOutputStyle );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepOutputFileOnly")	, common.m_sSearch.m_bGrepOutputFileOnly );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepOutputBaseFolder")	, common.m_sSearch.m_bGrepOutputBaseFolder );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepSeparateFolder")	, common.m_sSearch.m_bGrepSeparateFolder );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepDefaultFolder")		, common.m_sSearch.m_bGrepDefaultFolder );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepBackup")			, common.m_sSearch.m_bGrepBackup );
	
	// 2002/09/21 Moca 追加
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nGrepCharSet")	, common.m_sSearch.m_nGrepCharSet );
	cProfile.IOProfileData( pszSecName, LTEXT("bGrepRealTime")			, common.m_sSearch.m_bGrepRealTimeView ); // 2003.06.16 Moca
	cProfile.IOProfileData( pszSecName, LTEXT("bCaretTextForSearch")	, common.m_sSearch.m_bCaretTextForSearch );	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	cProfile.IOProfileData( pszSecName, LTEXT("m_bInheritKeyOtherView")	, common.m_sSearch.m_bInheritKeyOtherView );
	cProfile.IOProfileData( pszSecName, LTEXT("nTagJumpMode")			, common.m_sSearch.m_nTagJumpMode );
	cProfile.IOProfileData( pszSecName, LTEXT("nTagJumpModeKeyword")	, common.m_sSearch.m_nTagJumpModeKeyword );
	
	/* 正規表現DLL 2007.08.12 genta */
	cProfile.IOProfileData( pszSecName, LTEXT("szRegexpLib")			, MakeStringBufferT(common.m_sSearch.m_szRegexpLib) );
	cProfile.IOProfileData( pszSecName, LTEXT("bGTJW_RETURN")			, common.m_sSearch.m_bGTJW_RETURN );
	cProfile.IOProfileData( pszSecName, LTEXT("bGTJW_LDBLCLK")			, common.m_sSearch.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUp")				, common.m_sBackup.m_bBackUp );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpDialog")			, common.m_sBackup.m_bBackUpDialog );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpFolder")			, common.m_sBackup.m_bBackUpFolder );
	cProfile.IOProfileData( pszSecName, LTEXT("bBackUpFolderRM")		, common.m_sBackup.m_bBackUpFolderRM );	// 2010/5/27 Uchi
	
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
		ShareData_IO_Sub_LogFont( cProfile, pszSecName, L"khlf", L"khps", L"khlfFaceName",
			common.m_sHelper.m_lf, common.m_sHelper.m_nPointSize );
	}// Keword Help Font
	
	
	cProfile.IOProfileData( pszSecName, LTEXT("nMRUArrNum_MAX")			, common.m_sGeneral.m_nMRUArrNum_MAX );
	SetValueLimit( common.m_sGeneral.m_nMRUArrNum_MAX, MAX_MRU );
	cProfile.IOProfileData( pszSecName, LTEXT("nOPENFOLDERArrNum_MAX")	, common.m_sGeneral.m_nOPENFOLDERArrNum_MAX );
	SetValueLimit( common.m_sGeneral.m_nOPENFOLDERArrNum_MAX, MAX_OPENFOLDER );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTOOLBAR")			, common.m_sWindow.m_bDispTOOLBAR );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispSTATUSBAR")			, common.m_sWindow.m_bDispSTATUSBAR );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispFUNCKEYWND")		, common.m_sWindow.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( pszSecName, LTEXT("bDispMiniMap")			, common.m_sWindow.m_bDispMiniMap );
	cProfile.IOProfileData( pszSecName, LTEXT("nFUNCKEYWND_Place")		, common.m_sWindow.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( pszSecName, LTEXT("nFUNCKEYWND_GroupNum")	, common.m_sWindow.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca ファンクションキーのグループボタン数
	cProfile.IOProfileData( pszSecName, LTEXT("szLanguageDll")			, MakeStringBufferT( common.m_sWindow.m_szLanguageDll ) );
	cProfile.IOProfileData( pszSecName, LTEXT("nMiniMapFontSize")		, common.m_sWindow.m_nMiniMapFontSize );
	cProfile.IOProfileData( pszSecName, LTEXT("nMiniMapQuality")		, common.m_sWindow.m_nMiniMapQuality );
	cProfile.IOProfileData( pszSecName, LTEXT("nMiniMapWidth")			, common.m_sWindow.m_nMiniMapWidth );
	
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTabWnd")			, common.m_sTabBar.m_bDispTabWnd );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTabWndMultiWin")	, common.m_sTabBar.m_bDispTabWndMultiWin );	//タブウインドウ	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("szTabWndCaption")		, MakeStringBufferW(common.m_sTabBar.m_szTabWndCaption) );	//@@@ 2003.06.13 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bSameTabWidth")			, common.m_sTabBar.m_bSameTabWidth );	// 2006.01.28 ryoji タブを等幅にする
	cProfile.IOProfileData( pszSecName, LTEXT("bDispTabIcon")			, common.m_sTabBar.m_bDispTabIcon );	// 2006.01.28 ryoji タブにアイコンを表示する
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("bDispTabClose")	, common.m_sTabBar.m_bDispTabClose );	// 2012.04.14 syat
	cProfile.IOProfileData( pszSecName, LTEXT("bSortTabList")			, common.m_sTabBar.m_bSortTabList );	// 2006.05.10 ryoji タブ一覧をソートする
	cProfile.IOProfileData( pszSecName, LTEXT("bTab_RetainEmptyWin")	, common.m_sTabBar.m_bTab_RetainEmptyWin );	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, LTEXT("bTab_CloseOneWin")	, common.m_sTabBar.m_bTab_CloseOneWin );	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
	cProfile.IOProfileData( pszSecName, LTEXT("bTab_ListFull")			, common.m_sTabBar.m_bTab_ListFull );	// タブ一覧をフルパス表示する	// 2007.02.28 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bChgWndByWheel")		, common.m_sTabBar.m_bChgWndByWheel );	// 2006.03.26 ryoji マウスホイールでウィンドウ切り替え
	cProfile.IOProfileData( pszSecName, LTEXT("bNewWindow")			, common.m_sTabBar.m_bNewWindow );	// 外部から起動するときは新しいウインドウで開く
	cProfile.IOProfileData( pszSecName, L"bTabMultiLine"			, common.m_sTabBar.m_bTabMultiLine );	// タブ多段
	cProfile.IOProfileData_WrapInt( pszSecName, L"eTabPosition"		, common.m_sTabBar.m_eTabPosition );	// タブ位置

	ShareData_IO_Sub_LogFont( cProfile, pszSecName, L"lfTabFont", L"lfTabFontPs", L"lfTabFaceName",
		common.m_sTabBar.m_lf, common.m_sTabBar.m_nPointSize );
	
	cProfile.IOProfileData( pszSecName, LTEXT("nTabMaxWidth")			, common.m_sTabBar.m_nTabMaxWidth );
	cProfile.IOProfileData( pszSecName, LTEXT("nTabMinWidth")			, common.m_sTabBar.m_nTabMinWidth );
	cProfile.IOProfileData( pszSecName, LTEXT("nTabMinWidthOnMulti")	, common.m_sTabBar.m_nTabMinWidthOnMulti );

	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	cProfile.IOProfileData( pszSecName, LTEXT("bSplitterWndHScroll")	, common.m_sWindow.m_bSplitterWndHScroll );
	cProfile.IOProfileData( pszSecName, LTEXT("bSplitterWndVScroll")	, common.m_sWindow.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( pszSecName, LTEXT("szMidashiKigou")		, MakeStringBufferW(common.m_sFormat.m_szMidashiKigou) );
	cProfile.IOProfileData( pszSecName, LTEXT("szInyouKigou")			, MakeStringBufferW(common.m_sFormat.m_szInyouKigou) );
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("bSaveWindowSize"), common.m_sWindow.m_eSaveWindowSize );	//#####フラグ名が激しくきもい
	cProfile.IOProfileData( pszSecName, LTEXT("nWinSizeType")			, common.m_sWindow.m_nWinSizeType );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinSizeCX")				, common.m_sWindow.m_nWinSizeCX );
	cProfile.IOProfileData( pszSecName, LTEXT("nWinSizeCY")				, common.m_sWindow.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*を追加
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nSaveWindowPos")	, common.m_sWindow.m_eSaveWindowPos );	//#####フラグ名がきもい
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
	cProfile.IOProfileData( pszSecName, LTEXT("bUseCompotibleBMP")			, common.m_sWindow.m_bUseCompatibleBMP ); // 2007.09.09 Moca
	cProfile.IOProfileData( pszSecName, LTEXT("bCopyAndDisablSelection")	, common.m_sEdit.m_bCopyAndDisablSelection );/* コピーしたら選択解除 */
	cProfile.IOProfileData( pszSecName, LTEXT("bEnableNoSelectCopy")		, common.m_sEdit.m_bEnableNoSelectCopy );/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bEnableLineModePaste")		, common.m_sEdit.m_bEnableLineModePaste );/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bConvertEOLPaste")			, common.m_sEdit.m_bConvertEOLPaste );	/* 改行コードを変換して貼り付ける */	// 2009.02.28 salarm
	cProfile.IOProfileData( pszSecName, LTEXT("bEnableExtEol")				, common.m_sEdit.m_bEnableExtEol );
	
	cProfile.IOProfileData( pszSecName, LTEXT("bHtmlHelpIsSingle")			, common.m_sHelper.m_bHtmlHelpIsSingle );/* HtmlHelpビューアはひとつ */
	cProfile.IOProfileData( pszSecName, LTEXT("bCompareAndTileHorz")		, common.m_sCompare.m_bCompareAndTileHorz );/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	cProfile.IOProfileData( pszSecName, LTEXT("bDropFileAndClose")			, common.m_sFile.m_bDropFileAndClose );/* ファイルをドロップしたときは閉じて開く */
	cProfile.IOProfileData( pszSecName, LTEXT("nDropFileNumMax")			, common.m_sFile.m_nDropFileNumMax );/* 一度にドロップ可能なファイル数 */
	cProfile.IOProfileData( pszSecName, LTEXT("bCheckFileTimeStamp")		, common.m_sFile.m_bCheckFileTimeStamp );/* 更新の監視 */
	cProfile.IOProfileData( pszSecName, LTEXT("nAutoloadDelay")				, common.m_sFile.m_nAutoloadDelay );/* 自動読込時遅延 */
	cProfile.IOProfileData( pszSecName, LTEXT("bUneditableIfUnwritable")	, common.m_sFile.m_bUneditableIfUnwritable );/* 上書き禁止検出時は編集禁止にする */
	cProfile.IOProfileData( pszSecName, LTEXT("bNotOverWriteCRLF")			, common.m_sEdit.m_bNotOverWriteCRLF );/* 改行は上書きしない */
	cProfile.IOProfileData( pszSecName, LTEXT("bOverWriteFixMode")			, common.m_sEdit.m_bOverWriteFixMode );// 文字幅に合わせてスペースを詰める
	cProfile.IOProfileData( pszSecName, LTEXT("bOverWriteBoxDelete")		, common.m_sEdit.m_bOverWriteBoxDelete );
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoCloseDlgFind")			, common.m_sSearch.m_bAutoCloseDlgFind );/* 検索ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoCloseDlgFuncList")		, common.m_sOutline.m_bAutoCloseDlgFuncList );/* アウトライン ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoCloseDlgReplace")		, common.m_sSearch.m_bAutoCloseDlgReplace );/* 置換 ダイアログを自動的に閉じる */
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoColmnPaste")			, common.m_sEdit.m_bAutoColumnPaste );/* 矩形コピーのテキストは常に矩形貼り付け */ // 2013.5.23 aroka iniファイルのtypo未修正
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
	cProfile.IOProfileData( pszSecName, LTEXT("bAlertIfLargeFile")			, common.m_sFile.m_bAlertIfLargeFile );	// 開こうとしたファイルが大きい場合に警告する
	cProfile.IOProfileData( pszSecName, LTEXT("nAlertFileSize")				, common.m_sFile.m_nAlertFileSize );	// 警告を開始するファイルサイズ(MB単位)
	
	/* 「開く」ダイアログのサイズと位置 */
	ShareData_IO_RECT( cProfile,  pszSecName, LTEXT("rcOpenDialog"), common.m_sOthers.m_rcOpenDialog );
	ShareData_IO_RECT( cProfile,  pszSecName, LTEXT("rcCompareDialog"), common.m_sOthers.m_rcCompareDialog );
	ShareData_IO_RECT( cProfile,  pszSecName, LTEXT("rcDiffDialog"), common.m_sOthers.m_rcDiffDialog );
	ShareData_IO_RECT( cProfile,  pszSecName, LTEXT("rcFavoriteDialog"), common.m_sOthers.m_rcFavoriteDialog );
	ShareData_IO_RECT( cProfile,  pszSecName, LTEXT("rcTagJumpDialog"), common.m_sOthers.m_rcTagJumpDialog );
	ShareData_IO_RECT( cProfile,  pszSecName, LTEXT("rcWindowListDialog"), common.m_sOthers.m_rcWindowListDialog );
	
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
	cProfile.IOProfileData( pszSecName, LTEXT("nOutlineDockSet"), common.m_sOutline.m_nOutlineDockSet );
	cProfile.IOProfileData( pszSecName, LTEXT("bOutlineDockSync"), common.m_sOutline.m_bOutlineDockSync );
	cProfile.IOProfileData( pszSecName, LTEXT("bOutlineDockDisp"), common.m_sOutline.m_bOutlineDockDisp );
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("eOutlineDockSide"), common.m_sOutline.m_eOutlineDockSide );
	{
		const WCHAR* pszKeyName = LTEXT("xyOutlineDock");
		const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
		WCHAR		szKeyData[1024];
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData)) ){
				int buf[4];
				scan_ints( szKeyData, pszForm, buf );
				common.m_sOutline.m_cxOutlineDockLeft	= buf[0];
				common.m_sOutline.m_cyOutlineDockTop	= buf[1];
				common.m_sOutline.m_cxOutlineDockRight	= buf[2];
				common.m_sOutline.m_cyOutlineDockBottom	= buf[3];
			}
		}else{
			auto_sprintf(
				szKeyData,
				pszForm,
				common.m_sOutline.m_cxOutlineDockLeft,
				common.m_sOutline.m_cyOutlineDockTop,
				common.m_sOutline.m_cxOutlineDockRight,
				common.m_sOutline.m_cyOutlineDockBottom
			);
			cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData) );
		}
	}
	cProfile.IOProfileData( pszSecName, LTEXT("nDockOutline"), common.m_sOutline.m_nDockOutline );
	ShareData_IO_FileTree( cProfile, common.m_sOutline.m_sFileTree, pszSecName );
	cProfile.IOProfileData( pszSecName, LTEXT("szFileTreeDefIniName"), common.m_sOutline.m_sFileTreeDefIniName );
}


// プラグインコマンドを名前から機能番号へ変換
EFunctionCode GetPlugCmdInfoByName(
	const WCHAR*	pszFuncName			//!< [in]  プラグインコマンド名
)
{
	CommonSetting_Plugin& plugin = GetDllShareData().m_Common.m_sPlugin;
	WCHAR		sPluginName[MAX_PLUGIN_ID+1];
	const WCHAR* psCmdName;
	size_t		nLen;
	int			i;
	int			nId;
	int			nNo;

	if (pszFuncName == NULL) {
		return F_INVALID;
	}
	if ((psCmdName = wcschr(pszFuncName, L'/')) == NULL) {
		return F_INVALID;
	}
	nLen = MAX_PLUGIN_ID < (psCmdName - pszFuncName) ? MAX_PLUGIN_ID : (psCmdName - pszFuncName);
	wcsncpy( sPluginName, pszFuncName, nLen);
	sPluginName[nLen] = L'\0'; 
	psCmdName++;

	nId = -1;
	for (i = 0; i < MAX_PLUGIN; i++) {
		PluginRec& pluginrec = plugin.m_PluginTable[i];
		if (auto_strcmp( pluginrec.m_szId, sPluginName ) == 0) {
			nId = i;
			break;
		}
	}
	nNo = _wtoi( psCmdName );

	if (nId < 0 || nNo <= 0 || nNo >= MAX_PLUG_CMD) {
		// プラグインがない/番号がおかしい
		return F_INVALID;
	}
	
	return CPlug::GetPluginFunctionCode( nId, nNo );
}

// プラグインコマンドを機能番号から名前へ変換
bool GetPlugCmdInfoByFuncCode(
	EFunctionCode	eFuncCode,				//!< [in]  機能コード
	WCHAR*			pszFuncName				//!< [out] 機能名．この先にはMAX_PLUGIN_ID + 20文字のメモリが必要．
)
{
	CommonSetting_Plugin& plugin = GetDllShareData().m_Common.m_sPlugin;

	if (eFuncCode < F_PLUGCOMMAND_FIRST || eFuncCode > F_PLUGCOMMAND_LAST) {
		return false;
	}

	PluginId nID = CPlug::GetPluginId( eFuncCode );
	PlugId nNo = CPlug::GetPlugId( eFuncCode );
	if (nID < 0 || nNo < 0) {
		return false;
	}
	auto_sprintf(pszFuncName, L"%ls/%02d", plugin.m_PluginTable[nID].m_szId, nNo);
	return true;
}


/*! プラグイン名or機能番号文字列をEFunctionCodeにする

	@param[in]	pszFuncName		プラグイン名or機能番号文字列
	@return 機能コード
*/
static EFunctionCode GetFunctionStrToFunctionCode(const WCHAR* pszFuncName)
{
	EFunctionCode n;
	if (pszFuncName == NULL) {
		n = F_DEFAULT;
	}else if (wcschr(pszFuncName, L'/') != NULL) {
		// Plugin名
		n = GetPlugCmdInfoByName(pszFuncName);
	}else if (WCODE::Is09(pszFuncName[0]) 
	  && (pszFuncName[1] == L'\0' || WCODE::Is09(pszFuncName[1]))) {
		n = (EFunctionCode)auto_atol(pszFuncName);
	}else {
		n = CSMacroMgr::GetFuncInfoByName(0, pszFuncName, NULL);
	}
	if (n == F_INVALID) {
		n = F_DEFAULT;
	}
	return n;
}

/*!
	@brief 共有データのToolbarセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void CShareData_IO::ShareData_IO_Toolbar( CDataProfile& cProfile, CMenuDrawer* pcMenuDrawer )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Toolbar");
	int		i;
	WCHAR	szKeyName[64];
	CommonSetting_ToolBar& toolbar = pShare->m_Common.m_sToolBar;

	EFunctionCode	eFunc;
	WCHAR			szText[MAX_PLUGIN_ID+20];
	int				nInvalid = -1;

	cProfile.IOProfileData( pszSecName, LTEXT("bToolBarIsFlat"), toolbar.m_bToolBarIsFlat );

	cProfile.IOProfileData( pszSecName, LTEXT("nToolBarButtonNum"), toolbar.m_nToolBarButtonNum );
	SetValueLimit( toolbar.m_nToolBarButtonNum, MAX_TOOLBAR_BUTTON_ITEMS );
	int	nSize = toolbar.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("nTBB[%03d]"), i );
		// Plugin String Parametor
		if( cProfile.IsReadingMode() ){
			//読み込み
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szText) );
			if (wcschr(szText, L'/') == NULL) {
				// 番号
				toolbar.m_nToolBarButtonIdxArr[i] = _wtoi( szText );
			}
			else {
				// Plugin
				eFunc = GetPlugCmdInfoByName( szText );
				if ( eFunc == F_INVALID ) {
					toolbar.m_nToolBarButtonIdxArr[i] = -1;		// 未解決
				}
				else {
					toolbar.m_nToolBarButtonIdxArr[i] = pcMenuDrawer->FindToolbarNoFromCommandId( eFunc, false );
				}
			}
		}
		else {
			//書き込み
			if (toolbar.m_nToolBarButtonIdxArr[i] <= MAX_TOOLBAR_ICON_COUNT + 1) {	// +1はセパレータ分
				cProfile.IOProfileData( pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i] );	
			}
			else {
				// Plugin
				eFunc = (EFunctionCode)toolbar.m_nToolBarButtonIdxArr[i];
				if (eFunc == F_DEFAULT) {
					cProfile.IOProfileData( pszSecName, szKeyName, nInvalid );	
				}
				else if (GetPlugCmdInfoByFuncCode( eFunc, szText )) {
					cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szText) );	
				}
				else {
					cProfile.IOProfileData( pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i] );	
				}
			}
		}
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

	@date 2010.08.21 Moca 旧ShareData_IO_CustMenuをIO_CustMenuに変更
*/
void CShareData_IO::ShareData_IO_CustMenu( CDataProfile& cProfile )
{
	IO_CustMenu( cProfile, GetDllShareData().m_Common.m_sCustomMenu, false );
}

/*!
	@brief CustMenuの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス
	@param[in,out]	menu	入出力対象
	@param	bOutCmdName	出力時にマクロ名で出力

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData_IO::IO_CustMenu( CDataProfile& cProfile, CommonSetting_CustomMenu& menu, bool bOutCmdName)
{

	const WCHAR* pszSecName = LTEXT("CustMenu");
	int		i, j;
	WCHAR	szKeyName[64];
	wchar_t	szFuncName[1024];
	EFunctionCode n;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		auto_sprintf( szKeyName, LTEXT("szCMN[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(menu.m_szCustMenuNameArr[i]) );	//	Oct. 15, 2001 genta 最大長指定
		auto_sprintf( szKeyName, LTEXT("bCMPOP[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_bCustMenuPopupArr[i] );
		auto_sprintf( szKeyName, LTEXT("nCMIN[%02d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, menu.m_nCustMenuItemNumArr[i] );
		SetValueLimit( menu.m_nCustMenuItemNumArr[i], _countof(menu.m_nCustMenuItemFuncArr[0]) );
		int nSize = menu.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			// start マクロ名でも設定できるように 2008/5/24 Uchi
			auto_sprintf( szKeyName, LTEXT("nCMIF[%02d][%02d]"), i, j );
			if (cProfile.IsReadingMode()) {
				cProfile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szFuncName));
				n = GetFunctionStrToFunctionCode(szFuncName);
				menu.m_nCustMenuItemFuncArr[i][j] = n;
			}
			else {
				if (GetPlugCmdInfoByFuncCode( menu.m_nCustMenuItemFuncArr[i][j], szFuncName)) {
					// Plugin
					cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szFuncName) );
				}
				else {
					if (bOutCmdName) {
						WCHAR	*p = CSMacroMgr::GetFuncInfoByID(
							G_AppInstance(),
							menu.m_nCustMenuItemFuncArr[i][j],
							szFuncName,
							NULL
						);
						if ( p == NULL ) {
							auto_sprintf( szFuncName, L"%d", menu.m_nCustMenuItemFuncArr[i][j] );
						}
						cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szFuncName) );
					}
					else {
						cProfile.IOProfileData_WrapInt( pszSecName, szKeyName, menu.m_nCustMenuItemFuncArr[i][j] );
					}
				}
			}
			// end

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
void CShareData_IO::ShareData_IO_Font( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Font");
	CommonSetting_View& view = pShare->m_Common.m_sView;
	ShareData_IO_Sub_LogFont( cProfile, pszSecName, L"lf", L"nPointSize", L"lfFaceName",
		view.m_lf, view.m_nPointSize );

	cProfile.IOProfileData( pszSecName, LTEXT("bFontIs_FIXED_PITCH"), view.m_bFontIs_FIXED_PITCH );
}

/*!
	@brief 共有データのKeyBindセクションの入出力
*/
void CShareData_IO::ShareData_IO_KeyBind( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	IO_KeyBind( cProfile, pShare->m_Common.m_sKeyBind, false );	// add Parameter 2008/5/24
}

/*!
	@brief KeyBindセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス
	@param[in,out]	sKeyBind	キー割り当て設定

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
	@date 2010.08.21 Moca ShareData_IO_KeyBindをIO_KeyBindに名称変更
	@date 2012.11.20 aroka 引数を CommonSetting_KeyBind に変更
	@date 2012.11.25 aroka マウスコードの固定と重複排除
*/
void CShareData_IO::IO_KeyBind( CDataProfile& cProfile, CommonSetting_KeyBind& sKeyBind, bool bOutCmdName)
{
	const WCHAR*	szSecName = L"KeyBind";
	int		i;
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
//	int		nSize = m_pShareData->m_nKeyNameArrNum;
	WCHAR	szWork[MAX_PLUGIN_ID+20+4];
	bool	bOldVer = false;
	const int KEYNAME_SIZE = _countof(sKeyBind.m_pKeyNameArr)-1;// 最後の１要素はダミー用に予約 2012.11.25 aroka
	int nKeyNameArrUsed = sKeyBind.m_nKeyNameArrNum; // 使用済み領域

	if( cProfile.IsReadingMode() ){ 
		if (!cProfile.IOProfileData( szSecName, L"KeyBind[000]", MakeStringBufferW(szKeyData) ) ) {
			bOldVer = true;
		}
		else {
			// 新スタイルのImportは割り当て表サイズぎりぎりまで読み込む
			// 旧スタイルは初期値と一致しないKeyNameは捨てるのでデータ数に変化なし
			sKeyBind.m_nKeyNameArrNum = KEYNAME_SIZE;
		}
	}

	for( i = 0; i < sKeyBind.m_nKeyNameArrNum; ++i ){
		// 2005.04.07 D.S.Koba
		//KEYDATA& keydata = m_pShareData->m_pKeyNameArr[i];
		//KEYDATA& keydata = sKeyBind.ppKeyNameArr[i];
		
		if( cProfile.IsReadingMode() ){
			if (bOldVer) {
				KEYDATA& keydata = sKeyBind.m_pKeyNameArr[i];
				_tcstowcs( szKeyName, keydata.m_szKeyName, _countof(szKeyName) );
				if( cProfile.IOProfileData( szSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
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
			}
			else {		// 新バージョン(キー割り当てのImport,export の合わせた)	2008/5/25 Uchi
				KEYDATA tmpKeydata;
				auto_sprintf(szKeyName, L"KeyBind[%03d]", i);
				if( cProfile.IOProfileData( szSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
					wchar_t	*p;
					wchar_t	*pn;
					int		nRes;

					p = szKeyData;
					// keycode取得
					int keycode;
					pn = auto_strchr(p,',');
					if (pn == NULL)	continue;
					*pn = 0;
					nRes = scan_ints(p, L"%04x", &keycode);
					if (nRes!=1)	continue;
					tmpKeydata.m_nKeyCode = (short)keycode;
					p = pn+1;

					//後に続くトークン 
					for (int j = 0; j < 8; j++) {
						EFunctionCode n;
						//機能名を数値に置き換える。(数値の機能名もあるかも)
						//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
						pn = auto_strchr(p,',');
						if (pn == NULL)	break;
						*pn = 0;
						n = GetFunctionStrToFunctionCode(p);
						tmpKeydata.m_nFuncCodeArr[j] = n;
						p = pn+1;
					}
					// KeyName
					auto_strncpy(tmpKeydata.m_szKeyName, to_tchar(p), _countof(tmpKeydata.m_szKeyName)-1);
					tmpKeydata.m_szKeyName[_countof(tmpKeydata.m_szKeyName)-1] = '\0';

					if( tmpKeydata.m_nKeyCode <= 0 ){ // マウスコードは先頭に固定されている KeyCodeが同じなのでKeyNameで判別
						// 2013.10.23 syat マウスのキーコードを拡張仮想キーコードに変更。以下は互換性のため残す。
						for( int im=0; im< jpVKEXNamesLen; im++ ){
							if( _tcscmp( tmpKeydata.m_szKeyName, jpVKEXNames[im] ) == 0 ){
								_tcscpy( tmpKeydata.m_szKeyName, sKeyBind.m_pKeyNameArr[im].m_szKeyName );
								sKeyBind.m_pKeyNameArr[im + 0x0100] = tmpKeydata;
							}
						}
					}
					else{
						// 割り当て済みキーコードは上書き
						int idx = sKeyBind.m_VKeyToKeyNameArr[tmpKeydata.m_nKeyCode];
						if( idx != KEYNAME_SIZE ){
							_tcscpy( tmpKeydata.m_szKeyName, sKeyBind.m_pKeyNameArr[idx].m_szKeyName );
							sKeyBind.m_pKeyNameArr[idx] = tmpKeydata;
						}else{// 未割り当てキーコードは末尾に追加
							if( nKeyNameArrUsed >= KEYNAME_SIZE ){}
							else{
								_tcscpy( tmpKeydata.m_szKeyName, sKeyBind.m_pKeyNameArr[nKeyNameArrUsed].m_szKeyName );
								sKeyBind.m_pKeyNameArr[nKeyNameArrUsed] = tmpKeydata;
								sKeyBind.m_VKeyToKeyNameArr[tmpKeydata.m_nKeyCode] = (BYTE)nKeyNameArrUsed++;
							}
						}
					}
				}
			}
		}else{
		//	auto_sprintf( szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"),
		//		keydata.m_nFuncCodeArr[0],
		//		keydata.m_nFuncCodeArr[1],
		//		keydata.m_nFuncCodeArr[2],
		//		keydata.m_nFuncCodeArr[3],
		//		keydata.m_nFuncCodeArr[4],
		//		keydata.m_nFuncCodeArr[5],
		//		keydata.m_nFuncCodeArr[6],
		//		keydata.m_nFuncCodeArr[7]
		//	);
		//	cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );

// start 新バージョン	2008/5/25 Uchi
			KEYDATA& keydata = sKeyBind.m_pKeyNameArr[i];
			auto_sprintf(szKeyName, L"KeyBind[%03d]", i);
			auto_sprintf(szKeyData, L"%04x", keydata.m_nKeyCode);
			for(int j = 0; j < 8; j++)
			{
				WCHAR	szFuncName[256];
				if (GetPlugCmdInfoByFuncCode( keydata.m_nFuncCodeArr[j], szFuncName )) {
					// Plugin
					auto_sprintf( szWork, L",%ls", szFuncName );
				}
				else {
					if (bOutCmdName) {
						//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
						// 2010.06.30 Moca 日本語名を取得しないように
						WCHAR	*p = CSMacroMgr::GetFuncInfoByID(
							0,
							keydata.m_nFuncCodeArr[j],
							szFuncName,
							NULL
						);
						if( p ) {
							auto_sprintf(szWork, L",%ls", p);
						}
						else {
							auto_sprintf(szWork, L",%d", keydata.m_nFuncCodeArr[j]);
						}
					}
					else {
						auto_sprintf(szWork, L",%d", keydata.m_nFuncCodeArr[j]);
					}
				}
				wcscat(szKeyData, szWork);
			}

			if( 0x0100 <= keydata.m_nKeyCode ){
				auto_sprintf(szWork, L",%ts", jpVKEXNames[ keydata.m_nKeyCode - 0x0100 ]);
			}else{
				auto_sprintf(szWork, L",%ts", keydata.m_szKeyName);
			}
			wcscat(szKeyData, szWork);
			cProfile.IOProfileData( szSecName, szKeyName, MakeStringBufferW(szKeyData) );
//
		}
	}

	if( cProfile.IsReadingMode() ){
		sKeyBind.m_nKeyNameArrNum = nKeyNameArrUsed;
	}
}

/*!
	@brief 共有データのPrintセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData_IO::ShareData_IO_Print( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Print");
	int		i, j;
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		// 2005.04.07 D.S.Koba
		PRINTSETTING& printsetting = pShare->m_PrintSettingArr[i];
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
				printsetting.m_nPrintPaperOrientation	= (short)buf[ 9];
				printsetting.m_nPrintPaperSize			= (short)buf[10];
				printsetting.m_bPrintWordWrap			= (buf[11]!=0);
				printsetting.m_bPrintLineNumber			= (buf[12]!=0);
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
		// ヘッダ/フッタ
		for( j = 0; j < 3; ++j ){
			auto_sprintf( szKeyName, LTEXT("PS[%02d].szHF[%d]") , i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(printsetting.m_szHeaderForm[j]) );
			auto_sprintf( szKeyName, LTEXT("PS[%02d].szFTF[%d]"), i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(printsetting.m_szFooterForm[j]) );
		}
		{ // ヘッダ/フッタ フォント設定
			WCHAR	szKeyName2[64];
			WCHAR	szKeyName3[64];
			auto_sprintf( szKeyName,  LTEXT("PS[%02d].lfHeader"),			i );
			auto_sprintf( szKeyName2, LTEXT("PS[%02d].nHeaderPointSize"),	i );
			auto_sprintf( szKeyName3, LTEXT("PS[%02d].lfHeaderFaceName"),	i );
			ShareData_IO_Sub_LogFont( cProfile, pszSecName, szKeyName,szKeyName2, szKeyName3,
				printsetting.m_lfHeader, printsetting.m_nHeaderPointSize );
			auto_sprintf( szKeyName,  LTEXT("PS[%02d].lfFooter"),			i );
			auto_sprintf( szKeyName2, LTEXT("PS[%02d].nFooterPointSize"),	i );
			auto_sprintf( szKeyName3, LTEXT("PS[%02d].lfFooterFaceName"),	i );
			ShareData_IO_Sub_LogFont( cProfile, pszSecName, szKeyName,szKeyName2, szKeyName3,
				printsetting.m_lfFooter, printsetting.m_nFooterPointSize );
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

		//カラー印刷
		auto_sprintf( szKeyName, LTEXT("PS[%02d].bColorPrint"), i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bColorPrint );	// 2013/4/26 Uchi
	}
}

/*!
	@brief 共有データのSTypeConfigセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
	@date 2010/04/17 Uchi ループ内をShareData_IO_Type_Oneに分離。
*/
void CShareData_IO::ShareData_IO_Types( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	int		i;
	WCHAR	szKey[32];
	
	int nCountOld = pShare->m_nTypesCount;
	if( !cProfile.IOProfileData( L"Other", LTEXT("nTypesCount"), pShare->m_nTypesCount ) ){
		pShare->m_nTypesCount = 30; // 旧バージョン読み込み用
	}
	SetValueLimit( pShare->m_nTypesCount, 1, MAX_TYPES );
	// 注：コントロールプロセス専用
	std::vector<STypeConfig*>& types = CShareData::getInstance()->GetTypeSettings();
	for( i = GetDllShareData().m_nTypesCount; i < nCountOld; i++ ){
		delete types[i];
		types[i] = NULL;
	}
	types.resize(pShare->m_nTypesCount);
	for( i = nCountOld; i < pShare->m_nTypesCount; i++ ){
		types[i] = new STypeConfig();
		*types[i] = *types[0]; // 基本をコピー
		auto_sprintf( types[i]->m_szTypeName, LS(STR_TRAY_TYPE_NAME), i );
		types[i]->m_nIdx = i;
		types[i]->m_id = i;
	}

	for( i = 0; i < pShare->m_nTypesCount; ++i ){
		auto_sprintf( szKey, LTEXT("Types(%d)"), i );
		STypeConfig& type = *(types[i]);
		ShareData_IO_Type_One(cProfile, type, szKey);
		if( cProfile.IsReadingMode() ){
			type.m_nIdx = i;
			if( i == 0 ){
				pShare->m_TypeBasis = type;
			}
			auto_strcpy(pShare->m_TypeMini[i].m_szTypeExts, type.m_szTypeExts);
			auto_strcpy(pShare->m_TypeMini[i].m_szTypeName, type.m_szTypeName);
			pShare->m_TypeMini[i].m_id = type.m_id;
			pShare->m_TypeMini[i].m_encoding = type.m_encoding;
		}
	}
	if( cProfile.IsReadingMode() ){
		// Id重複チェック、更新
		for( i = 0; i < pShare->m_nTypesCount - 1; i++ ){
			STypeConfig& type = *(types[i]);
			for( int k = i + 1; k < pShare->m_nTypesCount; k++ ){
				STypeConfig& type2 = *(types[k]);
				if( type.m_id == type2.m_id ){
					type2.m_id = (::GetTickCount() & 0x3fffffff) + k * 0x10000;
					pShare->m_TypeMini[k].m_id = type2.m_id;
				}
			}
		}
	}
}

/*!
@brief 共有データのSTypeConfigセクションの入出力(１個分)
	@param[in,out]	cProfile	INIファイル入出力クラス
	@param[in]		type		タイプ別
	@param[in]		pszSecName	セクション名

	@date 2010/04/17 Uchi ShareData_IO_TypesOneから分離。
*/
void CShareData_IO::ShareData_IO_Type_One( CDataProfile& cProfile, STypeConfig& types, const WCHAR* pszSecName)
{
	int		j;
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[MAX_REGEX_KEYWORDLEN + 20];
	assert( 100 < MAX_REGEX_KEYWORDLEN + 20 );

	// 2005.04.07 D.S.Koba
	static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");	//MIK
	auto_strcpy( szKeyName, LTEXT("nInts") );
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
			int buf[12];
			scan_ints( szKeyData, pszForm, buf );
			types.m_nIdx					= buf[ 0];
			types.m_nMaxLineKetas			= buf[ 1];
			types.m_nColumnSpace			= buf[ 2];
			types.m_nTabSpace				= buf[ 3];
			types.m_nKeyWordSetIdx[0]		= buf[ 4];
			types.m_nKeyWordSetIdx[1]		= buf[ 5];
			types.m_nStringType				= buf[ 6];
			types.m_bLineNumIsCRLF			= (buf[ 7]!=0);
			types.m_nLineTermType			= buf[ 8];
			types.m_bWordWrap				= (buf[ 9]!=0);
			types.m_nCurrentPrintSetting	= buf[10];
			types.m_nTsvMode				= buf[11];
		}
		// 折り返し幅の最小値は10。少なくとも４ないとハングアップする。 // 20050818 aroka
		if( types.m_nMaxLineKetas < CKetaXInt(MINLINEKETAS) ){
			types.m_nMaxLineKetas = CKetaXInt(MINLINEKETAS);
		}
		if( types.m_nMaxLineKetas - 2 < types.m_nTabSpace ){
			types.m_nTabSpace = types.m_nMaxLineKetas - 2;
		}
	}
	else{
		auto_sprintf( szKeyData, pszForm,
			types.m_nIdx,
			types.m_nMaxLineKetas,
			types.m_nColumnSpace,
			types.m_nTabSpace,
			types.m_nKeyWordSetIdx[0],
			types.m_nKeyWordSetIdx[1],
			types.m_nStringType,
			types.m_bLineNumIsCRLF?1:0,
			types.m_nLineTermType,
			types.m_bWordWrap?1:0,
			types.m_nCurrentPrintSetting,
			types.m_nTsvMode
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
	cProfile.IOProfileData( pszSecName, LTEXT("nLineSpace"), types.m_nLineSpace );
	if( cProfile.IsReadingMode() ){
		if( types.m_nLineSpace < -LINESPACE_MAX ){
			types.m_nLineSpace = -LINESPACE_MAX;
		}
		if( types.m_nLineSpace > LINESPACE_MAX ){
			types.m_nLineSpace = LINESPACE_MAX;
		}
	}

	/* 行番号の最小桁数 */	// 加追 2014.08.02 katze
	cProfile.IOProfileData( pszSecName, LTEXT("nLineNumWidth"), types.m_nLineNumWidth );
	if( cProfile.IsReadingMode() ){
		if( types.m_nLineNumWidth < LINENUMWIDTH_MIN ){
			types.m_nLineNumWidth = LINENUMWIDTH_MIN;
		}
		if( types.m_nLineNumWidth > LINENUMWIDTH_MAX ){
			types.m_nLineNumWidth = LINENUMWIDTH_MAX;
		}
	}

	cProfile.IOProfileData( pszSecName, LTEXT("szTypeName"), MakeStringBufferT(types.m_szTypeName) );
	cProfile.IOProfileData( pszSecName, LTEXT("szTypeExts"), MakeStringBufferT(types.m_szTypeExts) );
	cProfile.IOProfileData( pszSecName, LTEXT("id"), types.m_id );
	if( types.m_id < 0 ){
		types.m_id *= -1;
	}
	cProfile.IOProfileData( pszSecName, LTEXT("szTabViewString"), MakeStringBufferW(types.m_szTabViewString) );
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("bTabArrow")	, types.m_bTabArrow );	//@@@ 2003.03.26 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bInsSpace")			, types.m_bInsSpace );	// 2001.12.03 hor

	cProfile.IOProfileData( pszSecName, LTEXT("nTextWrapMethod"), types.m_nTextWrapMethod );		// 2008.05.30 nasukoji

	cProfile.IOProfileData( pszSecName, LTEXT("bStringLineOnly"), types.m_bStringLineOnly );
	cProfile.IOProfileData( pszSecName, LTEXT("bStringEndLine"), types.m_bStringEndLine );

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
		if( bRet1 && bRet2 ) types.m_cBlockComments[0].SetBlockCommentRule( buffer[0], buffer[1] );

		//@@@ 2001.03.10 by MIK
		buffer[0][0] = buffer[1][0] = L'\0';
		bRet1 = cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentFrom2"), MakeStringBufferW(buffer[0]) );
		bRet2 = cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo2")	, MakeStringBufferW(buffer[1]) );
		if( bRet1 && bRet2 ) types.m_cBlockComments[1].SetBlockCommentRule( buffer[0], buffer[1] );
		
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
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComments[0].getBlockCommentFrom())) );
		cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComments[0].getBlockCommentTo())) );

		//@@@ 2001.03.10 by MIK
		cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentFrom2"),
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComments[1].getBlockCommentFrom())) );
		cProfile.IOProfileData( pszSecName, LTEXT("szBlockCommentTo2")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_cBlockComments[1].getBlockCommentTo())) );

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

	cProfile.IOProfileData( pszSecName, LTEXT("bOutlineDockDisp")			, types.m_bOutlineDockDisp );/* アウトライン解析表示の有無 */
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("eOutlineDockSide")	, types.m_eOutlineDockSide );/* アウトライン解析ドッキング配置 */
	{
		const WCHAR* pszKeyName = LTEXT("xyOutlineDock");
		const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
		WCHAR		szKeyData[1024];
		if( cProfile.IsReadingMode() ){
			if( cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData)) ){
				int buf[4];
				scan_ints( szKeyData, pszForm, buf );
				types.m_cxOutlineDockLeft	= buf[0];
				types.m_cyOutlineDockTop	= buf[1];
				types.m_cxOutlineDockRight	= buf[2];
				types.m_cyOutlineDockBottom	= buf[3];
			}
		}else{
			auto_sprintf(
				szKeyData,
				pszForm,
				types.m_cxOutlineDockLeft,
				types.m_cyOutlineDockTop,
				types.m_cxOutlineDockRight,
				types.m_cyOutlineDockBottom
			);
			cProfile.IOProfileData( pszSecName, pszKeyName, MakeStringBufferW(szKeyData) );
		}
	}
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nDockOutline")	, 	types.m_nDockOutline );/* アウトライン解析方法 */
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nDefaultOutline")	, types.m_eDefaultOutline );/* アウトライン解析方法 */
	cProfile.IOProfileData( pszSecName, LTEXT("szOutlineRuleFilename")	, types.m_szOutlineRuleFilename );/* アウトライン解析ルールファイル */
	cProfile.IOProfileData( pszSecName, LTEXT("nOutlineSortCol")		, types.m_nOutlineSortCol );/* アウトライン解析ソート列番号 */
	cProfile.IOProfileData( pszSecName, LTEXT("bOutlineSortDesc")		, types.m_bOutlineSortDesc );/* アウトライン解析ソート降順 */
	cProfile.IOProfileData( pszSecName, LTEXT("nOutlineSortType")		, types.m_nOutlineSortType );/* アウトライン解析ソート基準 */
	ShareData_IO_FileTree( cProfile, types.m_sFileTree, pszSecName );
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("nSmartIndent")		, types.m_eSmartIndent );/* スマートインデント種別 */
	cProfile.IOProfileData( pszSecName, LTEXT("bIndentCppStringIgnore")		, types.m_bIndentCppStringIgnore );
	cProfile.IOProfileData( pszSecName, LTEXT("bIndentCppCommentIgnore")	, types.m_bIndentCppCommentIgnore );
	cProfile.IOProfileData( pszSecName, LTEXT("bIndentCppUndoSep")	, types.m_bIndentCppUndoSep );
	//	Nov. 20, 2000 genta
	cProfile.IOProfileData( pszSecName, LTEXT("nImeState")			, types.m_nImeState );	//	IME制御

	//	2001/06/14 Start By asa-o: タイプ別の補完ファイル
	//	Oct. 5, 2002 genta _countof()で誤ってポインタのサイズを取得していたのを修正
	cProfile.IOProfileData( pszSecName, LTEXT("szHokanFile")		, types.m_szHokanFile );		//	補完ファイル
	//	2001/06/14 End
	cProfile.IOProfileData( pszSecName, LTEXT("nHokanType")			, types.m_nHokanType );		//	補完種別

	//	2001/06/19 asa-o
	cProfile.IOProfileData( pszSecName, LTEXT("bHokanLoHiCase")		, types.m_bHokanLoHiCase );

	//	2003.06.23 Moca ファイル内からの入力補完機能
	cProfile.IOProfileData( pszSecName, LTEXT("bUseHokanByFile")		, types.m_bUseHokanByFile );
	cProfile.IOProfileData( pszSecName, LTEXT("bUseHokanByKeyword")		, types.m_bUseHokanByKeyword );

	//@@@ 2002.2.4 YAZAKI
	cProfile.IOProfileData( pszSecName, LTEXT("szExtHelp")			, types.m_szExtHelp );

	cProfile.IOProfileData( pszSecName, LTEXT("szExtHtmlHelp")		, types.m_szExtHtmlHelp );
	cProfile.IOProfileData( pszSecName, LTEXT("bTypeHtmlHelpIsSingle"), types.m_bHtmlHelpIsSingle ); // 2012.06.30 Fix m_bHokanLoHiCase -> m_bHtmlHelpIsSingle

	cProfile.IOProfileData( pszSecName, LTEXT("bPriorCesu8")		, types.m_encoding.m_bPriorCesu8 );
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("eDefaultCodetype")	, types.m_encoding.m_eDefaultCodetype );
	cProfile.IOProfileData_WrapInt( pszSecName, LTEXT("eDefaultEoltype")	, types.m_encoding.m_eDefaultEoltype );
	cProfile.IOProfileData( pszSecName, LTEXT("bDefaultBom")		, types.m_encoding.m_bDefaultBom );

	cProfile.IOProfileData( pszSecName, LTEXT("bAutoIndent")			, types.m_bAutoIndent );
	cProfile.IOProfileData( pszSecName, LTEXT("bAutoIndent_ZENSPACE")	, types.m_bAutoIndent_ZENSPACE );
	cProfile.IOProfileData( pszSecName, LTEXT("bRTrimPrevLine")			, types.m_bRTrimPrevLine );			// 2005.10.08 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("nIndentLayout")			, types.m_nIndentLayout );

	/* 色設定 I/O */
	IO_ColorSet( &cProfile, pszSecName, types.m_ColorInfoArr  );

	// 2010.09.17 背景画像
	cProfile.IOProfileData( pszSecName, L"bgImgPath", types.m_szBackImgPath );
	cProfile.IOProfileData_WrapInt( pszSecName, L"bgImgPos", types.m_backImgPos );
	cProfile.IOProfileData( pszSecName, L"bgImgScrollX",   types.m_backImgScrollX );
	cProfile.IOProfileData( pszSecName, L"bgImgScrollY",   types.m_backImgScrollY );
	cProfile.IOProfileData( pszSecName, L"bgImgRepeartX",  types.m_backImgRepeatX );
	cProfile.IOProfileData( pszSecName, L"bgImgRepeartY",  types.m_backImgRepeatY );
	cProfile.IOProfileData_WrapInt( pszSecName, L"bgImgPosOffsetX",  types.m_backImgPosOffset.x );
	cProfile.IOProfileData_WrapInt( pszSecName, L"bgImgPosOffsetY",  types.m_backImgPosOffset.y );

	// 2005.11.08 Moca 指定桁縦線
	for(j = 0; j < MAX_VERTLINES; j++ ){
		auto_sprintf( szKeyName, LTEXT("nVertLineIdx%d"), j + 1 );
		cProfile.IOProfileData( pszSecName, szKeyName, types.m_nVertLineIdx[j] );
		if( types.m_nVertLineIdx[j] == 0 ){
			break;
		}
	}
	cProfile.IOProfileData( pszSecName, L"nNoteLineOffset", types.m_nNoteLineOffset );

//@@@ 2001.11.17 add start MIK
	{	//正規表現キーワード
		WCHAR	*p;
		cProfile.IOProfileData( pszSecName, LTEXT("bUseRegexKeyword"), types.m_bUseRegexKeyword );/* 正規表現キーワード使用するか？ */
		wchar_t* pKeyword = types.m_RegexKeywordList;
		int nPos = 0;
		int nKeywordSize = _countof(types.m_RegexKeywordList);
		for(j = 0; j < _countof(types.m_RegexKeywordArr); j++)
		{
			auto_sprintf( szKeyName, LTEXT("RxKey[%03d]"), j );
			if( cProfile.IsReadingMode() )
			{
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
						if( 0 < nKeywordSize - nPos - 1 ){
							wcscpyn(&pKeyword[nPos], p, nKeywordSize - nPos - 1 );
						}
						if( types.m_RegexKeywordArr[j].m_nColorIndex < 0
						 || types.m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
						{
							types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
						}
						if( pKeyword[nPos] ){
							nPos += auto_strlen(&pKeyword[nPos]) + 1;
						}
					}
				}else{
					// 2010.06.18 Moca 値がない場合は終了
					break;
				}
			}
			// 2002.02.08 hor 未定義値を無視
			else if(pKeyword[nPos])
			{
				auto_sprintf( szKeyData, LTEXT("%ls,%ls"),
					GetColorNameByIndex( types.m_RegexKeywordArr[j].m_nColorIndex ),
					&pKeyword[nPos]);
				cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
				nPos += auto_strlen(&pKeyword[nPos]) + 1;
			}
		}
		if( cProfile.IsReadingMode() ){
			pKeyword[nPos] = L'\0';
		}
	}
//@@@ 2001.11.17 add end MIK

	/* 禁則 */
	cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuHead")	, types.m_bKinsokuHead );
	cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuTail")	, types.m_bKinsokuTail );
	cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuRet")	, types.m_bKinsokuRet );	//@@@ 2002.04.13 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuKuto")	, types.m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("bKinsokuHide")	, types.m_bKinsokuHide );	//2012/11/30 Uchi
	cProfile.IOProfileData( pszSecName, LTEXT("szKinsokuHead")	, MakeStringBufferW(types.m_szKinsokuHead) );
	cProfile.IOProfileData( pszSecName, LTEXT("szKinsokuTail")	, MakeStringBufferW(types.m_szKinsokuTail) );
	cProfile.IOProfileData( pszSecName, LTEXT("szKinsokuKuto")	, MakeStringBufferW(types.m_szKinsokuKuto) );	// 2009.08.07 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("bUseDocumentIcon")	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta 変数名誤り修正

//@@@ 2006.04.10 fon ADD-start
	{	/* キーワード辞書 */
		WCHAR	*pH, *pT;	/* <pH>keyword<pT> */
		cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyWordHelp"), types.m_bUseKeyWordHelp );	/* キーワード辞書選択を使用するか？ */
//		cProfile.IOProfileData( pszSecName, LTEXT("nKeyHelpNum"), types.m_nKeyHelpNum );				/* 登録辞書数 */
		cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyHelpAllSearch"), types.m_bUseKeyHelpAllSearch );	/* ヒットした次の辞書も検索(&A) */
		cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyHelpKeyDisp"), types.m_bUseKeyHelpKeyDisp );		/* 1行目にキーワードも表示する(&W) */
		cProfile.IOProfileData( pszSecName, LTEXT("bUseKeyHelpPrefix"), types.m_bUseKeyHelpPrefix );		/* 選択範囲で前方一致検索(&P) */
		cProfile.IOProfileData_WrapInt(pszSecName, LTEXT("nKeyHelpRMenuShowType"), types.m_eKeyHelpRMenuShowType);
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
							if( L'\0' != (*pH) ){
								_wcstotcs( types.m_KeyHelpArr[j].m_szPath, pH, _countof2(types.m_KeyHelpArr[j].m_szPath) );
								types.m_nKeyHelpNum = j+1;	// iniに保存せずに、読み出せたファイル分を辞書数とする
							}
						}
					}
				}
			}/* 書き込み */
			else{
				if(types.m_KeyHelpArr[j].m_szPath[0] != _T('\0')){
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

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	cProfile.IOProfileData( pszSecName, LTEXT("bChkEnterAtEnd")	, types.m_bChkEnterAtEnd );

	{ // フォント設定
		cProfile.IOProfileData( pszSecName, LTEXT("bUseTypeFont"), types.m_bUseTypeFont );
		ShareData_IO_Sub_LogFont( cProfile, pszSecName, L"lf", L"nPointSize", L"lfFaceName",
			types.m_lf, types.m_nPointSize );
	}
}

/*!
	@brief 共有データのKeyWordsセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData_IO::ShareData_IO_KeyWords( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR*		pszSecName = LTEXT("KeyWords");
	int				i, j;
	WCHAR			szKeyName[64];
	WCHAR			szKeyData[1024];
	CKeyWordSetMgr*	pCKeyWordSetMgr = &pShare->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	int				nKeyWordSetNum = pCKeyWordSetMgr->m_nKeyWordSetNum;

	cProfile.IOProfileData( pszSecName, LTEXT("nCurrentKeyWordSetIdx")	, pCKeyWordSetMgr->m_nCurrentKeyWordSetIdx );
	bool bIOSuccess = cProfile.IOProfileData( pszSecName, LTEXT("nKeyWordSetNum"), nKeyWordSetNum );
	if( cProfile.IsReadingMode() ){
		// nKeyWordSetNum が読み込めていれば、すべての情報がそろっていると仮定して処理を進める
		if( bIOSuccess ){
			// 2004.11.25 Moca キーワードセットの情報は、直接書き換えないで関数を利用する
			// 初期設定されているため、先に削除しないと固定メモリの確保に失敗する可能性がある
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
				std::wstring sValue;	// wstring のまま受ける（古い ini ファイルのキーワードは中身が NULL 文字区切りなので StringBufferW では NG だった）
				if( cProfile.IOProfileData( pszSecName, szKeyName, sValue ) ){
					pCKeyWordSetMgr->SetKeyWordArr( i, nKeyWordNum, sValue.c_str() );
				}
			}
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
void CShareData_IO::ShareData_IO_Macro( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Macro");
	int		i;	
	WCHAR	szKeyName[64];
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		MacroRec& macrorec = pShare->m_Common.m_sMacro.m_MacroTable[i];
		//	Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if( !cProfile.IsReadingMode() && macrorec.m_szName[0] == _T('\0') && macrorec.m_szFile[0] == _T('\0') ) continue;
		auto_sprintf( szKeyName, LTEXT("Name[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(macrorec.m_szName) );
		auto_sprintf( szKeyName, LTEXT("File[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferT(macrorec.m_szFile) );
		auto_sprintf( szKeyName, LTEXT("ReloadWhenExecute[%03d]"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
	cProfile.IOProfileData( pszSecName, LTEXT("nMacroOnOpened"), pShare->m_Common.m_sMacro.m_nMacroOnOpened );	/* オープン後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("nMacroOnTypeChanged"), pShare->m_Common.m_sMacro.m_nMacroOnTypeChanged );	/* タイプ変更後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("nMacroOnSave"), pShare->m_Common.m_sMacro.m_nMacroOnSave );	/* 保存前自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
	cProfile.IOProfileData( pszSecName, LTEXT("nMacroCancelTimer"), pShare->m_Common.m_sMacro.m_nMacroCancelTimer );	// マクロ停止ダイアログ表示待ち時間	// 2011.08.04 syat
}

/*!
	@brief 共有データのStatusbarセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2008/6/21 Uchi
*/
void CShareData_IO::ShareData_IO_Statusbar( CDataProfile& cProfile )
{
	const WCHAR* pszSecName = LTEXT("Statusbar");
	CommonSetting_Statusbar& statusbar = GetDllShareData().m_Common.m_sStatusbar;

	// 表示文字コードの指定
	cProfile.IOProfileData( pszSecName, LTEXT("DispUnicodeInSjis")			, statusbar.m_bDispUniInSjis);		// SJISで文字コード値をUnicodeで表示する
	cProfile.IOProfileData( pszSecName, LTEXT("DispUnicodeInJis")			, statusbar.m_bDispUniInJis);		// JISで文字コード値をUnicodeで表示する
	cProfile.IOProfileData( pszSecName, LTEXT("DispUnicodeInEuc")			, statusbar.m_bDispUniInEuc);		// EUCで文字コード値をUnicodeで表示する
	cProfile.IOProfileData( pszSecName, LTEXT("DispUtf8Codepoint")			, statusbar.m_bDispUtf8Codepoint);	// UTF-8をコードポイントで表示する
	cProfile.IOProfileData( pszSecName, LTEXT("DispSurrogatePairCodepoint")	, statusbar.m_bDispSPCodepoint);	// サロゲートペアをコードポイントで表示する
	cProfile.IOProfileData( pszSecName, LTEXT("DispSelectCountByByte")		, statusbar.m_bDispSelCountByByte);	// 選択文字数を文字単位ではなくバイト単位で表示する
	cProfile.IOProfileData( pszSecName, LTEXT("DispColByChar")				, statusbar.m_bDispColByChar);		// 現在桁をルーラー単位ではなく文字単位で表示する
}

/*!
	@brief 共有データのPluginセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2009/11/30 syat
*/
void CShareData_IO::ShareData_IO_Plugin( CDataProfile& cProfile, CMenuDrawer* pcMenuDrawer )
{
	const WCHAR* pszSecName = LTEXT("Plugin");
	CommonSetting& common = GetDllShareData().m_Common;
	CommonSetting_Plugin& plugin = GetDllShareData().m_Common.m_sPlugin;

	cProfile.IOProfileData( pszSecName, LTEXT("EnablePlugin"), plugin.m_bEnablePlugin);		// プラグインを使用する

	//プラグインテーブル
	int		i;
	int		j;
	WCHAR	szKeyName[64];
	for( i = 0; i < MAX_PLUGIN; ++i ){
		PluginRec& pluginrec = common.m_sPlugin.m_PluginTable[i];

		// 2010.08.04 Moca 書き込み直前に削除フラグで削除扱いにする
		if( pluginrec.m_state == PLS_DELETED ){
			pluginrec.m_szName[0] = L'\0';
			pluginrec.m_szId[0] = L'\0';
		}
		auto_sprintf( szKeyName, LTEXT("P[%02d].Name"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pluginrec.m_szName) );
		auto_sprintf( szKeyName, LTEXT("P[%02d].Id"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pluginrec.m_szId) );
		auto_sprintf( szKeyName, LTEXT("P[%02d].CmdNum"), i );
		cProfile.IOProfileData( pszSecName, szKeyName, pluginrec.m_nCmdNum );	// 2010/7/4 Uchi
		pluginrec.m_state = ( pluginrec.m_szId[0] == '\0' ? PLS_NONE : PLS_STOPPED );
		// Command 仮設定	// 2010/7/4 Uchi
		if (pluginrec.m_szId[0] != '\0' && pluginrec.m_nCmdNum >0) {
			for (j = 1; j <= pluginrec.m_nCmdNum; j++) {
				pcMenuDrawer->AddToolButton( CMenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT, CPlug::GetPluginFunctionCode(i, j) );
			}
		}
	}
}

struct SMainMenuAddItemInfo
{
	int m_nVer;
	EFunctionCode m_nAddFuncCode;
	EFunctionCode m_nPrevFuncCode;
	wchar_t m_cAccKey;
	bool m_bAddPrevSeparete;
	bool m_bAddNextSeparete;
};

void CShareData_IO::ShareData_IO_MainMenu( CDataProfile& cProfile )
{
	IO_MainMenu( cProfile, GetDllShareData().m_Common.m_sMainMenu, false );		// 2010/5/15 Uchi

	// 2015.02.26 Moca メインメニュー自動更新
	const WCHAR*	pszSecName = LTEXT("MainMenu");
	int& nVersion = GetDllShareData().m_Common.m_sMainMenu.m_nVersion;
	// ※メニュー定義を追加したらnCurrentVerを修正
	const int nCurrentVer = 2;
	nVersion = nCurrentVer;
	if( cProfile.IOProfileData(pszSecName, LTEXT("nMainMenuVer"), nVersion) ){
	}else{
		if( cProfile.IsReadingMode() ){
			int menuNum;
			if( cProfile.IOProfileData(pszSecName, LTEXT("nMainMenuNum"), menuNum) ){
				// メインメニューが定義されていた
				nVersion = 0; // 旧定義はVer0
			}else{
				// メインメニューすらない古いバージョンからのアップデートでは、最新メニューになるのでパス
			}
		}
	}
	if( cProfile.IsReadingMode() && nVersion < nCurrentVer ){
		CommonSetting_MainMenu& mainmenu = GetDllShareData().m_Common.m_sMainMenu;
		SMainMenuAddItemInfo addInfos[] = {
			{1, F_FILENEW_NEWWINDOW, F_FILENEW, L'M', false, false},	// 新しいウインドウを開く
			{1, F_CHG_CHARSET, F_TOGGLE_KEY_SEARCH, L'A', false, false},	// 文字コード変更
			{1, F_CHG_CHARSET, F_VIEWMODE, L'A', false, false}, 	// 文字コード変更(Sub)
			{1, F_FILE_REOPEN_LATIN1, F_FILE_REOPEN_EUC, L'L', false, false}, 	// Latin1で開き直す
			{1, F_FILE_REOPEN_LATIN1, F_FILE_REOPEN, L'L', false, false}, 	// Latin1で開き直す(Sub)
			{1, F_COPY_COLOR_HTML, F_COPYLINESWITHLINENUMBER, L'C', false, false}, 	// 選択範囲内色付きHTMLコピー
			{1, F_COPY_COLOR_HTML_LINENUMBER, F_COPY_COLOR_HTML, L'F', false, false}, 	// 選択範囲内行番号色付きHTMLコピー
			// 矩形選択類は省略...
			{1, F_GREP_REPLACE_DLG, F_GREP_DIALOG, L'\0', false, false}, 	// Grep置換
			{1, F_FILETREE, F_OUTLINE, L'E', false, false}, 	// ファイルツリー表示
			{1, F_FILETREE, F_OUTLINE_TOGGLE, L'E', false, false}, 	// ファイルツリー表示(Sub)
			{1, F_SHOWMINIMAP, F_SHOWSTATUSBAR, L'N', false, false}, 	// ミニマップ表示
			{1, F_SHOWMINIMAP, F_SHOWTAB, L'N', false, false}, 	// ミニマップ表示(Sub)
			{1, F_SHOWMINIMAP, F_SHOWFUNCKEY, L'N', false, false}, 	// ミニマップ表示(Sub)
			{1, F_SHOWMINIMAP, F_SHOWTOOLBAR, L'N', false, false}, 	// ミニマップ表示(Sub)
			{1, F_FUNCLIST_NEXT, F_JUMPHIST_SET, L'\0', true, false}, 	// 次の関数リストマーク(セパレータ追加)
			{1, F_FUNCLIST_PREV, F_FUNCLIST_NEXT, L'\0', false, false}, 	// 前の関数リストマーク
			{1, F_MODIFYLINE_NEXT, F_FUNCLIST_PREV, L'\0', false, false}, 	// 次の変更行へ
			{1, F_MODIFYLINE_PREV, F_MODIFYLINE_NEXT, L'\0', false, false}, 	// 前の変更行へ
			{1, F_MODIFYLINE_NEXT_SEL, F_GOFILEEND_SEL, L'\0', true, false}, 	// (選択)次の変更行へ
			{1, F_MODIFYLINE_PREV_SEL, F_MODIFYLINE_NEXT_SEL, L'\0', false, false}, 	// (選択)前の変更行へ
			{2, F_DLGWINLIST, F_WIN_OUTPUT, L'D', false, false}, 	// ウインドウ一覧表示
		};
		for( int i = 0; i < _countof(addInfos); i++ ){
			SMainMenuAddItemInfo& item = addInfos[i];
			if( item.m_nVer <= nVersion ){
				continue;
			}
			CMainMenu*	pcMenuTlb = mainmenu.m_cMainMenuTbl;
			int k = 0;
			for(; k < mainmenu.m_nMainMenuNum; k++ ){
				if( pcMenuTlb[k].m_nFunc == item.m_nAddFuncCode ){
					break;
				}
			}
			int nAddSep = 0;
			if( item.m_bAddPrevSeparete ){
				nAddSep++;
			}
			if( item.m_bAddNextSeparete ){
				nAddSep++;
			}
			if( k == mainmenu.m_nMainMenuNum && mainmenu.m_nMainMenuNum + nAddSep < _countof(mainmenu.m_cMainMenuTbl) ){
				// メニュー内にまだ追加されていないので追加する
				for( int r = 0; r < mainmenu.m_nMainMenuNum; r++ ){
					if( pcMenuTlb[r].m_nFunc == item.m_nPrevFuncCode && 0 < pcMenuTlb[r].m_nLevel ){
						// 追加分後ろにずらす
						for( int n = mainmenu.m_nMainMenuNum - 1; r < n; n-- ){
							pcMenuTlb[n + 1 + nAddSep] = pcMenuTlb[n];
						}
						for( int n = 0; n < MAX_MAINMENU_TOP; n++ ){
							if( r < mainmenu.m_nMenuTopIdx[n] ){
								mainmenu.m_nMenuTopIdx[n] += 1 + nAddSep;
							}
						}
						CMainMenu* pcMenu = &pcMenuTlb[r+1];
						const int nLevel = pcMenuTlb[r].m_nLevel;
						if( item.m_bAddPrevSeparete ){
							pcMenu->m_nType    = T_SEPARATOR;
							pcMenu->m_nFunc    = F_SEPARATOR;
							pcMenu->m_nLevel   = nLevel;
							pcMenu->m_sName[0] = L'\0';
							pcMenu->m_sKey[0]  = L'\0';
							pcMenu->m_sKey[1]  = L'\0';
							pcMenu++;
							mainmenu.m_nMainMenuNum++;
						}
						pcMenu->m_nType    = T_LEAF;
						pcMenu->m_nFunc    = item.m_nAddFuncCode;
						pcMenu->m_nLevel   = nLevel;
						pcMenu->m_sName[0] = L'\0';
						pcMenu->m_sKey[0]  = L'\0';
						pcMenu->m_sKey[1]  = L'\0';
						mainmenu.m_nMainMenuNum++;
						if( item.m_bAddNextSeparete ){
							pcMenu++;
							pcMenu->m_nType    = T_SEPARATOR;
							pcMenu->m_nFunc    = F_SEPARATOR;
							pcMenu->m_nLevel   = nLevel;
							pcMenu->m_sName[0] = L'\0';
							pcMenu->m_sKey[0]  = L'\0';
							pcMenu->m_sKey[1]  = L'\0';
							mainmenu.m_nMainMenuNum++;
						}
						break;
					}
				}
			}
		}
	}
}


/*!
	@brief 共有データのMainMenuセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス
	@param[in,out]	mainmenu	共通設定MainMenuクラス
	@param[in]		bOutCmdName	出力時、名前で出力

	@date 2010/5/15 Uchi
	@date 2014.11.21 Moca pData追加。データのみのタイプを追加
*/
void CShareData_IO::IO_MainMenu( CDataProfile& cProfile, std::vector<std::wstring>* pData, CommonSetting_MainMenu& mainmenu, bool bOutCmdName)
{
	const WCHAR*	pszSecName = LTEXT("MainMenu");
	CMainMenu*		pcMenu;
	WCHAR	szKeyName[64];
	WCHAR	szFuncName[MAX_PLUGIN_ID+20];
	EFunctionCode n;
	int		nIdx;
	WCHAR	szLine[1024];
	WCHAR*	p = NULL;
	WCHAR*	pn;
	std::vector<std::wstring>& data = *pData;
	int dataNum = 0;

	if (cProfile.IsReadingMode()) {
		int menuNum = 0;
		if( pData ){
			menuNum = (int)data.size() - 1;
		}else{
			cProfile.IOProfileData( pszSecName, LTEXT("nMainMenuNum"), menuNum);
		}
		if (menuNum == 0) {
			return;
		}
		mainmenu.m_nMainMenuNum = menuNum;
		SetValueLimit( mainmenu.m_nMainMenuNum, MAX_MAINMENU );
	}
	else {
		cProfile.IOProfileData( pszSecName, LTEXT("nMainMenuNum"), mainmenu.m_nMainMenuNum);
	}
	
	if( pData ){
		mainmenu.m_bMainMenuKeyParentheses = (_wtoi(data[dataNum++].c_str()) != 0);
	}else{
		cProfile.IOProfileData( pszSecName, LTEXT("bKeyParentheses"), mainmenu.m_bMainMenuKeyParentheses );
	}

	if (cProfile.IsReadingMode()) {
		// Top Level 初期化
		memset( mainmenu.m_nMenuTopIdx, -1, sizeof(mainmenu.m_nMenuTopIdx) );
	}

	nIdx = 0;
	for (int i = 0; i < mainmenu.m_nMainMenuNum; i++) {
		//メインメニューテーブル
		pcMenu = &mainmenu.m_cMainMenuTbl[i];

		auto_sprintf( szKeyName, LTEXT("MM[%03d]"), i );
		if (cProfile.IsReadingMode()) {
			// 読み込み時初期化
			pcMenu->m_nType    = T_NODE;
			pcMenu->m_nFunc    = F_INVALID;
			pcMenu->m_nLevel   = 0;
			pcMenu->m_sName[0] = L'\0';
			pcMenu->m_sKey[0]  = L'\0';
			pcMenu->m_sKey[1]  = L'\0';

			// 読み出し
			if( pData ){
				wcscpy(szLine, data[dataNum++].c_str());
			}else{
				cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW( szLine ) );
			}

			// レベル
			p = szLine;
			pn = wcschr( p, L',' );
			if (pn != NULL)		*pn++ = L'\0';
			pcMenu->m_nLevel = auto_atol( p );
			if (pn == NULL) {
				continue;
			}

			// 種類
			p = pn;
			pn = wcschr( p, L',' );
			if (pn != NULL)		*pn++ = L'\0';
			pcMenu->m_nType = (EMainMenuType)auto_atol( p );
			if (pn == NULL) {
				continue;
			}
			
			// 機能(マクロ名対応)
			p = pn;
			pn = wcschr( p, L',' );
			if (pn != NULL)		*pn++ = L'\0';
			n = GetFunctionStrToFunctionCode(p);
			pcMenu->m_nFunc = n;
			if (pn == NULL) {
				continue;
			}

			// アクセスキー
			p = pn;
			if ( *p == L',' ) {
				// Key なし or ,
				if ( p[1] == L',') {
					// Key = ,
					pcMenu->m_sKey[0]  = *p++;
				}
			}
			else {
				pcMenu->m_sKey[0]  = *p++;
			}
			if (*p == L'\0') {
				continue;
			}

			// 表示名
			p++;
			auto_strcpy_s( pcMenu->m_sName, MAX_MAIN_MENU_NAME_LEN+1, p );
		}
		else {
			if (GetPlugCmdInfoByFuncCode( pcMenu->m_nFunc, szFuncName )) {
				// Plugin
			}
			else {
				if (bOutCmdName) {
					// マクロ名対応
					p = CSMacroMgr::GetFuncInfoByID(
						G_AppInstance(),
						pcMenu->m_nFunc,
						szFuncName,
						NULL
					);
				}
				if ( !bOutCmdName || p == NULL ) {
					auto_sprintf( szFuncName, L"%d", pcMenu->m_nFunc );
				}
			}
			// 書き込み
			// ラベル編集後のノードはノード名を出力する 2012.10.14 syat 各国語対応
			auto_sprintf( szLine, L"%d,%d,%ls,%ls,%ls", 
				pcMenu->m_nLevel, 
				pcMenu->m_nType, 
				szFuncName, 
				pcMenu->m_sKey, 
				pcMenu->m_nFunc == F_NODE ? pcMenu->m_sName : L"" );
			cProfile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW( szLine ) );
		}

		if (cProfile.IsReadingMode() && pcMenu->m_nLevel == 0) {
			// Top Level設定
			if (nIdx < MAX_MAINMENU_TOP) {
				mainmenu.m_nMenuTopIdx[nIdx++] = i;
			}
		}
	}
}

/*!
	@brief 共有データのOtherセクションの入出力
	@param[in,out]	cProfile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void CShareData_IO::ShareData_IO_Other( CDataProfile& cProfile )
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("Other");	//セクションを1個作成した。2003.05.12 MIK
	int		i;	
	WCHAR	szKeyName[64];

	/* **** その他のダイアログ **** */
	/* 外部コマンド実行の「標準出力を得る」 */
	if(!cProfile.IOProfileData( pszSecName, LTEXT("nExecFlgOpt")	, pShare->m_nExecFlgOpt ) ){ //	2006.12.03 maru オプション拡張
		cProfile.IOProfileData( pszSecName, LTEXT("bGetStdout")		, pShare->m_nExecFlgOpt );
	}

	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
	cProfile.IOProfileData( pszSecName, LTEXT("bLineNumIsCRLF")	, pShare->m_bLineNumIsCRLF_ForJump );
	
	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("nDiffFlgOpt")	, pShare->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( pszSecName, LTEXT("nTagsOpt")		, pShare->m_nTagsOpt );
	cProfile.IOProfileData( pszSecName, LTEXT("szTagsCmdLine")	, MakeStringBufferT(pShare->m_szTagsCmdLine) );
	
	//From Here 2005.04.03 MIK キーワード指定タグジャンプ
	cProfile.IOProfileData( pszSecName, LTEXT("_TagJumpKeyword_Counts"), pShare->m_sTagJump.m_aTagJumpKeywords._GetSizeRef() );
	pShare->m_sHistory.m_aCommands.SetSizeLimit();
	int nSize = pShare->m_sTagJump.m_aTagJumpKeywords.size();
	for( i = 0; i < nSize; ++i ){
		auto_sprintf( szKeyName, LTEXT("TagJumpKeyword[%02d]"), i );
		if( i >= nSize ){
			pShare->m_sTagJump.m_aTagJumpKeywords[i][0] = LTEXT('\0');
		}
		cProfile.IOProfileData( pszSecName, szKeyName, pShare->m_sTagJump.m_aTagJumpKeywords[i] );
	}
	cProfile.IOProfileData( pszSecName, LTEXT("m_bTagJumpICase")		, pShare->m_sTagJump.m_bTagJumpICase );
	cProfile.IOProfileData( pszSecName, LTEXT("m_bTagJumpAnyWhere")		, pShare->m_sTagJump.m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK キーワード指定タグジャンプの

	//	MIK バージョン情報（書き込みのみ）
	if( ! cProfile.IsReadingMode() ){
		TCHAR	iniVer[256];
		auto_sprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( pShare->m_sVersion.m_dwProductVersionMS ),
					LOWORD( pShare->m_sVersion.m_dwProductVersionMS ),
					HIWORD( pShare->m_sVersion.m_dwProductVersionLS ),
					LOWORD( pShare->m_sVersion.m_dwProductVersionLS ) );
		cProfile.IOProfileData( pszSecName, LTEXT("szVersion"), MakeStringBufferT(iniVer) );

		// 共有メモリバージョン	2010/5/20 Uchi
		int		nStructureVersion;
		nStructureVersion = int(pShare->m_vStructureVersion);
		cProfile.IOProfileData( pszSecName, LTEXT("vStructureVersion"), nStructureVersion );
	}
	cProfile.IOProfileData( pszSecName, L"bIniReadOnly", pShare->m_Common.m_sOthers.m_bIniReadOnly );
}

/*!
	@brief 色設定 I/O

	指定された色設定を指定されたセクションに書き込む。または
	指定されたセクションからいろ設定を読み込む。

	@param[in,out]	pcProfile		書き出し、読み込み先Profile object (入出力方向はbReadに依存)
	@param[in]		pszSecName		セクション名
	@param[in,out]	pColorInfoArr	書き出し、読み込み対象の色設定へのポインタ (入出力方向はbReadに依存)
*/
void CShareData_IO::IO_ColorSet( CDataProfile* pcProfile, const WCHAR* pszSecName, ColorInfo* pColorInfoArr )
{
	WCHAR	szKeyName[256];
	WCHAR	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const WCHAR* pszForm = LTEXT("%d,%d,%06x,%06x,%d");
		auto_sprintf( szKeyName, LTEXT("C[%ts]"), g_ColorAttributeArr[j].szName );	//Stonee, 2001/01/12, 2001/01/15
		if( pcProfile->IsReadingMode() ){
			if( pcProfile->IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) ) ){
				int buf[5];
				scan_ints( szKeyData, pszForm, buf);
				pColorInfoArr[j].m_bDisp                  = (buf[0]!=0);
				pColorInfoArr[j].m_sFontAttr.m_bBoldFont  = (buf[1]!=0);
				pColorInfoArr[j].m_sColorAttr.m_cTEXT     = buf[2];
				pColorInfoArr[j].m_sColorAttr.m_cBACK     = buf[3];
				pColorInfoArr[j].m_sFontAttr.m_bUnderLine = (buf[4]!=0);
			}
			else{
				// 2006.12.07 ryoji
				// sakura Ver1.5.13.1 以前のiniファイルを読んだときにキャレットがテキスト背景色と同じになると
				// ちょっと困るのでキャレット色が読めないときはキャレット色をテキスト色と同じにする
				if( COLORIDX_CARET == j )
					pColorInfoArr[j].m_sColorAttr.m_cTEXT = pColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT;
			}
			// 2006.12.18 ryoji
			// 矛盾設定があれば修復する
			unsigned int fAttribute = g_ColorAttributeArr[j].fAttribute;
			if( 0 != (fAttribute & COLOR_ATTRIB_FORCE_DISP) )
				pColorInfoArr[j].m_bDisp = true;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_BOLD) )
				pColorInfoArr[j].m_sFontAttr.m_bBoldFont = false;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )
				pColorInfoArr[j].m_sFontAttr.m_bUnderLine = false;
		}
		else{
			auto_sprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp?1:0,
				pColorInfoArr[j].m_sFontAttr.m_bBoldFont?1:0,
				pColorInfoArr[j].m_sColorAttr.m_cTEXT,
				pColorInfoArr[j].m_sColorAttr.m_cBACK,
				pColorInfoArr[j].m_sFontAttr.m_bUnderLine?1:0
			);
			pcProfile->IOProfileData( pszSecName, szKeyName, MakeStringBufferW(szKeyData) );
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void ShareData_IO_Sub_LogFont( CDataProfile& cProfile, const WCHAR* pszSecName,
	const WCHAR* pszKeyLf, const WCHAR* pszKeyPointSize, const WCHAR* pszKeyFaceName, LOGFONT& lf, INT& nPointSize )
{
	const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
	WCHAR		szKeyData[1024];

	cProfile.IOProfileData( pszSecName, pszKeyPointSize, nPointSize );	// 2009.10.01 ryoji
	if( cProfile.IsReadingMode() ){
		if( cProfile.IOProfileData( pszSecName, pszKeyLf, MakeStringBufferW(szKeyData) ) ){
			int buf[13];
			scan_ints( szKeyData, pszForm, buf );
			lf.lfHeight			= buf[ 0];
			lf.lfWidth			= buf[ 1];
			lf.lfEscapement		= buf[ 2];
			lf.lfOrientation	= buf[ 3];
			lf.lfWeight			= buf[ 4];
			lf.lfItalic			= (BYTE)buf[ 5];
			lf.lfUnderline		= (BYTE)buf[ 6];
			lf.lfStrikeOut		= (BYTE)buf[ 7];
			lf.lfCharSet		= (BYTE)buf[ 8];
			lf.lfOutPrecision	= (BYTE)buf[ 9];
			lf.lfClipPrecision	= (BYTE)buf[10];
			lf.lfQuality		= (BYTE)buf[11];
			lf.lfPitchAndFamily	= (BYTE)buf[12];
			if( nPointSize != 0 ){
				// DPI変更してもフォントのポイントサイズが変わらないように
				// ポイント数からピクセル数に変換する
				lf.lfHeight = -DpiPointsToPixels( abs(nPointSize), 10 );	// pointSize: 1/10ポイント単位のサイズ
			}else{
				// 初回または古いバージョンからの更新時はポイント数をピクセル数から逆算して仮設定
				nPointSize = DpiPixelsToPoints( abs(lf.lfHeight), 10 );		// （従来フォントダイアログで小数点は指定不可）
			}
		}
	}else{
		auto_sprintf( szKeyData, pszForm,
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
		cProfile.IOProfileData( pszSecName, pszKeyLf, MakeStringBufferW(szKeyData) );
	}
	
	cProfile.IOProfileData( pszSecName, pszKeyFaceName, MakeStringBufferT(lf.lfFaceName) );
}


void CShareData_IO::ShareData_IO_FileTree( CDataProfile& cProfile, SFileTree& fileTree, const WCHAR* pszSecName )
{
	cProfile.IOProfileData( pszSecName, L"bFileTreeProject", fileTree.m_bProject );
	cProfile.IOProfileData( pszSecName, L"szFileTreeProjectIni", fileTree.m_szProjectIni );
	cProfile.IOProfileData( pszSecName, L"nFileTreeItemCount", fileTree.m_nItemCount );
	SetValueLimit( fileTree.m_nItemCount, _countof(fileTree.m_aItems) );
	for( int i = 0;i < fileTree.m_nItemCount; i++ ){
		ShareData_IO_FileTreeItem( cProfile, fileTree.m_aItems[i], pszSecName, i );
	}
}

void CShareData_IO::ShareData_IO_FileTreeItem(
	CDataProfile& cProfile, SFileTreeItem& item, const WCHAR* pszSecName, int i )
{
	WCHAR szKey[64];
	auto_sprintf( szKey, L"FileTree(%d).eItemType", i );
	cProfile.IOProfileData_WrapInt( pszSecName, szKey, item.m_eFileTreeItemType );
	if( cProfile.IsReadingMode() || item.m_eFileTreeItemType == EFileTreeItemType_Grep
		|| item.m_eFileTreeItemType == EFileTreeItemType_File ){
		auto_sprintf( szKey, L"FileTree(%d).szTargetPath", i );
		cProfile.IOProfileData( pszSecName, szKey, item.m_szTargetPath );
	}
	if( cProfile.IsReadingMode()
		|| ((item.m_eFileTreeItemType == EFileTreeItemType_Grep || item.m_eFileTreeItemType == EFileTreeItemType_File)
			&& item.m_szLabelName[0] != _T('\0') )
		|| item.m_eFileTreeItemType == EFileTreeItemType_Folder ){
		auto_sprintf( szKey, L"FileTree(%d).szLabelName", i );
		cProfile.IOProfileData( pszSecName, szKey, item.m_szLabelName );
	}
	auto_sprintf( szKey, L"FileTree(%d).nDepth", i );
	cProfile.IOProfileData( pszSecName, szKey, item.m_nDepth );
	if( cProfile.IsReadingMode() || item.m_eFileTreeItemType == EFileTreeItemType_Grep ){
		auto_sprintf( szKey, L"FileTree(%d).szTargetFile", i );
		cProfile.IOProfileData( pszSecName, szKey, item.m_szTargetFile );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreHidden", i );
		cProfile.IOProfileData( pszSecName, szKey, item.m_bIgnoreHidden );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreReadOny", i );
		cProfile.IOProfileData( pszSecName, szKey, item.m_bIgnoreReadOnly );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreSystem", i );
		cProfile.IOProfileData( pszSecName, szKey, item.m_bIgnoreSystem );
	}
}
