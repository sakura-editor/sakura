/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <fstream>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "util/file.h"
#include "config/system_constants.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"

#include "testing/StartEditorProcess.hpp"
#include "cxx/ResourceHolder.hpp"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

namespace cxx {

/*!
 * @brief システムエラーを例外として発生させる
 *
 * @param message 追加のエラーメッセージ
 * @throw std::system_error システムエラー例外
 * @note 使い物になるかどうか試作してみただけ
 */
[[noreturn]] void raise_system_error(const std::string& message) {
	throw std::system_error(int(::GetLastError()), std::system_category(), message);
}

/*!
 * @brief システムディレクトリのパスを取得する
 *
 * @return システムディレクトリのパス
 */
std::filesystem::path GetSystemDirectoryW()
{
	SFilePath buf;
	::GetSystemDirectoryW(buf, int(std::size(buf)));
	return LPCWSTR(buf);
}

/*!
 * @brief テキストファイルを書き出す
 *
 * @param outPath 出力先パス
 * @param lines 書き込む行の配列
 * @note 使い物になるかどうか試作してみただけ
 */
void writeTextFile(
	const std::filesystem::path& outPath,
	std::span<const std::u8string_view> lines
)
{
	if (const auto parentPath = outPath.parent_path(); !fexist(parentPath)) {
		// ディレクトリが存在しない場合は作成する
		std::filesystem::create_directories(parentPath);
	}

	// ファイル出力ストリームをバイナリモードで開く
	std::ofstream fs(outPath, std::ios::binary);

	// UTF-8 BOMを出力
	const std::array bom = { '\xEF', '\xBB', '\xBF' };
	fs.write(bom.data(), bom.size());

	// 各行を書き込む
	for (const auto& line : lines) {
		if (!line.empty()) {
			fs.write(LPCSTR(std::data(line)), std::size(line));
		}
		fs << "\r\n";
	}

	fs.close();
}

//! HANDLE型のスマートポインタ
class HandleHolder final : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = HandleHolder;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::ResourceHolder;

	void lock() const noexcept
	{
		Lock(INFINITE);	//無限に待つ
	}

	bool try_lock() const noexcept
	{
		return Lock(0);	//ロック取得を試行
	}

	template<class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time) const noexcept
	{
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(rel_time);
		return Lock(DWORD(milliseconds.count()));
	}

	bool Lock(DWORD dwTimeout = INFINITE) const noexcept
	{
		// ロック取得を試行
		const auto dwRet = ::WaitForSingleObject(get(), dwTimeout);

		return WAIT_OBJECT_0 == dwRet || WAIT_ABANDONED == dwRet;
	}
};

} // namespace cxx

namespace winmain {

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct WinMainTest : public ::testing::TestWithParam<const wchar_t*> {
	/*!
	 * テスト用ファイルのパス
	 */
	static inline std::filesystem::path gm_TestDataPath = std::filesystem::current_path() / L"test_1000lines.txt";

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// テスト用ファイル作成
		std::wofstream fs(gm_TestDataPath);
		for (int n = 1; n <= 1000; n++) {
			fs << n << std::endl;
		}
		fs.close();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// テスト用ファイルの後始末
		if (fexist(gm_TestDataPath)) {
			std::filesystem::remove(gm_TestDataPath);
		}
	}

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
		const std::wstring_view profileName(GetParam());

		// コマンドラインのインスタンスを用意する
		CCommandLine commandLine;
		const auto strCommandLine = std::format(LR"(-PROF="{}")", profileName);
		commandLine.ParseCommandLine(strCommandLine.c_str(), false);

		// プロセスのインスタンスを用意する
		CControlProcess dummy(nullptr, strCommandLine.data());

		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::filesystem::remove(iniPath);
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::filesystem::remove(iniPath);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (const std::wstring_view profileName(GetParam()); !profileName.empty()) {
			std::filesystem::remove_all(iniPath.parent_path());
		}
	}

	/*!
	 * wWinMain呼出ラッパー
	 *
	 * テスト内で使うためのラッパー。
	 * 関数が呼出元に返らないことをマークしたバージョン。
	 */
	[[noreturn]] static void StartEditorProcess(const std::wstring& command) {
		exit(testing::StartEditorProcess(command));
	}
};

} // namespace winmain

/*!
 * @brief コントロールプロセスの初期化完了を待つ
 *
 * CControlProcess::WaitForInitializedとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::WaitForInitializedControlProcess()と概ね等価です。
 */
void CControlProcess_WaitForInitialized(const std::optional<std::wstring>& optProfileName)
{
	// 初期化完了イベントを作成する
	std::wstring strInitEvent{ GSTR_EVENT_SAKURA_CP_INITIALIZED };
	if (optProfileName.has_value()) {
		strInitEvent += *optProfileName;
	}
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, strInitEvent.c_str());
	if (!hEvent) {
		cxx::raise_system_error("create event failed.");
	}

	// 初期化完了イベントを待つ
	if (!hEvent.try_lock_for(std::chrono::milliseconds(30000))){
		cxx::raise_system_error("waitEvent is timeout.");
	}
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * CControlProcess::Startとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::StartControlProcess()と概ね等価です。
 */
void CControlProcess_Start(const std::optional<std::wstring>& optProfileName)
{
	const auto exePath = GetExeFileName();
	const auto lpszAppName = exePath.c_str();

	std::wstring strCommandLine = std::format(LR"("{}")", exePath.c_str());
	if (optProfileName.has_value()) {
		strCommandLine += std::format(LR"( -PROF="{}" -NOWIN)", optProfileName.value());
	}
	strCommandLine += L" -NOWIN";
	auto lpszCommandLine = std::data(strCommandLine);

	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;

	const auto systemDir = cxx::GetSystemDirectoryW();
	const auto lpszWorkingDir = systemDir.c_str();

	std::wstring title{ L"sakura control process" };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.lpTitle = std::data(title);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	// プロセス情報（出力用構造体なので値は入れない）
	PROCESS_INFORMATION pi{};

	// コントロールプロセスを起動する
	if (!::CreateProcessW(
		lpszAppName,		// 実行可能モジュールパス
		lpszCommandLine,	// コマンドラインバッファ
		nullptr,			// プロセスのセキュリティ記述子
		nullptr,			// スレッドのセキュリティ記述子
		FALSE,				// ハンドルの継承オプション(継承させない)
		dwCreationFlag,		// 作成のフラグ
		nullptr,			// 環境変数(変更しない)
		lpszWorkingDir,		// カレントディレクトリ(変更しない)
		&si,				// スタートアップ情報
		&pi					// プロセス情報(作成されたプロセス情報を格納する構造体)
	))
	{
		cxx::raise_system_error("create process failed.");
	}

	// 開いたハンドルは使わないので閉じておく
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);

	// コントロールプロセスの初期化完了を待つ
	CControlProcess_WaitForInitialized(optProfileName);
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 *
 * CControlProcess::Terminateとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存コードに該当する処理はありません。
 */
void CControlProcess_Terminate(const std::optional<std::wstring>& optProfileName)
{
	// トレイウインドウを検索する
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	if (optProfileName.has_value()) {
		trayWndClassName += *optProfileName;
	}

	const auto hTrayWnd = ::FindWindowW(trayWndClassName.c_str(), trayWndClassName.c_str());
	if (!hTrayWnd) {
		// ウインドウがなければそのまま抜ける
		return;
	}

	// トレイウインドウからプロセスIDを取得する
	DWORD dwControlProcessId = 0;
	::GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId);
	if (!dwControlProcessId) {
		cxx::raise_system_error("dwControlProcessId can't be retrived.");
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	cxx::HandleHolder hControlProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId);
	if (!hControlProcess) {
		cxx::raise_system_error("hControlProcess can't be opened.");
	}

	// トレイウインドウが閉じられるまで繰り返す
	while (::IsWindow(hTrayWnd)) {
		// トレイウインドウにクローズを要求する
		if (!::SendMessageTimeoutW(hTrayWnd, WM_CLOSE, 0, 0,
			SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT,
			5000,
			nullptr
		)) {
			// Sendが失敗したらPostしておく
			::PostMessageW(hTrayWnd, WM_CLOSE, 0, 0);

			// 少し待つ
			::Sleep(100);
		}
	}

	// メインウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	if (!hControlProcess.try_lock_for(std::chrono::milliseconds(30000))) {
		cxx::raise_system_error("waitProcess is timeout.");
	}
}

/*!
 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
 */
void CControlProcess_StartAndTerminate(const std::wstring& profileName)
{
	// コントロールプロセスを起動する
	CControlProcess_Start(profileName);

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(profileName);
}

namespace winmain {

/*!
 * @brief wWinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセスを実行する。
 *  プロセス起動は2回行い、1回目でINI作成＆書き込み、2回目でINI読み取りを検証する。
 */
TEST_P(WinMainTest, runWithNoWin)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(profileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(profileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_P(WinMainTest, runEditorProcess)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// ツールバー設定を出力
		u8"[Toolbar]"sv,
		u8"bToolBarIsFlat=1"sv,
		u8"nTBB[000]=1"sv,
		u8"nTBB[001]=25"sv,
		u8"nTBB[002]=3"sv,
		u8"nTBB[003]=4"sv,
		u8"nTBB[004]=0"sv,
		u8"nTBB[005]=33"sv,
		u8"nTBB[006]=34"sv,
		u8"nTBB[007]=0"sv,
		u8"nTBB[008]=87"sv,
		u8"nTBB[009]=88"sv,
		u8"nTBB[010]=0"sv,
		u8"nTBB[011]=225"sv,
		u8"nTBB[012]=226"sv,
		u8"nTBB[013]=227"sv,
		u8"nTBB[014]=228"sv,
		u8"nTBB[015]=229"sv,
		u8"nTBB[016]=230"sv,
		u8"nTBB[017]=0"sv,
		u8"nTBB[018]=232"sv,
		u8"nTBB[019]=0"sv,
		u8"nTBB[020]=264"sv,
		u8"nTBB[021]=265"sv,
		u8"nTBB[022]=266"sv,
		u8"nTBB[023]=0"sv,
		u8"nTBB[024]=346"sv,
		u8"nTBB[025]=246"sv,
		u8"nTBB[026]=384"sv,
		u8"nTBB[027]=246"sv,
		u8"nTBB[028]=384"sv,
		u8"nToolBarButtonNum=29"sv,
		u8""sv,

		// プリンター設定を出力
		u8"[Print]"sv,
		u8"PS[00].bColorPrint=0"sv,
		u8"PS[00].bKinsokuHead=0"sv,
		u8"PS[00].bKinsokuKuto=0"sv,
		u8"PS[00].bKinsokuRet=0"sv,
		u8"PS[00].bKinsokuTail=0"sv,
		u8"PS[00].lfFooter=0,0,0,0,0,0,0,0,0,0,0,0,0"sv,
		u8"PS[00].lfFooterFaceName="sv,
		u8"PS[00].lfHeader=0,0,0,0,0,0,0,0,0,0,0,0,0"sv,
		u8"PS[00].lfHeaderFaceName="sv,
		u8"PS[00].nFooterPointSize=0"sv,
		u8"PS[00].nHeaderPointSize=0"sv,
		u8"PS[00].nInts=12,24,1,70,30,100,200,200,100,1,9,1,0,1,0,0,1,0,0"sv,
		u8"PS[00].szDevice=Microsoft Print to PDF"sv,
		u8"PS[00].szDriver=winspool"sv,
		u8"PS[00].szFF=ＭＳ 明朝"sv,
		u8"PS[00].szFFZ=ＭＳ 明朝"sv,
		u8"PS[00].szFTF[0]="sv,
		u8"PS[00].szFTF[1]=- $p / $P -"sv, // ページ番号 / 総ページ数
		u8"PS[00].szFTF[2]="sv,
		u8"PS[00].szHF[0]=$f"sv,
		u8"PS[00].szHF[1]=$Q"sv,
		u8"PS[00].szHF[2]=$d $t"sv,
		u8"PS[00].szOutput=PORTPROMPT:"sv,
		u8"PS[00].szSName=印刷設定 1"sv,
	};
	cxx::writeTextFile(iniPath, iniLines);

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

		L"PrintPreview();"sv,			// 印刷プレビュー出す
		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,
		L"WheelPageDown();"sv,
		L"WheelPageUp();"sv,
		L"WheelPageRight();"sv,
		L"WheelPageLeft();"sv,
		L"PrintPreview();"sv,			// 印刷プレビュー消す

		L"SplitWinVH();"sv,
		L"NextWindow();NextWindow();NextWindow();NextWindow();"sv,

		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,

		L"PrevWindow();PrevWindow();PrevWindow();PrevWindow();"sv,
		L"SplitWinVH();"sv,

		L"Outline(1);"sv,				//アウトライン解析をリロード
		L"Outline(2);"sv,				//アウトライン解析を閉じる

		L"ShowFunckey();"sv,			//ShowFunckey 消す
		L"ShowMiniMap();"sv,			//ShowMiniMap 消す
		L"ShowTab();"sv,				//ShowTab 消す

		L"ShowToolbar();"sv,			//ShowToolbar 消す
		L"ShowStatusbar();"sv,			//ShowStatusbar 消す
		L"ShowStatusbar();"sv,			//ShowStatusbar 出す
		L"ShowToolbar();"sv,			//ShowToolbar 出す

		L"ExpandParameter('$I');"sv,	// INIファイルパスの取得(呼ぶだけ)

		L"ChgmodINS();"sv,
		L"ChgmodINS();"sv,

		L"SearchNext('3');"sv,			// 検索(呼ぶだけ)
		L"SearchPrev('3');"sv,			// 検索(呼ぶだけ)

		// ↓コマンドライン経由なので日本語入れると危険！
		L"Replace('3', 'threeeee');"sv,	// 置換(呼ぶだけ)
		L"Undo();"sv,
		L"Redo();"sv,
		L"Undo();"sv,

		// ↓コマンドライン経由なので日本語入れると危険！
		L"ReplaceAll('3', 'threeee');"sv,	// すべて置換(呼ぶだけ)
		L"Undo();"sv,

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

		L"ExitAll();"sv		//NOTE: このコマンドにより、エディタプロセスは起動された直後に終了する。
	};

	// 起動時実行マクロを組み立てる
	const auto strStartupMacro = std::accumulate(macroCommands.cbegin(), macroCommands.cend(), std::wstring(), [](const std::wstring& a, std::wstring_view b) { return a + std::data(b); });

	// コマンドラインを組み立てる
	std::wstring command(gm_TestDataPath);
	command += std::format(LR"( -PROF="{}")", std::wstring_view(profileName));
	command += std::format(LR"( -MTYPE=js -M="{}")", std::regex_replace(strStartupMacro, std::wregex(LR"(")"), LR"("")"));

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	EXPECT_EXIT({ StartEditorProcess(command); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスに終了指示を出して終了を待つ
	CControlProcess_Terminate(profileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());
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

} // namespace winmain
