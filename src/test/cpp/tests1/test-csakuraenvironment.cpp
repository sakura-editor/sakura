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
#include "util/module.h"
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
	static inline std::filesystem::path exePath{};
	static inline std::filesystem::path dummyDir{};
	static inline std::filesystem::path mru1Dir{};
	static inline std::filesystem::path testDataPath{};

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		exePath = GetExeFileName();
		dummyDir = exePath.parent_path() / L"dummy_dir";
		mru1Dir = exePath.parent_path() / L"mru1";
		testDataPath = exePath.replace_filename("test_data.txt");

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
		std::error_code ec;

		if (exists(testDataPath)) {
			std::filesystem::remove(testDataPath, ec);
		}

		if (exists(mru1Dir)) {
			std::filesystem::remove_all(mru1Dir, ec);
		}

		if (exists(dummyDir)) {
			std::filesystem::remove_all(dummyDir, ec);
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

TEST_F(CSakuraEnvironmentTest, ExpandParameter001)
{
	// 呼ぶだけ
	ExpandParameter(L"${w?$h$:アウトプット$:${I?$f$n$:$N$n$}$}${U?(更新)$} - $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>");	// STR_ERR_CSHAREDATA17を更新して使うべき。
}

TEST_F(CSakuraEnvironmentTest, ExpandParameter002)
{
	// 呼ぶだけ
	ExpandParameter(L"${w?$h$:アウトプット$:$f$n$}${U?(更新)$} - $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>");	// STR_ERR_CSHAREDATA18を更新して使うべき。
}

TEST_F(CSakuraEnvironmentTest, ExpandParameter003)
{
	// 呼ぶだけ
	ExpandParameter(L"${w?【Grep】$h$:【アウトプット】$:$f$n$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}");	// STR_ERR_CSHAREDATA10を更新して使うべき。
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

	std::error_code ec;
	std::filesystem::remove(link, ec);
	std::filesystem::remove(target, ec);
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

/*!
 * Kernel32のAPI呼出テスト
 */
struct Kernel32 : public ::testing::Test {
	using Target = ::Kernel32;

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		Target::setInstance<MockKernel32>();

		pKernel32 = (MockKernel32*)Target::getInstance();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		Target::resetInstance();
	}

	MockKernel32* pKernel32 = nullptr;
};

TEST_F(Kernel32, GetCurrentDirectoryW101)
{
	// APIが0を返したら例外。
	EXPECT_CALL(*pKernel32, GetCurrentDirectoryW(_, _))
		.WillOnce(Return(0));

	// システム例外のメッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::GetCurrentDirectoryW();
		}),
		ThrowsMessage<std::system_error>(StartsWith("GetCurrentDirectoryW() failed"))
	);
}

TEST_F(Kernel32, GetCurrentDirectoryW102)
{
	// バッファを溢れさせる
	EXPECT_CALL(*pKernel32, GetCurrentDirectoryW(_, _))
		.WillOnce(Invoke([](DWORD nBufferLength, LPWSTR lpBuffer) -> DWORD {
			return nBufferLength + 1;
		}));

	// メッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::GetCurrentDirectoryW();
		}),
		ThrowsMessage<std::overflow_error>(StartsWith("current path is too long."))
	);
}

TEST_F(Kernel32, GetModuleFileNameW101)
{
	// APIが0を返したら例外。
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Return(0));

	// システム例外のメッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::GetModuleFileNameW(nullptr);
		}),
		ThrowsMessage<std::system_error>(StartsWith("GetModuleFileNameW() failed"))
	);
}

TEST_F(Kernel32, GetModuleFileNameW102)
{
	// バッファを溢れさせる
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Invoke([](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			return nSize;
		}));

	// メッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::GetModuleFileNameW(nullptr);
		}),
		ThrowsMessage<std::overflow_error>(StartsWith("module file path is too long."))
	);
}

TEST_F(Kernel32, GetModuleFileNameW103)
{
	const auto hModule = std::bit_cast<HMODULE>(uintptr_t{ 1 });
	constexpr std::wstring_view expected = L"C:\\sakura\\sakura.exe";

	EXPECT_CALL(*pKernel32, GetModuleFileNameW(hModule, _, _))
		.WillOnce(Invoke([expected](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			std::ranges::copy(expected, lpFilename);
			lpFilename[expected.size()] = L'\0';
			return DWORD(expected.size());
		}));

	EXPECT_THAT(cxx::GetModuleFileNameW(hModule), StrEq(expected.data()));
}

TEST_F(Kernel32, GetExeFileName001)
{
	// SFilePathに収まる最大長のパスを返す
	const std::wstring expected(SFilePath::size() - 1, L'a');
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Invoke([expected](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			std::ranges::copy(expected, lpFilename);
			lpFilename[expected.size()] = L'\0';
			return DWORD(expected.size());
		}));

	EXPECT_THAT(GetExeFileName().native(), StrEq(expected.c_str()));
}

TEST_F(Kernel32, GetExeFileName101)
{
	// 終端NULを含めるとSFilePathに収まらない長さのパスを返す
	const std::wstring path(SFilePath::size(), L'a');
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Invoke([path](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			std::ranges::copy(path, lpFilename);
			lpFilename[path.size()] = L'\0';
			return DWORD(path.size());
		}));

	EXPECT_THAT(([] {
			GetExeFileName();
		}),
		ThrowsMessage<std::overflow_error>(StartsWith("exe path is too long."))
	);
}

TEST_F(Kernel32, GetSystemDirectoryW101)
{
	// APIが0を返したら例外。
	EXPECT_CALL(*pKernel32, GetSystemDirectoryW(_, _))
		.WillOnce(Return(0));

	// システム例外のメッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::GetSystemDirectoryW();
		}),
		ThrowsMessage<std::system_error>(StartsWith("GetSystemDirectoryW() failed"))
	);
}

TEST_F(Kernel32, GetSystemDirectoryW102)
{
	// バッファを溢れさせる
	EXPECT_CALL(*pKernel32, GetSystemDirectoryW(_, _))
		.WillOnce(Invoke([](LPWSTR lpBuffer, UINT uSize) -> UINT {
			return uSize + 1;
		}));

	// メッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::GetSystemDirectoryW();
		}),
		ThrowsMessage<std::overflow_error>(StartsWith("system directory path is too long."))
	);
}

TEST_F(Kernel32, SetCurrentDirectoryW101)
{
	// APIが0を返したら例外。
	EXPECT_CALL(*pKernel32, SetCurrentDirectoryW(_))
		.WillOnce(Return(FALSE));

	// システム例外のメッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			cxx::SetCurrentDirectoryW(L"path/to/file");
		}),
		ThrowsMessage<std::system_error>(StartsWith("SetCurrentDirectoryW() failed"))
	);
}

/*!
 * ChangeCurrentDirectoryToExeDirのテスト
 */
struct ChangeCurrentDirectoryToExeDir : public ::testing::Test {
	using Kernel32 = ::Kernel32;

	void SetUp() override
	{
		Kernel32::setInstance<MockKernel32>();

		pKernel32 = (MockKernel32*)Kernel32::getInstance();
	}

	void TearDown() override
	{
		Kernel32::resetInstance();
	}

	MockKernel32* pKernel32 = nullptr;
};

/*!
 * @brief 実行ファイルのディレクトリへの移動に成功するパターン
 */
TEST_F(ChangeCurrentDirectoryToExeDir, test001)
{
	constexpr std::wstring_view exePath = LR"(C:\sakura\sakura.exe)";
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Invoke([exePath](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			std::ranges::copy(exePath, lpFilename);
			lpFilename[exePath.size()] = L'\0';
			return DWORD(exePath.size());
		}));

	EXPECT_CALL(*pKernel32, SetCurrentDirectoryW(StrEq(LR"(C:\sakura)")))
		.WillOnce(Return(TRUE));

	::ChangeCurrentDirectoryToExeDir();
}

/*!
 * @brief 実行ファイルのパスが長過ぎるとき
 */
TEST_F(ChangeCurrentDirectoryToExeDir, test101)
{
	const std::wstring exePath(_MAX_PATH, L'a');
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Invoke([exePath](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			std::ranges::copy(exePath, lpFilename);
			lpFilename[exePath.size()] = L'\0';
			return DWORD(exePath.size());
		}));
	EXPECT_CALL(*pKernel32, SetCurrentDirectoryW(_)).Times(0);

	EXPECT_THAT(([] {
			::ChangeCurrentDirectoryToExeDir();
		}),
		ThrowsMessage<std::overflow_error>(StartsWith("exe path is too long."))
	);
}

/*!
 * @brief カレントディレクトリの変更に失敗したとき
 */
TEST_F(ChangeCurrentDirectoryToExeDir, test102)
{
	constexpr std::wstring_view exePath = LR"(C:\sakura\sakura.exe)";
	EXPECT_CALL(*pKernel32, GetModuleFileNameW(nullptr, _, _))
		.WillOnce(Invoke([exePath](HMODULE hModule, LPWSTR lpFilename, DWORD nSize) -> DWORD {
			std::ranges::copy(exePath, lpFilename);
			lpFilename[exePath.size()] = L'\0';
			return DWORD(exePath.size());
		}));

	EXPECT_CALL(*pKernel32, SetCurrentDirectoryW(StrEq(LR"(C:\sakura)")))
		.WillOnce(Return(FALSE));

	EXPECT_THAT(([] {
			::ChangeCurrentDirectoryToExeDir();
		}),
		ThrowsMessage<std::system_error>(StartsWith("SetCurrentDirectoryW() failed"))
	);
}

/*!
 * CCurrentDirectoryBackupPointのテスト
 */
struct CCurrentDirectoryBackupPoint : public ::testing::Test {
	using Target = ::CCurrentDirectoryBackupPoint;
	using Kernel32 = ::Kernel32;

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		Kernel32::setInstance<MockKernel32>();

		pKernel32 = (MockKernel32*)Kernel32::getInstance();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		Kernel32::resetInstance();
	}

	MockKernel32* pKernel32 = nullptr;
};

/*!
 * @brief カレントディレクトリの取得＆復元に成功するパターン
 */
TEST_F(CCurrentDirectoryBackupPoint, test001)
{
	// それっぽいカレントディレクトリを返す
	EXPECT_CALL(*pKernel32, GetCurrentDirectoryW(_, _))
		.WillOnce(Invoke([](DWORD nBufferLength, LPWSTR lpBuffer) -> DWORD {
			std::wstring buffer = LR"(C:\Windows\System32)";
			::wcsncpy_s(lpBuffer, nBufferLength, buffer.c_str(), _TRUNCATE);
			return DWORD(buffer.length() + 1);
		}));

	// APIが1を返したら正常。
	EXPECT_CALL(*pKernel32, SetCurrentDirectoryW(_))
		.WillOnce(Return(TRUE));

	// テスト実行（上記APIが呼ばれる）
	{
		Target t;
	}
}

/*!
 * @brief カレントディレクトリの取得に失敗したとき
 */
TEST_F(CCurrentDirectoryBackupPoint, test101)
{
	// APIが0を返したら例外。
	EXPECT_CALL(*pKernel32, GetCurrentDirectoryW(_, _))
		.WillOnce(Return(0));

	// システム例外のメッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			Target t;
		}),
		ThrowsMessage<std::system_error>(StartsWith("GetCurrentDirectoryW() failed"))
	);
}

/*!
 * @brief 長過ぎるディレクトリが返った時
 */
TEST_F(CCurrentDirectoryBackupPoint, test102)
{
	// 長さ_MAX_PATHのパスを返す
	EXPECT_CALL(*pKernel32, GetCurrentDirectoryW(_, _))
		.WillOnce(Invoke([](DWORD nBufferLength, LPWSTR lpBuffer) -> DWORD {
			std::wstring buffer(_MAX_PATH, L'a');
			::wcsncpy_s(lpBuffer, nBufferLength, buffer.c_str(), _TRUNCATE);
			return DWORD(buffer.length());
		}));

	// メッセージは先頭一致で評価する
	EXPECT_THAT(([] {
			Target t;
		}),
		ThrowsMessage<std::overflow_error>(StartsWith("source string is too long."))
	);
}

/*!
 * @brief ディレクトリの復元に失敗したとき
 */
TEST_F(CCurrentDirectoryBackupPoint, test103)
{
	// それっぽいカレントディレクトリを返す
	EXPECT_CALL(*pKernel32, GetCurrentDirectoryW(_, _))
		.WillOnce(Invoke([](DWORD nBufferLength, LPWSTR lpBuffer) -> DWORD {
			std::wstring buffer = LR"(C:\Windows\System32)";
			::wcsncpy_s(lpBuffer, nBufferLength, buffer.c_str(), _TRUNCATE);
			return DWORD(buffer.length() + 1);
		}));

	// APIが0を返したら例外。
	EXPECT_CALL(*pKernel32, SetCurrentDirectoryW(_))
		.WillOnce(Return(FALSE));

	// テスト実行（失敗はログで確認する）
	{
		Target t;
	}
}

} // namespace env
