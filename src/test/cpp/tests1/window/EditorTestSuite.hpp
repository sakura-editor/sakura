/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "env/ShareDataTestSuite.hpp"
#include "window/UiaTestSuite.hpp"

#include "_main/CCommandLine.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"

#include "CEditApp.h"

#include "agent/CGrepAgent.h"
#include "agent/CLoadAgent.h"
#include "agent/CSaveAgent.h"
#include "config/system_constants.h"
#include "macro/CSMacroMgr.h"
#include "uiparts/CVisualProgress.h"

namespace window {

struct EditorTestSuite : public env::ShareDataTestSuite
{
	static constexpr HINSTANCE unusedArg1 = nullptr;

	static inline std::filesystem::path dmpPath{};
	static inline std::filesystem::path iniBkPath{};
	static inline std::filesystem::path profIniPath{};

	static inline std::unique_ptr<CCommandLine> pCommandLine = nullptr;

	static inline std::unique_ptr<CEditDoc> pcEditDoc = nullptr;
	static inline std::unique_ptr<CEditWnd> pcEditWnd = nullptr;
	static inline std::unique_ptr<CSMacroMgr> pcSMacroMgr = nullptr;

	static inline std::unique_ptr<CLoadAgent> pcLoadAgent = nullptr;
	static inline std::unique_ptr<CSaveAgent> pcSaveAgent = nullptr;
	static inline std::unique_ptr<CVisualProgress> pcVisualProgress = nullptr;
	static inline std::unique_ptr<CGrepAgent> pcGrepAgent = nullptr;

	static void SetUpEditor();
	static void TearDownEditor();

	/*!
	 * 編集中ドキュメントの末尾に文字列を追加する
	 */
	void AddString(
		std::wstring_view text
	) const
	{
		auto& sWorkBuffer = GetDllShareDataPtr()->m_sWorkBuffer;
		auto buffer = std::span(sWorkBuffer.GetWorkBuffer<WCHAR>(), sWorkBuffer.GetWorkBufferCount<WCHAR>());

		wcscpy_s(buffer, text);

		HWND hWnd = nullptr;
		pcEditWnd->DispatchEvent(hWnd, MYWM_ADDSTRINGLEN_W, text.size(), 0L);
	}

	/*!
	 * エディターコマンドを実行する
	 * 
	 * @param[in] nFuncCode 機能コード
	 */
	void ExecEditorCommand(int nFuncCode) const
	{
		HWND hWnd = nullptr;
		FORWARD_WM_COMMAND(hWnd, nFuncCode, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
	}

	/*!
	 * キャレット位置(ロジック座標)を取得する
	 */
	CLogicPoint GetCaretPos() const
	{
		HWND hWnd = nullptr;
		pcEditWnd->DispatchEvent(hWnd, MYWM_GETCARETPOS, 0L, 0L);

		const auto& sWorkBuffer = GetDllShareData().m_sWorkBuffer;
		return sWorkBuffer.m_LogicPoint;
	}

	/*!
	 * 編集中の内容を取得する
	 */
	std::wstring GetLineData() const
	{
		auto pShareData = GetDllShareDataPtr();
		auto& sWorkBuffer = pShareData->m_sWorkBuffer;
		auto buffer = std::span(sWorkBuffer.GetWorkBuffer<WCHAR>(), sWorkBuffer.GetWorkBufferCount<WCHAR>());

		// 全データを取得する
		HWND hWnd = nullptr;
		const auto nLineLen = static_cast<size_t>(pcEditWnd->DispatchEvent(hWnd, MYWM_GETLINEDATA, 0L, 0L));

		std::wstring line{ std::data(buffer), nLineLen };
		return line;
	}

	/*!
	 * 編集中の内容を置き換えてキャレットを移動する
	 */
	void ResetText(
		std::optional<std::wstring> optText = std::nullopt,
		std::optional<CLogicPoint> optCaret = std::nullopt
	) const
	{
		ExecEditorCommand(F_SELECTALL);
		ExecEditorCommand(F_DELETE);

		if (optText.has_value()) {
			AddString(*optText);
		}

		if (optCaret.has_value()) {
			SetCaretPos(*optCaret);
		}
	}

	/*!
	 * キャレット位置を調整する
	 */
	void SetCaretPos(
		CLogicPoint ptCaret
	) const
	{
		auto& sWorkBuffer = GetDllShareDataPtr()->m_sWorkBuffer;
		sWorkBuffer.m_LogicPoint = ptCaret;

		HWND hWnd = nullptr;
		pcEditWnd->DispatchEvent(hWnd, MYWM_SETCARETPOS, 0, 0L);
	}
};

} // namespace env
