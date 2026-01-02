/*! @file */
/*
	Copyright (C) 2021-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include "cxx/TComImpl.hpp"

/*!
 * @brief COMエラー情報クラス
 *
 * Windowsのエラー情報を提供するために使用するクラスです。
 * _com_raise_error関数にエラー情報を渡すことにより、日本語メッセージを含む例外を投げることができます。
 *
 * @see https://docs.microsoft.com/en-us/windows/win32/api/oaidl/nn-oaidl-ierrorinfo
 * @see https://docs.microsoft.com/en-us/cpp/cpp/com-raise-error
 */
class CErrorInfo : public cxx::TComImpl<IErrorInfo> {
private:
	using Base = cxx::TComImpl<IErrorInfo>;
	using Me = CErrorInfo;

	_bstr_t		bstrSource_;
	_bstr_t		bstrDescription_;

public:
	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<CErrorInfo, Args...>
	{
		return Base::template make_instance<CErrorInfo>(std::forward<Args>(args)...);
	}

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
#define MakeMsgError(msg) CErrorInfo::make_instance(std::format(L"{}({})", std::filesystem::path(std::source_location::current().file_name()).wstring(), std::source_location::current().line()), (msg))
