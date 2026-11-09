/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "util/file.h"

#include "_main/CProcessFactory.h"

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include <cstdlib>
#include <fstream>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "io/CTextStream.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "dlg/CDlgTagJumpList.h"
#include "env/CDataProfile.h"
#include "util/file/FileMapping.hpp"
#include "util/file/NamedFilePointer.hpp"
#include "util/os.h"

#include "testing/MsvcInvalidParameterHandlerDisabler.hpp"
#include "testing/MsvcReportMode.hpp"

#include "util/os/MockKernel32.hpp"

std::filesystem::path GetIniFileNameForIO(bool bWrite);

namespace cxx {

using namespace testing;

int FdCloseFunc(const int* pFd) noexcept;
void resize_file(int fd, int64_t fileSize);

std::wstring GetTempPath2W();

bool WritePrivateProfileStringW(
	std::wstring_view appName,
	std::wstring_view keyName,
	std::wstring_view varValue,
	const std::optional<std::filesystem::path>& iniPath = std::nullopt
) noexcept
{
	return ::WritePrivateProfileStringW(std::data(appName), std::data(keyName), std::data(varValue), iniPath.has_value() ? iniPath.value().c_str() : nullptr);
}

std::wstring ExpandEnvironmentStringsW(const std::wstring& src)
{
	std::wstring expected(2048, L'\0');
	if (const auto ret = ::ExpandEnvironmentStringsW(std::data(src), std::data(expected), DWORD(std::size(expected)))) {
		expected.resize(ret - 1);
	}
	return expected;
}

TEST(CopyDirDir, test001)
{
	WCHAR szTemp[_MAX_PATH];
	EXPECT_THAT(CopyDirDir(szTemp, L"Windows", LR"(C:)"), StrEq(LR"(C:\Windows\)"));
}

TEST(CopyDirDir, test002)
{
	WCHAR szTemp[_MAX_PATH];
	EXPECT_THAT(CopyDirDir(szTemp, LR"(C:\Windows)", LR"(A:)"), StrEq(LR"(C:\Windows\)"));
}

TEST(CopyDirDir, test101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	WCHAR szTemp5[5];	// バッファ不足を発生させる
	EXPECT_THAT(CopyDirDir(szTemp5, L"Windows", LR"(C:)"), StrEq(L""));	// 入りきらない場合、バッファは空になる

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

TEST(CopyDirDir, test102)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	WCHAR szTemp3[3];	// バッファ不足を発生させる
	EXPECT_THAT(CopyDirDir(szTemp3, L"Windows", LR"(C:)"), StrEq(L""));	// 入りきらない場合、バッファは空になる

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

TEST(CopyDirDir, test103)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	WCHAR szTemp11[11];	// バッファ不足を発生させる
	EXPECT_THAT(CopyDirDir(szTemp11, L"Windows", LR"(C:)"), StrEq(L""));	// 入りきらない場合、バッファは空になる

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

} // namespace cxx

namespace io {

using namespace testing;

using namespace cxx;

/*!
 * 旧実装 CFileAttributeの妥当性をなんとなく示すテスト
 *
 * Cランタイムの fopen では、隠し属性やシステム属性が付与されたファイルを開けない。
 * 古い実装では、ファイルを開く前に属性を落とし、閉じた後に復帰させるという処理を行っていた。
 */
struct CFileAttribute : public ::testing::Test
{
	static inline std::filesystem::path testDataPath;
	static inline DWORD dwAttributeOld = 0;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		// 一時フォルダーのパスを取得する
		SFilePath szTempDir{ cxx::GetTempPath2W() };

		auto tempDir = std::filesystem::path{ szTempDir };
		tempDir = tempDir.parent_path();

		std::wstring_view prefix{ L"tests1_" };
		std::wstring_view ext{ L".tmp" };

		for(;;) {
			testDataPath = tempDir / std::format(L"{:s}{:04x}{:s}", prefix, LOWORD(::GetTickCount64()), ext);

			// ファイル出力ストリームを開いてデータを書き込む
			std::ofstream os(testDataPath);
			if (!os) continue;
			os << "test" << std::endl;
			os.close();

			break;
		}

		// ファイルの属性を取得
		dwAttributeOld = ::GetFileAttributesW(testDataPath.c_str());
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
	}

	FILE* fp = nullptr;


	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		// 開いているファイルを閉じる
		if (fp) ::fclose(fp);

		// ファイルの属性を元に戻す
		::SetFileAttributesW(testDataPath.c_str(), dwAttributeOld);
	}
};

TEST_F(CFileAttribute, test001)
{
	// 通常ファイルなら fopen でも開ける
	EXPECT_TRUE(0 == ::_wfopen_s(&fp, testDataPath.c_str(), L"wb"));

	// 開いたファイルを閉じる
	if (fp) {
		::fclose(fp);
		fp = nullptr;
	}

	// 自作ライブラリでも、当然開ける
	auto out = CTextOutputStream(testDataPath.c_str());
	EXPECT_THAT(out, IsTrue());
}

TEST_F(CFileAttribute, test002)
{
	// ファイルに隠し属性を付与する
	::SetFileAttributesW(testDataPath.c_str(), dwAttributeOld | FILE_ATTRIBUTE_HIDDEN);

	// 隠し属性を付与したらfopenでは開けない
	EXPECT_FALSE(0 == ::_wfopen_s(&fp, testDataPath.c_str(), L"wb"));
	EXPECT_THAT(fp, IsNull());

	// 自作ライブラリは隠し属性が付いたファイルを開くために作られた
	auto out = CTextOutputStream(testDataPath.c_str());
	EXPECT_THAT(out, IsTrue());
}

TEST_F(CFileAttribute, test003)
{
	// ファイルにシステム属性を付与する
	::SetFileAttributesW(testDataPath.c_str(), dwAttributeOld | FILE_ATTRIBUTE_SYSTEM);

	// システム属性を付与したらfopenでは開けなくなる
	EXPECT_FALSE(0 == ::_wfopen_s(&fp, testDataPath.c_str(), L"wb"));
	EXPECT_THAT(fp, IsNull());

	// 自作ライブラリはシステム属性が付いたファイルを開くために作られた
	auto out = CTextOutputStream(testDataPath.c_str());
	EXPECT_THAT(out, IsTrue());
}

/*!
 * @brief resize_file() のテスト
 */
TEST(resize_file, test101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	EXPECT_THROW(
		cxx::resize_file(-1, 0),
		std::system_error
	);

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief CreateFilePath() のテスト
 *
 * ファイル名を指定してCストリームを開くメソッド。
 */
TEST(FilePointer, CreateFilePath001)
{
	// テスト用に作成するファイルのパス（INIパスを流用）
	const auto path = GetIniFileName();

	// 新規作成したいので削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// 名前を指定してCストリームを開く
	auto fp = FilePointer::CreateFilePath(path);
	EXPECT_TRUE(fp);

	// ファイルが作られる
	EXPECT_TRUE(fexist(path));

	// ファイルを閉じる
	fp = nullptr;

	// ファイルは削除されない
	EXPECT_TRUE(fexist(path));

	// 自分で削除する
	std::filesystem::remove(path, ec);

	// ファイルは削除されている
	EXPECT_FALSE(fexist(path));
}

/*!
 * @brief CreateTempFile() のテスト
 *
 * ファイル名を指定してCストリームを開くメソッド。
 */
TEST(FilePointer, CreateTempFile001)
{
	// 一時ファイルを作成する
	auto fp = FilePointer::CreateTempFile();
	EXPECT_TRUE(fp);

	// 作られた一時ファイルのパスを取得する
	const auto path = std::filesystem::path{ fp.GetPath() };

	// ファイルが作られる
	EXPECT_TRUE(fexist(path));

	// 検証用にテストデータを書き込む
	fp.write("This is test data.");

	// 書き込んだデータをフラッシュする
	fp.seek(0);

	auto other = FilePointer::OpenFilePath(path, L"r+b");
	EXPECT_TRUE(other);

	std::string buffer(_countof("This is test data.") - 1, '\0');
	other.read(buffer);
	EXPECT_THAT( buffer, StrEq("This is test data."));

	// ファイルを閉じる
	other = nullptr;

	// ファイルを閉じる
	fp = nullptr;

	// ファイルは削除されている
	EXPECT_FALSE(fexist(path));
}

/*!
 * @brief OpenFilePath() のテスト
 *
 * ファイル名を指定してCストリームを開くメソッド。
 */
TEST(FilePointer, OpenFilePath001)
{
	// テスト用に作成するファイルのパス（INIパスを流用）
	const auto path = GetIniFileName();

	// 新規作成したいので削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// ファイル出力ストリームを開く
	std::ofstream fos{ path };

	// データを書き込む
	fos << "This is test data." << std::endl;

	fos.close();

	// 名前を指定してCストリームを開く
	auto fp = FilePointer::OpenFilePath(path, L"wb");
	EXPECT_TRUE(fp);

	// ファイルが作られる
	EXPECT_TRUE(fexist(path));

	// ファイルを閉じる
	fp = nullptr;

	// ファイルは削除されない
	EXPECT_TRUE(fexist(path));

	// 自分で削除する
	std::filesystem::remove(path, ec);

	// ファイルは削除されている
	EXPECT_FALSE(fexist(path));
}

/*!
 * @brief OpenFilePath() の異常系テスト
 *
 * パスを指定しないと空が返る
 */
TEST(FilePointer, OpenFilePath101)
{
	auto fp = FilePointer::OpenFilePath(L"", L"wb");
	EXPECT_FALSE(fp);
}

/*!
 * @brief OpenFilePath() の異常系テスト
 *
 * モードを指定しないと例外
 */
TEST(FilePointer, OpenFilePath102)
{
	EXPECT_THROW(
		FilePointer::OpenFilePath(L"test.txt", L""),
		std::invalid_argument
	);
}

/*!
 * @brief OpenFilePath() の異常系テスト
 *
 * パスが長過ぎると例外
 */
TEST(FilePointer, OpenFilePath103)
{
	EXPECT_THROW(
		FilePointer::OpenFilePath(std::wstring(SFilePath::size(), L'a'), L"wb"),
		std::system_error
	);
}

/*!
 * @brief OpenFromHandle() のテスト
 *
 * OSハンドルを指定してCストリームを開くメソッド。
 */
TEST(FilePointer, OpenFromHandle101)
{
	EXPECT_THAT(([] {
		// モード指定がないと例外。
		auto file = FilePointer::OpenFileHandle(FileHandle{ INVALID_HANDLE_VALUE }, L""); }),
		ThrowsMessage<std::invalid_argument>(Eq("missing mode"))
	);
}

/*!
 * @brief OpenFromHandle() のテスト
 *
 * OSハンドルを指定してCストリームを開くメソッド。
 */
TEST(FilePointer, OpenFromHandle102)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	auto file = FilePointer::OpenFileHandle(FileHandle{ INVALID_HANDLE_VALUE }, L"wb");	// 無効なOSハンドルを指定して失敗させる
	EXPECT_THAT(file, IsFalse());

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief OpenFromHandleForWrite() のテスト
 *
 * OSハンドルを指定して書き込み用のCストリームを開くメソッド。
 */
TEST(FilePointer, OpenFromHandleForWrite001)
{
	// テスト用に作成するファイルのパス（INIパスを流用）
	const auto path = GetIniFileName();

	// 新規作成したいので削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// 作成するファイルの属性（一時ファイル、閉じたら削除）
	const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;

	// OSのファイルハンドルを作成する
	auto hFile = FileHandle::CreateNew(path, dwFlagsAndAttributes);
	EXPECT_TRUE(hFile);

	// ファイルが作られる
	EXPECT_TRUE(fexist(path));

	// Cストリームを開く
	auto fp = FilePointer::OpenFileHandle(std::move(hFile), L"wb");
	EXPECT_TRUE(fp);

	// ファイルハンドルはFilePointerに譲渡するので、スマートポインターとの紐付けを解除しておく
	hFile.release();

	// ファイルを閉じる
	fp = nullptr;

	// ファイルは削除されている
	EXPECT_FALSE(fexist(path));
}

/*!
 * @brief OpenFromHandleForWrite() がファイル内容を変更しないことを確認する
 */
TEST(FilePointer, OpenFromHandleForWrite002)
{
	const auto path = GetIniFileName();
	constexpr std::array<std::string_view, 5> testData = {
		"",
		"a",
		"abc",
		"test",
		"This is test data."
	};

	for (const auto expected : testData) {
		SCOPED_TRACE(expected);

		{
			std::ofstream output{ path, std::ios::binary | std::ios::trunc };
			output.write(expected.data(), expected.size());
		}

		auto hFile = FileHandle::OpenExisting(
			path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE
		);
		ASSERT_TRUE(hFile);

		const FILETIME creationTime{ 0, 29000000 };
		const FILETIME lastAccessTime{ 0, 29100000 };
		const FILETIME lastWriteTime{ 0, 29200000 };
		ASSERT_TRUE(::SetFileTime(hFile.get(), &creationTime, &lastAccessTime, &lastWriteTime));

		auto fp = FilePointer::OpenFileHandle(std::move(hFile), L"r+b");
		ASSERT_TRUE(fp);
		EXPECT_FALSE(hFile);

		fp = nullptr;

		auto timestampHandle = FileHandle::OpenExisting(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE);
		ASSERT_TRUE(timestampHandle);
		FILETIME actualCreationTime{}, actualLastAccessTime{}, actualLastWriteTime{};
		ASSERT_TRUE(::GetFileTime(timestampHandle.get(), &actualCreationTime, &actualLastAccessTime, &actualLastWriteTime));
		EXPECT_EQ(0, ::CompareFileTime(&creationTime, &actualCreationTime));
		EXPECT_EQ(0, ::CompareFileTime(&lastAccessTime, &actualLastAccessTime));
		EXPECT_EQ(0, ::CompareFileTime(&lastWriteTime, &actualLastWriteTime));
		timestampHandle = nullptr;

		std::ifstream input{ path, std::ios::binary };
		const std::string actual{
			std::istreambuf_iterator<char>{ input },
			std::istreambuf_iterator<char>{}
		};
		EXPECT_EQ(expected, actual);
	}

	std::error_code ec;
	std::filesystem::remove(path, ec);
}

TEST(FilePointer, check_write_permissionsRestoresTimestampsOnFailure)
{
#if defined(_MSC_VER) && defined(_DEBUG)
	MsvcInvalidParameterHandlerDisabler disabler{};
	MsvcReportMode reportMode{};
#endif

	auto tempFile = FilePointer::CreateTempFile();
	ASSERT_TRUE(tempFile);
	tempFile.write("Original data");
	tempFile.seek(0);

	auto hFile = FileHandle::OpenExisting(
		tempFile.GetPath(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
	);
	ASSERT_TRUE(hFile);
	const FILETIME creationTime{ 0, 29000000 };
	const FILETIME lastAccessTime{ 0, 29100000 };
	const FILETIME lastWriteTime{ 0, 29200000 };
	ASSERT_TRUE(::SetFileTime(hFile.get(), &creationTime, &lastAccessTime, &lastWriteTime));

	// OSハンドルには日時変更権限を残し、Cストリームの書き込みだけを失敗させる。
	auto fp = FilePointer::OpenFileHandle(std::move(hFile), L"rb");
	ASSERT_TRUE(fp);
	EXPECT_THROW(fp.check_write_permissions(), std::system_error);
	fp = nullptr;

	auto timestampHandle = FileHandle::OpenExisting(
		tempFile.GetPath(), FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
	);
	ASSERT_TRUE(timestampHandle);
	FILETIME actualCreationTime{}, actualLastAccessTime{}, actualLastWriteTime{};
	ASSERT_TRUE(::GetFileTime(timestampHandle.get(), &actualCreationTime, &actualLastAccessTime, &actualLastWriteTime));
	EXPECT_EQ(0, ::CompareFileTime(&creationTime, &actualCreationTime));
	EXPECT_EQ(0, ::CompareFileTime(&lastAccessTime, &actualLastAccessTime));
	EXPECT_EQ(0, ::CompareFileTime(&lastWriteTime, &actualLastWriteTime));
}

/*!
 * @brief check_write_permissions() のテスト
 *
 * check_write_permissions失敗時に例外が投げられることを確認する。
 */
TEST(FilePointer, check_write_permissions101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	// テスト用に作成するファイルのパス（INIパスを流用）
	const auto path = GetIniFileName();

	// 新規作成したいので削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// テストデータを書き込む
	std::ofstream fos{ path };
	fos << "This is test data." << std::endl;
	fos.close();

	// 存在しているファイルを読み取り専用で開く
	auto hFileHolder = FileHandle::OpenExisting(
		path,
		GENERIC_READ,
		0
	);
	EXPECT_TRUE(hFileHolder);

	// 読み取り専用で開く
	auto fp = FilePointer::OpenFileHandle(std::move(hFileHolder), L"rb");
	EXPECT_TRUE(fp);

	// OSのファイルハンドルから Cストリーム を開く
	EXPECT_THROW(
		fp.check_write_permissions(),
		std::system_error
	);

	std::filesystem::remove(path, ec);

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief check_write_permissions() のテスト
 *
 * check_write_permissions失敗時に例外が投げられることを確認する。
 */
TEST(FilePointer, check_write_permissions103)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	// テスト用に作成するファイルのパス（INIパスを流用）
	const auto path = GetIniFileName();

	// 新規作成したいので削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// テストデータを書き込む
	std::ofstream fos{ path };
	fos << "This is test data." << std::endl;
	fos.close();

	// 存在しているファイルを書き込み専用で開く
	auto hFileHolder = FileHandle::OpenExisting(
		path,
		GENERIC_WRITE,
		0
	);
	EXPECT_TRUE(hFileHolder);

	// 書き込み専用で開く
	auto fp = FilePointer::OpenFileHandle(std::move(hFileHolder), L"wb");
	EXPECT_TRUE(fp);

	// データを書き込んでバックアップを試行させる
	fp.write("This is test data.");
	fp.seek(0);

	Kernel32::setInstance<MockKernel32>();
	auto pKernel32 = (MockKernel32*)Kernel32::getInstance();

	EXPECT_CALL(*pKernel32, GetFileTime(_, _, _, _))
		.WillOnce(testing::Throw(std::system_error(ERROR_NOT_FOUND, std::system_category())));

	try {
		// タイムスタンプの取得失敗は続行する
		fp.check_write_permissions();

	} catch (const std::system_error&) {
		// 例外が投げられた場合は無視して続行	
	}

	Kernel32::resetInstance();

	std::filesystem::remove(path, ec);

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief read() のテスト
 *
 * read失敗時に例外が投げられることを確認する。
 */
TEST(FilePointer, read101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	FilePointer fp{};

	std::string buffer(80, '\0');

	EXPECT_THROW(
		fp.read(buffer),
		std::system_error
	);

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief seek() のテスト
 *
 * seek失敗時に例外が投げられることを確認する。
 */
TEST(FilePointer, seek101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	FilePointer fp{};

	EXPECT_THROW(
		fp.seek(0),
		std::system_error
	);

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief write() のテスト
 *
 * write失敗時に例外が投げられることを確認する。
 */
TEST(FilePointer, write101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	FilePointer fp{};

	EXPECT_THROW(
		fp.write("test"),
		std::system_error
	);

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

/*!
 * @brief FdCloseFunc のテスト（正常系）
 */
TEST(FdCloseFunc, operator001)
{
	// テスト用に作成するファイルのパス（INIパスを流用）
	const auto path = GetIniFileName();

	// 新規作成したいので削除しておく
	std::error_code ec;
	std::filesystem::remove(path, ec);

	// 作成するファイルの属性（一時ファイル、閉じたら削除）
	const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;

	// OSのファイルハンドルを作成する
	auto hFile = FileHandle::CreateNew(path, dwFlagsAndAttributes);
	EXPECT_TRUE(hFile);

	// ファイルが作られる
	EXPECT_TRUE(fexist(path));

	// OSのファイルハンドルからファイル記述子を開く
	auto fd = ::_open_osfhandle(
		intptr_t(hFile.get()),
		0
	);

	// ファイルハンドルはfdに譲渡するので、スマートポインターとの紐付けを解除しておく
	hFile.release();

	// ファイルを閉じる
	cxx::FdCloseFunc(&fd);

	// ファイルは削除されている
	EXPECT_FALSE(fexist(path));
}

/*!
 * @brief FdCloseFunc のテスト（異常系）
 */
TEST(FdCloseFunc, operator101)
{
#if defined(_MSC_VER) && defined(_DEBUG)

	MsvcInvalidParameterHandlerDisabler disabler{};	// 無効なパラメーターハンドラーを無効化

	MsvcReportMode reportMode{}; // アサーションダイアログを抑制

	int fd = -1;

	EXPECT_THAT(cxx::FdCloseFunc(&fd), Eq(-1));

#endif // defined(_MSC_VER) && defined(_DEBUG)
}

} // namespace io

namespace path_util {

using cxx::WritePrivateProfileStringW;
using cxx::ExpandEnvironmentStringsW;

/*!
 * @brief パスがファイル名に使えない文字を含んでいるかチェックする
 */
TEST( file, IsInvalidFilenameChars )
{
	// ファイル名に使えない文字 = "\\/:*?\"<>|"
	// このうち、\\と/はパス区切りのため実質対象外になる。
	EXPECT_FALSE(IsInvalidFilenameChars(L"test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L".\\test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"./test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:\\test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:/test.txt"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:\\"));
	EXPECT_FALSE(IsInvalidFilenameChars(L"C:/"));

	EXPECT_FALSE(IsInvalidFilenameChars(L"test:001.txt"));

	EXPECT_TRUE(IsInvalidFilenameChars(L"test*.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test?.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test\".txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test<.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test>.txt"));
	EXPECT_TRUE(IsInvalidFilenameChars(L"test|.txt"));
}

TEST(file, IsValidPathAvailableChar)
{
	EXPECT_TRUE(IsValidPathAvailableChar(L"test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L".\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"./test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:/test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:/"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\dir\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:\\dir\\dir2\\test.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"C:dir\\dir2\\test.txt"));

	EXPECT_TRUE(IsValidPathAvailableChar(L"test:001.txt"));
	EXPECT_TRUE(IsValidPathAvailableChar(L"\\dir\\dir2\\test:001.txt"));

	// 特別考慮：DOSデバイスパスの?はtrue
	EXPECT_TRUE(IsValidPathAvailableChar(L"\\\\?\\C:\\test.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"test*.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test?.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test\".txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test<.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test>.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"test|.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir\\test*.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir\\test?.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir*\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir?\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir\"\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir<\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir>\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\dir|\\text.txt"));

	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\*dir\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\?dir\\text.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"C:\\di*r\\text.txt"));


	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\test?.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\test*.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\d*ir\\test.txt"));
	EXPECT_FALSE(IsValidPathAvailableChar(L"\\\\?\\C:\\dir*\\test.txt"));
}

/*!
 * @brief パスの区切りの / を \ に置き換える(Issue #2727)
 */
TEST(file, ReplaceSlashToYen)
{
	EXPECT_THAT(ReplaceSlashToYen(L"C:/work/a.txt"), StrEq(LR"(C:\work\a.txt)"));
	EXPECT_THAT(ReplaceSlashToYen(LR"(C:\work/sub\a.txt)"), StrEq(LR"(C:\work\sub\a.txt)"));		// 混在
	EXPECT_THAT(ReplaceSlashToYen(L"//server/share/a.txt"), StrEq(LR"(\\server\share\a.txt)"));	// UNC
	EXPECT_THAT(ReplaceSlashToYen(L"sub/a.txt"), StrEq(LR"(sub\a.txt)"));						// 相対パス
	EXPECT_THAT(ReplaceSlashToYen(LR"(C:\work\a.txt)"), StrEq(LR"(C:\work\a.txt)"));			// 変わらない
	EXPECT_THAT(ReplaceSlashToYen(L""), StrEq(L""));

	// 複数選択の書き方(引用符で囲んだ名前の並び)も、区切りだけが変わる
	const std::wstring multi = LR"("C:/work/a.txt" "b.txt")";
	const std::wstring multiExpected = LR"("C:\work\a.txt" "b.txt")";
	EXPECT_THAT(ReplaceSlashToYen(multi), StrEq(multiExpected));

	// URL は変えない
	EXPECT_THAT(ReplaceSlashToYen(L"https://example.com/a.txt"), StrEq(L"https://example.com/a.txt"));
	EXPECT_THAT(ReplaceSlashToYen(L"file:///C:/work/a.txt"), StrEq(L"file:///C:/work/a.txt"));
}

/*!
 * @brief exeファイルパスの取得
 */
TEST(file, GetExeFileName)
{
	// 標準的なコードでexeファイルのパスを取得
	std::wstring expected(_MAX_PATH, L'\0');
	::GetModuleFileNameW(nullptr, std::data(expected), DWORD(std::size(expected)));
	expected.resize(::wcsnlen(std::data(expected), std::size(expected)));

	// 関数戻り値が、標準的なコードで取得した結果と一致すること
	auto exePath = GetExeFileName();
	EXPECT_THAT(exePath, StrEq(expected));
}

/*!
 * @brief 既存コード互換用に残しておく関数のリグレッション
 */
TEST(file, Deprecated_GetExedir)
{
	// テストに使うファイル名(空でなければなんでもいい)
	constexpr const auto filename = L"README.txt";

	// 比較用関数呼び出し
	auto exeBasePath = GetExeFileName().parent_path().append(filename);

	// 戻り値取得用のバッファを指定しない場合、何も起きない
	GetExedir(nullptr);

	// 戻り値取得用のバッファ
	std::wstring buffer(_MAX_PATH, L'\0');

	// exeフォルダーの取得
	GetExedir(std::data(buffer));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	buffer += filename;
	EXPECT_THAT(buffer, StrEq(exeBasePath.c_str()));

	// バッファを再確保する
	buffer = std::wstring(_MAX_PATH, L'\0');

	// exe基準ファイルパスの取得
	GetExedir(std::data(buffer), filename);
	EXPECT_THAT(buffer, StartsWith(exeBasePath.c_str()));
}

/*!
 * @brief iniファイルパスの取得(プロセス未作成時)
 */
TEST(file, GetIniFileName_OutOfProcess)
{
	// exeファイルの拡張子をiniに変えたパスが返る
	auto iniPath = GetExeFileName().replace_extension(L".ini");
	EXPECT_THAT(GetIniFileName(), StrEq(iniPath.c_str()));
}

/*!
 * @brief iniファイルパスの取得(コントロールプロセス未初期化、かつ、プロファイル指定なし時)
 */
TEST(file, GetIniFileName_InProcessDefaultProfileUnInitialized)
{
	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="")");

	// exeファイルの拡張子をiniに変えたパスが返る
	auto path = GetExeFileName().replace_extension(L".ini");
	EXPECT_THAT(GetIniFileName(), StrEq(path.c_str()));
}

/*!
 * @brief iniファイルパスの取得(コントロールプロセス未初期化、かつ、プロファイル指定あり時)
 */
TEST(file, GetIniFileName_InProcessNamedProfileUnInitialized)
{
	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="profile1")");

	// exeファイルの拡張子をiniに変えたパスの最後のフォルダーにプロファイル名を加えたパスが返る
	auto iniPath = GetExeFileName().replace_extension(L".ini");
	auto path = iniPath.parent_path().append(L"profile1").append(iniPath.filename().c_str());
	EXPECT_THAT(GetIniFileName(), StrEq(path.c_str()));
}

/*!
 * マルチユーザー設定ファイルを使うテストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class CExeIniTest : public ::testing::Test {
protected:
	/*!
	 * マルチユーザー構成設定ファイルのパス
	 */
	std::filesystem::path exeIniPath;

	/*
	 * profile1 配下のフォルダーのパス
	 */
	std::filesystem::path profileDirPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// マルチユーザー構成設定ファイルのパス
		exeIniPath = GetExeFileName().concat(L".ini");
		profileDirPath.clear();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		std::error_code ec;

		// 存在チェック
		if (std::filesystem::exists(exeIniPath)) {
			// マルチユーザー構成設定ファイルを削除する
			std::filesystem::remove(exeIniPath, ec);
		}

		// profile1 配下のフォルダーも削除する
		if (!profileDirPath.empty()) {
			std::filesystem::remove_all(profileDirPath, ec);
		}

		// 削除チェック
		EXPECT_FALSE(fexist(exeIniPath));
		if (!profileDirPath.empty()) {
			EXPECT_FALSE(std::filesystem::exists(profileDirPath));
		}
	}
};

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateRoamingAppData)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"0", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="profile1")");

	const auto iniPath = GetIniFileName();
	profileDirPath = iniPath.parent_path();

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\AppData\Roaming\sakura\profile1\)");
	expected += iniPath.filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateDesktop)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"3", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\Desktop\sakura\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateProfile)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\sakura\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST_F(CExeIniTest, GetIniFileName_PrivateDocument)
{
	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"MultiUser", L"1", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"2", exeIniPath);
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", exeIniPath);

	// 実在チェック
	EXPECT_TRUE(fexist(exeIniPath));

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="")");

	// 期待値を取得する
	auto expected = ExpandEnvironmentStringsW(LR"(%USERPROFILE%\Documents\sakura\)");
	expected += GetIniFileName().filename();

	// テスト実施
	EXPECT_THAT(GetIniFileName(), StrEq(expected));
}

/*!
 * @brief 既存コード互換用に残しておく関数のリグレッション
 */
TEST(file, Deprecated_GetInidir)
{
	// テストに使うファイル名(空でなければなんでもいい)
	constexpr const auto filename = L"README.txt";

	// 比較用関数呼び出し
	auto iniBasePath = GetIniFileName().parent_path().append(filename);

	// 戻り値取得用のバッファを指定しない場合、何も起きない
	GetInidir(nullptr);

	// 戻り値取得用のバッファ
	std::wstring buffer(_MAX_PATH, L'\0');

	// iniフォルダーの取得
	GetInidir(std::data(buffer));
	buffer.resize(::wcsnlen(std::data(buffer), std::size(buffer)));
	buffer += filename;
	EXPECT_THAT(buffer, StrEq(iniBasePath.c_str()));

	// バッファを再確保する
	buffer = std::wstring(_MAX_PATH, L'\0');

	// ini基準ファイルパスの取得
	GetInidir(std::data(buffer), filename);
	EXPECT_THAT(buffer, StartsWith(iniBasePath.c_str()));
}

/*!
 * @brief INIファイルまたはEXEファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す（INIを優先）
 */
TEST(file, GetInidirOrExedir)
{
	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="profile1")");

	std::wstring buf(_MAX_PATH, L'\0');

	GetInidirOrExedir(buf.data(), L"", true);
	EXPECT_THAT(buf, StartsWith(GetExeFileName().replace_filename(L"").c_str()));

	constexpr auto filename = L"test.txt";
	auto exeBasePath = GetExeFileName().parent_path().append(filename);
	auto iniBasePath = GetIniFileName().parent_path().append(filename);
	ASSERT_NE(exeBasePath.parent_path(), iniBasePath.parent_path());

	// EXE基準のファイルを作る
	CProfile().WriteProfile(exeBasePath.c_str(), L"file, GetInidirOrExedirのテスト");

	// INI基準のファイルを作る
	CProfile().WriteProfile(iniBasePath.c_str(), L"file, GetInidirOrExedirのテスト");

	// 両方あるときはINI基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	EXPECT_THAT(buf, StartsWith(iniBasePath.c_str()));

	std::error_code ec;

	// INI基準パスのファイルを削除する
	std::filesystem::remove(iniBasePath, ec);
	EXPECT_FALSE(fexist(iniBasePath));

	// EXE基準のみ存在するときはEXE基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	EXPECT_THAT(buf, StartsWith(exeBasePath.c_str()));

	// EXE基準パスのファイルを削除する
	std::filesystem::remove(exeBasePath, ec);
	EXPECT_FALSE(fexist(exeBasePath));

	// 両方ないときはINI基準のパスが変える
	GetInidirOrExedir(buf.data(), filename, true);
	EXPECT_THAT(buf, StartsWith(iniBasePath.c_str()));

	std::filesystem::remove(iniBasePath, ec);
	std::filesystem::remove(exeBasePath, ec);
	std::filesystem::remove(iniBasePath.parent_path(), ec);
}

/*!
 * @brief 入出力に使うiniファイルの判定
 */
TEST(file, GetIniFileNameForIO)
{
	auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 書き込みモードのとき
	EXPECT_THAT(GetIniFileNameForIO(true), StrEq(iniPath.c_str()));

	// 書き込みモードでないとき
	EXPECT_THAT(GetIniFileNameForIO(false), StrEq(iniPath.c_str()));

	// 書き込みモードでないがiniファイルが実在するとき
	CProfile().WriteProfile(iniPath.c_str(), L"file, GetIniFileNameForIOのテスト");
	EXPECT_TRUE(fexist(iniPath));

	// テスト実施
	EXPECT_THAT(GetIniFileNameForIO(false), StrEq(iniPath.c_str()));

	// INIファイルを削除する
	std::error_code ec;
	std::filesystem::remove(iniPath, ec);
	EXPECT_FALSE(fexist(iniPath));
}

/*!
 * @brief フルパスからファイル名を取り出す
 */
TEST(file, GetFileTitlePointer)
{
	// フルパスからファイル名を取得する
	EXPECT_STREQ(L"test.txt", GetFileTitlePointer(LR"(C:\Temp\test.txt)"));

	// フルパスにファイル名が含まれていない場合
	EXPECT_STREQ(L"", GetFileTitlePointer(LR"(C:\Temp\)"));

	// フルパスに\\が含まれていない場合
	EXPECT_STREQ(L"test.txt", GetFileTitlePointer(L"test.txt"));

	// 渡したパスが無効な場合は落ちます。
	EXPECT_DEATH({ GetFileTitlePointer(nullptr); }, ".*");
}

/*!
 * @brief ディレクトリの深さを計算する
 */
TEST(file, CalcDirectoryDepth)
{
	// ドライブ文字を含むフルパス
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(C:\Temp\test.txt)"));

	// 共有フォルダーを含むフルパス
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(\\host\Temp\test.txt)"));

	// ドライブなしのフルパス
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(\Temp\test.txt)"));

	// 相対パス（？）
	EXPECT_EQ(1, CalcDirectoryDepth(LR"(C:\Temp\.\test.txt)"));

	// 渡したパスが無効な場合は落ちます。
	EXPECT_DEATH({ CalcDirectoryDepth(nullptr); }, ".*");
}

/*!
	FileMatchScoreSepExtのテスト
 */
TEST(file, FileMatchScoreSepExt)
{
	int result = 0;

	// FileNameSepExtのテストパターン
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// FileNameSepExtのテストパターン（パスにフォルダーが含まれない）
	result = FileMatchScoreSepExt(
		LR"(TEST.TXT)",
		LR"(test.txt)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// FileNameSepExtのテストパターン（ファイル名がない）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\.txt)",
		LR"(C:\TEMP\.txt)");
	EXPECT_THAT(result, std::size(LR"(.txt)") - 1);

	// FileNameSepExtのテストパターン（拡張子がない）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test)",
		LR"(C:\TEMP\test)");
	EXPECT_THAT(result, std::size(LR"(test)") - 1);

	// 全く同じパス同士の比較（ファイル名＋拡張子が完全一致）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// 異なるパスでファイル名＋拡張子が同じ（ファイル名＋拡張子が完全一致）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP1\TEST.TXT)",
		LR"(C:\TEMP2\test.txt)");
	EXPECT_THAT(result, std::size(LR"(test.txt)") - 1);

	// ファイル名が異なる1（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEST1.TST)");
	EXPECT_THAT(result, std::size(LR"(test)") - 1 + std::size(LR"(.t)") - 1);

	// ファイル名が異なる2（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test1.tst)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(test)") - 1 + std::size(LR"(.t)") - 1);

	// 拡張子が異なる1（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\test.txt)",
		LR"(C:\TEMP\TEXT.TXTX)");
	EXPECT_THAT(result, std::size(LR"(te)") - 1 + std::size(LR"(.txt)") - 1);

	// 拡張子が異なる2（最長一致を取得）
	result = FileMatchScoreSepExt(
		LR"(C:\TEMP\text.txtx)",
		LR"(C:\TEMP\TEST.TXT)");
	EXPECT_THAT(result, std::size(LR"(te)") - 1 + std::size(LR"(.txt)") - 1);

	// サロゲート文字を含む1
	result = FileMatchScoreSepExt(
		L"C:\\TEMP\\test\xD83D\xDC49\xD83D\xDC46.TST",
		L"C:\\TEMP\\TEST\xD83D\xDC49\xD83D\xDC47.txt");
	EXPECT_THAT(result, std::size(LR"(testXX)") - 1 + std::size(LR"(.t)") - 1);

	// サロゲート文字を含む2
	result = FileMatchScoreSepExt(
		L"C:\\TEMP\\TEST\xD83D\xDC49\xD83D\xDC47.txt",
		L"C:\\TEMP\\test\xD83D\xDC49\xD83D\xDC46.TST");
	EXPECT_THAT(result, std::size(LR"(testXX)") - 1 + std::size(LR"(.t)") - 1);
}

/*!
	GetExtのテスト
 */
TEST(CFilePath, GetExt)
{
	CFilePath path;

	// 最も単純なパターン
	path = L"test.txt";
	EXPECT_THAT(path.GetExt(true), StrEq(L"txt"));

	// ファイルに拡張子がないパターン
	path = L"lib\\.NET Core\\README";
	EXPECT_THAT(path.GetExt(true), StrEq(L""));

	// 拡張子がない場合に返却されるポインタ値の確認
	EXPECT_EQ(path.GetExt(), path.c_str() + path.Length());

	// ファイルに拡張子がないパターン
	path = L"lib/.NET Core/README";
	EXPECT_THAT(path.GetExt(true), StrEq(L""));

	// 拡張子がない場合に返却されるポインタ値の確認
	EXPECT_EQ(path.GetExt(), path.c_str() + path.Length());
}

/*!
	CFileNameManager::GetFilePathFormatのテスト
 */
TEST(CFileNameManager, GetFilePathFormat)
{
	// バッファ
	std::wstring strBuf;

	// 十分な大きさのバッファを指定
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.txt)"));

	// バッファ不足（パターンに一致した部分が切り捨てられる）
	strBuf = std::wstring(6, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポ)"));

	// バッファ不足（パターンに一致しない部分が切り捨てられる）
	strBuf = std::wstring(15, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.t)"));

	// ソースが部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(std::wstring_view(LR"(C:\%Temp%\test.txt.bak)", 18), strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.txt)"));

	// ソースが部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(std::wstring_view(LR"(C:\test.txt\%Temp%.bak)", 18), strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\test.txt\テンポラリ)"));

	// ソースが部分文字列（置換文字が1文字アウト、十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(std::wstring_view(LR"(C:\test.txt\%Temp%.bak)", 17), strBuf.data(), strBuf.size() + 1, L"%Temp%", L"テンポラリ"), StrEq(LR"(C:\test.txt\%Temp)"));

	// 置換対象が部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, std::wstring_view(LR"(%Temp%\)", 6), L"テンポラリ"), StrEq(LR"(C:\テンポラリ\test.txt)"));

	// 置換先が部分文字列（十分な大きさのバッファを指定）
	strBuf = std::wstring(50, L'x');
	EXPECT_THAT(CFileNameManager::GetFilePathFormat(LR"(C:\%Temp%\test.txt)", strBuf.data(), strBuf.size() + 1, L"%Temp%", std::wstring_view(L"テンポラリってる", 5)), StrEq(LR"(C:\テンポラリ\test.txt)"));
}

TEST(CFilePath, GetDirPath001)
{
	CFilePath path(LR"(C:\Temp\test.txt)");
	EXPECT_THAT(path.GetDirPath(), StrEq(LR"(C:\Temp\)"));
}

TEST(CFilePath, GetDirPath002)
{
	CFilePath path(LR"(C:\Temp\)");	//ファイル名がない
	EXPECT_THAT(path.GetDirPath(), StrEq(LR"(C:\Temp\)"));
}

TEST(CFilePath, GetDirPath003)
{
	CFilePath path(LR"(C:\Temp)");	//末尾 \ がない
	EXPECT_THAT(path.GetDirPath(), StrEq(LR"(C:\)"));
}

TEST(CFilePath, GetDirPath101)
{
	CFilePath path(L"");	//パスが空
	EXPECT_THAT(path.GetDirPath(), StrEq(L""));
}

TEST(CFilePath, GetDirPath102)
{
	CFilePath path(L"test.txt");	//ディレクトリがない
	EXPECT_THAT(path.GetDirPath(), StrEq(L""));
}

} // namespace path_util
