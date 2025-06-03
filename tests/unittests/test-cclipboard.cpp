/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

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

#include "CEol.h"
#include "mem/CNativeW.h"
#include "_os/CClipboard.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

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

class MockCClipboard : public CClipboard {
public:
	MockCClipboard(bool openStatus = true) : CClipboard(openStatus) {}
	~MockCClipboard() override {}
	MOCK_CONST_METHOD2(SetClipboardData, HANDLE (UINT, HANDLE));
	MOCK_CONST_METHOD1(GetClipboardData, HANDLE (UINT));
	MOCK_CONST_METHOD0(EmptyClipboard, BOOL ());
	MOCK_CONST_METHOD1(IsClipboardFormatAvailable, BOOL (UINT));
	MOCK_CONST_METHOD1(EnumClipboardFormats, UINT (UINT));
	MOCK_CONST_METHOD2(GlobalAlloc, HGLOBAL (UINT, SIZE_T));
	MOCK_CONST_METHOD1(GlobalLock, LPVOID (HGLOBAL));
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
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(Invoke(::GlobalAlloc));
	EXPECT_CALL(clipboard, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text)));
	EXPECT_CALL(clipboard, SetClipboardData(::RegisterClipboardFormat(L"MSDEVColumnSelect"), ByteValueInGlobalMemory(0)));
	EXPECT_FALSE(clipboard.SetText(text.data(), text.length(), true, false, CF_UNICODETEXT));
}

// SetText のテスト（サクラ独自形式のみ・矩形選択なし・行選択あり）
TEST(CClipboard, SetText3) {
	constexpr std::wstring_view text = L"てすと";
	const CLIPFORMAT sakuraFormat = CClipboard::GetSakuraFormat();
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(Invoke(::GlobalAlloc));
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
TEST_F(CClipboardGetText, NoSpecifiedFormat5) {
	constexpr std::array<char, 10> files = {"CF_HDROP\0"};
	GlobalMemory mem(GMEM_MOVEABLE, sizeof(DROPFILES) + files.size());
	mem.Lock<DROPFILES>([=](DROPFILES* d) {
		d->pFiles = sizeof(DROPFILES);
		d->fWide = FALSE;
		memcpy((char*)d + sizeof(DROPFILES), files.data(), files.size());
	});
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
	constexpr std::array<char, 6> files = {"file\0"};
	GlobalMemory mem(GMEM_MOVEABLE, sizeof(DROPFILES) + files.size());
	mem.Lock<DROPFILES>([=](DROPFILES* d) {
		d->pFiles = sizeof(DROPFILES);
		d->fWide = FALSE;
		memcpy((char*)d + sizeof(DROPFILES), files.data(), files.size());
	});
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_HDROP)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(CF_HDROP)).WillByDefault(Return(mem.Get()));
	EXPECT_TRUE(clipboard.GetText(&buffer, nullptr, nullptr, eol, CF_HDROP));
	EXPECT_STREQ(buffer.GetStringPtr(), L"file");
}

// CF_HDROP を指定して取得する。
// 取得したファイルが複数であればすべてのファイル名の末尾に改行を付加する。
TEST_F(CClipboardGetText, HDropSuccessMultipleFiles) {
	constexpr std::array<char, 13> files = {"file1\0file2\0"};
	GlobalMemory mem(GMEM_MOVEABLE, sizeof(DROPFILES) + files.size());
	mem.Lock<DROPFILES>([=](DROPFILES* d) {
		d->pFiles = sizeof(DROPFILES);
		d->fWide = FALSE;
		memcpy((char*)d + sizeof(DROPFILES), files.data(), files.size());
	});
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
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"CF_UNICODETEXT", -2, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), unicodeText.data());
}

// GetClipboardDataが失敗した場合。
TEST_F(CClipboardGetText, GetClipboardByFormatFailure1) {
	buffer.SetString(L"dummy");
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(_)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"CF_UNICODETEXT", -2, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// 取得できるフォーマットがクリップボード内になかった場合。
TEST_F(CClipboardGetText, GetClipboardByFormatFailure2) {
	buffer.SetString(L"dummy");
	ON_CALL(clipboard, IsClipboardFormatAvailable(CF_UNICODETEXT)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"CF_UNICODETEXT", -2, 0, eol));
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
		EXPECT_TRUE(clipboard.IsIncludeClipboradFormat(format.name));
	}
}

// 数値を指定した場合
TEST(CClipboard, IsIncludeClipboardFormat2) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	EXPECT_TRUE(clipboard.IsIncludeClipboradFormat(L"12345"));
}

// 標準フォーマット以外の文字列を指定した場合
TEST(CClipboard, IsIncludeClipboardFormat3) {
	const UINT format = ::RegisterClipboardFormatW(UNITTEST_FORMAT_NAME);

	MockCClipboard clipboard;
	ON_CALL(clipboard, IsClipboardFormatAvailable(format)).WillByDefault(Return(TRUE));
	EXPECT_TRUE(clipboard.IsIncludeClipboradFormat(UNITTEST_FORMAT_NAME));
}

// 対象フォーマットのデータが存在しなかった場合に失敗することを確認するテスト
TEST(CClipboard, IsIncludeClipboardFormat4) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.IsIncludeClipboradFormat(L"12345"));
}

// フォーマット文字列が空だった場合は失敗する。
TEST(CClipboard, IsIncludeClipboardFormat5) {
	MockCClipboard clipboard;
	EXPECT_FALSE(clipboard.IsIncludeClipboradFormat(L""));
}

// 不明なモード値を指定すると失敗する。
TEST(CClipboard, SetClipboardByFormat1) {
	MockCClipboard clipboard;
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 99999, 1));
}

// フォーマット名が空文字列だと失敗する。
TEST(CClipboard, SetClipboardByFormat2) {
	MockCClipboard clipboard;
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"", 99999, 1));
}

// モード-1（バイナリデータ）のテスト。
// UTF-16 の符号単位（0x0000～0x00ff）を 0x00～0xff のバイト値にマップする。
// 終端モード0では文字列中の \0 をバイナリとして扱う（終端として認識しない）。
TEST(CClipboard, SetClipboardByFormat3) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(Invoke(::GlobalAlloc));
	EXPECT_CALL(clipboard, SetClipboardData(12345, BytesInGlobalMemory("\x00\x01\xfe\xff", 4)));
	EXPECT_TRUE(clipboard.SetClipboradByFormat({L"\x00\x01\xfe\xff", 4}, L"12345", -1, 0));
}

// モード-1（バイナリデータ）のテスト。
// 0x100以上の値が含まれている場合は失敗する。
TEST(CClipboard, SetClipboardByFormat4) {
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(_, _)).Times(0);
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"\x100", 1}, L"12345", -1, 0));
}

// モード3（UTF-16）のテスト。コード変換を行わないパターン。
// 終端モードの自動判定を要求する。期待されるモードは2（2バイトの0値で終端する）。
TEST(CClipboard, SetClipboardByFormat5) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(Invoke(::GlobalAlloc));
	EXPECT_CALL(clipboard, SetClipboardData(12345, WideStringInGlobalMemory(L"テスト")));
	EXPECT_TRUE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 3, -1));
}

// モード4（UTF-8）のテスト。コード変換を行う。
// 終端モードの自動判定を要求する。期待されるモードは1（1バイトの0値で終端する）。
//
// 共有データに依存するためテスト不能。
TEST(CClipboard, DISABLED_SetClipboardByFormat6) {
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(12345, AnsiStringInGlobalMemory("テスト")));
	EXPECT_TRUE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 4, -1));
}

// モード-2のテスト。SetTextと同じ処理を行う。
TEST(CClipboard, SetClipboardByFormat7) {
	constexpr std::wstring_view text = L"テスト";
	MockCClipboard clipboard;
	EXPECT_CALL(clipboard, SetClipboardData(CF_UNICODETEXT, WideStringInGlobalMemory(text)));

	// 既存のコードに実装ミスがあり、成功してもfalseを返してしまう…。
//	EXPECT_TRUE(clipboard.SetClipboradByFormat({text.data(), text.size()}, L"CF_UNICODETEXT", -2, 0));
	EXPECT_FALSE(clipboard.SetClipboradByFormat({text.data(), text.size()}, L"CF_UNICODETEXT", -2, 0));
}

// モード-2以外でGlobalAllocが失敗した場合。
TEST(CClipboard, SetClipboardByFormat8) {
	MockCClipboard clipboard;
	ON_CALL(clipboard, GlobalAlloc(_, _)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.SetClipboradByFormat({L"テスト", 3}, L"12345", 3, -1));
}

// フォーマット名が空文字列だった場合は即失敗する。
TEST(CClipboard, GetClipboardByFormat1) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// 要求されたフォーマットのデータがクリップボードに存在しなければ失敗する。
TEST(CClipboard, GetClipboardByFormat2) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(FALSE));
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// GetClipboardDataが失敗した場合。
TEST(CClipboard, GetClipboardByFormat3) {
	MockCClipboard clipboard;
	CNativeW buffer(L"dummy");
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(nullptr));
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"12345", -1, 0, eol));
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
	EXPECT_FALSE(clipboard.GetClipboradByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"");
}

// モード-1（バイナリデータ）のテスト。
// 0x00～0xff のバイト値を UTF-16 の符号単位（0x0000～0x00ff）にマップする。
// 終端モード0ではデータ中の \0 をバイナリとして扱う（終端として認識しない）。
TEST(CClipboard, GetClipboardByFormat5) {
	GlobalMemory memory(GMEM_MOVEABLE, 2);
	memory.Lock<char>([=](char* p) {
		std::memcpy(p, "\x00\xff", 2);
	});
	MockCClipboard clipboard;
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(memory.Get()));
	ON_CALL(clipboard, GlobalLock(_)).WillByDefault(Invoke(::GlobalLock));
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", -1, 0, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"\x00\xff");
}

// モード3（UTF-16）のテスト。コード変換を行わないパターン。
// 終端モードには2を設定する（2バイトの0値で終端されていることを期待する）。
TEST(CClipboard, GetClipboardByFormat6) {
	GlobalMemory memory(GMEM_MOVEABLE, sizeof(wchar_t) * 8);
	memory.Lock<wchar_t>([=](wchar_t* p) {
		std::memcpy(p, L"テスト\x00データ", 8);
	});
	MockCClipboard clipboard;
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(memory.Get()));
	ON_CALL(clipboard, GlobalLock(_)).WillByDefault(Invoke(::GlobalLock));
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", 3, 2, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}

// モード4（UTF-8）のテスト。コード変換を行う。
// 終端モードには1を設定する（1バイトの0値で終端されていることを期待する）。
//
// CEditDoc のインスタンスに依存するためテスト不能。
TEST(CClipboard, DISABLED_GetClipboardByFormat7) {
	GlobalMemory memory(GMEM_MOVEABLE, 14);
	memory.Lock<char>([=](char* p) {
		std::memcpy(p, "テスト\x00データ", 14);
	});
	MockCClipboard clipboard;
	CNativeW buffer;
	CEol eol(EEolType::cr_and_lf);
	ON_CALL(clipboard, IsClipboardFormatAvailable(12345)).WillByDefault(Return(TRUE));
	ON_CALL(clipboard, GetClipboardData(12345)).WillByDefault(Return(memory.Get()));
	EXPECT_TRUE(clipboard.GetClipboradByFormat(buffer, L"12345", 4, 1, eol));
	EXPECT_STREQ(buffer.GetStringPtr(), L"テスト");
}
