/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "macro/CPPA.h"

#include "doc/CEditDoc.h"
#include "env/CShareData.h"
#include "macro/CSMacroMgr.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"

#include "eval_outputs.hpp"

#include "env/ShareDataTestSuite.hpp"
#include "macro/CMacroFactory.h"
#include "macro/CPPAMacroMgr.h"

#include <fstream>

std::filesystem::path GetTempFilePathWithExt(std::wstring_view prefix, std::wstring_view extension);

namespace macro {

struct CPpaStub : public CPPA
{
	CPpaStub()
	{
		EXPECT_THAT(InitDll(L"ppa_stub.dll"), Eq(EDllResult::DLL_SUCCESS));
	}
};

struct CPpaTest : public ::testing::Test, public env::ShareDataTestSuite {
	static inline std::unique_ptr<CEditDoc> pcEditDoc = nullptr;
	static inline std::unique_ptr<CEditWnd> pcEditWnd = nullptr;
	static inline std::unique_ptr<CSMacroMgr> pcSMacroMgr = nullptr;

	static inline CPPA::PpaExecInfo info{};

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpShareData();

		// CanBeMoveリージョンをテストケースに分割する。（すぐ対応できないのでコメント残し）

		// ドキュメントの初期化前に文字幅キャッシュの生成が必要
		SelectCharWidthCache(CWM_FONT_EDIT, CWM_CACHE_SHARE);
		InitCharWidthCache(GetDllShareData().m_Common.m_sView.m_lf);

#pragma region CanBeMove
		// ドキュメントがなくてもエラーにならない
		EXPECT_THAT(GetDocument(), IsNull());

		// ドキュメントがないのでエラー
		EXPECT_ANY_THROW(GetEditDoc());

#pragma endregion CanBeMove

		// CEditViewをインスタンス化するにはドキュメントのインスタンスが必要
		pcEditDoc = std::make_unique<CEditDoc>(nullptr);

#pragma region CanBeMove
		// ドキュメントがあるので値を返す
		EXPECT_THAT(GetDocument(), pcEditDoc.get());

		// ドキュメントがあるのでエラーにならない
		EXPECT_NO_THROW([] { GetEditDoc(); });

		EXPECT_THAT(&GetEditDoc(), GetDocument());

		// 編集ウインドウがなくてもエラーにならない
		EXPECT_THAT(GetEditWndPtr(), IsNull());

		// 編集ウインドウがないのでエラー
		EXPECT_ANY_THROW(GetEditWnd());

#pragma endregion CanBeMove

		// CEditWndを用意する
		pcEditWnd = std::make_unique<CEditWnd>();

		// SMacroMgrを用意する
		pcSMacroMgr = std::make_unique<CSMacroMgr>();

#pragma region CanBeMove
		// 編集ウインドウがあるので値を返す
		EXPECT_THAT(GetEditWndPtr(), pcEditWnd.get());

		// 編集ウインドウがあるのでエラーにならない
		EXPECT_NO_THROW([] { GetEditWnd(); });

#pragma endregion CanBeMove

		// PPA実行情報を初期化する
		info.m_pShareData = &GetDllShareData();
		info.m_pcEditView = &pcEditWnd->GetActiveView();
		info.m_bError = false;
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		pcSMacroMgr = nullptr;

		pcEditWnd = nullptr;

		pcEditDoc = nullptr;

		TearDownShareData();
	}
};

/*!
	CPPA::GetDllNameImpのテスト
 */
TEST_F(CPpaTest, GetDllNameImp)
{
	CPPA cPpa;
	EXPECT_THAT(cPpa.GetDllNameImp(0), StrEq(L"PPA.DLL"));
}

/*!
	CPPA::GetVersionのテスト

	アプリから使っているわけではないので、削除しても良いかも知れない。
 */
TEST_F(CPpaTest, GetVersion)
{
	// テスト対象はスタブDLL
	CPpaStub cPpa;
	EXPECT_THAT(cPpa.GetVersion(), StrEq("PPA.DLL Version 1.24"));

	cPpa.DeinitDll();
	EXPECT_THAT(cPpa.GetVersion(), StrEq(""));
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
	EXPECT_THAT(cPpa.IsAvailable(), IsTrue());

	// Execute呼出は成功する
	EXPECT_TRUE(cPpa.Execute(&pcEditWnd->GetActiveView(), 0));
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
	setlocale(LC_ALL, "Japanese");

	// 引数型 int
	MacroFuncInfo funcInfo1 = { 1, L"Cmd1",  { VT_I4,   }, VT_EMPTY };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo1), StrEq("procedure S_Cmd1(i0: Integer); index 1;"));

	// 引数型 string
	MacroFuncInfo funcInfo2 = { 2, L"Cmd2",  { VT_BSTR, }, VT_EMPTY };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo2), StrEq("procedure S_Cmd2(s0: string); index 2;"));

	// 引数型 なし
	MacroFuncInfo funcInfo3 = { 3, L"Cmd3",  { VT_EMPTY, }, VT_EMPTY };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo3), StrEq("procedure S_Cmd3; index 3;"));

	// 引数型 不明（intでもstringでもない）
	MacroFuncInfo funcInfo4 = { 4, L"Cmd4",  { VT_BOOL, }, VT_EMPTY };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo4), StrEq("procedure S_Cmd4(u0: Unknown); index 4;"));

	// 戻り型 int
	MacroFuncInfo funcInfo5 = { 5, L"Func1", { VT_EMPTY }, VT_I4 };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo5), StrEq("function S_Func1: Integer; index 5;"));

	// 戻り型 string
	MacroFuncInfo funcInfo6 = { 6, L"Func2", { VT_EMPTY }, VT_BSTR };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo6), StrEq("function S_Func2: string; index 6;"));

	// 戻り型 不明（intでもstringでもない）
	MacroFuncInfo funcInfo7 = { 7, L"Func3", { VT_EMPTY }, VT_BOOL };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo7), StrEq("function S_Func3; index 7;"));

	// 引数をたくさん指定する
	VARTYPE varArgEx8[] = { VT_I4, VT_BSTR, VT_I4, VT_BSTR };
	MacroFuncInfoEx funcInfoEx8 = { 8, 8, varArgEx8 };
	MacroFuncInfo funcInfo8 = { 8, L"Func4", { VT_I4, VT_BSTR, VT_I4, VT_BSTR }, VT_BSTR, &funcInfoEx8 };
	EXPECT_THAT(CPPA::GetDeclarations(funcInfo8), StrEq("function S_Func4(i0: Integer; s1: string; i2: Integer; s3: string; i4: Integer; s5: string; i6: Integer; s7: string): string; index 8;"));
}

/*!
 * CPPAエラー情報コールバックのテスト
 *
 * 実装が想定するメッセージを出力できるかチェックする
 * 本来は確認ケースを分割すべきだが、初期化に手間がかかるため1つにまとめている
 */
TEST_F(CPpaTest, ppaErrorProc)
{
	// 既にエラーフラグが立っていたらメッセージは出さない
	info.m_bError = true;
	CPPA::CallErrorProc(info, int(F_FILENEW) + 1, nullptr);

	// コマンドエラー
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, int(F_FILENEW) + 1, nullptr), L"関数の実行エラー\nprocedure S_FileNew; index 30101;");

	// 関数エラー
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, int(F_GETFILENAME) + 1, nullptr), L"関数の実行エラー\nfunction S_GetFilename: string; index 40001;");

	// 不明な関数エラー
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, 1 + 1, nullptr), L"不明な関数の実行エラー(バグです)\nFunc_ID=1");

	// エラー情報が不正
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, 0, nullptr), L"エラー情報が不正");

	// 詳細不明のPPAエラー
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, 0, ""), L"詳細不明のエラー");

	// 詳細ありのPPAエラー
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, 0, "test"), L"test");

	// 未定義のエラー
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, -1, "test"), L"未定義のエラー\nError_CD=-1\ntest");

	// デバッグ情報付きのエラー
	info.m_cMemDebug = "debug";
	info.m_bError = false;
	EXPECT_ERROUT(CPPA::CallErrorProc(info, 0, nullptr), L"エラー情報が不正\ndebug");
}

/*!
 * CPPAマクロ呼出コールバックのテスト
 *
 * 実装が想定するメッセージを出力できるかチェックする
 * 本来は確認ケースを分割すべきだが、初期化に手間がかかるため1つにまとめている
 */
TEST_F(CPpaTest, ppaProc)
{
	std::array ppszArgs1 = { "something is wrong." };
	std::array ppszArgs2 = { LPCSTR(nullptr) };

	// 正常なマクロ呼出
	EXPECT_THAT(CPPA::CallProc(info, F_OKCANCELBOX, ppszArgs1), Eq(0));

	// パラメーター不足でエラー
	EXPECT_THAT(CPPA::CallProc(info, F_WCHAR, ppszArgs2), Eq(int(F_WCHAR) + 1));
}

/*!
 * CPPAマクロ関数呼出コールバックのテスト
 *
 * 実装が想定するメッセージを出力できるかチェックする
 * 本来は確認ケースを分割すべきだが、初期化に手間がかかるため1つにまとめている
 */
TEST_F(CPpaTest, ppaIntFunc)
{
	std::array ppszArgs1 = { "$I" };
	std::array ppszArgs2 = { LPCSTR(nullptr) };
	std::array ppszArgs3 = { "0" };
	std::array ppszArgs4 = { "1" };

	// 数値を返すマクロ関数を呼び出す
	int nValue = -1;
	EXPECT_THAT(CPPA::CallIntFunc(info, F_ISINSMODE, ppszArgs2, &nValue), Eq(0));

	EXPECT_THAT(CPPA::CallIntFunc(info, F_GETLINECOUNT, ppszArgs3, &nValue), Eq(0));

	// パラメーター不足でfalseを返すケース
	EXPECT_THAT(CPPA::CallIntFunc(info, F_GETLINECOUNT, ppszArgs4, &nValue), Eq(int(F_GETLINECOUNT) + 1));

	// 文字列を返すマクロ関数を呼び出す
	EXPECT_THAT(CPPA::CallIntFunc(info, F_EXPANDPARAMETER, ppszArgs1, &nValue), Eq(-2));

	// コマンドを呼び出す
	EXPECT_THAT(CPPA::CallIntFunc(info, F_FILENEW, ppszArgs2, &nValue), Eq(int(F_FILENEW) + 1));
}

/*!
 * CPPAマクロ関数呼出コールバックのテスト
 *
 * 実装が想定するメッセージを出力できるかチェックする
 * 本来は確認ケースを分割すべきだが、初期化に手間がかかるため1つにまとめている
 */
TEST_F(CPpaTest, ppaStrFunc)
{
	std::array ppszArgs1 = { "$I" };
	std::array ppszArgs2 = { LPCSTR(nullptr) };
	std::array ppszArgs3 = { "0" };

	// 文字列を返すマクロ関数を呼び出す
	LPSTR pszValue = nullptr;
	EXPECT_THAT(CPPA::CallStrFunc(info, F_EXPANDPARAMETER, ppszArgs1, &pszValue), Eq(0));

	// 数値を返すマクロ関数を呼び出す
	EXPECT_THAT(CPPA::CallStrFunc(info, F_ISINSMODE, ppszArgs2, &pszValue), Eq(int(F_ISINSMODE) + 1));
	EXPECT_THAT(pszValue, StrEq(""));

	// コマンドを呼び出す
	pszValue = nullptr;
	EXPECT_THAT(CPPA::CallStrFunc(info, F_FILENEW, ppszArgs2, &pszValue), Eq(int(F_FILENEW) + 1));
	EXPECT_THAT(pszValue, StrEq(""));
}

/*!
 * CPPAユーザー定義文字列コールバックのテスト
 *
 * 実装が想定するメッセージを出力できるかチェックする
 * 本来は確認ケースを分割すべきだが、初期化に手間がかかるため1つにまとめている
 */
TEST_F(CPpaTest, ppaStrObj)
{
	LPSTR pszValue = nullptr;
	EXPECT_THAT(CPPA::CallStrObj(info, 2, false, &pszValue), Eq(0));
	EXPECT_THAT(pszValue, StrEq("debug"));

	std::string test{ "test" };
	pszValue = test.data();
	EXPECT_THAT(CPPA::CallStrObj(info, 2, true, &pszValue), Eq(0));

	pszValue = nullptr;
	EXPECT_THAT(CPPA::CallStrObj(info, 2, false, &pszValue), Eq(0));
	EXPECT_THAT(pszValue, StrEq(test));

	EXPECT_THAT(CPPA::CallStrObj(info, 0, true, &pszValue), Eq(-1));
}

/*!
 * CPPAマクロマネージャーのテスト
 */
TEST_F(CPpaTest, CPPAMacroMgr001)
{
	EXPECT_THAT(CPPAMacroMgr::Creator(L"mac"), IsNull());

	// スタブDLLを読み込む
	CPPAMacroMgr::m_cPPA.InitDll(L"ppa_stub.dll");

	CPPAMacroMgr::declare();

	auto mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"ppa"));

	const HINSTANCE unusedArg1 = nullptr;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"macro str;"), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	const auto path = GetTempFilePathWithExt(L"tes", L"ppa");
	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	std::wofstream fs(path);
	fs << L"macro str1;" << std::endl;
	fs << L"macro str2;" << std::endl;
	fs << L"macro str3;" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsTrue());

	std::filesystem::remove(path);

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	mgr = nullptr;

	CMacroFactory::getInstance()->Unregister(CPPAMacroMgr::Creator);
}

TEST(CSMacroMgr, GetFuncInfoByID001)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByID(F_FILENEW), NotNull());
}

TEST(CSMacroMgr, GetFuncInfoByID101)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByID(F_INVALID), IsNull());
}

TEST(CSMacroMgr, GetFuncInfoByID102)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByID(F_0), IsNull());
}

TEST(CSMacroMgr, GetFuncInfoByName001)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByName(nullptr, L"S_FileNew", nullptr), F_FILENEW);
}

TEST(CSMacroMgr, GetFuncInfoByName002)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByName(nullptr, L"ChangeInsertSpace", nullptr), F_CHGINSSPACE);
}

TEST(CSMacroMgr, GetFuncInfoByName101)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByName(nullptr, nullptr, nullptr), F_INVALID);
}

} // namespace macro
