/*! @file */
/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/CSakuraEnvironment.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "cxx/com_pointer.hpp"
#include "io/CFileLoad.h"
#include "util/file.h"
#include "util/os.h"
#include "util/tchar_convert.h"
#include "recent/CMRUFolder.h"
#include "version.h"

#include "window/EditorTestSuite.hpp"

#include <fstream>

using namespace std::literals::string_literals;

std::filesystem::path GetTempFilePathWithExt(std::wstring_view prefix, std::wstring_view extension);

namespace env {

const std::wstring versionStr{ LTEXT(VERSION_STR) };

/*!
 * @brief 期待値を表す構造体
 *
 * resourceIdが0でないときはresourceIdに対応するリソース文字列を使う
 * resourceIdが0のときはsourceを使う
 */
struct SExpectedStr {
	WORD resourceId = 0;
	std::wstring source{};

	constexpr /* implicit */ SExpectedStr(int expected)
		: resourceId(static_cast<WORD>(expected))
	{
	}

	constexpr /* implicit */ SExpectedStr(const std::wstring& expected)
		: source(expected)
	{
	}

	/* implicit */ SExpectedStr(const std::filesystem::path& expected)
		: SExpectedStr(expected.native())
	{
	}

	/* implicit */ operator std::wstring() const &
	{
		if (!resourceId) {
			return source;
		}
		return std::wstring{ cxx::load_string(resourceId) };
	}
};

/*!
 * googletestにSExpectedStrを出力させる
 * 
 * パラメータテストのパラメータにSExpectedStrを渡した場合に文字列を併記して分かりやすくする。
 */
void PrintTo(SExpectedStr expected, std::ostream* os)
{
	*os << '\"' <<cxx::to_string(static_cast<std::wstring>(expected), CP_UTF8) << '\"';
}

template<typename Base>
struct TSakuraEnvironmentTest : public Base, public window::EditorTestSuite {
	static inline std::filesystem::path exePath = GetExeFileName();
	static inline std::filesystem::path dummyDir = exePath.parent_path() / L"dummy_dir";
	static inline std::filesystem::path mru1Dir = exePath.parent_path() / L"mru1";
	static inline std::filesystem::path testDataPath = exePath.replace_filename("test_data.txt");

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpEditor();

		if (!exists(dummyDir)) {
			std::filesystem::create_directories(dummyDir);
		}

		if (!exists(mru1Dir)) {
			std::filesystem::create_directories(mru1Dir);
		}

		// ファイル出力ストリームをバイナリモードで開く
		std::ofstream fs(testDataPath, std::ios::binary);

		// UTF-8 BOMを出力
		const std::array bom = { '\xEF', '\xBB', '\xBF' };
		fs.write(bom.data(), bom.size());

		// 各行を書き込む
		fs << "test data.\r\n";

		fs.close();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		if (exists(testDataPath)) {
			std::filesystem::remove(testDataPath);
		}

		if (exists(mru1Dir)) {
			std::filesystem::remove_all(mru1Dir);
		}

		if (exists(dummyDir)) {
			std::filesystem::remove_all(dummyDir);
		}

		TearDownEditor();
	}

	std::wstring ExpandParameter(std::wstring_view source, std::optional<size_t> optBufSize = std::nullopt) const
	{
		// 受け取り用バッファを用意する
		const auto bufSize = optBufSize.value_or(_MAX_PATH);
		std::wstring buffer(bufSize, L'\0');

		CSakuraEnvironment::ExpandParameter(std::data(source), std::data(buffer), int(std::size(buffer)));
		buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));

		return buffer;
	}

	std::wstring ResolvePath(const std::filesystem::path& link) const
	{
		// 受け取り用バッファを用意する
		std::wstring buffer{ link.native() };
		buffer.resize(4096, L'\0');

		CSakuraEnvironment::ResolvePath(std::data(buffer));
		buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));

		return buffer;
	}
};

using CSakuraEnvironmentTest = TSakuraEnvironmentTest<::testing::Test>;

/*!
 * @brief プロファイル名の取得
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_ProfileName)
{
	EXPECT_THAT(ExpandParameter(L"$<profile>"), StrEq(L""));

	// コマンドラインを書き替える
	auto pCommandLine = std::make_unique<CCommandLine>();
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	EXPECT_THAT(ExpandParameter(L"$<profile>"), StrEq(L"profile1"));

	// コマンドラインを書き替える
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// バッファを再確保する
	EXPECT_THAT(ExpandParameter(L"$<profile>"), StrEq(L""));
}

/*!
 * @brief 現在日時の取得
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_Date)
{
	// 日付の書式 yyyy年MM月dd日
	EXPECT_THAT(cxx::to_string(ExpandParameter(L"$d")), MatchesRegex(R"(2\d+年\d+月\d+日)"));
}

/*!
 * @brief 現在日時の取得
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_Time)
{
	// 時刻の書式 HH:mm:ss
	EXPECT_THAT(cxx::to_string(ExpandParameter(L"$t")), MatchesRegex(R"(\d+:\d+:\d+)"));
}

/*!
 * @brief ExpandParameterのテスト
 */
TEST_F(CSakuraEnvironmentTest, ExpandParameter_mixed)
{
	const auto& type = pcEditDoc->m_cDocType.GetDocumentAttribute();

	auto& sFileInfo = pcEditDoc->m_cDocFile.m_sFileInfo;

	pcEditDoc->m_cDocFile.SetFilePath(testDataPath.c_str());

	// ファイルを開く
	CFileLoad cfl(type.m_encoding);
	bool bBigFile = false;
	bool bBom = false;
	cfl.FileOpen(testDataPath.c_str(), bBigFile, CODE_UTF8, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &bBom);
	sFileInfo.SetBomExist(bBom);

	// ファイル時刻の取得
	if (FILETIME ft{}; cfl.GetFileTime(nullptr, nullptr, &ft)) {
		sFileInfo.SetFileTime(ft);
	}

	EXPECT_THAT(ExpandParameter(L"$F"), StrEq(testDataPath.c_str()));

	EXPECT_THAT(ExpandParameter(L"$f"), StrEq(testDataPath.filename().c_str()));

	EXPECT_THAT(ExpandParameter(L"$g"), StrEq(testDataPath.stem().c_str()));

	EXPECT_THAT(ExpandParameter(L"$/"), StrEq(std::regex_replace(testDataPath.native(), std::wregex(LR"(\\)"), L"/").c_str()));

	EXPECT_THAT(ExpandParameter(L"$N"), StrEq(testDataPath.c_str()));

	EXPECT_THAT(ExpandParameter(L"$E"), StrEq((testDataPath.parent_path() / "").c_str()));

	EXPECT_THAT(ExpandParameter(L"$e"), StrEq((testDataPath.parent_path() / "").c_str()));

	EXPECT_THAT(ExpandParameter(L"$b"), StrEq(testDataPath.extension().native().substr(1).c_str()));

	// 日付の書式 yyyy年MM月dd日
	EXPECT_THAT(cxx::to_string(ExpandParameter(L"$D")), MatchesRegex(R"(2\d+年\d+月\d+日)"));

	// 時刻の書式 HH:mm:ss
	EXPECT_THAT(cxx::to_string(ExpandParameter(L"$T")), MatchesRegex(R"(\d+:\d+:\d+)"));

	cfl.FileClose();

	pcEditDoc->m_cDocFile.SetFilePath(L"");
}

/*!
 * @brief 初期ディレクトリの取得
 */
TEST_F(CSakuraEnvironmentTest, GetDlgInitialDir001)
{
	// カレントディレクトリーをバックアップする
	CCurrentDirectoryBackupPoint backupPoint;

	// カレントディレクトリーを変更する
	::SetCurrentDirectoryW(dummyDir.c_str());

	bool isControlProcess = true;

	EXPECT_THAT(CSakuraEnvironment::GetDlgInitialDir(isControlProcess), StrEq(dummyDir.c_str()));

	CMRUFolder cMRU;
	cMRU.Add(mru1Dir.c_str());

	EXPECT_THAT(CSakuraEnvironment::GetDlgInitialDir(isControlProcess), StrEq(mru1Dir.c_str()));

	cMRU.ClearAll();

	isControlProcess = false;

	EXPECT_THAT(CSakuraEnvironment::GetDlgInitialDir(isControlProcess), StrEq(dummyDir.c_str()));

	GetDllShareData().m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_SEL;
	GetDllShareData().m_Common.m_sEdit.m_OpenDialogSelDir = exePath.parent_path();
	EXPECT_THAT(CSakuraEnvironment::GetDlgInitialDir(isControlProcess), StrEq(exePath.parent_path().c_str()));

	isControlProcess = true;

	std::filesystem::path path{ LR"(X:\Undefined\NotExists\unsaved.txt)" };
	pcEditDoc->m_cDocFile.SetFilePath(path.c_str());
	EXPECT_THAT(CSakuraEnvironment::GetDlgInitialDir(isControlProcess), StrEq(path.remove_filename().c_str()));

	pcEditDoc->m_cDocFile.SetFilePath(L"");

	// 不正な値を入れて例外発生をチェックする
	GetDllShareData().m_Common.m_sEdit.m_eOpenDialogDir = EOpenDialogDir(3);
	EXPECT_ANY_THROW(CSakuraEnvironment::GetDlgInitialDir(isControlProcess));

	// 設定を元に戻しておく
	GetDllShareData().m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_CUR;
}

/*!
 * @brief ショートカットの解決とロングファイル名へ変換
 *
 * 実際にショートカットを生成して、パス解決できることを確認する
 */
TEST_F(CSakuraEnvironmentTest, ResolvePath001)
{
	// ショートカットのターゲットとなるファイルを作成する
	const auto target = GetTempFilePathWithExt(L"lnk", L"txt");
	std::wofstream fs(target);
	EXPECT_TRUE(fs.is_open());
	fs << "dummy";
	fs.close();

	// IShellLinkを作成する
	cxx::com_pointer<IShellLink> shellLink;
	EXPECT_HRESULT_SUCCEEDED(shellLink.CreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER));
	EXPECT_HRESULT_SUCCEEDED(shellLink->SetPath(target.c_str()));

	// ショートカットを作成する
	const auto link = GetTempFilePathWithExt(L"tar", L"lnk");
	cxx::com_pointer<IPersistFile> persistFile;
	EXPECT_HRESULT_SUCCEEDED(shellLink->QueryInterface(&persistFile));
	EXPECT_HRESULT_SUCCEEDED(persistFile->Save(link.c_str(), TRUE));

	// ロングファイル名を取得する
	std::wstring targetLongFileName(4096, L'\0');
	if (::GetLongFileName(target.c_str(), std::data(targetLongFileName))) {
		targetLongFileName.resize(::wcsnlen(std::data(targetLongFileName), std::size(targetLongFileName)));
	}

	// パス解決を試みる
	EXPECT_THAT(ResolvePath(link), StrEq(targetLongFileName));

	std::filesystem::remove(link);
	std::filesystem::remove(target);
}

/*!
 * @brief ExpandParamテストのパラメーター
 *
 * @param source 展開パターンを含む文字列
 * @param expected 展開後の文字列（期待値）
 */
using ExpandParamTestParam = std::tuple<std::wstring_view, SExpectedStr>;

//! GetEolテストのためのフィクスチャクラス
using ExpandParamTest = TSakuraEnvironmentTest<::testing::TestWithParam<ExpandParamTestParam>>;

/*!
 * @brief 環境パラメーターの展開
 */
TEST_P(ExpandParamTest, test)
{
	const auto  source     = std::get<0>(GetParam());
	const auto& expected   = std::get<1>(GetParam());
	EXPECT_THAT(ExpandParameter(std::data(source)), StrEq(expected));
}

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(ExpandParamCases
	, ExpandParamTest
	, ::testing::Values(
		ExpandParamTestParam{ L"$$",           L"$"s                  },
		ExpandParamTestParam{ L"$A",           STR_GSTR_APPNAME       },
		ExpandParamTestParam{ L"$F",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$f",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$g",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$/",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$N",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$E",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$e",           STR_NO_TITLE1          },
		ExpandParamTestParam{ L"$B",           L""s                   },
		ExpandParamTestParam{ L"$Q",           L"印刷設定 1"s         },
		ExpandParamTestParam{ L"$C",           L""s                   },
		ExpandParamTestParam{ L"$x",           L"1"s                  },
		ExpandParamTestParam{ L"$y",           L"1"s                  },
		ExpandParamTestParam{ L"$P",           STR_PREVIEW_ONLY       },
		ExpandParamTestParam{ L"$p",           STR_PREVIEW_ONLY       },
		ExpandParamTestParam{ L"$D",           STR_NOT_SAVED          },
		ExpandParamTestParam{ L"$T",           STR_NOT_SAVED          },
		ExpandParamTestParam{ L"$V",           versionStr             },
		ExpandParamTestParam{ L"$S",           GetExeFileName()       },
		ExpandParamTestParam{ L"$I",           GetIniFileName()       },
		ExpandParamTestParam{ L"$<profile>",   L""s                   }
	)
);

} // namespace env
