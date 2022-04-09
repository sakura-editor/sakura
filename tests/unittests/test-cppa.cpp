/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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

#include <cstdlib>
#include <filesystem>

#include "eval_outputs.hpp"
#include "TExtModule.hpp"
#include "CEditorProcessForTest.hpp"

#include "macro/CPPA.h"
#include "macro/CPPAMacroMgr.h"

// PPAのコールバック関数
extern "C" {
	void CALLBACK ppaProc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd);
	void CALLBACK ppaIntFunc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd, int* resultValue);
	void CALLBACK ppaStrFunc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd, LPSTR* resultValue);
	void CALLBACK ppaStrObj(LPCSTR objName, int index, BYTE GS_Mode, int* errCd, LPSTR* value);
	void CALLBACK ppaErrorProc(int errCd, LPCSTR errMsg);
	void CALLBACK ppaFinishProc();
}

// PPAの定数
namespace ppa {
	constexpr auto omGet = 0;
	constexpr auto omSet = 1;
}

/*!
	スタブを用いてCPPAクラスをテストするためのもの
 */
class CPpaStub : public TExtModule<CPPA> {
public:
	CPpaStub()
		: TExtModule(L"ppa_stub.dll") {
	}

	void CallProc(LPCSTR FuncName, int Index, LPCSTR* Argument, int ArgSize, int* Err_CD) const {
		CallDllProc<decltype(&ppaProc)>(__func__, FuncName, Index, Argument, ArgSize, Err_CD);
	}

	void CallIntFunc(LPCSTR FuncName, int Index, LPCSTR* Argument, int ArgSize, int* Err_CD, int* ResultValue) const {
		CallDllProc<decltype(&ppaIntFunc)>(__func__, FuncName, Index, Argument, ArgSize, Err_CD, ResultValue);
	}

	void CallStrFunc(LPCSTR FuncName, int Index, LPCSTR* Argument, int ArgSize, int* Err_CD, LPSTR* ResultValue) const {
		CallDllProc<decltype(&ppaStrFunc)>(__func__, FuncName, Index, Argument, ArgSize, Err_CD, ResultValue);
	}

	void CallStrObj(LPCSTR ObjName, int Index, BYTE GS_Mode, int* Err_CD, LPSTR* Value) const {
		CallDllProc<decltype(&ppaStrObj)>(__func__, ObjName, Index, GS_Mode, Err_CD, Value);
	}

	//! エラーハンドラを呼び出す 
	void CallErrorProc(int errCd, LPCSTR errMsg) const {
		if (auto pcEditWnd = CEditWnd::getInstance()) {
			// PPAの実行コンテキストを用意する
			PpaExecInfo execInfo;
			execInfo.m_pcEditView = &pcEditWnd->GetActiveView();
			const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

			// エラーハンドラを呼び出す
			CallDllProc<decltype(&ppaErrorProc)>(__func__, errCd, errMsg);
		}
	}

	LPCWSTR CallGetDllNameImpl() {
		return CPPA::GetDllNameImp(0);
	}

	//! 終了ハンドラを呼び出す 
	void CallFinishProc() const {
		CallDllProc<decltype(&ppaFinishProc)>(__func__);
	}
};

/*!
	CPPAのためのフィクスチャクラス
 */
class CPpaTest : public TEditorProcessTest<CPPA> {
protected:
	[[nodiscard]] CEditView* GetActiveView() const noexcept {
		if (auto pcEditWnd = CEditWnd::getInstance()) {
			return &pcEditWnd->GetActiveView();
		}
		return nullptr;
	}
};

/*!
	CPPA::GetDllNameImpのテスト
 */
TEST_F(CPpaTest, GetDllNameImp)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	EXPECT_STREQ(L"PPA.DLL", cPpa.CallGetDllNameImpl());
}

/*!
	CPPAのインポート関数テスト

	スタブを使ったテスト。
 */
TEST_F(CPpaTest, CallApis)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	// DLL読み込みは成功する
	EXPECT_TRUE(cPpa.IsAvailable());

	// ひたすら呼ぶだけ。
	cPpa.SetProc(nullptr);
	cPpa.SetIntFunc(nullptr);
	cPpa.SetStrFunc(nullptr);
	cPpa.SetStrObj(nullptr);
	cPpa.SetErrProc(nullptr);
	cPpa.SetFinishProc(nullptr);

	cPpa.AddIntObj("", 0, FALSE, 0);
	cPpa.AddIntVar("", 0, 0);
	cPpa.AddRealObj("", 0.0, FALSE, 0L);
	cPpa.AddRealVar("", 0.0, FALSE);
	cPpa.AddStrObj("", "", FALSE, 0);
	cPpa.AddStrVar("", "", 0);
	cPpa.DeleteVar("");
	cPpa.GetArgBStr(0);
	cPpa.GetArgInt(0);
	cPpa.GetArgReal(0);
	cPpa.GetArgStr(0);
	cPpa.GetBStrVar("");
	cPpa.GetIntVar("");
	cPpa.GetVersion();
	cPpa.GetRealVar("");
	cPpa.GetStrVar("");
	cPpa.IsRunning();
	cPpa.SetDeclare("");
	cPpa.SetDefProc("");
	cPpa.SetDefine("");
	cPpa.SetIntObj(nullptr);
	cPpa.SetIntVar("", 0);
	cPpa.SetRealFunc(nullptr);
	cPpa.SetRealObj(nullptr);
	cPpa.SetRealVar("", 0.0);
	cPpa.SetSource("");
	cPpa.SetStrVar("", "");
	cPpa.Abort();
}

/*!
	CPPA::Executeのテスト

	スタブを使ったテスト。
	ppa_stub.Executeは何もしないのでテストは成功する。
 */
TEST_F(CPpaTest, Execute)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	// DLL読み込みは成功する
	EXPECT_TRUE(cPpa.IsAvailable());

	// Execute呼出は成功する
	EXPECT_TRUE(cPpa.Execute(GetActiveView(), 0));
}

/*!
	CPPA::Executeのテスト

	スタブを使ったテスト。
	多重実行防止機構をテストする。
 */
TEST_F(CPpaTest, Execute_mutex)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	// DLL読み込みは成功する
	EXPECT_TRUE(cPpa.IsAvailable());

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	// Execute呼出結果
	bool ret = false;

	// Execute内でエラーメッセージが出力される
	EXPECT_ERROUT(ret = cPpa.Execute(GetActiveView(), 0), L"PPA実行中に新たにPPAマクロを呼び出すことはできません");

	// Execute呼出は失敗する
	EXPECT_FALSE(ret);
}

/*!
	CPPA::GetDeclarationsのテスト

	PPAに渡す関数名を作る関数
	戻り値があればfunction、なければprocedureとみなす。
	対応する型はintとstringのみ。（PPA1.2は実数型にも対応しているが未対応）
	引数は最大8個まで指定できる。(PPAは32個まで対応しているが未対応）
 */
TEST_F(CPpaTest, GetDeclarations)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	setlocale(LC_ALL, "Japanese");

	// バッファ
	std::string buffer(1024, L'\0');

	// 引数型 int
	MacroFuncInfo funcInfo1 = { 1, L"Cmd1",  { VT_I4,   }, VT_EMPTY };
	cPpa.GetDeclarations(funcInfo1, buffer.data());
	EXPECT_STREQ("procedure S_Cmd1(i0: Integer); index 1;", buffer.data());

	// 引数型 string
	MacroFuncInfo funcInfo2 = { 2, L"Cmd2",  { VT_BSTR, }, VT_EMPTY };
	cPpa.GetDeclarations(funcInfo2, buffer.data());
	EXPECT_STREQ("procedure S_Cmd2(s0: string); index 2;", buffer.data());

	// 引数型 なし
	MacroFuncInfo funcInfo3 = { 3, L"Cmd3",  { VT_EMPTY, }, VT_EMPTY };
	cPpa.GetDeclarations(funcInfo3, buffer.data());
	EXPECT_STREQ("procedure S_Cmd3; index 3;", buffer.data());

	// 引数型 不明（intでもstringでもない）
	MacroFuncInfo funcInfo4 = { 4, L"Cmd4",  { VT_BOOL, }, VT_EMPTY };
	cPpa.GetDeclarations(funcInfo4, buffer.data());
	EXPECT_STREQ("procedure S_Cmd4(u0: Unknown); index 4;", buffer.data());

	// 戻り型 int
	MacroFuncInfo funcInfo5 = { 5, L"Func1", { VT_EMPTY }, VT_I4 };
	cPpa.GetDeclarations(funcInfo5, buffer.data());
	EXPECT_STREQ("function S_Func1: Integer; index 5;", buffer.data());

	// 戻り型 string
	MacroFuncInfo funcInfo6 = { 6, L"Func2", { VT_EMPTY }, VT_BSTR };
	cPpa.GetDeclarations(funcInfo6, buffer.data());
	EXPECT_STREQ("function S_Func2: string; index 6;", buffer.data());

	// 戻り型 不明（intでもstringでもない）
	MacroFuncInfo funcInfo7 = { 7, L"Func3", { VT_EMPTY }, VT_BOOL };
	cPpa.GetDeclarations(funcInfo7, buffer.data());
	EXPECT_STREQ("function S_Func3; index 7;", buffer.data());

	// 引数をたくさん指定する
	VARTYPE varArgEx8[] = { VT_I4, VT_BSTR, VT_I4, VT_BSTR };
	MacroFuncInfoEx funcInfoEx8 = { 8, 8, varArgEx8 };
	MacroFuncInfo funcInfo8 = { 8, L"Func4", { VT_I4, VT_BSTR, VT_I4, VT_BSTR }, VT_BSTR, &funcInfoEx8 };
	cPpa.GetDeclarations(funcInfo8, buffer.data());
	EXPECT_STREQ("function S_Func4(i0: Integer; s1: string; i2: Integer; s3: string; i4: Integer; s5: string; i6: Integer; s7: string): string; index 8;", buffer.data());
}

/*!
	マクロコマンドのテスト
 */
TEST_F(CPpaTest, Proc)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";

	setlocale(LC_ALL, "Japanese");

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	LPCSTR ppszArgs[] = { msg.data() };
	int errCd = 0;
	EXPECT_ERROUT(cPpa.CallProc("OkCancelBox", (int)F_OKCANCELBOX, ppszArgs, _countof(ppszArgs), &errCd), L"something is wrong.");
}

/*!
	数値を返すマクロ関数のテスト
 */
TEST_F(CPpaTest, IntFunc)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";

	setlocale(LC_ALL, "Japanese");

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	int errCd = 0;
	int nResultValue = 1234;

	// 数値を返却する関数を呼び出す
	LPCSTR ppszArgs1[4] = { msg.data(), nullptr };
	EXPECT_ERROUT(cPpa.CallIntFunc("OkCancelBox", (int)F_OKCANCELBOX, ppszArgs1, _countof(ppszArgs1), &errCd, &nResultValue), L"something is wrong.");
	EXPECT_EQ(0, nResultValue);
	EXPECT_EQ(0, errCd);

	// 文字列を返却する関数を呼び出す
	LPCSTR ppszArgs2[] = { "$I" };
	cPpa.CallIntFunc("ExpandParameter", (int)F_EXPANDPARAMETER, ppszArgs2, _countof(ppszArgs2), &errCd, &nResultValue);
	EXPECT_EQ(0, nResultValue);
	EXPECT_EQ(-2, errCd);

	// コマンドを呼び出す
	LPCSTR ppszArgs3[] = { nullptr };
	cPpa.CallIntFunc("FileNew", (int)F_FILENEW, ppszArgs3, _countof(ppszArgs3), &errCd, &nResultValue);
	EXPECT_EQ(0, nResultValue);
	EXPECT_EQ((int)F_FILENEW + 1, errCd);
}

/*!
	文字列を返すマクロ関数のテスト
 */
TEST_F(CPpaTest, StrFunc)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";

	setlocale(LC_ALL, "Japanese");

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	std::string buffer(256, L'\0');
	LPSTR pszResultValue = buffer.data();
	int errCd = 0;

	// 文字列を返却する関数を呼び出す
	LPCSTR ppszArgs1[] = { "$I" };
	cPpa.CallStrFunc("ExpandParameter", (int)F_EXPANDPARAMETER, ppszArgs1, _countof(ppszArgs1), &errCd, &pszResultValue);
	EXPECT_STREQ(iniPath.string().c_str(), pszResultValue);
	EXPECT_EQ(0, errCd);

	// 数値を返却する関数を呼び出す
	LPCSTR ppszArgs2[4] = { msg.data(), nullptr };
	EXPECT_ERROUT(cPpa.CallStrFunc("OkCancelBox", (int)F_OKCANCELBOX, ppszArgs2, _countof(ppszArgs2), &errCd, &pszResultValue), L"something is wrong.");
	EXPECT_STREQ("", pszResultValue);
	EXPECT_EQ((int)F_OKCANCELBOX + 1, errCd);

	// コマンドを呼び出す
	LPCSTR ppszArgs3[] = { nullptr };
	cPpa.CallStrFunc("FileNew", (int)F_FILENEW, ppszArgs3, _countof(ppszArgs3), &errCd, &pszResultValue);
	EXPECT_STREQ("", pszResultValue);
	EXPECT_EQ((int)F_FILENEW + 1, errCd);
}

/*!
	文字列ハンドラのテスト
 */
TEST_F(CPpaTest, StrObj)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	setlocale(LC_ALL, "Japanese");

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	constexpr auto& objName = "テキトーな名前";
	constexpr int index = 2;
	int errCd = 0;
	LPSTR value = nullptr;

	// 初期値チェック
	cPpa.CallStrObj(objName, index, ppa::omGet, &errCd, &value);
	EXPECT_STREQ("", value);
	EXPECT_EQ(0, errCd);

	// 更新に使う文字列値を用意する
	std::string testValue = "適当な値";
	value = testValue.data();

	// 更新
	cPpa.CallStrObj(objName, index, ppa::omSet, &errCd, &value);
	EXPECT_EQ(0, errCd);

	// 取得値チェック
	cPpa.CallStrObj(objName, index, ppa::omSet, &errCd, &value);
	EXPECT_STREQ(testValue.data(), value);
	EXPECT_EQ(0, errCd);

	// パラメータ不正時の挙動チェック
	cPpa.CallStrObj(objName, 0, ppa::omSet, &errCd, &value);
	EXPECT_EQ(-1, errCd);
	cPpa.CallStrObj(objName, 1, ppa::omSet, &errCd, &value);
	EXPECT_EQ(-1, errCd);
	cPpa.CallStrObj(objName, 3, ppa::omSet, &errCd, &value);
	EXPECT_EQ(-1, errCd);
}

/*!
	エラーハンドラのテスト

	引数に応じてメッセージボックスを表示する。
 */
TEST_F(CPpaTest, ErrorProc_funcCode_commands)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";

	setlocale(LC_ALL, "Japanese");

	// Err_CD > 0 の場合「サクラエディタのエラー」
	// 機能IDに対応する関数名をメッセージに含める
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILENEW + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileNew", (int)F_FILENEW).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILEOPEN2 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileOpen(s0: string; i1: Integer; i2: Integer; s3: string)", (int)F_FILEOPEN2).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILESAVE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileSave", (int)F_FILESAVE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILESAVEALL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileSaveAll", (int)F_FILESAVEALL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILESAVEAS_DIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileSaveAsDialog(s0: string; i1: Integer; i2: Integer)", (int)F_FILESAVEAS_DIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILESAVEAS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileSaveAs(s0: string; i1: Integer; i2: Integer)", (int)F_FILESAVEAS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILECLOSE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileClose", (int)F_FILECLOSE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILECLOSE_OPEN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileCloseOpen", (int)F_FILECLOSE_OPEN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopen(i0: Integer)", (int)F_FILE_REOPEN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenSJIS(i0: Integer)", (int)F_FILE_REOPEN_SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_JIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenJIS(i0: Integer)", (int)F_FILE_REOPEN_JIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_EUC + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenEUC(i0: Integer)", (int)F_FILE_REOPEN_EUC).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_LATIN1 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenLatin1(i0: Integer)", (int)F_FILE_REOPEN_LATIN1).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_UNICODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenUNICODE(i0: Integer)", (int)F_FILE_REOPEN_UNICODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_UNICODEBE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenUNICODEBE(i0: Integer)", (int)F_FILE_REOPEN_UNICODEBE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_UTF8 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenUTF8(i0: Integer)", (int)F_FILE_REOPEN_UTF8).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_CESU8 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenCESU8(i0: Integer)", (int)F_FILE_REOPEN_CESU8).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILE_REOPEN_UTF7 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FileReopenUTF7(i0: Integer)", (int)F_FILE_REOPEN_UTF7).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PRINT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Print", (int)F_PRINT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PRINT_PREVIEW + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PrintPreview", (int)F_PRINT_PREVIEW).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PRINT_PAGESETUP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PrintPageSetup", (int)F_PRINT_PAGESETUP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_OPEN_HfromtoC + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_OpenHfromtoC", (int)F_OPEN_HfromtoC).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ACTIVATE_SQLPLUS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ActivateSQLPLUS", (int)F_ACTIVATE_SQLPLUS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PLSQL_COMPILE_ON_SQLPLUS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExecSQLPLUS", (int)F_PLSQL_COMPILE_ON_SQLPLUS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BROWSE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Browse", (int)F_BROWSE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_VIEWMODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ViewMode", (int)F_VIEWMODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PROPERTY_FILE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PropertyFile", (int)F_PROPERTY_FILE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXITALLEDITORS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExitAllEditors", (int)F_EXITALLEDITORS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXITALL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExitAll", (int)F_EXITALL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PUTFILE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PutFile(s0: string; i1: Integer; i2: Integer)", (int)F_PUTFILE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INSFILE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsFile(s0: string; i1: Integer; i2: Integer)", (int)F_INSFILE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WCHAR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Char(i0: Integer)", (int)F_WCHAR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_IME_CHAR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CharIme(i0: Integer)", (int)F_IME_CHAR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UNDO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Undo", (int)F_UNDO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_REDO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Redo", (int)F_REDO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DELETE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Delete", (int)F_DELETE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DELETE_BACK + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DeleteBack", (int)F_DELETE_BACK).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WordDeleteToStart + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordDeleteToStart", (int)F_WordDeleteToStart).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WordDeleteToEnd + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordDeleteToEnd", (int)F_WordDeleteToEnd).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WordCut + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordCut", (int)F_WordCut).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WordDelete + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordDelete", (int)F_WordDelete).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LineCutToStart + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_LineCutToStart", (int)F_LineCutToStart).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LineCutToEnd + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_LineCutToEnd", (int)F_LineCutToEnd).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LineDeleteToStart + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_LineDeleteToStart", (int)F_LineDeleteToStart).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LineDeleteToEnd + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_LineDeleteToEnd", (int)F_LineDeleteToEnd).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUT_LINE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CutLine", (int)F_CUT_LINE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DELETE_LINE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DeleteLine", (int)F_DELETE_LINE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DUPLICATELINE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DuplicateLine", (int)F_DUPLICATELINE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INDENT_TAB + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_IndentTab", (int)F_INDENT_TAB).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UNINDENT_TAB + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_UnindentTab", (int)F_UNINDENT_TAB).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INDENT_SPACE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_IndentSpace", (int)F_INDENT_SPACE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UNINDENT_SPACE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_UnindentSpace", (int)F_UNINDENT_SPACE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LTRIM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_LTrim", (int)F_LTRIM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_RTRIM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_RTrim", (int)F_RTRIM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SORT_ASC + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SortAsc", (int)F_SORT_ASC).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SORT_DESC + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SortDesc", (int)F_SORT_DESC).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MERGE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Merge", (int)F_MERGE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Up", (int)F_UP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DOWN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Down", (int)F_DOWN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Left", (int)F_LEFT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_RIGHT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Right", (int)F_RIGHT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UP2 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Up2", (int)F_UP2).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DOWN2 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Down2", (int)F_DOWN2).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WORDLEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordLeft", (int)F_WORDLEFT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WORDRIGHT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordRight", (int)F_WORDRIGHT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLINETOP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLineTop(i0: Integer)", (int)F_GOLINETOP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLINEEND + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLineEnd(i0: Integer)", (int)F_GOLINEEND).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HalfPageUp + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HalfPageUp(i0: Integer)", (int)F_HalfPageUp).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HalfPageDown + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HalfPageDown(i0: Integer)", (int)F_HalfPageDown).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_1PageUp + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PageUp(i0: Integer)", (int)F_1PageUp).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_1PageDown + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PageDown(i0: Integer)", (int)F_1PageDown).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOFILETOP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoFileTop", (int)F_GOFILETOP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOFILEEND + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoFileEnd", (int)F_GOFILEEND).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CURLINECENTER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CurLineCenter", (int)F_CURLINECENTER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CURLINETOP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CurLineTop", (int)F_CURLINETOP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CURLINEBOTTOM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CurLineBottom", (int)F_CURLINEBOTTOM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_JUMPHIST_PREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MoveHistPrev", (int)F_JUMPHIST_PREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_JUMPHIST_NEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MoveHistNext", (int)F_JUMPHIST_NEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_JUMPHIST_SET + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MoveHistSet", (int)F_JUMPHIST_SET).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WndScrollDown + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_F_WndScrollDown", (int)F_WndScrollDown).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WndScrollUp + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_F_WndScrollUp", (int)F_WndScrollUp).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GONEXTPARAGRAPH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoNextParagraph", (int)F_GONEXTPARAGRAPH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOPREVPARAGRAPH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoPrevParagraph", (int)F_GOPREVPARAGRAPH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MODIFYLINE_NEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoModifyLineNext", (int)F_MODIFYLINE_NEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MODIFYLINE_PREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoModifyLinePrev", (int)F_MODIFYLINE_PREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MOVECURSOR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MoveCursor(i0: Integer; i1: Integer; i2: Integer)", (int)F_MOVECURSOR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MOVECURSORLAYOUT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MoveCursorLayout(i0: Integer; i1: Integer; i2: Integer)", (int)F_MOVECURSORLAYOUT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELUP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelUp(i0: Integer)", (int)F_WHEELUP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELDOWN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelDown(i0: Integer)", (int)F_WHEELDOWN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELLEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelLeft(i0: Integer)", (int)F_WHEELLEFT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELRIGHT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelRight(i0: Integer)", (int)F_WHEELRIGHT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELPAGEUP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelPageUp(i0: Integer)", (int)F_WHEELPAGEUP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELPAGEDOWN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelPageDown(i0: Integer)", (int)F_WHEELPAGEDOWN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELPAGELEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelPageLeft(i0: Integer)", (int)F_WHEELPAGELEFT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WHEELPAGERIGHT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WheelPageRight(i0: Integer)", (int)F_WHEELPAGERIGHT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SELECTWORD + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SelectWord", (int)F_SELECTWORD).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SELECTALL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SelectAll", (int)F_SELECTALL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SELECTLINE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SelectLine(i0: Integer)", (int)F_SELECTLINE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BEGIN_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BeginSelect", (int)F_BEGIN_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UP_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Up_Sel", (int)F_UP_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DOWN_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Down_Sel", (int)F_DOWN_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LEFT_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Left_Sel", (int)F_LEFT_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_RIGHT_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Right_Sel", (int)F_RIGHT_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UP2_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Up2_Sel", (int)F_UP2_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DOWN2_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Down2_Sel", (int)F_DOWN2_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WORDLEFT_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordLeft_Sel", (int)F_WORDLEFT_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WORDRIGHT_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordRight_Sel", (int)F_WORDRIGHT_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLINETOP_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLineTop_Sel(i0: Integer)", (int)F_GOLINETOP_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLINEEND_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLineEnd_Sel(i0: Integer)", (int)F_GOLINEEND_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HalfPageUp_Sel + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HalfPageUp_Sel(i0: Integer)", (int)F_HalfPageUp_Sel).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HalfPageDown_Sel + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HalfPageDown_Sel(i0: Integer)", (int)F_HalfPageDown_Sel).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_1PageUp_Sel + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PageUp_Sel(i0: Integer)", (int)F_1PageUp_Sel).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_1PageDown_Sel + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PageDown_Sel(i0: Integer)", (int)F_1PageDown_Sel).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOFILETOP_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoFileTop_Sel", (int)F_GOFILETOP_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOFILEEND_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoFileEnd_Sel", (int)F_GOFILEEND_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GONEXTPARAGRAPH_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoNextParagraph_Sel", (int)F_GONEXTPARAGRAPH_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOPREVPARAGRAPH_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoPrevParagraph_Sel", (int)F_GOPREVPARAGRAPH_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MODIFYLINE_NEXT_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoModifyLineNext_Sel", (int)F_MODIFYLINE_NEXT_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MODIFYLINE_PREV_SEL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoModifyLinePrev_Sel", (int)F_MODIFYLINE_PREV_SEL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BEGIN_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BeginBoxSelect", (int)F_BEGIN_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UP_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Up_BoxSel(i0: Integer)", (int)F_UP_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DOWN_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Down_BoxSel(i0: Integer)", (int)F_DOWN_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LEFT_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Left_BoxSel(i0: Integer)", (int)F_LEFT_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_RIGHT_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Right_BoxSel(i0: Integer)", (int)F_RIGHT_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UP2_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Up2_BoxSel(i0: Integer)", (int)F_UP2_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DOWN2_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Down2_BoxSel(i0: Integer)", (int)F_DOWN2_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WORDLEFT_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordLeft_BoxSel(i0: Integer)", (int)F_WORDLEFT_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WORDRIGHT_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WordRight_BoxSel(i0: Integer)", (int)F_WORDRIGHT_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLOGICALLINETOP_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLogicalLineTop_BoxSel(i0: Integer; i1: Integer)", (int)F_GOLOGICALLINETOP_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLINETOP_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLineTop_BoxSel(i0: Integer; i1: Integer)", (int)F_GOLINETOP_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOLINEEND_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoLineEnd_BoxSel(i0: Integer; i1: Integer)", (int)F_GOLINEEND_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HalfPageUp_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HalfPageUp_BoxSel(i0: Integer; i1: Integer)", (int)F_HalfPageUp_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HalfPageDown_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HalfPageDown_BoxSel(i0: Integer; i1: Integer)", (int)F_HalfPageDown_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_1PageUp_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PageUp_BoxSel(i0: Integer; i1: Integer)", (int)F_1PageUp_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_1PageDown_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PageDown_BoxSel(i0: Integer; i1: Integer)", (int)F_1PageDown_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOFILETOP_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoFileTop_BoxSel(i0: Integer)", (int)F_GOFILETOP_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GOFILEEND_BOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GoFileEnd_BoxSel(i0: Integer)", (int)F_GOFILEEND_BOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Cut", (int)F_CUT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPY + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Copy", (int)F_COPY).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PASTE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Paste(i0: Integer)", (int)F_PASTE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPY_ADDCRLF + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyAddCRLF", (int)F_COPY_ADDCRLF).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPY_CRLF + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyCRLF", (int)F_COPY_CRLF).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PASTEBOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PasteBox(i0: Integer)", (int)F_PASTEBOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INSBOXTEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsBoxText(s0: string)", (int)F_INSBOXTEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INSTEXT_W + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsText(s0: string)", (int)F_INSTEXT_W).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ADDTAIL_W + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_AddTail(s0: string)", (int)F_ADDTAIL_W).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYLINES + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyLines", (int)F_COPYLINES).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYLINESASPASSAGE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyLinesAsPassage", (int)F_COPYLINESASPASSAGE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYLINESWITHLINENUMBER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyLinesWithLineNumber", (int)F_COPYLINESWITHLINENUMBER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPY_COLOR_HTML + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyColorHtml", (int)F_COPY_COLOR_HTML).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPY_COLOR_HTML_LINENUMBER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyColorHtmlWithLineNumber", (int)F_COPY_COLOR_HTML_LINENUMBER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYPATH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyPath", (int)F_COPYPATH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYDIRPATH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyDirPath", (int)F_COPYDIRPATH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYFNAME + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyFilename", (int)F_COPYFNAME).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COPYTAG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyTag", (int)F_COPYTAG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CREATEKEYBINDLIST + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CopyKeyBindList", (int)F_CREATEKEYBINDLIST).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INS_DATE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsertDate", (int)F_INS_DATE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INS_TIME + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsertTime", (int)F_INS_TIME).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CTRL_CODE_DIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CtrlCodeDialog", (int)F_CTRL_CODE_DIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CTRL_CODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CtrlCode(i0: Integer)", (int)F_CTRL_CODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INS_FILE_USED_RECENTLY + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsertFileUsedRecently", (int)F_INS_FILE_USED_RECENTLY).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INS_FOLDER_USED_RECENTLY + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_InsertFolderUsedRecently", (int)F_INS_FOLDER_USED_RECENTLY).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOLOWER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToLower", (int)F_TOLOWER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOUPPER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToUpper", (int)F_TOUPPER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOHANKAKU + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToHankaku", (int)F_TOHANKAKU).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOHANKATA + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToHankata", (int)F_TOHANKATA).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOZENEI + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToZenEi", (int)F_TOZENEI).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOHANEI + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToHanEi", (int)F_TOHANEI).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOZENKAKUKATA + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToZenKata", (int)F_TOZENKAKUKATA).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOZENKAKUHIRA + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToZenHira", (int)F_TOZENKAKUHIRA).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HANKATATOZENKATA + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HanKataToZenKata", (int)F_HANKATATOZENKATA).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HANKATATOZENHIRA + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HanKataToZenHira", (int)F_HANKATATOZENHIRA).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TABTOSPACE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TABToSPACE", (int)F_TABTOSPACE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SPACETOTAB + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SPACEToTAB", (int)F_SPACETOTAB).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_AUTO2SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_AutoToSJIS", (int)F_CODECNV_AUTO2SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_EMAIL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_JIStoSJIS", (int)F_CODECNV_EMAIL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_EUC2SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_EUCtoSJIS", (int)F_CODECNV_EUC2SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_UNICODE2SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CodeCnvUNICODEtoSJIS", (int)F_CODECNV_UNICODE2SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_UNICODEBE2SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CodeCnvUNICODEBEtoSJIS", (int)F_CODECNV_UNICODEBE2SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_UTF82SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_UTF8toSJIS", (int)F_CODECNV_UTF82SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_UTF72SJIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_UTF7toSJIS", (int)F_CODECNV_UTF72SJIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_SJIS2JIS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SJIStoJIS", (int)F_CODECNV_SJIS2JIS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_SJIS2EUC + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SJIStoEUC", (int)F_CODECNV_SJIS2EUC).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_SJIS2UTF8 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SJIStoUTF8", (int)F_CODECNV_SJIS2UTF8).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CODECNV_SJIS2UTF7 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SJIStoUTF7", (int)F_CODECNV_SJIS2UTF7).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BASE64DECODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Base64Decode", (int)F_BASE64DECODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_UUDECODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Uudecode", (int)F_UUDECODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SEARCH_DIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SearchDialog", (int)F_SEARCH_DIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SEARCH_NEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SearchNext(s0: string; i1: Integer)", (int)F_SEARCH_NEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SEARCH_PREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SearchPrev(s0: string; i1: Integer)", (int)F_SEARCH_PREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_REPLACE_DIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ReplaceDialog", (int)F_REPLACE_DIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_REPLACE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Replace(s0: string; s1: string; i2: Integer)", (int)F_REPLACE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_REPLACE_ALL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ReplaceAll(s0: string; s1: string; i2: Integer)", (int)F_REPLACE_ALL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SEARCH_CLEARMARK + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SearchClearMark", (int)F_SEARCH_CLEARMARK).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_JUMP_SRCHSTARTPOS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SearchStartPos", (int)F_JUMP_SRCHSTARTPOS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GREP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Grep(s0: string; s1: string; s2: string; i3: Integer; i4: Integer)", (int)F_GREP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GREP_REPLACE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GrepReplace(s0: string; s1: string; s2: string; s3: string; i4: Integer; i5: Integer)", (int)F_GREP_REPLACE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_JUMP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Jump(i0: Integer; i1: Integer)", (int)F_JUMP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_OUTLINE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Outline(i0: Integer)", (int)F_OUTLINE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAGJUMP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TagJump", (int)F_TAGJUMP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAGJUMPBACK + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TagJumpBack", (int)F_TAGJUMPBACK).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAGS_MAKE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TagMake", (int)F_TAGS_MAKE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DIRECT_TAGJUMP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DirectTagJump", (int)F_DIRECT_TAGJUMP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAGJUMP_KEYWORD + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_KeywordTagJump(s0: string)", (int)F_TAGJUMP_KEYWORD).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COMPARE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Compare", (int)F_COMPARE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DIFF_DIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DiffDialog", (int)F_DIFF_DIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DIFF + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Diff(s0: string; i1: Integer)", (int)F_DIFF).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DIFF_NEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DiffNext", (int)F_DIFF_NEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DIFF_PREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DiffPrev", (int)F_DIFF_PREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DIFF_RESET + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_DiffReset", (int)F_DIFF_RESET).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BRACKETPAIR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BracketPair", (int)F_BRACKETPAIR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BOOKMARK_SET + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BookmarkSet", (int)F_BOOKMARK_SET).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BOOKMARK_NEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BookmarkNext", (int)F_BOOKMARK_NEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BOOKMARK_PREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BookmarkPrev", (int)F_BOOKMARK_PREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BOOKMARK_RESET + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BookmarkReset", (int)F_BOOKMARK_RESET).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BOOKMARK_VIEW + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BookmarkView", (int)F_BOOKMARK_VIEW).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_BOOKMARK_PATTERN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_BookmarkPattern(s0: string; i1: Integer)", (int)F_BOOKMARK_PATTERN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FUNCLIST_NEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FuncListNext", (int)F_FUNCLIST_NEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FUNCLIST_PREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_FuncListPrev", (int)F_FUNCLIST_PREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CHGMOD_INS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ChgmodINS", (int)F_CHGMOD_INS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CHG_CHARSET + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ChgCharSet(i0: Integer; i1: Integer)", (int)F_CHG_CHARSET).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CHGMOD_EOL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ChgmodEOL(i0: Integer)", (int)F_CHGMOD_EOL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CANCEL_MODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CancelMode", (int)F_CANCEL_MODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXECEXTMACRO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExecExternalMacro(s0: string; s1: string)", (int)F_EXECEXTMACRO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SHOWTOOLBAR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ShowToolbar", (int)F_SHOWTOOLBAR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SHOWFUNCKEY + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ShowFunckey", (int)F_SHOWFUNCKEY).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SHOWTAB + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ShowTab", (int)F_SHOWTAB).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SHOWSTATUSBAR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ShowStatusbar", (int)F_SHOWSTATUSBAR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SHOWMINIMAP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ShowMiniMap", (int)F_SHOWMINIMAP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TYPE_LIST + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TypeList", (int)F_TYPE_LIST).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CHANGETYPE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ChangeType(i0: Integer)", (int)F_CHANGETYPE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_OPTION_TYPE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_OptionType", (int)F_OPTION_TYPE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_OPTION + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_OptionCommon", (int)F_OPTION).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FONT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SelectFont", (int)F_FONT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETFONTSIZE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SetFontSize(i0: Integer; i1: Integer; i2: Integer)", (int)F_SETFONTSIZE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WRAPWINDOWWIDTH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WrapWindowWidth", (int)F_WRAPWINDOWWIDTH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FAVORITE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_OptionFavorite", (int)F_FAVORITE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SET_QUOTESTRING + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SetMsgQuoteStr(s0: string)", (int)F_SET_QUOTESTRING).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TEXTWRAPMETHOD + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TextWrapMethod(i0: Integer)", (int)F_TEXTWRAPMETHOD).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SELECT_COUNT_MODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SelectCountMode(i0: Integer)", (int)F_SELECT_COUNT_MODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXECMD + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExecCommand(s0: string; i1: Integer; s2: string)", (int)F_EXECMD).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXECMD_DIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExecCommandDialog", (int)F_EXECMD_DIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MENU_RBUTTON + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_RMenu", (int)F_MENU_RBUTTON).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_1 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu1", (int)F_CUSTMENU_1).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_2 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu2", (int)F_CUSTMENU_2).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_3 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu3", (int)F_CUSTMENU_3).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_4 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu4", (int)F_CUSTMENU_4).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_5 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu5", (int)F_CUSTMENU_5).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_6 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu6", (int)F_CUSTMENU_6).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_7 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu7", (int)F_CUSTMENU_7).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_8 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu8", (int)F_CUSTMENU_8).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_9 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu9", (int)F_CUSTMENU_9).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_10 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu10", (int)F_CUSTMENU_10).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_11 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu11", (int)F_CUSTMENU_11).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_12 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu12", (int)F_CUSTMENU_12).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_13 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu13", (int)F_CUSTMENU_13).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_14 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu14", (int)F_CUSTMENU_14).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_15 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu15", (int)F_CUSTMENU_15).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_16 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu16", (int)F_CUSTMENU_16).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_17 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu17", (int)F_CUSTMENU_17).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_18 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu18", (int)F_CUSTMENU_18).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_19 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu19", (int)F_CUSTMENU_19).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_20 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu20", (int)F_CUSTMENU_20).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_21 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu21", (int)F_CUSTMENU_21).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_22 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu22", (int)F_CUSTMENU_22).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_23 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu23", (int)F_CUSTMENU_23).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CUSTMENU_24 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CustMenu24", (int)F_CUSTMENU_24).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SPLIT_V + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SplitWinV", (int)F_SPLIT_V).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SPLIT_H + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SplitWinH", (int)F_SPLIT_H).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SPLIT_VH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SplitWinVH", (int)F_SPLIT_VH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WINCLOSE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WinClose", (int)F_WINCLOSE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WIN_CLOSEALL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WinCloseAll", (int)F_WIN_CLOSEALL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CASCADE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CascadeWin", (int)F_CASCADE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TILE_V + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TileWinV", (int)F_TILE_V).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TILE_H + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TileWinH", (int)F_TILE_H).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_NEXTWINDOW + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_NextWindow", (int)F_NEXTWINDOW).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PREVWINDOW + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PrevWindow", (int)F_PREVWINDOW).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WINLIST + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WindowList", (int)F_WINLIST).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MAXIMIZE_V + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MaximizeV", (int)F_MAXIMIZE_V).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MAXIMIZE_H + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MaximizeH", (int)F_MAXIMIZE_H).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MINIMIZE_ALL + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MinimizeAll", (int)F_MINIMIZE_ALL).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_REDRAW + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ReDraw", (int)F_REDRAW).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WIN_OUTPUT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ActivateWinOutput", (int)F_WIN_OUTPUT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TRACEOUT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TraceOut(s0: string; i1: Integer)", (int)F_TRACEOUT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOPMOST + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_WindowTopMost(i0: Integer)", (int)F_TOPMOST).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GROUPCLOSE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_GroupClose", (int)F_GROUPCLOSE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_NEXTGROUP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_NextGroup", (int)F_NEXTGROUP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_PREVGROUP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_PrevGroup", (int)F_PREVGROUP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_MOVERIGHT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabMoveRight", (int)F_TAB_MOVERIGHT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_MOVELEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabMoveLeft", (int)F_TAB_MOVELEFT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_SEPARATE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabSeparate", (int)F_TAB_SEPARATE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_JOINTNEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabJointNext", (int)F_TAB_JOINTNEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_JOINTPREV + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabJointPrev", (int)F_TAB_JOINTPREV).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_CLOSEOTHER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabCloseOther", (int)F_TAB_CLOSEOTHER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_CLOSELEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabCloseLeft", (int)F_TAB_CLOSELEFT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TAB_CLOSERIGHT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_TabCloseRight", (int)F_TAB_CLOSERIGHT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HOKAN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_Complete", (int)F_HOKAN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_TOGGLE_KEY_SEARCH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ToggleKeyHelpSearch(i0: Integer)", (int)F_TOGGLE_KEY_SEARCH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HELP_CONTENTS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HelpContents", (int)F_HELP_CONTENTS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_HELP_SEARCH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_HelpSearch", (int)F_HELP_SEARCH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MENU_ALLFUNC + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CommandList", (int)F_MENU_ALLFUNC).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXTHELP1 + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExtHelp1", (int)F_EXTHELP1).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXTHTMLHELP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ExtHtmlHelp(s0: string; s1: string)", (int)F_EXTHTMLHELP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ABOUT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_About", (int)F_ABOUT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_STATUSMSG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_StatusMsg(s0: string; i1: Integer)", (int)F_STATUSMSG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MSGBEEP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_MsgBeep(i0: Integer)", (int)F_MSGBEEP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COMMITUNDOBUFFER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_CommitUndoBuffer", (int)F_COMMITUNDOBUFFER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ADDREFUNDOBUFFER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_AddRefUndoBuffer", (int)F_ADDREFUNDOBUFFER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETUNDOBUFFER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SetUndoBuffer", (int)F_SETUNDOBUFFER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_APPENDUNDOBUFFERCURSOR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_AppendUndoBufferCursor", (int)F_APPENDUNDOBUFFERCURSOR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CLIPBOARDEMPTY + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_ClipboardEmpty", (int)F_CLIPBOARDEMPTY).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETVIEWTOP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SetViewTop(i0: Integer)", (int)F_SETVIEWTOP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETVIEWLEFT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "procedure S_SetViewLeft(i0: Integer)", (int)F_SETVIEWLEFT).data());
}

/*!
	エラーハンドラのテスト

	引数に応じてメッセージボックスを表示する。
 */
TEST_F(CPpaTest, ErrorProc_funcCode_functions)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";

	setlocale(LC_ALL, "Japanese");

	// Err_CD > 0 の場合「サクラエディタのエラー」
	// 機能IDに対応する関数名をメッセージに含める
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETFILENAME + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetFilename: string", (int)F_GETFILENAME).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSAVEFILENAME + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetSaveFilename: string", (int)F_GETSAVEFILENAME).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSELECTED + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetSelectedString(i0: Integer): string", (int)F_GETSELECTED).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_EXPANDPARAMETER + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_ExpandParameter(s0: string): string", (int)F_EXPANDPARAMETER).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETLINESTR + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetLineStr(i0: Integer): string", (int)F_GETLINESTR).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETLINECOUNT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetLineCount(i0: Integer): Integer", (int)F_GETLINECOUNT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CHGTABWIDTH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_ChangeTabWidth(i0: Integer): Integer", (int)F_CHGTABWIDTH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISTEXTSELECTED + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsTextSelected: Integer", (int)F_ISTEXTSELECTED).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSELLINEFROM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetSelectLineFrom: Integer", (int)F_GETSELLINEFROM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSELCOLUMNFROM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetSelectColmFrom: Integer", (int)F_GETSELCOLUMNFROM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSELLINETO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetSelectLineTo: Integer", (int)F_GETSELLINETO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSELCOLUMNTO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetSelectColmTo: Integer", (int)F_GETSELCOLUMNTO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISINSMODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsInsMode: Integer", (int)F_ISINSMODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETCHARCODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetCharCode: Integer", (int)F_GETCHARCODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETLINECODE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetLineCode: Integer", (int)F_GETLINECODE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISPOSSIBLEUNDO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsPossibleUndo: Integer", (int)F_ISPOSSIBLEUNDO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISPOSSIBLEREDO + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsPossibleRedo: Integer", (int)F_ISPOSSIBLEREDO).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CHGWRAPCOLUMN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_ChangeWrapColm(i0: Integer): Integer", (int)F_CHGWRAPCOLUMN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISCURTYPEEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsCurTypeExt(s0: string): Integer", (int)F_ISCURTYPEEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISSAMETYPEEXT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsSameTypeExt(s0: string; s1: string): Integer", (int)F_ISSAMETYPEEXT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INPUTBOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_InputBox(s0: string; s1: string; i2: Integer): string", (int)F_INPUTBOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MESSAGEBOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_MessageBox(s0: string; i1: Integer): Integer", (int)F_MESSAGEBOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ERRORMSG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_ErrorMsg(s0: string): Integer", (int)F_ERRORMSG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_WARNMSG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_WarnMsg(s0: string): Integer", (int)F_WARNMSG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_INFOMSG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_InfoMsg(s0: string): Integer", (int)F_INFOMSG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_OKCANCELBOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_OkCancelBox(s0: string): Integer", (int)F_OKCANCELBOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_YESNOBOX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_YesNoBox(s0: string): Integer", (int)F_YESNOBOX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_COMPAREVERSION + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_CompareVersion(s0: string; s1: string): Integer", (int)F_COMPAREVERSION).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_MACROSLEEP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_Sleep(i0: Integer): Integer", (int)F_MACROSLEEP).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILEOPENDIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_FileOpenDialog(s0: string; s1: string): string", (int)F_FILEOPENDIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FILESAVEDIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_FileSaveDialog(s0: string; s1: string): string", (int)F_FILESAVEDIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_FOLDERDIALOG + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_FolderDialog(s0: string; s1: string): string", (int)F_FOLDERDIALOG).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETCLIPBOARD + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetClipboard(i0: Integer): string", (int)F_GETCLIPBOARD).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETCLIPBOARD + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_SetClipboard(i0: Integer; s1: string): Integer", (int)F_SETCLIPBOARD).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LAYOUTTOLOGICLINENUM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_LayoutToLogicLineNum(i0: Integer): Integer", (int)F_LAYOUTTOLOGICLINENUM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LOGICTOLAYOUTLINENUM + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_LogicToLayoutLineNum(i0: Integer; i1: Integer): Integer", (int)F_LOGICTOLAYOUTLINENUM).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LINECOLUMNTOINDEX + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_LineColumnToIndex(i0: Integer; i1: Integer): Integer", (int)F_LINECOLUMNTOINDEX).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_LINEINDEXTOCOLUMN + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_LineIndexToColumn(i0: Integer; i1: Integer): Integer", (int)F_LINEINDEXTOCOLUMN).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETCOOKIE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetCookie(s0: string; s1: string): string", (int)F_GETCOOKIE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETCOOKIEDEFAULT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetCookieDefault(s0: string; s1: string; s2: string): string", (int)F_GETCOOKIEDEFAULT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETCOOKIE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_SetCookie(s0: string; s1: string; s2: string): Integer", (int)F_SETCOOKIE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_DELETECOOKIE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_DeleteCookie(s0: string; s1: string): Integer", (int)F_DELETECOOKIE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETCOOKIENAMES + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetCookieNames(s0: string): string", (int)F_GETCOOKIENAMES).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETDRAWSWITCH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_SetDrawSwitch(i0: Integer): Integer", (int)F_SETDRAWSWITCH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETDRAWSWITCH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetDrawSwitch: Integer", (int)F_GETDRAWSWITCH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISSHOWNSTATUS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsShownStatus: Integer", (int)F_ISSHOWNSTATUS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSTRWIDTH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetStrWidth(s0: string; i1: Integer): Integer", (int)F_GETSTRWIDTH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETSTRLAYOUTLENGTH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetStrLayoutLength(s0: string; i1: Integer): Integer", (int)F_GETSTRLAYOUTLENGTH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETDEFAULTCHARLENGTH + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetDefaultCharLength: Integer", (int)F_GETDEFAULTCHARLENGTH).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISINCLUDECLIPBOARDFORMAT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsIncludeClipboardFormat(s0: string): Integer", (int)F_ISINCLUDECLIPBOARDFORMAT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETCLIPBOARDBYFORMAT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetClipboardByFormat(s0: string; i1: Integer; i2: Integer): string", (int)F_GETCLIPBOARDBYFORMAT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_SETCLIPBOARDBYFORMAT + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_SetClipboardByFormat(s0: string; s1: string; i2: Integer; i3: Integer): Integer", (int)F_SETCLIPBOARDBYFORMAT).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETLINEATTRIBUTE + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetLineAttribute(i0: Integer; i1: Integer): Integer", (int)F_GETLINEATTRIBUTE).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_ISTEXTSELECTINGLOCK + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_IsTextSelectingLock: Integer", (int)F_ISTEXTSELECTINGLOCK).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETVIEWLINES + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetViewLines: Integer", (int)F_GETVIEWLINES).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETVIEWCOLUMNS + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetViewColumns: Integer", (int)F_GETVIEWCOLUMNS).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_CREATEMENU + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_CreateMenu(i0: Integer; s1: string): Integer", (int)F_CREATEMENU).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(F_GETVIEWTOP + 1, msg.data()), strprintf(L"関数の実行エラー\n%hs; index %d;", "function S_GetViewTop: Integer", (int)F_GETVIEWTOP).data());
}

/*!
	エラーハンドラのテスト

	引数に応じてメッセージボックスを表示する。
 */
TEST_F(CPpaTest, ErrorProc_funcCode_unknown)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";

	setlocale(LC_ALL, "Japanese");

	// Err_CD > 0 の場合「サクラエディタのエラー」
	// 機能IDに対応する関数名をメッセージに含める
	EXPECT_ERROUT(cPpa.CallErrorProc(55555 + 1, msg.data()), strprintf(L"不明な関数の実行エラー(バグです)\nFunc_ID=%d", 55555).data());
}

/*!
	エラーハンドラのテスト

	引数に応じてメッセージボックスを表示する。
 */
TEST_F(CPpaTest, ErrorProc_PPA_errors)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";
	constexpr char chNul = '\0';

	setlocale(LC_ALL, "Japanese");

	// Err_CD == 0 の場合「PPAのエラー」
	EXPECT_ERROUT(cPpa.CallErrorProc(0, msg.data()), strprintf(L"%hs", msg.data()).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(0, nullptr), L"エラー情報が不正");
	EXPECT_ERROUT(cPpa.CallErrorProc(0, &chNul), L"詳細不明のエラー");
}

/*!
	エラーハンドラのテスト

	引数に応じてメッセージボックスを表示する。
 */
TEST_F(CPpaTest, ErrorProc_user_errors)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";
	constexpr char chNul = '\0';

	setlocale(LC_ALL, "Japanese");

	// Err_CD < 0 の場合「ユーザー定義のエラー」
	EXPECT_ERROUT(cPpa.CallErrorProc(-1, msg.data()), strprintf(L"未定義のエラー\nError_CD=%d\n%hs", -1, msg.data()).data());
	EXPECT_ERROUT(cPpa.CallErrorProc(-1, nullptr), L"エラー情報が不正");
	EXPECT_ERROUT(cPpa.CallErrorProc(-1, &chNul), L"未定義のエラー\nError_CD=-1\n");
}

/*!
	エラーハンドラのテスト

	引数に応じてメッセージボックスを表示する。
 */
TEST_F(CPpaTest, ErrorProc_withDebugStr)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	std::string_view msg = "something is wrong.";
	std::string_view debugStr = "でばっぐ";

	setlocale(LC_ALL, "Japanese");

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	execInfo.m_cMemDebug = debugStr.data();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	// Err_CD == 0 の場合「PPAのエラー」
	EXPECT_ERROUT(cPpa.CallDllProc<decltype(&ppaErrorProc)>("CallErrorProc", 0, msg.data()), strprintf(L"%hs\n%hs", msg.data(), debugStr.data()).data());

	// 2回目の呼出に対するメッセージボックスは表示されない
	EXPECT_NOERROUT(cPpa.CallDllProc<decltype(&ppaErrorProc)>("CallErrorProc", 0, msg.data()));
}

/*!
	終了ハンドラのテスト
 */
TEST_F(CPpaTest, FinishProc)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;

	// PPAの実行コンテキストを用意する
	PpaExecInfo execInfo;
	execInfo.m_pcEditView = GetActiveView();
	const auto execInfoHolder = CPPA::RegisterExecInfo(execInfo);

	// 何もしないハンドラなので呼ぶだけ。
	cPpa.CallFinishProc();
}
