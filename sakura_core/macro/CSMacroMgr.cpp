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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "macro/CSMacroMgr.h"
#include "macro/CPPAMacroMgr.h"
#include "macro/CWSHManager.h"
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
	{F_FILENEW,						LTEXT("FileNew"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //新規作成
	// {F_FILEOPEN,					LTEXT("FileOpen"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //開く
	{F_FILEOPEN2,					LTEXT("FileOpen"),				{VT_BSTR,  VT_I4,    VT_I4,    VT_BSTR},	VT_EMPTY,	NULL}, //開く2
	{F_FILESAVE,					LTEXT("FileSave"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上書き保存
	{F_FILESAVEALL,					LTEXT("FileSaveAll"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上書き保存
	{F_FILESAVEAS_DIALOG,			LTEXT("FileSaveAsDialog"),		{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //名前を付けて保存(ダイアログ) 2013.05.02
	{F_FILESAVEAS,					LTEXT("FileSaveAs"),			{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //名前を付けて保存
	{F_FILECLOSE,					LTEXT("FileClose"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	{F_FILECLOSE_OPEN,				LTEXT("FileCloseOpen"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //閉じて開く
	{F_FILE_REOPEN,					LTEXT("FileReopen"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //開き直す	//Dec. 4, 2002 genta
	{F_FILE_REOPEN_SJIS,			LTEXT("FileReopenSJIS"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJISで開き直す
	{F_FILE_REOPEN_JIS,				LTEXT("FileReopenJIS"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //JISで開き直す
	{F_FILE_REOPEN_EUC,				LTEXT("FileReopenEUC"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUCで開き直す
	{F_FILE_REOPEN_LATIN1,			LTEXT("FileReopenLatin1"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Latin1で開き直す	// 2010/3/20 Uchi
	{F_FILE_REOPEN_UNICODE,			LTEXT("FileReopenUNICODE"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicodeで開き直す
	{F_FILE_REOPEN_UNICODEBE,		LTEXT("FileReopenUNICODEBE"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UnicodeBEで開き直す
	{F_FILE_REOPEN_UTF8,			LTEXT("FileReopenUTF8"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-8で開き直す
	{F_FILE_REOPEN_CESU8,			LTEXT("FileReopenCESU8"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //CESU-8で開き直す
	{F_FILE_REOPEN_UTF7,			LTEXT("FileReopenUTF7"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-7で開き直す
	{F_PRINT,						LTEXT("Print"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷
//	{F_PRINT_DIALOG,				LTEXT("PrintDialog"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷ダイアログ
	{F_PRINT_PREVIEW,				LTEXT("PrintPreview"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷プレビュー
	{F_PRINT_PAGESETUP,				LTEXT("PrintPageSetup"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	{F_OPEN_HfromtoC,				LTEXT("OpenHfromtoC"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
//	{F_OPEN_HHPP,					LTEXT("OpenHHpp"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
//	{F_OPEN_CCPP,					LTEXT("OpenCCpp"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
	{F_ACTIVATE_SQLPLUS,			LTEXT("ActivateSQLPLUS"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plusをアクティブ表示 */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	LTEXT("ExecSQLPLUS"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plusで実行 */
	{F_BROWSE,						LTEXT("Browse"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブラウズ
	{F_VIEWMODE,					LTEXT("ViewMode"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ビューモード
	{F_VIEWMODE,					LTEXT("ReadOnly"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ビューモード(旧)
	{F_PROPERTY_FILE,				LTEXT("PropertyFile"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイルのプロパティ
	{F_EXITALLEDITORS,				LTEXT("ExitAllEditors"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //編集の全終了	// 2007.02.13 ryoji 追加
	{F_EXITALL,						LTEXT("ExitAll"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
	{F_PUTFILE,						LTEXT("PutFile"),				{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},   VT_EMPTY,	NULL}, // 作業中ファイルの一時出力 2006.12.10 maru
	{F_INSFILE,						LTEXT("InsFile"),				{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},   VT_EMPTY,	NULL}, // キャレット位置にファイル挿入 2006.12.10 maru

	/* 編集系 */
	{F_WCHAR,				LTEXT("Char"),					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //文字入力
	{F_IME_CHAR,			LTEXT("CharIme"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //全角文字入力
	{F_UNDO,				LTEXT("Undo"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //元に戻す(Undo)
	{F_REDO,				LTEXT("Redo"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //やり直し(Redo)
	{F_DELETE,				LTEXT("Delete"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //削除
	{F_DELETE_BACK,			LTEXT("DeleteBack"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル前を削除
	{F_WordDeleteToStart,	LTEXT("WordDeleteToStart"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の左端まで削除
	{F_WordDeleteToEnd,		LTEXT("WordDeleteToEnd"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の右端まで削除
	{F_WordCut,				LTEXT("WordCut"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語切り取り
	{F_WordDelete,			LTEXT("WordDelete"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語削除
	{F_LineCutToStart,		LTEXT("LineCutToStart"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行頭まで切り取り(改行単位)
	{F_LineCutToEnd,		LTEXT("LineCutToEnd"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行末まで切り取り(改行単位)
	{F_LineDeleteToStart,	LTEXT("LineDeleteToStart"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行頭まで削除(改行単位)
	{F_LineDeleteToEnd,		LTEXT("LineDeleteToEnd"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行末まで削除(改行単位)
	{F_CUT_LINE,			LTEXT("CutLine"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行切り取り(折り返し単位)
	{F_DELETE_LINE,			LTEXT("DeleteLine"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行削除(折り返し単位)
	{F_DUPLICATELINE,		LTEXT("DuplicateLine"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行の二重化(折り返し単位)
	{F_INDENT_TAB,			LTEXT("IndentTab"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //TABインデント
	{F_UNINDENT_TAB,		LTEXT("UnindentTab"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //逆TABインデント
	{F_INDENT_SPACE,		LTEXT("IndentSpace"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SPACEインデント
	{F_UNINDENT_SPACE,		LTEXT("UnindentSpace"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //逆SPACEインデント
//	{F_WORDSREFERENCE,		LTEXT("WordReference"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語リファレンス
	{F_LTRIM,				LTEXT("LTrim"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左(先頭)の空白を削除 2001.12.03 hor
	{F_RTRIM,				LTEXT("RTrim"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //右(末尾)の空白を削除 2001.12.03 hor
	{F_SORT_ASC,			LTEXT("SortAsc"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択行の昇順ソート 2001.12.06 hor
	{F_SORT_DESC,			LTEXT("SortDesc"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択行の降順ソート 2001.12.06 hor
	{F_MERGE,				LTEXT("Merge"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択行のマージ 2001.12.06 hor

	/* カーソル移動系 */
	{F_UP,					LTEXT("Up"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル上移動
	{F_DOWN,				LTEXT("Down"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル下移動
	{F_LEFT,				LTEXT("Left"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル左移動
	{F_RIGHT,				LTEXT("Right"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル右移動
	{F_UP2,					LTEXT("Up2"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル上移動(２行ごと)
	{F_DOWN2,				LTEXT("Down2"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル下移動(２行ごと)
	{F_WORDLEFT,			LTEXT("WordLeft"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の左端に移動
	{F_WORDRIGHT,			LTEXT("WordRight"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //単語の右端に移動
	{F_GOLINETOP,			LTEXT("GoLineTop"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行頭に移動(折り返し単位/改行単位)
	{F_GOLINEEND,			LTEXT("GoLineEnd"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //行末に移動(折り返し単位)
	{F_HalfPageUp,			LTEXT("HalfPageUp"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown,		LTEXT("HalfPageDown"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp,				LTEXT("PageUp"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageUp,				LTEXT("1PageUp"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown,			LTEXT("PageDown"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_1PageDown,			LTEXT("1PageDown"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP,			LTEXT("GoFileTop"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイルの先頭に移動
	{F_GOFILEEND,			LTEXT("GoFileEnd"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイルの最後に移動
	{F_CURLINECENTER,		LTEXT("CurLineCenter"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル行をウィンドウ中央へ
	{F_JUMPHIST_PREV,		LTEXT("MoveHistPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //移動履歴: 前へ
	{F_JUMPHIST_NEXT,		LTEXT("MoveHistNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //移動履歴: 次へ
	{F_JUMPHIST_SET,		LTEXT("MoveHistSet"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //現在位置を移動履歴に登録
	{F_WndScrollDown,		LTEXT("F_WndScrollDown"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //テキストを１行下へスクロール	// 2001/06/20 asa-o
	{F_WndScrollUp,			LTEXT("F_WndScrollUp"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //テキストを１行上へスクロール	// 2001/06/20 asa-o
	{F_GONEXTPARAGRAPH,		LTEXT("GoNextParagraph"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次の段落へ移動
	{F_GOPREVPARAGRAPH,		LTEXT("GoPrevParagraph"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前の段落へ移動
	{F_MODIFYLINE_NEXT,		LTEXT("GoModifyLineNext"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次の変更行へ移動
	{F_MODIFYLINE_PREV,		LTEXT("GoModifyLinePrev"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前の変更行へ移動
	{F_MOVECURSOR,			LTEXT("MoveCursor"),		{VT_I4,    VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル移動
	{F_MOVECURSORLAYOUT,	LTEXT("MoveCursorLayout"),	{VT_I4,    VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //カーソル移動(レイアウト単位)
	{F_WHEELUP,				LTEXT("WheelUp"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイールアップ
	{F_WHEELDOWN,			LTEXT("WheelDown"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイールダウン
	{F_WHEELLEFT,			LTEXT("WheelLeft"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイール左
	{F_WHEELRIGHT,			LTEXT("WheelRight"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイール右
	{F_WHEELPAGEUP,			LTEXT("WheelPageUp"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイールページアップ
	{F_WHEELPAGEDOWN,		LTEXT("WheelPageDown"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイールページダウン
	{F_WHEELPAGELEFT,		LTEXT("WheelPageLeft"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイールページ左
	{F_WHEELPAGERIGHT,		LTEXT("WheelPageRight"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ホイールページ右

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	{F_SELECTWORD,			LTEXT("SelectWord"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //現在位置の単語選択
	{F_SELECTALL,			LTEXT("SelectAll"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //すべて選択
	{F_SELECTLINE,			LTEXT("SelectLine"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //1行選択	// 2007.10.13 nasukoji
	{F_BEGIN_SEL,			LTEXT("BeginSelect"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //範囲選択開始 Mar. 5, 2001 genta 名称修正
	{F_UP_SEL,				LTEXT("Up_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル上移動
	{F_DOWN_SEL,			LTEXT("Down_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル下移動
	{F_LEFT_SEL,			LTEXT("Left_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル左移動
	{F_RIGHT_SEL,			LTEXT("Right_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル右移動
	{F_UP2_SEL,				LTEXT("Up2_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル上移動(２行ごと)
	{F_DOWN2_SEL,			LTEXT("Down2_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)カーソル下移動(２行ごと)
	{F_WORDLEFT_SEL,		LTEXT("WordLeft_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)単語の左端に移動
	{F_WORDRIGHT_SEL,		LTEXT("WordRight_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)単語の右端に移動
	{F_GOLINETOP_SEL,		LTEXT("GoLineTop_Sel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)行頭に移動(折り返し単位/改行単位)
	{F_GOLINEEND_SEL,		LTEXT("GoLineEnd_Sel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)行末に移動(折り返し単位)
	{F_HalfPageUp_Sel,		LTEXT("HalfPageUp_Sel"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown_Sel,	LTEXT("HalfPageDown_Sel"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp_Sel,			LTEXT("PageUp_Sel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageUp_Sel,			LTEXT("1PageUp_Sel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown_Sel,		LTEXT("PageDown_Sel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_1PageDown_Sel,		LTEXT("1PageDown_Sel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP_SEL,		LTEXT("GoFileTop_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)ファイルの先頭に移動
	{F_GOFILEEND_SEL,		LTEXT("GoFileEnd_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)ファイルの最後に移動
	{F_GONEXTPARAGRAPH_SEL,	LTEXT("GoNextParagraph_Sel"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)次の段落へ移動
	{F_GOPREVPARAGRAPH_SEL,	LTEXT("GoPrevParagraph_Sel"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)前の段落へ移動
	{F_MODIFYLINE_NEXT_SEL,	LTEXT("GoModifyLineNext_Sel"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)次の変更行へ移動
	{F_MODIFYLINE_PREV_SEL,	LTEXT("GoModifyLinePrev_Sel"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(範囲選択)前の変更行へ移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	{F_BEGIN_BOX,			LTEXT("BeginBoxSelect"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //矩形範囲選択開始
	{F_UP_BOX,				LTEXT("Up_BoxSel"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)カーソル上移動
	{F_DOWN_BOX,			LTEXT("Down_BoxSel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)カーソル下移動
	{F_LEFT_BOX,			LTEXT("Left_BoxSel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)カーソル左移動
	{F_RIGHT_BOX,			LTEXT("Right_BoxSel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)カーソル右移動
	{F_UP2_BOX,				LTEXT("Up2_BoxSel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)カーソル上移動(２行ごと)
	{F_DOWN2_BOX,			LTEXT("Down2_BoxSel"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)カーソル下移動(２行ごと)
	{F_WORDLEFT_BOX,		LTEXT("WordLeft_BoxSel"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)単語の左端に移動
	{F_WORDRIGHT_BOX,		LTEXT("WordRight_BoxSel"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)単語の右端に移動
	{F_GOLOGICALLINETOP_BOX,LTEXT("GoLogicalLineTop_BoxSel"),{VT_I4,   VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)行頭に移動(改行単位)
	{F_GOLINETOP_BOX,		LTEXT("GoLineTop_BoxSel"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)行頭に移動(折り返し単位/改行単位)
	{F_GOLINEEND_BOX,		LTEXT("GoLineEnd_BoxSel"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)行末に移動(折り返し単位)
	{F_HalfPageUp_BOX,		LTEXT("HalfPageUp_BoxSel"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)半ページアップ
	{F_HalfPageDown_BOX,	LTEXT("HalfPageDown_BoxSel"),	{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)半ページダウン
	{F_1PageUp_BOX,			LTEXT("PageUp_BoxSel"),			{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)１ページアップ
	{F_1PageUp_BOX,			LTEXT("1PageUp_BoxSel"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)１ページアップ
	{F_1PageDown_BOX,		LTEXT("PageDown_BoxSel"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)１ページダウン
	{F_1PageDown_BOX,		LTEXT("1PageDown_BoxSel"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)１ページダウン
	{F_GOFILETOP_BOX,		LTEXT("GoFileTop_BoxSel"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)ファイルの先頭に移動
	{F_GOFILEEND_BOX,		LTEXT("GoFileEnd_BoxSel"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	{F_CUT,						LTEXT("Cut"),						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //切り取り(選択範囲をクリップボードにコピーして削除)
	{F_COPY,					LTEXT("Copy"),						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //コピー(選択範囲をクリップボードにコピー)
	{F_PASTE,					LTEXT("Paste"),						{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //貼り付け(クリップボードから貼り付け)
	{F_COPY_ADDCRLF,			LTEXT("CopyAddCRLF"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //折り返し位置に改行をつけてコピー
	{F_COPY_CRLF,				LTEXT("CopyCRLF"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	{F_PASTEBOX,				LTEXT("PasteBox"),					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //矩形貼り付け(クリップボードから矩形貼り付け)
	{F_INSBOXTEXT,				LTEXT("InsBoxText"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 矩形テキスト挿入
	{F_INSTEXT_W,				LTEXT("InsText"),					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // テキストを貼り付け
	{F_ADDTAIL_W,				LTEXT("AddTail"),					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 最後にテキストを追加
	{F_COPYLINES,				LTEXT("CopyLines"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内全行コピー
	{F_COPYLINESASPASSAGE,		LTEXT("CopyLinesAsPassage"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内全行引用符付きコピー
	{F_COPYLINESWITHLINENUMBER,	LTEXT("CopyLinesWithLineNumber"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内全行行番号付きコピー
	{F_COPY_COLOR_HTML,			LTEXT("CopyColorHtml"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内色付きHTMLコピー
	{F_COPY_COLOR_HTML_LINENUMBER,	LTEXT("CopyColorHtmlWithLineNumber"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //選択範囲内行番号色付きHTMLコピー
	{F_COPYPATH,				LTEXT("CopyPath"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このファイルのパス名をクリップボードにコピー
	{F_COPYFNAME,				LTEXT("CopyFilename"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このファイル名をクリップボードにコピー // 2002/2/3 aroka
	{F_COPYTAG,					LTEXT("CopyTag"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	{F_CREATEKEYBINDLIST,		LTEXT("CopyKeyBindList"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	{F_INS_DATE,				LTEXT("InsertDate"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 日付挿入
	{F_INS_TIME,				LTEXT("InsertTime"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 時刻挿入
	{F_CTRL_CODE_DIALOG,		LTEXT("CtrlCodeDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK
	{F_CTRL_CODE,				LTEXT("CtrlCode"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //コントロールコードの入力 2013.12.12

	/* 変換系 */
	{F_TOLOWER,		 			LTEXT("ToLower"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //小文字
	{F_TOUPPER,		 			LTEXT("ToUpper"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //大文字
	{F_TOHANKAKU,		 		LTEXT("ToHankaku"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 全角→半角 */
	{F_TOHANKATA,		 		LTEXT("ToHankata"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	{F_TOZENEI,		 			LTEXT("ToZenEi"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			LTEXT("ToHanEi"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 全角英数→半角英数 */
	{F_TOZENKAKUKATA,	 		LTEXT("ToZenKata"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	{F_TOZENKAKUHIRA,	 		LTEXT("ToZenHira"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	{F_HANKATATOZENKATA,	LTEXT("HanKataToZenKata"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角カタカナ→全角カタカナ */
	{F_HANKATATOZENHIRA,	LTEXT("HanKataToZenHira"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 半角カタカナ→全角ひらがな */
	{F_TABTOSPACE,				LTEXT("TABToSPACE"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* TAB→空白 */
	{F_SPACETOTAB,				LTEXT("SPACEToTAB"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 空白→TAB */ //---- Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		LTEXT("AutoToSJIS"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 自動判別→SJISコード変換 */
	{F_CODECNV_EMAIL,			LTEXT("JIStoSJIS"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //E-Mail(JIS→SJIS)コード変換
	{F_CODECNV_EUC2SJIS,		LTEXT("EUCtoSJIS"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUC→SJISコード変換
	{F_CODECNV_UNICODE2SJIS,	LTEXT("CodeCnvUNICODEtoSJIS"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicode→SJISコード変換
	{F_CODECNV_UNICODEBE2SJIS,	LTEXT("CodeCnvUNICODEBEtoSJIS"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // UnicodeBE→SJISコード変換
	{F_CODECNV_UTF82SJIS,		LTEXT("UTF8toSJIS"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-8→SJISコード変換 */
	{F_CODECNV_UTF72SJIS,		LTEXT("UTF7toSJIS"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-7→SJISコード変換 */
	{F_CODECNV_SJIS2JIS,		LTEXT("SJIStoJIS"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→JISコード変換 */
	{F_CODECNV_SJIS2EUC,		LTEXT("SJIStoEUC"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→EUCコード変換 */
	{F_CODECNV_SJIS2UTF8,		LTEXT("SJIStoUTF8"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→UTF-8コード変換 */
	{F_CODECNV_SJIS2UTF7,		LTEXT("SJIStoUTF7"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS→UTF-7コード変換 */
	{F_BASE64DECODE,	 		LTEXT("Base64Decode"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Base64デコードして保存
	{F_UUDECODE,		 		LTEXT("Uudecode"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


	/* 検索系 */
	{F_SEARCH_DIALOG,			LTEXT("SearchDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //検索(単語検索ダイアログ)
	{F_SEARCH_NEXT,				LTEXT("SearchNext"),		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次を検索
	{F_SEARCH_PREV,				LTEXT("SearchPrev"),		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前を検索
	{F_REPLACE_DIALOG,			LTEXT("ReplaceDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //置換(置換ダイアログ)
	{F_REPLACE,					LTEXT("Replace"),			{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //置換(実行)
	{F_REPLACE_ALL,				LTEXT("ReplaceAll"),		{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //すべて置換(実行)
	{F_SEARCH_CLEARMARK,		LTEXT("SearchClearMark"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //検索マークのクリア
	{F_JUMP_SRCHSTARTPOS,		LTEXT("SearchStartPos"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //検索開始位置へ戻る			// 02/06/26 ai
	{F_GREP,					LTEXT("Grep"),				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_I4   },	VT_EMPTY,	&s_MacroInfoEx_i}, //Grep
	{F_GREP_REPLACE,			LTEXT("GrepReplace"),		{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_BSTR },	VT_EMPTY,	&s_MacroInfoEx_ii}, //Grep置換
	{F_JUMP,					LTEXT("Jump"),				{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //指定行ヘジャンプ
	{F_OUTLINE,					LTEXT("Outline"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //アウトライン解析
	{F_TAGJUMP,					LTEXT("TagJump"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タグジャンプ機能
	{F_TAGJUMPBACK,				LTEXT("TagJumpBack"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タグジャンプバック機能
	{F_TAGS_MAKE,				LTEXT("TagMake"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タグファイルの作成	//@@@ 2003.04.13 MIK
	{F_DIRECT_TAGJUMP,			LTEXT("DirectTagJump"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ダイレクトタグジャンプ機能	//@@@ 2003.04.15 MIK
	{F_TAGJUMP_KEYWORD,			LTEXT("KeywordTagJump"),	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //キーワードを指定してダイレクトタグジャンプ機能 //@@@ 2005.03.31 MIK
	{F_COMPARE,					LTEXT("Compare"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ファイル内容比較
	{F_DIFF_DIALOG,				LTEXT("DiffDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
	{F_DIFF,					LTEXT("Diff"),				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示				//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	{F_DIFF_NEXT,				LTEXT("DiffNext"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(次へ)			//@@@ 2002.05.25 MIK
	{F_DIFF_PREV,				LTEXT("DiffPrev"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(前へ)			//@@@ 2002.05.25 MIK
	{F_DIFF_RESET,				LTEXT("DiffReset"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF差分表示(全解除)		//@@@ 2002.05.25 MIK
	{F_BRACKETPAIR,				LTEXT("BracketPair"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //対括弧の検索
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			LTEXT("BookmarkSet"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブックマーク設定・解除
	{F_BOOKMARK_NEXT,			LTEXT("BookmarkNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次のブックマークへ
	{F_BOOKMARK_PREV,			LTEXT("BookmarkPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前のブックマークへ
	{F_BOOKMARK_RESET,			LTEXT("BookmarkReset"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブックマークの全解除
	{F_BOOKMARK_VIEW,			LTEXT("BookmarkView"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ブックマークの一覧
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		LTEXT("BookmarkPattern"),	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 2002.01.16 hor 指定パターンに一致する行をマーク
	{F_FUNCLIST_NEXT,			LTEXT("FuncListNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次の関数リストマークへ
	{F_FUNCLIST_PREV,			LTEXT("FuncListPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前の関数リストマークへ

	/* モード切り替え系 */
	{F_CHGMOD_INS,				LTEXT("ChgmodINS"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //挿入／上書きモード切り替え
	{F_CHG_CHARSET,				LTEXT("ChgCharSet"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //文字コードセット指定		2010/6/14 Uchi
	{F_CHGMOD_EOL,				LTEXT("ChgmodEOL"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //入力改行コード指定 2003.06.23 Moca
	{F_CANCEL_MODE,				LTEXT("CancelMode"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //各種モードの取り消し

	/* マクロ系 */
	{F_EXECEXTMACRO,			LTEXT("ExecExternalMacro"),	{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //名前を指定してマクロ実行

	/* 設定系 */
	{F_SHOWTOOLBAR,				LTEXT("ShowToolbar"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ツールバーの表示 */
	{F_SHOWFUNCKEY,				LTEXT("ShowFunckey"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ファンクションキーの表示 */
	{F_SHOWTAB,					LTEXT("ShowTab"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* タブの表示 */	//@@@ 2003.06.10 MIK
	{F_SHOWSTATUSBAR,			LTEXT("ShowStatusbar"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ステータスバーの表示 */
	{F_SHOWMINIMAP,				LTEXT("ShowMiniMap"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // ミニマップの表示
	{F_TYPE_LIST,				LTEXT("TypeList"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* タイプ別設定一覧 */
	{F_CHANGETYPE,				LTEXT("ChangeType"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タイプ別設定一時適用 2013.05.02
	{F_OPTION_TYPE,				LTEXT("OptionType"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* タイプ別設定 */
	{F_OPTION,					LTEXT("OptionCommon"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 共通設定 */
	{F_FONT,					LTEXT("SelectFont"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* フォント設定 */
	{F_SETFONTSIZE,				LTEXT("SetFontSize"),		{VT_I4,    VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, /* フォントサイズ設定 */
	{F_WRAPWINDOWWIDTH,			LTEXT("WrapWindowWidth"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	{F_FAVORITE,				LTEXT("OptionFavorite"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 履歴の管理 */	//@@@ 2003.04.08 MIK
	{F_SET_QUOTESTRING,			LTEXT("SetMsgQuoteStr"),	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 共通設定→書式→引用符の設定 */	//Jan. 29, 2005 genta
	{F_TEXTWRAPMETHOD,			LTEXT("TextWrapMethod"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* テキストの折り返し方法 */	// 2008.05.30 nasukoji
	{F_SELECT_COUNT_MODE,		LTEXT("SelectCountMode"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //文字カウント方法
	//	Oct. 9, 2001 genta 追加
	{F_EXECMD,					LTEXT("ExecCommand"),		{VT_BSTR,  VT_I4,    VT_BSTR,  VT_EMPTY},	VT_EMPTY,	NULL}, /* 外部コマンド実行 */
	{F_EXECMD_DIALOG,			LTEXT("ExecCommandDialog"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //外部コマンド実行(ダイアログ)

	/* カスタムメニュー */
	{F_MENU_RBUTTON,			LTEXT("RMenu"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 右クリックメニュー */
	{F_CUSTMENU_1,				LTEXT("CustMenu1"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー1 */
	{F_CUSTMENU_2,				LTEXT("CustMenu2"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー2 */
	{F_CUSTMENU_3,				LTEXT("CustMenu3"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー3 */
	{F_CUSTMENU_4,				LTEXT("CustMenu4"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー4 */
	{F_CUSTMENU_5,				LTEXT("CustMenu5"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー5 */
	{F_CUSTMENU_6,				LTEXT("CustMenu6"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー6 */
	{F_CUSTMENU_7,				LTEXT("CustMenu7"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー7 */
	{F_CUSTMENU_8,				LTEXT("CustMenu8"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー8 */
	{F_CUSTMENU_9,				LTEXT("CustMenu9"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー9 */
	{F_CUSTMENU_10,				LTEXT("CustMenu10"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー10 */
	{F_CUSTMENU_11,				LTEXT("CustMenu11"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー11 */
	{F_CUSTMENU_12,				LTEXT("CustMenu12"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー12 */
	{F_CUSTMENU_13,				LTEXT("CustMenu13"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー13 */
	{F_CUSTMENU_14,				LTEXT("CustMenu14"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー14 */
	{F_CUSTMENU_15,				LTEXT("CustMenu15"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー15 */
	{F_CUSTMENU_16,				LTEXT("CustMenu16"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー16 */
	{F_CUSTMENU_17,				LTEXT("CustMenu17"), 		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー17 */
	{F_CUSTMENU_18,				LTEXT("CustMenu18"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー18 */
	{F_CUSTMENU_19,				LTEXT("CustMenu19"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー19 */
	{F_CUSTMENU_20,				LTEXT("CustMenu20"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー20 */
	{F_CUSTMENU_21,				LTEXT("CustMenu21"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー21 */
	{F_CUSTMENU_22,				LTEXT("CustMenu22"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー22 */
	{F_CUSTMENU_23,				LTEXT("CustMenu23"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー23 */
	{F_CUSTMENU_24,				LTEXT("CustMenu24"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* カスタムメニュー24 */

	/* ウィンドウ系 */
	{F_SPLIT_V,					LTEXT("SplitWinV"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	{F_SPLIT_H,					LTEXT("SplitWinH"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	{F_SPLIT_VH,				LTEXT("SplitWinVH"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	{F_WINCLOSE,				LTEXT("WinClose"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ウィンドウを閉じる
	{F_WIN_CLOSEALL,			LTEXT("WinCloseAll"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	{F_CASCADE,					LTEXT("CascadeWin"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //重ねて表示
	{F_TILE_V,					LTEXT("TileWinV"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //上下に並べて表示
	{F_TILE_H,					LTEXT("TileWinH"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左右に並べて表示
	{F_NEXTWINDOW,				LTEXT("NextWindow"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次のウィンドウ
	{F_PREVWINDOW,				LTEXT("PrevWindow"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前のウィンドウ
	{F_WINLIST,					LTEXT("WindowList"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
	{F_MAXIMIZE_V,				LTEXT("MaximizeV"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //縦方向に最大化
	{F_MAXIMIZE_H,				LTEXT("MaximizeH"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //横方向に最大化 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			LTEXT("MinimizeAll"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	{F_REDRAW,					LTEXT("ReDraw"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //再描画
	{F_WIN_OUTPUT,				LTEXT("ActivateWinOutput"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //アウトプットウィンドウ表示
	{F_TRACEOUT,				LTEXT("TraceOut"),			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //マクロ用アウトプットウィンドウに出力	2006.04.26 maru
	{F_TOPMOST,					LTEXT("WindowTopMost"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //常に手前に表示
	{F_GROUPCLOSE,				LTEXT("GroupClose"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //グループを閉じる	// 2007.06.20 ryoji
	{F_NEXTGROUP,				LTEXT("NextGroup"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次のグループ	// 2007.06.20 ryoji
	{F_PREVGROUP,				LTEXT("PrevGroup"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前のグループ	// 2007.06.20 ryoji
	{F_TAB_MOVERIGHT,			LTEXT("TabMoveRight"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タブを右に移動	// 2007.06.20 ryoji
	{F_TAB_MOVELEFT,			LTEXT("TabMoveLeft"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //タブを左に移動	// 2007.06.20 ryoji
	{F_TAB_SEPARATE,			LTEXT("TabSeparate"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //新規グループ	// 2007.06.20 ryoji
	{F_TAB_JOINTNEXT,			LTEXT("TabJointNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //次のグループに移動	// 2007.06.20 ryoji
	{F_TAB_JOINTPREV,			LTEXT("TabJointPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //前のグループに移動	// 2007.06.20 ryoji
	{F_TAB_CLOSEOTHER,			LTEXT("TabCloseOther"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //このタブ以外を閉じる	// 2010/3/14 Uchi
	{F_TAB_CLOSELEFT,			LTEXT("TabCloseLeft"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //左をすべて閉じる		// 2010/3/14 Uchi
	{F_TAB_CLOSERIGHT,			LTEXT("TabCloseRight"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //右をすべて閉じる		// 2010/3/14 Uchi

	/* 支援 */
	{F_HOKAN,					LTEXT("Complete"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 入力補完 */	//Oct. 15, 2000 JEPRO 入ってなかったので英名を付けて入れてみた
	{F_TOGGLE_KEY_SEARCH,		LTEXT("ToggleKeyHelpSearch"), {VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //キーワードヘルプ自動表示 2013.05.03
	{F_HELP_CONTENTS,			LTEXT("HelpContents"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	{F_HELP_SEARCH,				LTEXT("HelpSearch"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	{F_MENU_ALLFUNC,			LTEXT("CommandList"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* コマンド一覧 */
	{F_EXTHELP1,				LTEXT("ExtHelp1"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 外部ヘルプ１ */
	//	Jul. 5, 2002 genta 引数追加
	{F_EXTHTMLHELP,				LTEXT("ExtHtmlHelp"),		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* 外部HTMLヘルプ */
	{F_ABOUT,					LTEXT("About"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/*マクロ用*/
	{F_STATUSMSG,				LTEXT("StatusMsg"),			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //ステータスメッセージ
	{F_MSGBEEP,					LTEXT("MsgBeep"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Beep音
	{F_COMMITUNDOBUFFER,		LTEXT("CommitUndoBuffer"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL }, //OpeBlKコミット
	{F_ADDREFUNDOBUFFER,		LTEXT("AddRefUndoBuffer"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL }, //OpeBlK AddRef
	{F_SETUNDOBUFFER,			LTEXT("SetUndoBuffer"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL }, //OpeBlK Release
	{F_APPENDUNDOBUFFERCURSOR,	L"AppendUndoBufferCursor",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL }, //OpeBlK にカーソル位置を追加
	{F_CLIPBOARDEMPTY,			LTEXT("ClipboardEmpty"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL},
	{F_SETVIEWTOP,				L"SetViewTop",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // ビューの上の行数を設定
	{F_SETVIEWLEFT,				L"SetViewLeft",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // ビューの左端の桁数を設定

	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
	//ID					関数名							引数										戻り値の型	m_pszData
	{F_GETFILENAME,			LTEXT("GetFilename"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //ファイル名を返す
	{F_GETSAVEFILENAME,		LTEXT("GetSaveFilename"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //保存時のファイル名を返す 2006.09.04 ryoji
	{F_GETSELECTED,			LTEXT("GetSelectedString"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //選択部分
	{F_EXPANDPARAMETER,		LTEXT("ExpandParameter"),		{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //特殊文字の展開
	{F_GETLINESTR,			LTEXT("GetLineStr"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // 指定論理行の取得 2003.06.01 Moca
	{F_GETLINECOUNT,		LTEXT("GetLineCount"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 全論理行数の取得 2003.06.01 Moca
	{F_CHGTABWIDTH,			LTEXT("ChangeTabWidth"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //タブサイズ変更 2004.03.16 zenryaku
	{F_ISTEXTSELECTED,		LTEXT("IsTextSelected"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //テキストが選択されているか 2005.7.30 maru
	{F_GETSELLINEFROM,		LTEXT("GetSelectLineFrom"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 選択開始行の取得 2005.7.30 maru
	{F_GETSELCOLUMNFROM,	LTEXT("GetSelectColmFrom"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 選択開始桁の取得 2005.7.30 maru
	{F_GETSELCOLUMNFROM,	LTEXT("GetSelectColumnFrom"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 選択開始桁の取得 2005.7.30 maru
	{F_GETSELLINETO,		LTEXT("GetSelectLineTo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 選択終了行の取得 2005.7.30 maru
	{F_GETSELCOLUMNTO,		LTEXT("GetSelectColmTo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 選択終了桁の取得 2005.7.30 maru
	{F_GETSELCOLUMNTO,		LTEXT("GetSelectColumnTo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 選択終了桁の取得 2005.7.30 maru
	{F_ISINSMODE,			LTEXT("IsInsMode"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 挿入／上書きモードの取得 2005.7.30 maru
	{F_GETCHARCODE,			LTEXT("GetCharCode"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 文字コード取得 2005.07.31 maru
	{F_GETLINECODE,			LTEXT("GetLineCode"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 改行コード取得 2005.08.05 maru
	{F_ISPOSSIBLEUNDO,		LTEXT("IsPossibleUndo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // Undo可能か調べる 2005.08.05 maru
	{F_ISPOSSIBLEREDO,		LTEXT("IsPossibleRedo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // Redo可能か調べる 2005.08.05 maru
	{F_CHGWRAPCOLUMN,		LTEXT("ChangeWrapColm"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //折り返し桁変更 2008.06.19 ryoji
	{F_CHGWRAPCOLUMN,		LTEXT("ChangeWrapColumn"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //折り返し桁変更 2008.06.19 ryoji
	{F_ISCURTYPEEXT,		LTEXT("IsCurTypeExt"),			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
	{F_ISSAMETYPEEXT,		LTEXT("IsSameTypeExt"),			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // ２つの拡張子が同じタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
	{F_INPUTBOX,			LTEXT("InputBox"),				{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_BSTR,	NULL }, //テキスト入力ダイアログの表示
	{F_MESSAGEBOX,			LTEXT("MessageBox"),			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メッセージボックスの表示
	{F_ERRORMSG,			LTEXT("ErrorMsg"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メッセージボックス（エラー）の表示
	{F_WARNMSG,				LTEXT("WarnMsg"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メッセージボックス（警告）の表示
	{F_INFOMSG,				LTEXT("InfoMsg"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メッセージボックス（情報）の表示
	{F_OKCANCELBOX,			LTEXT("OkCancelBox"),			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メッセージボックス（確認：OK／キャンセル）の表示
	{F_YESNOBOX,			LTEXT("YesNoBox"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メッセージボックス（確認：はい／いいえ）の表示
	{F_COMPAREVERSION,		LTEXT("CompareVersion"),		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //バージョン番号の比較
	{F_MACROSLEEP,			LTEXT("Sleep"),					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //指定した時間（ミリ秒）停止
	{F_FILEOPENDIALOG,		LTEXT("FileOpenDialog"),		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //ファイルを開くダイアログの表示
	{F_FILESAVEDIALOG,		LTEXT("FileSaveDialog"),		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //ファイルを保存ダイアログの表示
	{F_FOLDERDIALOG,		LTEXT("FolderDialog"),			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //フォルダを開くダイアログの表示
	{F_GETCLIPBOARD,		LTEXT("GetClipboard"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //クリップボードの文字列を取得
	{F_SETCLIPBOARD,		LTEXT("SetClipboard"),			{VT_I4,    VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //クリップボードに文字列を設定
	{F_LAYOUTTOLOGICLINENUM,LTEXT("LayoutToLogicLineNum"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //ロジック行番号取得
	{F_LOGICTOLAYOUTLINENUM,LTEXT("LogicToLayoutLineNum"),	{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //レイアウト行番号取得
	{F_LINECOLUMNTOINDEX,	LTEXT("LineColumnToIndex"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //ロジック桁番号取得
	{F_LINEINDEXTOCOLUMN,	LTEXT("LineIndexToColumn"),		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //レイアウト桁番号取得
	{F_GETCOOKIE,			LTEXT("GetCookie"),				{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //Cookie取得
	{F_GETCOOKIEDEFAULT,	LTEXT("GetCookieDefault"),		{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_EMPTY},	VT_BSTR,	NULL }, //Cookie取得デフォルト値
	{F_SETCOOKIE,			LTEXT("SetCookie"),				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_EMPTY},	VT_I4,		NULL }, //Cookie設定
	{F_DELETECOOKIE,		LTEXT("DeleteCookie"),			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //Cookie削除
	{F_GETCOOKIENAMES,		LTEXT("GetCookieNames"),		{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //Cookie名前取得
	{F_SETDRAWSWITCH,		LTEXT("SetDrawSwitch"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //再描画スイッチ設定
	{F_GETDRAWSWITCH,		LTEXT("GetDrawSwitch"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //再描画スイッチ取得
	{F_ISSHOWNSTATUS,		LTEXT("IsShownStatus"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //ステータスバーが表示されているか
	{F_GETSTRWIDTH,			LTEXT("GetStrWidth"),			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //文字列幅取得
	{F_GETSTRLAYOUTLENGTH,	LTEXT("GetStrLayoutLength"),	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //文字列のレイアウト幅取得
	{F_GETDEFAULTCHARLENGTH,	L"GetDefaultCharLength",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //デフォルト文字幅の取得
	{F_ISINCLUDECLIPBOARDFORMAT,L"IsIncludeClipboardFormat",{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //クリップボードの形式取得
	{F_GETCLIPBOARDBYFORMAT,	L"GetClipboardByFormat",	{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_BSTR,	NULL }, //クリップボードの指定形式で取得
	{F_SETCLIPBOARDBYFORMAT,	L"SetClipboardByFormat",	{VT_BSTR,  VT_BSTR,  VT_I4,    VT_I4,    },	VT_I4,		NULL }, //クリップボードの指定形式で設定
	{F_GETLINEATTRIBUTE,		L"GetLineAttribute",		{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //行属性取得
	{F_ISTEXTSELECTINGLOCK,		L"IsTextSelectingLock",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //選択状態のロックを取得
	{F_GETVIEWLINES,			L"GetViewLines",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //ビューの行数取得
	{F_GETVIEWCOLUMNS,			L"GetViewColumns",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //ビューの列数取得
	{F_CREATEMENU,				L"CreateMenu",				{VT_I4,    VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //メニュー作成

	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

/*!
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2002.04.29 genta オブジェクトの実体は実行時まで生成しない。
*/
CSMacroMgr::CSMacroMgr()
{
	MY_RUNNINGTIMER( cRunningTimer, "CSMacroMgr::CSMacroMgr" );
	
	m_pShareData = &GetDllShareData();
	
	CPPAMacroMgr::declare();
	CKeyMacroMgr::declare();
	CWSHMacroManager::declare();
	
	int i;
	for ( i = 0 ; i < MAX_CUSTMACRO ; i++ ){
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	m_pKeyMacro = NULL;
	m_pTempMacro = NULL;

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
	delete m_pTempMacro;
	m_pTempMacro = NULL;
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
		if( pKeyMacro == NULL ){
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
		if( m_pKeyMacro != NULL ){
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
		if( m_pTempMacro != NULL ){
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
	if( m_cSavedKeyMacro[idx] == NULL || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareDataから、マクロファイル名を取得
		//	Jun. 08, 2003 Moca 呼び出し側でパス名を用意
		//	Jun. 16, 2003 genta 書式をちょっと変更
		TCHAR ptr[_MAX_PATH * 2];
		int n = CShareData::getInstance()->GetMacroFilename( idx, ptr, _countof(ptr) );
		if ( n <= 0 ){
			return FALSE;
		}

		if( !Load( idx, hInstance, ptr, NULL ) )
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
BOOL CSMacroMgr::Load( int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType )
{
	CMacroManagerBase** ppMacro = Idx2Ptr( idx );

	if( ppMacro == NULL ){
		DEBUG_TRACE( _T("CSMacroMgr::Load() Out of range: idx=%d Path=%ts\n"), idx, pszPath);
	}

	//	バッファクリア
	delete *ppMacro;
	*ppMacro = NULL;
	
	const TCHAR *ext;
	if( pszType == NULL ){				//ファイル指定
		//ファイルの拡張子を取得する
		ext = _tcsrchr( pszPath, _T('.'));
		//	Feb. 02, 2004 genta .が無い場合にext==NULLとなるのでNULLチェック追加
		if( ext != NULL ){
			const TCHAR *chk = _tcsrchr( ext, _T('\\') );
			if( chk != NULL ){	//	.のあとに\があったらそれは拡張子の区切りではない
								//	\が漢字の2バイト目の場合も拡張子ではない。
				ext = NULL;
			}
		}
		if(ext != NULL){
			++ext;
		}
	}else{								//コード指定
		ext = pszType;
	}

	m_sMacroPath = _T("");
	*ppMacro = CMacroFactory::getInstance()->Create(ext);
	if( *ppMacro == NULL )
		return FALSE;
	BOOL bRet;
	if( pszType == NULL ){
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
		*ppMacro = NULL;
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
		m_cSavedKeyMacro[idx] = NULL;
	}

}

/*! キーボードマクロの保存

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名
	@param hInstance [in] インスタンスハンドル

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx, HINSTANCE hInstance, const TCHAR* pszPath )
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
	@date 2003.02.24 m_MacroFuncInfoArrも検索対象にする
*/
const MacroFuncInfo* CSMacroMgr::GetFuncInfoByID( int nFuncID )
{
	int i;
	//	Jun. 27, 2002 genta
	//	番人をコード0として拾ってしまうので，配列サイズによる判定をやめた．
	for( i = 0; m_MacroFuncInfoCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoCommandArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoCommandArr[i];
		}
	}
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoArr[i];
		}
	}
	return NULL;
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
WCHAR* CSMacroMgr::GetFuncInfoByID(
	HINSTANCE	hInstance,			//!< [in] リソース取得のためのInstance Handle
	int			nFuncID,			//!< [in] 機能番号
	WCHAR*		pszFuncName,		//!< [out] 関数名．この先には最長関数名＋1バイトのメモリが必要．
	WCHAR*		pszFuncNameJapanese	//!< [out] 機能名日本語．NULL許容. この先には256バイトのメモリが必要．
)
{
	const MacroFuncInfo* MacroInfo = GetFuncInfoByID( nFuncID );
	if( MacroInfo != NULL ){
		if( pszFuncName != NULL ){
			auto_strcpy( pszFuncName, MacroInfo->m_pszFuncName );
			WCHAR *p = pszFuncName;
			while (*p){
				if (*p == LTEXT('(')){
					*p = LTEXT('\0');
					break;
				}
				p++;
			}
		}
		//	Jun. 16, 2002 genta NULLのときは何もしない．
		if( pszFuncNameJapanese != NULL ){
			wcsncpy( pszFuncNameJapanese, LSW( nFuncID ), 255 );
		}
		return pszFuncName;
	}
	return NULL;
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
	HINSTANCE		hInstance,				//!< [in]  リソース取得のためのInstance Handle
	const WCHAR*	pszFuncName,			//!< [in]  関数名
	WCHAR*			pszFuncNameJapanese		//!< [out] 機能名日本語．この先には256バイトのメモリが必要．
)
{
	//	Jun. 16, 2002 genta
	const WCHAR* normalizedFuncName;
	
	//	S_で始まっているか
	if( pszFuncName == NULL ){
		return F_INVALID;
	}
	if( pszFuncName[0] == LTEXT('S') && pszFuncName[1] == LTEXT('_') ){
		normalizedFuncName = pszFuncName + 2;
	}
	else {
		normalizedFuncName = pszFuncName;
	}

	// コマンド関数を検索
	for( int i = 0; m_MacroFuncInfoCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == auto_strcmp( normalizedFuncName, m_MacroFuncInfoCommandArr[i].m_pszFuncName )){
			EFunctionCode nFuncID = EFunctionCode(m_MacroFuncInfoCommandArr[i].m_nFuncID);
			if( pszFuncNameJapanese != NULL ){
				wcsncpy( pszFuncNameJapanese, LSW( nFuncID ), 255 );
				pszFuncNameJapanese[255] = L'\0';
			}
			return nFuncID;
		}
	}
	// 非コマンド関数を検索
	for( int i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == auto_strcmp( normalizedFuncName, m_MacroFuncInfoArr[i].m_pszFuncName )){
			EFunctionCode nFuncID = EFunctionCode(m_MacroFuncInfoArr[i].m_nFuncID);
			if( pszFuncNameJapanese != NULL ){
				wcsncpy( pszFuncNameJapanese, LSW( nFuncID ), 255 );
				pszFuncNameJapanese[255] = L'\0';
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
//	case F_OPEN_HfromtoC:			://同名のC/C++ヘッダ(ソース)を開く	//Feb. 9, 2001 JEPRO 追加
//	case F_OPEN_HHPP				://同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
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
	case F_COPYTAG					://このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	case F_COPYFNAME				://このファイル名をクリップボードにコピー // 2002/2/3 aroka
	case F_CREATEKEYBINDLIST		://キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加	//Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE					:// 日付挿入
	case F_INS_TIME					:// 時刻挿入
//	case F_CTRL_CODE_DIALOG			://コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK
	case F_CTRL_CODE				://コントロールコードの入力 2013.12.12

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

	DEBUG_TRACE( _T("CSMacroMgr::Idx2Ptr() Out of range: idx=%d\n"), idx);

	return NULL;
}

/*!
	キーボードマクロの保存が可能かどうか
	
	@retval true 保存可能
	@retval false 保存不可
*/
bool CSMacroMgr::IsSaveOk(void)
{
	return dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro ) == NULL ? false : true;
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
