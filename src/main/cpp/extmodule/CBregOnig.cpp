/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "extmodule/CBregOnig.hpp"

#include "env/DLLSHAREDATA.h"

/*!
 * Compile時、行頭置換(len=0)の時にダミー文字列(１つに統一) by かろと
 */
std::wstring_view CBregOnig::gm_DummyStr(L"\0", 1);

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
	const std::array table = {
		ImportTable{ &m_BMatchEx,			"BMatchExW" },
		ImportTable{ &m_BSubstEx,			"BSubstExW" },
		ImportTable{ &m_BRegexpVersion,		"BRegexpVersionW" },
		ImportTable{ &m_BRegfree,			"BRegfreeW" },
		ImportTable{ nullptr, nullptr }
	};
	return RegisterEntries(std::data(table));
}

int CBregOnig::BMatchExW(BREGEXP** rxp, std::span<WCHAR> msg, std::wstring_view target, size_t offset, const std::optional<std::wstring>& optQuotedRegex) const noexcept
{
	auto targetbegp = std::data(target);
	auto targetp = targetbegp + offset;
	auto targetendp = targetbegp + std::size(target);
	return m_BMatchEx(
		// 検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
		LPWSTR(optQuotedRegex.has_value() ? std::data(optQuotedRegex.value()) : nullptr),
		LPWSTR(targetbegp),
		LPWSTR(targetp),
		LPWSTR(targetendp),
		rxp,
		std::data(msg)
	);
}

int CBregOnig::BSubstExW(BREGEXP** rxp, std::span<WCHAR> msg, std::wstring_view target, size_t offset, const std::optional<std::wstring>& optQuotedRegex) const noexcept
{
	auto targetbegp = std::data(target);
	auto targetp = targetbegp + offset;
	auto targetendp = targetbegp + std::size(target);
	return m_BSubstEx(
		// 検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
		LPWSTR(optQuotedRegex.has_value() ? std::data(optQuotedRegex.value()) : nullptr),
		LPWSTR(targetbegp),
		LPWSTR(targetp),
		LPWSTR(targetendp),
		rxp,
		std::data(msg)
	);
}

CBregOnig::CPattern::CPattern(
	const CBregOnig& cDll,
	BREGEXP* pRegExp,
	const std::wstring& msg
) noexcept
	: m_cDll(cDll)
	, m_pRegExp(pRegExp)
	, m_Msg(msg)
{
}

CBregOnig::CPattern::CPattern(Me&& other) noexcept
	: m_cDll(other.m_cDll)
	, m_Msg(other.m_Msg)
{
	std::swap(m_pRegExp, other.m_pRegExp);
}

CBregOnig::CPattern::~CPattern() noexcept
{
	if (m_pRegExp) {
		m_cDll.BRegfreeW(m_pRegExp);
		m_pRegExp = nullptr;
	}
}

bool CBregOnig::CPattern::Match(std::wstring_view target, size_t offset)
{
	if (m_pRegExp) {
		m_Target = L"";		//!< 対象文字列クリア
		m_Msg.clear();		//!< エラー解除
		if (const auto matched = m_cDll.BMatchExW(&m_pRegExp, m_Msg, target, offset); 0 <= matched && !m_Msg[0]) {
			m_Target = target;
			return 0 < matched;
		}
	}
	return false;
}

int CBregOnig::CPattern::Replace(std::wstring_view target, size_t offset)
{
	if (m_pRegExp) {
		m_Target = L"";		//!< 対象文字列クリア
		m_Msg.clear();		//!< エラー解除
		if (const auto result = m_cDll.BSubstExW(&m_pRegExp, m_Msg, target, offset); 0 <= result && !m_Msg[0]) {
			m_Target = target;
			return result;
		}
	}
	return 0;
}
