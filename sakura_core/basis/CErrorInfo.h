/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include <Windows.h>

#include <comdef.h>
#include <oaidl.h>

#include <string_view>

#include "basis/TComImpl.hpp"

/*!
 * @brief COMエラー情報クラス
 *
 * Windowsのエラー情報を提供するために使用するクラスです。
 * _com_raise_error関数にエラー情報を渡すことにより、日本語メッセージを含む例外を投げることができます。
 *
 * @see https://docs.microsoft.com/en-us/windows/win32/api/oaidl/nn-oaidl-ierrorinfo
 * @see https://docs.microsoft.com/en-us/cpp/cpp/com-raise-error
 */
class CErrorInfo : public TComImpl<IErrorInfo> {
	_bstr_t		bstrSource_;
	_bstr_t		bstrDescription_;

public:
	CErrorInfo(
		std::wstring_view source,		//!< [in] ソース
		std::wstring_view description	//!< [in] 説明
	);

	IFACEMETHODIMP GetGUID(GUID *pGUID) override;
	IFACEMETHODIMP GetSource(BSTR *pBstrSource) override;
	IFACEMETHODIMP GetDescription(BSTR *pBstrDescription) override;
	IFACEMETHODIMP GetHelpFile(BSTR *pBstrHelpFile) override;
	IFACEMETHODIMP GetHelpContext(DWORD *pdwHelpContext) override;
};

//! メッセージからエラー情報を生成する
#define MakeMsgError(msg)	new CErrorInfo(_CRT_WIDE(__FILE__) L"(" _CRT_WIDE(_CRT_STRINGIZE(__LINE__)) L")", (msg))
