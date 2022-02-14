/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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
#include <gmock/gmock.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include <Windows.h>
#include <CommCtrl.h>

#include "CEol.h"
#include "mem/CNativeW.h"
#include "_os/CClipboard.h"

using ::testing::_;
using ::testing::Return;

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

class CClipboardTestFixture : public testing::Test {
protected:
	void SetUp() override {
		hInstance = ::GetModuleHandle(nullptr);
		hWnd = ::CreateWindowExW(0, WC_STATICW, L"test", 0, 1, 1, 1, 1, nullptr, nullptr, hInstance, nullptr);
		if (!hWnd) FAIL();
	}
	void TearDown() override {
		if (hWnd)
			::DestroyWindow(hWnd);
	}

	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
};

TEST_F(CClipboardTestFixture, SetTextAndGetText)
{
	const std::wstring_view text = L"てすと";
	CClipboard clipboard(hWnd);
	CNativeW buffer;
	bool column;
	bool line;
	CEol eol(EEolType::cr_and_lf);

	// テストを実行する前にクリップボードの内容を破棄しておく。
	clipboard.Empty();

	// テキストを設定する（矩形選択フラグなし・行選択フラグなし）
	EXPECT_TRUE(clipboard.SetText(text.data(), text.length(), false, false, -1));
	EXPECT_TRUE(CClipboard::HasValidData());
	// Unicode文字列を取得する
	EXPECT_TRUE(clipboard.GetText(&buffer, &column, &line, eol, CF_UNICODETEXT));
	EXPECT_STREQ(buffer.GetStringPtr(), text.data());
	EXPECT_FALSE(column);
	EXPECT_FALSE(line);

	clipboard.Empty();

	// テキストを設定する（矩形選択あり・行選択あり）
	EXPECT_TRUE(clipboard.SetText(text.data(), text.length(), true, true, -1));
	EXPECT_TRUE(CClipboard::HasValidData());
	// サクラエディタ独自形式データを取得する
	EXPECT_TRUE(clipboard.GetText(&buffer, &column, nullptr, eol, CClipboard::GetSakuraFormat()));
	EXPECT_STREQ(buffer.GetStringPtr(), text.data());
	EXPECT_TRUE(column);
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, &line, eol, CClipboard::GetSakuraFormat()));
	EXPECT_STREQ(buffer.GetStringPtr(), text.data());
	EXPECT_TRUE(line);
}

// グローバルメモリに書き込まれた特定の Unicode 文字列にマッチする述語関数
MATCHER_P(WideStringInGlobalMemory, expected_string, "") {
	const wchar_t* s = (const wchar_t*)::GlobalLock(arg);
	if (!s) return false;
	std::wstring_view actual(s);
	bool match = actual == expected_string;
	::GlobalUnlock(arg);
	return match;
}

// グローバルメモリに書き込まれた特定の ANSI 文字列にマッチする述語関数
MATCHER_P(AnsiStringInGlobalMemory, expected_string, "") {
	const char* s = (const char*)::GlobalLock(arg);
	if (!s) return false;
	std::string_view actual(s);
	bool match = actual == expected_string;
	::GlobalUnlock(arg);
	return match;
}

// グローバルメモリに書き込まれたサクラ独自形式データにマッチする述語関数
MATCHER_P(SakuraFormatInGlobalMemory, expected_string, "") {
	char* p = (char*)::GlobalLock(arg);
	if (!p) return false;
	int length = *(int*)p;
	p += sizeof(int);
	std::wstring_view actual((const wchar_t*)p);
	bool match = actual.size() == length && actual == expected_string;
	::GlobalUnlock(arg);
	return match;
}

// グローバルメモリに書き込まれた特定のバイト値にマッチする述語関数
MATCHER_P(ByteValueInGlobalMemory, value, "") {
	unsigned char* p = (unsigned char*)::GlobalLock(arg);
	if (!p) return false;
	bool match = *p == value;
	::GlobalUnlock(arg);
	return match;
}

class MockCClipboard : public CClipboard {
public:
	MockCClipboard(bool openStatus = true) : CClipboard(openStatus) {}
	~MockCClipboard() override {}
	MOCK_CONST_METHOD2(SetClipboardData, HANDLE (UINT, HANDLE));
	MOCK_CONST_METHOD1(GetClipboardData, HANDLE (UINT));
	MOCK_CONST_METHOD0(EmptyClipboard, BOOL ());
	MOCK_CONST_METHOD1(IsClipboardFormatAvailable, BOOL (UINT));
};

// Empty のテスト。
// EmptyClipboard が呼ばれることを確認する。
TEST(CClipboard, Empty) {
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, EmptyClipboard()).WillOnce(Return(TRUE));
	clipboard.Empty();
}

// SetHtmlTextのテスト。
// SetClipboardData に渡された引数が期待される結果と一致することを確認する。
TEST(CClipboard, SetHtmlText1)
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

	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(uHtmlFormat, AnsiStringInGlobalMemory(expected)));
	EXPECT_TRUE(clipboard.SetHtmlText(inputData));
}

// クリップボードのオープンに失敗していた場合、SetHtmlText は何もせずに失敗する。
TEST(CClipboard, SetHtmlText2) {
	MockCClipboard clipboard(false);
	EXPECT_CALL(clipboard, SetClipboardData(_, _)).Times(0);
	EXPECT_FALSE(clipboard.SetHtmlText(L"test"));
}

// SetText のテスト（フォーマット指定なし・矩形選択なし・行選択なし）
TEST(CClipboard, SetText1) {
	constexpr std::wstring_view text = L"てすと";
	const CLIPFORMAT sakuraFormat = CClipboard::GetSakuraFormat();
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text)));
	EXPECT_CALL(clipboard, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text)));
	EXPECT_TRUE(clipboard.SetText(text.data(), text.length(), false, false, -1));
}

// SetText のテスト（CF_UNICODETEXTのみ・矩形選択あり・行選択なし）
TEST(CClipboard, SetText2) {
	constexpr std::wstring_view text = L"てすと";
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text)));
	EXPECT_CALL(clipboard, SetClipboardData(::RegisterClipboardFormat(L"MSDEVColumnSelect"), ByteValueInGlobalMemory(0)));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), true, false, CF_UNICODETEXT));
}

// SetText のテスト（サクラ独自形式のみ・矩形選択なし・行選択あり）
TEST(CClipboard, SetText3) {
	constexpr std::wstring_view text = L"てすと";
	const CLIPFORMAT sakuraFormat = CClipboard::GetSakuraFormat();
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text)));
	EXPECT_CALL(clipboard, SetClipboardData(::RegisterClipboardFormat(L"MSDEVLineSelect"), ByteValueInGlobalMemory(1)));
	EXPECT_CALL(clipboard, SetClipboardData(::RegisterClipboardFormat(L"VisualStudioEditorOperationsLineCutCopyClipboardTag"), ByteValueInGlobalMemory(1)));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, true, sakuraFormat));
}

// SetText のテスト。
// クリップボードのオープンに失敗していた場合、SetText は何もせずに失敗する。
TEST(CClipboard, SetText4) {
	constexpr std::wstring_view text = L"てすと";
	MockCClipboard clipboard(false);
	EXPECT_CALL(clipboard, SetClipboardData(_, _)).Times(0);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, false, -1));
}

// グローバルメモリを RAII で管理する簡易ヘルパークラス
class GlobalMemory {
public:
	GlobalMemory(UINT flags, SIZE_T bytes) : handle_(::GlobalAlloc(flags, bytes)) {}
	GlobalMemory(const GlobalMemory&) = delete;
	GlobalMemory& operator=(const GlobalMemory&) = delete;
	~GlobalMemory() {
		if (handle_)
			::GlobalFree(handle_);
	}
	HGLOBAL Get() { return handle_; }
	template <typename T> void Lock(std::function<void (T*)> f) {
		f(reinterpret_cast<T*>(::GlobalLock(handle_)));
		::GlobalUnlock(handle_);
	}
private:
	HGLOBAL handle_;
};

// GetText のテストで使用するダミーデータを準備するためのフィクスチャクラス
class CClipboardGetText : public testing::Test {
protected:
	MockCClipboard clipboard;
	CNativeW buffer;
	const CLIPFORMAT sakuraFormat = CClipboard::GetSakuraFormat();
	const CEol eol{ EEolType::cr_and_lf };
	static constexpr std::wstring_view unicodeText = L"CF_UNICODE";
	static constexpr std::wstring_view sakuraText = L"SAKURAClipW";
	static constexpr std::string_view oemText = "CF_OEMTEXT";
	GlobalMemory unicodeMemory{ GMEM_MOVEABLE, (unicodeText.size() + 1) * sizeof(wchar_t) };
	GlobalMemory sakuraMemory{ GMEM_MOVEABLE, sizeof(int) + (sakuraText.size() + 1) * sizeof(wchar_t) };
	GlobalMemory oemMemory{ GMEM_MOVEABLE, oemText.size() + 1 };

	CClipboardGetText() {
		unicodeMemory.Lock<wchar_t>([=](wchar_t* p) {
			std::wcscpy(p, unicodeText.data());
		});
		sakuraMemory.Lock<unsigned char>([=](unsigned char* p) {
			*(int*)p = sakuraText.size();
			std::wcscpy((wchar_t*)(p + sizeof(int)), sakuraText.data());
		});
		oemMemory.Lock<char>([=](char* p) {
			std::strcpy(p, oemText.data());
		});
	}
};

// CClipboard::GetText のテスト群。
//
// GetText で取得したいデータ形式が特に指定されていない場合、
// サクラ形式 -> CF_UNICODETEXT -> CF_OEMTEXT -> CF_HDROP の順で取得を試みる。

// サクラ形式を正常に取得するパス。
TEST_F(CClipboardGetText, NoSpecifiedFormat1) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(sakuraFormat)).WillByDefault(Return(sakuraMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, -1));
	EXPECT_STREQ(buffer.GetStringPtr(), sakuraText.data());
}

// クリップボードにサクラ形式がなかった場合、CF_UNICODETEXTを取得する。
TEST_F(CClipboardGetText, NoSpecifiedFormat2) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(FALSE));
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(unicodeMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, -1));
	EXPECT_STREQ(buffer.GetStringPtr(), unicodeText.data());
}

// クリップボードにはサクラ形式があるはずだが、GetClipboardDataが失敗した場合、CF_UNICODETEXTを取得する。
TEST_F(CClipboardGetText, NoSpecifiedFormat3) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(sakuraFormat)).WillByDefault(Return(nullptr));
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(unicodeMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, -1));
	EXPECT_STREQ(buffer.GetStringPtr(), unicodeText.data());
}

// サクラ形式とCF_UNICODETEXTの取得に失敗した場合、CF_OEMTEXTを取得する。
TEST_F(CClipboardGetText, NoSpecifiedFormat4) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(FALSE));
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(nullptr));
	ON_CALL(clipboard, GetClipboardData(CF_OEMTEXT)).WillByDefault(Return(oemMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, -1));
	EXPECT_STREQ(buffer.GetStringPtr(), L"CF_OEMTEXT");
}

// サクラ形式とCF_UNICODETEXTとCF_OEMTEXTが失敗した場合、CF_HDROPを取得する。
TEST_F(CClipboardGetText, DISABLED_NoSpecifiedFormat5) {
	// 適切なダミーデータを用意するのが難しいため未実装
}

// GetText で取得したいデータ形式が指定されている場合、他のデータ形式は無視する。

// サクラ形式を指定して取得する。
TEST_F(CClipboardGetText, SakuraFormatSuccess) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(sakuraFormat)).WillByDefault(Return(sakuraMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, sakuraFormat));
	EXPECT_STREQ(buffer.GetStringPtr(), sakuraText.data());
}

// サクラ形式が指定されているが取得に失敗した場合。
TEST_F(CClipboardGetText, SakuraFormatFailure) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, sakuraFormat));
}

// CF_UNICODETEXTを指定して取得する。
TEST_F(CClipboardGetText, UnicodeTextSucces) {
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(unicodeMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_UNICODETEXT));
	EXPECT_STREQ(buffer.GetStringPtr(), unicodeText.data());
}

// CF_UNICODETEXTが指定されているが取得に失敗した場合。
TEST_F(CClipboardGetText, UnicodeTextFailure) {
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_UNICODETEXT));
}

// CF_OEMTEXTを指定して取得する。
TEST_F(CClipboardGetText, OemTextSuccess) {
	ON_CALL(clipboard, GetClipboardData(CF_OEMTEXT)).WillByDefault(Return(oemMemory.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_OEMTEXT));
	EXPECT_STREQ(buffer.GetStringPtr(), L"CF_OEMTEXT");
}

// CF_OEMTEXTが指定されているが取得に失敗した場合。
TEST_F(CClipboardGetText, OemTextFailure) {
	ON_CALL(clipboard, GetClipboardData(CF_OEMTEXT)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_OEMTEXT));
}

// CF_HDROP を指定して取得する。
TEST_F(CClipboardGetText, DISABLED_HDropSuccess) {
	// 適切なダミーデータを用意するのが難しいため未実装
}
