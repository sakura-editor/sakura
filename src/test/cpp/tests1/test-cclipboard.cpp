/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include <Windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>

#include "basis/CEol.h"
#include "mem/CNativeW.h"
#include "_os/CClipboard.h"
#include "_os/CDropTarget.h"

#include "cxx/com_pointer.hpp"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

namespace testing {

inline std::wstring HResultName(HRESULT hr)
{
    switch (hr) {
    case S_OK: return L"S_OK";
    case S_FALSE: return L"S_FALSE";
    case E_FAIL: return L"E_FAIL";
    case E_INVALIDARG: return L"E_INVALIDARG";
    case E_ACCESSDENIED: return L"E_ACCESSDENIED";
    case E_POINTER: return L"E_POINTER";
    case E_OUTOFMEMORY: return L"E_OUTOFMEMORY";
    case E_NOTIMPL: return L"E_NOTIMPL";
    case E_NOINTERFACE: return L"E_NOINTERFACE";
    case E_UNEXPECTED: return L"E_UNEXPECTED";
    default: break;
    }

	return L"";
}

inline std::wstring FormatHResultMessage(HRESULT hr)
{
	_com_error err{ hr };
	const auto msg = err.ErrorMessage();
	return msg ? msg : L"";
}

inline std::wstring DescribeHResult(HRESULT hr)
{
    const std::wstring wname = HResultName(hr);
    const std::wstring wmsg  = FormatHResultMessage(hr);

	if (wname.empty() && wmsg.empty()) {
		return std::format(L"(0x{:X})", static_cast<unsigned long>(hr));
	}

	if (wmsg.empty()) {
		return std::format(LR"((0x{:X}) "{}")", static_cast<unsigned long>(hr), wmsg);
	}

	return std::format(LR"({}(0x{:X}) "{}")", wname, static_cast<unsigned long>(hr), wmsg);
}

inline ::testing::AssertionResult HResultEq(
    HRESULT actual,
    HRESULT expected,
    const char* actual_expr,
	const char* expected_expr [[maybe_unused]]
)
{
    if (actual == expected) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::AssertionFailure()
        << actual_expr << " returned " << std::data(DescribeHResult(actual))
        << ", expected " << std::data(DescribeHResult(expected));
}

} // namespace testing

#define EXPECT_HRESULT_EQ(actual, expected) \
    EXPECT_PRED_FORMAT2(                    \
        [](const char* a, const char* e, auto av, auto ev) { \
            return testing::HResultEq(av, ev, a, e);             \
        },                                                   \
        actual, expected)

// グローバルメモリに書き込まれた特定の Unicode 文字列にマッチする述語関数
MATCHER_P(WideStringInGlobalMemory, expected_string, "") {
	cxx::GlobalWString actual{ arg };
	return actual.wstring() == expected_string;
}

// グローバルメモリに書き込まれた特定の ANSI 文字列にマッチする述語関数
MATCHER_P(AnsiStringInGlobalMemory, expected_string, "") {
	cxx::GlobalString actual{ arg };
	return actual.string() == expected_string;
}

// グローバルメモリに書き込まれたサクラ独自形式データにマッチする述語関数
MATCHER_P(SakuraFormatInGlobalMemory, expected_string, "") {
	cxx::GlobalSakura actual{ arg };
	return actual.wstring() == expected_string;
}

// グローバルメモリに書き込まれた特定のバイト値にマッチする述語関数
MATCHER_P(ByteValueInGlobalMemory, value, "") {
	cxx::GlobalData<BYTE> actual{ arg };
	return actual.value() == value;
}

// グローバルメモリに書き込まれた特定のバイト列にマッチする述語関数
MATCHER_P2(BytesInGlobalMemory, bytes, size, "") {
	cxx::GlobalData<BYTE> mem{ arg };
	const auto actual = mem.data();
	return 0 == std::ranges::equal(actual, std::span(bytes, size));
}

struct MockCClipboard : public CClipboard {
	MOCK_CONST_METHOD1(OpenClipboard, BOOL (HWND));
	MOCK_CONST_METHOD0(CloseClipboard, BOOL ());
	MOCK_CONST_METHOD2(SetClipboardData, HANDLE (UINT, HANDLE));
	MOCK_CONST_METHOD1(GetClipboardData, HANDLE (UINT));
	MOCK_CONST_METHOD0(EmptyClipboard, BOOL ());
	MOCK_CONST_METHOD1(IsClipboardFormatAvailable, BOOL (UINT));
	MOCK_CONST_METHOD1(EnumClipboardFormats, UINT (UINT));
	MOCK_CONST_METHOD2(GlobalAlloc, HGLOBAL (UINT, SIZE_T));
	MOCK_CONST_METHOD1(GlobalLock, LPVOID (HGLOBAL));

	explicit MockCClipboard(bool openStatus = true)
		: CClipboard(openStatus)
	{
		ON_CALL(*this, OpenClipboard(_)).WillByDefault(Return(TRUE));
		ON_CALL(*this, CloseClipboard()).WillByDefault(Return(TRUE));
		ON_CALL(*this, GlobalAlloc(_, _)).WillByDefault(Invoke(&::GlobalAlloc ));
		ON_CALL(*this, GlobalLock(_)).WillByDefault(Invoke(&::GlobalLock));
	}
};

// Open/Close のテスト。
// OpenClipboard が呼ばれることを確認する。
// CloseClipboard が呼ばれることを確認する。
TEST(CClipboard, OpenAndClose) {
	MockCClipboard clipboard(false);

	EXPECT_CALL(clipboard, OpenClipboard(_))
		.Times(2)
		.WillOnce(Return(FALSE))
		.WillOnce(Return(TRUE));
	EXPECT_CALL(clipboard, CloseClipboard())
		.Times(1)
		.WillOnce(Return(TRUE));

	EXPECT_THAT(clipboard, IsFalse());

	EXPECT_THAT(clipboard.Open(HWND(1234)), IsTrue());

	EXPECT_THAT(clipboard, IsTrue());

	clipboard.Close();

	EXPECT_THAT(clipboard, IsFalse());
}

// Open/Close のテスト。
// OpenClipboard が呼ばれることを確認する。
// CloseClipboard が呼ばれることを確認する。
TEST(CClipboard, OpenRetryOver) {
	MockCClipboard clipboard(false);

	EXPECT_CALL(clipboard, OpenClipboard(_))
		.Times(10)
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE))
		.WillOnce(Return(FALSE));
	EXPECT_CALL(clipboard, CloseClipboard())
		.Times(0);

	EXPECT_THAT(clipboard, IsFalse());

	EXPECT_THAT(clipboard.Open(HWND(1234)), IsFalse());

	EXPECT_THAT(clipboard, IsFalse());

	clipboard.Close();

	EXPECT_THAT(clipboard, IsFalse());
}

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
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(::GlobalAlloc);
	EXPECT_CALL(clipboard, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text)));
	EXPECT_CALL(clipboard, SetClipboardData(::RegisterClipboardFormat(L"MSDEVColumnSelect"), ByteValueInGlobalMemory(0)));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), true, false, CF_UNICODETEXT));
}

// SetText のテスト（サクラ独自形式のみ・矩形選択なし・行選択あり）
TEST(CClipboard, SetText3) {
	constexpr std::wstring_view text = L"てすと";
	const CLIPFORMAT sakuraFormat = CClipboard::GetSakuraFormat();
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(::GlobalAlloc);
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

// SetText のテスト。矩形選択マークの設定に失敗した場合。
TEST(CClipboard, SetText5) {
	constexpr std::wstring_view text = L"てすと";
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, 1)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), true, false, 0));
}

// SetText のテスト。行選択マークの設定に失敗した場合。
TEST(CClipboard, SetText6) {
	constexpr std::wstring_view text = L"てすと";
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, 1)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), false, true, 0));
}

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
	cxx::GlobalWString unicodeMemory{ unicodeText };
	cxx::GlobalSakura sakuraMemory{ sakuraText };
	cxx::GlobalString oemMemory{ oemText };

	CClipboardGetText() = default;
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
TEST_F(CClipboardGetText, NoSpecifiedFormat5) {
	const std::array files = { std::filesystem::path("CF_HDROP") };
	auto mem = cxx::MakeDropFiles(files);
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(FALSE));
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(nullptr));
	ON_CALL(clipboard, GetClipboardData(CF_OEMTEXT)).WillByDefault(Return(nullptr));
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_HDROP)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(CF_HDROP)).WillByDefault(Return(mem.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, -1));
	EXPECT_STREQ(buffer.GetStringPtr(), L"CF_HDROP");
}

// ここまでのすべてに失敗した場合、falseを返す。
TEST_F(CClipboardGetText, NoSpecifiedFormat6) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(sakuraFormat)).WillByDefault(Return(FALSE));
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(nullptr));
	ON_CALL(clipboard, GetClipboardData(CF_OEMTEXT)).WillByDefault(Return(nullptr));
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_HDROP)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, -1));
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
// 取得したファイルが1つであれば末尾に改行を付加しない。
TEST_F(CClipboardGetText, HDropSuccessSingleFile) {
	const std::array files = { std::filesystem::path("file") };
	auto mem = cxx::MakeDropFiles(files);
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_HDROP)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(CF_HDROP)).WillByDefault(Return(mem.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_HDROP));
	EXPECT_STREQ(buffer.GetStringPtr(), L"file");
}

// CF_HDROP を指定して取得する。
// 取得したファイルが複数であればすべてのファイル名の末尾に改行を付加する。
TEST_F(CClipboardGetText, HDropSuccessMultipleFiles) {
	const std::array files = {
		std::filesystem::path("file1"),
		std::filesystem::path("file2")
	};
	auto mem = cxx::MakeDropFiles(files);
	EXPECT_THAT(mem.data(), testing::SizeIs(Eq(files.size())));
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_HDROP)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(CF_HDROP)).WillByDefault(Return(mem.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_HDROP));
	EXPECT_STREQ(buffer.GetStringPtr(), L"file1\r\nfile2\r\n");
}

// CF_HDROP が指定されているが取得に失敗した場合。
TEST_F(CClipboardGetText, HDropFailure) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_HDROP)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(CF_HDROP)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_HDROP));
}

// 矩形選択マーク取得のテスト。
TEST_F(CClipboardGetText, ColumnSelectIsFalse) {
	ON_CALL(clipboard, EnumClipboardFormats(0)).WillByDefault(Return(0));
	bool column;
	clipboard.GetText(&buffer, &column, nullptr, eol);
	EXPECT_FALSE(column);
}

TEST_F(CClipboardGetText, ColumnSelectIsTrue) {
	UINT format = RegisterClipboardFormatW(L"MSDEVColumnSelect");
	ON_CALL(clipboard, EnumClipboardFormats(0)).WillByDefault(Return(format));
	bool column;
	clipboard.GetText(&buffer, &column, nullptr, eol);
	EXPECT_TRUE(column);
}

// 行選択マーク取得のテスト。
TEST_F(CClipboardGetText, LineSelectIsFalse) {
	ON_CALL(clipboard, EnumClipboardFormats(0)).WillByDefault(Return(0));
	bool line;
	clipboard.GetText(&buffer, &line, nullptr, eol);
	EXPECT_FALSE(line);
}

TEST_F(CClipboardGetText, LineSelectIsTrue1) {
	UINT format = RegisterClipboardFormatW(L"MSDEVLineSelect");
	ON_CALL(clipboard, EnumClipboardFormats(0)).WillByDefault(Return(format));
	bool line;
	clipboard.GetText(&buffer, nullptr, &line, eol);
	EXPECT_TRUE(line);
}

TEST_F(CClipboardGetText, LineSelectIsTrue2) {
	UINT format = RegisterClipboardFormatW(L"VisualStudioEditorOperationsLineCutCopyClipboardTag");
	ON_CALL(clipboard, EnumClipboardFormats(0)).WillByDefault(Return(format));
	bool line;
	clipboard.GetText(&buffer, nullptr, &line, eol);
	EXPECT_TRUE(line);
}

// GetClipboardByFormatのモード-2のテスト。GetTextと同じ処理が行われることを期待する。
// CF_UNICODETEXTの取得に成功した場合。
TEST_F(CClipboardGetText, GetClipboardByFormatSuccess) {
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(CF_UNICODETEXT)).WillByDefault(Return(unicodeMemory.Get()));
	EXPECT_TRUE(clipboard.GetClipboardByFormat(buffer, L"CF_UNICODETEXT", -2, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), unicodeText.data());
}

// GetClipboardDataが失敗した場合。
TEST_F(CClipboardGetText, GetClipboardByFormatFailure1) {
	buffer.SetString(L"dummy");
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(_)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetClipboardByFormat(buffer, L"CF_UNICODETEXT", -2, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// 取得できるフォーマットがクリップボード内になかった場合。
TEST_F(CClipboardGetText, GetClipboardByFormatFailure2) {
	buffer.SetString(L"dummy");
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.GetClipboardByFormat(buffer, L"CF_UNICODETEXT", -2, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// OpenClipboard が失敗していた場合。
TEST(CClipboard, GetTextNoClipboard) {
	CNativeW buffer;
	CEol eol;
	MockCClipboard clipboard(false);
	EXPECT_FALSE(clipboard.GetText(&buffer, nullptr, nullptr, eol));
}

struct ClipboardFormatDefinition {
	CLIPFORMAT id;
	const wchar_t* const name;
};

const std::array<ClipboardFormatDefinition, 17> KNOWN_FORMATS = {
	{
		{CF_TEXT        ,L"CF_TEXT"},
		{CF_BITMAP      ,L"CF_BITMAP"},
		{CF_METAFILEPICT,L"CF_METAFILEPICT"},
		{CF_SYLK        ,L"CF_SYLK"},
		{CF_DIF         ,L"CF_DIF"},
		{CF_TIFF        ,L"CF_TIFF"},
		{CF_OEMTEXT     ,L"CF_OEMTEXT"},
		{CF_DIB         ,L"CF_DIB"},
		{CF_PALETTE     ,L"CF_PALETTE"},
		{CF_PENDATA     ,L"CF_PENDATA"},
		{CF_RIFF        ,L"CF_RIFF"},
		{CF_WAVE        ,L"CF_WAVE"},
		{CF_UNICODETEXT ,L"CF_UNICODETEXT"},
		{CF_ENHMETAFILE ,L"CF_ENHMETAFILE"},
		{CF_HDROP       ,L"CF_HDROP"},
		{CF_LOCALE      ,L"CF_LOCALE"},
		{CF_DIBV5       ,L"CF_DIBV5"}
	}
};

const wchar_t* const UNITTEST_FORMAT_NAME = L"123SakuraUnittest";

// 標準フォーマットを指定した場合
TEST(CClipboard, IsIncludeClipboardFormat1) {
	for (auto format : KNOWN_FORMATS) {
		MockCClipboard clipboard;
		ON_CALL(clipboard, IsClipboardFormatAvailable(format.id)).WillByDefault(Return(TRUE));
		EXPECT_TRUE(clipboard.IsIncludeClipboardFormat(format.name));
	}
}

// 数値を指定した場合
TEST(CClipboard, IsIncludeClipboardFormat2) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	EXPECT_TRUE(clipboard.IsIncludeClipboardFormat(L"12345"));
}

// 標準フォーマット以外の文字列を指定した場合
TEST(CClipboard, IsIncludeClipboardFormat3) {
	const UINT format = ::RegisterClipboardFormatW(UNITTEST_FORMAT_NAME);

	MockCClipboard clipboard;
	ON_CALL(clipboard, IsClipboardFormatAvailable(format)).WillByDefault(Return(TRUE));
	EXPECT_TRUE(clipboard.IsIncludeClipboardFormat(UNITTEST_FORMAT_NAME));
}

// 対象フォーマットのデータが存在しなかった場合に失敗することを確認するテスト
TEST(CClipboard, IsIncludeClipboardFormat4) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.IsIncludeClipboardFormat(L"12345"));
}

// フォーマット文字列が空だった場合は失敗する。
TEST(CClipboard, IsIncludeClipboardFormat5) {
	MockCClipboard clipboard;
	EXPECT_FALSE(clipboard.IsIncludeClipboardFormat(L""));
}

// 不明なモード値を指定すると失敗する。
TEST(CClipboard, SetClipboardByFormat1) {
	MockCClipboard clipboard;
	EXPECT_FALSE(clipboard.SetClipboardByFormat({L"テスト", 3}, L"12345", 99999, 1));
}

// フォーマット名が空文字列だと失敗する。
TEST(CClipboard, SetClipboardByFormat2) {
	MockCClipboard clipboard;
	EXPECT_FALSE(clipboard.SetClipboardByFormat({L"テスト", 3}, L"", 99999, 1));
}

// モード-1（バイナリデータ）のテスト。
// UTF-16 の符号単位（0x0000～0x00ff）を 0x00～0xff のバイト値にマップする。
// 終端モード0では文字列中の \0 をバイナリとして扱う（終端として認識しない）。
TEST(CClipboard, SetClipboardByFormat3) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(::GlobalAlloc);
	EXPECT_CALL(clipboard, SetClipboardData(12345, BytesInGlobalMemory("\x00\x01\xfe\xff", 4)));
	EXPECT_TRUE(clipboard.SetClipboardByFormat({L"\x00\x01\xfe\xff", 4}, L"12345", -1, 0));
}

// モード-1（バイナリデータ）のテスト。
// 0x100以上の値が含まれている場合は失敗する。
TEST(CClipboard, SetClipboardByFormat4) {
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(_, _)).Times(0);
	EXPECT_FALSE(clipboard.SetClipboardByFormat({L"\x100", 1}, L"12345", -1, 0));
}

// モード3（UTF-16）のテスト。コード変換を行わないパターン。
// 終端モードの自動判定を要求する。期待されるモードは2（2バイトの0値で終端する）。
TEST(CClipboard, SetClipboardByFormat5) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(::GlobalAlloc);
	EXPECT_CALL(clipboard, SetClipboardData(12345, WideStringInGlobalMemory(L"テスト")));
	EXPECT_TRUE(clipboard.SetClipboardByFormat({L"テスト", 3}, L"12345", 3, -1));
}

// モード4（UTF-8）のテスト。コード変換を行う。
// 終端モードの自動判定を要求する。期待されるモードは1（1バイトの0値で終端する）。
//
// 共有データに依存するためテスト不能。
TEST(CClipboard, DISABLED_SetClipboardByFormat6) {
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(12345, AnsiStringInGlobalMemory("テスト")));
	EXPECT_TRUE(clipboard.SetClipboardByFormat({L"テスト", 3}, L"12345", 4, -1));
}

// モード-2のテスト。SetTextと同じ処理を行う。
TEST(CClipboard, SetClipboardByFormat7) {
	constexpr std::wstring_view text = L"テスト";
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text)));

	// 既存のコードに実装ミスがあり、成功してもfalseを返してしまう…。
//	EXPECT_TRUE(clipboard.SetClipboardByFormat({text.data(), text.size()}, L"CF_UNICODETEXT", -2, 0));
	EXPECT_FALSE(clipboard.SetClipboardByFormat({text.data(), text.size()}, L"CF_UNICODETEXT", -2, 0));
}

// モード-2以外でGlobalAllocが失敗した場合。
TEST(CClipboard, SetClipboardByFormat8) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.SetClipboardByFormat({L"テスト", 3}, L"12345", 3, -1));
}

// フォーマット名が空文字列だった場合は即失敗する。
TEST(CClipboard, GetClipboardByFormat1) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	EXPECT_FALSE(clipboard.GetClipboardByFormat(buffer, L"", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// 要求されたフォーマットのデータがクリップボードに存在しなければ失敗する。
TEST(CClipboard, GetClipboardByFormat2) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.GetClipboardByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// GetClipboardDataが失敗した場合。
TEST(CClipboard, GetClipboardByFormat3) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetClipboardByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// GlobalLockが失敗した場合。
TEST(CClipboard, GetClipboardByFormat4) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return((HANDLE)67890));
	ON_CALL(clipboard, GlobalLock((HANDLE)67890)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetClipboardByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// モード-1（バイナリデータ）のテスト。
// 0x00～0xff のバイト値を UTF-16 の符号単位（0x0000～0x00ff）にマップする。
// 終端モード0ではデータ中の \0 をバイナリとして扱う（終端として認識しない）。
TEST(CClipboard, GetClipboardByFormat5) {
	std::string_view ansiText{ "\x00\xff", 2 };
	cxx::GlobalString memory{ ansiText };
	MockCClipboard clipboard;
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(memory.Get()));
	ON_CALL(clipboard, GlobalLock(_)).WillByDefault(::GlobalLock);
	EXPECT_TRUE(clipboard.GetClipboardByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"\x00\xff");
}

// モード3（UTF-16）のテスト。コード変換を行わないパターン。
// 終端モードには2を設定する（2バイトの0値で終端されていることを期待する）。
TEST(CClipboard, GetClipboardByFormat6) {
	std::wstring_view unicodeText{ L"テスト\0データ", 8 };
	cxx::GlobalWString memory{ unicodeText };
	MockCClipboard clipboard;
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(memory.Get()));
	ON_CALL(clipboard, GlobalLock(_)).WillByDefault(::GlobalLock);
	EXPECT_TRUE(clipboard.GetClipboardByFormat(buffer, L"12345", 3, 2, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

// モード4（UTF-8）のテスト。コード変換を行う。
// 終端モードには1を設定する（1バイトの0値で終端されていることを期待する）。
//
// CEditDoc のインスタンスに依存するためテスト不能。
TEST(CClipboard, DISABLED_GetClipboardByFormat7) {
	std::string_view sjisText{ "テスト\0データ", 13 };
	cxx::GlobalString memory{ sjisText };
	MockCClipboard clipboard;
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(memory.Get()));
	EXPECT_TRUE(clipboard.GetClipboardByFormat(buffer, L"12345", 4, 1, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

// Test that clipboard retry constants are properly defined
TEST(CClipboard, ClipboardRetryConstants) {
	// Verify retry constants are sensible values
	EXPECT_GT(CClipboard::CLIPBOARD_RETRY_COUNT, 1);
	EXPECT_GE(CClipboard::CLIPBOARD_RETRY_DELAY_MS, 0);
	EXPECT_LE(CClipboard::CLIPBOARD_RETRY_COUNT, 100);  // Should not be excessive
	EXPECT_LE(CClipboard::CLIPBOARD_RETRY_DELAY_MS, 1000);  // Should not be too long
}

namespace cxx {

TEST(GlobalData, test001)
{
	cxx::GlobalData<char> memory{ 5 };
	EXPECT_THAT(memory.get(), NotNull());
	EXPECT_THAT(memory.size(), 5U);

	EXPECT_THAT(memory.value(), '\0');

	EXPECT_THAT(memory.SetValue('a'), IsTrue());
	EXPECT_THAT(memory.value(), 'a');

	EXPECT_THAT(memory.SetData("test"), IsTrue());
	EXPECT_THAT(memory.data().data(), StrEq("test"));
}

TEST(GlobalData, test101)
{
	cxx::GlobalData<char> memory{ nullptr };
	EXPECT_THAT(memory.get(), IsNull());
	EXPECT_THAT(memory.size(), 0U);

	EXPECT_THAT(memory.value(), '\0');

	memory.Lock([](LPCSTR) {
		//何もしない
	});
}

} // namespace cxx

namespace ole {

TEST(CDropSource, QueryContinueDrag001)
{
	const auto target = std::make_unique<CDropSource>(TRUE);

	EXPECT_HRESULT_EQ(target->QueryContinueDrag(TRUE, MK_LBUTTON), DRAGDROP_S_CANCEL);
	EXPECT_HRESULT_EQ(target->QueryContinueDrag(FALSE, MK_RBUTTON), DRAGDROP_S_CANCEL);

	EXPECT_HRESULT_EQ(target->QueryContinueDrag(FALSE, 0), DRAGDROP_S_DROP);

	EXPECT_HRESULT_SUCCEEDED(target->QueryContinueDrag(FALSE, MK_LBUTTON));
}

TEST(CDropSource, QueryContinueDrag002)
{
	const auto target = std::make_unique<CDropSource>(FALSE);

	EXPECT_HRESULT_EQ(target->QueryContinueDrag(FALSE, MK_LBUTTON), DRAGDROP_S_CANCEL);

	EXPECT_HRESULT_EQ(target->QueryContinueDrag(FALSE, 0), DRAGDROP_S_DROP);

	EXPECT_HRESULT_SUCCEEDED(target->QueryContinueDrag(FALSE, MK_RBUTTON));
}

TEST(CDropSource, GiveFeedback001)
{
	const auto target = std::make_unique<CDropSource>(TRUE);
	EXPECT_HRESULT_EQ(target->GiveFeedback(DROPEFFECT_MOVE), DRAGDROP_S_USEDEFAULTCURSORS);
}

/*!
 * DoDragDrop のテスト。
 */
TEST(CDropSource, DoDragDrop101)
{
	const auto target = std::make_unique<CDropSource>(true);

	LPDATAOBJECT pDataObject = nullptr;
	DWORD dwEffects = 0;
	EXPECT_HRESULT_EQ(target->DoDragDrop(pDataObject, dwEffects), DROPEFFECT_NONE);
}

/*!
 * 意図的にサポートしないメソッド のテスト。
 */
TEST(CopiedTextData, NotSupportedMethods)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	//構築後の変更はサポートしてない。
	EXPECT_HRESULT_EQ(target->SetData(nullptr, nullptr, FALSE), E_NOTIMPL);

	//固定値を返却するナゾ実装。たぶんバグ。（未実装なら、そう返すべき。）
	EXPECT_HRESULT_EQ(target->GetCanonicalFormatEtc(nullptr, nullptr), DATA_S_SAMEFORMATETC);

	//アドバイスはサポートしない。
	EXPECT_HRESULT_EQ(target->DAdvise(nullptr, 0, nullptr, nullptr), OLE_E_ADVISENOTSUPPORTED);
	EXPECT_HRESULT_EQ(target->DUnadvise(0), OLE_E_ADVISENOTSUPPORTED);
	EXPECT_HRESULT_EQ(target->EnumDAdvise(nullptr), OLE_E_ADVISENOTSUPPORTED);
}

/*!
 * EnumFormatEtc のテスト。
 */
TEST(CopiedTextData, EnumFormatEtc001)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	cxx::com_pointer<IEnumFORMATETC> pEnumFormatEtc = nullptr;
	EXPECT_HRESULT_EQ(target->EnumFormatEtc(DATADIR_SET, &pEnumFormatEtc), S_FALSE);
	EXPECT_THAT(pEnumFormatEtc, IsNull());

	EXPECT_HRESULT_SUCCEEDED(target->EnumFormatEtc(DATADIR_GET, &pEnumFormatEtc));
	EXPECT_THAT(pEnumFormatEtc, NotNull());

	FORMATETC format = {};
	ULONG fetched = 0;
	EXPECT_HRESULT_SUCCEEDED(pEnumFormatEtc->Next(1, &format, &fetched));
	EXPECT_THAT(fetched, 1UL);
	EXPECT_THAT(format.cfFormat, testing::Eq<CLIPFORMAT>(CF_UNICODETEXT));
	EXPECT_THAT(format.tymed, TYMED_HGLOBAL);

	// 全部で3つなので2個まではスキップできる
	EXPECT_HRESULT_SUCCEEDED(pEnumFormatEtc->Skip(2));

	// 現在位置が3つ目なので、さらにスキップしようとすると失敗する
	EXPECT_HRESULT_EQ(pEnumFormatEtc->Skip(1), S_FALSE);
}

/*!
 * QueryGetData のテスト。
 */
TEST(CopiedTextData, QueryGetData101)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	EXPECT_HRESULT_EQ(target->QueryGetData(nullptr), E_INVALIDARG);

	FORMATETC unknownFormat = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->QueryGetData(&unknownFormat), DATA_E_FORMATETC);

	DVTARGETDEVICE targetDevice = {};
	FORMATETC withPtd = { CF_UNICODETEXT, &targetDevice, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->QueryGetData(&withPtd), DATA_E_FORMATETC);

	FORMATETC badAspect = { CF_UNICODETEXT, nullptr, DVASPECT_THUMBNAIL, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->QueryGetData(&badAspect), DATA_E_FORMATETC);

	FORMATETC badIndex = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, 0, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->QueryGetData(&badIndex), DATA_E_FORMATETC);

	FORMATETC badTymed = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_ISTREAM };
	EXPECT_HRESULT_EQ(target->QueryGetData(&badTymed), DATA_E_FORMATETC);

	FORMATETC ok = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_SUCCEEDED(target->QueryGetData(&ok));
}

/*!
 * QueryGetData のテスト。
 *
 * このパターンはあり得ないのでそのうち削除する
 */
TEST(CopiedTextData, QueryGetData102)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	//データを空にする
	target->SetText(nullptr, 0, FALSE);

	FORMATETC format = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_THAT(target->QueryGetData(&format), OLE_E_NOTRUNNING);	// 👈バグです。サクラエディタは起動時にOLEを初期化するため、「OLE未実行」にはならない。
}

/*!
 * GetData のテスト。
 */
TEST(CopiedTextData, GetData001)
{
	const auto text = L"abc"s;
	const auto target = std::make_unique<CDataObject>(text.data(), text.size(), FALSE);

	FORMATETC format = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM medium = {};
	EXPECT_HRESULT_SUCCEEDED(target->GetData(&format, &medium));
	EXPECT_THAT(medium.tymed, TYMED_HGLOBAL);

	cxx::GlobalWString buffer(medium.hGlobal);
	EXPECT_THAT(buffer.wstring(), StrEq(text));
}

/*!
 * GetData のテスト。
 */
TEST(CopiedTextData, GetData101)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	STGMEDIUM emptyMedium = {};
	EXPECT_HRESULT_EQ(target->GetData(nullptr, &emptyMedium), E_INVALIDARG);

	FORMATETC format = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetData(&format, nullptr), E_INVALIDARG);

	FORMATETC unknownFormat = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetData(&unknownFormat, &emptyMedium), DV_E_FORMATETC);

	FORMATETC badAspect = { CF_UNICODETEXT, nullptr, DVASPECT_THUMBNAIL, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetData(&badAspect, &emptyMedium), DV_E_DVASPECT);

	FORMATETC badIndex = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, 0, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetData(&badIndex, &emptyMedium), DV_E_LINDEX);

	FORMATETC badTymed = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_ISTREAM };
	EXPECT_HRESULT_EQ(target->GetData(&badTymed, &emptyMedium), DV_E_TYMED);
}

/*!
 * GetData のテスト。
 *
 * このパターンはあり得ないのでそのうち削除する
 */
TEST(CopiedTextData, GetData102)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	//データを空にする
	target->SetText(nullptr, 0, FALSE);

	FORMATETC format = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM emptyMedium = {};
	EXPECT_HRESULT_EQ(target->GetData(&format, &emptyMedium), OLE_E_NOTRUNNING);	// 👈バグです。サクラエディタは起動時にOLEを初期化するため、「OLE未実行」にはならない。
}

/*!
 * GetDataHere のテスト。
 */
TEST(CopiedTextData, GetDataHere001)
{
	const auto text = L"abcdef"s;

	const auto target = std::make_unique<CDataObject>(text.data(), text.size(), FALSE);

	FORMATETC format = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	STGMEDIUM medium = {};
	medium.tymed = TYMED_HGLOBAL;

	cxx::GlobalWString buffer(text.size());
	medium.hGlobal = buffer;

	EXPECT_HRESULT_SUCCEEDED(target->GetDataHere(&format, &medium));
	EXPECT_THAT(buffer.wstring(), StrEq(text));
}

/*!
 * GetDataHere のテスト。
 */
TEST(CopiedTextData, GetDataHere101)
{
	const auto target = std::make_unique<CDataObject>(L"abc", 3, FALSE);

	FORMATETC format = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetDataHere(&format, nullptr), E_INVALIDARG);

	STGMEDIUM emptyMedium = {};
	EXPECT_HRESULT_EQ(target->GetDataHere(nullptr, &emptyMedium), E_INVALIDARG);

	STGMEDIUM nullMedium = {};
	nullMedium.tymed = TYMED_HGLOBAL;
	nullMedium.hGlobal = nullptr;
	EXPECT_HRESULT_EQ(target->GetDataHere(&format, &nullMedium), E_INVALIDARG);

	cxx::GlobalWString buffer(64);

	STGMEDIUM medium = {};
	medium.tymed = TYMED_HGLOBAL;
	medium.hGlobal = buffer;

	FORMATETC unknownFormat = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetDataHere(&unknownFormat, &medium), DV_E_FORMATETC);

	FORMATETC badAspect = { CF_UNICODETEXT, nullptr, DVASPECT_THUMBNAIL, -1, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetDataHere(&badAspect, &medium), DV_E_DVASPECT);

	FORMATETC badIndex = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, 0, TYMED_HGLOBAL };
	EXPECT_HRESULT_EQ(target->GetDataHere(&badIndex, &medium), DV_E_LINDEX);

	FORMATETC badTymed = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_ISTREAM };
	EXPECT_HRESULT_EQ(target->GetDataHere(&badTymed, &medium), DV_E_TYMED);

	cxx::GlobalWString smallBuffer(2);

	STGMEDIUM smallMedium = {};
	smallMedium.tymed = TYMED_HGLOBAL;
	smallMedium.hGlobal = smallBuffer;

	EXPECT_HRESULT_EQ(target->GetDataHere(&format, &smallMedium), STG_E_MEDIUMFULL);
}

/*!
 * CEnumFORMATETC のテスト。
 */
TEST(CEnumFORMATETC, test001)
{
	auto* target = new CEnumFORMATETC(nullptr);

	//AddRefする。内部カウンタが増えて2になる
	EXPECT_THAT(target->AddRef(), 2UL);

	//Cloneは未実装。
	EXPECT_THAT(target->Clone(nullptr), E_NOTIMPL);

	//Resetする。
	EXPECT_HRESULT_SUCCEEDED(target->Reset());

	//Releaseする。内部カウンタが減って1になる
	EXPECT_THAT(target->Release(), 1UL);

	//生ポインタなのでReleaseして解放する
	EXPECT_THAT(target->Release(), 0UL);
}

} //namespace ole
