/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "dlg/CDlgProfileMgr.h"

#include "_main/CProcessFactory.h"

#include <tchar.h>
#include <Windows.h>


#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "env/CDataProfile.h"
#include "util/file.h"

#include "window/TrayWndTestSuite.hpp"

/*!
 * プロファイルマネージャ設定ファイルを使うテストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct CDlgProfileMgrTest : public ::testing::Test, public window::TrayWndTestSuite {
	/*!
	 * プロファイルマネージャ設定ファイルのパス
	 */
	static inline std::filesystem::path profileMgrIniPath{};

	static void SetUpTestSuite()
	{
		SetUpTrayWnd();

		// プロファイルマネージャ設定ファイルのパスを生成
		profileMgrIniPath = GetProfileMgrFileName();

		// プロファイルマネージャー設定を削除する
		std::error_code ec;
		std::filesystem::remove(profileMgrIniPath, ec);
	}

	static void TearDownTestSuite()
	{
		// プロファイルマネージャー設定を削除する
		std::error_code ec;
		std::filesystem::remove(profileMgrIniPath, ec);

		TearDownTrayWnd();
	}

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		if( fexist( profileMgrIniPath.c_str() ) ){
			// プロファイルマネージャー設定を削除する
			std::filesystem::remove( profileMgrIniPath );
		}

		// ケースごとに異なるコマンドラインを使うのでクリアしておく
		pCommandLine = std::make_unique<CCommandLine>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// プロファイルマネージャー設定を削除する
		std::filesystem::remove( profileMgrIniPath );
	}
};

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_001 )
{
	// プロファイルマネージャ表示オプションが付いてたらプロファイルは確定しない
	pCommandLine->ParseCommandLine(L"-PROFMGR", false);
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile(pCommandLine.get()) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_002 )
{
	// プロファイル名が指定されていたらプロファイルは確定する
	pCommandLine->ParseCommandLine(L"-PROF=執筆用", false);
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile(pCommandLine.get()) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_003 )
{
	// プロファイルマネージャー設定がなかったらプロファイルは確定する
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile(pCommandLine.get()) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_004 )
{
	// プロファイル設定を作る
	CDataProfile cProfile;
	cProfile.SetWritingMode();
	cProfile.SetProfileData(L"Profile", L"szDllLanguage", L"");
	cProfile.SetProfileData(L"Profile", L"nDefaultIndex", L"3");
	cProfile.SetProfileData(L"Profile", L"nCount", L"3");
	cProfile.SetProfileData(L"Profile", L"P[1]", L"保存用");
	cProfile.SetProfileData(L"Profile", L"P[2]", L"鑑賞用");
	cProfile.SetProfileData(L"Profile", L"P[3]", L"使用用");
	cProfile.SetProfileData(L"Profile", L"bDefaultSelect", L"1");
	cProfile.WriteProfile(profileMgrIniPath.c_str(), L"Sakura Profile ini");

	// プロファイルマネージャー設定にデフォルト定義があればプロファイルは確定する
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile(pCommandLine.get()) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_005 )
{
	// プロファイル設定を作る
	CDataProfile cProfile;
	cProfile.SetWritingMode();
	cProfile.SetProfileData(L"Profile", L"szDllLanguage", L"");
	cProfile.SetProfileData(L"Profile", L"nDefaultIndex", L"4");
	cProfile.SetProfileData(L"Profile", L"nCount", L"3");
	cProfile.SetProfileData(L"Profile", L"P[1]", L"保存用");
	cProfile.SetProfileData(L"Profile", L"P[2]", L"鑑賞用");
	cProfile.SetProfileData(L"Profile", L"P[3]", L"使用用");
	cProfile.SetProfileData(L"Profile", L"bDefaultSelect", L"1");
	cProfile.WriteProfile(profileMgrIniPath.c_str(), L"Sakura Profile ini");

	// プロファイルマネージャー設定のデフォルト定義がおかしればプロファイルは確定しない
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile(pCommandLine.get()) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_006 )
{
	// 空のプロファイル設定を作る
	CDataProfile cProfile;
	cProfile.SetWritingMode();
	cProfile.SetProfileData(L"Profile", L"szDllLanguage", L"");
	cProfile.SetProfileData(L"Profile", L"nDefaultIndex", L"-1");
	cProfile.SetProfileData(L"Profile", L"bDefaultSelect", L"0");
	cProfile.WriteProfile(profileMgrIniPath.c_str(), L"Sakura Profile ini");

	// プロファイルマネージャー設定が空定義ならプロファイルは確定しない
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile(pCommandLine.get()) );
}

/*!
 * @brief プロファイルマネージャ設定ファイルパスの取得
 */
TEST(file, GetProfileMgrFileName_NoArg1)
{
	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="")");

	// iniファイルの拡張子を_prof.iniに変えたパスが返る
	const auto profileMgrIniPath = GetIniFileName().replace_extension().concat(L"_prof.ini").native();
	ASSERT_THAT(GetProfileMgrFileName(), StrEq(profileMgrIniPath));
}

/*!
 * @brief プロファイルマネージャ設定ファイルパスの取得
 */
TEST(file, GetProfileMgrFileName_NoArg2)
{
	// プロセスのインスタンスを用意する
	constexpr auto profileName = L"profile1";
	const auto dummy = CProcessFactory().CreateInstance(std::format(LR"(-NOWIN -PROF="{}")", profileName));

	// 実行ファイルの拡張子を_prof.iniに変えたパスが返る
	const auto profileMgrIniPath = GetExeFileName().replace_extension().concat(L"_prof.ini").native();
	ASSERT_THAT(GetProfileMgrFileName(), StrEq(profileMgrIniPath));
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空の時)
 */
TEST(file, GetProfileMgrFileName_DefaultProfile)
{
	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-NOWIN -PROF="")");

	// 設定フォルダーのパスが返る
	const auto iniDir = GetIniFileName().remove_filename().native();
	ASSERT_THAT(GetProfileMgrFileName(L""), StrEq(iniDir));
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空でない時)
 */
TEST(file, GetProfileMgrFileName_NamedProfile)
{
	// プロセスのインスタンスを用意する
	constexpr auto profileName = L"profile1";
	const auto dummy = CProcessFactory().CreateInstance(std::format(LR"(-NOWIN -PROF="{}")", profileName));

	// 指定したプロファイルの設定保存先フォルダーのパスが返る
	const auto profileDir = GetExeFileName().replace_filename(profileName).append("").native();
	ASSERT_THAT(GetProfileMgrFileName(profileName), StrEq(profileDir));
}
