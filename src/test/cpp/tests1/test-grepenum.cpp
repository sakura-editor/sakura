/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "grep/CGrepEnumKeys.h"
#include "grep/CGrepEnumFiles.h"
#include "grep/CGrepEnumFolders.h"
#include "grep/CGrepEnumFilterFiles.h"
#include "grep/CGrepEnumFilterFolders.h"

#include <filesystem>
#include <fstream>

namespace {

/*!
	@brief テスト用の一時フォルダー

	%TEMP%\sakura_test_grepenum\<テスト名> に作り、デストラクターで消す。
	読み取り専用などの属性を付けたファイルも消せるよう、削除の前に属性を戻す。
*/
class TempFolder {
public:
	TempFolder()
		: m_path(std::filesystem::temp_directory_path() / L"sakura_test_grepenum" / ::testing::UnitTest::GetInstance()->current_test_info()->name())
	{
		Remove();
		std::filesystem::create_directories(m_path);
	}

	~TempFolder()
	{
		Remove();
	}

	TempFolder(const TempFolder&) = delete;
	TempFolder& operator=(const TempFolder&) = delete;

	//! ファイルを作る(途中のフォルダーも作る)
	void AddFile(const std::filesystem::path& relPath, DWORD attributes = FILE_ATTRIBUTE_NORMAL) const
	{
		const auto path = m_path / relPath;
		std::filesystem::create_directories(path.parent_path());
		std::ofstream(path) << "x";
		::SetFileAttributesW(path.c_str(), attributes);
	}

	//! フォルダーを作る
	void AddFolder(const std::filesystem::path& relPath) const
	{
		std::filesystem::create_directories(m_path / relPath);
	}

	const std::filesystem::path& Path() const noexcept { return m_path; }

private:
	void Remove() const
	{
		std::error_code ec;
		if (std::filesystem::exists(m_path, ec)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(m_path, ec)) {
				::SetFileAttributesW(entry.path().c_str(), FILE_ATTRIBUTE_NORMAL);
			}
		}
		std::filesystem::remove_all(m_path, ec);
	}

	std::filesystem::path m_path;
};

//! 列挙結果の名前を昇順(符号単位)で返す
std::vector<std::wstring> Names(const CGrepEnumFileBase& items)
{
	std::vector<std::wstring> names;
	for (int i = 0; i < items.GetCount(); ++i) {
		names.emplace_back(items.GetFileName(i));
	}
	std::ranges::sort(names);
	return names;
}

//! キーの配列を作る
VGrepEnumKeys Keys(std::initializer_list<const wchar_t*> keys)
{
	return VGrepEnumKeys(keys.begin(), keys.end());
}

} // namespace

// ---------------------------------------------------------------------------
// CGrepEnumFiles
// ---------------------------------------------------------------------------

/*!
	@brief ワイルドカードは英大文字小文字を区別せずに一致すること
*/
TEST(CGrepEnumFiles, MatchesWildcardIgnoringCase)
{
	TempFolder folder;
	folder.AddFile(L"a.txt");
	folder.AddFile(L"b.log");
	folder.AddFile(L"C.TXT");

	CGrepEnumFiles files;
	auto keys = Keys({ L"*.txt" });
	CGrepEnumOptions options;
	EXPECT_EQ(2, files.Enumerates(folder.Path().c_str(), keys, options));
	EXPECT_EQ((std::vector<std::wstring>{ L"C.TXT", L"a.txt" }), Names(files));
}

/*!
	@brief フォルダーはファイルとして列挙されないこと
*/
TEST(CGrepEnumFiles, SkipsFolders)
{
	TempFolder folder;
	folder.AddFolder(L"dir.txt");
	folder.AddFile(L"a.txt");

	CGrepEnumFiles files;
	auto keys = Keys({ L"*.txt" });
	CGrepEnumOptions options;
	files.Enumerates(folder.Path().c_str(), keys, options);
	EXPECT_EQ((std::vector<std::wstring>{ L"a.txt" }), Names(files));
}

/*!
	@brief 複数のキーに一致しても 1 回だけ列挙されること(大文字小文字違いのキーを含む)
*/
TEST(CGrepEnumFiles, NoDuplicatesAcrossKeys)
{
	TempFolder folder;
	folder.AddFile(L"a.txt");

	CGrepEnumFiles files;
	auto keys = Keys({ L"*.txt", L"a.*", L"A.TXT" });
	CGrepEnumOptions options;
	EXPECT_EQ(1, files.Enumerates(folder.Path().c_str(), keys, options));
	EXPECT_EQ((std::vector<std::wstring>{ L"a.txt" }), Names(files));
}

/*!
	@brief フォルダー部分を含むキーでは、名前にフォルダー部分が付くこと
*/
TEST(CGrepEnumFiles, KeyWithSubFolder)
{
	TempFolder folder;
	folder.AddFile(L"x.txt");
	folder.AddFile(LR"(sub\x.txt)");

	CGrepEnumFiles files;
	auto keys = Keys({ LR"(sub\*.txt)" });
	CGrepEnumOptions options;
	files.Enumerates(folder.Path().c_str(), keys, options);
	EXPECT_EQ((std::vector<std::wstring>{ LR"(sub\x.txt)" }), Names(files));
}

/*!
	@brief 存在しないフォルダーでは何も列挙されず、エラーにもならないこと
*/
TEST(CGrepEnumFiles, NonexistentFolder)
{
	TempFolder folder;

	CGrepEnumFiles files;
	auto keys = Keys({ L"*" });
	CGrepEnumOptions options;
	EXPECT_EQ(0, files.Enumerates((folder.Path() / L"none").c_str(), keys, options));
	EXPECT_EQ(0, files.GetCount());
}

/*!
	@brief 区切り文字や接頭辞と同じ文字、日本語を含むファイル名も列挙されること
*/
TEST(CGrepEnumFiles, SpecialCharactersInFileName)
{
	TempFolder folder;
	for (const auto name : { L"a b.txt", L"a,b.txt", L"a;b.txt", L"#x.txt", L"!y.txt", L"テスト.txt", L"x.txt.bak" }) {
		folder.AddFile(name);
	}

	CGrepEnumFiles files;
	auto keys = Keys({ L"*.txt" });
	CGrepEnumOptions options;
	files.Enumerates(folder.Path().c_str(), keys, options);
	EXPECT_EQ((std::vector<std::wstring>{ L"!y.txt", L"#x.txt", L"a b.txt", L"a,b.txt", L"a;b.txt", L"テスト.txt" }), Names(files));
}

/*!
	@brief 短いファイル名(8.3形式)で一致しても、長いファイル名で一致しなければ列挙されないこと

	FindFirstFile は短いファイル名にも一致するが、PathMatchSpec で長いファイル名を確かめ直している。
*/
TEST(CGrepEnumFiles, ShortNameDoesNotMatch)
{
	TempFolder folder;
	folder.AddFile(L"a.html");

	CGrepEnumFiles files;
	auto keys = Keys({ L"*.htm" });
	CGrepEnumOptions options;
	files.Enumerates(folder.Path().c_str(), keys, options);
	EXPECT_EQ(0, files.GetCount());
}

/*!
	@brief 隠し・読み取り専用・システムの各属性を、オプションに従って除外すること
*/
TEST(CGrepEnumFiles, AttributeOptions)
{
	TempFolder folder;
	folder.AddFile(L"n.txt");
	folder.AddFile(L"h.txt", FILE_ATTRIBUTE_HIDDEN);
	folder.AddFile(L"r.txt", FILE_ATTRIBUTE_READONLY);
	folder.AddFile(L"s.txt", FILE_ATTRIBUTE_SYSTEM);
	auto keys = Keys({ L"*.txt" });

	{
		CGrepEnumFiles files;
		CGrepEnumOptions options;
		files.Enumerates(folder.Path().c_str(), keys, options);
		EXPECT_EQ((std::vector<std::wstring>{ L"h.txt", L"n.txt", L"r.txt", L"s.txt" }), Names(files));
	}
	{
		CGrepEnumFiles files;
		CGrepEnumOptions options;
		options.m_bIgnoreHidden = true;
		options.m_bIgnoreReadOnly = true;
		options.m_bIgnoreSystem = true;
		files.Enumerates(folder.Path().c_str(), keys, options);
		EXPECT_EQ((std::vector<std::wstring>{ L"n.txt" }), Names(files));
	}
}

// ---------------------------------------------------------------------------
// CGrepEnumFolders
// ---------------------------------------------------------------------------

/*!
	@brief ファイルと「.」「..」はフォルダーとして列挙されないこと
*/
TEST(CGrepEnumFolders, SkipsFilesAndDots)
{
	TempFolder folder;
	folder.AddFolder(L"a");
	folder.AddFolder(L"b");
	folder.AddFile(L"c");

	CGrepEnumFolders folders;
	auto keys = Keys({ L"*" });
	CGrepEnumOptions options;
	folders.Enumerates(folder.Path().c_str(), keys, options);
	EXPECT_EQ((std::vector<std::wstring>{ L"a", L"b" }), Names(folders));
}

/*!
	@brief 既定のキー「*.*」は「.」を含まない名前のフォルダーにも一致すること

	SetFileKeys() はフォルダーのキーが無いとき「*.*」を入れる。サブフォルダー検索はこれに依存する。
*/
TEST(CGrepEnumFolders, DefaultKeyMatchesNamesWithoutDot)
{
	TempFolder folder;
	folder.AddFolder(L"src");
	folder.AddFolder(L"a.b");

	CGrepEnumFolders folders;
	auto keys = Keys({ L"*.*" });
	CGrepEnumOptions options;
	folders.Enumerates(folder.Path().c_str(), keys, options);
	EXPECT_EQ((std::vector<std::wstring>{ L"a.b", L"src" }), Names(folders));
}

// ---------------------------------------------------------------------------
// CGrepEnumFilterFiles / CGrepEnumFilterFolders(除外の組み合わせ)
// ---------------------------------------------------------------------------

/*!
	@brief 除外ファイルのワイルドカードは英大文字小文字を区別しないこと
*/
TEST(CGrepEnumFilterFiles, ExceptWildcardIgnoringCase)
{
	TempFolder folder;
	folder.AddFile(L"a.txt");
	folder.AddFile(L"a.bak");
	folder.AddFile(L"b.BAK");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*;!*.bak"));
	CGrepEnumFilterFiles files;
	CGrepEnumFiles absExcept;
	files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ L"a.txt" }), Names(files));
}

/*!
	@brief 空白を含む除外ファイルは引用符で囲めば除外されること
*/
TEST(CGrepEnumFilterFiles, ExceptQuotedNameWithSpace)
{
	TempFolder folder;
	folder.AddFile(L"a b.txt");
	folder.AddFile(L"c.txt");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(LR"(*;!"a b.txt")"));
	CGrepEnumFilterFiles files;
	CGrepEnumFiles absExcept;
	files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ L"c.txt" }), Names(files));
}

/*!
	@brief 引用符で囲んだ「,」を含む除外ファイル・除外フォルダー(ダイアログが組み立てる形)

	Issue #2677 の修正後に Grep ダイアログが渡す「#"obj,old";!"a,b.txt"」を、列挙が正しく扱うこと。
*/
TEST(CGrepEnumFilterFiles, ExceptQuotedNamesWithComma)
{
	TempFolder folder;
	folder.AddFile(L"a,b.txt");
	folder.AddFile(L"b.txt");
	folder.AddFolder(L"obj,old");
	folder.AddFolder(L"obj");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(LR"(*.txt;#"obj,old";!"a,b.txt")"));
	{
		CGrepEnumFilterFiles files;
		CGrepEnumFiles absExcept;
		files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
		EXPECT_EQ((std::vector<std::wstring>{ L"b.txt" }), Names(files));
	}
	{
		CGrepEnumFilterFolders folders;
		CGrepEnumFolders absExcept;
		folders.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
		EXPECT_EQ((std::vector<std::wstring>{ L"obj" }), Names(folders));
	}
}

/*!
	@brief 【現状の制限】除外ファイルは基準フォルダー直下で列挙されるので、フォルダー部分を含む検索キーで見つかったファイルには効かない
*/
TEST(CGrepEnumFilterFiles, ExceptDoesNotApplyToKeyWithSubFolder)
{
	TempFolder folder;
	folder.AddFile(LR"(sub\x.bak)");

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(LR"(sub\*;!*.bak)"));
	CGrepEnumFilterFiles files;
	CGrepEnumFiles absExcept;
	files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ LR"(sub\x.bak)" }), Names(files));
}

/*!
	@brief 絶対パスの除外ファイルが除外されること

	CGrepAgent::DoGrep() と同じく、絶対パスの除外は基準フォルダー無し(L"")で列挙してから渡す。
	パスに空白が含まれうるので引用符で囲む。
*/
TEST(CGrepEnumFilterFiles, ExceptAbsolutePath)
{
	TempFolder folder;
	folder.AddFile(L"a.txt");
	folder.AddFile(L"b.txt");

	CGrepEnumKeys keys;
	const auto excludePath = (folder.Path() / L"a.txt").wstring();
	const std::wstring fileKeys = L"*;!\"" + excludePath + L"\"";
	ASSERT_EQ(0, keys.SetFileKeys(fileKeys.c_str()));
	ASSERT_EQ((std::vector<std::wstring>{ excludePath }), std::vector<std::wstring>(keys.m_vecExceptAbsFileKeys.cbegin(), keys.m_vecExceptAbsFileKeys.cend()));

	CGrepEnumOptions options;
	CGrepEnumFiles absExcept;
	absExcept.Enumerates(L"", keys.m_vecExceptAbsFileKeys, options);
	EXPECT_EQ(1, absExcept.GetCount());

	CGrepEnumFilterFiles files;
	files.Enumerates(folder.Path().c_str(), keys, options, absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ L"b.txt" }), Names(files));
}

/*!
	@brief 絶対パスの除外ファイルにワイルドカードを使えること
*/
TEST(CGrepEnumFilterFiles, ExceptAbsolutePathWithWildcard)
{
	TempFolder folder;
	folder.AddFile(L"a.bak");
	folder.AddFile(L"b.bak");
	folder.AddFile(L"c.txt");

	CGrepEnumKeys keys;
	const auto excludePath = (folder.Path() / L"*.bak").wstring();
	const std::wstring fileKeys = L"*;!\"" + excludePath + L"\"";
	ASSERT_EQ(0, keys.SetFileKeys(fileKeys.c_str()));

	CGrepEnumOptions options;
	CGrepEnumFiles absExcept;
	absExcept.Enumerates(L"", keys.m_vecExceptAbsFileKeys, options);
	EXPECT_EQ(2, absExcept.GetCount());

	CGrepEnumFilterFiles files;
	files.Enumerates(folder.Path().c_str(), keys, options, absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ L"c.txt" }), Names(files));
}

/*!
	@brief 除外フォルダーは名前が一致したものだけ除外され、英大文字小文字を区別しないこと
*/
TEST(CGrepEnumFilterFolders, ExceptFolder)
{
	TempFolder folder;
	for (const auto name : { L"Obj", L"obj2", L"obj,old", L"src" }) {
		folder.AddFolder(name);
	}

	CGrepEnumKeys keys;
	ASSERT_EQ(0, keys.SetFileKeys(L"*;#OBJ"));
	CGrepEnumFilterFolders folders;
	CGrepEnumFolders absExcept;
	folders.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ L"obj,old", L"obj2", L"src" }), Names(folders));
}

/*!
	@brief 絶対パスの除外フォルダーが除外されること

	CGrepAgent::DoGrep() と同じく、絶対パスの除外は基準フォルダー無し(L"")で列挙してから渡す。
*/
TEST(CGrepEnumFilterFolders, ExceptAbsolutePath)
{
	TempFolder folder;
	folder.AddFolder(L"obj");
	folder.AddFolder(L"src");

	CGrepEnumKeys keys;
	const auto excludePath = (folder.Path() / L"obj").wstring();
	const std::wstring fileKeys = L"*;#\"" + excludePath + L"\"";
	ASSERT_EQ(0, keys.SetFileKeys(fileKeys.c_str()));
	ASSERT_EQ((std::vector<std::wstring>{ excludePath }), std::vector<std::wstring>(keys.m_vecExceptAbsFolderKeys.cbegin(), keys.m_vecExceptAbsFolderKeys.cend()));

	CGrepEnumOptions options;
	CGrepEnumFolders absExcept;
	absExcept.Enumerates(L"", keys.m_vecExceptAbsFolderKeys, options);
	EXPECT_EQ(1, absExcept.GetCount());

	CGrepEnumFilterFolders folders;
	folders.Enumerates(folder.Path().c_str(), keys, options, absExcept);
	EXPECT_EQ((std::vector<std::wstring>{ L"src" }), Names(folders));
}
