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

/*!
	CPPA::GetDllNameImpのテスト
 */
TEST(CPPA, GetDllNameImp)
{
	CPPA cPpa;
	EXPECT_STREQ(L"PPA.DLL", cPpa.GetDllNameImp(0));
}

/*!
	CPPA::GetDeclarationsのテスト

	PPAに渡す関数名を作る関数
	戻り値があればfunction、なければprocedureとみなす。
	対応する型はintとstringのみ。（PPA1.2は実数型にも対応しているが未対応）
	引数は最大8個まで指定できる。(PPAは32個まで対応しているが未対応）
 */
TEST(CPPA, GetDeclarations)
{
	setlocale(LC_ALL, "Japanese");

	CPPA cPpa;

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
 * CPPAエラー情報コールバックのテスト
 *
 * 実装が想定するメッセージを出力できるかチェックする
 * 本来は確認ケースを分割すべきだが、初期化に手間がかかるため1つにまとめている
 */
TEST(CPPA, ppaErrorProc)
{
	// CanBeMoveリージョンをテストケースに分割する。（すぐ対応できないのでコメント残し）

#pragma region CanBeMove
	// 共有データがなくてもエラーにならない
	EXPECT_THAT(GetDllShareDataPtr(), IsNull());

	// 共有データがないのでエラー
	EXPECT_ANY_THROW(GetDllShareData());

#pragma endregion CanBeMove

	// 共有メモリのインスタンスを生成する
	const auto pcShareData = std::make_unique<CShareData>();

#pragma region CanBeMove
	// 共有データがなくてもエラーにならない
	EXPECT_THAT(GetDllShareDataPtr(), IsNull());

	// 共有データがないのでエラー
	EXPECT_ANY_THROW(GetDllShareData());

	// 共有データがないのでエラー
	EXPECT_ANY_THROW(CDialog(false, true));	//2つ目の引数がtrueなら共有データ必須

	// 共有データがなくてもエラーにならない
	EXPECT_NO_THROW([] { CDialog(false, false).GetHwnd(); });

#pragma endregion CanBeMove

	// 共有メモリを初期化する
	pcShareData->InitShareData();

#pragma region CanBeMove
	// 共有データがあるので値を返す
	EXPECT_THAT(GetDllShareDataPtr(), pcShareData->GetDllShareDataPtr());

	// 共有データがあるのでエラーにならない
	EXPECT_NO_THROW([] { GetDllShareData(); });

	// 共有データがあるのでエラーにならない
	EXPECT_NO_THROW([] { CDialog(false, true).GetHwnd(); });

#pragma endregion CanBeMove

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
	const auto pcEditDoc = std::make_unique<CEditDoc>(nullptr);

#pragma region CanBeMove
	// ドキュメントがあるので値を返す
	EXPECT_THAT(GetDocument(), pcEditDoc.get());

	// ドキュメントがあるのでエラーにならない
	EXPECT_NO_THROW([] { GetEditDoc(); });

	EXPECT_THAT(&GetEditDoc(), GetDocument());

#pragma endregion CanBeMove

	// CEditWndを用意する
	const auto pcEditWnd = std::make_unique<CEditWnd>();

	// SMacroMgrを用意する
	const auto pcSMacroMgr = std::make_unique<CSMacroMgr>();

	// PPA実行情報を用意する
	CPPA::PpaExecInfo info{};
	info.m_pShareData = &GetDllShareData();
	info.m_pcEditView = &pcEditWnd->GetActiveView();

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
