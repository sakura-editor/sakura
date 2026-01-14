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
	Copyright (C) 2006, かろと, fon, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "macro/CSMacroMgr.h"
#include "macro/CPPAMacroMgr.h"
#include "macro/CWSHManager.h"
#include "macro/CPythonMacroManager.h"
#include "macro/CMacroFactory.h"
#include "env/CShareData.h"
#include "view/CEditView.h"
#include "debug/CRunningTimer.h"

VARTYPE s_MacroArgEx_i[] = {VT_I4};
MacroFuncInfoEx s_MacroInfoEx_i = {5, 5, s_MacroArgEx_i};
VARTYPE s_MacroArgEx_ii[] = {VT_I4, VT_I4};
MacroFuncInfoEx s_MacroInfoEx_ii = {6, 6, s_MacroArgEx_ii};
#if 0
VARTYPE s_MacroArgEx_s[] = {VT_BSTR};
MacroFuncInfoEx s_MacroInfoEx_s = {5, 5, s_MacroArgEx_s};
#endif

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoCommandArr[] = 
{
//	機能番号			関数名			引数				作業用バッファ

	/* ファイル操作系 */
	{F_FILENEW,						L"FileNew",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //新規作成
	// {F_FILEOPEN,					L"FileOpen",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //開く
	{F_FILEOPEN2,					L"FileOpen",				{VT_BSTR,  VT_I4,    VT_I4,    VT_BSTR},	VT_EMPTY,	nullptr}, //開く2
	{F_FILESAVE,					L"FileSave",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //上書き保存
	{F_FILESAVEALL,					L"FileSaveAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //上書き保存
	{F_FILESAVEAS_DIALOG,			L"FileSaveAsDialog",		{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, //名前を付けて保存(ダイアログ) 2013.05.02
	{F_FILESAVEAS,					L"FileSaveAs",			{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, //名前を付けて保存
	{F_FILECLOSE,					L"FileClose",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	{F_FILECLOSE_OPEN,				L"FileCloseOpen",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //閉じて開く
	{F_FILE_REOPEN,					L"FileReopen",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //開き直す	//Dec. 4, 2002 genta
	{F_FILE_REOPEN_SJIS,			L"FileReopenSJIS",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //SJISで開き直す
	{F_FILE_REOPEN_JIS,				L"FileReopenJIS",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //JISで開き直す
	{F_FILE_REOPEN_EUC,				L"FileReopenEUC",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //EUCで開き直す
	{F_FILE_REOPEN_LATIN1,			L"FileReopenLatin1",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //Latin1で開き直す	// 2010/3/20 Uchi
	{F_FILE_REOPEN_UNICODE,			L"FileReopenUNICODE",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //Unicodeで開き直す
	{F_FILE_REOPEN_UNICODEBE,		L"FileReopenUNICODEBE",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //UnicodeBEで開き直す
	{F_FILE_REOPEN_UTF8,			L"FileReopenUTF8",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //UTF-8で開き直す
	{F_FILE_REOPEN_CESU8,			L"FileReopenCESU8",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //CESU-8で開き直す
	{F_FILE_REOPEN_UTF7,			L"FileReopenUTF7",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //UTF-7で開き直す
	{F_PRINT,						L"Print",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //印刷
//	{F_PRINT_DIALOG,				L"PrintDialog",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //印刷ダイアログ
	{F_PRINT_PREVIEW,				L"PrintPreview",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //印刷プレビュー
	{F_PRINT_PAGESETUP,				L"PrintPageSetup",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	{F_OPEN_HfromtoC,				L"OpenHfromtoC",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //同名のC/C++ヘッダー(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
//	{F_OPEN_HHPP,					L"OpenHHpp",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //同名のC/C++ヘッダーファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
//	{F_OPEN_CCPP,					L"OpenCCpp",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
	{F_ACTIVATE_SQLPLUS,			L"ActivateSQLPLUS",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* Oracle SQL*Plusをアクティブ表示 */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	L"ExecSQLPLUS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* Oracle SQL*Plusで実行 */
	{F_BROWSE,						L"Browse",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ブラウズ
	{F_VIEWMODE,					L"ViewMode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ビューモード
	{F_VIEWMODE,					L"ReadOnly",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ビューモード(旧)
	{F_PROPERTY_FILE,				L"PropertyFile",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ファイルのプロパティ
	{F_EXITALLEDITORS,				L"ExitAllEditors",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //編集の全終了	// 2007.02.13 ryoji 追加
	{F_EXITALL,						L"ExitAll",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
	{F_PUTFILE,						L"PutFile",				{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},   VT_EMPTY,	nullptr}, // 作業中ファイルの一時出力 2006.12.10 maru
	{F_INSFILE,						L"InsFile",				{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},   VT_EMPTY,	nullptr}, // キャレット位置にファイル挿入 2006.12.10 maru

	/* 編集系 */
	{F_WCHAR,				L"Char",					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //文字入力
	{F_IME_CHAR,			L"CharIme",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //全角文字入力
	{F_UNDO,				L"Undo",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //元に戻す(Undo)
	{F_REDO,				L"Redo",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //やり直し(Redo)
	{F_DELETE,				L"Delete",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //削除
	{F_DELETE_BACK,			L"DeleteBack",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル前を削除
	{F_WordDeleteToStart,	L"WordDeleteToStart",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語の左端まで削除
	{F_WordDeleteToEnd,		L"WordDeleteToEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語の右端まで削除
	{F_WordCut,				L"WordCut",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語切り取り
	{F_WordDelete,			L"WordDelete",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語削除
	{F_LineCutToStart,		L"LineCutToStart",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行頭まで切り取り(改行単位)
	{F_LineCutToEnd,		L"LineCutToEnd",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行末まで切り取り(改行単位)
	{F_LineDeleteToStart,	L"LineDeleteToStart",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行頭まで削除(改行単位)
	{F_LineDeleteToEnd,		L"LineDeleteToEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行末まで削除(改行単位)
	{F_CUT_LINE,			L"CutLine",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行切り取り(折り返し単位)
	{F_DELETE_LINE,			L"DeleteLine",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行削除(折り返し単位)
	{F_DUPLICATELINE,		L"DuplicateLine",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行の二重化(折り返し単位)
	{F_INDENT_TAB,			L"IndentTab",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //TABインデント
	{F_UNINDENT_TAB,		L"UnindentTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //逆TABインデント
	{F_INDENT_SPACE,		L"IndentSpace",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //SPACEインデント
	{F_UNINDENT_SPACE,		L"UnindentSpace",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //逆SPACEインデント
//	{F_WORDSREFERENCE,		L"WordReference",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語リファレンス
	{F_LTRIM,				L"LTrim",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //左(先頭)の空白を削除 2001.12.03 hor
	{F_RTRIM,				L"RTrim",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //右(末尾)の空白を削除 2001.12.03 hor
	{F_SORT_ASC,			L"SortAsc",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択行の昇順ソート 2001.12.06 hor
	{F_SORT_DESC,			L"SortDesc",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択行の降順ソート 2001.12.06 hor
	{F_MERGE,				L"Merge",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択行のマージ 2001.12.06 hor

	/* カーソル移動系 */
	{F_UP,					L"Up",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル上移動
	{F_DOWN,				L"Down",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル下移動
	{F_LEFT,				L"Left",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル左移動
	{F_RIGHT,				L"Right",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル右移動
	{F_UP2,					L"Up2",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル上移動(２行ごと)
	{F_DOWN2,				L"Down2",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル下移動(２行ごと)
	{F_WORDLEFT,			L"WordLeft",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語の左端に移動
	{F_WORDRIGHT,			L"WordRight",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //単語の右端に移動
	{F_GOLINETOP,			L"GoLineTop",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行頭に移動(折り返し単位/改行単位)
	{F_GOLINEEND,			L"GoLineEnd",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //行末に移動(折り返し単位)
	{F_HalfPageUp,			L"HalfPageUp",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown,		L"HalfPageDown",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp,				L"PageUp",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageUp,				L"1PageUp",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown,			L"PageDown",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_1PageDown,			L"1PageDown",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP,			L"GoFileTop",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ファイルの先頭に移動
	{F_GOFILEEND,			L"GoFileEnd",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ファイルの最後に移動
	{F_CURLINECENTER,		L"CurLineCenter",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル行をウィンドウ中央へ
	{F_CURLINETOP,			L"CurLineTop",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル行をウィンドウ上部へ
	{F_CURLINEBOTTOM,		L"CurLineBottom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル行をウィンドウ下部へ
	{F_JUMPHIST_PREV,		L"MoveHistPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //移動履歴: 前へ
	{F_JUMPHIST_NEXT,		L"MoveHistNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //移動履歴: 次へ
	{F_JUMPHIST_SET,		L"MoveHistSet",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //現在位置を移動履歴に登録
	{F_WndScrollDown,		L"F_WndScrollDown",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //テキストを１行下へスクロール	// 2001/06/20 asa-o
	{F_WndScrollUp,			L"F_WndScrollUp",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //テキストを１行上へスクロール	// 2001/06/20 asa-o
	{F_GONEXTPARAGRAPH,		L"GoNextParagraph",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次の段落へ移動
	{F_GOPREVPARAGRAPH,		L"GoPrevParagraph",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前の段落へ移動
	{F_MODIFYLINE_NEXT,		L"GoModifyLineNext",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次の変更行へ移動
	{F_MODIFYLINE_PREV,		L"GoModifyLinePrev",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前の変更行へ移動
	{F_MOVECURSOR,			L"MoveCursor",		{VT_I4,    VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル移動
	{F_MOVECURSORLAYOUT,	L"MoveCursorLayout",	{VT_I4,    VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, //カーソル移動(レイアウト単位)
	{F_WHEELUP,				L"WheelUp",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイールアップ
	{F_WHEELDOWN,			L"WheelDown",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイールダウン
	{F_WHEELLEFT,			L"WheelLeft",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイール左
	{F_WHEELRIGHT,			L"WheelRight",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイール右
	{F_WHEELPAGEUP,			L"WheelPageUp",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイールページアップ
	{F_WHEELPAGEDOWN,		L"WheelPageDown",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイールページダウン
	{F_WHEELPAGELEFT,		L"WheelPageLeft",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイールページ左
	{F_WHEELPAGERIGHT,		L"WheelPageRight",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ホイールページ右

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	{F_SELECTWORD,			L"SelectWord",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //現在位置の単語選択
	{F_SELECTALL,			L"SelectAll",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //すべて選択
	{F_SELECTLINE,			L"SelectLine",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //1行選択	// 2007.10.13 nasukoji
	{F_BEGIN_SEL,			L"BeginSelect",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //範囲選択開始 Mar. 5, 2001 genta 名称修正
	{F_UP_SEL,				L"Up_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)カーソル上移動
	{F_DOWN_SEL,			L"Down_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)カーソル下移動
	{F_LEFT_SEL,			L"Left_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)カーソル左移動
	{F_RIGHT_SEL,			L"Right_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)カーソル右移動
	{F_UP2_SEL,				L"Up2_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)カーソル上移動(２行ごと)
	{F_DOWN2_SEL,			L"Down2_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)カーソル下移動(２行ごと)
	{F_WORDLEFT_SEL,		L"WordLeft_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)単語の左端に移動
	{F_WORDRIGHT_SEL,		L"WordRight_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)単語の右端に移動
	{F_GOLINETOP_SEL,		L"GoLineTop_Sel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)行頭に移動(折り返し単位/改行単位)
	{F_GOLINEEND_SEL,		L"GoLineEnd_Sel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)行末に移動(折り返し単位)
	{F_HalfPageUp_Sel,		L"HalfPageUp_Sel",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown_Sel,	L"HalfPageDown_Sel",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp_Sel,			L"PageUp_Sel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageUp_Sel,			L"1PageUp_Sel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown_Sel,		L"PageDown_Sel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_1PageDown_Sel,		L"1PageDown_Sel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP_SEL,		L"GoFileTop_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)ファイルの先頭に移動
	{F_GOFILEEND_SEL,		L"GoFileEnd_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)ファイルの最後に移動
	{F_GONEXTPARAGRAPH_SEL,	L"GoNextParagraph_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)次の段落へ移動
	{F_GOPREVPARAGRAPH_SEL,	L"GoPrevParagraph_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)前の段落へ移動
	{F_MODIFYLINE_NEXT_SEL,	L"GoModifyLineNext_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)次の変更行へ移動
	{F_MODIFYLINE_PREV_SEL,	L"GoModifyLinePrev_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(範囲選択)前の変更行へ移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	{F_BEGIN_BOX,			L"BeginBoxSelect",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //矩形範囲選択開始
	{F_UP_BOX,				L"Up_BoxSel",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)カーソル上移動
	{F_DOWN_BOX,			L"Down_BoxSel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)カーソル下移動
	{F_LEFT_BOX,			L"Left_BoxSel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)カーソル左移動
	{F_RIGHT_BOX,			L"Right_BoxSel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)カーソル右移動
	{F_UP2_BOX,				L"Up2_BoxSel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)カーソル上移動(２行ごと)
	{F_DOWN2_BOX,			L"Down2_BoxSel",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)カーソル下移動(２行ごと)
	{F_WORDLEFT_BOX,		L"WordLeft_BoxSel",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)単語の左端に移動
	{F_WORDRIGHT_BOX,		L"WordRight_BoxSel",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)単語の右端に移動
	{F_GOLOGICALLINETOP_BOX,L"GoLogicalLineTop_BoxSel",{VT_I4,   VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)行頭に移動(改行単位)
	{F_GOLINETOP_BOX,		L"GoLineTop_BoxSel",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)行頭に移動(折り返し単位/改行単位)
	{F_GOLINEEND_BOX,		L"GoLineEnd_BoxSel",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)行末に移動(折り返し単位)
	{F_HalfPageUp_BOX,		L"HalfPageUp_BoxSel",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)半ページアップ
	{F_HalfPageDown_BOX,	L"HalfPageDown_BoxSel",	{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)半ページダウン
	{F_1PageUp_BOX,			L"PageUp_BoxSel",			{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)１ページアップ
	{F_1PageUp_BOX,			L"1PageUp_BoxSel",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)１ページアップ
	{F_1PageDown_BOX,		L"PageDown_BoxSel",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)１ページダウン
	{F_1PageDown_BOX,		L"1PageDown_BoxSel",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)１ページダウン
	{F_GOFILETOP_BOX,		L"GoFileTop_BoxSel",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)ファイルの先頭に移動
	{F_GOFILEEND_BOX,		L"GoFileEnd_BoxSel",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	{F_CUT,						L"Cut",						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //切り取り(選択範囲をクリップボードにコピーして削除)
	{F_COPY,					L"Copy",						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //コピー(選択範囲をクリップボードにコピー)
	{F_PASTE,					L"Paste",						{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //貼り付け(クリップボードから貼り付け)
	{F_COPY_ADDCRLF,			L"CopyAddCRLF",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //折り返し位置に改行をつけてコピー
	{F_COPY_CRLF,				L"CopyCRLF",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	{F_PASTEBOX,				L"PasteBox",					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //矩形貼り付け(クリップボードから矩形貼り付け)
	{F_INSBOXTEXT,				L"InsBoxText",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 矩形テキスト挿入
	{F_INSTEXT_W,				L"InsText",					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // テキストを貼り付け
	{F_ADDTAIL_W,				L"AddTail",					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 最後にテキストを追加
	{F_COPYLINES,				L"CopyLines",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択範囲内全行コピー
	{F_COPYLINESASPASSAGE,		L"CopyLinesAsPassage",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択範囲内全行引用符付きコピー
	{F_COPYLINESWITHLINENUMBER,	L"CopyLinesWithLineNumber",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択範囲内全行行番号付きコピー
	{F_COPY_COLOR_HTML,			L"CopyColorHtml",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択範囲内色付きHTMLコピー
	{F_COPY_COLOR_HTML_LINENUMBER,	L"CopyColorHtmlWithLineNumber",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //選択範囲内行番号色付きHTMLコピー
	{F_COPYPATH,				L"CopyPath",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //このファイルのパス名をクリップボードにコピー
	{F_COPYDIRPATH,				L"CopyDirPath",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //このファイルのフォルダー名をクリップボードにコピー
	{F_COPYFNAME,				L"CopyFilename",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //このファイル名をクリップボードにコピー // 2002/2/3 aroka
	{F_COPYTAG,					L"CopyTag",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	{F_CREATEKEYBINDLIST,		L"CopyKeyBindList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	{F_INS_DATE,				L"InsertDate",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 日付挿入
	{F_INS_TIME,				L"InsertTime",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 時刻挿入
	{F_CTRL_CODE_DIALOG,		L"CtrlCodeDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK
	{F_CTRL_CODE,				L"CtrlCode",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //コントロールコードの入力 2013.12.12
	{F_INS_FILE_USED_RECENTLY,	L"InsertFileUsedRecently",{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 最近使ったファイル挿入
	{F_INS_FOLDER_USED_RECENTLY,L"InsertFolderUsedRecently",{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 最近使ったフォルダー挿入

	/* 変換系 */
	{F_TOLOWER,		 			L"ToLower",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //小文字
	{F_TOUPPER,		 			L"ToUpper",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //大文字
	{F_TOHANKAKU,		 		L"ToHankaku",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 全角→半角 */
	{F_TOHANKATA,		 		L"ToHankata",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	{F_TOZENEI,		 			L"ToZenEi",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			L"ToHanEi",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 全角英数→半角英数 */
	{F_TOZENKAKUKATA,	 		L"ToZenKata",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	{F_TOZENKAKUHIRA,	 		L"ToZenHira",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	{F_HANKATATOZENKATA,	L"HanKataToZenKata",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 半角カタカナ→全角カタカナ */
	{F_HANKATATOZENHIRA,	L"HanKataToZenHira",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 半角カタカナ→全角ひらがな */
	{F_TABTOSPACE,				L"TABToSPACE",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* TAB→空白 */
	{F_SPACETOTAB,				L"SPACEToTAB",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 空白→TAB */ //---- Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		L"AutoToSJIS",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 自動判別→SJISコード変換 */
	{F_CODECNV_EMAIL,			L"JIStoSJIS",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //E-Mail(JIS→SJIS)コード変換
	{F_CODECNV_EUC2SJIS,		L"EUCtoSJIS",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //EUC→SJISコード変換
	{F_CODECNV_UNICODE2SJIS,	L"CodeCnvUNICODEtoSJIS",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //Unicode→SJISコード変換
	{F_CODECNV_UNICODEBE2SJIS,	L"CodeCnvUNICODEBEtoSJIS",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // UnicodeBE→SJISコード変換
	{F_CODECNV_UTF82SJIS,		L"UTF8toSJIS",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* UTF-8→SJISコード変換 */
	{F_CODECNV_UTF72SJIS,		L"UTF7toSJIS",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* UTF-7→SJISコード変換 */
	{F_CODECNV_SJIS2JIS,		L"SJIStoJIS",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* SJIS→JISコード変換 */
	{F_CODECNV_SJIS2EUC,		L"SJIStoEUC",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* SJIS→EUCコード変換 */
	{F_CODECNV_SJIS2UTF8,		L"SJIStoUTF8",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* SJIS→UTF-8コード変換 */
	{F_CODECNV_SJIS2UTF7,		L"SJIStoUTF7",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* SJIS→UTF-7コード変換 */
	{F_BASE64DECODE,	 		L"Base64Decode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //Base64デコードして保存
	{F_UUDECODE,		 		L"Uudecode",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
	{F_SEARCH_DIALOG,			L"SearchDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //検索(単語検索ダイアログ)
	{F_SEARCH_NEXT,				L"SearchNext",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次を検索
	{F_SEARCH_PREV,				L"SearchPrev",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前を検索
	{F_REPLACE_DIALOG,			L"ReplaceDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //置換(置換ダイアログ)
	{F_REPLACE,					L"Replace",			{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, //置換(実行)
	{F_REPLACE_ALL,				L"ReplaceAll",		{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, //すべて置換(実行)
	{F_SEARCH_CLEARMARK,		L"SearchClearMark",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //検索マークのクリア
	{F_JUMP_SRCHSTARTPOS,		L"SearchStartPos",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //検索開始位置へ戻る			// 02/06/26 ai
	{F_GREP,					L"Grep",				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_I4   },	VT_EMPTY,	&s_MacroInfoEx_i}, //Grep
	{F_GREP_REPLACE,			L"GrepReplace",		{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_BSTR },	VT_EMPTY,	&s_MacroInfoEx_ii}, //Grep置換
	{F_JUMP,					L"Jump",				{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //指定行ヘジャンプ
	{F_OUTLINE,					L"Outline",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //アウトライン解析
	{F_TAGJUMP,					L"TagJump",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //タグジャンプ機能
	{F_TAGJUMPBACK,				L"TagJumpBack",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //タグジャンプバック機能
	{F_TAGS_MAKE,				L"TagMake",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //タグファイルの作成	//@@@ 2003.04.13 MIK
	{F_DIRECT_TAGJUMP,			L"DirectTagJump",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ダイレクトタグジャンプ機能	//@@@ 2003.04.15 MIK
	{F_TAGJUMP_KEYWORD,			L"KeywordTagJump",	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //キーワードを指定してダイレクトタグジャンプ機能 //@@@ 2005.03.31 MIK
	{F_COMPARE,					L"Compare",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ファイル内容比較
	{F_DIFF_DIALOG,				L"DiffDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
	{F_DIFF,					L"Diff",				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //DIFF差分表示				//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	{F_DIFF_NEXT,				L"DiffNext",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //DIFF差分表示(次へ)			//@@@ 2002.05.25 MIK
	{F_DIFF_PREV,				L"DiffPrev",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //DIFF差分表示(前へ)			//@@@ 2002.05.25 MIK
	{F_DIFF_RESET,				L"DiffReset",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //DIFF差分表示(全解除)		//@@@ 2002.05.25 MIK
	{F_BRACKETPAIR,				L"BracketPair",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //対括弧の検索
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			L"BookmarkSet",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ブックマーク設定・解除
	{F_BOOKMARK_NEXT,			L"BookmarkNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次のブックマークへ
	{F_BOOKMARK_PREV,			L"BookmarkPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前のブックマークへ
	{F_BOOKMARK_RESET,			L"BookmarkReset",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ブックマークの全解除
	{F_BOOKMARK_VIEW,			L"BookmarkView",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ブックマークの一覧
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		L"BookmarkPattern",	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // 2002.01.16 hor 指定パターンに一致する行をマーク
	{F_FUNCLIST_NEXT,			L"FuncListNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次の関数リストマークへ
	{F_FUNCLIST_PREV,			L"FuncListPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前の関数リストマークへ

	/* モード切り替え系 */
	{F_CHGMOD_INS,				L"ChgmodINS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //挿入／上書きモード切り替え
	{F_CHG_CHARSET,				L"ChgCharSet",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //文字コードセット指定		2010/6/14 Uchi
	{F_CHGMOD_EOL,				L"ChgmodEOL",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //入力改行コード指定 2003.06.23 Moca
	{F_CANCEL_MODE,				L"CancelMode",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //各種モードの取り消し

	/* マクロ系 */
	{F_EXECEXTMACRO,			L"ExecExternalMacro",	{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //名前を指定してマクロ実行

	/* 設定系 */
	{F_SHOWTOOLBAR,				L"ShowToolbar",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* ツールバーの表示 */
	{F_SHOWFUNCKEY,				L"ShowFunckey",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* ファンクションキーの表示 */
	{F_SHOWTAB,					L"ShowTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* タブの表示 */	//@@@ 2003.06.10 MIK
	{F_SHOWSTATUSBAR,			L"ShowStatusbar",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* ステータスバーの表示 */
	{F_SHOWMINIMAP,				L"ShowMiniMap",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // ミニマップの表示
	{F_TYPE_LIST,				L"TypeList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* タイプ別設定一覧 */
	{F_CHANGETYPE,				L"ChangeType",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //タイプ別設定一時適用 2013.05.02
	{F_OPTION_TYPE,				L"OptionType",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* タイプ別設定 */
	{F_OPTION,					L"OptionCommon",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 共通設定 */
	{F_FONT,					L"SelectFont",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* フォント設定 */
	{F_SETFONTSIZE,				L"SetFontSize",		{VT_I4,    VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	nullptr}, /* フォントサイズ設定 */
	{F_WRAPWINDOWWIDTH,			L"WrapWindowWidth",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	{F_FAVORITE,				L"OptionFavorite",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 履歴の管理 */	//@@@ 2003.04.08 MIK
	{F_SET_QUOTESTRING,			L"SetMsgQuoteStr",	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 共通設定→書式→引用符の設定 */	//Jan. 29, 2005 genta
	{F_TEXTWRAPMETHOD,			L"TextWrapMethod",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* テキストの折り返し方法 */	// 2008.05.30 nasukoji
	{F_SELECT_COUNT_MODE,		L"SelectCountMode",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //文字カウント方法
	//	Oct. 9, 2001 genta 追加
	{F_EXECMD,					L"ExecCommand",		{VT_BSTR,  VT_I4,    VT_BSTR,  VT_EMPTY},	VT_EMPTY,	nullptr}, /* 外部コマンド実行 */
	{F_EXECMD_DIALOG,			L"ExecCommandDialog",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //外部コマンド実行(ダイアログ)

	/* カスタムメニュー */
	{F_MENU_RBUTTON,			L"RMenu",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 右クリックメニュー */
	{F_CUSTMENU_1,				L"CustMenu1",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー1 */
	{F_CUSTMENU_2,				L"CustMenu2",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー2 */
	{F_CUSTMENU_3,				L"CustMenu3",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー3 */
	{F_CUSTMENU_4,				L"CustMenu4",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー4 */
	{F_CUSTMENU_5,				L"CustMenu5",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー5 */
	{F_CUSTMENU_6,				L"CustMenu6",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー6 */
	{F_CUSTMENU_7,				L"CustMenu7",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー7 */
	{F_CUSTMENU_8,				L"CustMenu8",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー8 */
	{F_CUSTMENU_9,				L"CustMenu9",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー9 */
	{F_CUSTMENU_10,				L"CustMenu10",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー10 */
	{F_CUSTMENU_11,				L"CustMenu11",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー11 */
	{F_CUSTMENU_12,				L"CustMenu12",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー12 */
	{F_CUSTMENU_13,				L"CustMenu13",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー13 */
	{F_CUSTMENU_14,				L"CustMenu14",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー14 */
	{F_CUSTMENU_15,				L"CustMenu15",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー15 */
	{F_CUSTMENU_16,				L"CustMenu16",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー16 */
	{F_CUSTMENU_17,				L"CustMenu17",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー17 */
	{F_CUSTMENU_18,				L"CustMenu18",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー18 */
	{F_CUSTMENU_19,				L"CustMenu19",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー19 */
	{F_CUSTMENU_20,				L"CustMenu20",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー20 */
	{F_CUSTMENU_21,				L"CustMenu21",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー21 */
	{F_CUSTMENU_22,				L"CustMenu22",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー22 */
	{F_CUSTMENU_23,				L"CustMenu23",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー23 */
	{F_CUSTMENU_24,				L"CustMenu24",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* カスタムメニュー24 */

	/* ウィンドウ系 */
	{F_SPLIT_V,					L"SplitWinV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	{F_SPLIT_H,					L"SplitWinH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	{F_SPLIT_VH,				L"SplitWinVH",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	{F_WINCLOSE,				L"WinClose",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ウィンドウを閉じる
	{F_WIN_CLOSEALL,			L"WinCloseAll",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	{F_CASCADE,					L"CascadeWin",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //重ねて表示
	{F_TILE_V,					L"TileWinV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //上下に並べて表示
	{F_TILE_H,					L"TileWinH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //左右に並べて表示
	{F_NEXTWINDOW,				L"NextWindow",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次のウィンドウ
	{F_PREVWINDOW,				L"PrevWindow",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前のウィンドウ
	{F_WINLIST,					L"WindowList",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
	{F_MAXIMIZE_V,				L"MaximizeV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //縦方向に最大化
	{F_MAXIMIZE_H,				L"MaximizeH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //横方向に最大化 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			L"MinimizeAll",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	{F_REDRAW,					L"ReDraw",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //再描画
	{F_WIN_OUTPUT,				L"ActivateWinOutput",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //アウトプットウィンドウ表示
	{F_TRACEOUT,				L"TraceOut",			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //マクロ用アウトプットウィンドウに出力	2006.04.26 maru
	{F_TOPMOST,					L"WindowTopMost",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //常に手前に表示
	{F_GROUPCLOSE,				L"GroupClose",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //グループを閉じる	// 2007.06.20 ryoji
	{F_NEXTGROUP,				L"NextGroup",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次のグループ	// 2007.06.20 ryoji
	{F_PREVGROUP,				L"PrevGroup",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前のグループ	// 2007.06.20 ryoji
	{F_TAB_MOVERIGHT,			L"TabMoveRight",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //タブを右に移動	// 2007.06.20 ryoji
	{F_TAB_MOVELEFT,			L"TabMoveLeft",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //タブを左に移動	// 2007.06.20 ryoji
	{F_TAB_SEPARATE,			L"TabSeparate",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //新規グループ	// 2007.06.20 ryoji
	{F_TAB_JOINTNEXT,			L"TabJointNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //次のグループに移動	// 2007.06.20 ryoji
	{F_TAB_JOINTPREV,			L"TabJointPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //前のグループに移動	// 2007.06.20 ryoji
	{F_TAB_CLOSEOTHER,			L"TabCloseOther",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //このタブ以外を閉じる	// 2010/3/14 Uchi
	{F_TAB_CLOSELEFT,			L"TabCloseLeft",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //左をすべて閉じる		// 2010/3/14 Uchi
	{F_TAB_CLOSERIGHT,			L"TabCloseRight",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //右をすべて閉じる		// 2010/3/14 Uchi

	/* 支援 */
	{F_HOKAN,					L"Complete",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 入力補完 */	//Oct. 15, 2000 JEPRO 入ってなかったので英名を付けて入れてみた
	{F_TOGGLE_KEY_SEARCH,		L"ToggleKeyHelpSearch", {VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //キーワードヘルプ自動表示 2013.05.03
	{F_HELP_CONTENTS,			L"HelpContents",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	{F_HELP_SEARCH,				L"HelpSearch",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	{F_MENU_ALLFUNC,			L"CommandList",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* コマンド一覧 */
	{F_EXTHELP1,				L"ExtHelp1",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 外部ヘルプ１ */
	//	Jul. 5, 2002 genta 引数追加
	{F_EXTHTMLHELP,				L"ExtHtmlHelp",		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* 外部HTMLヘルプ */
	{F_ABOUT,					L"About",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, /* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/*マクロ用*/
	{F_STATUSMSG,				L"StatusMsg",			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //ステータスメッセージ
	{F_MSGBEEP,					L"MsgBeep",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, //Beep音
	{F_COMMITUNDOBUFFER,		L"CommitUndoBuffer",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr }, //OpeBlKコミット
	{F_ADDREFUNDOBUFFER,		L"AddRefUndoBuffer",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr }, //OpeBlK AddRef
	{F_SETUNDOBUFFER,			L"SetUndoBuffer",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr }, //OpeBlK Release
	{F_APPENDUNDOBUFFERCURSOR,	L"AppendUndoBufferCursor",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr }, //OpeBlK にカーソル位置を追加
	{F_CLIPBOARDEMPTY,			L"ClipboardEmpty",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr},
	{F_SETVIEWTOP,				L"SetViewTop",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // ビューの上の行数を設定
	{F_SETVIEWLEFT,				L"SetViewLeft",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}, // ビューの左端の桁数を設定

	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{F_INVALID,	nullptr, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
	//ID					関数名							引数										戻り値の型	m_pszData
	{F_GETFILENAME,			L"GetFilename",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //ファイル名を返す
	{F_GETSAVEFILENAME,		L"GetSaveFilename",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //保存時のファイル名を返す 2006.09.04 ryoji
	{F_GETSELECTED,			L"GetSelectedString",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //選択部分
	{F_EXPANDPARAMETER,		L"ExpandParameter",		{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //特殊文字の展開
	{F_GETLINESTR,			L"GetLineStr",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, // 指定論理行の取得 2003.06.01 Moca
	{F_GETLINECOUNT,		L"GetLineCount",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 全論理行数の取得 2003.06.01 Moca
	{F_CHGTABWIDTH,			L"ChangeTabWidth",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //タブサイズ変更 2004.03.16 zenryaku
	{F_ISTEXTSELECTED,		L"IsTextSelected",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //テキストが選択されているか 2005.7.30 maru
	{F_GETSELLINEFROM,		L"GetSelectLineFrom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 選択開始行の取得 2005.7.30 maru
	{F_GETSELCOLUMNFROM,	L"GetSelectColmFrom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 選択開始桁の取得 2005.7.30 maru
	{F_GETSELCOLUMNFROM,	L"GetSelectColumnFrom",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 選択開始桁の取得 2005.7.30 maru
	{F_GETSELLINETO,		L"GetSelectLineTo",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 選択終了行の取得 2005.7.30 maru
	{F_GETSELCOLUMNTO,		L"GetSelectColmTo",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 選択終了桁の取得 2005.7.30 maru
	{F_GETSELCOLUMNTO,		L"GetSelectColumnTo",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 選択終了桁の取得 2005.7.30 maru
	{F_ISINSMODE,			L"IsInsMode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 挿入／上書きモードの取得 2005.7.30 maru
	{F_GETCHARCODE,			L"GetCharCode",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 文字コード取得 2005.07.31 maru
	{F_GETLINECODE,			L"GetLineCode",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 改行コード取得 2005.08.05 maru
	{F_ISPOSSIBLEUNDO,		L"IsPossibleUndo",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // Undo可能か調べる 2005.08.05 maru
	{F_ISPOSSIBLEREDO,		L"IsPossibleRedo",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // Redo可能か調べる 2005.08.05 maru
	{F_CHGWRAPCOLUMN,		L"ChangeWrapColm",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //折り返し桁変更 2008.06.19 ryoji
	{F_CHGWRAPCOLUMN,		L"ChangeWrapColumn",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //折り返し桁変更 2008.06.19 ryoji
	{F_ISCURTYPEEXT,		L"IsCurTypeExt",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
	{F_ISSAMETYPEEXT,		L"IsSameTypeExt",			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // ２つの拡張子が同じタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
	{F_INPUTBOX,			L"InputBox",				{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_BSTR,	nullptr }, //テキスト入力ダイアログの表示
	{F_MESSAGEBOX,			L"MessageBox",			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メッセージボックスの表示
	{F_ERRORMSG,			L"ErrorMsg",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メッセージボックス（エラー）の表示
	{F_WARNMSG,				L"WarnMsg",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メッセージボックス（警告）の表示
	{F_INFOMSG,				L"InfoMsg",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メッセージボックス（情報）の表示
	{F_OKCANCELBOX,			L"OkCancelBox",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メッセージボックス（確認：OK／キャンセル）の表示
	{F_YESNOBOX,			L"YesNoBox",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メッセージボックス（確認：はい／いいえ）の表示
	{F_COMPAREVERSION,		L"CompareVersion",		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //バージョン番号の比較
	{F_MACROSLEEP,			L"Sleep",					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //指定した時間（ミリ秒）停止
	{F_FILEOPENDIALOG,		L"FileOpenDialog",		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //ファイルを開くダイアログの表示
	{F_FILESAVEDIALOG,		L"FileSaveDialog",		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //ファイルを保存ダイアログの表示
	{F_FOLDERDIALOG,		L"FolderDialog",			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //フォルダーを開くダイアログの表示
	{F_GETCLIPBOARD,		L"GetClipboard",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //クリップボードの文字列を取得
	{F_SETCLIPBOARD,		L"SetClipboard",			{VT_I4,    VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //クリップボードに文字列を設定
	{F_LAYOUTTOLOGICLINENUM,L"LayoutToLogicLineNum",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //ロジック行番号取得
	{F_LOGICTOLAYOUTLINENUM,L"LogicToLayoutLineNum",	{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //レイアウト行番号取得
	{F_LINECOLUMNTOINDEX,	L"LineColumnToIndex",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //ロジック桁番号取得
	{F_LINEINDEXTOCOLUMN,	L"LineIndexToColumn",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //レイアウト桁番号取得
	{F_GETCOOKIE,			L"GetCookie",				{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //Cookie取得
	{F_GETCOOKIEDEFAULT,	L"GetCookieDefault",		{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_EMPTY},	VT_BSTR,	nullptr }, //Cookie取得デフォルト値
	{F_SETCOOKIE,			L"SetCookie",				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_EMPTY},	VT_I4,		nullptr }, //Cookie設定
	{F_DELETECOOKIE,		L"DeleteCookie",			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //Cookie削除
	{F_GETCOOKIENAMES,		L"GetCookieNames",		{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, //Cookie名前取得
	{F_SETDRAWSWITCH,		L"SetDrawSwitch",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //再描画スイッチ設定
	{F_GETDRAWSWITCH,		L"GetDrawSwitch",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //再描画スイッチ取得
	{F_ISSHOWNSTATUS,		L"IsShownStatus",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //ステータスバーが表示されているか
	{F_GETSTRWIDTH,			L"GetStrWidth",			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //文字列幅取得
	{F_GETSTRLAYOUTLENGTH,	L"GetStrLayoutLength",	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //文字列のレイアウト幅取得
	{F_GETDEFAULTCHARLENGTH,	L"GetDefaultCharLength",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //デフォルト文字幅の取得
	{F_ISINCLUDECLIPBOARDFORMAT,L"IsIncludeClipboardFormat",{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //クリップボードの形式取得
	{F_GETCLIPBOARDBYFORMAT,	L"GetClipboardByFormat",	{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_BSTR,	nullptr }, //クリップボードの指定形式で取得
	{F_SETCLIPBOARDBYFORMAT,	L"SetClipboardByFormat",	{VT_BSTR,  VT_BSTR,  VT_I4,    VT_I4,    },	VT_I4,		nullptr }, //クリップボードの指定形式で設定
	{F_GETLINEATTRIBUTE,		L"GetLineAttribute",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //行属性取得
	{F_ISTEXTSELECTINGLOCK,		L"IsTextSelectingLock",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //選択状態のロックを取得
	{F_GETVIEWLINES,			L"GetViewLines",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //ビューの行数取得
	{F_GETVIEWCOLUMNS,			L"GetViewColumns",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //ビューの列数取得
	{F_CREATEMENU,				L"CreateMenu",				{VT_I4,    VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //メニュー作成
	{F_GETVIEWTOP,				L"GetViewTop",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, //画面に表示される一番上の行番号を取得
	{F_CHGINSSPACE,				L"ChangeInsertSpace",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		nullptr }, // SPACEの挿入の変更

	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{F_INVALID,	nullptr, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

/* static */ std::span<MacroFuncInfo> CSMacroMgr::GetCommandInfo() noexcept
{
	return std::span{ m_MacroFuncInfoCommandArr, std::size(m_MacroFuncInfoCommandArr) - 1 };
}

/* static */ std::span<MacroFuncInfo> CSMacroMgr::GetFuncInfo() noexcept
{
	return std::span{ m_MacroFuncInfoArr, std::size(m_MacroFuncInfoArr) - 1 };
}

/*!
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2002.04.29 genta オブジェクトの実体は実行時まで生成しない。
*/
CSMacroMgr::CSMacroMgr()
{
	MY_RUNNINGTIMER( cRunningTimer, L"CSMacroMgr::CSMacroMgr" );
	
	m_pShareData = &GetDllShareData();
	
	CPPAMacroMgr::declare();
	CKeyMacroMgr::declare();
	CWSHMacroManager::declare();
	CPythonMacroManager::declare();
	
	int i;
	for ( i = 0 ; i < MAX_CUSTMACRO ; i++ ){
		m_cSavedKeyMacro[i] = nullptr;
	}
	//	Jun. 16, 2002 genta
	m_pKeyMacro = nullptr;
	m_pTempMacro = nullptr;

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
		m_cSavedKeyMacro[i] = nullptr;
	}
	//	Jun. 16, 2002 genta
	delete m_pKeyMacro;
	m_pKeyMacro = nullptr;
	delete m_pTempMacro;
	m_pTempMacro = nullptr;
}

/*! @briefキーマクロのバッファにデータ追加

	@param mbuf [in] 読み込み先マクロバッファ
	
	@date 2002.06.16 genta キーマクロの多種対応のため変更
*/
int CSMacroMgr::Append(
	int				idx,		//!<
	EFunctionCode	nFuncID,	//!< [in] 機能番号
	const LPARAM*	lParams,	//!< [in] パラメータ。
	CEditView*		pcEditView	//!< 
)
{
	assert( idx == STAND_KEYMACRO );
	if (idx == STAND_KEYMACRO){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro == nullptr ){
			//	1. 実体がまだ無い場合
			//	2. CKeyMacroMgr以外の物が入っていた場合
			//	いずれにしても再生成する．
			delete m_pKeyMacro;
			m_pKeyMacro = new CKeyMacroMgr;
			pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		}
		pKeyMacro->Append( nFuncID, lParams, pcEditView );
	}
	return TRUE;
}

/*!	@brief キーボードマクロの実行

	CShareDataからファイル名を取得し、実行する。

	@param hInstance [in] インスタンス
	@param hwndParent [in] 親ウィンドウの
	@param pViewClass [in] macro実行対象のView
	@param idx [in] マクロ番号。
	@param flags [in] マクロ実行フラグ．HandleCommandに渡すオプション．

	@date 2007.07.16 genta flags追加
*/
BOOL CSMacroMgr::Exec( int idx , HINSTANCE hInstance, CEditView* pcEditView, int flags )
{
	if( idx == STAND_KEYMACRO ){
		//	Jun. 16, 2002 genta
		//	キーマクロ以外のサポートによりNULLの可能性が出てきたので判定追加
		if( m_pKeyMacro != nullptr ){
			//	Sep. 15, 2005 FILE
			//	Jul. 01, 2007 マクロの多重実行時に備えて直前のマクロ番号を退避
			int prevmacro = SetCurrentIdx( idx );
			m_pKeyMacro->ExecKeyMacro2( pcEditView, flags );
			SetCurrentIdx( prevmacro );
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	if( idx == TEMP_KEYMACRO ){		// 一時マクロ
		if( m_pTempMacro != nullptr ){
			int prevmacro = SetCurrentIdx( idx );
			m_pTempMacro->ExecKeyMacro2( pcEditView, flags );
			SetCurrentIdx( prevmacro );
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
	if( m_cSavedKeyMacro[idx] == nullptr || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareDataから、マクロファイル名を取得
		//	Jun. 08, 2003 Moca 呼び出し側でパス名を用意
		//	Jun. 16, 2003 genta 書式をちょっと変更
		WCHAR ptr[_MAX_PATH * 2];
		int n = CShareData::getInstance()->GetMacroFilename( idx, ptr, int(std::size(ptr)) );
		if ( n <= 0 ){
			return FALSE;
		}

		if( !Load( idx, hInstance, ptr, nullptr ) )
			return FALSE;
	}

	//	Sep. 15, 2005 FILE
	//	Jul. 01, 2007 マクロの多重実行時に備えて直前のマクロ番号を退避
	int prevmacro = SetCurrentIdx( idx );
	SetCurrentIdx( idx );
	m_cSavedKeyMacro[idx]->ExecKeyMacro2(pcEditView, flags);
	SetCurrentIdx( prevmacro );

	return TRUE;
}

/*! キーボードマクロの読み込み

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名、またはコード文字列
	@param pszType [in] 種別。NULLの場合ファイルから読み込む。NULL以外の場合は言語の拡張子

	読み込みに失敗したときはマクロバッファのオブジェクトは解放され，
	NULLが設定される．

	@author Norio Nakatani, YAZAKI, genta
*/
BOOL CSMacroMgr::Load( int idx, HINSTANCE hInstance, const WCHAR* pszPath, const WCHAR* pszType )
{
	CMacroManagerBase** ppMacro = Idx2Ptr( idx );

	if( ppMacro == nullptr ){
		DEBUG_TRACE( L"CSMacroMgr::Load() Out of range: idx=%d Path=%s\n", idx, pszPath);
	}

	//	バッファクリア
	delete *ppMacro;
	*ppMacro = nullptr;
	
	const WCHAR *ext;
	if( pszType == nullptr ){				//ファイル指定
		//ファイルの拡張子を取得する
		ext = wcsrchr( pszPath, L'.');
		//	Feb. 02, 2004 genta .が無い場合にext==NULLとなるのでNULLチェック追加
		if( ext != nullptr ){
			const WCHAR *chk = wcsrchr( ext, L'\\' );
			if( chk != nullptr ){	//	.のあとに\があったらそれは拡張子の区切りではない
								//	\が漢字の2バイト目の場合も拡張子ではない。
				ext = nullptr;
			}
		}
		if(ext != nullptr){
			++ext;
		}
	}else{								//コード指定
		ext = pszType;
	}

	m_sMacroPath.clear();
	*ppMacro = CMacroFactory::getInstance()->Create(ext);
	if( *ppMacro == nullptr )
		return FALSE;
	BOOL bRet;
	if( pszType == nullptr ){
		bRet = (*ppMacro)->LoadKeyMacro(hInstance, pszPath);
		if (idx == STAND_KEYMACRO || idx == TEMP_KEYMACRO) {
			m_sMacroPath = pszPath;
		}
	}else{
		bRet = (*ppMacro)->LoadKeyMacroStr(hInstance, pszPath);
	}

	//	From Here Jun. 16, 2002 genta
	//	読み込みエラー時はインスタンス削除
	if( bRet ){
		return TRUE;
	}
	else {
		delete *ppMacro;
		*ppMacro = nullptr;
	}
	//	To Here Jun. 16, 2002 genta
	return FALSE;
}

/** マクロオブジェクトをすべて破棄する(キーボードマクロ以外)

	マクロの登録を変更した場合に，変更前のマクロが
	引き続き実行されてしまうのを防ぐ．

	@date 2007.10.19 genta 新規作成
*/
void CSMacroMgr::UnloadAll(void)
{
	for ( int idx = 0; idx < MAX_CUSTMACRO; idx++ ){
		delete m_cSavedKeyMacro[idx];
		m_cSavedKeyMacro[idx] = nullptr;
	}
}

/*! キーボードマクロの保存

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名
	@param hInstance [in] インスタンスハンドル

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx, HINSTANCE hInstance, const WCHAR* pszPath )
{
	assert( idx == STAND_KEYMACRO );
	if ( idx == STAND_KEYMACRO ){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro != nullptr ){
			return pKeyMacro->SaveKeyMacro(hInstance, pszPath );
		}
		//	Jun. 27, 2002 genta
		//	空マクロの場合は正常終了と見なす．
		if( m_pKeyMacro == nullptr ){
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
	if( ppMacro != nullptr ){
		delete *ppMacro;
		*ppMacro = nullptr;
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
	@date 2003.02.24 m_MacroFuncInfoArrも検索対象にする
*/
const MacroFuncInfo* CSMacroMgr::GetFuncInfoByID( int nFuncID )
{
	if (nFuncID <= 0) {
		return nullptr;
	}

	const auto commands = GetCommandInfo();
	if (const auto cmdFound = std::ranges::find_if(commands, [nFuncID](const auto& funcInfo) { return funcInfo.m_nFuncID == nFuncID; }); cmdFound != commands.end()) {
		return std::to_address(cmdFound);
	}

	const auto functions = GetFuncInfo();
	if (const auto funcFound = std::ranges::find_if(functions, [nFuncID](const auto& funcInfo) { return funcInfo.m_nFuncID == nFuncID; }); funcFound != functions.end()) {
		return std::to_address(funcFound);
	}

	return nullptr;
}

/*!
	機能番号から関数名と機能名日本語を取得
	
	@return 成功したときはpszFuncName．見つからなかったときはNULL．
	
	@note
	それぞれ，文字列格納領域の指す先がNULLの時は文字列を格納しない．
	ただし，pszFuncNameをNULLにしてしまうと戻り値が常にNULLになって
	成功判定が行えなくなる．
	各国語メッセージリソース対応により機能名が日本語でない場合がある	

	@date 2002.06.16 genta 新設のGetFuncInfoById(int)を内部で使うように．
	@date 2011.04.10 nasukoji 各国語メッセージリソース対応
*/
LPWSTR CSMacroMgr::GetFuncInfoByID(
	int					nFuncID,			//!< [in] 機能番号
	std::span<WCHAR>	szFuncName,			//!< [out] 関数名．この先には最長関数名＋1バイトのメモリが必要．
	const std::optional<std::span<WCHAR>>& optFuncNameJapanese	//!< [out] 機能名日本語．NULL許容. この先には256バイトのメモリが必要．
)
{
	const auto MacroInfo = GetFuncInfoByID(nFuncID);
	if (!MacroInfo) {
		return nullptr;	//見付からなかった
	}

	// 見付かった機能名をコピー
	::wcsncpy_s(std::data(szFuncName), std::size(szFuncName), MacroInfo->m_pszFuncName, _TRUNCATE);

	// 機能名を含む場合、丸括弧の手前までに切り詰める
	WCHAR *p = std::data(szFuncName);
	while (*p){
		if (*p == L'('){
			*p = L'\0';
			break;
		}
		p++;
	}

	//	Jun. 16, 2002 genta NULLのときは何もしない．
	if (optFuncNameJapanese.has_value()) {
		auto& szFuncNameJapanese = optFuncNameJapanese.value();
		::wcsncpy_s(std::data(szFuncNameJapanese), std::size(szFuncNameJapanese), LS(nFuncID), _TRUNCATE);
	}
	return std::data(szFuncName);
}

/*!
	関数名（S_xxxx）から機能番号と機能名日本語を取得．
	関数名はS_で始まる場合と始まらない場合の両方に対応．

	@return 成功したときは機能番号．見つからなかったときは-1．
	
	@note
	pszFuncNameJapanese の指す先がNULLの時は日本語名を格納しない．
	
	@date 2002.06.16 genta ループ内の文字列コピーを排除
*/
EFunctionCode CSMacroMgr::GetFuncInfoByName(
	[[maybe_unused]] HINSTANCE		hInstance,				//!< [in]  リソース取得のためのInstance Handle
	const WCHAR*	pszFuncName,			//!< [in]  関数名
	WCHAR*			pszFuncNameJapanese		//!< [out] 機能名日本語．この先には256バイトのメモリが必要．
)
{
	//	Jun. 16, 2002 genta
	const WCHAR* normalizedFuncName;
	
	//	S_で始まっているか
	if( pszFuncName == nullptr ){
		return F_INVALID;
	}
	if( pszFuncName[0] == L'S' && pszFuncName[1] == L'_' ){
		normalizedFuncName = pszFuncName + 2;
	}
	else {
		normalizedFuncName = pszFuncName;
	}

	// コマンド関数を検索
	for (const auto& funcInfo : GetFuncInfo()) {
		if( 0 == wcscmp( normalizedFuncName, funcInfo.m_pszFuncName )){
			const auto nFuncID = EFunctionCode(funcInfo.m_nFuncID);
			if( pszFuncNameJapanese != nullptr ){
				::wcsncpy_s(pszFuncNameJapanese, 256, LS(nFuncID), _TRUNCATE);
			}
			return nFuncID;
		}
	}
	// 非コマンド関数を検索
	for (const auto& funcInfo : GetCommandInfo()) {
		if( 0 == wcscmp( normalizedFuncName, funcInfo.m_pszFuncName )){
			const auto nFuncID = EFunctionCode(funcInfo.m_nFuncID);
			if( pszFuncNameJapanese != nullptr ){
				::wcsncpy_s(pszFuncNameJapanese, 256, LS(nFuncID), _TRUNCATE);
			}
			return nFuncID;
		}
	}
	return F_INVALID;
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
	case F_FILE_REOPEN_LATIN1		://Latin1で開き直す	// 2010/3/20 Uchi
	case F_FILE_REOPEN_UNICODE		://Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE	://UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8			://UTF-8で開き直す
	case F_FILE_REOPEN_CESU8		://CESU-8で開き直す	// 2010/3/20 Uchi
	case F_FILE_REOPEN_UTF7			://UTF-7で開き直す
//	case F_PRINT					://印刷
//	case F_PRINT_DIALOG				://印刷ダイアログ
//	case F_PRINT_PREVIEW			://印刷プレビュー
//	case F_PRINT_PAGESETUP			://印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
//	case F_OPEN_HfromtoC:			://同名のC/C++ヘッダー(ソース)を開く	//Feb. 9, 2001 JEPRO 追加
//	case F_OPEN_HHPP				://同名のC/C++ヘッダーファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
//	case F_OPEN_CCPP				://同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
//	case F_ACTIVATE_SQLPLUS			:/* Oracle SQL*Plusをアクティブ表示 */
//	case F_PLSQL_COMPILE_ON_SQLPLUS	:/* Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
///	case F_BROWSE					://ブラウズ
//	case F_PROPERTY_FILE			://ファイルのプロパティ
//	case F_EXITALLEDITORS			://編集の全終了	// 2007.02.13 ryoji 追加
//	case F_EXITALL					://サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
//	case F_PUTFILE					://作業中ファイルの一時出力	2006.12.10 maru
//	case F_INSFILE					://キャレット位置にファイル挿入	2006.12.10 maru

	/* 編集系 */
	case F_WCHAR					://文字入力
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
	// 2014.01.15 (Half)Page[Up/down] を有効化
	case F_HalfPageUp				://半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown				://半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp					://１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown				://１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_UP2						://カーソル上移動(２行ごと)
	case F_DOWN2					://カーソル下移動(２行ごと)
	case F_GOLINETOP				://行頭に移動(折り返し単位)
	case F_GOLINEEND				://行末に移動(折り返し単位)
	case F_GOFILETOP				://ファイルの先頭に移動
	case F_GOFILEEND				://ファイルの最後に移動
	case F_WORDLEFT					://単語の左端に移動
	case F_WORDRIGHT				://単語の右端に移動
	case F_CURLINECENTER			://カーソル行をウィンドウ中央へ
	case F_CURLINETOP				://カーソル行をウィンドウ上部へ
	case F_CURLINEBOTTOM			://カーソル行をウィンドウ下部へ
	case F_JUMPHIST_PREV			://移動履歴: 前へ
	case F_JUMPHIST_NEXT			://移動履歴: 次へ
	case F_JUMPHIST_SET				://現在位置を移動履歴に登録
	case F_MODIFYLINE_NEXT			://次の変更行へ移動
	case F_MODIFYLINE_PREV			://前の変更行へ移動

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので独立化して(選択)を移動(サブメニュー化は構造上できないので)
	case F_SELECTWORD				://現在位置の単語選択
	case F_SELECTALL				://すべて選択
	case F_SELECTLINE				://1行選択	// 2007.10.06 nasukoji
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
	// 2014.01.15 (Half)Page[Up/down] を有効化
	case F_HalfPageUp_Sel			://(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown_Sel			://(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp_Sel				://(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown_Sel			://(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_GOFILETOP_SEL			://(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL			://(範囲選択)ファイルの最後に移動
	case F_MODIFYLINE_NEXT_SEL		://(範囲選択)次の変更行へ移動
	case F_MODIFYLINE_PREV_SEL		://(範囲選択)前の変更行へ移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
//	case F_BOXSELALL				//矩形ですべて選択
	case F_BEGIN_BOX				://矩形範囲選択開始

	case F_UP_BOX					://(矩形選択)カーソル上移動
	case F_DOWN_BOX					://(矩形選択)カーソル下移動
	case F_LEFT_BOX					://(矩形選択)カーソル左移動
	case F_RIGHT_BOX				://(矩形選択)カーソル右移動
	case F_UP2_BOX					://(矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX				://(矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX				://(矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX			://(矩形選択)単語の右端に移動
	case F_GOLOGICALLINETOP_BOX		://(矩形選択)行頭に移動(改行単位)
	case F_GOLINETOP_BOX			://(矩形選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_BOX			://(矩形選択)行末に移動(折り返し単位)
	case F_HalfPageUp_BOX			://(矩形選択)半ページアップ
	case F_HalfPageDown_BOX			://(矩形選択)半ページダウン
	case F_1PageUp_BOX				://(矩形選択)１ページアップ
	case F_1PageDown_BOX			://(矩形選択)１ページダウン
	case F_GOFILETOP_BOX			://(矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX			://(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	case F_CUT						://切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY						://コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF				://折り返し位置に改行をつけてコピー
	case F_COPY_CRLF				://CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	case F_PASTE					://貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX					://矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSTEXT_W					://テキストを貼り付け
//	case F_ADDTAIL_W					://最後にテキストを追加
	case F_COPYLINES				://選択範囲内全行コピー
	case F_COPYLINESASPASSAGE		://選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER 	://選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML			://選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER://選択範囲内行番号色付きHTMLコピー
	case F_COPYPATH					://このファイルのパス名をクリップボードにコピー
	case F_COPYDIRPATH				://このファイルのフォルダー名をクリップボードにコピー
	case F_COPYTAG					://このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	case F_COPYFNAME				://このファイル名をクリップボードにコピー // 2002/2/3 aroka
	case F_CREATEKEYBINDLIST		://キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加	//Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE					:// 日付挿入
	case F_INS_TIME					:// 時刻挿入
//	case F_CTRL_CODE_DIALOG			://コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK
	case F_CTRL_CODE				://コントロールコードの入力 2013.12.12
	case F_INS_FILE_USED_RECENTLY	:// 最近使ったファイル挿入
	case F_INS_FOLDER_USED_RECENTLY	:// 最近使ったフォルダー挿入

	/* 変換系 */
	case F_TOLOWER		 			://小文字
	case F_TOUPPER		 			://大文字
	case F_TOHANKAKU		 		:/* 全角→半角 */
	case F_TOHANKATA		 		:/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI			 		:/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI			 		:/* 全角英数→半角英数 */
	case F_TOZENKAKUKATA	 		:/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA	 		:/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKATA			:/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENHIRA			:/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE				:/* TAB→空白 */
	case F_SPACETOTAB				:/* 空白→TAB */  //---- Stonee, 2001/05/27
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
	case F_FUNCLIST_NEXT			://次の関数リストマークへ
	case F_FUNCLIST_PREV			://前の関数リストマークへ

	/* モード切り替え系 */
	case F_CHGMOD_INS				://挿入／上書きモード切り替え
	case F_CHG_CHARSET				://文字コードセット指定	2010/6/14 Uchi
	case F_CHGMOD_EOL				://入力改行コード指定	2003.06.23 Moca

	case F_CANCEL_MODE				://各種モードの取り消し

	/* マクロ系 */
//	case F_RECKEYMACRO				://キーマクロの記録開始／終了
//	case F_SAVEKEYMACRO				://キーマクロの保存
//	case F_LOADKEYMACRO				://キーマクロの読み込み
//	case F_EXECKEYMACRO				://キーマクロの実行
	case F_EXECEXTMACRO				://名前を指定してマクロ実行

	/* 設定系 */
//	case F_SHOWTOOLBAR				:/* ツールバーの表示 */
//	case F_SHOWFUNCKEY				:/* ファンクションキーの表示 */
//	case F_SHOWTAB					:/* タブの表示 */
//	case F_SHOWSTATUSBAR			:/* ステータスバーの表示 */
//	case F_TYPE_LIST				:/* タイプ別設定一覧 */
//	case F_OPTION_TYPE				:/* タイプ別設定 */
//	case F_OPTION					:/* 共通設定 */
//	case F_FONT						:/* フォント設定 */
	case F_SETFONTSIZE				:// フォントサイズ設定
//	case F_WRAPWINDOWWIDTH			:/* 現在のウィンドウ幅で折り返し */	//Oct. 15, 2000 JEPRO
//	case F_FAVORITE					:/* 履歴の管理 */	//@@@ 2003.04.08 MIK
//	case F_TMPWRAPNOWRAP			:// 折り返さない（一時設定）		// 2008.05.30 nasukoji
//	case F_TMPWRAPSETTING			:// 指定桁で折り返す（一時設定）	// 2008.05.30 nasukoji
//	case F_TMPWRAPWINDOW			:// 右端で折り返す（一時設定）		// 2008.05.30 nasukoji
	case F_TEXTWRAPMETHOD			:// テキストの折り返し方法			// 2008.05.30 nasukoji
	case F_SELECT_COUNT_MODE		:// 文字カウントの方法を取得、設定	// 2009.07.06 syat

	case F_EXECMD					:/* 外部コマンド実行 */	//@@@2002.2.2 YAZAKI 追加

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
//	case F_GROUPCLOSE				://グループを閉じる	// 2007.06.20 ryoji
//	case F_NEXTGROUP				://次のグループ	// 2007.06.20 ryoji
//	case F_PREVGROUP				://前のグループ	// 2007.06.20 ryoji
//	case F_TAB_MOVERIGHT			://タブを右に移動	// 2007.06.20 ryoji
//	case F_TAB_MOVELEFT				://タブを左に移動	// 2007.06.20 ryoji
//	case F_TAB_SEPARATE				://新規グループ	// 2007.06.20 ryoji
//	case F_TAB_JOINTNEXT			://次のグループに移動	// 2007.06.20 ryoji
//	case F_TAB_JOINTPREV			://前のグループに移動	// 2007.06.20 ryoji

	/* 支援 */
//  case F_HOKAN					:/* 入力補完 */				//Oct. 15, 2000 JEPRO 入ってなかったので入れてみた
//	case F_HELP_CONTENTS			:/* ヘルプ目次 */			//Dec. 25, 2000 JEPRO 追加
//	case F_HELP_SEARCH				:/* ヘルプキーワード検索 */	//Dec. 25, 2000 JEPRO 追加
//	case F_MENU_ALLFUNC				:/* コマンド一覧 */
//	case F_EXTHELP1					:/* 外部ヘルプ１ */
//	case F_EXTHTMLHELP				:/* 外部HTMLヘルプ */
//	case F_ABOUT					:/* バージョン情報 */		//Dec. 25, 2000 JEPRO 追加

	/* その他 */
		return TRUE;
	}
	return FALSE;
}

/*!
	マクロ番号から対応するマクロオブジェクト格納位置へのポインタへの変換
	
	@param idx [in] マクロ番号(0-), STAND_KEYMACROは標準キーマクロバッファ、TEMP_KEYMACROは一時マクロバッファを表す．
	@return オブジェクト位置へのポインタ．マクロ番号が不当な場合はNULL．
*/
CMacroManagerBase** CSMacroMgr::Idx2Ptr(int idx)
{
	//	Jun. 16, 2002 genta
	//	キーマクロ以外のマクロを読み込めるように
	if ( idx == STAND_KEYMACRO ){
		return &m_pKeyMacro;
	}
	else if ( idx == TEMP_KEYMACRO ){
		return &m_pTempMacro;
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return &m_cSavedKeyMacro[idx];
	}

	DEBUG_TRACE( L"CSMacroMgr::Idx2Ptr() Out of range: idx=%d\n", idx);

	return nullptr;
}

/*!
	キーボードマクロの保存が可能かどうか
	
	@retval true 保存可能
	@retval false 保存不可
*/
bool CSMacroMgr::IsSaveOk(void)
{
	return dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro ) == nullptr ? false : true;
}

/*!
	一時マクロを交換する
	
	@param newMacro [in] 新しいマクロバッファのポインタ．
	@return 前の一時マクロバッファのポインタ．
*/
CMacroManagerBase* CSMacroMgr::SetTempMacro( CMacroManagerBase *newMacro )
{
	CMacroManagerBase *oldMacro = m_pTempMacro;

	m_pTempMacro = newMacro;

	return oldMacro;
}
