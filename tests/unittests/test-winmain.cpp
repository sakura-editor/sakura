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
#include "_main/CProcessFactory.h"

#include "util/file.h"
#include "config/system_constants.h"

#include "StartEditorProcessForTest.h"

TEST(WinMain, OleInitialize)
{
	//先にOleInitializeを呼び出して失敗させる
	EXPECT_EQ(S_OK, OleInitialize(nullptr));

	EXPECT_EQ(1, wWinMain(GetModuleHandleW(nullptr), nullptr, L"", SW_SHOWDEFAULT));

	OleUninitialize();
}

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct WinMainTest : public ::testing::TestWithParam<const wchar_t*> {

	/*!
	 * プロセスのインスタンス
	 */
	std::unique_ptr<CProcess> process;

	/*!
	 * 設定ファイルのパス
	 *
	 * GetIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	std::filesystem::path iniPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テスト用プロファイル名
		const auto profileName(GetParam());

		ASSERT_FALSE(CProcess::getInstance());

		// プロセスのインスタンスを用意する
		process = CProcessFactory().CreateInstance(fmt::format(LR"(-PROF="{}")", profileName));

		// 起動中プロセスがあれば終了する
		if (process->IsExistControlProcess(profileName)) {
			process->TerminateControlProcess(profileName);
		}

		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		// INIファイルを削除する
		if (fexist(iniPath.c_str())) {
			std::filesystem::remove(iniPath);
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		if (fexist(iniPath.c_str())) {
			std::filesystem::remove(iniPath);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (const std::wstring_view profileName(GetParam()); profileName.length() > 0) {
			std::filesystem::remove(iniPath.parent_path());
		}
	}

	/*!
	 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
	 */
	void WinMainTest::CControlProcess_StartAndTerminate(LPCWSTR profileName) const
	{
		// コントロールプロセスを起動する
		process->StartControlProcess(profileName);

		EXPECT_TRUE(process->IsExistControlProcess(profileName));

		// コントロールプロセスに終了指示を出して終了を待つ
		process->TerminateControlProcess(profileName);

		// コントロールプロセスが終了すると、INIファイルが作成される
		ASSERT_TRUE(fexist(iniPath.c_str()));
	}
};

/*!
 * @brief wWinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセスを実行する。
 *  プロセス起動は2回行い、1回目でINI作成＆書き込み、2回目でINI読み取りを検証する。
 */
TEST_P(WinMainTest, runWithNoWin)
{
	// テスト用プロファイル名
	const auto szProfileName(GetParam());

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(szProfileName);

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(szProfileName);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_P(WinMainTest, runEditorProcess)
{
	// テスト用プロファイル名
	const auto szProfileName(GetParam());

	// 起動時実行マクロの中身
	constexpr std::array macroCommands = {
		L"Down();"sv,
		L"Up();"sv,
		L"Right();"sv,
		L"Left();"sv,
		L"Outline(0);"sv,				//アウトライン解析
		L"ShowFunckey();"sv,			//ShowFunckey 出す
		L"ShowMiniMap();"sv,			//ShowMiniMap 出す
		L"ShowTab();"sv,				//ShowTab 出す
		L"SelectAll();"sv,
		L"GoFileEnd();"sv,
		L"GoFileTop();"sv,
		L"ShowFunckey();"sv,			//ShowFunckey 消す
		L"ShowMiniMap();"sv,			//ShowMiniMap 消す
		L"ShowTab();"sv,				//ShowTab 消す
		L"ExpandParameter('$I');"sv,	// INIファイルパスの取得(呼ぶだけ)

		// フォントサイズ設定のテスト(ここから)
		L"SetFontSize(0, 1, 0);"sv,		// 相対指定 - 拡大 - 対象：共通設定
		L"SetFontSize(0, -1, 0);"sv,	// 相対指定 - 縮小 - 対象：共通設定
		L"SetFontSize(100, 0, 0);"sv,	// 直接指定 - 対象：共通設定
		L"SetFontSize(100, 0, 1);"sv,	// 直接指定 - 対象：タイプ別設定
		L"SetFontSize(100, 0, 2);"sv,	// 直接指定 - 対象：一時適用
		L"SetFontSize(100, 0, 3);"sv,	// 直接指定 - 対象が不正
		L"SetFontSize(0, 0, 0);"sv,		// 直接指定 - フォントサイズ下限未満
		L"SetFontSize(9999, 0, 0);"sv,	// 直接指定 - フォントサイズ上限超過
		L"SetFontSize(0, 0, 2);"sv,		// 相対指定 - サイズ変化なし
		L"SetFontSize(0, 1, 2);"sv,		// 相対指定 - 拡大
		L"SetFontSize(0, -1, 2);"sv,	// 相対指定 - 縮小
		L"SetFontSize(0, 9999, 2);"sv,	// 相対指定 - 限界まで拡大
		L"SetFontSize(0, 1, 2);"sv,		// 相対指定 - これ以上拡大できない
		L"SetFontSize(0, -9999, 2);"sv,	// 相対指定 - 限界まで縮小
		L"SetFontSize(0, -1, 2);"sv,	// 相対指定 - これ以上縮小できない
		L"SetFontSize(100, 0, 2);"sv,	// 元に戻す
		// フォントサイズ設定のテスト(ここまで)

		L"Outline(2);"sv,	//アウトライン解析を閉じる

		L"ExitAll();"sv		//NOTE: このコマンドにより、エディタプロセスは起動された直後に終了する。
	};

	// 起動時実行マクロを組み立てる
	const auto strStartupMacro = std::accumulate(macroCommands.begin(), macroCommands.end(), std::wstring(),
		[](const auto& a, std::wstring_view b) {
			return a + b.data();
		});

	constexpr auto& quote= LR"(")";
	constexpr auto& doubled_quote = LR"("")";

	const std::wregex quote_regex(quote);

	// コマンドラインを組み立てる
	std::wstring strCommandLine(_T(__FILE__));
	strCommandLine += fmt::format(LR"( -PROF="{}")", szProfileName);
	strCommandLine += fmt::format(LR"( -MTYPE=js -M="{}")", std::regex_replace(strStartupMacro, quote_regex, doubled_quote));

	// プロセスのインスタンスを一回消す
	process.reset();

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	ASSERT_EXIT({ exit(StartEditorProcessForTest(strCommandLine)); }, ::testing::ExitedWithCode(0), ".*" );

	// プロセスのインスタンスを再生成する
	process = CProcessFactory().CreateInstance(fmt::format(LR"(-PROF="{}")", szProfileName));

	// コントロールプロセスに終了指示を出して終了を待つ
	process->TerminateControlProcess(szProfileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	ASSERT_TRUE(fexist(iniPath.c_str()));
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  プロファイル指定なしとプロファイル指定ありの2パターンで実体化させる
 */
INSTANTIATE_TEST_CASE_P(ParameterizedTestWinMain
	, WinMainTest
	, ::testing::Values(L"", L"profile1")
);
