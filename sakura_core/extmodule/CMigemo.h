/*!	@file
	@brief Migemo関連

	C/Migemoアクセス関数

	@author isearch
	@date 2004.09.14 新規作成
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, aroka
	Copyright (C) 2009, miau
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
/*
Migemo はローマ字のまま日本語をインクリメンタル検索するためのツールです。
*/

#ifndef SAKURA_CMIGEMO_BEF7C3E9_DC35_4C9D_BC69_8B7F9EBB4463_H_
#define SAKURA_CMIGEMO_BEF7C3E9_DC35_4C9D_BC69_8B7F9EBB4463_H_
#pragma once

#include "basis/primitive.h"

#include "extmodule/CDllHandler.h"
#include "util/design_template.h"

#include "cmigemo/migemo.h"

class CMigemo : public CDllImp, public TSingleInstance<CMigemo> {
public:
	CMigemo() = default;
	~CMigemo() override;

private:
	//	Entry Point
	decltype(&::migemo_open)                m_migemo_open = nullptr;
	decltype(&::migemo_close)               m_migemo_close = nullptr;
	decltype(&::migemo_query)               m_migemo_query = nullptr;
	decltype(&::migemo_release)             m_migemo_release = nullptr;
	decltype(&::migemo_setproc_char2int)    m_migemo_setproc_char2int = nullptr;
	decltype(&::migemo_setproc_int2char)    m_migemo_setproc_int2char = nullptr;
	decltype(&::migemo_load)                m_migemo_load = nullptr;
	decltype(&::migemo_is_enable)           m_migemo_is_enable = nullptr;

	/**
	 * Migemoオブジェクト。migemo_open()で作成され、migemo_closeで破棄される。
	 */
	migemo* m_migemo = nullptr;

	bool	m_bUtf8 = false;

protected:
	LPCWSTR GetDllNameImp(int nIndex) override;

private:
	bool InitDllImp() override;
	bool DeinitDllImp(void) override;

	std::string_view _migemo_query(const std::string& query) noexcept;
	void _migemo_release(std::string_view found) noexcept;
	int _migemo_load(int dict_id, const std::filesystem::path& dict_file) noexcept;

public:
	int				migemo_load_all() noexcept;
	int				migemo_is_enable() noexcept;
	std::wstring	migemo_query_w(std::wstring_view query) noexcept;
};

#endif /* SAKURA_CMIGEMO_BEF7C3E9_DC35_4C9D_BC69_8B7F9EBB4463_H_ */
