/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#include "pch.h"

#include "dlg/CDlgProfileMgr.h"

#include "_main/CProcessFactory.h"

#include "CDataProfile.h"
#include "util/file.h"

/*!
 * プロファイルマネージャ設定ファイルを使うテストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class CDlgProfileMgrTest : public ::testing::Test {
protected:
	/*!
	 * プロファイルマネージャ設定ファイルのパス
	 */
	std::filesystem::path profileMgrIniPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// コマンドラインのインスタンスを用意する
		CCommandLine cCommandLine;

		// プロファイルマネージャ設定ファイルのパスを生成
		profileMgrIniPath = GetProfileMgrFileName();
		if( fexist( profileMgrIniPath.c_str() ) ){
			// プロファイルマネージャー設定を削除する
			std::filesystem::remove( profileMgrIniPath );
		}
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
	std::wstring strProfileName;
	ASSERT_FALSE(CDlgProfileMgr::TrySelectProfile(strProfileName, false, true));
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_002 )
{
	// プロファイル名が指定されていたらプロファイルは確定する
	std::wstring strProfileName = L"執筆用";
	ASSERT_TRUE(CDlgProfileMgr::TrySelectProfile(strProfileName, true));
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_003 )
{
	// プロファイルマネージャー設定がなかったらプロファイルは確定する
	std::wstring strProfileName;
	ASSERT_TRUE(CDlgProfileMgr::TrySelectProfile(strProfileName));
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
	std::wstring strProfileName;
	ASSERT_TRUE(CDlgProfileMgr::TrySelectProfile(strProfileName));
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
	std::wstring strProfileName;
	ASSERT_FALSE(CDlgProfileMgr::TrySelectProfile(strProfileName));
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
	std::wstring strProfileName;
	ASSERT_FALSE(CDlgProfileMgr::TrySelectProfile(strProfileName));
}

/*!
 * @brief プロファイルマネージャ設定ファイルパスの取得
 */
TEST(file, GetProfileMgrFileName_NoArg1)
{
	// iniファイルの拡張子を_prof.iniに変えたパスが返る
	const auto profileMgrIniPath = GetIniFileName().replace_extension().concat(L"_prof.ini");
	ASSERT_STREQ(profileMgrIniPath.c_str(), GetProfileMgrFileName().c_str());
}

/*!
 * @brief プロファイルマネージャ設定ファイルパスの取得
 */
TEST(file, GetProfileMgrFileName_NoArg2)
{
	ASSERT_FALSE(CProcess::getInstance());

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(LR"(-PROF="profile1")");

	// iniファイルの拡張子を_prof.iniに変えたパスが返る
	const auto profileMgrIniPath = GetIniFileName().parent_path().parent_path().append(GetIniFileName().replace_extension().concat(L"_prof.ini").filename().c_str());
	ASSERT_STREQ(profileMgrIniPath.c_str(), GetProfileMgrFileName().c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空の時)
 */
TEST(file, GetProfileMgrFileName_DefaultProfile1)
{
	// 設定フォルダーのパスが返る
	const auto iniDir = GetExeFileName().replace_filename(L"").append("a.txt").remove_filename();
	ASSERT_STREQ(iniDir.c_str(), GetProfileMgrFileName(L"").c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空の時)
 */
TEST(file, GetProfileMgrFileName_DefaultProfile2)
{
	// テスト用プロファイル名
	constexpr auto& profileName = L"profile1";

	ASSERT_FALSE(CProcess::getInstance());

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(fmt::format(LR"(-PROF="{}")", profileName));

	// 設定フォルダーのパスが返る
	const auto iniDir = GetIniFileName().parent_path().parent_path().append("a.txt").remove_filename();
	ASSERT_STREQ(iniDir.c_str(), GetProfileMgrFileName(L"").c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空でない時)
 */
TEST(file, GetProfileMgrFileName_NamedProfile1)
{
	// テスト用プロファイル名
	constexpr auto& profileName = L"profile1";

	ASSERT_FALSE(CProcess::getInstance());

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(fmt::format(LR"(-PROF="{}")", profileName));

	// 指定したプロファイルの設定保存先フォルダーのパスが返る
	const auto profileDir = GetExeFileName().replace_filename(profileName).append("a.txt").remove_filename();
	ASSERT_STREQ(profileDir.c_str(), GetProfileMgrFileName(profileName).c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空でない時)
 */
TEST(file, GetProfileMgrFileName_NamedProfile2)
{
	// テスト用プロファイル名
	constexpr auto& profileName = L"profile1";

	ASSERT_FALSE(CProcess::getInstance());

	// プロセスのインスタンスを用意する
	const auto dummy = CProcessFactory().CreateInstance(fmt::format(LR"(-PROF="{}")", profileName));

	// 指定したプロファイルの設定保存先フォルダーのパスが返る
	const auto profileDir = GetIniFileName().parent_path().parent_path().append(profileName).append("a.txt").remove_filename();
	ASSERT_STREQ(profileDir.c_str(), GetProfileMgrFileName(profileName).c_str());
}
