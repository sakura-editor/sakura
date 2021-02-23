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
#pragma once

#include <Windows.h>

#include <comdef.h>
#include <OAIdl.h>

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
