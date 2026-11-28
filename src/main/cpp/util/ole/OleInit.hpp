/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <Windows.h>

namespace cxx {

/*!
 * @brief COMライブラリの初期化状態をRAIIで管理するクラス
 *
 * クラス生成と同時にSTAモードでCOMライブラリを初期化する。
 * クラス破棄時はOLEをシャットダウンする。
 */
class OleInit final
{
private:
	/*!
	 * @brief COMライブラリのクリーンアップ
	 */
	static void _CleanUp(
		const OleInit* pThis
	) noexcept;

	using Holder = cxx::ResourceHolder<&_CleanUp>;

	using Me = OleInit;

public:
	/*!
	 * @brief COMライブラリを初期化する
	 */
	OleInit() = default;

	OleInit(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	bool IsInitialized() const noexcept { return SUCCEEDED(m_hrOleInit); }

	explicit operator bool() const noexcept { return IsInitialized(); }

private:
	Holder		m_CleanupHolder{ this };
	HRESULT		m_hrOleInit = ::OleInitialize(nullptr);
};

}	// namespace cxx

