/*!	@file
	@brief 表示用文字列等の取得

	機能名，機能分類，機能番号などの変換．設定画面での表示用文字列を用意する．

	@author genta
	@date Oct.  1, 2001 マクロ
	@date Oct. 15, 2001 カスタムメニュー
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2007, ryoji

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

#include "stdafx.h"
#include "CFuncLookup.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
#include <stdio.h>
#include "CNormalProcess.h"

//	オフセット値
const int LUOFFSET_MACRO = 0;
const int LUOFFSET_CUSTMENU = 1;

//! 動的に内容が変わる分類の名前
const TCHAR *DynCategory[] = {
	_T("外部マクロ"),
	_T("カスタムメニュー")
};

/*!	@brief 分類中の位置に対応する機能番号を返す．

	@param category [in] 分類番号 (0-)
	@param position [in] 分類中のindex (0-)
	@param bGetUnavailable [in] 未登録マクロでも機能番号を返す

	@retval 機能番号

	@date 2007.11.02 ryoji bGetUnavailableパラメータ追加
*/
EFunctionCode CFuncLookup::Pos2FuncCode( int category, int position, bool bGetUnavailable ) const
{
	if( category < 0 || position < 0 )
		return F_DISABLE;

	if( category < nsFuncCode::nFuncKindNum ){
		if( position < nsFuncCode::pnFuncListNumArr[category] )
			return nsFuncCode::ppnFuncListArr[category][position];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		//	キー割り当てマクロ
		if( position < MAX_CUSTMACRO ){
			if( bGetUnavailable || m_pMacroRec[position].IsEnabled() )
				return (EFunctionCode)(F_USERMACRO_0 + position);
		}
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		//	キー割り当てマクロ
		if( position == 0 )
			return F_MENU_RBUTTON;
		else if( position < MAX_CUSTOM_MENU )
			return (EFunctionCode)(F_CUSTMENU_BASE + position);
	}
	return F_DISABLE;
}

/*!	@brief 分類中の位置に対応する機能名称を返す．

	@retval true 指定された機能番号は定義されている
	@retval false 指定された機能番号は未定義

	@date 2007.11.02 ryoji 処理を簡素化
*/
bool CFuncLookup::Pos2FuncName(
	int		category,	//!< [in]  分類番号 (0-)
	int		position,	//!< [in]  分類中のindex (0-)
	WCHAR*	ptr,		//!< [out] 文字列を格納するバッファの先頭
	int		bufsize		//!< [in]  文字列を格納するバッファのサイズ
) const
{
	int funccode = Pos2FuncCode( category, position );
	return Funccode2Name( funccode, ptr, bufsize );
}

/*!	@brief 機能番号に対応する機能名称を返す．

	@param funccode [in] 機能番号
	@param ptr [out] 文字列を格納するバッファの先頭
	@param bufsize [in] 文字列を格納するバッファのサイズ
	
	@retval true 指定された機能番号は定義されている
	@retval false 指定された機能番号は未定義

	@date 2007.11.02 ryoji 未登録マクロも文字列を格納．戻り値の意味を変更（文字列は必ず格納）．
*/
bool CFuncLookup::Funccode2Name( int funccode, WCHAR* ptr, int bufsize ) const
{
	if( F_USERMACRO_0 <= funccode && funccode < F_USERMACRO_0 + MAX_CUSTMACRO ){
		int position = funccode - F_USERMACRO_0;
		if( m_pMacroRec[position].IsEnabled() ){
			const TCHAR *p = m_pMacroRec[position].GetTitle();
			_tcstowcs( ptr, p, bufsize - 1 );
			ptr[ bufsize - 1 ] = LTEXT('\0');
		}else{
			_snwprintf( ptr, bufsize, LTEXT("マクロ %d (未登録)"), position );
			ptr[ bufsize - 1 ] = LTEXT('\0');
		}
		return true;
	}
	else if( funccode == F_MENU_RBUTTON ){
		wcsncpy( ptr, m_pCommon->m_sCustomMenu.m_szCustMenuNameArr[0], bufsize );
		ptr[bufsize-1] = LTEXT('\0');
		return true;
	}
	else if( F_CUSTMENU_1 <= funccode && funccode < F_CUSTMENU_BASE + MAX_CUSTMACRO ){
		wcsncpy( ptr, m_pCommon->m_sCustomMenu.m_szCustMenuNameArr[ funccode - F_CUSTMENU_BASE ], bufsize );
		ptr[bufsize-1] = LTEXT('\0');
		return true;
	}
	else if( F_MENU_FIRST <= funccode && funccode < F_MENU_NOT_USED_FIRST ){
		if( ::LoadStringW_AnyBuild( CNormalProcess::Instance()->GetProcessInstance(), funccode, ptr, bufsize ) > 0 ){
			return true;	// 定義されたコマンド
		}
	}

	// 未定義コマンド
	if( ::LoadStringW_AnyBuild( CNormalProcess::Instance()->GetProcessInstance(), F_DISABLE, ptr, bufsize ) > 0 ){
		return false;
	}
	ptr[0] = LTEXT('\0');
	return false;
}

/*!	@brief 機能分類番号に対応する機能名称を返す．

	@param category [in] 機能分類番号
	
	@return NULL 分類名称．取得に失敗したらNULL．
*/
const TCHAR* CFuncLookup::Category2Name( int category ) const
{
	if( category < 0 )
		return NULL;

	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppszFuncKind[category];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		return DynCategory[0];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		return DynCategory[1];
	}
	return NULL;
}

/*!	@brief ComboBoxに利用可能な機能分類一覧を登録する

	@param hComboBox [in(out)] データを設定するコンボボックス
*/
void CFuncLookup::SetCategory2Combo( HWND hComboBox ) const
{
	int i;

	//	リストを初期化する
	::SendMessageAny( hComboBox, CB_RESETCONTENT, 0, (LPARAM)0 );

	//	固定機能リスト
	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
		Combo_AddString( hComboBox, nsFuncCode::ppszFuncKind[i] );
	}

	//	ユーザマクロ
	Combo_AddString( hComboBox, DynCategory[0] );
	//	カスタムメニュー
	Combo_AddString( hComboBox, DynCategory[1] );
}

/*!	@brief 指定された分類に属する機能リストをListBoxに登録する．
	
	@param hListBox [in(out)] 値を設定するリストボックス
	@param category [in] 機能分類番号

	@date 2007.11.02 ryoji 未定義コマンドは除外．処理も簡素化．
*/
void CFuncLookup::SetListItem( HWND hListBox, int category ) const
{
	WCHAR pszLabel[256];
	int n;
	int i;

	//	リストを初期化する
	::SendMessageAny( hListBox, LB_RESETCONTENT , 0, (LPARAM)0 );

	n = GetItemCount( category );
	for( i = 0; i < n; i++ ){
		if( Pos2FuncCode( category, i ) == F_DISABLE )
			continue;
		Pos2FuncName( category, i, pszLabel, _countof(pszLabel) );
		List_AddString( hListBox, pszLabel );
	}
}

/*!
	指定分類中の機能数を取得する．
	
	@param category [in] 機能分類番号
*/
int CFuncLookup::GetItemCount(int category) const
{
	if( category < 0 )
		return 0;

	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::pnFuncListNumArr[category];
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO ){
		//	マクロ
		return MAX_CUSTMACRO;
	}
	else if( category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU ){
		//	マクロ
		return MAX_CUSTOM_MENU;
	}
	return 0;
}



