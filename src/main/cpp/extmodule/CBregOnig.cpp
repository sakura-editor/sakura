/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "extmodule/CBregOnig.hpp"

#include "env/DLLSHAREDATA.h"

CBregOnig::CBregOnig() = default;

CBregOnig::~CBregOnig() = default;

/*!
	@date 2001.07.05 genta 引数追加。ただし、ここでは使わない。
	@date 2007.06.25 genta 複数のDLL名に対応
	@date 2007.09.13 genta サーチルールを変更
		@li 指定有りの場合はそれのみを返す
		@li 指定無し(NULLまたは空文字列)の場合はBREGONIG, BREGEXPの順で試みる
*/
LPCWSTR CBregOnig::GetDllNameImp(int index)
{
	UNREFERENCED_PARAMETER(index); // ←CDllImplの再設計を推奨

	const auto& szBregOnigDll = GetDllShareData().m_Common.m_sSearch.m_szRegexpLib;

	if (std::filesystem::path dllPath{ szBregOnigDll }; !dllPath.empty()) {
		// 相対パスはiniファイル基準に変換
		if(dllPath.is_relative()) {
			dllPath = GetIniFileName().parent_path() / dllPath;
		}

		// 指定されたパスが存在する場合はそれを使う
		if (fexist(dllPath)) {
			return szBregOnigDll;
		}
	}

	// デフォルトのDLL名を返す
	return L"bregonig.dll";
}

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval true 成功
	@retval false アドレス取得に失敗
*/
bool CBregOnig::InitDllImp()
{
	//DLL内関数名リスト
	const ImportTable table[] = {
		{ &m_BRegfree,			"BRegfreeW" },
		{ &m_BRegexpVersion,	"BRegexpVersionW" },
		{ &m_BMatchEx,			"BMatchExW" },
		{ &m_BSubstEx,			"BSubstExW" },
		{ nullptr, nullptr }
	};
	
	if( ! RegisterEntries( table )){
		return false;
	}
	
	return true;
}
