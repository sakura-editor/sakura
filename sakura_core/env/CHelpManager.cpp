/*
	2008.05.18 kobake CShareData から分離
*/

#include "stdafx.h"
#include "CHelpManager.h"


/*!	外部Winヘルプが設定されているか確認。
*/
bool CHelpManager::ExtWinHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != L'\0'){
		return true;	//	共通設定に設定されている
	}
	if (!nTypeNo.IsValid()) {
		return false;	//	共通設定に設定されていない＆nTypeNoが範囲外。
	}
	if (CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHelp[0] != L'\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const TCHAR* CHelpManager::GetExtWinHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHelp[0] != _T('\0')){
		return CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}

/*!	外部HTMLヘルプが設定されているか確認。
*/
bool CHelpManager::ExtHTMLHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	共通設定に設定されている
	}
	if (!nTypeNo.IsValid()){
		return false;	//	共通設定に設定されていない＆nTypeNoが範囲外。
	}
	if (nTypeNo->m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const TCHAR* CHelpManager::GetExtHTMLHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != _T('\0')){
		return CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}

/*!	ビューアを複数起動しないがONかを返す。
*/
bool CHelpManager::HTMLHelpIsSingle( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != L'\0'){
		return (CDocTypeManager().GetTypeSetting(nTypeNo).m_bHtmlHelpIsSingle != FALSE);
	}
	
	return (m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle != FALSE);
}
