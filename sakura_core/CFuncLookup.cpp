//	$Id$
/*!	@file

	機能名，機能分類，機能番号などの変換．設定画面での表示用文字列を用意する．

	@author genta
	@date Oct. 1, 2001
*/

/*!	@brief 分類中の位置に対応する機能番号を返す．

	@param category [in] 分類番号 (0-)
	@param position [in] 分類中のindex (0-)
*/

#include "CFuncLookup.h"

const char *DynCategory[] = {
	"外部マクロ"
};

int CFuncLookup::Pos2FuncCode( int category, int position ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppnFuncListArr[category][position];
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	キー割り当てマクロ
		if( m_pcSMacroMgr->IsEnabled(position))
			return F_USERMACRO_0 + position;
	}
	return 0;
}

/*!	@brief 分類中の位置に対応する機能名称を返す．

	@param category [in] 分類番号 (0-)
	@param position [in] 分類中のindex (0-)
	@param ptr [out] 文字列を格納するバッファの先頭
	@param bufsize [in] 文字列を格納するバッファのサイズ
*/
bool CFuncLookup::Pos2FuncName( int category, int position, char *ptr, int bufsize ) const
{
	int func;
	if( category < nsFuncCode::nFuncKindNum ){
		func = nsFuncCode::ppnFuncListArr[category][position];
		return ( ::LoadString( m_hInstance, func, ptr, bufsize ) > 0 );
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	キー割り当てマクロ
		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	return true;
}

/*!	@brief 分類中の位置に対応する機能名称を返す．

	@param category [in] 分類番号 (0-)
	@param position [in] 分類中のindex (0-)
	@param ptr [out] 文字列を格納するバッファの先頭
	@param bufsize [in] 文字列を格納するバッファのサイズ
*/
bool CFuncLookup::Funccode2Name( int funccode, char *ptr, int bufsize ) const
{
	if( F_USERMACRO_0 <= funccode && funccode < F_USERMACRO_0 + MAX_CUSTMACRO ){
		int position = funccode - F_USERMACRO_0;
		if( !m_pcSMacroMgr->IsEnabled( position )){
			*ptr = '\0';
			return false;
		}

		const char *p = m_pcSMacroMgr->GetTitle( position );
		if( p == NULL )
			return false;
		strncpy( ptr, p, bufsize - 1 );
		ptr[ bufsize - 1 ] = '\0';
	}
	else {
		return ( ::LoadString( m_hInstance, funccode, ptr, bufsize ) > 0 );
	}
	return true;
}

const char* CFuncLookup::Category2Name( int category ) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::ppszFuncKind[category];
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		return DynCategory[0];
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
	::SendMessage( hComboBox, CB_RESETCONTENT, 0, (LPARAM)0 );

	//	固定機能リスト
	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
		::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
	}

	//	ユーザマクロ
	::SendMessage( hComboBox, CB_ADDSTRING, 0, (LPARAM)DynCategory[0] );
}

/*!	@brief 指定された分類に属する機能リストをListBoxに登録する．
	
	@param hListBox [in(out)] 値を設定するリストボックス
	@param category [in] 機能分類
*/
void CFuncLookup::SetListItem( HWND hListBox, int category ) const
{
	char pszLabel[256];
	int i;

	//	リストを初期化する
	::SendMessage( hListBox, LB_RESETCONTENT , 0, (LPARAM)0 );

	if( category < nsFuncCode::nFuncKindNum ){
		for( i = 0; i < nsFuncCode::pnFuncListNumArr[category]; ++i ){
			if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[category])[i], pszLabel, 255 ) ){
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)pszLabel );
			}else{
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)"--未定義--" );
			}
		}
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	マクロ
		for( i = 0; i < MAX_CUSTMACRO ; ++i ){
			if( m_pcSMacroMgr->IsEnabled(i)){
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)m_pcSMacroMgr->GetTitle(i));
			}
			else {
				::SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)"unavailable" );
			}
		}
	}
}

int CFuncLookup::GetItemCount(int category) const
{
	if( category < nsFuncCode::nFuncKindNum ){
		return nsFuncCode::pnFuncListNumArr[category];
	}
	else if( category == nsFuncCode::nFuncKindNum ){
		//	マクロ
		return MAX_CUSTMACRO;
	}
	return 0;
}
