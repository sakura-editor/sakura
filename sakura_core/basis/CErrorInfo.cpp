/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
