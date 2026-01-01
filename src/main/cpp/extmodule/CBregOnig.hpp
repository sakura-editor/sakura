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

using BREGEXP_W = BREGEXP;

//!BREGONIG.DLLをラップしたもの。
//2007.09.13 kobake 作成
class CBregOnig : public CDllImp
{
public:
	CBregOnig();
	~CBregOnig() override;

protected:
	// CDllImpインターフェース
	LPCWSTR	GetDllNameImp(int nIndex) override;

	bool	InitDllImp() override;

public:
	// UNICODEインターフェースを提供する
	int BMatchW(LPCWSTR str, LPCWSTR target, LPCWSTR targetendp, BREGEXP** rxp, std::span<WCHAR> msg) const noexcept
	{
		return m_BMatch(LPWSTR(str), LPWSTR(target), LPWSTR(targetendp), rxp, std::data(msg));
	}
	int BSubstW(LPCWSTR str, LPCWSTR target, LPCWSTR targetendp, BREGEXP** rxp, std::span<WCHAR> msg) const noexcept
	{
		return m_BSubst(LPWSTR(str), LPWSTR(target), LPWSTR(targetendp), rxp, std::data(msg));
	}

	LPCWSTR BRegexpVersionW(void) const noexcept
	{
		return m_BRegexpVersion();
	}
	void BRegfreeW(BREGEXP* rx) const noexcept
	{
		return m_BRegfree(rx);
	}

	int BMatchExW(LPCWSTR str, LPCWSTR targetbeg, LPCWSTR target, LPCWSTR targetendp, BREGEXP** rxp, std::span<WCHAR> msg) const noexcept
	{
		return m_BMatchEx(LPWSTR(str), LPWSTR(targetbeg), LPWSTR(target), LPWSTR(targetendp), rxp, std::data(msg));
	}
	int BSubstExW(LPCWSTR str, LPCWSTR targetbeg, LPCWSTR target, LPCWSTR targetendp, BREGEXP** rxp, std::span<WCHAR> msg) const noexcept
	{
		return m_BSubstEx(LPWSTR(str), LPWSTR(targetbeg), LPWSTR(target), LPWSTR(targetendp), rxp, std::data(msg));
	}

	// 関数があるかどうか
	bool ExistBMatchEx() const noexcept { return m_BMatchEx!=nullptr; }
	bool ExistBSubstEx() const noexcept { return m_BSubstEx!=nullptr; }

private:
	//DLL内関数ポインタ
	decltype(&::BMatchW)		 m_BMatch = nullptr;
	decltype(&::BSubstW)		 m_BSubst = nullptr;
	decltype(&::BRegfreeW)       m_BRegfree = nullptr;
	decltype(&::BRegexpVersionW) m_BRegexpVersion = nullptr;
	decltype(&::BMatchExW)       m_BMatchEx = nullptr;
	decltype(&::BSubstExW)       m_BSubstEx = nullptr;
};

#endif /* SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_ */
