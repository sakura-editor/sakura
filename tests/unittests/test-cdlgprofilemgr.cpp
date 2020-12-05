/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "dlg/CDlgProfileMgr.h"

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
#include "util/file.h"

/*!
 * テスト用の極薄ラッパークラス
 */
class CCommandLineWrapper : public CCommandLine
{
public:
	CCommandLineWrapper() = default;
};

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
	WCHAR szProfileMgrIniName[_MAX_PATH];

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// INIファイルのパスを取得
		WCHAR szPrivateIniFile[_MAX_PATH];
		WCHAR szIniFile[_MAX_PATH];
		CFileNameManager::GetIniFileNameDirect( szPrivateIniFile, szIniFile, L"" );

		// プロファイルマネージャ設定ファイルのパスを生成
		std::wregex re( L"\\.ini$", std::wregex::icase );
		const auto strProfileMgrIniName = std::regex_replace( szIniFile, re, L"_prof$&" );
		::wcscpy_s( szProfileMgrIniName, strProfileMgrIniName.data() );

		if( fexist( szProfileMgrIniName ) ){
			// プロファイルマネージャー設定を削除する
			std::filesystem::remove( szProfileMgrIniName );
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// プロファイルマネージャー設定を削除する
		std::filesystem::remove( szProfileMgrIniName );
	}
};

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_001 )
{
	// プロファイルマネージャ表示オプションが付いてたらプロファイルは確定しない
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine( L"-PROFMGR", false );
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_002 )
{
	// プロファイル名が指定されていたらプロファイルは確定する
	CCommandLineWrapper cCommandLine;
	cCommandLine.ParseCommandLine( L"-PROF=執筆用", false );
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_003 )
{
	// プロファイルマネージャー設定がなかったらプロファイルは確定する
	CCommandLineWrapper cCommandLine;
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_004 )
{
	// プロファイル設定を作る
	SProfileSettings settings;
	settings.m_szDllLanguage[0] = L'\0';
	settings.m_nDefaultIndex = 3;
	settings.m_vProfList = { L"保存用", L"鑑賞用", L"使用用" };
	settings.m_bDefaultSelect = true;
	CDlgProfileMgr::WriteProfSettings( settings );

	// プロファイルマネージャー設定にデフォルト定義があればプロファイルは確定する
	CCommandLineWrapper cCommandLine;
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_005 )
{
	// プロファイル設定を作る
	SProfileSettings settings;
	settings.m_szDllLanguage[0] = L'\0';
	settings.m_nDefaultIndex = 4;
	settings.m_vProfList = { L"保存用", L"鑑賞用", L"使用用" };
	settings.m_bDefaultSelect = true;
	CDlgProfileMgr::WriteProfSettings( settings );

	// プロファイルマネージャー設定のデフォルト定義がおかしればプロファイルは確定しない
	CCommandLineWrapper cCommandLine;
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_006 )
{
	// 空のプロファイル設定を作る
	SProfileSettings settings;
	settings.m_szDllLanguage[0] = L'\0';
	settings.m_nDefaultIndex = -1;
	settings.m_bDefaultSelect = false;
	CDlgProfileMgr::WriteProfSettings( settings );

	// プロファイルマネージャー設定が空定義ならプロファイルは確定しない
	CCommandLineWrapper cCommandLine;
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}
