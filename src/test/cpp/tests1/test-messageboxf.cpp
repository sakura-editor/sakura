/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "eval_outputs.hpp"

#include "util/MessageBoxF.h"

#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"

/*!
	MessageBoxFのテスト 
 */
TEST(MessageBoxF, test)
{
	const HWND hWnd = nullptr;
	EXPECT_ERROUT(MessageBoxF(hWnd, MB_OK, L"caption", L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST(MessageBoxF, customMessageBoxFunctions)
{
	const HWND hWnd = nullptr;

	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	//エラー：赤丸に「×」[OK]
	EXPECT_ERROUT(ErrorMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopErrorMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//警告：三角に「！」[OK]
	EXPECT_ERROUT(WarningMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopWarningMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//情報：青丸に「i」[OK]
	EXPECT_ERROUT(InfoMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopInfoMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
	EXPECT_ERROUT(ConfirmMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopConfirmMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
	EXPECT_ERROUT(Select3Message(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopSelect3Message(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//その他メッセージ表示用ボックス[OK]
	EXPECT_ERROUT(OkMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopOkMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//タイプ指定メッセージ表示用ボックス
	EXPECT_ERROUT(CustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2), L"2行をマージしました。");
	EXPECT_ERROUT(TopCustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2), L"2行をマージしました。");

	//作者に教えて欲しいエラー
	EXPECT_ERROUT(PleaseReportToAuthor(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}
