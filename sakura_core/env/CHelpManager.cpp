/*
	2008.05.18 kobake CShareData から分離
*/
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
const TCHAR* CHelpManager::GetExtWinHelp( const STypeConfig* type )
{
	if (type && type->m_szExtHelp[0] != _T('\0')){
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
const TCHAR* CHelpManager::GetExtHTMLHelp( const STypeConfig* type )
{
	if (type && type->m_szExtHtmlHelp[0] != _T('\0')){
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
