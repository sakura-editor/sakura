//	$Id$
/*!	@file
	プロセス間共有データのファイルへ／からの入出力

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "mymessage.h"
#include "debug.h"
#include "global.h"
#include "CRunningTimer.h"
#include "CProfile.h"

//#define REGCNV_INT2SZ		1
//#define REGCNV_SZ2SZ		2
//#define REGCNV_CHAR2SZ	3
#define STR_COLORDATA_HEAD3		" テキストエディタ色設定 Ver3"	//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした

/*!
  iniの色設定を番号でなく文字列で書き出す。(added by Stonee, 2001/01/12, 2001/01/15)
  配列の順番は共有メモリ中のデータの順番と一致している。

  @note 数値による内部的対応はCShareData.hで行っているので参照のこと。(Mar. 7, 2001 jepro)
*/
static char* colorIDXKeyName[] =
{
	"TXT",
	"RUL",
	"UND",
	"LNO",
	"MOD",
	"TAB",
	"ZEN",
	"CTL",
	"EOL",
	"RAP",
	"EOF",
//#ifdef COMPILE_COLOR_DIGIT
	"NUM",	//@@@ 2001.02.17 by MIK 半角数値の強調
//#endif
	"FND",
	"KW1",
	"KW2",
	"CMT",
	"SQT",
	"WQT",
	"URL",
	"RK1",	//@@@ 2001.11.17 add MIK
	"RK2",	//@@@ 2001.11.17 add MIK
	"RK3",	//@@@ 2001.11.17 add MIK
	"RK4",	//@@@ 2001.11.17 add MIK
	"RK5",	//@@@ 2001.11.17 add MIK
	"RK6",	//@@@ 2001.11.17 add MIK
	"RK7",	//@@@ 2001.11.17 add MIK
	"RK8",	//@@@ 2001.11.17 add MIK
	"RK9",	//@@@ 2001.11.17 add MIK
	"RKA",	//@@@ 2001.11.17 add MIK
	"LAST"	// Not Used
};


#if 0
/* レジストリ項目 値の読み込み/書き込み */
LONG CShareData::MY_RegVal_IO(
	BOOL			bRead,
	HKEY			hKey,			// handle of key to set value for
	LPCTSTR			lpValueName,	// address of value to set
	int				nRegCnvID,		//
	BYTE *			lpDataSrc,		// address of value data
	DWORD			cbDataSrc 		// size of value data,
)
{j
	LONG			lRet;
	DWORD			dwType;
	CONST BYTE *	pData;		// address of value data
	DWORD			nDataLen;	// size of value data
//	char			szValueStr[1024];
	char			szValueStr[MAX_SETNUM * MAX_KEYWORDNUM * ( MAX_KEYWORDLEN ) + 1];
	int*			pnWork;
//	if( NULL == lpDataSrc ){
//		MYTRACE( "MY_RegVal_IO() NULL == lpDataSrc\n" );
//	}
//	szValueStr[0] = '\0';
	/* 「読み込み」 か 「書き込み」か */
	if( bRead ){
		/* 読み込み */
		switch( nRegCnvID ){
		case REGCNV_INT2SZ:
			dwType = REG_SZ;
			break;
		case REGCNV_SZ2SZ:
			dwType = REG_SZ;
			break;
		case REG_MULTI_SZ:
			dwType = REG_MULTI_SZ;
			break;
		default:
			dwType = REG_BINARY;
			break;
		}
		nDataLen = sizeof( szValueStr ) - 1;
		szValueStr[0] = '\0';
		lRet = ::RegQueryValueEx( hKey, lpValueName, NULL, &dwType, (unsigned char *)szValueStr, &nDataLen );
		if( ERROR_SUCCESS != lRet ){
//			char*	pszMsgBuf;
//			::FormatMessage(
//				FORMAT_MESSAGE_ALLOCATE_BUFFER |
//				FORMAT_MESSAGE_FROM_SYSTEM |
//				FORMAT_MESSAGE_IGNORE_INSERTS,
//				NULL,
//				::GetLastError(),
//				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// デフォルト言語
//				(LPTSTR) &pszMsgBuf,
//				0,
//				NULL
//			);
//			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
//				"レジストリ項目 値の読み込み失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf
//			);
//			MYTRACE( "レジストリ項目 値の読み込み失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf );
//			::LocalFree( pszMsgBuf );
		}else{
			switch( nRegCnvID ){
			case REGCNV_INT2SZ:
				*((int*)lpDataSrc) = atoi( szValueStr );
				break;
			case REGCNV_SZ2SZ:
				strcpy( (char *)lpDataSrc, szValueStr );
				break;
			case REG_MULTI_SZ:
				memcpy( lpDataSrc, szValueStr, cbDataSrc );
				break;
			default:
				memcpy( lpDataSrc, szValueStr, cbDataSrc );
				break;
			}
		}
		return lRet;
	}else{
		/* 書き込み */
		switch( nRegCnvID ){
		case REGCNV_INT2SZ:
			pnWork = (int*)lpDataSrc;
			itoa( *pnWork, szValueStr, 10 );
			dwType = REG_SZ;
			pData = (CONST BYTE *)szValueStr;
			nDataLen = strlen( szValueStr );
			break;
		case REGCNV_SZ2SZ:
			dwType = REG_SZ;
			pData = lpDataSrc;
			nDataLen = strlen( (const char*)lpDataSrc );
			break;
		case REG_MULTI_SZ:
			dwType = REG_MULTI_SZ;
			pData = lpDataSrc;
			nDataLen = cbDataSrc;
			break;
		default:
			dwType = REG_BINARY;
			pData = lpDataSrc;
			nDataLen = cbDataSrc;
			break;
		}
		lRet = ::RegSetValueEx( hKey, lpValueName, 0, dwType, pData, nDataLen );
		if( ERROR_SUCCESS != lRet ){
			char*	pszMsgBuf;
			::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				::GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// デフォルト言語
				(LPTSTR) &pszMsgBuf,
				0,
				NULL
			);
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
				"レジストリ項目 値の書き込み()失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf
			);
//			MYTRACE( "レジストリ項目 値の書き込み()失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf );
			::LocalFree( pszMsgBuf );
		}
		return lRet;
	}
}



/* 共有データの読み込み/保存 */
BOOL CShareData::ShareData_IO( BOOL bRead )
{
	return ShareData_IO_2( bRead );

	HKEY			hkReg;
	DWORD			dwDisposition;
//	int				nIdx;
	char			szKey[256];
	char			szKeyData[1024];
	char*			pszKeyRoot = GSTR_REG_ROOTPATH;
	int				i, j;
	char			szKeyName[64];
//	char			szKeyStr[1024];
	FileInfo		fiInit;
	FileInfo*		pfiWork;
	int				nZero = 0;
	BOOL			bRet;
	int				nMemLen;
	char*			pszMem;

//	DWORD			dwTimeStart;
//	DWORD			dwTime;
	CRunningTimer	cRunningTimer;
//	CRunningTimer	cRunningTimerStart;
	bRet = TRUE;

//	if( bRead ){
//		MYTRACE( "レジストリ処理 読み込み\n" );
//	}else{
//		MYTRACE( "レジストリ処理 書き込み\n" );
//	}

//	cRunningTimerStart.Reset();
	cRunningTimer.Reset();
//	dwTimeStart = cRunningTimer.Read();
//	dwTime = dwTimeStart;

	wsprintf( szKey, "%s", pszKeyRoot );
	if( bRead ){
		/* レジストリの存在チェック */
		if( ERROR_SUCCESS != ::RegOpenKeyEx( HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hkReg ) ){
//			MYTRACE( "レジストリは存在しない\n" );
			return FALSE;
		}
		::RegCloseKey( hkReg );
	}
	wsprintf( szKey, "%s\\MRU", pszKeyRoot );
	if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
		bRet = FALSE;
		goto Section02;
	}
	MY_RegVal_IO( bRead, hkReg, "_MRU_Counts", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nMRUArrNum, 0 );
	fiInit.m_nCharCode = 0;
	fiInit.m_nViewLeftCol = 0;
	fiInit.m_nViewTopLine = 0;
	fiInit.m_nX = 0;
	fiInit.m_nY = 0;
	strcpy( fiInit.m_szPath, "" );
//	for( i = 0; i < MAX_MRU; ++i ){
	char	szKeyNameTop[64];

	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		if( i < m_pShareData->m_nMRUArrNum ){
			pfiWork = &m_pShareData->m_fiMRUArr[i];
		}else{
			m_pShareData->m_fiMRUArr[i] = fiInit;
			pfiWork = &m_pShareData->m_fiMRUArr[i];
		}
		wsprintf( szKeyNameTop, "MRU[%02d].", i );

		strcpy( szKeyName, szKeyNameTop );
		strcat( szKeyName, "nViewTopLine" );
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&pfiWork->m_nViewTopLine, 0 );
		strcpy( szKeyName, szKeyNameTop );
		strcat( szKeyName, "nViewLeftCol" );
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&pfiWork->m_nViewLeftCol, 0 );
		strcpy( szKeyName, szKeyNameTop );
		strcat( szKeyName, "nX" );
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&pfiWork->m_nX, 0 );
		strcpy( szKeyName, szKeyNameTop );
		strcat( szKeyName, "nY" );
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&pfiWork->m_nY, 0 );
		strcpy( szKeyName, szKeyNameTop );
		strcat( szKeyName, "nCharCode" );
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&pfiWork->m_nCharCode, 0 );
		strcpy( szKeyName, szKeyNameTop );
		strcat( szKeyName, "szPath" );
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/pfiWork->m_szPath, 0 );
	}
	MY_RegVal_IO( bRead, hkReg, "_MRUFOLDER_Counts", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nOPENFOLDERArrNum, 0 );
//	for( i = 0; i < MAX_OPENFOLDER; ++i ){
	for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		if( i >= m_pShareData->m_nOPENFOLDERArrNum ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
		}
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szOPENFOLDERArr[i], 0 );
	}
	MY_RegVal_IO( bRead, hkReg, "_SEARCHKEY_Counts", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nSEARCHKEYArrNum, 0 );
//	for( i = 0; i < MAX_SEARCHKEY; ++i ){
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		if( i >= m_pShareData->m_nSEARCHKEYArrNum ){
			strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
		}
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szSEARCHKEYArr[i], 0 );
	}
	MY_RegVal_IO( bRead, hkReg, "_REPLACEKEY_Counts", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nREPLACEKEYArrNum, 0 );
//	for( i = 0; i < MAX_REPLACEKEY; ++i ){
	for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		if( i >= m_pShareData->m_nREPLACEKEYArrNum ){
			strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
		}
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szREPLACEKEYArr[i], 0 );
	}

	MY_RegVal_IO( bRead, hkReg, "_GREPFILE_Counts", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nGREPFILEArrNum, 0 );
//	for( i = 0; i < MAX_GREPFILE; ++i ){
	for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		if( i >= m_pShareData->m_nGREPFILEArrNum ){
			strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
		}
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szGREPFILEArr[i], 0 );
	}
	MY_RegVal_IO( bRead, hkReg, "_GREPFOLDER_Counts", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nGREPFOLDERArrNum, 0 );
//	for( i = 0; i < MAX_GREPFOLDER; ++i ){
	for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		if( i >= m_pShareData->m_nGREPFOLDERArrNum ){
			strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
		}
		MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szGREPFOLDERArr[i], 0 );
	}

	/* マクロ用フォルダ */
	MY_RegVal_IO( bRead, hkReg, "szMACROFOLDER", REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szMACROFOLDER, 0 );
	/* 設定インポート用フォルダ */
	MY_RegVal_IO( bRead, hkReg, "szIMPORTFOLDER", REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_szIMPORTFOLDER, 0 );

	::RegCloseKey( hkReg );


//	MYTRACE( "レジストリ処理 1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	/* 変更フラグ(共通設定の全体) */
//	if( !bRead && FALSE == m_pShareData->m_nCommonModify ){
//		/* 書き込みしない */
//	}else{
		/* 共通設定 */
		wsprintf( szKey, "%s\\Common", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		MY_RegVal_IO( bRead, hkReg, "nCaretType",			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nCaretType, 0 );
		MY_RegVal_IO( bRead, hkReg, "bIsINSMode"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bIsINSMode, 0 );
		MY_RegVal_IO( bRead, hkReg, "bIsFreeCursorMode",	, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bIsFreeCursorMode, 0 );
		MY_RegVal_IO( bRead, hkReg, "bAutoIndent"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bAutoIndent, 0 );
		MY_RegVal_IO( bRead, hkReg, "bAutoIndent_ZENSPACE"	, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bAutoIndent_ZENSPACE, 0 );
		MY_RegVal_IO( bRead, hkReg, "bAddCRLFWhenCopy"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bAddCRLFWhenCopy, 0 );
		MY_RegVal_IO( bRead, hkReg, "nRepeatedScrollLineNum", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nRepeatedScrollLineNum, 0 );
		MY_RegVal_IO( bRead, hkReg, "nRepeatedScroll_Smooth", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nRepeatedScroll_Smooth, 0 );
		MY_RegVal_IO( bRead, hkReg, "bExitConfirm"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bExitConfirm, 0 );
		MY_RegVal_IO( bRead, hkReg, "bSearchRegularExp"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bRegularExp, 0 );
		MY_RegVal_IO( bRead, hkReg, "bSearchLoHiCase"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bLoHiCase, 0 );
		MY_RegVal_IO( bRead, hkReg, "bSearchWordOnly"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bWordOnly, 0 );
		MY_RegVal_IO( bRead, hkReg, "bSearchNOTIFYNOTFOUND"	, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bNOTIFYNOTFOUND, 0 );
		MY_RegVal_IO( bRead, hkReg, "bSearchSelectedArea"	, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bSelectedArea, 0 );
		MY_RegVal_IO( bRead, hkReg, "bGrepSubFolder"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bGrepSubFolder, 0 );
		MY_RegVal_IO( bRead, hkReg, "bGrepOutputLine"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bGrepOutputLine, 0 );
		MY_RegVal_IO( bRead, hkReg, "nGrepOutputStyle"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nGrepOutputStyle, 0 );

		MY_RegVal_IO( bRead, hkReg, "bGTJW_RETURN"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bGTJW_RETURN, 0 );
		MY_RegVal_IO( bRead, hkReg, "bGTJW_LDBLCLK"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bGTJW_LDBLCLK, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUp"				, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bBackUp, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUpDialog"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bBackUpDialog, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUpFolder"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bBackUpFolder, 0 );

		if( !bRead ){
			int		nDummy;
			int		nCharChars;
			nDummy = strlen( m_pShareData->m_Common.m_szBackUpFolder );
			/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
			nCharChars = &m_pShareData->m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_pShareData->m_Common.m_szBackUpFolder, nDummy, &m_pShareData->m_Common.m_szBackUpFolder[nDummy] );
			if( 1 == nCharChars && m_pShareData->m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
			}else{
				strcat( m_pShareData->m_Common.m_szBackUpFolder, "\\" );
			}
		}
		MY_RegVal_IO( bRead, hkReg, "szBackUpFolder", REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szBackUpFolder, 0 );
		if( bRead ){
			int		nDummy;
			int		nCharChars;
			nDummy = strlen( m_pShareData->m_Common.m_szBackUpFolder );
			/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
			nCharChars = &m_pShareData->m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_pShareData->m_Common.m_szBackUpFolder, nDummy, &m_pShareData->m_Common.m_szBackUpFolder[nDummy] );
			if( 1 == nCharChars && m_pShareData->m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
			}else{
				strcat( m_pShareData->m_Common.m_szBackUpFolder, "\\" );
			}
		}


		MY_RegVal_IO( bRead, hkReg, "nBackUpType"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUpType2_Opt1"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType_Opt1, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUpType2_Opt2"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType_Opt2, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUpType2_Opt3"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType_Opt3, 0 );
//		MY_RegVal_IO( bRead, hkReg, "bBackUpType2_Opt4"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType_Opt4, 0 );
//		MY_RegVal_IO( bRead, hkReg, "bBackUpType2_Opt5"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType_Opt5, 0 );
//		MY_RegVal_IO( bRead, hkReg, "bBackUpType2_Opt6"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nBackUpType_Opt6, 0 );
		MY_RegVal_IO( bRead, hkReg, "bBackUpDustBox"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bBackUpDustBox, 0 );	//@@@ 2001.12.11 add MIK
		MY_RegVal_IO( bRead, hkReg, "nFileShareMode"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nFileShareMode, 0 );
		MY_RegVal_IO( bRead, hkReg, "szExtHelp1"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szExtHelp1, 0 );
		MY_RegVal_IO( bRead, hkReg, "szExtHtmlHelp"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szExtHtmlHelp, 0 );

		MY_RegVal_IO( bRead, hkReg, "nMRUArrNum_MAX"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nMRUArrNum_MAX, 0 );
		MY_RegVal_IO( bRead, hkReg, "nOPENFOLDERArrNum_MAX"	, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX, 0 );
		MY_RegVal_IO( bRead, hkReg, "bDispTOOLBAR"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bDispTOOLBAR, 0 );
		MY_RegVal_IO( bRead, hkReg, "bDispSTATUSBAR"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bDispSTATUSBAR, 0 );
		MY_RegVal_IO( bRead, hkReg, "bDispFUNCKEYWND"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bDispFUNCKEYWND, 0 );
		MY_RegVal_IO( bRead, hkReg, "nFUNCKEYWND_Place"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nFUNCKEYWND_Place, 0 );
		MY_RegVal_IO( bRead, hkReg, "szMidashiKigou"		, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szMidashiKigou, 0 );
		MY_RegVal_IO( bRead, hkReg, "szInyouKigou"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szInyouKigou, 0 );
		MY_RegVal_IO( bRead, hkReg, "bUseKeyWordHelp"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bUseKeyWordHelp, 0 );
		MY_RegVal_IO( bRead, hkReg, "szKeyWordHelpFile"		, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szKeyWordHelpFile, 0 );
		MY_RegVal_IO( bRead, hkReg, "bUseHokan"				, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bUseHokan, 0 );
		MY_RegVal_IO( bRead, hkReg, "szHokanFile"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szHokanFile, 0 );
		MY_RegVal_IO( bRead, hkReg, "bGrepKanjiCode_AutoDetect", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bGrepKanjiCode_AutoDetect, 0 );
		MY_RegVal_IO( bRead, hkReg, "bHokanLoHiCase"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bHokanLoHiCase, 0 );
		MY_RegVal_IO( bRead, hkReg, "bSaveWindowSize"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bSaveWindowSize, 0 );
		MY_RegVal_IO( bRead, hkReg, "nWinSizeType"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nWinSizeType, 0 );
		MY_RegVal_IO( bRead, hkReg, "nWinSizeCX"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nWinSizeCX, 0 );
		MY_RegVal_IO( bRead, hkReg, "nWinSizeCY"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nWinSizeCY, 0 );

		MY_RegVal_IO( bRead, hkReg, "bTaskTrayUse"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bUseTaskTray, 0 );
		MY_RegVal_IO( bRead, hkReg, "bTaskTrayStay"			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bStayTaskTray, 0 );
		if( bRead ){
			/* タスクトレイに常駐するときは、必ずタスクトレイアイコンを使う */
			if( m_pShareData->m_Common.m_bStayTaskTray ){
				m_pShareData->m_Common.m_bUseTaskTray = TRUE;
			}
		}




		::RegCloseKey( hkReg );


		/* ツールバー */
		wsprintf( szKey, "%s\\Common\\Toolbar", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		MY_RegVal_IO( bRead, hkReg, "nToolBarButtonNum"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nToolBarButtonNum, 0 );
		MY_RegVal_IO( bRead, hkReg, "bToolBarIsFlat"		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bToolBarIsFlat, 0 );
	//	for( i = 0; i < MAX_TOOLBARBUTTONS; ++i ){
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			wsprintf( szKeyName, "nTBB[%03d]", i );
			if( i < m_pShareData->m_Common.m_nToolBarButtonNum ){
				MY_RegVal_IO( bRead, hkReg, szKeyName		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nToolBarButtonIdxArr[i], 0 );
			}else{
				MY_RegVal_IO( bRead, hkReg, szKeyName		, REGCNV_INT2SZ, (BYTE *)&nZero, 0 );
			}
		}
		::RegCloseKey( hkReg );


		/* カスタムメニュー */
		wsprintf( szKey, "%s\\Common\\CustMenu", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( szKeyName, "szCMN[%02d]", i );
			MY_RegVal_IO( bRead, hkReg, szKeyName			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Common.m_szCustMenuNameArr[i], 0 );
			wsprintf( szKeyName, "nCMIN[%02d]", i );
			MY_RegVal_IO( bRead, hkReg, szKeyName			, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nCustMenuItemNumArr[i], 0 );
	//		for( j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
			for( j = 0; j < m_pShareData->m_Common.m_nCustMenuItemNumArr[i]; ++j ){
				wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
				MY_RegVal_IO( bRead, hkReg, szKeyName		, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j], 0 );
				wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
				MY_RegVal_IO( bRead, hkReg, szKeyName		, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_nCustMenuItemKeyArr[i][j], sizeof( m_pShareData->m_Common.m_nCustMenuItemKeyArr[i][j] ) );
			}
		}
		::RegCloseKey( hkReg );
//	}

//	/* 変更フラグ フォント */
//	if( !bRead && FALSE == m_pShareData->m_bFontModify ){
//		/* 書き込みしない */
//	}else{
		/* フォント */
		wsprintf( szKey, "%s\\Common\\Font", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		MY_RegVal_IO( bRead, hkReg, "lf.lfHeight"			, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfHeight			, sizeof( m_pShareData->m_Common.m_lf.lfHeight ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfWidth"			, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfWidth			, sizeof( m_pShareData->m_Common.m_lf.lfWidth ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfEscapement"		, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfEscapement		, sizeof( m_pShareData->m_Common.m_lf.lfEscapement ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfOrientation"		, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfOrientation	, sizeof( m_pShareData->m_Common.m_lf.lfOrientation ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfWeight"			, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfWeight			, sizeof( m_pShareData->m_Common.m_lf.lfWeight ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfItalic"			, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfItalic			, sizeof( m_pShareData->m_Common.m_lf.lfItalic ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfUnderline"		, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfUnderline		, sizeof( m_pShareData->m_Common.m_lf.lfUnderline ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfStrikeOut"		, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfStrikeOut		, sizeof( m_pShareData->m_Common.m_lf.lfStrikeOut ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfCharSet"			, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfCharSet		, sizeof( m_pShareData->m_Common.m_lf.lfCharSet ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfOutPrecision"		, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfOutPrecision	, sizeof( m_pShareData->m_Common.m_lf.lfOutPrecision ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfClipPrecision"	, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfClipPrecision	, sizeof( m_pShareData->m_Common.m_lf.lfClipPrecision ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfQuality"			, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfQuality		, sizeof( m_pShareData->m_Common.m_lf.lfQuality ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfPitchAndFamily"	, REG_BINARY, (BYTE *)&m_pShareData->m_Common.m_lf.lfPitchAndFamily	, sizeof( m_pShareData->m_Common.m_lf.lfPitchAndFamily ) );
		MY_RegVal_IO( bRead, hkReg, "lf.lfFaceName"			, REGCNV_SZ2SZ , (BYTE *)/*&*/m_pShareData->m_Common.m_lf.lfFaceName, 0 );
		MY_RegVal_IO( bRead, hkReg, "bFontIs_FIXED_PITCH"	, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_Common.m_bFontIs_FIXED_PITCH, 0 );
		::RegCloseKey( hkReg );
//	}

//	MYTRACE( "レジストリ処理 2 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	/* 変更フラグ キー割り当て */
//	if( !bRead && FALSE == m_pShareData->m_bKeyBindModify ){
//		/* 書き込みしない */
//	}else{
		/* キー割り当て */
		wsprintf( szKey, "%s\\Common\\KeyBind", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
	//保存不要	MY_RegVal_IO( bRead, hkReg, "nKeyNameArrNum", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_nKeyNameArrNum, 0 );
	//	for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
		for( i = 0; i < m_pShareData->m_nKeyNameArrNum; ++i ){
//			if( !bRead && FALSE == m_pShareData->m_bKeyBindModifyArr[i] ){	/* 変更フラグ キー割り当て(キーごと) */
//				/* 書き込みしない */
//				continue;
//			}

//			wsprintf( szKeyName, "Key[%03d].nKC", i );
//			MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_pKeyNameArr[i].m_nKeyCode, 0 );
//			wsprintf( szKeyName, "Key[%03d].szKN", i );
//			MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_pKeyNameArr[i].m_szKeyName, 0 );

//			wsprintf( szKeyName_1, "Key[%03d].nFC[", i );
//			for( j = 0; j < sizeof( m_pShareData->m_pKeyNameArr[0].m_nFuncCodeArr ) / sizeof( m_pShareData->m_pKeyNameArr[0].m_nFuncCodeArr[0] ); ++j ){
//				wsprintf( szKeyName_2, "%02d]", j );
//				strcpy( szKeyName, szKeyName_1 );
//				strcat( szKeyName, szKeyName_2 );
//				MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[j], 0 );
//			}

			wsprintf( szKeyName, "K[%03d].", i );
			if( bRead ){
				if( ERROR_SUCCESS == MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 ) ){
					wsscanf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
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
				MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 );
			}

		}
		::RegCloseKey( hkReg );
//	}

//	MYTRACE( "レジストリ処理 3 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	/* 変更フラグ(印刷の全体) */
//	if( !bRead && FALSE == m_pShareData->m_bPrintSettingModify ){
//		/* 書き込みしない */
//	}else{
		/* 印刷設定 */
		wsprintf( szKey, "%s\\Common\\Print", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//			if( !bRead && FALSE == m_pShareData->m_bPrintSettingModifyArr[i] ){	/* 変更フラグ(印刷設定ごと) */
//				/* 書き込みしない */
//				continue;
//			}
			wsprintf( szKeyName, "PS[%02d].nInts", i );
			static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( ERROR_SUCCESS == MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 ) ){
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
				MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 );
			}
//			wsprintf( szKeyName, "PS[%02d].nFTW2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nFTH2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nDSu"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu				, 0 );
//			wsprintf( szKeyName, "PS[%02d].nDSp2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nLSp"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing		, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGTY2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGBY2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGLX2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGRX2"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nPOrient", i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation	, 0 );
//			wsprintf( szKeyName, "PS[%02d].nPSize"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize			, 0 );
//			wsprintf( szKeyName, "PS[%02d].bWWrap"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap			, 0 );
//			wsprintf( szKeyName, "PS[%02d].bLNum"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber			, 0 );


			wsprintf( szKeyName, "PS[%02d].szSName"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName	, 0 );
			wsprintf( szKeyName, "PS[%02d].szFF"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan	, 0 );
			wsprintf( szKeyName, "PS[%02d].szFFZ"	, i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen	, 0 );
			for( j = 0; j < 3; ++j ){
//				wsprintf( szKeyName, "PS[%02d].bHU[%d]"  , i, j ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse   [j], 0 );
				wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ , (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_szHeaderForm [j], 0 );
//				wsprintf( szKeyName, "PS[%02d].bFTU[%d]" , i, j ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_PrintSettingArr[i].m_bFooterUse   [j], 0 );
				wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ , (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_szFooterForm [j], 0 );
			}
			wsprintf( szKeyName, "PS[%02d].szDriver", i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDriverName, 0 );
			wsprintf( szKeyName, "PS[%02d].szDevice", i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDeviceName, 0 );
			wsprintf( szKeyName, "PS[%02d].szOutput", i ); MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName, 0 );
		}
		::RegCloseKey( hkReg );
//	}


//	MYTRACE( "レジストリ処理 4 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	/* 変更フラグ(キーワードの全体) */
//	if( !bRead && FALSE == m_pShareData->m_bKeyWordSetModify ){
//	}else{
		/* 強調キーワード */
		wsprintf( szKey, "%s\\Common\\KeyWords", pszKeyRoot );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		MY_RegVal_IO( bRead, hkReg, "nCurrentKeyWordSetIdx", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, 0 );
		MY_RegVal_IO( bRead, hkReg, "nKeyWordSetNum", REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum, 0 );


		for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
//			/* 変更フラグ(キーワードのセットごと) */
//			if( !bRead && FALSE == m_pShareData->m_bKeyWordSetModifyArr[i] ){
//				continue;
//			}

			wsprintf( szKeyName, "szSN[%02d]", i );
			MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[i], 0 );
			wsprintf( szKeyName, "nCASE[%02d]", i );
			MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_CKeyWordSetMgr.m_nKEYWORDCASEArr[i], 0 );
			wsprintf( szKeyName, "nKWN[%02d]", i );
			MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i], 0 );
		}
		if( bRead ){
			pszMem = new char[MAX_SETNUM * MAX_KEYWORDNUM * ( MAX_KEYWORDLEN ) + 1];
			for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
				wsprintf( szKeyName, "szKW[%02d].Size", i );
				MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&nMemLen, 0 );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( ERROR_SUCCESS == MY_RegVal_IO( bRead, hkReg, szKeyName, REG_MULTI_SZ, (BYTE *)pszMem, nMemLen ) ){
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
//				/* 変更フラグ(キーワードのセットごと) */
//				if( !bRead && FALSE == m_pShareData->m_bKeyWordSetModifyArr[i] ){
//					continue;
//				}
				nMemLen = 0;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
					nMemLen ++;
				}
				nMemLen ++;
				wsprintf( szKeyName, "szKW[%02d].Size", i );
				MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_INT2SZ, (BYTE *)&nMemLen, 0 );
				pszMem = new char[nMemLen];
				nMemLen = 0;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					strcpy( &pszMem[nMemLen], m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
					nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.m_szKeyWordArr[i][j] );
					nMemLen ++;
				}
				pszMem[nMemLen] = '\0';
				wsprintf( szKeyName, "szKW[%02d]", i );
				MY_RegVal_IO( bRead, hkReg, szKeyName, REG_MULTI_SZ, (BYTE *)pszMem, nMemLen );
				delete [] pszMem;
			}
		}
		::RegCloseKey( hkReg );
//	}


//	MYTRACE( "レジストリ処理 5 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	wsprintf( szKey, "%s\\Types", pszKeyRoot );
//	if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//		bRet = FALSE;
//		goto Section02;
//	}
//	char	szTypeNum[16];
//	char	szNum[16];
	for( i = 0; i < MAX_TYPES; ++i ){
//		/* 変更フラグ(タイプ別設定) */
//		/* 無変更のタイプの設定は、保存しない */
//		if( !bRead && FALSE == m_pShareData->m_nTypesModifyArr[i] ){
//			continue;
//		}

		wsprintf( szKey, "%s\\Types[%d]", pszKeyRoot,  i );
		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
			bRet = FALSE;
			goto Section02;
		}
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( bRead ){
			if( ERROR_SUCCESS == MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 ) ){
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
			MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 );
		}
//		strcpy( szKeyName, "szTypeName" );
		MY_RegVal_IO( bRead, hkReg, "szTypeName"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szTypeName, 0 );
//		strcpy( szKeyName, "szTypeExts" );
		MY_RegVal_IO( bRead, hkReg, "szTypeExts"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szTypeExts, 0 );
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		{
			/* TAB表示文字列：前後で正当性をチェック */
			int  i;
			char szTab[9];
//			strcpy( szKeyName, "szTabViewString" );
			strcpy( szTab, "^       " );
			for( i = 0; i < 8; i++ ){
				if( (m_pShareData->m_Types[i].m_szTabViewString[i] == '\0') || (m_pShareData->m_Types[i].m_szTabViewString[i] < 0x20 || m_pShareData->m_Types[i].m_szTabViewString[i] >= 0x7f) ) break;
				szTab[i] = m_pShareData->m_Types[i].m_szTabViewString[i];
			}
			MY_RegVal_IO( bRead, hkReg, "szTabViewString"		, REGCNV_SZ2SZ, (BYTE *)/*&*/szTab, 0 );
			strcpy( m_pShareData->m_Types[i].m_szTabViewString, "^       " );
			for( i = 0; i < 8; i++ ){
				if( (szTab[i] == '\0') || (szTab[i] < 0x20 || szTab[i] >= 0x7f) ) break;
				m_pShareData->m_Types[i].m_szTabViewString[i] = szTab[i];
			}
		}
//#endif
//		strcpy( szKeyName, "szLineComment" );
		MY_RegVal_IO( bRead, hkReg, "szLineComment"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szLineComment, 0 );
//		strcpy( szKeyName, "szLineComment2" );
		MY_RegVal_IO( bRead, hkReg, "szLineComment2"		, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szLineComment2, 0 );
//		strcpy( szKeyName, "szLineComment3" );
		MY_RegVal_IO( bRead, hkReg, "szLineComment3"		, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szLineComment3, 0 );	//Jun. 01, 2001 JEPRO 追加
//		strcpy( szKeyName, "szBlockCommentFrom" );
		MY_RegVal_IO( bRead, hkReg, "szBlockCommentFrom"	, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szBlockCommentFrom, 0 );
//		strcpy( szKeyName, "szBlockCommentTo" );
		MY_RegVal_IO( bRead, hkReg, "szBlockCommentTo"		, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szBlockCommentTo, 0 );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
//		strcpy( szKeyName, "szBlockCommentFrom2" );
		MY_RegVal_IO( bRead, hkReg, "szBlockCommentFrom2"	, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szBlockCommentFrom2, 0 );
//		strcpy( szKeyName, "szBlockCommentTo2" );
		MY_RegVal_IO( bRead, hkReg, "szBlockCommentTo2"		, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szBlockCommentTo2, 0 );
//#endif
//		strcpy( szKeyName, "szIndentChars" );
		MY_RegVal_IO( bRead, hkReg, "szIndentChars"			, REGCNV_SZ2SZ, (BYTE *)/*&*/m_pShareData->m_Types[i].m_szIndentChars, 0 );
//		strcpy( szKeyName, "cLineTermChar" );
		MY_RegVal_IO( bRead, hkReg, "cLineTermChar"			, REG_BINARY, (BYTE *)&m_pShareData->m_Types[i].m_cLineTermChar, sizeof( m_pShareData->m_Types[i].m_cLineTermChar ) );
		for( j = 0; j < m_pShareData->m_Types[i].m_nColorInfoArrNum; ++j ){
//			static const char* pszForm = "%d,%d,%06x,%06x,%s";
			static const char* pszForm = "%d,%d,%06x,%06x";
//			wsprintf( szKeyName, "CI[%02d]", j );				//Jan. 15, 2001 Stonee commented out
			wsprintf( szKeyName, "C[%s]", colorIDXKeyName[j] );	//Jan. 15, 2001 Stonee added
//			strcpy( szKeyName, "CI[" );
//			itoa( j, szNum, 10 );
//			strcat( szKeyName, szNum );
//			strcat( szKeyName, "]" );

			if( bRead ){
				if( ERROR_SUCCESS == MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 ) ){
					sscanf( szKeyData, pszForm,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bDisp,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bFatFont,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colTEXT,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colBACK
//						m_pShareData->m_Types[i].m_ColorInfoArr[j].m_szName
					 );
				}
			}else{
				wsprintf( szKeyData, pszForm,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bDisp,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bFatFont,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colTEXT,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colBACK
//					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_szName
				);
				MY_RegVal_IO( bRead, hkReg, szKeyName, REGCNV_SZ2SZ, (BYTE *)szKeyData, 0 );
			}

		}

//		/* 変更フラグ(タイプ別設定) のクリア */
//		if( !bRead ){
//			m_pShareData->m_nTypesModifyArr[i] = FALSE;
//		}

		::RegCloseKey( hkReg );
	}
//	::RegCloseKey( hkReg );

//	MYTRACE( "レジストリ処理 6 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

Section02:;
//	/* 変更フラグ(共通設定の全体)のクリア */
//	m_pShareData->m_nCommonModify = FALSE;
//
//	/* 変更フラグ フォント */
//	m_pShareData->m_bFontModify = FALSE;
//
//	m_pShareData->m_bKeyBindModify = FALSE;	/* 変更フラグ キー割り当て */
//	for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
//		m_pShareData->m_bKeyBindModifyArr[i] = FALSE;	/* 変更フラグ キー割り当て(キーごと) */
//	}
//	/* 変更フラグ(印刷の全体)のクリア */
//	m_pShareData->m_bPrintSettingModify = FALSE;
//	/* 変更フラグ(印刷設定ごと)のクリア */
//	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//		m_pShareData->m_bPrintSettingModifyArr[i] = FALSE;
//	}
//	m_pShareData->m_bKeyWordSetModify = FALSE;	/* 変更フラグ(キーワードの全体) */
//	for( i = 0; i < MAX_SETNUM; ++i ){
//		m_pShareData->m_bKeyWordSetModifyArr[i] = FALSE;	/* 変更フラグ(キーワードのセットごと) */
//	}
//	/* 変更フラグ(タイプ別設定) のクリア */
//	for( i = 0; i < MAX_TYPES; ++i ){
//		m_pShareData->m_nTypesModifyArr[i] = FALSE;
//	}

//	MYTRACE( "レジストリ処理 7 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();


//	MYTRACE( "レジストリ処理 7 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	MYTRACE( "レジストリ処理 所要時間(ミリ秒) = %d\n", cRunningTimerStart.Read() );

	return bRet;
}




#endif







/*!
	共有データの読み込み/保存 2

	@param bRead [in] true: 読み込み / false: 書き込み
*/
BOOL CShareData::ShareData_IO_2( BOOL bRead )
{
	CRunningTimer cRunningTimer( "CShareData::ShareData_IO_2" );
	cRunningTimer.Reset();

//	DWORD			dwDisposition;
//	int				nIdx;
	char			szKey[256];
	char			szKeyData[1024];
//	char*			pszKeyRoot = GSTR_REG_ROOTPATH;
	int				i, j;
	char			szKeyName[64];
//	char			szKeyStr[1024];
	FileInfo		fiInit;
	FileInfo*		pfiWork;
	int				nZero = 0;
	BOOL			bRet;
	int				nMemLen;
	char*			pszMem;
	char			szIniFileName[_MAX_PATH + 1];
	CProfile		cProfile;
	const char*		pszSecName;

//	DWORD			dwTimeStart;
//	DWORD			dwTime;
//	CRunningTimer	cRunningTimerStart;
	char			szPath[_MAX_PATH + 1];
	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFname[_MAX_FNAME];
	char			szExt[_MAX_EXT];
	const char*		pszKeyName;
	const char*		pszForm;
	bRet = TRUE;

	/* exeのあるフォルダ */
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof(szPath)
	);
	_splitpath( szPath, szDrive, szDir, szFname, szExt );
	_makepath( szIniFileName, szDrive, szDir, szFname, "ini" );

//	strcpy( szIniFileName, "c:\\tmp\\sakura.ini" );


//	MYTRACE( "Iniファイル処理-1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );


	if( bRead ){
//		/* レジストリの存在チェック */
//		if( -1 == _access( szIniFileName, 0 ) ){
//			MYTRACE( "Iniファイルは存在しない\n%s", szIniFileName );
//			return FALSE;
//		}
		if( FALSE == cProfile.ReadProfile( szIniFileName ) ){
			/* 設定ファイルが存在しない */
			return FALSE;
		}
#ifdef _DEBUG
		cProfile.DUMP();
#endif

	}
//	MYTRACE( "Iniファイル処理 0 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );

	pszSecName = "MRU";
	cProfile.IOProfileData( bRead, pszSecName, "_MRU_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nMRUArrNum, 0 );
	fiInit.m_nCharCode = 0;
	fiInit.m_nViewLeftCol = 0;
	fiInit.m_nViewTopLine = 0;
	fiInit.m_nX = 0;
	fiInit.m_nY = 0;
	strcpy( fiInit.m_szPath, "" );
//	for( i = 0; i < MAX_MRU; ++i ){
	char	szKeyNameTop[64];

	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		if( i < m_pShareData->m_nMRUArrNum ){
			pfiWork = &m_pShareData->m_fiMRUArr[i];
		}else{
			m_pShareData->m_fiMRUArr[i] = fiInit;
			pfiWork = &m_pShareData->m_fiMRUArr[i];
		}
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
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/pfiWork->m_szPath, 0 );
	}
	cProfile.IOProfileData( bRead, pszSecName, "_MRUFOLDER_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nOPENFOLDERArrNum, 0 );

//	for( i = 0; i < MAX_OPENFOLDER; ++i ){
	for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		if( i >= m_pShareData->m_nOPENFOLDERArrNum ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szOPENFOLDERArr[i], 0 );
	}


	pszSecName = "Keys";
	cProfile.IOProfileData( bRead, pszSecName, "_SEARCHKEY_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nSEARCHKEYArrNum, 0 );
//	for( i = 0; i < MAX_SEARCHKEY; ++i ){
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		if( i >= m_pShareData->m_nSEARCHKEYArrNum ){
			strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szSEARCHKEYArr[i], 0 );
	}
	cProfile.IOProfileData( bRead, pszSecName, "_REPLACEKEY_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nREPLACEKEYArrNum, 0 );
//	for( i = 0; i < MAX_REPLACEKEY; ++i ){
	for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		if( i >= m_pShareData->m_nREPLACEKEYArrNum ){
			strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szREPLACEKEYArr[i], 0 );
	}

	pszSecName = "Grep";
	cProfile.IOProfileData( bRead, pszSecName, "_GREPFILE_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nGREPFILEArrNum, 0 );
//	for( i = 0; i < MAX_GREPFILE; ++i ){
	for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		if( i >= m_pShareData->m_nGREPFILEArrNum ){
			strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szGREPFILEArr[i], 0 );
	}
	cProfile.IOProfileData( bRead, pszSecName, "_GREPFOLDER_Counts", REGCNV_INT2SZ, (char*)&m_pShareData->m_nGREPFOLDERArrNum, 0 );
//	for( i = 0; i < MAX_GREPFOLDER; ++i ){
	for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		if( i >= m_pShareData->m_nGREPFOLDERArrNum ){
			strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szGREPFOLDERArr[i], 0 );
	}

	pszSecName = "Folders";
	/* マクロ用フォルダ */
	cProfile.IOProfileData( bRead, pszSecName, "szMACROFOLDER", REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szMACROFOLDER, 0 );
	/* 設定インポート用フォルダ */
	cProfile.IOProfileData( bRead, pszSecName, "szIMPORTFOLDER", REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szIMPORTFOLDER, 0 );

	pszSecName = "Cmd";
//	MRU_ExtCmd			m_MRU_ExtCmd;	/* MRU 外部コマンド */
	cProfile.IOProfileData( bRead, pszSecName, "nCmdArrNum", REGCNV_INT2SZ, (char*)&m_pShareData->m_nCmdArrNum, 0 );
	for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
		wsprintf( szKeyName, "szCmdArr[%02d]", i );
		if( i >= m_pShareData->m_nCmdArrNum ){
			strcpy( m_pShareData->m_szCmdArr[i], "" );
		}
		cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_szCmdArr[i], 0 );
	}



//	MYTRACE( "Iniファイル処理 1 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	/* 変更フラグ(共通設定の全体) */
//	if( !bRead && FALSE == m_pShareData->m_nCommonModify ){
//		/* 書き込みしない */
//	}else{
//		/* 共通設定 */
//		wsprintf( szKey, "%s\\Common", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "Common";

		cProfile.IOProfileData( bRead, pszSecName, "nCaretType"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nCaretType, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bIsINSMode"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bIsINSMode, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bIsFreeCursorMode"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bIsFreeCursorMode, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoIndent, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent_ZENSPACE"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAutoIndent_ZENSPACE, 0 );
		//	Oct. 27, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreCurPosition"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bRestoreCurPosition, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bAddCRLFWhenCopy"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bAddCRLFWhenCopy, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScrollLineNum"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRepeatedScrollLineNum, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScroll_Smooth"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nRepeatedScroll_Smooth, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bExitConfirm"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bExitConfirm, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchRegularExp"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bRegularExp, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchLoHiCase"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bLoHiCase, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchWordOnly"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bWordOnly, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchNOTIFYNOTFOUND"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bNOTIFYNOTFOUND, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchSelectedArea"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSelectedArea, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepSubFolder"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepSubFolder, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepOutputLine"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepOutputLine, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nGrepOutputStyle"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nGrepOutputStyle, 0 );

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
		cProfile.IOProfileData( bRead, pszSecName, "szBackUpFolder", REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szBackUpFolder, 0 );
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
//		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt4"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType_Opt4, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt5"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType_Opt5, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt6"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nBackUpType_Opt6, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDustBox"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bBackUpDustBox, 0 );	//@@@ 2001.12.11 add MIK
		cProfile.IOProfileData( bRead, pszSecName, "nFileShareMode"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nFileShareMode, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp1"					, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szExtHelp1, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szExtHtmlHelp, 0 );


		cProfile.IOProfileData( bRead, pszSecName, "nMRUArrNum_MAX"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nMRUArrNum_MAX, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nOPENFOLDERArrNum_MAX"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispTOOLBAR"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispTOOLBAR, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispSTATUSBAR"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispSTATUSBAR, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bDispFUNCKEYWND"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bDispFUNCKEYWND, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_Place"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nFUNCKEYWND_Place, 0 );
		// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndHScroll"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSplitterWndHScroll, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndVScroll"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSplitterWndVScroll, 0 );

		cProfile.IOProfileData( bRead, pszSecName, "szMidashiKigou"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szMidashiKigou, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szInyouKigou"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szInyouKigou, 0 );

		// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除
//		cProfile.IOProfileData( bRead, pszSecName, "bUseKeyWordHelp"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseKeyWordHelp, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "szKeyWordHelpFile"			, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szKeyWordHelpFile, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokan"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseHokan, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "szHokanFile"				, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szHokanFile, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepKanjiCode_AutoDetect"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bGrepKanjiCode_AutoDetect, 0 );
		// 2001/06/19 asa-o タイプ別に移動したので削除
//		cProfile.IOProfileData( bRead, pszSecName, "bHokanLoHiCase"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bHokanLoHiCase, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bSaveWindowSize"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bSaveWindowSize, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeType"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nWinSizeType, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCX"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nWinSizeCX, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCY"					, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nWinSizeCY, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayUse"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bUseTaskTray, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayStay"				, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bStayTaskTray, 0 );
		if( bRead ){
			/* タスクトレイに常駐するときは、必ずタスクトレイアイコンを使う */
			if( m_pShareData->m_Common.m_bStayTaskTray ){
				m_pShareData->m_Common.m_bUseTaskTray = TRUE;
			}
		}
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
			if( TRUE == cProfile.IOProfileData( bRead, pszSecName, pszKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
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

		/* ツールバー */
//		wsprintf( szKey, "%s\\Common\\Toolbar", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "Toolbar";
		cProfile.IOProfileData( bRead, pszSecName, "nToolBarButtonNum", REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nToolBarButtonNum, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "bToolBarIsFlat", REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bToolBarIsFlat, 0 );
	//	for( i = 0; i < MAX_TOOLBARBUTTONS; ++i ){
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			wsprintf( szKeyName, "nTBB[%03d]", i );
			if( i < m_pShareData->m_Common.m_nToolBarButtonNum ){
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nToolBarButtonIdxArr[i], 0 );
			}else{
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&nZero, 0 );
			}
		}

		/* カスタムメニュー */
//		wsprintf( szKey, "%s\\Common\\CustMenu", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "CustMenu";
		for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( szKeyName, "szCMN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta 最大長指定
			wsprintf( szKeyName, "nCMIN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nCustMenuItemNumArr[i], 0 );
	//		for( j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
			for( j = 0; j < m_pShareData->m_Common.m_nCustMenuItemNumArr[i]; ++j ){
				wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j], 0 );
				wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_CHAR2SZ, (char*)&m_pShareData->m_Common.m_nCustMenuItemKeyArr[i][j], 0 );
			}
		}
//	}

	/* 変更フラグ フォント */
//	if( !bRead && FALSE == m_pShareData->m_bFontModify ){
//		/* 書き込みしない */
//	}else{
		/* フォント */
//		wsprintf( szKey, "%s\\Common\\Font", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "Font";
		pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( bRead ){
			if( TRUE == cProfile.IOProfileData( bRead, pszSecName, "lf", REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
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


//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfHeight"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_lf.lfHeight		, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfWidth"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_lf.lfWidth		, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfEscapement"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_lf.lfEscapement	, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfOrientation"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_lf.lfOrientation	, 0 );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfWeight"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_lf.lfWeight		, 0  );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfItalic"		, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfItalic			, sizeof( m_pShareData->m_Common.m_lf.lfItalic ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfUnderline"		, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfUnderline		, sizeof( m_pShareData->m_Common.m_lf.lfUnderline ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfStrikeOut"		, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfStrikeOut		, sizeof( m_pShareData->m_Common.m_lf.lfStrikeOut ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfCharSet"		, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfCharSet			, sizeof( m_pShareData->m_Common.m_lf.lfCharSet ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfOutPrecision"	, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfOutPrecision	, sizeof( m_pShareData->m_Common.m_lf.lfOutPrecision ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfClipPrecision"	, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfClipPrecision	, sizeof( m_pShareData->m_Common.m_lf.lfClipPrecision ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfQuality"		, REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfQuality			, sizeof( m_pShareData->m_Common.m_lf.lfQuality ) );
//		cProfile.IOProfileData( bRead, pszSecName, "lf.lfPitchAndFamily", REG_BINARY, (char*)&m_pShareData->m_Common.m_lf.lfPitchAndFamily	, sizeof( m_pShareData->m_Common.m_lf.lfPitchAndFamily ) );
		cProfile.IOProfileData( bRead, pszSecName, "lfFaceName"			, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Common.m_lf.lfFaceName, 0 );

		cProfile.IOProfileData( bRead, pszSecName, "bFontIs_FIXED_PITCH", REGCNV_INT2SZ, (char*)&m_pShareData->m_Common.m_bFontIs_FIXED_PITCH, 0 );
//	}

//	MYTRACE( "Iniファイル処理 2 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

	/* 変更フラグ キー割り当て */
//	if( !bRead && FALSE == m_pShareData->m_bKeyBindModify ){
//		/* 書き込みしない */
//	}else{
		/* キー割り当て */
//		wsprintf( szKey, "%s\\Common\\KeyBind", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "KeyBind";

	//保存不要	cProfile.IOProfileData( bRead, pszSecName, "nKeyNameArrNum", REGCNV_INT2SZ, (char*)&m_pShareData->m_nKeyNameArrNum, 0 );
	//	for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
		for( i = 0; i < m_pShareData->m_nKeyNameArrNum; ++i ){
//			if( !bRead && FALSE == m_pShareData->m_bKeyBindModifyArr[i] ){	/* 変更フラグ キー割り当て(キーごと) */
//				/* 書き込みしない */
//				continue;
//			}

//			wsprintf( szKeyName, "Key[%03d].nKC", i );
//			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_pKeyNameArr[i].m_nKeyCode, 0 );
//			wsprintf( szKeyName, "Key[%03d].szKN", i );
//			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_pKeyNameArr[i].m_szKeyName, 0 );

//			wsprintf( szKeyName_1, "Key[%03d].nFC[", i );
//			for( j = 0; j < sizeof( m_pShareData->m_pKeyNameArr[0].m_nFuncCodeArr ) / sizeof( m_pShareData->m_pKeyNameArr[0].m_nFuncCodeArr[0] ); ++j ){
//				wsprintf( szKeyName_2, "%02d]", j );
//				strcpy( szKeyName, szKeyName_1 );
//				strcat( szKeyName, szKeyName_2 );
//				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[j], 0 );
//			}

//			wsprintf( szKeyName, "K[%03d]", i );
			strcpy( szKeyName, m_pShareData->m_pKeyNameArr[i].m_szKeyName );

			if( bRead ){
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
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
//	}

//	MYTRACE( "Iniファイル処理 3 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

	/* 変更フラグ(印刷の全体) */
//	if( !bRead && FALSE == m_pShareData->m_bPrintSettingModify ){
//		/* 書き込みしない */
//	}else{
		/* 印刷設定 */
//		wsprintf( szKey, "%s\\Common\\Print", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "Print";
		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//			if( !bRead && FALSE == m_pShareData->m_bPrintSettingModifyArr[i] ){	/* 変更フラグ(印刷設定ごと) */
//				/* 書き込みしない */
//				continue;
//			}
			wsprintf( szKeyName, "PS[%02d].nInts", i );
			static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
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
//			wsprintf( szKeyName, "PS[%02d].nFTW2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth		, 0 );
//			wsprintf( szKeyName, "PS[%02d].nFTH2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight		, 0 );
//			wsprintf( szKeyName, "PS[%02d].nDSu"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nDSp2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nLSp"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing		, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGTY2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGBY2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGLX2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nMGRX2"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX			, 0 );
//			wsprintf( szKeyName, "PS[%02d].nPOrient", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation	, 0 );
//			wsprintf( szKeyName, "PS[%02d].nPSize"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize		, 0 );
//			wsprintf( szKeyName, "PS[%02d].bWWrap"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap			, 0 );
//			wsprintf( szKeyName, "PS[%02d].bLNum"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber		, 0 );


			wsprintf( szKeyName, "PS[%02d].szSName"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName	, 0 );
			wsprintf( szKeyName, "PS[%02d].szFF"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan	, 0 );
			wsprintf( szKeyName, "PS[%02d].szFFZ"	, i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen	, 0 );
			for( j = 0; j < 3; ++j ){
//				wsprintf( szKeyName, "PS[%02d].bHU[%d]"	 , i, j ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse   [j], 0 );
				wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ , (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szHeaderForm [j], 0 );
//				wsprintf( szKeyName, "PS[%02d].bFTU[%d]" , i, j ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ, (char*)&m_pShareData->m_PrintSettingArr[i].m_bFooterUse   [j], 0 );
				wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ , (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_szFooterForm [j], 0 );
			}
			wsprintf( szKeyName, "PS[%02d].szDriver", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDriverName, 0 );
			wsprintf( szKeyName, "PS[%02d].szDevice", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDeviceName, 0 );
			wsprintf( szKeyName, "PS[%02d].szOutput", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName, 0 );
		}
//	}


//	MYTRACE( "Iniファイル処理 4 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();



//	MYTRACE( "Iniファイル処理 5 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//	wsprintf( szKey, "%s\\Types", pszKeyRoot );
//	if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//		bRet = FALSE;
//		goto Section02;
//	}
//	char	szTypeNum[16];
//	char	szNum[16];
	for( i = 0; i < MAX_TYPES; ++i ){
		/* 変更フラグ(タイプ別設定) */
//		/* 無変更のタイプの設定は、保存しない */
//		if( !bRead && FALSE == m_pShareData->m_nTypesModifyArr[i] ){
//			continue;
//		}

//		wsprintf( szKey, "%s\\Types[%d]", pszKeyRoot,  i );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		wsprintf( szKey, "Types(%d)", i );
		pszSecName = szKey;
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( bRead ){
			if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
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


//		strcpy( szKeyName, "szTypeName" );
		cProfile.IOProfileData( bRead, pszSecName, "szTypeName"			, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szTypeName, 0 );
//		strcpy( szKeyName, "szTypeExts" );
		cProfile.IOProfileData( bRead, pszSecName, "szTypeExts"			, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szTypeExts, 0 );
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
//		strcpy( szKeyName, "szTabViewString" );
		cProfile.IOProfileData( bRead, pszSecName, "szTabViewString"	, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szTabViewString, 0 );
//#endif
		cProfile.IOProfileData( bRead, pszSecName, "bInsSpace"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bInsSpace, 0 );	// 2001.12.03 hor
//		strcpy( szKeyName, "szLineComment" );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szLineComment, 0 );
//		strcpy( szKeyName, "szLineComment2" );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szLineComment2, 0 );
//		strcpy( szKeyName, "szLineComment3" );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szLineComment3, 0 );	//Jun. 01, 2001 JEPRO 追加
//		strcpy( szKeyName, "szBlockCommentFrom" );
		cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szBlockCommentFrom, 0 );
//		strcpy( szKeyName, "szBlockCommentTo" );
		cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szBlockCommentTo, 0 );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
//		strcpy( szKeyName, "szBlockCommentFrom2" );
		cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2", REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szBlockCommentFrom2, 0 );
//		strcpy( szKeyName, "szBlockCommentTo2" );
		cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szBlockCommentTo2, 0 );
//#endif
//		strcpy( szKeyName, "szIndentChars" );
		//	From here May 12, 2001 genta
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nLineCommentPos, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nLineCommentPos2, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nLineCommentPos3, 0 );	//Jun. 01, 2001 JEPRO 追加
		//	To here May 12, 2001 genta

		cProfile.IOProfileData( bRead, pszSecName, "szIndentChars"		, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_Types[i].m_szIndentChars, 0 );
//		strcpy( szKeyName, "cLineTermChar" );
		cProfile.IOProfileData( bRead, pszSecName, "cLineTermChar"		, REG_BINARY, (char*)&m_pShareData->m_Types[i].m_cLineTermChar, sizeof( m_pShareData->m_Types[i].m_cLineTermChar ) );

		cProfile.IOProfileData( bRead, pszSecName, "nDefaultOutline"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nDefaultOutline, 0 );/* アウトライン解析方法 */
		cProfile.IOProfileData( bRead, pszSecName, "nSmartIndent"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nSmartIndent, 0 );/* スマートインデント種別 */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "nImeState"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_nImeState, 0 );	//	IME制御

		//	2001/06/14 Start By asa-o: タイプ別の補完ファイルとキーワードヘルプ
		cProfile.IOProfileData( bRead, pszSecName, "szHokanFile"		, REGCNV_SZ2SZ, (char*)&m_pShareData->m_Types[i].m_szHokanFile, 0 );		//	補完ファイル
		cProfile.IOProfileData( bRead, pszSecName, "bUseKeyWordHelp"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bUseKeyWordHelp, 0 );	//	キーワードヘルプを使用する
		cProfile.IOProfileData( bRead, pszSecName, "szKeyWordHelpFile"	, REGCNV_SZ2SZ, (char*)&m_pShareData->m_Types[i].m_szKeyWordHelpFile, 0 );	//	キーワードヘルプ 辞書ファイル
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( bRead, pszSecName, "bHokanLoHiCase"		, REGCNV_INT2SZ, (char*)&m_pShareData->m_Types[i].m_bHokanLoHiCase, 0 );

		/* 色設定 I/O */
		IO_ColorSet( &cProfile, bRead, pszSecName, m_pShareData->m_Types[i].m_ColorInfoArr  );

/*
		for( j = 0; j < m_pShareData->m_Types[i].m_nColorInfoArrNum; ++j ){
			static const char* pszForm = "%d,%d,%06x,%06x";
			wsprintf( szKeyName, "CI[%02d]", j );
			if( bRead ){
				if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
					sscanf( szKeyData, pszForm,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bDisp,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bFatFont,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colTEXT,
						&m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colBACK
					 );
				}
			}else{
				wsprintf( szKeyData, pszForm,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bDisp,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_bFatFont,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colTEXT,
					m_pShareData->m_Types[i].m_ColorInfoArr[j].m_colBACK
				);
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
			}
		}
*/

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
					if( TRUE == cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) )
					{
						p = strchr(szKeyData, ',');
						if( p )
						{
							*p = '\0';
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
				{
					wsprintf( szKeyData, "%d,%s",
						m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex,
						m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword);
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 );
				}
			}
		}
//@@@ 2001.11.17 add end MIK


//		/* 変更フラグ(タイプ別設定) のクリア */
//		if( !bRead ){
//			m_pShareData->m_nTypesModifyArr[i] = FALSE;
//		}

	}


	/* 変更フラグ(キーワードの全体) */
//	if( !bRead && FALSE == m_pShareData->m_bKeyWordSetModify ){
//	}else{
		/* 強調キーワード */
//		wsprintf( szKey, "%s\\Common\\KeyWords", pszKeyRoot );
//		if( ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkReg, &dwDisposition ) ){
//			bRet = FALSE;
//			goto Section02;
//		}
		pszSecName = "KeyWords";
		cProfile.IOProfileData( bRead, pszSecName, "nCurrentKeyWordSetIdx"	, REGCNV_INT2SZ, (char*)&m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nKeyWordSetNum"			, REGCNV_INT2SZ, (char*)&m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum, 0 );


		for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
//			/* 変更フラグ(キーワードのセットごと) */
//			if( !bRead && FALSE == m_pShareData->m_bKeyWordSetModifyArr[i] ){
//				continue;
//			}

			wsprintf( szKeyName, "szSN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)/*&*/m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[i], 0 );
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
//				/* 変更フラグ(キーワードのセットごと) */
//				if( !bRead && FALSE == m_pShareData->m_bKeyWordSetModifyArr[i] ){
//					continue;
//				}
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
//	}

		//	From Here Sep. 14, 2001 genta
		//	Macro
		pszSecName = "Macro";
		
		for( i = 0; i < MAX_CUSTMACRO; ++i ){
			//	Oct. 4, 2001 genta あまり意味がなさそうなので削除
//			wsprintf( szKeyName, "Flag[%03d]", i );
//			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_INT2SZ,
//				(char*)&m_pShareData->m_MacroTable[i].m_bEnabled, FALSE );
			wsprintf( szKeyName, "Name[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)m_pShareData->m_MacroTable[i].m_szName, MACRONAME_MAX - 1 );
			wsprintf( szKeyName, "File[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ,
				(char*)m_pShareData->m_MacroTable[i].m_szFile, _MAX_PATH );
		}
		//	To Here  Sep. 14, 2001 genta

//	MYTRACE( "Iniファイル処理 6 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

//Section02:;
//	/* 変更フラグ(共通設定の全体)のクリア */
//	m_pShareData->m_nCommonModify = FALSE;

//	/* 変更フラグ フォント */
//	m_pShareData->m_bFontModify = FALSE;

//	m_pShareData->m_bKeyBindModify = FALSE;	/* 変更フラグ キー割り当て */
//	for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
//		m_pShareData->m_bKeyBindModifyArr[i] = FALSE;	/* 変更フラグ キー割り当て(キーごと) */
//	}
//	/* 変更フラグ(印刷の全体)のクリア */
//	m_pShareData->m_bPrintSettingModify = FALSE;
//	/* 変更フラグ(印刷設定ごと)のクリア */
//	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//		m_pShareData->m_bPrintSettingModifyArr[i] = FALSE;
//	}
//	m_pShareData->m_bKeyWordSetModify = FALSE;	/* 変更フラグ(キーワードの全体) */
//	for( i = 0; i < MAX_SETNUM; ++i ){
//		m_pShareData->m_bKeyWordSetModifyArr[i] = FALSE;	/* 変更フラグ(キーワードのセットごと) */
//	}
//	/* 変更フラグ(タイプ別設定) のクリア */
//	for( i = 0; i < MAX_TYPES; ++i ){
//		m_pShareData->m_nTypesModifyArr[i] = FALSE;
//	}

//	MYTRACE( "Iniファイル処理 7 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );
//	cRunningTimer.Reset();

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
			if( TRUE == pcProfile->IOProfileData( bRead, pszSecName, szKeyName, REGCNV_SZ2SZ, (char*)szKeyData, 0 ) ){
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
