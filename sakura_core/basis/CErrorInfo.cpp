/*! @file */
/*
	Copyright (C) 2021, Sakura Editor Organization

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
#include "basis/CErrorInfo.h"

/*!
 * コンストラクタ
 */
CErrorInfo::CErrorInfo(
	std::wstring_view source,		//!< [in] ソース
	std::wstring_view description	//!< [in] 説明
)
	: bstrSource_(source.data())
	, bstrDescription_(description.data())
{
}

/*!
 * GUIDを取得する
 */
IFACEMETHODIMP CErrorInfo::GetGUID(GUID *pGUID)
{
	if (!pGUID) return E_POINTER;
	*pGUID = GUID_NULL;
	return S_OK;
}

/*!
 * ソース情報を取得する
 */
IFACEMETHODIMP CErrorInfo::GetSource(BSTR *pBstrSource)
{
	if (!pBstrSource) return E_POINTER;
	*pBstrSource = bstrSource_.copy();
	return S_OK;
}

/*!
 * 説明を取得する
 */
IFACEMETHODIMP CErrorInfo::GetDescription(BSTR *pBstrDescription)
{
	if (!pBstrDescription) return E_POINTER;
	*pBstrDescription = bstrDescription_.copy();
	return S_OK;
}

/*!
 * ヘルプファイルのパスを取得する
 */
IFACEMETHODIMP CErrorInfo::GetHelpFile(BSTR *pBstrHelpFile)
{
	if (!pBstrHelpFile) return E_POINTER;
	_bstr_t bstrHelpFile_;
	*pBstrHelpFile = bstrHelpFile_.copy();
	return S_OK;
}

/*!
 * ヘルプコンテキストを取得する
 */
IFACEMETHODIMP CErrorInfo::GetHelpContext(DWORD *pdwHelpContext)
{
	if (!pdwHelpContext) return E_POINTER;
	constexpr DWORD dwHelpContext_ = 0;
	*pdwHelpContext = dwHelpContext_;
	return S_OK;
}
