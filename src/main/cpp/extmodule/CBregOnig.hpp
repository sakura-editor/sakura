/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_
#define SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_
#pragma once

#include "extmodule/CDllHandler.h"

#include "bregonig/bregexp.h"

//!BREGONIG.DLLをラップしたもの。
//2007.09.13 kobake 作成
class CBregOnig : public CDllImp
{
public:
	struct CPattern {
		using Me = CPattern;

		const CBregOnig&	m_cDll;					//!< BregOnigクラスへのポインタ
		BREGEXP*			m_pRegExp = nullptr;	//!< コンパイル構造体
		std::wstring_view	m_Target = L"";			//!< 対象文字列へのポインタ
		std::wstring		m_Msg;					//!< bregonig.dllからのメッセージ

		CPattern(
			const CBregOnig& cDll,
			BREGEXP* pRegExp,
			const std::wstring& msg
		) noexcept;

		CPattern(const Me&) = delete;
		Me& operator = (const Me&) = delete;

		CPattern(Me&&) noexcept;
		Me& operator = (Me&&) noexcept = default;

		~CPattern() noexcept;

		bool	Match(std::wstring_view target, size_t offset = 0);						//!< 検索を実行する
		int		Replace(std::wstring_view target, size_t offset = 0);					//!< 置換を実行する	// 2007.01.16 ryoji 戻り値を置換個数に変更

		auto starti() const noexcept {
			return m_pRegExp->startp ? *m_pRegExp->startp - std::data(m_Target) : 0;
		}
		auto endi() const noexcept {
			return m_pRegExp->endp ? *m_pRegExp->endp - std::data(m_Target) : 0;
		}
		auto matched() const noexcept {
			const auto start = starti();
			const auto end = endi();
			if (m_Target.empty() || m_Target.length() <= start || m_Target.length() <= end) {
				return std::wstring_view{};
			}
			return m_Target.substr(start, end - start);
		}
		auto replaced() const noexcept {
			if (!m_pRegExp->outp) {
				return std::wstring_view{};
			}
			return std::wstring_view{ m_pRegExp->outp, m_pRegExp->outendp };
		}
	};
	using CPatternHolder = std::unique_ptr<CPattern>;

protected:
	static std::wstring_view	gm_DummyStr;

private:
	using Me = CBregOnig;

public:
	CBregOnig() = default;
	~CBregOnig() override = default;

	std::wstring_view BRegexpVersionW() const noexcept
	{
		return m_BRegexpVersion();
	}

protected:
	// CDllImpインターフェース
	LPCWSTR	GetDllNameImp(int nIndex) override;

	bool	InitDllImp() override;

	int		BMatchExW(BREGEXP** rxp, std::span<WCHAR> msg, std::wstring_view target, size_t offset, const std::optional<std::wstring>& optQuotedRegex = std::nullopt) const noexcept;
	int		BSubstExW(BREGEXP** rxp, std::span<WCHAR> msg, std::wstring_view target, size_t offset, const std::optional<std::wstring>& optQuotedRegex = std::nullopt) const noexcept;

private:
	void BRegfreeW(BREGEXP* rx) const noexcept
	{
		return m_BRegfree(rx);
	}

	//DLL内関数ポインタ
	decltype(&::BMatchExW)       m_BMatchEx = nullptr;
	decltype(&::BSubstExW)       m_BSubstEx = nullptr;
	decltype(&::BRegexpVersionW) m_BRegexpVersion = nullptr;
	decltype(&::BRegfreeW)       m_BRegfree = nullptr;
};

#endif /* SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_ */
