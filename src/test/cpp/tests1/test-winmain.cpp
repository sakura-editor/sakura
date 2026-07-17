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
#include "dlg/ModalDialogCloser.hpp"
#include "window/EditorTestSuite.hpp"

#include "tests1_rc.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void extract_zip_resource(WORD id, const std::optional<std::filesystem::path>& optOutDir);

namespace cxx {

/*!
 * @brief システムエラーを例外として発生させる
 *
 * @param message 追加のエラーメッセージ
 * @throw std::system_error システムエラー例外
 * @note 使い物になるかどうか試作してみただけ
 */
NORETURN void raise_system_error(const std::string& message) {
	throw std::system_error(int(::GetLastError()), std::system_category(), message);
}

/*!
 * @brief トップレベルウインドウを検索する
 */
HWND FindWindowW(std::wstring_view className, const std::optional<std::wstring>& optWindowName = std::nullopt)
{
	return ::FindWindowW(std::data(std::wstring(className)), optWindowName.has_value() ? std::data(*optWindowName) : nullptr);
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
class HandleHolder : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = HandleHolder;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::ResourceHolder;

	virtual ~HandleHolder() = default;

	void lock()
	{
		Lock(INFINITE);	//無限に待つ
	}

	bool try_lock()
	{
		return Lock(0);	//ロック取得を試行
	}

	template<class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
	{
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(rel_time);
		return Lock(DWORD(milliseconds.count()));
	}

	bool unlock()
	{
		return Unlock();
	}

	virtual bool Lock(DWORD dwTimeout = INFINITE)
	{
		// ロック取得を試行
		const auto dwRet = ::WaitForSingleObject(get(), dwTimeout);

		if (WAIT_FAILED == dwRet) {
			// エラー
			return false;
		}

		return WAIT_OBJECT_0 == dwRet || WAIT_ABANDONED == dwRet;
	}

	virtual bool Unlock()
	{
		return true;
	}
};

/*!
 * @brief 起動したプロセスオブジェクト
 *
 * @note 使い物になるかどうか試作してみた
 */
class ProcessHolder : public cxx::HandleHolder
{
private:
	using Base = cxx::HandleHolder;
	using Me = ProcessHolder;

public:
	explicit ProcessHolder(
		HANDLE hProcess,
		DWORD dwProcessId,
		DWORD dwThreadId
	)
		: Base(hProcess)
		, dwProcessId(dwProcessId)
		, dwThreadId(dwThreadId)
	{
	}

	ProcessHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	ProcessHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	DWORD dwProcessId;
	DWORD dwThreadId;
};

} // namespace cxx

namespace testing {

/*!
 * @brief コマンドライン引数を引用符で囲む
 *
 * @param arg [in] コマンドライン引数
 * @return 引用符で囲まれたコマンドライン引数
 */
std::wstring QuoteArg(const std::wstring_view arg)
{
	const auto endsWithQuote = arg.ends_with(LR"(")");
	const auto containsQuotes = std::wstring_view::npos != arg.find_first_of(LR"(")");
	if (const auto needsEscape = std::wstring_view::npos != arg.find_first_of(L"\t ");
		!endsWithQuote && containsQuotes)
	{
		return std::format(LR"("{:s}")", std::regex_replace(arg.data(), std::wregex(LR"(")"), LR"("")"));
	}
	else if (!endsWithQuote && needsEscape)
	{
		return std::format(LR"("{:s}")", arg);
	}
	else
	{
		return std::wstring(arg);
	}
}

/*!
 * @brief サクラエディタのプロセスを起動する
 *
 * @param si スタートアップ情報
 * @param args コマンドライン引数
 * @param optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
 * @param optProfileName プロファイル名（省略した場合は指定なし）
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
cxx::ProcessHolder CreateSakuraProcess(
	STARTUPINFO& si,
	std::vector<std::wstring>& args,
	const std::optional<std::filesystem::path>& optWorkingDir = std::nullopt,
	const std::optional<std::wstring_view>& optProfileName = std::nullopt,
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE
)
{
	const auto exePath = GetExeFileName();

	if (optProfileName.has_value()) {
		args.emplace(args.begin(), std::format(LR"(-PROF="{}")", *optProfileName));
	}

	args.emplace(args.begin(), std::format(LR"("{:s}")", exePath.native()));

	auto strCommandLine = std::accumulate(args.begin(), args.end(), std::wstring(), [](const std::wstring& a, std::wstring_view b) { return std::format(LR"({} {})", a, QuoteArg(b)); });
	strCommandLine.erase(strCommandLine.cbegin());	// 先頭のスペースを削除

	auto lpszCommandLine = std::data(strCommandLine);

	LPCWSTR lpszWorkingDir = nullptr;
	if (optWorkingDir.has_value()) {
		if (const auto attr = ::GetFileAttributesW(optWorkingDir->c_str());
			INVALID_FILE_ATTRIBUTES != attr && (attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			lpszWorkingDir = optWorkingDir->c_str();
		}
	}

	// プロセス情報（出力用構造体なので値は入れない）
	PROCESS_INFORMATION pi{};

	// コントロールプロセスを起動する
	if (!::CreateProcessW(
		exePath.c_str(),	// 実行可能モジュールパス
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

	return cxx::ProcessHolder{ pi.hProcess, pi.dwProcessId, pi.dwThreadId };
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * @param profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
cxx::ProcessHolder CreateControlProcess(std::wstring_view profileName)
{
	// 初期化完了イベントの名前を決める
	SFilePath initEventName{ GSTR_EVENT_SAKURA_CP_INITIALIZED };
	initEventName += profileName;

	// プロセス起動前に初期化完了イベントを作成する
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);
	if (!hEvent || ERROR_ALREADY_EXISTS == ::GetLastError()) {
		cxx::raise_system_error("create event failed.");
	}

	std::wstring title{ L"sakura control process" };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.lpTitle = std::data(title);
	si.wShowWindow = SW_SHOWDEFAULT;

	std::vector<std::wstring> commandArgs{ LR"(-NOWIN)" };

	// コントロールプロセスを起動する
	auto cp = CreateSakuraProcess(si, commandArgs, cxx::GetSystemDirectoryW(), profileName);
	EXPECT_THAT(cp, NotNull());

	// 初期化完了を待つ
	std::array handles{ hEvent.get(), cp.get() };
	::WaitForMultipleObjects(DWORD(std::size(handles)), std::data(handles), FALSE, 15000);

	// プロセスオブジェクトを返す
	return cxx::ProcessHolder{ cp.release(), cp.dwProcessId, cp.dwThreadId };
}

/*!
 * @brief プロセスを起動する
 *
 * @tparam T コマンドライン引数のコンテナ型
 * @param args コマンドライン引数
 * @param profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
template<class T>
	requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
cxx::ProcessHolder CreateSakuraProcess(
	const T& args,
	std::wstring_view profileName,
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE
)
{
	// コマンドライン引数の編集用vector
	std::vector<std::wstring> commandArgs{ std::begin(args), std::end(args) };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	// エディタープロセスを起動する
	auto ep = CreateSakuraProcess(si, commandArgs, std::nullopt, profileName, dwCreationFlag);
	EXPECT_THAT(ep, NotNull());

	// プロセスオブジェクトを返す
	return cxx::ProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId };
}

/*!
 * @brief エディタープロセスを起動する
 *
 * @tparam T コマンドライン引数のコンテナ型
 * @param args コマンドライン引数
 * @param profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
template<class T>
	requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
cxx::ProcessHolder CreateEditorProcess(
	const T& args,
	std::wstring_view profileName,
	bool sync = true
)
{
	// コマンドライン引数の編集用vector
	std::vector<std::wstring> commandArgs{ std::begin(args), std::end(args) };

	// コマンドラインに -CODE 指定がない場合は付与する
	if (const auto found = std::ranges::find_if(args, [](const std::wstring& arg) { return std::regex_match(arg, std::wregex(LR"(\s*-CODE.*)", std::wregex::icase)); }); found == args.end()) {
		commandArgs.emplace_back(std::format(LR"(-CODE={})", static_cast<int>(CODE_AUTODETECT)));
	}

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
	if (sync) dwCreationFlag |= CREATE_SUSPENDED;

	// エディタープロセスを起動する
	auto ep = CreateSakuraProcess(si, commandArgs, std::nullopt, profileName, dwCreationFlag);
	EXPECT_THAT(ep, NotNull());

	if (!sync) return cxx::ProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId };

	// エディターのメインスレッドを開く
	cxx::HandleHolder hThread{ ::OpenThread(THREAD_SUSPEND_RESUME, FALSE, ep.dwThreadId) };

	// 初期化完了イベントを作成する
	SFilePath initEventName{ std::format(GSTR_EVENT_SAKURA_EP_INITIALIZED, ep.dwThreadId) };
	cxx::HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, FALSE, initEventName) };

	// エディターのメインスレッドを再開する
	::ResumeThread(hThread);

	// エディター初期化完了を待つ
	std::array handles{ hEvent.get(), ep.get() };
	if (const auto dwRet = ::WaitForMultipleObjects(DWORD(std::size(handles)), std::data(handles), FALSE, 30000); WAIT_OBJECT_0 != dwRet) {
		return cxx::ProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId };
	}

	// プロセスオブジェクトを返す
	return cxx::ProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId };
}

//! 外部ウインドウにクローズを要求する
void RequestForeignWindowClose(HWND hWnd)
{
	// ウインドウが閉じられるまで繰り返す
	while (::IsWindow(hWnd)) {
		// ウインドウにクローズを要求する
		if (!::SendMessageTimeoutW(hWnd, WM_CLOSE, 0, 0,
			SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT,
			5000,
			nullptr
		)) {
			// Sendが失敗したらPostしておく
			::PostMessageW(hWnd, WM_CLOSE, 0, 0);

			// 少し待つ
			::Sleep(100);
		}
	}
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 */
void TerminateControlProcess(
	std::wstring_view profileName,
	DWORD dwControlProcessId = 0
)
{
	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	if (const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName)) {
		if (!dwControlProcessId) {
			// トレイウインドウからプロセスIDを取得する
			::GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId);
			if (!dwControlProcessId) {
				cxx::raise_system_error("dwControlProcessId can't be retrived.");
			}
		}

		// トレイウインドウにクローズを要求する
		RequestForeignWindowClose(hTrayWnd);
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	// タイムアウト時に強制終了へフォールバックできるよう、TERMINATE 権限も付与する
	cxx::HandleHolder process{ ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, dwControlProcessId) };
	if (!process) {
		// プロセスIDが無効は「既に終了している」なので、除外する
		if (ERROR_INVALID_PARAMETER == ::GetLastError()) {
			return;
		}
		cxx::raise_system_error("hControlProcess can't be opened.");
	}

	// メインウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	process.lock();
}

} // namespace testing

namespace winmain {

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
template<class T>
struct TWinMainTest : public T, public window::UiaTestSuite {
	using Base = T;

	/*!
	 * テスト用ファイルのパス
	 */
	static inline std::filesystem::path gm_TestDataPath = std::filesystem::current_path() / L"test_1000lines.txt";

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		if (FAILED(::OleInitialize(nullptr)))
			FAIL();

		// UI Automationを初期化する
		SetUpUia();

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
		std::error_code ec;

		// テスト用ファイルの後始末
		if (fexist(gm_TestDataPath)) {
			std::filesystem::remove(gm_TestDataPath, ec);
		}

		if (const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins"); fexist(pluginPath)) {
			std::filesystem::remove_all(pluginPath, ec);
		}

		// UI Automationをシャットダウンする
		TearDownUia();

		// OLEをシャットダウンする
		::OleUninitialize();
	}

	/*!
	 * 設定ファイルのパス
	 *
	 * GetIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	std::filesystem::path iniPath;

	virtual ~TWinMainTest() = default;

	virtual std::wstring_view GetProfileName() const = 0;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テスト用プロファイル名
		const std::wstring_view profileName{ GetProfileName() };

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
			std::error_code ec;
			std::filesystem::remove(iniPath, ec);
		}

		// テスト用INIファイル作成
		// Grepダイアログを日本語で表示させるために設定を入れる
		constexpr std::array iniLines = {
			// 全般設定を出力
			u8"[Common]"sv,
			u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		};
		cxx::writeTextFile(iniPath, iniLines);
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::error_code ec;
			std::filesystem::remove(iniPath, ec);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (const std::wstring_view profileName{ GetProfileName() }; !profileName.empty()) {
			std::filesystem::remove_all(iniPath.parent_path());
		}
	}

	/*!
	 * wWinMain呼出ラッパー
	 *
	 * テスト内で使うためのラッパー。
	 * 関数が呼出元に返らないことをマークしたバージョン。
	 */
	static NORETURN void StartEditorProcess(const std::wstring& command) {
		exit(testing::StartEditorProcess(command));
	}

	/*!
	 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
	 */
	void CControlProcess_StartAndTerminate(std::wstring_view profileName) const
	{
		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	}
};

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct WinMainTest : public TWinMainTest<::testing::TestWithParam<std::wstring_view>> {
	std::wstring_view GetProfileName() const override {
		return GetParam();
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

	// プラグイン設定フォルダー
	const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins");

	// プラグイン定義を展開する
	extract_zip_resource(IDR_ZIPRES1, pluginPath);
	extract_zip_resource(IDR_ZIPRES4, pluginPath);

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::error_code ec;
	std::filesystem::remove(iniPath, ec);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)

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

		// プラグイン設定を出力
		u8"[Plugin]"sv,
		u8"EnablePlugin=1"sv,
		u8"P[00].CmdNum=1"sv,
		u8"P[00].Id=TestWshPlugin"sv,
		u8"P[00].Name=test-plugin"sv,
		u8"P[01].CmdNum=2"sv,
		u8"P[01].Id=TestDllPlugin"sv,
		u8"P[01].Name=test-dllplugin"sv,

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

	// プロファイル指定がある場合の追加テスト
	if (!profileName.empty()) {
		// ファイル出力ストリームをバイナリモードで開く
		std::ofstream fs(iniPath, std::ios::binary | std::ios::app);

		// ダークモードをONにする
		fs << "[Common]\r\n";
		fs << "bDarkMode=1\r\n";

		fs.close();
	}

	// コントロールプロセスを起動する
	auto cp = testing::CreateControlProcess(profileName);
	EXPECT_THAT(cp, NotNull());

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
		L"WinMaximize();"sv,
		L"WinRestore();"sv,
		L"WinMinimize();"sv,
		L"WinRestore();"sv,
		L"PrintPreview();"sv,			// 印刷プレビュー消す

		L"SplitWinVH();"sv,
		L"NextWindow();NextWindow();NextWindow();NextWindow();"sv,

		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,

		L"WinMaximize();"sv,
		L"WinRestore();"sv,
		L"WinMinimize();"sv,
		L"WinRestore();"sv,

		L"PrevWindow();PrevWindow();PrevWindow();PrevWindow();"sv,
		L"SplitWinVH();"sv,

		L"SplitWinV();"sv,
		L"SplitWinH();"sv,
		L"SplitWinH();"sv,
		L"SplitWinV();"sv,

		L"SplitWinH();"sv,
		L"SplitWinV();"sv,
		L"SplitWinV();"sv,
		L"SplitWinH();"sv,

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

		L"GoFileTop();"sv,
		L"SearchNext('3');"sv,			// 検索(呼ぶだけ)
		L"GoFileEnd();"sv,
		L"SearchPrev('3');"sv,			// 検索(呼ぶだけ)

		L"GoFileTop();"sv,
		// ↓コマンドライン経由なので日本語入れると危険！
		L"Replace('3', 'threeeee');"sv,	// 置換(呼ぶだけ)
		L"Undo();"sv,
		L"Redo();"sv,
		L"Undo();"sv,

		// ↓コマンドライン経由なので日本語入れると危険！
		L"ReplaceAll('3', 'threeee');"sv,	// すべて置換(呼ぶだけ)
		L"Undo();"sv,

		// OLEクリップボード
		L"SetClipboard(3, 'test');"sv,
		L"GetClipboard();"sv,
		L"ClipboardEmpty();"sv,

		// 生クリップボード
		L"SetClipboardByFormat('test', '12345', 0, -1);"sv,
		L"IsIncludeClipboardFormat('12345');"sv,
		L"GetClipboardByFormat('12345', 0, 0);"sv,
		L"ClipboardEmpty();"sv,

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
	command += std::format(LR"( -PROF="{}")", profileName);
	command += std::format(LR"( -MTYPE=js -M="{}")", std::regex_replace(strStartupMacro, std::wregex(LR"(")"), LR"("")"));

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	EXPECT_EXIT({ StartEditorProcess(command); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, cp.dwProcessId);

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

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct WinMainFuncTest : public TWinMainTest<::testing::Test> {
	std::wstring_view GetProfileName() const override {
		return L"";
	}
};

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセス起動の失敗をテストする。（ミューテックス競合）
 */
TEST_F(WinMainFuncTest, CreateControlProcess101)
{
	// テスト用プロファイル名
	const auto profileName{ GetProfileName() };

	// ミューテックスの名前を組み立てる
	SFilePath szMutexName{ GSTR_MUTEX_SAKURA_CP };
	szMutexName += profileName;

	// ミューテックスを作成してロックする
	cxx::HandleHolder hMutex{ ::CreateMutexW(nullptr, TRUE, szMutexName) };
	EXPECT_THAT(hMutex, NotNull());

	// コントロールプロセスを起動する
	EXPECT_EXIT({ StartEditorProcess(std::format(LR"(-NOWIN -PROF="{:s}")", profileName)); }, ::testing::ExitedWithCode(0), ".*");	// たぶんバグです。エラー終了なのに0を返してる。
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセス起動の失敗をテストする。（異なるバージョン）
 */
TEST_F(WinMainFuncTest, CreateControlProcess102)
{
	// テスト用プロファイル名
	const auto profileName{ GetProfileName() };

	// 共有データの名前を組み立てる
	SFilePath shareDataName{ GSTR_SHAREDATA };
	shareDataName.append(profileName);

	// ファイルマッピングオブジェクトを作る
	const auto hFileMap = ::CreateFileMappingW(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof(DLLSHAREDATA),
		shareDataName
	);

	EXPECT_THAT(hFileMap, NotNull());

	// スマートポインターに入れる
	cxx::HandleHolder fileMapHolder{ hFileMap };

	// ファイルマッピングオブジェクトをマップする
	using MappedDataHolder = cxx::ResourceHolder<&::UnmapViewOfFile, DLLSHAREDATA*>;
	MappedDataHolder mappedData = (DLLSHAREDATA*)::MapViewOfFile(
		hFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0
	);

	EXPECT_THAT(mappedData, NotNull());

	auto pShareData = static_cast<DLLSHAREDATA*>(mappedData);

	EXPECT_THAT(pShareData, NotNull());

	pShareData->m_nSize = sizeof(DLLSHAREDATA) + 1;

	// コントロールプロセスを起動する
	EXPECT_EXIT({ StartEditorProcess(std::format(LR"(-NOWIN -PROF="{:s}")", profileName)); }, ::testing::ExitedWithCode(0), ".*");	// たぶんバグです。エラー終了なのに0を返してる。
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepを実行する。
 */
TEST_F(WinMainFuncTest, DoGrep001)
{
	// テスト用プロファイル名
	const auto profileName{ GetProfileName() };

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::filesystem::remove(iniPath);

	// テスト用INIファイル作成
	// 常駐設定にしないとコントロールプロセスが落ちてしまうので設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"bTaskTrayStay=1"sv,	// コントロールプロセスを常駐させる
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
	};
	cxx::writeTextFile(iniPath, iniLines);

	// コントロールプロセスを起動する
	auto cp = testing::CreateControlProcess(profileName);
	EXPECT_THAT(cp, NotNull());

	std::array args{
		LR"(-GREPMODE)"s,
		LR"(-GKEY="localhost")"s,
		LR"(-GFILE="*.*;#en-US;#DriverData;#UMDF;#udc;#mde;#wd;!*.sys;!*.dll;!*.exe;!*.mui;!*.nls;!*.chm;!*.dat;!*.tmp;!*.wdf")"s,
		LR"(-GFOLDER="C:\WINDOWS\System32\Drivers")"s,
		LR"(-GOPT=SP1)"s
	};

	// エディタープロセスを起動する
	auto ep = testing::CreateEditorProcess(args, profileName);
	EXPECT_THAT(ep, NotNull());

	// 編集ウインドウにクローズを要求する
	const auto hWndFound = WaitForEditor();
	testing::RequestForeignWindowClose(hWndFound);

	// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	ep.lock();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, cp.dwProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  アウトプットウインドウを表示する。
 */
TEST_F(WinMainFuncTest, OpenDebugWindow001)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		// エディタープロセスを起動する
		auto ep = testing::CreateEditorProcess(std::array{ LR"(-DEBUGMODE)" }, profileName);
		EXPECT_THAT(ep, NotNull());

		// 編集ウインドウにクローズを要求する
		const auto hWndFound = WaitForEditor();
		testing::RequestForeignWindowClose(hWndFound);

		// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
		ep.lock();

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	});
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepダイアログを表示してキャンセルで閉じる。
 */
TEST_F(WinMainFuncTest, ShowDlgGrep101)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		// 表示されたGrepダイアログを閉じるためのスレッドを起動する
		std::jthread t = StartWindowCloser(L"Grep", [this] (HWND hWndDlg) {
			EmulateInvokeButton(hWndDlg, L"キャンセル(X)");
		});

		// エディタープロセスを起動する
		auto ep = testing::CreateEditorProcess(std::array{ LR"(-GREPDLG)", LR"(-GREPMODE)" }, profileName);
		EXPECT_THAT(ep, NotNull());

		// Grepダイアログが表示されるのを待って閉じる
		t.join();

		const auto hWndFound = WaitForEditor();
		EXPECT_THAT(hWndFound, NotNull());

		// 編集ウインドウにクローズを要求する
		testing::RequestForeignWindowClose(hWndFound);

		// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
		ep.lock();

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	});
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示してキャンセルで閉じる。
 */
TEST_F(WinMainFuncTest, ShowDlgProfileMgr101)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// 表示されたプロファイルマネージャを閉じるためのスレッドを起動する
		std::jthread t = StartWindowCloser(L"プロファイルマネージャ", [&] (HWND hWndDlg) {
			// プロファイルマネージャを閉じる
			EmulateInvokeButton(hWndDlg, L"閉じる(X)");
		});

		// エディタープロセスを起動する
		auto ep = testing::CreateEditorProcess(std::array{ LR"(-PROFMGR)" }, profileName);
		EXPECT_THAT(ep, NotNull());

		// プロファイルマネージャが表示されるのを待って閉じる
		t.join();

		// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
		ep.lock();
	});
}

} // namespace winmain
