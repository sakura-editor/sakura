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
#include "testing/StartEditorProcess.hpp"

#include "testing/GuiAwareTestSuite.hpp"

#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "config/system_constants.h"
#include "version.h"

#define ID_HOTKEY_TRAYMENU	0x1234

#if defined(_MSC_VER)
#  define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#  define NORETURN __attribute__((noreturn))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define NORETURN _Noreturn
#else
#  define NORETURN
#endif

namespace cxx_util {

/*!
 * Windows システムリソースに対するスマートポインタ
 */
template<typename T, auto Deleter>
struct ResourceHolder
{
	using holder_type = std::unique_ptr<std::remove_pointer_t<T>, decltype(Deleter)>;
	holder_type holder;

	/* implicit */ ResourceHolder(T t) noexcept
		: holder(t, Deleter)
	{
	}

	/* implicit */ operator T() const noexcept { return holder.get(); }
};

/*!
 * ワイド文字列をマルチバイト文字列に変換する
 */
std::string wcstombs_s(std::wstring_view wcs) {

	using localeHolder = ResourceHolder<_locale_t, &_free_locale>;

	// 現在のスレッドロケールを取得
	const localeHolder locale = _get_current_locale();
	if (!locale) {
		throw basis::message_error(L"Failed to get current locale");
	}

	// 変換に必要なバッファサイズを求める
	size_t required = 0;
	if (const auto ret = _wcstombs_s_l(&required, nullptr, 0, std::data(wcs), 0, locale); EILSEQ == ret) {
		throw std::invalid_argument("Invalid wide character sequence.");
	}

	// 変換に必要な出力バッファを確保する
	std::string buffer(required, '\0');

	size_t converted = 0;
	_wcstombs_s_l(&converted, std::data(buffer), std::size(buffer), std::data(wcs), _TRUNCATE, locale);

	buffer.resize(converted - 1); // wcstombs_sの戻り値は終端NULを含むので -1 する

	return buffer;
}

} // end of namespace cxx_util

//! HANDLE型のスマートポインタ
using HandleHolder = cxx_util::ResourceHolder<HANDLE, &CloseHandle>;

//! コンテキストメニューのスマートポインタ
using ContextMenuHolder = cxx_util::ResourceHolder<HWND, &DestroyWindow>;

namespace basis {

/*!
 * メッセージエラー
 * 
 * ワイド文字列でインスタンス化できるエラー。
 */
message_error::message_error(std::wstring_view message)
	: std::runtime_error(cxx_util::wcstombs_s(message))
	, _Message(message)
{
}

} // namespace basis

namespace testing {

/*!
 * GUIテストのための共通クラス
 *
 */
template<typename BaseTestSuiteType>
class TSakuraGuiAware : public TGuiAware<BaseTestSuiteType> {
protected:
	using Base = TGuiAware<BaseTestSuiteType>;

public:
	/*!
	 * プロファイル名
	 */
	static inline std::wstring_view gm_ProfileName = L""sv;

	/*!
	 * 設定ファイルのパス
	 *
	 * CShareData::BuildPrivateIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	static inline std::filesystem::path gm_IniPath;

	/*!
	 * テスト用1000行データファイルのパス
	 */
	static inline std::filesystem::path gm_TestFilePath1 = std::filesystem::current_path() / L"test_1000lines.txt";

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpFuncTest() {
		SFilePath szCommand;
		if (gm_ProfileName.length()) {
			szCommand = strprintf(LR"(-PROF="%s")", gm_ProfileName.data());
		}

		// コマンドラインのインスタンスを用意する
		CCommandLine commandLine;
		commandLine.ParseCommandLine(szCommand, false);

		// プロセスのインスタンスを用意する
		CControlProcess dummy(nullptr, szCommand);

		const auto isMultiUserSettings = false;
		const auto userRootFolder = 0;
		const auto& userSubFolder = L"sakura";

		// exe基準のiniファイルパスを得る
		const auto defaultIniPath = GetExeFileName().replace_extension(L".ini");

		// 設定ファイルフォルダー
		auto iniFolder = defaultIniPath;
		iniFolder.remove_filename();

		// iniファイル名を得る
		const auto filename = defaultIniPath.filename();

		// INIファイルのパスを組み立てる
		gm_IniPath = CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, gm_ProfileName.data(), filename);

		// INIファイルを削除する
		if (fexist(gm_IniPath)) {
			std::filesystem::remove(gm_IniPath);
		}

		// テスト用ファイル作成
		std::wofstream fs(gm_TestFilePath1);
		for (int n = 1; n <= 1000; n++) {
			fs << n << std::endl;
		}
		fs.close();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownFuncTest() {
		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// テスト用ファイルを削除
		if (fexist(gm_TestFilePath1)) {
			std::filesystem::remove(gm_TestFilePath1);
		}

		// INIファイルを削除する
		if (fexist(gm_IniPath)) {
			std::filesystem::remove(gm_IniPath);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (gm_ProfileName.length()) {
			std::filesystem::remove(gm_IniPath.parent_path());
		}
	}

	/*!
	 * @brief キャプション文字列を構築します。
	 */
	static std::wstring BuildCaption(std::wstring_view title)
	{
		const auto szAppName = SFilePath(GSTR_APPNAME);
		const auto versionMS = MAKELONG(VER_B, VER_A);
		const auto versionLS = MAKELONG(VER_D, VER_C);

		return strprintf(L"%s - %s %d.%d.%d.%d  ",
			title.data(),
			szAppName.c_str(),
			HIWORD(versionMS),
			LOWORD(versionMS),
			HIWORD(versionLS),
			LOWORD(versionLS)
		);
	}

	static void StartControlProcess();
	static void WaitForControlProcess();
	static HWND FindTrayWindow();
	static void TerminateControlProcess();

	/*!
	 * コンストラクタは流用
	 */
	using Base::Base;

protected:
	_Success_(return != nullptr)
	HWND WaitForEditor(
		std::optional<std::wstring_view> caption = std::nullopt
	) const
	{
		const auto windowName = caption.has_value() ? caption.value().data() : nullptr;

		const auto hWndEditor = Base::WaitForWindow(GSTR_EDITWINDOWNAME, windowName);

		EXPECT_THAT(hWndEditor, Ne(nullptr));

		return hWndEditor;
	}

	void SendEditorCommand(
		_In_ HWND hWndEditor,
		EFunctionCode eFuncCd
	) const
	{
		auto pEditor = Base::ElementFromHandle(hWndEditor);
		_com_util::CheckError(pEditor->SetFocus());

		SendMessageTimeoutW(hWndEditor, WM_COMMAND, MAKEWPARAM(eFuncCd, 0), 0,
			SMTO_BLOCK
			| SMTO_ABORTIFHUNG
			| SMTO_ERRORONEXIT
			,
			Base::defaultTimeoutMillis / 4,
			nullptr
		);
	}
};

/*!
 * @brief コントロールプロセスの初期化完了を待つ
 *
 * CEditorProcess::WaitForControlProcessとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::WaitForInitializedControlProcess()と概ね等価です。
 */
template<typename BaseTestSuiteType>
/* static */ void TSakuraGuiAware<BaseTestSuiteType>::WaitForControlProcess()
{
	// 初期化完了イベントを作成する
	SFilePath szEventName = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	if (gm_ProfileName.length()) {
		szEventName += gm_ProfileName;
	}

	HandleHolder hEvent = CreateEventW(nullptr, TRUE, FALSE, szEventName);
	if (!hEvent) {
		throw basis::message_error(L"create event failed.");
	}

	// 初期化完了イベントを待つ
	if (const auto dwRet = WaitForSingleObject(hEvent, 30000); WAIT_TIMEOUT == dwRet) {
		throw basis::message_error(L"waitEvent is timeout.");
	}
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * CControlProcess::Startとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::StartControlProcess()と概ね等価です。
 */
template<typename BaseTestSuiteType>
/* static */ void TSakuraGuiAware<BaseTestSuiteType>::StartControlProcess()
{
	// プロセスのタイトル
	SFilePath szTitle = L"sakura control process";

	// スタートアップ情報
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.lpTitle = szTitle;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	const auto exePath = GetExeFileName();

	std::wstring command = strprintf(LR"("%s" -PROF="%s" -NOWIN)", exePath.c_str(), std::data(gm_ProfileName));
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
	PROCESS_INFORMATION pi;

	// コントロールプロセスを起動する
	if (!CreateProcessW(
		exePath.c_str(),	// 実行可能モジュールパス
		command.data(),		// コマンドラインバッファ
		nullptr,			// プロセスのセキュリティ記述子
		nullptr,			// スレッドのセキュリティ記述子
		FALSE,				// ハンドルの継承オプション(継承させない)
		dwCreationFlag,		// 作成のフラグ
		nullptr,			// 環境変数(変更しない)
		nullptr,			// カレントディレクトリ(変更しない)
		&si,				// スタートアップ情報
		&pi					// プロセス情報(作成されたプロセス情報を格納する構造体)
	))
	{
		throw basis::message_error(L"create process failed.");
	}

	// 開いたハンドルは使わないので閉じておく
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	// コントロールプロセスの初期化完了を待つ
	WaitForControlProcess();
}

/*!
 * @brief トレイウインドウを検索する
 */
template<typename BaseTestSuiteType>
/* static */ HWND TSakuraGuiAware<BaseTestSuiteType>::FindTrayWindow()
{
	SFilePath szEditAppName = GSTR_CEDITAPP;
	if (gm_ProfileName.length()) {
		szEditAppName += gm_ProfileName;
	}
	return FindWindowW(szEditAppName, szEditAppName);
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 *
 * CControlProcess::Terminateとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存コードに該当する処理はありません。
 */
template<typename BaseTestSuiteType>
/* static */ void TSakuraGuiAware<BaseTestSuiteType>::TerminateControlProcess()
{
	// トレイウインドウを検索する
	const auto hTrayWnd = FindTrayWindow();
	if (!hTrayWnd) {
		// ウインドウがなければそのまま抜ける
		return;
	}

	// トレイウインドウからプロセスIDを取得する
	DWORD dwControlProcessId = 0;
	if (!GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId) || !dwControlProcessId) {
		throw basis::message_error(L"dwControlProcessId can't be retrived.");
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	HandleHolder hControlProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId);
	if (!hControlProcess) {
		throw basis::message_error(L"hControlProcess can't be opened.");
	}

	// トレイウインドウを閉じる
	SendMessageW(hTrayWnd, WM_CLOSE, 0, 0);

	// プロセス終了を待つ
	if (DWORD dwExitCode = 0; GetExitCodeProcess(hControlProcess, &dwExitCode) && dwExitCode == STILL_ACTIVE) {
		if (const auto waitProcessResult = WaitForSingleObject(hControlProcess, INFINITE); WAIT_TIMEOUT == waitProcessResult) {
			throw basis::message_error(L"waitProcess is timeout.");
		}
	}
}

} // namespace testing

namespace func {

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class WinMainTest : public testing::TSakuraGuiAware<::testing::TestWithParam<std::wstring_view>> {
protected:
	using Base = testing::TSakuraGuiAware<::testing::TestWithParam<std::wstring_view>>;

public:
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(SetUpGuiTestSuite());
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// OLEの初期化を解除する
		TearDownGuiTestSuite();
	}

	/*!
	 * コンストラクタは流用
	 */
	using Base::Base;

protected:
	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// プロファイル名を取得する
		Base::gm_ProfileName = GetParam();

		// UI Automationオブジェクトを作成する
		Base::SetUp();

		// 機能テストを初期化する
		SetUpFuncTest();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// 機能テストをクリーンアップする
		TearDownFuncTest();

		// UI Automationオブジェクトを解放する
		Base::TearDown();
	}

	/*!
	 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
	 */
	static void StartAndTerminateControlProcess()
	{
		// コントロールプロセスを起動する
		StartControlProcess();

		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// コントロールプロセスが終了すると、INIファイルが作成される
		auto iniCreated = fexist(gm_IniPath);
		const auto startTick = GetTickCount64();
		while (iniCreated && GetTickCount64() - startTick < defaultTimeoutMillis) {
			Sleep(10);  // 10msスリープしてリトライ
			iniCreated = fexist(gm_IniPath);
		}
	}

	/*!
	 * wWinMain呼出ラッパー
	 *
	 * EXPECT_EXITやEXPECT_DEATHで使うことを意図しています。
	 */
	static NORETURN void StartEditorProcess(const std::wstring& command) {
		exit(testing::StartEditorProcess(command));
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
	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	StartAndTerminateControlProcess();

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_TRUE(fexist(gm_IniPath));

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	StartAndTerminateControlProcess();

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_TRUE(fexist(gm_IniPath));
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_P(WinMainTest, runEditorProcess)
{
	// 起動時実行マクロの中身を作る
	constexpr std::array macroCommands = {
		L"Down();"sv,
		L"Up();"sv,
		L"Right();"sv,
		L"Left();"sv,

		L"Outline(0);"sv,				// アウトライン解析

		L"ShowFunckey();"sv,			// ShowFunckey 出す
		L"ShowMiniMap();"sv,			// ShowMiniMap 出す
		L"ShowTab();"sv,				// ShowTab 出す

		L"SelectAll();"sv,
		L"GoFileEnd();"sv,
		L"GoFileTop();"sv,

		L"PrintPreview();"sv,
		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,
		L"PrintPreview();"sv,

		L"SplitWinVH();"sv,
		L"NextWindow();NextWindow();NextWindow();NextWindow();"sv,

		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,

		L"PrevWindow();PrevWindow();PrevWindow();PrevWindow();"sv,
		L"SplitWinVH();"sv,

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
	const auto strStartupMacro = std::accumulate(macroCommands.cbegin(), macroCommands.cend(), std::wstring(), [](const std::wstring& a, std::wstring_view b) { return a + std::data(b); });

	// コマンドラインを組み立てる
	std::wstring command(gm_TestFilePath1.wstring());
	command += strprintf(LR"( -PROF="%s")", gm_ProfileName.data());
	command += strprintf(LR"( -MTYPE=js -M="%s")", std::regex_replace(strStartupMacro, std::wregex(LR"(")"), LR"("")").c_str());

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	EXPECT_EXIT({ StartEditorProcess(command); }, ::testing::ExitedWithCode(0), ".*" );
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示してプロセスを起動する。
 */
TEST_P(WinMainTest, showProfileMgr001)
{
	// FIXME: プロファイル指定がある場合、起動されるプロセスとプロファイル名が合わない
	if (gm_ProfileName.length()) {
		return;
	}

	// コマンドラインを組み立てる
	const auto command = strprintf(LR"(-PROFMGR -PROF="%s")", gm_ProfileName.data());

	// wWinMainを別スレッドで実行する
	std::thread t([command] {
		testing::StartEditorProcess(command);
	});

	// プロファイルマネージャの表示を待つ
	if (const auto hWndDlgProfileMgr = WaitForDialog(L"プロファイルマネージャ")) {
		// ボタンを押下する
		EmulateInvokeButton(hWndDlgProfileMgr, L"起動(S)");
	}

	// 編集ウインドウが起動するのを待つ
	const auto hWndEditor = WaitForEditor();

	// 編集ウインドウを閉じる
	WaitForClose(hWndEditor, [this, hWndEditor] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});

	t.join();
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示して何もせず閉じる。
 */
TEST_P(WinMainTest, showProfileMgr101)
{
	// コマンドラインを組み立てる
	const auto command = strprintf(LR"(-PROFMGR -PROF="%s")", gm_ProfileName.data());

	// wWinMainを別スレッドで実行する
	std::thread t([command] {
		testing::StartEditorProcess(command);
	});

	// プロファイルマネージャの表示を待つ
	if (const auto hWndDlgProfileMgr = WaitForDialog(L"プロファイルマネージャ")) {
		// プロファイルマネージャを閉じる
		WaitForClose(hWndDlgProfileMgr, [this, hWndDlgProfileMgr] () {
			EmulateInvokeButton(hWndDlgProfileMgr, L"閉じる(X)");
		});
	}

	t.join();

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_FALSE(fexist(gm_IniPath));
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  プロファイル指定なしとプロファイル指定ありの2パターンで実体化させる
 */
INSTANTIATE_TEST_SUITE_P(WinMain
	, WinMainTest
	, ::testing::Values(
		L"",
		L"profile1"
	)
);

} // namespace func
