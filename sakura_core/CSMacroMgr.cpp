//	$Id$
/*!	@file
	@brief マクロ

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	@date 20011229 aroka バグ修正、コメント追加
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CSMacroMgr.h"
#include "CEditView.h"

CSMacroMgr::CSMacroMgr()
{
	CShareData	m_cShareData;
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
}

CSMacroMgr::~CSMacroMgr()
{
}

/*! キーマクロのバッファをクリアする */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		m_cKeyMacro[i].ClearAll();
	}
}

/*! @briefキーマクロのバッファにデータ追加

	@param nFuncID [in] 機能番号
	@param lParam1 [in] パラメータ。
	@param mbuf [in] 読み込み先マクロバッファ

*/
int CSMacroMgr::Append( int num, /*CSMacroMgr::Macro1& mbuf, */ int nFuncID, LPARAM lParam1 )
{
	m_cKeyMacro[num].Append( nFuncID, lParam1 );
	return TRUE;
}


/*!	@brief キーボードマクロの実行

	CShareDataからファイル名を取得し、実行する。

	@param hInstance [in] インスタンス
	@param hwndParent [in] 親ウィンドウの
	@param pViewClass [in] macro実行対象のView
	@param idx [in] マクロ番号。
*/
BOOL CSMacroMgr::Exec( HINSTANCE hInstance, CEditView* pCEditView, int idx )
{
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	範囲チェック
		return FALSE;

	if( !m_cKeyMacro[idx].IsReady() ){
		//	ファイル名を、m_pShareDataから取得。
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
		
		if( !Load( idx, hInstance, ptr ))
			return FALSE;
	}

	m_cKeyMacro[idx].ExecKeyMacro(pCEditView);
	pCEditView->Redraw();	//	必要？
	return TRUE;
}

/*! キーボードマクロの読み込み

	@param num [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名

	@author Norio Nakatani
*/
BOOL CSMacroMgr::Load( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	if ( idx < 0 || MAX_CUSTMACRO <= idx ){
		return FALSE;
	}
	/* キーマクロのバッファをクリアする */
	m_cKeyMacro[idx].ClearAll();
	return m_cKeyMacro[idx].LoadKeyMacro(hInstance, pszPath );
}


/*[EOF]*/
