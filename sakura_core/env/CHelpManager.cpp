/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "DLLSHAREDATA.h"

#include "CHelpManager.h"
#include "env/CDocTypeManager.h"

/*!	外部Winヘルプが設定されているか確認。
*/
bool CHelpManager::ExtWinHelpIsSet( const STypeConfig* type )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != L'\0'){
		return true;	//	共通設定に設定されている
	}
	if (type && type->m_szExtHelp[0] != L'\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const WCHAR* CHelpManager::GetExtWinHelp( const STypeConfig* type )
{
	if (type && type->m_szExtHelp[0] != L'\0'){
		return type->m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}

/*!	外部HTMLヘルプが設定されているか確認。
*/
bool CHelpManager::ExtHTMLHelpIsSet( const STypeConfig* type )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	共通設定に設定されている
	}
	if (type && type->m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const WCHAR* CHelpManager::GetExtHTMLHelp( const STypeConfig* type )
{
	if (type && type->m_szExtHtmlHelp[0] != L'\0'){
		return type->m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}

/*!	ビューアを複数起動しないがONかを返す。
*/
bool CHelpManager::HTMLHelpIsSingle( const STypeConfig* type )
{
	if (type && type->m_szExtHtmlHelp[0] != L'\0'){
		return type->m_bHtmlHelpIsSingle;
	}

	return m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle;
}
