//	$Id$
/*!	@file
	@brief マクロ

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "CMacro.h"
#include "CSMacroMgr.h"
#include "debug.h"
#include "charcode.h"
#include "etc_uty.h"
#include "global.h"
#include "CEditView.h"

CSMacroMgr::CSMacroMgr()
	: m_vMacro(MAX_CUSTMACRO)
{
	CShareData	m_cShareData;
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
}

CSMacroMgr::~CSMacroMgr()
{
}

/* キーマクロのバッファをクリアする */
void CSMacroMgr::ClearAll( void )
{
	for( vector<Macro1>::iterator ptr = m_vMacro.begin(); ptr != m_vMacro.end(); ++ptr ){
		ptr->Reset();
	}
}
/*! @briefキーマクロのバッファにデータ追加

	@param nFuncID [in] 機能番号
	@param lParam1 [in] パラメータ。
	@param mbuf [in] 読み込み先マクロバッファ

*/
int CSMacroMgr::Append( CSMacroMgr::Macro1& mbuf, int nFuncID, LPARAM lParam1 )
{
	KeyMacroData dat;

	switch( nFuncID ){
	case F_INSTEXT:
	case F_FILEOPEN:
	case F_EXECCOMMAND:
		mbuf.m_strlist.push_back( (const char*)lParam1 );
		dat.m_nFuncID = nFuncID;
		dat.m_lParam1 = mbuf.m_strlist.size() - 1;
		mbuf.m_mac.push_back( dat );
		break;
	default:
		dat.m_nFuncID = nFuncID;
		dat.m_lParam1 = lParam1;
		mbuf.m_mac.push_back( dat );
		break;
	}
	return TRUE;
}


/*!	@brief キーボードマクロの実行

	@param hInstance [in] インスタンス
	@param hwndParent [in] 親ウィンドウの
	@param pViewClass [in] macro実行対象のView
	@param idx [in] マクロ番号。
*/
BOOL CSMacroMgr::Exec( HINSTANCE hInstance, CEditView* pCEditView, int idx )
{
//	CEditView*	pCEditView = (CEditView*)pViewClass;
//	int			i;

	//::MessageBox( NULL, "Enter", "CSMacroMgr::Exec", MB_OK );

	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	範囲チェック
		return FALSE;

	char testbuf[100];
	wsprintf( testbuf, "idx: %d / max: %d", idx, m_vMacro.size());
	//::MessageBox( pCEditView->m_hwndParent, testbuf, "CSMacroMgr::Exec", MB_OK );

	CSMacroMgr::Macro1& mref = m_vMacro[idx];

	if( !mref.IsReady() ){
		
		//::MessageBox( pCEditView->m_hwndParent, "GetShareData", "CSMacroMgr::Exec", MB_OK );

		if( !m_pShareData->m_MacroTable[idx].IsEnabled() )
			return FALSE;
		
		char fbuf[_MAX_PATH * 2];
		char *ptr = m_pShareData->m_MacroTable[idx].m_szFile;

		if( ptr[0] == '\0' )	//	ファイル名が無い
			return FALSE;
		
		if( ptr[0] == '\\' || ( ptr[1] == ':' && ptr[2] == '\\' )){	// 絶対パス
		}
		else if( m_pShareData->m_szMACROFOLDER[0] != '\0' ){	//	フォルダ指定あり
			//	相対パス→絶対パス
			strcpy( fbuf, m_pShareData->m_szMACROFOLDER );
			ptr = fbuf + strlen( fbuf );
			//::MessageBox( pCEditView->m_hwndParent, ptr - 1, "CSMacroMgr::Exec/folder", MB_OK );
			if( ptr[-1] != '\\' ){
				*ptr++ = '\\';
			}
			strcpy( ptr, m_pShareData->m_MacroTable[idx].m_szFile );
			ptr = fbuf;
		}
		
		//::MessageBox( pCEditView->m_hwndParent, ptr, "CSMacroMgr::Exec", MB_OK );
		
		if( !Load( mref, hInstance, ptr ))
			return FALSE;
	}

	for( KeyMacroList::iterator ptr = mref.m_mac.begin(); ptr != mref.m_mac.end(); ++ptr ){
		switch( ptr->m_nFuncID ){
		case F_INSTEXT:
		case F_FILEOPEN:
		case F_EXECCOMMAND:
			//::MessageBox( pCEditView->m_hwndParent, mref.m_strlist[ptr->m_lParam1].c_str(), "CSMacroMgr::Exec/INSTEXT", MB_OK );
			pCEditView->HandleCommand( ptr->m_nFuncID, FALSE, (LONG)(mref.m_strlist[ptr->m_lParam1].c_str()), 0, 0, 0 );
			break;
		default:
			pCEditView->HandleCommand( ptr->m_nFuncID, FALSE, ptr->m_lParam1, 0, 0, 0 );
			break;
		}
	}
	pCEditView->Redraw();
	return TRUE;
}

/*! キーボードマクロの読み込み

	@param mbuf [in] 読み込み先マクロバッファ
	@param pszPath [in] マクロファイル名

	@author Norio Nakatani
*/
BOOL CSMacroMgr::Load( CSMacroMgr::Macro1& mbuf, HINSTANCE hInstance, const char* pszPath )
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
	mbuf.Reset();

	if( NULL == pFile ){
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
			//	Skip Space
			while( szLine[i] == ' ' || szLine[i] == '\t' )
				i++;

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
					}
				}

				cmemWork.SetData( szLine + nBgn, i - nBgn );
				cmemWork.Replace( "\\\"", "\"" );
				cmemWork.Replace( "\\\\", "\\" );
				/* キーマクロのバッファにデータ追加 */
				//::MessageBox( NULL, cmemWork.GetPtr( NULL ), "CSMacroMgr::Load/INSTEXT", MB_OK );
				Append( mbuf, nFuncID, (LPARAM)cmemWork.GetPtr( NULL ) );
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
				Append( mbuf, nFuncID, lParam1 );
			}
		}
	}
	fclose( pFile );
	mbuf.m_flag = TRUE;
	return TRUE;
}

//--------------------------------------------------------------
//	マクロ一覧へのアクセスインターフェース
//--------------------------------------------------------------

//!	表示名の設定
BOOL CSMacroMgr::SetName(int idx, const char *)
{
	return TRUE;
}

//!	ファイル名の設定
BOOL CSMacroMgr::SetFile(int idx, const char *)
{
	return TRUE;
}
/*[EOF]*/
