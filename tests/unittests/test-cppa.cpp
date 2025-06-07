/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "macro/CPPA.h"

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
