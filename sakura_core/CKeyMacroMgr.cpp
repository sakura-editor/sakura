//	$Id$
/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "CMacro.h"
#include "CKeyMacroMgr.h"
#include "debug.h"
#include "charcode.h"
#include "etc_uty.h"
#include "global.h"
#include "CEditView.h"

CKeyMacroMgr::CKeyMacroMgr()
{
	m_nKeyMacroDataArrNum = 0;
	return;
}

CKeyMacroMgr::~CKeyMacroMgr()
{
	/* キーマクロのバッファをクリアする */
	Clear();
	return;
}


/*! キーマクロのバッファをクリアする */
void CKeyMacroMgr::Clear( void )
{
	m_nKeyMacroDataArrNum = 0;
	return;

}
/*! キーマクロのバッファにデータ追加 */
int CKeyMacroMgr::Append( int nFuncID, LPARAM lParam1 )
{
	if( m_nKeyMacroDataArrNum + 1 > MAX_KEYMACRONUM ){
		return m_nKeyMacroDataArrNum;
	}

	switch( nFuncID ){
	case F_INSTEXT:
		m_pKeyMacroDataArr[m_nKeyMacroDataArrNum].m_nFuncID = nFuncID;
		m_pKeyMacroDataArr[m_nKeyMacroDataArrNum].m_lParam1 = 0;
		strcpy( m_szKeyMacroDataArr[m_nKeyMacroDataArrNum], (const char*)lParam1 );
		break;
	default:
		m_pKeyMacroDataArr[m_nKeyMacroDataArrNum].m_nFuncID = nFuncID;
		m_pKeyMacroDataArr[m_nKeyMacroDataArrNum].m_lParam1 = lParam1;
		break;
	}
	m_nKeyMacroDataArrNum++;

	return m_nKeyMacroDataArrNum;
}




/*! キーボードマクロの保存 */
BOOL CKeyMacroMgr::SaveKeyMacro( HINSTANCE hInstance, HWND hwndParent, const char* pszPath )
{
	int			i;
	HFILE		hFile;
	char		szFuncName[500];
	char		szFuncNameJapanese[500];
	char		szLine[1024];
	int			nPos;
	const char*	pText;
	int			nTextLen;
	CMemory		cmemWork;
	hFile = _lcreat( pszPath, 0 );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを作成できませんでした。\n\n%s", pszPath
		);
		return FALSE;
	}
	strcpy( szLine, "//キーボードマクロのファイル\r\n" );
	_lwrite( hFile, szLine, strlen( szLine ) );
	for( i = 0; i < m_nKeyMacroDataArrNum; ++i ){
		/* 機能ID→関数名，機能名日本語 */
		if( NULL != CMacro::GetFuncInfoByID( hInstance, m_pKeyMacroDataArr[i].m_nFuncID, szFuncName, szFuncNameJapanese ) ){
			switch( m_pKeyMacroDataArr[i].m_nFuncID ){
			case F_INSTEXT:
				nPos = 0;
				/* 指定長以下のテキストに切り分ける */
				while( NULL != ( pText = GetNextLimitedLengthText( m_szKeyMacroDataArr[i], strlen( m_szKeyMacroDataArr[i] ), 11/*MAX_STRLEN - 1*/, &nTextLen, &nPos ) ) ){
					cmemWork.SetData( pText, nTextLen );
					cmemWork.Replace( "\\\"", "\"" );
					cmemWork.Replace( "\\\\", "\\" );
					wsprintf( szLine, "%s(\"%s\");\t/* %s */\r\n", szFuncName, cmemWork.GetPtr( NULL ), szFuncNameJapanese );
					_lwrite( hFile, szLine, strlen( szLine ) );
				}

//				cmemWork = m_cmemKeyMacroDataArr[i];
//				cmemWork.SetData( m_szKeyMacroDataArr[i], strlen( m_szKeyMacroDataArr[i] ) );
//				cmemWork.Replace( "\\", "\\\\" );
//				cmemWork.Replace( "\"", "\\\"" );
//				sprintf( szLine, "%s(\"%s\");\t/* %s */\r\n", szFuncName, cmemWork.GetPtr( NULL ), szFuncNameJapanese );
				break;
			default:
				if( 0 == m_pKeyMacroDataArr[i].m_lParam1 ){
					wsprintf( szLine, "%s();\t/* %s */\r\n", szFuncName, szFuncNameJapanese );
				}else{
					wsprintf( szLine, "%s(%d);\t/* %s */\r\n", szFuncName, m_pKeyMacroDataArr[i].m_lParam1, szFuncNameJapanese );
				}
				_lwrite( hFile, szLine, strlen( szLine ) );
				break;
			}

		}else{
			wsprintf( szLine, "CMacro::GetFuncInfoByID()に、バグがあるのでエラーが出ましたぁぁぁぁぁぁあああ\r\n" );
			_lwrite( hFile, szLine, strlen( szLine ) );
		}
	}
	_lclose( hFile );
	return TRUE;
}



/*! キーボードマクロの実行 */
BOOL CKeyMacroMgr::ExecKeyMacro( void* pViewClass )
{
	CEditView*	pCEditView = (CEditView*)pViewClass;
	int			i;
	for( i = 0; i < m_nKeyMacroDataArrNum; ++i ){
		switch( m_pKeyMacroDataArr[i].m_nFuncID ){
		case F_INSTEXT:
			pCEditView->HandleCommand( m_pKeyMacroDataArr[i].m_nFuncID, TRUE, (LONG)m_szKeyMacroDataArr[i], 0, 0, 0 );
			break;
		default:
			pCEditView->HandleCommand( m_pKeyMacroDataArr[i].m_nFuncID, TRUE, m_pKeyMacroDataArr[i].m_lParam1, 0, 0, 0 );
			break;
		}
	}
	return TRUE;
}

/*! キーボードマクロの読み込み */
BOOL CKeyMacroMgr::LoadKeyMacro( HINSTANCE hInstance, HWND hwndParent, const char* pszPath )
{
	FILE*	pFile;
	char	szLine[10240];
	int		nLineLen;
	char	szFuncName[100];
	char	szlParam[100];
	char	szFuncNameJapanese[256];
	LPARAM	lParam1;
	int		i;
	int		nBgn;
	int		nFuncID;
	CMemory cmemWork;
	pFile = fopen( pszPath, "r" );

	/* キーマクロのバッファをクリアする */
	Clear();

	if( NULL == pFile ){
		::MYMESSAGEBOX(	hwndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを開けませんでした。\n\n%s", pszPath
		);
		return FALSE;
	}
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) ){
		nLineLen = strlen( szLine );
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		if( i + 1 < nLineLen && szLine[i] == '/' && szLine[i + 1] == '/' ){
			continue;
		}
		for( ; i < nLineLen; ++i ){
			if( szLine[i] == '(' ){
				memcpy( szFuncName, &szLine[nBgn], i - nBgn );
				szFuncName[i - nBgn] = '\0';
				++i;
				nBgn = i;
				break;
			}
		}
//		MYTRACE( "szFuncName=[%s]\n", szFuncName );
		/* 関数名→機能ID，機能名日本語 */
		nFuncID = CMacro::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		if( -1 != nFuncID ){
			if( '\"' == szLine[i] ){
				++i;
				nBgn = i;
				for( ; i < nLineLen; ++i ){
					if( szLine[i] == '\\' ){
						++i;
						continue;
					}
					if( szLine[i] == '\"' ){
						break;
						break;
					}
				}

				int			nPos;
				const char*	pText;
				int			nTextLen;
				nPos = 0;
				/* 指定長以下のテキストに切り分ける */
				while( NULL != ( pText = GetNextLimitedLengthText( &szLine[nBgn], i - nBgn, 11/*MAX_STRLEN - 1*/, &nTextLen, &nPos ) ) ){
					cmemWork.SetData( pText, nTextLen );
					cmemWork.Replace( "\\\"", "\"" );
					cmemWork.Replace( "\\\\", "\\" );
					/* キーマクロのバッファにデータ追加 */
					Append( nFuncID, (LPARAM)cmemWork.GetPtr( NULL ) );
				}
//
//				cmemWork.SetData( &szLine[nBgn], i - nBgn );
//				cmemWork.Replace( "\\\"", "\"" );
//				cmemWork.Replace( "\\\\", "\\" );
//				/* キーマクロのバッファにデータ追加 */
//				Append( nFuncID, (LPARAM)cmemWork.GetPtr( NULL ) );
			}else{
				for( ; i < nLineLen; ++i ){
					if( szLine[i] == ')' ){
						memcpy( szlParam, &szLine[nBgn], i - nBgn );
						szlParam[i - nBgn] = '\0';
						lParam1 = atoi( szlParam );
						nBgn = i + 1;
						break;
					}
				}
				/* キーマクロのバッファにデータ追加 */
				Append( nFuncID, lParam1 );
			}
		}
	}
	fclose( pFile );
	return TRUE;
}


/*[EOF]*/
