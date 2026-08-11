/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "util/MessageBoxF.h"

#include "eval_outputs.hpp"

/* static */ int MockUser32::_MessageBoxExW(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ UINT uType,
	_In_ WORD wLanguageId
)
{
	// lpText を標準エラー出力に書き出す
	std::clog << (lpText ? cxx::to_string(lpText, CP_UTF8) : "") << std::endl;

	// いい加減な戻り値を返す。(返り値0は未定義なので本来返らない値を返している)
	return 0;
}

MockUser32::MockUser32()
{
	// デフォルトの動作を設定する
	ON_CALL(*this, MessageBoxExW(_, _, _, _, _)).WillByDefault(&_MessageBoxExW);
}

/*!
	MessageBoxFのテスト 
 */
TEST(MessageBoxF, test)
{
	User32::setInstance<MockUser32>();
	auto pUser32 = (MockUser32*)User32::getInstance();

	const HWND hWnd = nullptr;
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	MessageBoxF(hWnd, MB_OK, L"caption", L"%d行をマージしました。", 2);

	User32::resetInstance();
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST(MessageBoxF, customMessageBoxFunctions)
{
	User32::setInstance<MockUser32>();
	auto pUser32 = (MockUser32*)User32::getInstance();

	const HWND hWnd = nullptr;

	//エラー：赤丸に「×」[OK]
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	ErrorMessage(hWnd, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopErrorMessage(hWnd, L"%d行をマージしました。", 2);

	//警告：三角に「！」[OK]
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	WarningMessage(hWnd, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopWarningMessage(hWnd, L"%d行をマージしました。", 2);

	//情報：青丸に「i」[OK]
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	InfoMessage(hWnd, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopInfoMessage(hWnd, L"%d行をマージしました。", 2);

	//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	ConfirmMessage(hWnd, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopConfirmMessage(hWnd, L"%d行をマージしました。", 2);

	//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	Select3Message(hWnd, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopSelect3Message(hWnd, L"%d行をマージしました。", 2);

	//その他メッセージ表示用ボックス[OK]
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	OkMessage(hWnd, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopOkMessage(hWnd, L"%d行をマージしました。", 2);

	//タイプ指定メッセージ表示用ボックス
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	CustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2);
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	TopCustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2);

	//作者に教えて欲しいエラー
	EXPECT_CALL(*pUser32, MessageBoxExW(_, StrEq(L"2行をマージしました。"), _, _, _)).WillOnce(Return(MB_OK));
	PleaseReportToAuthor(hWnd, L"%d行をマージしました。", 2);

	User32::resetInstance();
}
