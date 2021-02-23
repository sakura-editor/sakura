/*! @file */
/*
	Copyright (C) 2021 Sakura Editor Organization

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
#include <gtest/gtest.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <CommCtrl.h>

#include "mem/CNativeW.h"
#include "_os/CClipboard.h"

/*!
 * HWND型のスマートポインタを実現するためのdeleterクラス
 */
struct window_closer
{
	void operator()(HWND hWnd) const
	{
		::DestroyWindow(hWnd);
	}
};

//! HWND型のスマートポインタ
using windowHolder = std::unique_ptr<std::remove_pointer<HWND>::type, window_closer>;


/*!
 * @brief SetHtmlTextのテスト
 */
TEST(CClipboard, SetHtmlText)
{
	constexpr const wchar_t inputData[] = L"test 109";
	constexpr const char expected[] =
		"Version:0.9\r\n"
		"StartHTML:00000097\r\n"
		"EndHTML:00000178\r\n"
		"StartFragment:00000134\r\n"
		"EndFragment:00000142\r\n"
		"<html><body>\r\n"
		"<!--StartFragment -->\r\n"
		"test 109\r\n"
		"<!--EndFragment-->\r\n"
		"</body></html>\r\n";

	const UINT uHtmlFormat = ::RegisterClipboardFormat(L"HTML Format");

	auto hInstance = ::GetModuleHandleW(nullptr);
	if (HWND hWnd = ::CreateWindowExW(0, WC_STATICW, L"test", 0, 1, 1, 1, 1, nullptr, nullptr, hInstance, nullptr); hWnd) {
		// HWNDをスマートポインタに入れる
		windowHolder holder(hWnd);

		// クリップボード操作クラスでSetHtmlTextする
		CClipboard cClipBoard(hWnd);

		// 操作は失敗しないはず。
		ASSERT_TRUE(cClipBoard.SetHtmlText(inputData));

		// 操作に成功するとHTML形式のデータを利用できるはず。
		ASSERT_TRUE(::IsClipboardFormatAvailable(uHtmlFormat));

		// クリップボード操作クラスが対応してないので生APIを呼んで確認する。

		// グローバルメモリをロックできた場合のみ中身を取得しに行く
		if (HGLOBAL hClipData = ::GetClipboardData(uHtmlFormat); hClipData != nullptr) {
			// データをstd::stringにコピーする
			const size_t cchData = ::GlobalSize(hClipData);
			const char* pData = (char*)::GlobalLock(hClipData);
			std::string strClipData(pData, cchData);

			// 使い終わったらロック解除する
			::GlobalUnlock(hClipData);

			ASSERT_STREQ(expected, strClipData.c_str());
		}
		else {
			FAIL();
		}
	}
}
