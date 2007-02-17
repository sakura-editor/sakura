/*!	@file
	@brief マクロ

	@author Norio Nakatani
	@author genta
	@date Sep. 29, 2001 作成
	@date 20011229 aroka バグ修正、コメント追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, aroka, MIK, asa-o, hor, Misaka, jepro, Stonee
	Copyright (C) 2002, YAZAKI, MIK, aroka, hor, genta, ai
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, FILE
	Copyright (C) 2006, かろと, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CSMacroMgr.h"
#include "CEditView.h"
#include "CPPAMacroMgr.h"
#include "CWSHManager.h"
#include "CMacroFactory.h"
#include <stdio.h>
#include <assert.h> // 試験用
#include "debug.h"
#include "CRunningTimer.h"

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoNotCommandArr[] = 
{
	{F_GETFILENAME,	"GetFilename",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL}, //ファイル名を返す
	//	Oct. 19, 2002 genta
	{F_GETSELECTED,	"GetSelectedString",	{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL}, //選択部分
	{F_EXPANDPARAMETER,	"ExpandParameter",	{VT_BSTR, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL}, //特殊文字の展開
	{F_GETLINESTR, "GetLineStr", {VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_BSTR, NULL }, // 指定論理行の取得 2003.06.01 Moca
	{F_GETLINECOUNT, "GetLineCount", {VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 全論理行数の取得 2003.06.01 Moca
	{F_CHGTABWIDTH,			"ChangeTabWidth",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},  VT_I4, NULL }, //タブサイズ変更 2004.03.16 zenryaku
	{F_ISTEXTSELECTED,		"IsTextSelected",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, //テキストが選択されているか 2005.7.30 maru
	{F_GETSELLINEFROM,		"GetSelectLineFrom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 選択開始行の取得 2005.7.30 maru
	{F_GETSELCOLMFROM,		"GetSelectColmFrom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 選択開始桁の取得 2005.7.30 maru
	{F_GETSELLINETO,		"GetSelectLineTo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 選択終了行の取得 2005.7.30 maru
	{F_GETSELCOLMTO,		"GetSelectColmTo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 選択終了桁の取得 2005.7.30 maru
	{F_ISINSMODE,			"IsInsMode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 挿入／上書きモードの取得 2005.7.30 maru
	{F_GETCHARCODE,			"GetCharCode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 文字コード取得 2005.07.31 maru
	{F_GETLINECODE,			"GetLineCode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // 改行コード取得 2005.08.05 maru
	{F_ISPOSSIBLEUNDO,		"IsPossibleUndo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // Undo可能か調べる 2005.08.05 maru
	{F_ISPOSSIBLEREDO,		"IsPossibleRedo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // Redo可能か調べる 2005.08.05 maru
	
	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{-1,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
//	機能番号			関数名			引数				作業用バッファ

	/* ファイル操作系 */
	{F_FILENEW,						"FileNew",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //新規作成
	{F_FILEOPEN,					"FileOpen",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //開く
	{F_FILESAVE,					"FileSave",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上書き保存
	{F_FILESAVEALL,					"FileSaveAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上書き保存
	{F_FILESAVEAS,					"FileSaveAs",			{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //名前を付けて保存
	{F_FILECLOSE,					"FileClose",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	{F_FILECLOSE_OPEN,				"FileCloseOpen",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //閉じて開く
	{F_FILE_REOPEN,					"FileReopen",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJISで開き直す	//Dec. 4, 2002 genta
	{F_FILE_REOPEN_SJIS,			"FileReopenSJIS",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJISで開き直す
	{F_FILE_REOPEN_JIS,				"FileReopenJIS",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //JISで開き直す
	{F_FILE_REOPEN_EUC,				"FileReopenEUC",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUCで開き直す
	{F_FILE_REOPEN_UNICODE,			"FileReopenUNICODE",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicodeで開き直す
	{F_FILE_REOPEN_UNICODEBE,		"FileReopenUNICODEBE",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UnicodeBEで開き直す
	{F_FILE_REOPEN_UTF8,			"FileReopenUTF8",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-8で開き直す
	{F_FILE_REOPEN_UTF7,			"FileReopenUTF7",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-7で開き直す
	{F_PRINT,						"Print",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷
//	{F_PRINT_DIALOG,				"PrintDialog",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷ダイアログ
	{F_PRINT_PREVIEW,				"PrintPreview",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷プレビュー
	{F_PRINT_PAGESETUP,				"PrintPageSetup",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	{F_OPEN_HfromtoC,				"OpenHfromtoC",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	{F_OPEN_HHPP,					"OpenHHpp",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	{F_OPEN_CCPP,					"OpenCCpp",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	{F_ACTIVATE_SQLPLUS,			"ActivateSQLPLUS",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plusをアクティブ表示 */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	"ExecSQLPLUS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plusで実行 */
	{F_BROWSE,						"Browse",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブラウズ
	{F_READONLY,					"ReadOnly",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //読み取り専用
	{F_PROPERTY_FILE,				"PropertyFile",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイルのプロパティ
	{F_EXITALLEDITORS,				"ExitAllEditors",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //編集の全終了	// 2007.02.13 ryoji 追加
	{F_EXITALL,						"ExitAll",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加

	/* 編集系 */
	{F_CHAR,				"Char",					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //文字入力
	{F_IME_CHAR,			"CharIme",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //全角文字入力
	{F_UNDO,				"Undo",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //元に戻す(Undo)
	{F_REDO,				"Redo",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //やり直し(Redo)
	{F_DELETE,				"Delete",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //削除
	{F_DELETE_BACK,			"DeleteBack",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル前を削除
	{F_WordDeleteToStart,	"WordDeleteToStart",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の左端まで削除
	{F_WordDeleteToEnd,		"WordDeleteToEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の右端まで削除
	{F_WordCut,				"WordCut",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語切り取り
	{F_WordDelete,			"WordDelete",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語削除
	{F_LineCutToStart,		"LineCutToStart",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行頭まで切り取り(改行単位)
	{F_LineCutToEnd,		"LineCutToEnd",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行末まで切り取り(改行単位)
	{F_LineDeleteToStart,	"LineDeleteToStart",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行頭まで削除(改行単位)
	{F_LineDeleteToEnd,		"LineDeleteToEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行末まで削除(改行単位)
	{F_CUT_LINE,			"CutLine",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行切り取り(折り返し単位)
	{F_DELETE_LINE,			"DeleteLine",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行削除(折り返し単位)
	{F_DUPLICATELINE,		"DuplicateLine",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行の二重化(折り返し単位)
	{F_INDENT_TAB,			"IndentTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //TABインデント
	{F_UNINDENT_TAB,		"UnindentTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //逆TABインデント
	{F_INDENT_SPACE,		"IndentSpace",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SPACEインデント
	{F_UNINDENT_SPACE,		"UnindentSpace",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //逆SPACEインデント
//	{F_WORDSREFERENCE,		"WordReference",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語リファレンス
	{F_LTRIM,				"LTrim",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左(先頭)の空白を削除 2001.12.03 hor
	{F_RTRIM,				"RTrim",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //右(末尾)の空白を削除 2001.12.03 hor
	{F_SORT_ASC,			"SortAsc",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択行の昇順ソート 2001.12.06 hor
	{F_SORT_DESC,			"SortDesc",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択行の降順ソート 2001.12.06 hor
	{F_MERGE,				"Merge",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択行のマージ 2001.12.06 hor

	/* カーソル移動系 */
	{F_UP,					"Up",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル上移動
	{F_DOWN,				"Down",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル下移動
	{F_LEFT,				"Left",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル左移動
	{F_RIGHT,				"Right",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル右移動
	{F_UP2,					"Up2",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル上移動(２行ごと)
	{F_DOWN2,				"Down2",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル下移動(２行ごと)
	{F_WORDLEFT,			"WordLeft",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の左端に移動
	{F_WORDRIGHT,			"WordRight",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の右端に移動
	{F_GOLINETOP,			"GoLineTop",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行頭に移動(折り返し単位)
	{F_GOLINEEND,			"GoLineEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行末に移動(折り返し単位)
	{F_HalfPageUp,			"HalfPageUp",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown,		"HalfPageDown",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp,				"1PageUp",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown,			"1PageDown",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP,			"GoFileTop",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイルの先頭に移動
	{F_GOFILEEND,			"GoFileEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイルの最後に移動
	{F_CURLINECENTER,		"CurLineCenter",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル行をウィンドウ中央へ
	{F_JUMPHIST_PREV,		"MoveHistPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //移動履歴: 前へ
	{F_JUMPHIST_NEXT,		"MoveHistNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //移動履歴: 次へ
	{F_JUMPHIST_SET,		"MoveHistSet",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //現在位置を移動履歴に登録
	{F_WndScrollDown,		"F_WndScrollDown",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //テキストを１行下へスクロール	// 2001/06/20 asa-o
	{F_WndScrollUp,			"F_WndScrollUp",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //テキストを１行上へスクロール	// 2001/06/20 asa-o
	{F_GONEXTPARAGRAPH,		"GoNextParagraph",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次の段落へ移動
	{F_GOPREVPARAGRAPH,		"GoPrevParagraph",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前の段落へ移動

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	{F_SELECTWORD,			"SelectWord",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //現在位置の単語選択
	{F_SELECTALL,			"SelectAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //すべて選択
	{F_BEGIN_SEL,			"BeginSelect",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //範囲選択開始 Mar. 5, 2001 genta 名称修正
	{F_UP_SEL,				"Up_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル上移動
	{F_DOWN_SEL,			"Down_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル下移動
	{F_LEFT_SEL,			"Left_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル左移動
	{F_RIGHT_SEL,			"Right_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル右移動
	{F_UP2_SEL,				"Up2_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル上移動(２行ごと)
	{F_DOWN2_SEL,			"Down2_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル下移動(２行ごと)
	{F_WORDLEFT_SEL,		"WordLeft_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)単語の左端に移動
	{F_WORDRIGHT_SEL,		"WordRight_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)単語の右端に移動
	{F_GOLINETOP_SEL,		"GoLineTop_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)行頭に移動(折り返し単位)
	{F_GOLINEEND_SEL,		"GoLineEnd_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)行末に移動(折り返し単位)
	{F_HalfPageUp_Sel,		"HalfPageUp_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown_Sel,	"HalfPageDown_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp_Sel,			"1PageUp_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown_Sel,		"1PageDown_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP_SEL,		"GoFileTop_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)ファイルの先頭に移動
	{F_GOFILEEND_SEL,		"GoFileEnd_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)ファイルの最後に移動
	{F_GONEXTPARAGRAPH_SEL,	"GoNextParagraph_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次の段落へ移動
	{F_GOPREVPARAGRAPH_SEL,	"GoPrevParagraph_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前の段落へ移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	{F_BEGIN_BOX,			"BeginBoxSelect",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //矩形範囲選択開始

	/* クリップボード系 */
	{F_CUT,						"Cut",						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //切り取り(選択範囲をクリップボードにコピーして削除)
	{F_COPY,					"Copy",						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //コピー(選択範囲をクリップボードにコピー)
	{F_PASTE,					"Paste",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //貼り付け(クリップボードから貼り付け)
	{F_COPY_ADDCRLF,			"CopyAddCRLF",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //折り返し位置に改行をつけてコピー
	{F_COPY_CRLF,				"CopyCRLF",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	{F_PASTEBOX,				"PasteBox",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //矩形貼り付け(クリップボードから矩形貼り付け)
	{F_INSTEXT,					"InsText",					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // テキストを貼り付け
	{F_ADDTAIL,					"AddTail",					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 最後にテキストを追加
	{F_COPYLINES,				"CopyLines",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内全行コピー
	{F_COPYLINESASPASSAGE,		"CopyLinesAsPassage",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内全行引用符付きコピー
	{F_COPYLINESWITHLINENUMBER,	"CopyLinesWithLineNumber",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内全行行番号付きコピー
	{F_COPYPATH,				"CopyPath",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このファイルのパス名をクリップボードにコピー
	{F_COPYFNAME,				"CopyFilename",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このファイル名をクリップボードにコピー // 2002/2/3 aroka
	{F_COPYTAG,					"CopyTag",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	{F_CREATEKEYBINDLIST,		"CopyKeyBindList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	{F_INS_DATE,				"InsertDate",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 日付挿入
	{F_INS_TIME,				"InsertTime",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 時刻挿入
	{F_CTRL_CODE_DIALOG,		"CtrlCodeDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK

	/* 変換系 */
	{F_TOLOWER,		 			"ToLower",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //小文字
	{F_TOUPPER,		 			"ToUpper",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //大文字
	{F_TOHANKAKU,		 		"ToHankaku",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 全角→半角 */
	{F_TOHANKATA,		 		"ToHankata",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	{F_TOZENEI,		 			"ToZenEi",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			"ToHanEi",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 全角英数→半角英数 */
	{F_TOZENKAKUKATA,	 		"ToZenKata",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	{F_TOZENKAKUHIRA,	 		"ToZenHira",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	{F_HANKATATOZENKAKUKATA,	"HanKataToZenKata",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角カタカナ→全角カタカナ */
	{F_HANKATATOZENKAKUHIRA,	"HanKataToZenHira",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角カタカナ→全角ひらがな */
	{F_TABTOSPACE,				"TABToSPACE",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* TAB→空白 */
	{F_SPACETOTAB,				"SPACEToTAB",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 空白→TAB */ //#### Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		"AutoToSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 自動判別→SJISコード変換 */
	{F_CODECNV_EMAIL,			"JIStoSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //E-Mail(JIS→SJIS)コード変換
	{F_CODECNV_EUC2SJIS,		"EUCtoSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUC→SJISコード変換
	{F_CODECNV_UNICODE2SJIS,	"CodeCnvUNICODEtoSJIS",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicode→SJISコード変換
	{F_CODECNV_UNICODEBE2SJIS,	"CodeCnvUNICODEBEtoSJIS",{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // UnicodeBE→SJISコード変換
	{F_CODECNV_UTF82SJIS,		"UTF8toSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-8→SJISコード変換 */
	{F_CODECNV_UTF72SJIS,		"UTF7toSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-7→SJISコード変換 */
	{F_CODECNV_SJIS2JIS,		"SJIStoJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→JISコード変換 */
	{F_CODECNV_SJIS2EUC,		"SJIStoEUC",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→EUCコード変換 */
	{F_CODECNV_SJIS2UTF8,		"SJIStoUTF8",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→UTF-8コード変換 */
	{F_CODECNV_SJIS2UTF7,		"SJIStoUTF7",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→UTF-7コード変換 */
	{F_BASE64DECODE,	 		"Base64Decode",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Base64デコードして保存
	{F_UUDECODE,		 		"Uudecode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


	/* 検索系 */
	{F_SEARCH_DIALOG,			"SearchDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //検索(単語検索ダイアログ)
	{F_SEARCH_NEXT,				"SearchNext",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次を検索
	{F_SEARCH_PREV,				"SearchPrev",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前を検索
	{F_REPLACE_DIALOG,			"ReplaceDialog",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //置換(置換ダイアログ)
	{F_REPLACE,					"Replace",			{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //置換(実行)
	{F_REPLACE_ALL,				"ReplaceAll",		{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //すべて置換(実行)
	{F_SEARCH_CLEARMARK,		"SearchClearMark",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //検索マークのクリア
	{F_JUMP_SRCHSTARTPOS,		"SearchStartPos",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //検索開始位置へ戻る			// 02/06/26 ai
	{F_GREP,					"Grep",				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_I4   },	VT_EMPTY,	NULL}, //Grep
	{F_JUMP,					"Jump",				{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //指定行ヘジャンプ
	{F_OUTLINE,					"Outline",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //アウトライン解析
	{F_TAGJUMP,					"TagJump",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タグジャンプ機能
	{F_TAGJUMPBACK,				"TagJumpBack",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タグジャンプバック機能
	{F_TAGS_MAKE,				"TagMake",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タグファイルの作成	//@@@ 2003.04.13 MIK
	{F_DIRECT_TAGJUMP,			"DirectTagJump",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ダイレクトタグジャンプ機能	//@@@ 2003.04.15 MIK
	{F_TAGJUMP_KEYWORD,			"KeywordTagJump",	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //キーワードを指定してダイレクトタグジャンプ機能 //@@@ 2005.03.31 MIK
	{F_COMPARE,					"Compare",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイル内容比較
	{F_DIFF_DIALOG,				"DiffDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
	{F_DIFF,					"Diff",				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示				//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	{F_DIFF_NEXT,				"DiffNext",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(次へ)			//@@@ 2002.05.25 MIK
	{F_DIFF_PREV,				"DiffPrev",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(前へ)			//@@@ 2002.05.25 MIK
	{F_DIFF_RESET,				"DiffReset",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(全解除)		//@@@ 2002.05.25 MIK
	{F_BRACKETPAIR,				"BracketPair",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //対括弧の検索
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			"BookmarkSet",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブックマーク設定・解除
	{F_BOOKMARK_NEXT,			"BookmarkNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次のブックマークへ
	{F_BOOKMARK_PREV,			"BookmarkPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前のブックマークへ
	{F_BOOKMARK_RESET,			"BookmarkReset",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブックマークの全解除
	{F_BOOKMARK_VIEW,			"BookmarkView",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブックマークの一覧
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		"BookmarkPattern",	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 2002.01.16 hor 指定パターンに一致する行をマーク

	/* モード切り替え系 */
	{F_CHGMOD_INS,				"ChgmodINS",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //挿入／上書きモード切り替え
	{F_CHGMOD_EOL,				"ChgmodEOL",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //入力改行コード指定 2003.06.23 Moca
	{F_CANCEL_MODE,				"CancelMode",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //各種モードの取り消し

	/* 設定系 */
	{F_SHOWTOOLBAR,				"ShowToolbar",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ツールバーの表示 */
	{F_SHOWFUNCKEY,				"ShowFunckey",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ファンクションキーの表示 */
	{F_SHOWTAB,					"ShowTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* タブの表示 */	//@@@ 2003.06.10 MIK
	{F_SHOWSTATUSBAR,			"ShowStatusbar",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ステータスバーの表示 */
	{F_TYPE_LIST,				"TypeList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* タイプ別設定一覧 */
	{F_OPTION_TYPE,				"OptionType",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* タイプ別設定 */
	{F_OPTION,					"OptionCommon",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 共通設定 */
	{F_FONT,					"SelectFont",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* フォント設定 */
	{F_WRAPWINDOWWIDTH,			"WrapWindowWidth",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	{F_FAVORITE,				"OptionFavorite",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* お気に入りの設定 */	//@@@ 2003.04.08 MIK
	{F_SET_QUOTESTRING,			"SetMsgQuoteStr",	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 共通設定→書式→引用符の設定 */	//Jan. 29, 2005 genta


	//	Oct. 9, 2001 genta 追加
	{F_EXECCOMMAND,				"ExecCommand",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 外部コマンド実行 */

	/* カスタムメニュー */
	{F_MENU_RBUTTON,			"RMenu",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 右クリックメニュー */
	{F_CUSTMENU_1,				"CustMenu1",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー1 */
	{F_CUSTMENU_2,				"CustMenu2",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー2 */
	{F_CUSTMENU_3,				"CustMenu3",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー3 */
	{F_CUSTMENU_4,				"CustMenu4",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー4 */
	{F_CUSTMENU_5,				"CustMenu5",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー5 */
	{F_CUSTMENU_6,				"CustMenu6",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー6 */
	{F_CUSTMENU_7,				"CustMenu7",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー7 */
	{F_CUSTMENU_8,				"CustMenu8",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー8 */
	{F_CUSTMENU_9,				"CustMenu9",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー9 */
	{F_CUSTMENU_10,				"CustMenu10",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー10 */
	{F_CUSTMENU_11,				"CustMenu11",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー11 */
	{F_CUSTMENU_12,				"CustMenu12",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー12 */
	{F_CUSTMENU_13,				"CustMenu13",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー13 */
	{F_CUSTMENU_14,				"CustMenu14",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー14 */
	{F_CUSTMENU_15,				"CustMenu15",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー15 */
	{F_CUSTMENU_16,				"CustMenu16",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー16 */
	{F_CUSTMENU_17,				"CustMenu17", 		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー17 */
	{F_CUSTMENU_18,				"CustMenu18",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー18 */
	{F_CUSTMENU_19,				"CustMenu19",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー19 */
	{F_CUSTMENU_20,				"CustMenu20",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー20 */
	{F_CUSTMENU_21,				"CustMenu21",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー21 */
	{F_CUSTMENU_22,				"CustMenu22",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー22 */
	{F_CUSTMENU_23,				"CustMenu23",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー23 */
	{F_CUSTMENU_24,				"CustMenu24",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー24 */

	/* ウィンドウ系 */
	{F_SPLIT_V,					"SplitWinV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	{F_SPLIT_H,					"SplitWinH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	{F_SPLIT_VH,				"SplitWinVH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	{F_WINCLOSE,				"WinClose",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ウィンドウを閉じる
	{F_WIN_CLOSEALL,			"WinCloseAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	{F_CASCADE,					"CascadeWin",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //重ねて表示
	{F_TILE_V,					"TileWinV",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上下に並べて表示
	{F_TILE_H,					"TileWinH",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左右に並べて表示
	{F_NEXTWINDOW,				"NextWindow",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次のウィンドウ
	{F_PREVWINDOW,				"PrevWindow",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前のウィンドウ
	{F_WINLIST,					"WindowList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
	{F_MAXIMIZE_V,				"MaximizeV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //縦方向に最大化
	{F_MAXIMIZE_H,				"MaximizeH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //横方向に最大化 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			"MinimizeAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	{F_REDRAW,					"ReDraw",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //再描画
	{F_WIN_OUTPUT,				"ActivateWinOutput",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //アウトプットウィンドウ表示
	{F_TRACEOUT,				"TraceOut",				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //マクロ用アウトプットウィンドウに出力	2006.04.26 maru
	{F_TOPMOST,					"WindowTopMost",	{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //常に手前に表示

	/* 支援 */
	{F_HOKAN,					"Complete",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 入力補完 */	//Oct. 15, 2000 JEPRO 入ってなかったので英名を付けて入れてみた
	{F_HELP_CONTENTS,			"HelpContents",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	{F_HELP_SEARCH,				"HelpSearch",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	{F_MENU_ALLFUNC,			"CommandList",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* コマンド一覧 */
	{F_EXTHELP1,				"ExtHelp1",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 外部ヘルプ１ */
	//	Jul. 5, 2002 genta 引数追加
	{F_EXTHTMLHELP,				"ExtHtmlHelp",	{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 外部HTMLヘルプ */
	{F_ABOUT,					"About",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{-1,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};
//int	CSMacroMgr::m_nMacroFuncInfoArrNum = sizeof( CSMacroMgr::m_MacroFuncInfoArr ) / sizeof( CSMacroMgr::m_MacroFuncInfoArr[0] );

/*!
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2002.04.29 genta オブジェクトの実体は実行時まで生成しない。
*/
CSMacroMgr::CSMacroMgr()
{
	MY_RUNNINGTIMER( cRunningTimer, "CSMacroMgr::CSMacroMgr" );
	
	m_pShareData = CShareData::getInstance()->GetShareData();
	
	CPPAMacroMgr::declare();
	CKeyMacroMgr::declare();
	CWSHMacroManager::declare();
	
	int i;
	for ( i = 0 ; i < MAX_CUSTMACRO ; i++ ){
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	m_pKeyMacro = NULL;

	//	Sep. 15, 2005 FILE
	SetCurrentIdx( INVALID_MACRO_IDX );
}

CSMacroMgr::~CSMacroMgr()
{
	//- 20011229 add by aroka
	ClearAll();
	
	//	Jun. 16, 2002 genta
	//	ClearAllと同じ処理だったので削除
}

/*! キーマクロのバッファをクリアする */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		//	Apr. 29, 2002 genta
		delete m_cSavedKeyMacro[i];
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	delete m_pKeyMacro;
	m_pKeyMacro = NULL;
}

/*! @briefキーマクロのバッファにデータ追加

	@param nFuncID [in] 機能番号
	@param lParam1 [in] パラメータ。
	@param mbuf [in] 読み込み先マクロバッファ
	
	@date 2002.06.16 genta キーマクロの多種対応のため変更

*/
int CSMacroMgr::Append( int idx, /*CSMacroMgr::Macro1& mbuf, */ int nFuncID, LPARAM lParam1, CEditView* pcEditView )
{
	assert( idx == STAND_KEYMACRO );
	if (idx == STAND_KEYMACRO){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro == NULL ){
			//	1. 実体がまだ無い場合
			//	2. CKeyMacroMgr以外の物が入っていた場合
			//	いずれにしても再生成する．
			delete m_pKeyMacro;
			m_pKeyMacro = new CKeyMacroMgr;
			pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		}
		pKeyMacro->Append( nFuncID, lParam1, pcEditView );
	}
//	else {
		//m_cSavedKeyMacro[idx]->Append( nFuncID, lParam1, pcEditView );
//	}
	return TRUE;
}


/*!	@brief キーボードマクロの実行

	CShareDataからファイル名を取得し、実行する。

	@param hInstance [in] インスタンス
	@param hwndParent [in] 親ウィンドウの
	@param pViewClass [in] macro実行対象のView
	@param idx [in] マクロ番号。
*/
BOOL CSMacroMgr::Exec( int idx , HINSTANCE hInstance, CEditView* pcEditView )
{
	if( idx == STAND_KEYMACRO ){
		//	Jun. 16, 2002 genta
		//	キーマクロ以外のサポートによりNULLの可能性が出てきたので判定追加
		if( m_pKeyMacro != NULL ){
			//	Sep. 15, 2005 FILE
			SetCurrentIdx( idx );
			m_pKeyMacro->ExecKeyMacro( pcEditView );
			//	Sep. 15, 2005 FILE
			SetCurrentIdx( INVALID_MACRO_IDX );
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	範囲チェック
		return FALSE;

	/* 読み込み前か、毎回読み込む設定の場合は、ファイルを読み込みなおす */
	//	Apr. 29, 2002 genta
	if( m_cSavedKeyMacro[idx] == NULL || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareDataから、マクロファイル名を取得
		//	Jun. 08, 2003 Moca 呼び出し側でパス名を用意
		//	Jun. 16, 2003 genta 書式をちょっと変更
		char ptr[_MAX_PATH * 2];
		int n = CShareData::getInstance()->GetMacroFilename( idx, ptr, sizeof(ptr) );
		if ( n <= 0 ){
			return FALSE;
		}

		if( !Load( idx, hInstance, ptr ) )
			return FALSE;
	}

	//	Sep. 15, 2005 FILE
	SetCurrentIdx( idx );
	m_cSavedKeyMacro[idx]->ExecKeyMacro(pcEditView);
	//	Sep. 15, 2005 FILE
	SetCurrentIdx( INVALID_MACRO_IDX );

	pcEditView->Redraw();	//	必要？
	return TRUE;
}

/*! キーボードマクロの読み込み

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名

	読み込みに失敗したときはマクロバッファのオブジェクトは解放され，
	NULLが設定される．

	@author Norio Nakatani, YAZAKI, genta
*/
BOOL CSMacroMgr::Load( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	CMacroManagerBase** ppMacro = Idx2Ptr( idx );

	if( ppMacro == NULL ){
#ifdef _DEBUG
	MYTRACE( "CSMacroMgr::Load() Out of range: idx=%d Path=%s\n", idx, pszPath);
#endif
	}
	//	バッファクリア
	delete *ppMacro;
	*ppMacro = NULL;
	
	const char *ext = strrchr( pszPath, '.');
	//	Feb. 02, 2004 genta .が無い場合にext==NULLとなるのでNULLチェック追加
	if( ext != NULL ){
		const char *chk = strrchr( ext, '\\' );
		if( chk != NULL ){	//	.のあとに\があったらそれは拡張子の区切りではない
							//	\が漢字の2バイト目の場合も拡張子ではない。
			ext = NULL;
		}
	}
	if(ext != NULL){
		++ext;
	}
	*ppMacro = CMacroFactory::Instance()->Create(ext);
	if( *ppMacro == NULL )
		return FALSE;
	//	From Here Jun. 16, 2002 genta
	//	読み込みエラー時はインスタンス削除
	if( (*ppMacro)->LoadKeyMacro(hInstance, pszPath )){
		return TRUE;
	}
	else {
		delete *ppMacro;
		*ppMacro = NULL;
	}
	//	To Here Jun. 16, 2002 genta
	return FALSE;
}

/*! キーボードマクロの保存

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名
	@param hInstance [in] インスタンスハンドル

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	assert( idx == STAND_KEYMACRO );
	if ( idx == STAND_KEYMACRO ){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro != NULL ){
			return pKeyMacro->SaveKeyMacro(hInstance, pszPath );
		}
		//	Jun. 27, 2002 genta
		//	空マクロの場合は正常終了と見なす．
		if( m_pKeyMacro == NULL ){
			return TRUE;
		}

	}
//	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
//		return m_cSavedKeyMacro[idx]->SaveKeyMacro(hInstance, pszPath );
//	}
	return FALSE;
}

/*
	指定されたマクロをクリアする
	
	@param idx [in] マクロ番号(0-), STAND_KEYMACROは標準キーマクロバッファを表す．
*/
void CSMacroMgr::Clear( int idx )
{
	CMacroManagerBase **ppMacro = Idx2Ptr( idx );
	if( ppMacro != NULL ){
		delete *ppMacro;
		*ppMacro = NULL;
	}
}

/*
||  Attributes & Operations
*/
/*
	指定されたIDに対応するMacroInfo構造体へのポインタを返す．
	該当するIDに対応する構造体がなければNULLを返す．

	@param nFuncID [in] 機能番号
	@return 構造体へのポインタ．見つからなければNULL
	
	@date 2002.06.16 genta
	@data 2003.02.24 m_MacroFuncInfoNotCommandArrも検索対象にする
*/
const MacroFuncInfo* CSMacroMgr::GetFuncInfoByID( int nFuncID )
{
	int i;
	//	Jun. 27, 2002 genta
	//	番人をコード0として拾ってしまうので，配列サイズによる判定をやめた．
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoArr[i];
		}
	}
	for( i = 0; m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoNotCommandArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoNotCommandArr[i];
		}
	}
	return NULL;
}

/*!
	機能番号から関数名と機能名日本語を取得
	
	@param hInstance [in] リソース取得のためのInstance Handle
	@param nFuncID [in] 機能番号
	@param pszFuncName [out] 関数名．この先には最長関数名＋1バイトのメモリが必要．
	@param pszFuncNameJapanese [out] 機能名日本語．この先には256バイトのメモリが必要．
	@return 成功したときはpszFuncName．見つからなかったときはNULL．
	
	@note
	それぞれ，文字列格納領域の指す先がNULLの時は文字列を格納しない．
	ただし，pszFuncNameをNULLにしてしまうと戻り値が常にNULLになって
	成功判定が行えなくなる．
	
	@date 2002.06.16 genta 新設のGetFuncInfoById(int)を内部で使うように．
*/
char* CSMacroMgr::GetFuncInfoByID( HINSTANCE hInstance, int nFuncID, char* pszFuncName, char* pszFuncNameJapanese )
{
	const MacroFuncInfo* MacroInfo = GetFuncInfoByID( nFuncID );
	if( MacroInfo != NULL ){
		if( pszFuncName != NULL ){
			strcpy( pszFuncName, MacroInfo->m_pszFuncName );
			char *p = pszFuncName;
			while (*p){
				if (*p == '('){
					*p = '\0';
					break;
				}
				*p++;
			}
		}
		//	Jun. 16, 2002 genta NULLのときは何もしない．
		if( pszFuncNameJapanese != NULL ){
			::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
		}
		return pszFuncName;
	}
	return NULL;
}

/*!
	関数名（S_xxxx）から機能番号と機能名日本語を取得．
	関数名はS_で始まる場合と始まらない場合の両方に対応．

	@param hInstance [in] リソース取得のためのInstance Handle
	@param pszFuncName [in] 関数名
	@param pszFuncNameJapanese [out] 機能名日本語．この先には256バイトのメモリが必要．
	@return 成功したときは機能番号．見つからなかったときは-1．
	
	@note
	pszFuncNameJapanese の指す先がNULLの時は日本語名を格納しない．
	
	@date 2002.06.16 genta ループ内の文字列コピーを排除
*/
int CSMacroMgr::GetFuncInfoByName( HINSTANCE hInstance, const char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	int		nFuncID;
	//	Jun. 16, 2002 genta
	const char *normalizedFuncName;
	
	//	S_で始まっているか
	if( pszFuncName == NULL ){
		return -1;
	}
	if( pszFuncName[0] == 'S' && pszFuncName[1] == '_' ){
		normalizedFuncName = pszFuncName + 2;
	}
	else {
		normalizedFuncName = pszFuncName;
	}
	
	char szBuffer[1024] = "S_";
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == strcmp( normalizedFuncName, m_MacroFuncInfoArr[i].m_pszFuncName )){
			nFuncID = m_MacroFuncInfoArr[i].m_nFuncID;
			if( pszFuncNameJapanese != NULL ){
				::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			}
			return nFuncID;
		}
	}
	return -1;
}

/* キーマクロに記録可能な機能かどうかを調べる */
BOOL CSMacroMgr::CanFuncIsKeyMacro( int nFuncID )
{
	switch( nFuncID ){
	/* ファイル操作系 */
//	case F_FILENEW					://新規作成
//	case F_FILEOPEN					://開く
//	case F_FILESAVE					://上書き保存
//	case F_FILESAVEAS_DIALOG		://名前を付けて保存
//	case F_FILECLOSE				://閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
//	case F_FILECLOSE_OPEN			://閉じて開く
	case F_FILE_REOPEN				://開き直す	//Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS			://SJISで開き直す
	case F_FILE_REOPEN_JIS			://JISで開き直す
	case F_FILE_REOPEN_EUC			://EUCで開き直す
	case F_FILE_REOPEN_UNICODE		://Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE	://UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8			://UTF-8で開き直す
	case F_FILE_REOPEN_UTF7			://UTF-7で開き直す
//	case F_PRINT					://印刷
//	case F_PRINT_DIALOG				://印刷ダイアログ
//	case F_PRINT_PREVIEW			://印刷プレビュー
//	case F_PRINT_PAGESETUP			://印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
//	case F_OPEN_HfromtoC:			://同名のC/C++ヘッダ(ソース)を開く	//Feb. 9, 2001 JEPRO 追加
//	case F_OPEN_HHPP				://同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
//	case F_OPEN_CCPP				://同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
//	case F_ACTIVATE_SQLPLUS			:/* Oracle SQL*Plusをアクティブ表示 */
//	case F_PLSQL_COMPILE_ON_SQLPLUS	:/* Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
///	case F_BROWSE					://ブラウズ
//	case F_PROPERTY_FILE			://ファイルのプロパティ
//	case F_EXITALLEDITORS			://編集の全終了	// 2007.02.13 ryoji 追加
//	case F_EXITALL					://サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加

	/* 編集系 */
	case F_CHAR						://文字入力
	case F_IME_CHAR					://全角文字入力
	case F_UNDO						://元に戻す(Undo)
	case F_REDO						://やり直し(Redo)
	case F_DELETE					://削除
	case F_DELETE_BACK				://カーソル前を削除
	case F_WordDeleteToStart		://単語の左端まで削除
	case F_WordDeleteToEnd			://単語の右端まで削除
	case F_WordCut					://単語切り取り
	case F_WordDelete				://単語削除
	case F_LineCutToStart			://行頭まで切り取り(改行単位)
	case F_LineCutToEnd				://行末まで切り取り(改行単位)
	case F_LineDeleteToStart		://行頭まで削除(改行単位)
	case F_LineDeleteToEnd			://行末まで削除(改行単位)
	case F_CUT_LINE					://行切り取り(折り返し単位)
	case F_DELETE_LINE				://行削除(折り返し単位)
	case F_DUPLICATELINE			://行の二重化(折り返し単位)
	case F_INDENT_TAB				://TABインデント
	case F_UNINDENT_TAB				://逆TABインデント
	case F_INDENT_SPACE				://SPACEインデント
	case F_UNINDENT_SPACE			://逆SPACEインデント
	case F_LTRIM					:// 2001.12.03 hor
	case F_RTRIM					:// 2001.12.03 hor
	case F_SORT_ASC					:// 2001.12.06 hor
	case F_SORT_DESC				:// 2001.12.06 hor
	case F_MERGE					:// 2001.12.06 hor

	/* カーソル移動系 */
	case F_UP						://カーソル上移動
	case F_DOWN						://カーソル下移動
	case F_LEFT						://カーソル左移動
	case F_RIGHT					://カーソル右移動
//	case F_ROLLDOWN					://スクロールダウン
//	case F_ROLLUP					://スクロールアップ
//	case F_HalfPageUp				://半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	case F_HalfPageDown				://半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	case F_1PageUp					://１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
//	case F_1PageDown				://１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_UP2						://カーソル上移動(２行ごと)
	case F_DOWN2					://カーソル下移動(２行ごと)
	case F_GOLINETOP				://行頭に移動(折り返し単位)
	case F_GOLINEEND				://行末に移動(折り返し単位)
	case F_GOFILETOP				://ファイルの先頭に移動
	case F_GOFILEEND				://ファイルの最後に移動
	case F_WORDLEFT					://単語の左端に移動
	case F_WORDRIGHT				://単語の右端に移動
//	case F_CURLINECENTER			://カーソル行をウィンドウ中央へ
	case F_JUMPHIST_PREV			://移動履歴: 前へ
	case F_JUMPHIST_NEXT			://移動履歴: 次へ
	case F_JUMPHIST_SET				://現在位置を移動履歴に登録

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので独立化して(選択)を移動(サブメニュー化は構造上できないので)
	case F_SELECTWORD				://現在位置の単語選択
	case F_SELECTALL				://すべて選択
	case F_BEGIN_SEL				://範囲選択開始
	case F_UP_SEL					://(範囲選択)カーソル上移動
	case F_DOWN_SEL					://(範囲選択)カーソル下移動
	case F_LEFT_SEL					://(範囲選択)カーソル左移動
	case F_RIGHT_SEL				://(範囲選択)カーソル右移動
	case F_UP2_SEL					://(範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL				://(範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL				://(範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL			://(範囲選択)単語の右端に移動
	case F_GOLINETOP_SEL			://(範囲選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_SEL			://(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL				://(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL				://(範囲選択)スクロールアップ
//	case F_HalfPageUp_Sel			://(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	case F_HalfPageDown_Sel			://(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	case F_1PageUp_Sel				://(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
//	case F_1PageDown_Sel			://(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_GOFILETOP_SEL			://(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL			://(範囲選択)ファイルの最後に移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
//	case F_BOXSELALL				//矩形ですべて選択
	case F_BEGIN_BOX				://矩形範囲選択開始
/*
	case F_UP_BOX					://(矩形選択)カーソル上移動
	case F_DOWN_BOX					://(矩形選択)カーソル下移動
	case F_LEFT_BOX					://(矩形選択)カーソル左移動
	case F_RIGHT_BOX				://(矩形選択)カーソル右移動
	case F_UP2_BOX					://(矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX				://(矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX				://(矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX			://(矩形選択)単語の右端に移動
	case F_GOLINETOP_BOX			://(矩形選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_BOX			://(矩形選択)行末に移動(折り返し単位)
	case F_HalfPageUp_Box			://(矩形選択)半ページアップ
	case F_HalfPageDown_Box			://(矩形選択)半ページダウン
	case F_1PageUp_Box				://(矩形選択)１ページアップ
	case F_1PageDown_Box			://(矩形選択)１ページダウン
	case F_GOFILETOP_BOX			://(矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX			://(矩形選択)ファイルの最後に移動
*/
	/* クリップボード系 */
	case F_CUT						://切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY						://コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF				://折り返し位置に改行をつけてコピー
	case F_COPY_CRLF				://CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	case F_PASTE					://貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX					://矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSTEXT					://テキストを貼り付け
//	case F_ADDTAIL					://最後にテキストを追加
	case F_COPYLINES				://選択範囲内全行コピー
	case F_COPYLINESASPASSAGE		://選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER 	://選択範囲内全行行番号付きコピー
	case F_COPYPATH					://このファイルのパス名をクリップボードにコピー
	case F_COPYTAG					://このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	case F_COPYFNAME				://このファイル名をクリップボードにコピー // 2002/2/3 aroka
	case F_CREATEKEYBINDLIST		://キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加	//Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE					:// 日付挿入
	case F_INS_TIME					:// 時刻挿入
//	case F_CTRL_CODE_DIALOG			://コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK

	/* 変換系 */
	case F_TOLOWER		 			://小文字
	case F_TOUPPER		 			://大文字
	case F_TOHANKAKU		 		:/* 全角→半角 */
	case F_TOHANKATA		 		:/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI			 		:/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI			 		:/* 全角英数→半角英数 */
	case F_TOZENKAKUKATA	 		:/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA	 		:/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKAKUKATA		:/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENKAKUHIRA		:/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE				:/* TAB→空白 */
	case F_SPACETOTAB				:/* 空白→TAB */  //#### Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS		:/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL			://E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS			://EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS		://Unicode→SJISコード変換
	case F_CODECNV_UNICODEBE2SJIS	://UnicodeBE→SJISコード変換
	case F_CODECNV_UTF82SJIS		:/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS		:/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS			:/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC			:/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8		:/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7		:/* SJIS→UTF-7コード変換 */
//	case F_BASE64DECODE	 			://Base64デコードして保存
//	case F_UUDECODE		 			://uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
//	case F_SEARCH_DIALOG			://検索(単語検索ダイアログ)
	case F_SEARCH_NEXT				://次を検索
	case F_SEARCH_PREV				://前を検索
	case F_REPLACE					://置換(実行)
	case F_REPLACE_ALL				://すべて置換(実行)
	case F_SEARCH_CLEARMARK			://検索マークのクリア
	case F_JUMP_SRCHSTARTPOS		://検索開始位置へ戻る		// 02/06/26 ai
	case F_GREP						://Grep
//	case F_JUMP_DIALOG				://指定行ヘジャンプ
	case F_JUMP						://指定行へジャンプ @@@ 2002.2.2 YAZAKI
//	case F_OUTLINE					://アウトライン解析
	case F_TAGJUMP					://タグジャンプ機能
	case F_TAGJUMPBACK				://タグジャンプバック機能
//	case F_TAGS_MAKE				://タグファイルの作成	//@@@ 2003.04.13 MIK
//	case F_COMPARE					://ファイル内容比較
//	case F_DIFF_DIALOG				://DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
//	case F_DIFF						://DIFF差分表示				//@@@ 2002.05.25 MIK
//	case F_DIFF_NEXT				://DIFF差分表示(次へ)		//@@@ 2002.05.25 MIK
//	case F_DIFF_PREV				://DIFF差分表示(前へ)		//@@@ 2002.05.25 MIK
//	case F_DIFF_RESET				://DIFF差分表示(全解除)		//@@@ 2002.05.25 MIK
	case F_BRACKETPAIR				://対括弧の検索
// From Here 2001.12.03 hor
	case F_BOOKMARK_SET				://ブックマーク設定・解除
	case F_BOOKMARK_NEXT			://次のブックマークへ
	case F_BOOKMARK_PREV			://前のブックマークへ
	case F_BOOKMARK_RESET			://ブックマークの全解除
//	case F_BOOKMARK_VIEW			://ブックマークの一覧
// To Here 2001.12.03 hor
	case F_BOOKMARK_PATTERN			://検索しして該当行をマーク	// 2002.02.08 hor

	/* モード切り替え系 */
	case F_CHGMOD_INS				://挿入／上書きモード切り替え
	case F_CHGMOD_EOL				://入力改行コード指定	2003.06.23 Moca

	case F_CANCEL_MODE				://各種モードの取り消し

	/* 設定系 */
//	case F_SHOWTOOLBAR				:/* ツールバーの表示 */
//	case F_SHOWFUNCKEY				:/* ファンクションキーの表示 */
//	case F_SHOWTAB					:/* タブの表示 */
//	case F_SHOWSTATUSBAR			:/* ステータスバーの表示 */
//	case F_TYPE_LIST				:/* タイプ別設定一覧 */
//	case F_OPTION_TYPE				:/* タイプ別設定 */
//	case F_OPTION					:/* 共通設定 */
//	case F_FONT						:/* フォント設定 */
//	case F_WRAPWINDOWWIDTH			:/* 現在のウィンドウ幅で折り返し */	//Oct. 15, 2000 JEPRO
//	case F_FAVORITE					:/* お気に入りの設定 */	//@@@ 2003.04.08 MIK

	case F_EXECCOMMAND				:/* 外部コマンド実行 */	//@@@2002.2.2 YAZAKI 追加

	/* カスタムメニュー */
//	case F_MENU_RBUTTON				:/* 右クリックメニュー */
//	case F_CUSTMENU_1				:/* カスタムメニュー1 */
//	case F_CUSTMENU_2				:/* カスタムメニュー2 */
//	case F_CUSTMENU_3				:/* カスタムメニュー3 */
//	case F_CUSTMENU_4				:/* カスタムメニュー4 */
//	case F_CUSTMENU_5				:/* カスタムメニュー5 */
//	case F_CUSTMENU_6				:/* カスタムメニュー6 */
//	case F_CUSTMENU_7				:/* カスタムメニュー7 */
//	case F_CUSTMENU_8				:/* カスタムメニュー8 */
//	case F_CUSTMENU_9				:/* カスタムメニュー9 */
//	case F_CUSTMENU_10				:/* カスタムメニュー10 */
//	case F_CUSTMENU_11				:/* カスタムメニュー11 */
//	case F_CUSTMENU_12				:/* カスタムメニュー12 */
//	case F_CUSTMENU_13				:/* カスタムメニュー13 */
//	case F_CUSTMENU_14				:/* カスタムメニュー14 */
//	case F_CUSTMENU_15				:/* カスタムメニュー15 */
//	case F_CUSTMENU_16				:/* カスタムメニュー16 */
//	case F_CUSTMENU_17				:/* カスタムメニュー17 */
//	case F_CUSTMENU_18				:/* カスタムメニュー18 */
//	case F_CUSTMENU_19				:/* カスタムメニュー19 */
//	case F_CUSTMENU_20				:/* カスタムメニュー20 */
//	case F_CUSTMENU_21				:/* カスタムメニュー21 */
//	case F_CUSTMENU_22				:/* カスタムメニュー22 */
//	case F_CUSTMENU_23				:/* カスタムメニュー23 */
//	case F_CUSTMENU_24				:/* カスタムメニュー24 */

	/* ウィンドウ系 */
//	case F_SPLIT_V					://上下に分割	//Sept. 16, 2000 jepro 説明を「縦」から「上下に」に変更
//	case F_SPLIT_H					://左右に分割	//Sept. 16, 2000 jepro 説明を「横」から「左右に」に変更
//	case F_SPLIT_VH					://縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
//	case F_WINCLOSE					://ウィンドウを閉じる
//	case F_WIN_CLOSEALL				://すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
//	case F_NEXTWINDOW				://次のウィンドウ
//	case F_PREVWINDOW				://前のウィンドウ
//	case F_CASCADE					://重ねて表示
//	case F_TILE_V					://上下に並べて表示
//	case F_TILE_H					://左右に並べて表示
//	case F_MAXIMIZE_V				://縦方向に最大化
//	case F_MINIMIZE_ALL				://すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	case F_REDRAW					://再描画
	case F_WIN_OUTPUT				://アウトプットウィンドウ表示
//	case F_TRACEOUT					://マクロ用アウトプットウィンドウに表示	2006.04.26 maru
	case F_TOPMOST					://常に手前に表示

	/* 支援 */
//  case F_HOKAN					:/* 入力補完 */				//Oct. 15, 2000 JEPRO 入ってなかったので入れてみた
//	case F_HELP_CONTENTS			:/* ヘルプ目次 */			//Dec. 25, 2000 JEPRO 追加
//	case F_HELP_SEARCH				:/* ヘルプキーワード検索 */	//Dec. 25, 2000 JEPRO 追加
//	case F_MENU_ALLFUNC				:/* コマンド一覧 */
//	case F_EXTHELP1					:/* 外部ヘルプ１ */
//	case F_EXTHTMLHELP				:/* 外部HTMLヘルプ */
//	case F_ABOUT					:/* バージョン情報 */		//Dec. 25, 2000 JEPRO 追加

	/* その他 */
//	case F_SENDMAIL					:/* メール送信 */
		return TRUE;
	}
	return FALSE;

}

/*!
	マクロ番号から対応するマクロオブジェクト格納位置へのポインタへの変換
	
	@param idx [in] マクロ番号(0-), STAND_KEYMACROは標準キーマクロバッファを表す．
	@return オブジェクト位置へのポインタ．マクロ番号が不当な場合はNULL．
*/
CMacroManagerBase** CSMacroMgr::Idx2Ptr(int idx)
{
	//	Jun. 16, 2002 genta
	//	キーマクロ以外のマクロを読み込めるように
	if ( idx == STAND_KEYMACRO ){
		return &m_pKeyMacro;
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return &m_cSavedKeyMacro[idx];
	}

#ifdef _DEBUG
	MYTRACE( "CSMacroMgr::Idx2Ptr() Out of range: idx=%d\n", idx);
#endif

	return NULL;
}

/*!
	キーボードマクロの保存が可能かどうか
	
	@retval TRUE 保存可能
	@retval FALSE 保存不可
*/
BOOL CSMacroMgr::IsSaveOk(void)
{
	return dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro ) == NULL ? FALSE : TRUE;
}
/*[EOF]*/
