/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "basis/CMyString.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"
#include "parse/DetectIndentationStyle.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

/*!
 * @brief .editorconfigを置いた一時ディレクトリを用意するフィクスチャ
 *
 * .editorconfigの探索は対象ファイルのあるディレクトリから親へ遡るので、
 * 一時ディレクトリの外に影響されないよう、探索を打ち切らせるケースを除いて
 * 対象セクションを持つ.editorconfigを一時ディレクトリ内に置くこと。
 */
class DetectIndentationStyleTest : public ::testing::Test {
protected:
	std::filesystem::path m_dir;

	void SetUp() override {
		const auto* const testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
		m_dir = std::filesystem::temp_directory_path() / ("sakura-editorconfig-"s + testInfo->name());

		std::error_code ec;
		std::filesystem::remove_all(m_dir, ec);
		std::filesystem::create_directories(m_dir, ec);
	}

	void TearDown() override {
		std::error_code ec;
		std::filesystem::remove_all(m_dir, ec);
	}

	//! 指定ディレクトリに.editorconfigを書き出す
	void WriteEditorConfig(std::string_view content, const std::filesystem::path& dir) const {
		std::ofstream os(dir / ".editorconfig", std::ios::binary);
		os.write(content.data(), content.size());
	}

	//! 一時ディレクトリ直下に.editorconfigを書き出す
	void WriteEditorConfig(std::string_view content) const {
		WriteEditorConfig(content, m_dir);
	}

	//! 一時ディレクトリ内のファイルを対象にインデントスタイルを判定する
	//! @param cDocLineMgr 空を渡すとファイル内容からの検出は何も検出しない
	IndentationStyle Detect(const std::filesystem::path& filePath, const CDocLineMgr& cDocLineMgr) const {
		IndentationStyle style{};
		DetectIndentationStyle(CFilePath(filePath.wstring().c_str()), cDocLineMgr, 256, style);
		return style;
	}

	//! 一時ディレクトリ直下のtest.cppを対象にインデントスタイルを判定する
	IndentationStyle Detect() const {
		const CDocLineMgr cDocLineMgr;
		return Detect(m_dir / "test.cpp", cDocLineMgr);
	}
};

/*!
 * tab_widthは値を解析する。
 * キーを解析していたため常に解析に失敗していた。(#2467)
 */
TEST_F(DetectIndentationStyleTest, TabWidthIsParsedFromValue)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"tab_width = 8\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	EXPECT_EQ(style.tabSpace, 8);
}

/*!
 * 値の比較は厳密一致で行う。
 * 前方一致だったため"tabs"が"tab"と看做されていた。
 */
TEST_F(DetectIndentationStyleTest, ValueComparisonIsExactMatch)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tabs\n"sv);

	const auto style = Detect();

	// indent_styleを認識できないので指定が無かったものとして扱う
	EXPECT_EQ(style.character, IndentationStyle::Character::Unknown);
	EXPECT_EQ(style.tabSpace, -1);
}

/*!
 * indent_style = tabだけが指定された場合も設定を適用する。
 * 幅が決まらないため設定ごと無視されていた。(#2479)
 */
TEST_F(DetectIndentationStyleTest, IndentStyleTabAloneIsApplied)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	// 幅の指定が無いので-1のままとし、呼び出し元に現在のタブ幅を維持させる
	EXPECT_EQ(style.tabSpace, -1);
}

/*!
 * indent_style = tabでtab_widthが無い場合はindent_sizeを幅に使う。
 */
TEST_F(DetectIndentationStyleTest, IndentStyleTabFallsBackToIndentSize)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"indent_size = 2\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	EXPECT_EQ(style.tabSpace, 2);
}

/*!
 * indent_style = tabでは、indent_sizeよりtab_widthを優先する。
 */
TEST_F(DetectIndentationStyleTest, IndentStyleTabPrefersTabWidth)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"indent_size = 2\n"
		"tab_width = 8\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	EXPECT_EQ(style.tabSpace, 8);
}

/*!
 * 数値として解釈できない値は、そのプロパティだけを無視する。
 * ファイル全体の解析を打ち切っていたため、他のプロパティまで捨てられていた。
 */
TEST_F(DetectIndentationStyleTest, UnparsableIndentSizeIsIgnored)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"indent_size = unset\n"
		"tab_width = 8\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	EXPECT_EQ(style.tabSpace, 8);
}

/*!
 * 数値の後ろにゴミが続く値も解釈できない値として扱う。
 */
TEST_F(DetectIndentationStyleTest, PartiallyNumericTabWidthIsIgnored)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"tab_width = 8x\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	// tab_widthを採用しないので幅は決まらない
	EXPECT_EQ(style.tabSpace, -1);
}

/*!
 * indent_style = spaceでインデント幅が決まる場合は設定を適用する。
 */
TEST_F(DetectIndentationStyleTest, IndentStyleSpaceUsesIndentSize)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = space\n"
		"indent_size = 4\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Spaces);
	EXPECT_EQ(style.tabSpace, 4);
}

/*!
 * indent_style = spaceでindent_size = tabの場合はtab_widthを幅に使う。
 */
TEST_F(DetectIndentationStyleTest, IndentStyleSpaceWithIndentSizeTabUsesTabWidth)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = space\n"
		"indent_size = tab\n"
		"tab_width = 8\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Spaces);
	EXPECT_EQ(style.tabSpace, 8);
}

/*!
 * indent_style = spaceでインデント幅が決まらない場合は、
 * 指定が無かったものとしてファイル内容からの検出に委ねる。
 * タブ側と扱いが異なるのは意図的。
 */
TEST_F(DetectIndentationStyleTest, IndentStyleSpaceWithoutWidthFallsBackToContent)
{
	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = space\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Unknown);
	EXPECT_EQ(style.tabSpace, -1);
}

/*!
 * セクション名の波括弧による拡張子の列挙を解釈する。
 */
TEST_F(DetectIndentationStyleTest, SectionGlobMatchesBracedExtensionList)
{
	WriteEditorConfig(
		"[*.{h,cpp}]\n"
		"indent_style = tab\n"
		"tab_width = 3\n"sv);

	const auto style = Detect();

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	EXPECT_EQ(style.tabSpace, 3);
}

/*!
 * 対象セクションが無い.editorconfigは読み飛ばして親へ遡る。
 */
TEST_F(DetectIndentationStyleTest, SearchContinuesToParentDirectory)
{
	const auto childDir = m_dir / "child";
	std::filesystem::create_directories(childDir);

	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"tab_width = 5\n"sv, m_dir);
	WriteEditorConfig(
		"[*.txt]\n"
		"indent_style = space\n"
		"indent_size = 2\n"sv, childDir);

	const CDocLineMgr cDocLineMgr;
	const auto style = Detect(childDir / "test.cpp", cDocLineMgr);

	EXPECT_EQ(style.character, IndentationStyle::Character::Tabs);
	EXPECT_EQ(style.tabSpace, 5);
}

/*!
 * root = trueが指定された.editorconfigで親への遡上を打ち切る。
 */
TEST_F(DetectIndentationStyleTest, RootStopsSearchingParentDirectory)
{
	const auto childDir = m_dir / "child";
	std::filesystem::create_directories(childDir);

	WriteEditorConfig(
		"[*.cpp]\n"
		"indent_style = tab\n"
		"tab_width = 5\n"sv, m_dir);
	WriteEditorConfig(
		"root = true\n"
		"[*.txt]\n"
		"indent_style = space\n"
		"indent_size = 2\n"sv, childDir);

	const CDocLineMgr cDocLineMgr;
	const auto style = Detect(childDir / "test.cpp", cDocLineMgr);

	// 親の[*.cpp]まで到達しない
	EXPECT_EQ(style.character, IndentationStyle::Character::Unknown);
	EXPECT_EQ(style.tabSpace, -1);
}

/*!
 * 拡張子を持たないファイルは.editorconfigの探索対象にしない。
 */
TEST_F(DetectIndentationStyleTest, FileWithoutExtensionIsNotSearched)
{
	WriteEditorConfig(
		"[*]\n"
		"indent_style = tab\n"
		"tab_width = 5\n"sv);

	const CDocLineMgr cDocLineMgr;
	const auto style = Detect(m_dir / "Makefile", cDocLineMgr);

	EXPECT_EQ(style.character, IndentationStyle::Character::Unknown);
	EXPECT_EQ(style.tabSpace, -1);
}

/*!
 * .editorconfigの指定を得られない場合はファイル内容から検出する。
 */
TEST_F(DetectIndentationStyleTest, FallsBackToDetectionFromDocumentContent)
{
	// 一時ディレクトリより上の.editorconfigに影響されないよう探索を打ち切らせる
	WriteEditorConfig("root = true\n"sv);

	constexpr std::wstring_view lines[]{
		L"void f()"sv,
		L"    int a;"sv,
		L"        int b;"sv,
		L"    int c;"sv,
	};

	CDocLineMgr cDocLineMgr;
	for (const auto& line : lines) {
		CDocLine* pLine = cDocLineMgr.AddNewLine();
		ASSERT_THAT(pLine, NotNull());
		pLine->SetDocLineString(line.data(), int(line.length()), false);
	}

	const auto style = Detect(m_dir / "test.cpp", cDocLineMgr);

	EXPECT_EQ(style.character, IndentationStyle::Character::Spaces);
	EXPECT_EQ(style.tabSpace, 4);
}
