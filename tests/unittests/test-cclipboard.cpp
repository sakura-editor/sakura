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

#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <Windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>

#include "CEol.h"
#include "mem/CNativeW.h"
#include "_os/CClipboard.h"

#include "doc/CEditDoc.h"

#include "MockKernel32Dll.hpp"
#include "MockShell32Dll.hpp"
#include "MockUser32Dll.hpp"
#include "MockShareDataAccessor.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using apiwrap::HGlobal;

using ::testing::_;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrEq;

constexpr auto& CFN_UNITTEST = L"123SakuraUnittest";

auto MakeUser32DllForClipboardSuccess(HWND hWnd)
{
	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, OpenClipboard(hWnd)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, CloseClipboard()).WillOnce(Return(TRUE));
	return pUser32Dll;
}

auto MakeApiDllsForClipboardFail(HWND hWnd, DWORD dwMilliseconds)
{
	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();

	EXPECT_CALL(*pUser32Dll, OpenClipboard(hWnd)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pKernel32Dll, Sleep(dwMilliseconds)).Times(1 + CClipboard::MAX_RETRY_FOR_OPEN);
	EXPECT_CALL(*pUser32Dll, CloseClipboard()).Times(0);

	return std::make_tuple(std::move(pUser32Dll), std::move(pKernel32Dll));
}

auto MakeUser32DllForClipboardSuccess2(HWND hWnd)
{
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto devColFormat = RegisterClipboardFormatW(CClipboard::CFN_MSDEV_COLUMN);
	const auto devLn1Format = RegisterClipboardFormatW(CClipboard::CFN_MSDEV_LINE01);
	const auto devLn2Format = RegisterClipboardFormatW(CClipboard::CFN_MSDEV_LINE02);

	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_MSDEV_COLUMN))).WillRepeatedly(Return(devColFormat));
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_MSDEV_LINE01))).WillRepeatedly(Return(devLn1Format));
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_MSDEV_LINE02))).WillRepeatedly(Return(devLn2Format));

	return std::make_tuple(std::move(pUser32Dll), std::move(sakuraFormat), std::move(devColFormat), std::move(devLn1Format), std::move(devLn2Format));
}

// グローバルメモリに書き込まれた特定の Unicode 文字列にマッチする述語関数
MATCHER_P(WideStringInGlobalMemory,expected, "")
{
	const auto s = static_cast<wchar_t*>(::GlobalLock(arg));
	if (!s) return false;
	std::wstring_view actual(s);
	::GlobalUnlock(arg);
	return actual == expected;
}

// グローバルメモリに書き込まれた特定の ANSI 文字列にマッチする述語関数
MATCHER_P(AnsiStringInGlobalMemory, expected, "")
{
	const auto s = static_cast<char*>(::GlobalLock(arg));
	if (!s) return false;
	std::string_view actual(s);
	::GlobalUnlock(arg);
	return actual == expected;
}

// グローバルメモリに書き込まれたサクラ独自形式データにマッチする述語関数
MATCHER_P(SakuraFormatInGlobalMemory, expected, "")
{
	const auto p = static_cast<char*>(::GlobalLock(arg));
	if (!p) return false;
	const size_t length = *(int*)p;
	std::wstring_view actual((const wchar_t*)(p + sizeof(int)));
	::GlobalUnlock(arg);
	return actual.length() == length && actual == expected;
}

// グローバルメモリに書き込まれた特定のバイト値にマッチする述語関数
MATCHER_P(ByteValueInGlobalMemory, expected, "")
{
	const auto p = static_cast<unsigned char*>(::GlobalLock(arg));
	if (!p) return false;
	const auto actual = *p;
	::GlobalUnlock(arg);
	return actual == expected;
}

// グローバルメモリに書き込まれた特定のバイト列にマッチする述語関数
MATCHER_P2(BytesInGlobalMemory, bytes, size, "") {
	if (size != ::GlobalSize(arg))
		return false;
	void* p = ::GlobalLock(arg);
	if (!p) return false;
	bool match = std::memcmp(p, bytes, size) == 0;
	::GlobalUnlock(arg);
	return match;
}

TEST(CClipboard, Construct)
{
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard);
}

TEST(CClipboard, Construct_fail)
{
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard);
}

// Empty のテスト。
// EmptyClipboard が呼ばれることを確認する。
TEST(CClipboard, Empty)
{
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, EmptyClipboard()).WillOnce(Return(TRUE));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	clipboard.Empty();
}

TEST(CClipboard, Empty_without_ownership)
{
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	EXPECT_CALL(*pUser32Dll, EmptyClipboard()).Times(0);
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard);
	clipboard.Empty();
}

// SetHtmlTextのテスト。
// SetClipboardData に渡された引数が期待される結果と一致することを確認する。
TEST(CClipboard, SetHtmlText1)
{
	constexpr auto& inputData = L"test 109";
	constexpr auto& expected =
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
	const UINT uHtmlFormat = RegisterClipboardFormatW(CClipboard::CFN_HTML_FORMAT_);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_HTML_FORMAT_))).WillOnce(Return(uHtmlFormat));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(uHtmlFormat, AnsiStringInGlobalMemory(std::string_view(expected)))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.SetHtmlText(inputData));
}

// クリップボードのオープンに失敗していた場合、SetHtmlText は何もせずに失敗する。
TEST(CClipboard, SetHtmlText2) {
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(_, _)).Times(0);
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard.SetHtmlText(L"test"));
}

// SetClipboardDataに失敗、SetHtmlText は失敗する。
TEST(CClipboard, SetHtmlText3) {
	const auto hWnd = (HWND)0x1234;
	const UINT uHtmlFormat = RegisterClipboardFormatW(CClipboard::CFN_HTML_FORMAT_);
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_HTML_FORMAT_))).WillOnce(Return(uHtmlFormat));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(uHtmlFormat, _)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_FALSE(clipboard.SetHtmlText(L"test"));
}

// SetText のテスト（矩形選択なし・行選択なし）
TEST(CClipboard, SetText1) {
	constexpr auto text = L"てすと"sv;
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.SetText(text.data(), text.length(), false, false));
}

// SetText のテスト（矩形選択あり・行選択なし）
TEST(CClipboard, SetText2) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, ByteValueInGlobalMemory(0))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, _)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.SetText(text.data(), text.length(), true, false, CClipboard::CF_ANY));
}

// SetText のテスト（矩形選択なし・行選択あり）
TEST(CClipboard, SetText3) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, ByteValueInGlobalMemory(1))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, ByteValueInGlobalMemory(1))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.SetText(text.data(), text.length(), false, true, CClipboard::CF_ANY));
}

// SetText のテスト。
// クリップボードのオープンに失敗していた場合、SetText は何もせずに失敗する。
TEST(CClipboard, SetText4) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(_, _)).Times(0);
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, false, -1));
}

// SetText のテスト（サクラ独自形式・GlobalLock失敗）
TEST(CClipboard, SetText_fail_GlobalLock1) {
	constexpr auto text = L"てすと"sv;
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, _)).Times(0);
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, GlobalAlloc(_, _)).WillRepeatedly(Invoke(::GlobalAlloc));
	EXPECT_CALL(*pKernel32Dll, GlobalLock(_)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pKernel32Dll, GlobalFree(_)).WillRepeatedly(Invoke(GlobalFree));
	CClipboard clipboard(hWnd, pUser32Dll, pKernel32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, false, CClipboard::CF_ANY));
}

// SetText のテスト（CF_UNICODETEXT・GlobalLock失敗）
TEST(CClipboard, SetText_fail_GlobalLock2) {
	constexpr auto text = L"てすと"sv;
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, _)).Times(0);
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, GlobalAlloc(_, _)).WillRepeatedly(Invoke(::GlobalAlloc));
	int count = 0;
	EXPECT_CALL(*pKernel32Dll, GlobalLock(_)).WillRepeatedly(Invoke([&count](HGLOBAL hMem)
		{
			return 1 == count++ ? nullptr : ::GlobalLock(hMem); //2回目だけ失敗させる
		}));
	EXPECT_CALL(*pKernel32Dll, GlobalFree(_)).WillRepeatedly(Invoke(GlobalFree));
	CClipboard clipboard(hWnd, pUser32Dll, pKernel32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, false));
}

// SetText のテスト。矩形選択マークの設定に失敗した場合。
TEST(CClipboard, SetText_fail_SetClipboardData1) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return(nullptr));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, _)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, false));
}

// SetText のテスト。矩形選択マークの設定に失敗した場合。
TEST(CClipboard, SetText_fail_SetClipboardData2) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, _)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, _)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), true, false));
}

// SetText のテスト。矩形選択マークの設定に失敗した場合。
TEST(CClipboard, SetText_fail_SetClipboardData3) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, _)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, _)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), true, false, CClipboard::CF_ANY));
}

// SetText のテスト。行選択マークの設定に失敗した場合。
TEST(CClipboard, SetText_fail_SetClipboardData4) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, _)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, _)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, true, CClipboard::CF_ANY));
}

// SetText のテスト。行選択マークの設定に失敗した場合。
TEST(CClipboard, SetText_fail_SetClipboardData5) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devColFormat, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn1Format, _)).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(devLn2Format, _)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, true, CClipboard::CF_ANY));
}

// GetText のテスト（矩形選択なし・行選択なし・フォーマット指定なし）
TEST(CClipboard, GetText1) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	auto bColumnSelect = true;
	auto bLineSelect = true;
	const auto cEol = CEol(EEolType::cr_and_lf);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(int) + (text.length() + 1) * sizeof(wchar_t));
	mem.LockToWrite<std::byte>([text](std::byte* pLocked)
		{
			*std::bit_cast<int*>(pLocked) = static_cast<int>(text.length());
			memcpy_s(pLocked + sizeof(int), (text.length() + 1) * sizeof(wchar_t), text.data(), text.length() * sizeof(wchar_t));
			std::bit_cast<wchar_t*>(pLocked + sizeof(int))[text.length()] = 0;
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(sakuraFormat)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(sakuraFormat)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, &bColumnSelect, &bLineSelect, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
	EXPECT_FALSE(bColumnSelect);
	EXPECT_FALSE(bLineSelect);
}

// GetText のテスト（矩形選択あり・行選択なし・フォーマット指定なし）
TEST(CClipboard, GetText2) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	auto bColumnSelect = true;
	auto bLineSelect = true;
	const auto cEol = CEol(EEolType::cr_and_lf);
	const auto cbAlloc = (text.length() + 1) * sizeof(wchar_t);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, cbAlloc);
	mem.LockToWrite<wchar_t>([cbAlloc, text](wchar_t* pLocked)
		{
			const auto cchData = text.length();
			const auto cbData = cchData * sizeof(wchar_t);
			memcpy_s(pLocked, cbAlloc, text.data(), cbData);
			std::fill(&pLocked[cchData], &pLocked[cchData + 1], 0);
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(devColFormat));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_UNICODETEXT)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, &bColumnSelect, &bLineSelect, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
	EXPECT_TRUE(bColumnSelect);
	EXPECT_FALSE(bLineSelect);
}

// GetText のテスト（矩形選択なし・行選択あり・フォーマット指定なし）
TEST(CClipboard, GetText3) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	auto bColumnSelect = true;
	auto bLineSelect = true;
	const auto cEol = CEol(EEolType::cr_and_lf);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, 7);
	mem.LockToWrite<std::byte>([](std::byte* pLocked)
		{
			strcpy_s(std::bit_cast<char*>(pLocked), 7, "てすと");
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(devLn1Format));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_OEMTEXT)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, &bColumnSelect, &bLineSelect, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
	EXPECT_FALSE(bColumnSelect);
	EXPECT_TRUE(bLineSelect);
}

// GetText のテスト（矩形選択なし・行選択あり・サクラ形式）
TEST(CClipboard, GetText4) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	auto bColumnSelect = true;
	auto bLineSelect = true;
	const auto cEol = CEol(EEolType::cr_and_lf);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(int) + (text.length() + 1) * sizeof(wchar_t));
	mem.LockToWrite<std::byte>([text](std::byte* pLocked)
		{
			*std::bit_cast<int*>(pLocked) = static_cast<int>(text.length());
			memcpy_s(pLocked + sizeof(int), (text.length() + 1) * sizeof(wchar_t), text.data(), text.length() * sizeof(wchar_t));
			std::bit_cast<wchar_t*>(pLocked + sizeof(int))[text.length()] = 0;
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(devLn2Format));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(sakuraFormat)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, &bColumnSelect, &bLineSelect, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
	EXPECT_FALSE(bColumnSelect);
	EXPECT_TRUE(bLineSelect);
}

// GetText のテスト（矩形選択なし・行選択なし・文字列）
TEST(CClipboard, GetText5) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	auto bColumnSelect = true;
	auto bLineSelect = true;
	const auto cEol = CEol(EEolType::cr_and_lf);
	const auto cbAlloc = (text.length() + 1) * sizeof(wchar_t);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, cbAlloc);
	mem.LockToWrite<wchar_t>([cbAlloc, text](wchar_t* pLocked)
		{
			const auto cchData = text.length();
			const auto cbData = cchData * sizeof(wchar_t);
			memcpy_s(pLocked, cbAlloc, text.data(), cbData);
			std::fill(&pLocked[cchData], &pLocked[cchData + 1], 0);
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_UNICODETEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_UNICODETEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).Times(0);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).Times(0);
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_UNICODETEXT)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, &bColumnSelect, &bLineSelect, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
	EXPECT_FALSE(bColumnSelect);
	EXPECT_FALSE(bLineSelect);
}

// GetText のテスト（矩形選択なし・行選択なし・バイナリ）
TEST(CClipboard, GetText6) {
	constexpr auto text = L"てすと"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	auto bColumnSelect = true;
	auto bLineSelect = true;
	const auto cEol = CEol(EEolType::cr_and_lf);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, 7);
	mem.LockToWrite<std::byte>([](std::byte* pLocked)
		{
			strcpy_s(std::bit_cast<char*>(pLocked), 7, "てすと");
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_OEMTEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_OEMTEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).Times(0);
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_OEMTEXT)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, &bColumnSelect, &bLineSelect, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
	EXPECT_FALSE(bColumnSelect);
	EXPECT_FALSE(bLineSelect);
}

// GetText のテスト（サクラ独自形式・GetClipboardData失敗）
TEST(CClipboard, GetText_fail_GetClipboardData1) {
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	const auto cEol = CEol(EEolType::cr_and_lf);
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(sakuraFormat)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(sakuraFormat)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_FALSE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), L"");
}

// GetText のテスト（文字列・GetClipboardData失敗）
TEST(CClipboard, GetText_fail_GetClipboardData2) {
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	const auto cEol = CEol(EEolType::cr_and_lf);
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_UNICODETEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_UNICODETEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_UNICODETEXT)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_FALSE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), L"");
}

// GetText のテスト（バイナリ・GetClipboardData失敗）
TEST(CClipboard, GetText_fail_GetClipboardData3) {
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	const auto cEol = CEol(EEolType::cr_and_lf);
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_OEMTEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_OEMTEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_OEMTEXT)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, std::move(pUser32Dll));
	EXPECT_FALSE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), L"");
}

// GetText のテスト（サクラ独自形式・GlobalLock失敗）
TEST(CClipboard, GetText_fail_GlobalLock1) {
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf(L"dummy");
	const auto cEol = CEol(EEolType::cr_and_lf);
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(sakuraFormat)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(sakuraFormat)).WillOnce(Return((HANDLE)0x5678));
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, GlobalLock(_)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, pUser32Dll, pKernel32Dll);
	EXPECT_FALSE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), L"");
}

// GetText のテスト（CF_UNICODETEXT・GlobalLock失敗）
TEST(CClipboard, GetText_fail_GlobalLock2) {
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf(L"dummy");
	const auto cEol = CEol(EEolType::cr_and_lf);
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_UNICODETEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_UNICODETEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_UNICODETEXT)).WillOnce(Return((HANDLE)0x5678));
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, GlobalLock(_)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, pUser32Dll, pKernel32Dll);
	EXPECT_FALSE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), L"");
}

// GetText のテスト（CF_OEMTEXT・GlobalLock失敗）
TEST(CClipboard, GetText_fail_GlobalLock3) {
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf(L"dummy");
	const auto cEol = CEol(EEolType::cr_and_lf);
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_OEMTEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_OEMTEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_OEMTEXT)).WillOnce(Return((HANDLE)0x5678));
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, GlobalLock(_)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, pUser32Dll, pKernel32Dll);
	EXPECT_FALSE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), L"");
}

// GetText のテスト（HDROP）
TEST(CClipboard, GetDropFiles1) {
	constexpr auto text = L"C:\\Windows\\system.ini\r\nC:\\Windows\\win.ini\r\n"sv;
	const auto hWnd = (HWND)0x1234;
	CNativeW cmemBuf;
	const auto cEol = CEol(EEolType::cr_and_lf);
	HGlobal mem(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(HDROP));
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_HDROP));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_HDROP)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_HDROP)).WillOnce(Return(static_cast<HGLOBAL>(mem)));
	auto pShell32Dll = std::make_shared<MockShell32Dll>();
	EXPECT_CALL(*pShell32Dll, DragQueryFileW(_, 0xFFFFFFFF, nullptr, 0)).WillOnce(Return(2));
	EXPECT_CALL(*pShell32Dll, DragQueryFileW(_, 0, NotNull(), Gt(0))).WillOnce(Invoke([](HDROP, UINT, LPWSTR lpszFile, UINT cch)
		{
			constexpr auto& filename = LR"(C:\Windows\system.ini)";
			wcscpy_s(lpszFile, cch, filename);
			return (UINT)_countof(filename) - 1;
		}));
	EXPECT_CALL(*pShell32Dll, DragQueryFileW(_, 1, NotNull(), Gt(0))).WillOnce(Invoke([](HDROP, UINT, LPWSTR lpszFile, UINT cch)
		{
			constexpr auto& filename = LR"(C:\Windows\win.ini)";
			wcscpy_s(lpszFile, cch, filename);
			return (UINT)_countof(filename) - 1;
		}));
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::make_shared<Kernel32Dll>(), std::move(pShell32Dll));
	EXPECT_TRUE(clipboard.GetText(&cmemBuf, nullptr, nullptr, cEol, CClipboard::CF_ANY));
	EXPECT_STREQ(cmemBuf.GetStringPtr(), text.data());
}

// OpenClipboard が失敗していた場合。
TEST(CClipboard, GetTextWithoutOwnership) {
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	CNativeW buffer;
	CEol eol;
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CClipboard::CF_ANY));
}

// 指定された形式が存在しなかった場合。
TEST(CClipboard, GetTextNoClipboard) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(_)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(_)).Times(0);
	CNativeW buffer;
	CEol eol;
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CClipboard::CF_ANY));
}

// フォーマット文字列が空だった場合は失敗する。
TEST(CClipboard, IsIncludeClipboardFormat1) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(_)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.IsIncludeClipboradFormat(L""));
}

// 数値を指定した場合
TEST(CClipboard, IsIncludeClipboardFormat2) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(TRUE));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.IsIncludeClipboradFormat(L"12345"));
}

// 標準フォーマット以外の文字列を指定した場合
TEST(CClipboard, IsIncludeClipboardFormat3) {
	const auto format = ::RegisterClipboardFormatW(CFN_UNITTEST);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CFN_UNITTEST))).WillOnce(Return(format));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(format)).WillOnce(Return(TRUE));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.IsIncludeClipboradFormat(CFN_UNITTEST));
}

// 対象フォーマットのデータが存在しなかった場合に失敗することを確認するテスト
TEST(CClipboard, IsIncludeClipboardFormat4) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(FALSE));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.IsIncludeClipboradFormat(L"12345"));
}

// 不明なモード値を指定すると失敗する。
TEST(CClipboard, SetClipboardByFormat1) {
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(_, _)).Times(0);
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 99999, 1));
}

// フォーマット名が空文字列だと失敗する。
TEST(CClipboard, SetClipboardByFormat2) {
	const auto hWnd = (HWND)0x1234;
	const DWORD dwMilliseconds = 0;
	auto [pUser32Dll, pKernel32Dll] = MakeApiDllsForClipboardFail(hWnd, dwMilliseconds);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(_, _)).Times(0);
	CClipboard clipboard(hWnd, std::move(pUser32Dll), std::move(pKernel32Dll));
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"", 99999, 1));
}

// モード-1（バイナリデータ）のテスト。
// UTF-16 の符号単位（0x0000～0x00ff）を 0x00～0xff のバイト値にマップする。
// 終端モード0では文字列中の \0 をバイナリとして扱う（終端として認識しない）。
TEST(CClipboard, SetClipboardByFormat3) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(12345, BytesInGlobalMemory("\x00\x01\xfe\xff", 4))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.SetClipboradByFormat({L"\x00\x01\xfe\xff", 4}, L"12345", -1, 0));
}

// モード-1（バイナリデータ）のテスト。
// 0x100以上の値が含まれている場合は失敗する。
TEST(CClipboard, SetClipboardByFormat4) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(_, _)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"\x100", 1}, L"12345", -1, 0));
}

// モード3（UTF-16）のテスト。コード変換を行わないパターン。
// 終端モードの自動判定を要求する。期待されるモードは2（2バイトの0値で終端する）。
TEST(CClipboard, SetClipboardByFormat5) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(12345, WideStringInGlobalMemory(L"テスト"sv))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 3, -1));
}

// モード0（Shift-JIS）のテスト。コード変換を行う。
// 終端モードの自動判定を要求する。期待されるモードは1（1バイトの0値で終端する）。
TEST(CClipboard, SetClipboardByFormat6) {
	const auto hWnd = (HWND)0x1234;
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, SetClipboardData(12345, AnsiStringInGlobalMemory("テスト"sv))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, pUser32Dll, std::make_shared<Kernel32Dll>(), std::make_shared<Shell32Dll>(), std::move(pShareDataAccessor));
	EXPECT_TRUE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", static_cast<int>(CODE_SJIS), -1));
}

// モード-2のテスト。SetTextと同じ処理を行う。
TEST(CClipboard, SetClipboardByFormat7) {
	constexpr std::wstring_view text = L"テスト";
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	EXPECT_CALL(*pUser32Dll, SetClipboardData(sakuraFormat, SakuraFormatInGlobalMemory(text))).WillOnce(Return((HANDLE)1));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.SetClipboradByFormat({ text.data(), text.size() }, L"", -2, 0));
}

// モード-2以外でGlobalAllocが失敗した場合。
TEST(CClipboard, SetClipboardByFormat8) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, GlobalAlloc(_, _)).WillOnce(Return(nullptr));
	CClipboard clipboard(hWnd, pUser32Dll, pKernel32Dll);
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 3, -1));
}

// フォーマット名が空文字列だった場合は即失敗する。
TEST(CClipboard, GetClipboardByFormat1) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(_)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// 要求されたフォーマットのデータがクリップボードに存在しなければ失敗する。
TEST(CClipboard, GetClipboardByFormat2) {
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(FALSE));
	CClipboard clipboard(hWnd, pUser32Dll);
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// モード-1（バイナリデータ）のテスト。
// 0x00～0xff のバイト値を UTF-16 の符号単位（0x0000～0x00ff）にマップする。
// 終端モード0ではデータ中の \0 をバイナリとして扱う（終端として認識しない）。
TEST(CClipboard, GetClipboardByFormat3) {
	const auto hWnd = (HWND)0x1234;
	HGlobal memory(GMEM_MOVEABLE, 2);
	memory.LockToWrite<std::byte>([](std::byte* p) {
		std::memcpy(p, "\x00\xff", 2);
	});
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(12345)).WillOnce(Return(static_cast<HGLOBAL>(memory)));
	CClipboard clipboard(hWnd, pUser32Dll);
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"\x00\xff");
}

// モード-2（エディタと同じ）のテスト。
TEST(CClipboard, GetClipboardByFormat4) {
	const auto hWnd = (HWND)0x1234;
	const auto& bin = L"テスト";
	HGlobal memory(GMEM_MOVEABLE, sizeof(bin));
	memory.LockToWrite<wchar_t>([bin](wchar_t* p) {
		std::memcpy(p, bin, sizeof(bin)); //わざと終端NULを書き込まない
		});
	auto [pUser32Dll, sakuraFormat, devColFormat, devLn1Format, devLn2Format] = MakeUser32DllForClipboardSuccess2(hWnd);
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(0)).WillOnce(Return(CF_UNICODETEXT));
	EXPECT_CALL(*pUser32Dll, EnumClipboardFormats(CF_UNICODETEXT)).WillOnce(Return(0));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(CF_UNICODETEXT)).WillOnce(Return(static_cast<HGLOBAL>(memory)));
	CClipboard clipboard(hWnd, pUser32Dll);
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"CF_UNICODETEXT", -2, -1, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

// モード3（UTF-16）のテスト。コード変換を行わないパターン。
// 終端モードには2を設定する（2バイトの0値で終端されていることを期待する）。
TEST(CClipboard, GetClipboardByFormat5) {
	const auto hWnd = (HWND)0x1234;
	const auto& bin = L"テスト";
	HGlobal memory(GMEM_MOVEABLE, sizeof(bin));
	memory.LockToWrite<wchar_t>([bin](wchar_t* p) {
		std::memcpy(p, bin, sizeof(bin)); //わざと終端NULを書き込まない
	});
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(12345)).WillOnce(Return(static_cast<HGLOBAL>(memory)));
	CClipboard clipboard(hWnd, pUser32Dll);
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", (int)CODE_UNICODE, 2, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

// モード4（UTF-8）のテスト。コード変換を行う。
// 終端モードには1を設定する（1バイトの0値で終端されていることを期待する）。
TEST(CClipboard, GetClipboardByFormat6) {
	const auto hWnd = (HWND)0x1234;
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor); ;
	const auto& bin = u8"テスト";
	HGlobal memory(GMEM_MOVEABLE, sizeof(bin));
	memory.LockToWrite<std::byte>([bin](std::byte* p) {
		std::memcpy(p, bin, sizeof(bin));
	});
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(12345)).WillOnce(Return(static_cast<HGLOBAL>(memory)));
	CClipboard clipboard(hWnd, pUser32Dll, std::make_shared<Kernel32Dll>(), std::make_shared<Shell32Dll>(), std::move(pShareDataAccessor));
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", (int)CODE_UTF8, 1, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

// モード99（自動検出）のテスト。コード変換を行う。
TEST(CClipboard, GetClipboardByFormat7) {
	const auto hWnd = (HWND)0x1234;
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc doc(pShareDataAccessor); ;
	const auto& bin = u8"テスト";
	HGlobal memory(GMEM_MOVEABLE, sizeof(bin));
	memory.LockToWrite<std::byte>([bin](std::byte* p) {
		std::memcpy(p, bin, sizeof(bin));
		});
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(12345)).WillOnce(Return(static_cast<HGLOBAL>(memory)));
	CClipboard clipboard(hWnd, pUser32Dll, std::make_shared<Kernel32Dll>(), std::make_shared<Shell32Dll>(), std::move(pShareDataAccessor));
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", (int)CODE_AUTODETECT, -1, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

TEST(CClipboard, GetClipboardByFormat_GetClipboardData_fail1) {
	const auto hWnd = (HWND)0x1234;
	const auto& bin = L"テスト";
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(12345)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, GetClipboardData(12345)).WillOnce(Return((HANDLE)nullptr));
	CClipboard clipboard(hWnd, pUser32Dll);
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"12345", (int)CODE_UNICODE, 2, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

TEST(CClipboard, GetSakuraFormat) {
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_EQ(sakuraFormat, CClipboard::GetSakuraFormat(pUser32Dll));
}

// GetDataType（全部ある）
TEST(CClipboard, GetDataType1) {
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillRepeatedly(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).Times(0);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).Times(0);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(CClipboard::HasValidData(pUser32Dll));
	EXPECT_EQ(sakuraFormat, clipboard.GetDataType());
}

TEST(CClipboard, GetDataType2) {
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillRepeatedly(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).Times(0);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(CClipboard::HasValidData(pUser32Dll));
	EXPECT_EQ(CF_UNICODETEXT, clipboard.GetDataType());
}

TEST(CClipboard, GetDataType3) {
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillRepeatedly(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).Times(0);
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(CClipboard::HasValidData(pUser32Dll));
	EXPECT_EQ(CF_OEMTEXT, clipboard.GetDataType());
}

TEST(CClipboard, GetDataType4) {
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillRepeatedly(Return(TRUE));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(CClipboard::HasValidData(pUser32Dll));
	EXPECT_EQ(CF_HDROP, clipboard.GetDataType());
}

TEST(CClipboard, GetDataType5) {
	const auto sakuraFormat = RegisterClipboardFormatW(CClipboard::CFN_SAKURA_CLIP2);
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(StrEq(CClipboard::CFN_SAKURA_CLIP2))).WillRepeatedly(Return(sakuraFormat));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(sakuraFormat)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_OEMTEXT)).WillRepeatedly(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(CF_HDROP)).WillRepeatedly(Return(FALSE));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_FALSE(CClipboard::HasValidData(pUser32Dll));
	EXPECT_EQ(-1, clipboard.GetDataType());
}

//! 標準フォーマットのためのパラメータ型
using ClipFormatTestParams = std::tuple<std::wstring, CLIPFORMAT>;

//! 標準フォーマットのためのフィクスチャクラス
class ClipFormatTest : public ::testing::TestWithParam<ClipFormatTestParams> {};

TEST_P(ClipFormatTest, PreDefined) {
	const auto& formatName = std::get<0>(GetParam());
	const auto uFormat = std::get<1>(GetParam());
	const auto hWnd = (HWND)0x1234;
	auto pUser32Dll = MakeUser32DllForClipboardSuccess(hWnd);
	EXPECT_CALL(*pUser32Dll, RegisterClipboardFormatW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, IsClipboardFormatAvailable(uFormat)).WillOnce(Return(TRUE));
	CClipboard clipboard(hWnd, pUser32Dll);
	EXPECT_TRUE(clipboard.IsIncludeClipboradFormat(formatName.data()));
}

#pragma push_macro("PreDefinedClipFormatEntry")
#define PreDefinedClipFormatEntry(format) ClipFormatTestParams{ L ## #format ## s, format } 

INSTANTIATE_TEST_CASE_P(ParameterizedTestClipFormat
	, ClipFormatTest
	, ::testing::Values(
		PreDefinedClipFormatEntry(CF_TEXT),
		PreDefinedClipFormatEntry(CF_BITMAP),
		PreDefinedClipFormatEntry(CF_METAFILEPICT),
		PreDefinedClipFormatEntry(CF_SYLK),
		PreDefinedClipFormatEntry(CF_DIF),
		PreDefinedClipFormatEntry(CF_TIFF),
		PreDefinedClipFormatEntry(CF_OEMTEXT),
		PreDefinedClipFormatEntry(CF_DIB),
		PreDefinedClipFormatEntry(CF_PALETTE),
		PreDefinedClipFormatEntry(CF_PENDATA),
		PreDefinedClipFormatEntry(CF_RIFF),
		PreDefinedClipFormatEntry(CF_WAVE),
		PreDefinedClipFormatEntry(CF_UNICODETEXT),
		PreDefinedClipFormatEntry(CF_ENHMETAFILE),
		PreDefinedClipFormatEntry(CF_HDROP),
		PreDefinedClipFormatEntry(CF_LOCALE),
		PreDefinedClipFormatEntry(CF_DIBV5)
	)
);

#pragma pop_macro("PreDefinedClipFormatEntry")
