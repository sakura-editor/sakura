/*!	@file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "ppa.h"

#include <memory>

/*!
	PPAスタブ

	PPAから呼ばれるコールバック関数をテストするためのもの。
	DLLロード時に生成し、アンロード時に破棄する。
	テスト目的なのでマルチスレッドは考慮していない。
	マルチスレッド考慮が必要と判明したら拡張すること。
 */
class CPpaStub {
private:
	static std::unique_ptr<CPpaStub> gm_Instance;

	decltype(&ppaProc)			m_pfnProc = nullptr;
	decltype(&ppaIntFunc)		m_pfnIntFunc = nullptr;
	decltype(&ppaStrFunc)		m_pfnStrFunc = nullptr;
	decltype(&ppaStrObj)		m_pfnStrObj = nullptr;
	decltype(&ppaErrorProc)		m_pfnErrorProc = nullptr;
	decltype(&ppaFinishProc)	m_pfnFinishProc = nullptr;

public:
	static void createInstance();
	static CPpaStub* getInstance() noexcept;
	static void releaseInstance() noexcept;

	void	SetProc(decltype(&ppaProc) p);
	void	SetIntFunc(decltype(&ppaIntFunc) p);
	void	SetStrFunc(decltype(&ppaStrFunc) p);
	void	SetStrObj(decltype(&ppaStrObj) p);
	void	SetErrorProc(decltype(&ppaErrorProc) p);
	void	SetFinishProc(decltype(&ppaFinishProc) p);
};
