//	$Id$
/*!	@file
	@brief マクロ

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	@date 20011229 aroka バグ修正、コメント追加
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CSMacroMgr.h"
#include "CEditView.h"
#include "CPPAMacroMgr.h"
#include "CPPA.h"

CSMacroMgr::MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
//	機能ID			関数名			引数				作業用バッファ

	/* ファイル操作系 */
	{F_FILENEW,						"S_FileNew",			"",					NULL}, //新規作成
	{F_FILEOPEN,					"S_FileOpen",			"(str: string)",	NULL}, //開く
	{F_FILESAVE,					"S_FileSave",			"",					NULL}, //上書き保存
	{F_FILESAVEAS,					"S_FileSaveAs",			"(str: string; charcode, eolcode: Integer)",	NULL}, //名前を付けて保存
	{F_FILECLOSE,					"S_FileClose",			"",					NULL}, //閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
//	{F_FILECLOSE_OPEN,				"S_FileCloseOpen",		"(str: string)",	NULL}, //閉じて開く
	{F_FILE_REOPEN_SJIS,			"S_FileReopenSJIS",		"",					NULL}, //SJISで開き直す
	{F_FILE_REOPEN_JIS,				"S_FileReopenJIS",		"",					NULL}, //JISで開き直す
	{F_FILE_REOPEN_EUC,				"S_FileReopenEUC",		"",					NULL}, //EUCで開き直す
	{F_FILE_REOPEN_UNICODE,			"S_FileReopenUNICODE",	"",					NULL}, //Unicodeで開き直す
	{F_FILE_REOPEN_UTF8,			"S_FileReopenUTF8",		"",					NULL}, //UTF-8で開き直す
	{F_FILE_REOPEN_UTF7,			"S_FileReopenUTF7",		"",					NULL}, //UTF-7で開き直す
	{F_PRINT,						"S_Print",				"",					NULL}, //印刷
//	{F_PRINT_DIALOG,				"S_PrintDialog",		"",					NULL}, //印刷ダイアログ
	{F_PRINT_PREVIEW,				"S_PrintPreview",		"",					NULL}, //印刷プレビュー
	{F_PRINT_PAGESETUP,				"S_PrintPageSetup",		"",					NULL}, //印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	{F_OPEN_HfromtoC,				"S_OpenHfromtoC",		"",					NULL}, //同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	{F_OPEN_HHPP,					"S_OpenHHpp",			"",					NULL}, //同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	{F_OPEN_CCPP,					"S_OpenCCpp",			"",					NULL}, //同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	{F_ACTIVATE_SQLPLUS,			"S_ActivateSQLPLUS",	"",					NULL}, /* Oracle SQL*Plusをアクティブ表示 */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	"S_ExecSQLPLUS",		"",					NULL}, /* Oracle SQL*Plusで実行 */
	{F_BROWSE,						"S_Browse",				"",					NULL}, //ブラウズ
	{F_READONLY,					"S_ReadOnly",			"",					NULL}, //読み取り専用
	{F_PROPERTY_FILE,				"S_PropertyFile",		"",					NULL}, //ファイルのプロパティ
	{F_EXITALL,						"S_ExitAll",			"",					NULL}, //サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加

	/* 編集系 */
	{F_CHAR,				"S_Char",				"(Val: integer)",	NULL}, //文字入力
//	{F_IME_CHAR,			"S_CharIme",			"",					NULL}, //全角文字入力
	{F_UNDO,				"S_Undo",				"",					NULL}, //元に戻す(Undo)
	{F_REDO,				"S_Redo",				"",					NULL}, //やり直し(Redo)
	{F_DELETE,				"S_Delete",				"",					NULL}, //削除
	{F_DELETE_BACK,			"S_DeleteBack",			"",					NULL}, //カーソル前を削除
	{F_WordDeleteToStart,	"S_WordDeleteToStart",	"",					NULL}, //単語の左端まで削除
	{F_WordDeleteToEnd,		"S_WordDeleteToEnd",	"",					NULL}, //単語の右端まで削除
	{F_WordCut,				"S_WordCut",			"",					NULL}, //単語切り取り
	{F_WordDelete,			"S_WordDelete",			"",					NULL}, //単語削除
	{F_LineCutToStart,		"S_LineCutToStart",		"",					NULL}, //行頭まで切り取り(改行単位)
	{F_LineCutToEnd,		"S_LineCutToEnd",		"",					NULL}, //行末まで切り取り(改行単位)
	{F_LineDeleteToStart,	"S_LineDeleteToStart",	"",					NULL}, //行頭まで削除(改行単位)
	{F_LineDeleteToEnd,		"S_LineDeleteToEnd",	"",					NULL}, //行末まで削除(改行単位)
	{F_CUT_LINE,			"S_CutLine",			"",					NULL}, //行切り取り(折り返し単位)
	{F_DELETE_LINE,			"S_DeleteLine",			"",					NULL}, //行削除(折り返し単位)
	{F_DUPLICATELINE,		"S_DuplicateLine",		"",					NULL}, //行の二重化(折り返し単位)
	{F_INDENT_TAB,			"S_IndentTab",			"",					NULL}, //TABインデント
	{F_UNINDENT_TAB,		"S_UnindentTab",		"",					NULL}, //逆TABインデント
	{F_INDENT_SPACE,		"S_IndentSpace",		"",					NULL}, //SPACEインデント
	{F_UNINDENT_SPACE,		"S_UnindentSpace",		"",					NULL}, //逆SPACEインデント
	{F_WORDSREFERENCE,		"S_WordReference",		"",					NULL}, //単語リファレンス
	{F_LTRIM,				"S_LTrim",				"",					NULL}, //左(先頭)の空白を削除 2001.12.03 hor
	{F_RTRIM,				"S_RTrim",				"",					NULL}, //右(末尾)の空白を削除 2001.12.03 hor
	{F_SORT_ASC,			"S_SortAsc",			"",					NULL}, //選択行の昇順ソート 2001.12.06 hor
	{F_SORT_DESC,			"S_SortDesc",			"",					NULL}, //選択行の降順ソート 2001.12.06 hor
	{F_MERGE,				"S_Merge",				"",					NULL}, //選択行のマージ 2001.12.06 hor

	/* カーソル移動系 */
	{F_UP,					"S_Up",					"",					NULL}, //カーソル上移動
	{F_DOWN,				"S_Down",				"",					NULL}, //カーソル下移動
	{F_LEFT,				"S_Left",				"",					NULL}, //カーソル左移動
	{F_RIGHT,				"S_Right",				"",					NULL}, //カーソル右移動
	{F_UP2,					"S_Up2",				"",					NULL}, //カーソル上移動(２行ごと)
	{F_DOWN2,				"S_Down2",				"",					NULL}, //カーソル下移動(２行ごと)
	{F_WORDLEFT,			"S_WordLeft",			"",					NULL}, //単語の左端に移動
	{F_WORDRIGHT,			"S_WordRight",			"",					NULL}, //単語の右端に移動
	{F_GOLINETOP,			"S_GoLineTop",			"(flg: integer)",	NULL}, //行頭に移動(折り返し単位)
	{F_GOLINEEND,			"S_GoLineEnd",			"",					NULL}, //行末に移動(折り返し単位)
	{F_HalfPageUp,			"S_HalfPageUp",			"",					NULL}, //半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown,		"S_HalfPageDown",		"",					NULL}, //半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp,				"S_1PageUp",			"",					NULL}, //１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown,			"S_1PageDown",			"",					NULL}, //１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP,			"S_GoFileTop",			"",					NULL}, //ファイルの先頭に移動
	{F_GOFILEEND,			"S_GoFileEnd",			"",					NULL}, //ファイルの最後に移動
	{F_CURLINECENTER,		"S_CurLineCenter",		"",					NULL}, //カーソル行をウィンドウ中央へ
	{F_JUMPPREV,			"S_MoveHistPrev",		"",					NULL}, //移動履歴: 前へ
	{F_JUMPNEXT,			"S_MoveHistNext",		"",					NULL}, //移動履歴: 次へ
	{F_WndScrollDown,		"S_F_WndScrollDown",	"",					NULL}, //テキストを１行下へスクロール	// 2001/06/20 asa-o
	{F_WndScrollUp,			"S_F_WndScrollUp",		"",					NULL}, //テキストを１行上へスクロール	// 2001/06/20 asa-o

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	{F_SELECTWORD,			"S_SelectWord",			"",					NULL}, //現在位置の単語選択
	{F_SELECTALL,			"S_SelectAll",			"",					NULL}, //すべて選択
	{F_BEGIN_SEL,			"S_BeginSelect",		"",					NULL}, //範囲選択開始 Mar. 5, 2001 genta 名称修正
	{F_UP_SEL,				"S_Up_Sel",				"",					NULL}, //(範囲選択)カーソル上移動
	{F_DOWN_SEL,			"S_Down_Sel",			"",					NULL}, //(範囲選択)カーソル下移動
	{F_LEFT_SEL,			"S_Left_Sel",			"",					NULL}, //(範囲選択)カーソル左移動
	{F_RIGHT_SEL,			"S_Right_Sel",			"",					NULL}, //(範囲選択)カーソル右移動
	{F_UP2_SEL,				"S_Up2_Sel",			"",					NULL}, //(範囲選択)カーソル上移動(２行ごと)
	{F_DOWN2_SEL,			"S_Down2_Sel",			"",					NULL}, //(範囲選択)カーソル下移動(２行ごと)
	{F_WORDLEFT_SEL,		"S_WordLeft_Sel",		"",					NULL}, //(範囲選択)単語の左端に移動
	{F_WORDRIGHT_SEL,		"S_WordRight_Sel",		"",					NULL}, //(範囲選択)単語の右端に移動
	{F_GOLINETOP_SEL,		"S_GoLineTop_Sel",		"",					NULL}, //(範囲選択)行頭に移動(折り返し単位)
	{F_GOLINEEND_SEL,		"S_GoLineEnd_Sel",		"",					NULL}, //(範囲選択)行末に移動(折り返し単位)
	{F_HalfPageUp_Sel,		"S_HalfPageUp_Sel",		"",					NULL}, //(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_HalfPageDown_Sel,	"S_HalfPageDown_Sel",	"",					NULL}, //(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	{F_1PageUp_Sel,			"S_1PageUp_Sel",		"",					NULL}, //(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	{F_1PageDown_Sel,		"S_1PageDown_Sel",		"",					NULL}, //(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	{F_GOFILETOP_SEL,		"S_GoFileTop_Sel",		"",					NULL}, //(範囲選択)ファイルの先頭に移動
	{F_GOFILEEND_SEL,		"S_GoFileEnd_Sel",		"",					NULL}, //(範囲選択)ファイルの最後に移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	{F_BEGIN_BOX,			"S_BeginBoxSelect",		"",					NULL}, //矩形範囲選択開始

	/* クリップボード系 */
	{F_CUT,						"S_Cut",						"",					NULL}, //切り取り(選択範囲をクリップボードにコピーして削除)
	{F_COPY,					"S_Copy",						"",					NULL}, //コピー(選択範囲をクリップボードにコピー)
	{F_PASTE,					"S_Paste",						"",					NULL}, //貼り付け(クリップボードから貼り付け)
	{F_COPY_ADDCRLF,			"S_CopyAddCRLF",				"",					NULL}, //折り返し位置に改行をつけてコピー
	{F_COPY_CRLF,				"S_CopyCRLF",					"",					NULL}, //CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	{F_PASTEBOX,				"S_PasteBox",					"",					NULL}, //矩形貼り付け(クリップボードから矩形貼り付け)
	{F_INSTEXT,					"S_InsText",					"(str: string)",	NULL}, // テキストを貼り付け
	{F_ADDTAIL,					"S_AddTail",					"(str: string)",	NULL}, // 最後にテキストを追加
	{F_COPYLINES,				"S_CopyLines",					"",					NULL}, //選択範囲内全行コピー
	{F_COPYLINESASPASSAGE,		"S_CopyLinesAsPassage",			"",					NULL}, //選択範囲内全行引用符付きコピー
	{F_COPYLINESWITHLINENUMBER,	"S_CopyLinesWithLineNumber",	"",					NULL}, //選択範囲内全行行番号付きコピー
	{F_COPYPATH,				"S_CopyPath",					"",					NULL}, //このファイルのパス名をクリップボードにコピー
	{F_COPYFNAME,				"S_CopyFilename",				"",					NULL}, //このファイル名をクリップボードにコピー // 2002/2/3 aroka
	{F_COPYTAG,					"S_CopyTag",					"",					NULL}, //このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	{F_CREATEKEYBINDLIST,		"S_CopyKeyBindList",			"",					NULL}, //キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	{F_INS_DATE,				"S_InsertDate",				"",			NULL}, // 日付挿入
	{F_INS_TIME,				"S_InsertTime",				"",			NULL}, // 時刻挿入

	/* 変換系 */
	{F_TOLOWER,		 			"S_ToLower",				"",			NULL}, //英大文字→英小文字
	{F_TOUPPER,		 			"S_ToUpper",				"",			NULL}, //英小文字→英大文字
	{F_TOHANKAKU,		 		"S_ToHankaku",				"",			NULL}, /* 全角→半角 */
	{F_TOZENEI,		 			"S_ToZenEi",				"",			NULL}, /* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			"S_ToHanEi",				"",			NULL}, /* 全角英数→半角英数 */
	{F_TOZENKAKUKATA,	 		"S_ToZenKata",				"",			NULL}, /* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	{F_TOZENKAKUHIRA,	 		"S_ToZenHira",				"",			NULL}, /* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	{F_HANKATATOZENKAKUKATA,	"S_HanKataToZenKata",		"",			NULL}, /* 半角カタカナ→全角カタカナ */
	{F_HANKATATOZENKAKUHIRA,	"S_HanKataToZenHira",		"",			NULL}, /* 半角カタカナ→全角ひらがな */
	{F_TABTOSPACE,				"S_TABToSPACE",				"",			NULL}, /* TAB→空白 */
	{F_SPACETOTAB,				"S_SPACEToTAB",				"",			NULL}, /* 空白→TAB */ //#### Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		"S_AutoToSJIS",				"",			NULL}, /* 自動判別→SJISコード変換 */
	{F_CODECNV_EMAIL,			"S_JIStoSJIS",				"",			NULL}, //E-Mail(JIS→SJIS)コード変換
	{F_CODECNV_EUC2SJIS,		"S_EUCtoSJIS",				"",			NULL}, //EUC→SJISコード変換
	{F_CODECNV_UNICODE2SJIS,	"S_CodeCnvUNICODEtoJIS",	"",			NULL}, //Unicode→SJISコード変換
	{F_CODECNV_UTF82SJIS,		"S_UTF8toSJIS",				"",			NULL}, /* UTF-8→SJISコード変換 */
	{F_CODECNV_UTF72SJIS,		"S_UTF7toSJIS",				"",			NULL}, /* UTF-7→SJISコード変換 */
	{F_CODECNV_SJIS2JIS,		"S_SJIStoJIS",				"",			NULL}, /* SJIS→JISコード変換 */
	{F_CODECNV_SJIS2EUC,		"S_SJIStoEUC",				"",			NULL}, /* SJIS→EUCコード変換 */
	{F_CODECNV_SJIS2UTF8,		"S_SJIStoUTF8",				"",			NULL}, /* SJIS→UTF-8コード変換 */
	{F_CODECNV_SJIS2UTF7,		"S_SJIStoUTF7",				"",			NULL}, /* SJIS→UTF-7コード変換 */
	{F_BASE64DECODE,	 		"S_Base64Decode",			"",			NULL}, //Base64デコードして保存
	{F_UUDECODE,		 		"S_Uudecode",				"",			NULL}, //uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


	/* 検索系 */
	{F_SEARCH_DIALOG,			"S_SearchDialog",			"",			NULL}, //検索(単語検索ダイアログ)
	{F_SEARCH_NEXT,				"S_SearchNext",				"(str: String; flg: Integer)",			NULL}, //次を検索
	{F_SEARCH_PREV,				"S_SearchPrev",				"(str: String; flg: Integer)",			NULL}, //前を検索
	{F_REPLACE_DIALOG,			"S_ReplaceDialog",			"",			NULL}, //置換(置換ダイアログ)
	{F_REPLACE,					"S_Replace",				"(before, after: String; flg: Integer)",	NULL}, //置換(実行)
	{F_REPLACE_ALL,				"S_ReplaceAll",				"(before, after: String; flg: Integer)",	NULL}, //すべて置換(実行)
	{F_SEARCH_CLEARMARK,		"S_SearchClearMark",		"",			NULL}, //検索マークのクリア
	{F_GREP,					"S_Grep",					"(str, file, folder: String; flg: Integer)",			NULL}, //Grep
	{F_JUMP,					"S_Jump",					"(line, flg: Integer)",			NULL}, //指定行ヘジャンプ
	{F_OUTLINE,					"S_Outline",				"",			NULL}, //アウトライン解析
	{F_TAGJUMP,					"S_TagJump",				"",			NULL}, //タグジャンプ機能
	{F_TAGJUMPBACK,				"S_TagJumpBack",			"",			NULL}, //タグジャンプバック機能
	{F_COMPARE,					"S_Compare",				"",			NULL}, //ファイル内容比較
	{F_BRACKETPAIR,				"S_BracketPair",			"",			NULL}, //対括弧の検索
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			"S_BookmarkSet",			"",			NULL}, //ブックマーク設定・解除
	{F_BOOKMARK_NEXT,			"S_BookmarkNext",			"",			NULL}, //次のブックマークへ
	{F_BOOKMARK_PREV,			"S_BookmarkPrev",			"",			NULL}, //前のブックマークへ
	{F_BOOKMARK_RESET,			"S_BookmarkReset",			"",			NULL}, //ブックマークの全解除
	{F_BOOKMARK_VIEW,			"S_BookmarkView",			"",			NULL}, //ブックマークの一覧
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		"S_BookmarkPattern",		"(str: String; flg: Integer)",	NULL}, // 2002.01.16 hor 指定パターンに一致する行をマーク

	/* モード切り替え系 */
	{F_CHGMOD_INS,				"S_ChgmodINS",				"",			NULL}, //挿入／上書きモード切り替え
	{F_CANCEL_MODE,				"S_CancelMode",				"",			NULL}, //各種モードの取り消し

	/* 設定系 */
	{F_SHOWTOOLBAR,				"S_ShowToolbar",			"",			NULL}, /* ツールバーの表示 */
	{F_SHOWFUNCKEY,				"S_ShowFunckey",			"",			NULL}, /* ファンクションキーの表示 */
	{F_SHOWSTATUSBAR,			"S_ShowStatusbar",			"",			NULL}, /* ステータスバーの表示 */
	{F_TYPE_LIST,				"S_TypeList",				"",			NULL}, /* タイプ別設定一覧 */
	{F_OPTION_TYPE,				"S_OptionType",				"",			NULL}, /* タイプ別設定 */
	{F_OPTION,					"S_OptionCommon",			"",			NULL}, /* 共通設定 */
	{F_FONT,					"S_SelectFont",				"",			NULL}, /* フォント設定 */
	{F_WRAPWINDOWWIDTH,			"S_WrapWindowWidth",		"",			NULL}, /* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更

	//	Oct. 9, 2001 genta 追加
	{F_EXECCOMMAND,				"S_ExecCommand",			"(str: String; flg: Integer)",	NULL}, /* 外部コマンド実行 */

	/* カスタムメニュー */
	{F_MENU_RBUTTON,			"S_RMenu",					"",			NULL}, /* 右クリックメニュー */
	{F_CUSTMENU_1,				"S_CustMenu1",				"",			NULL}, /* カスタムメニュー1 */
	{F_CUSTMENU_2,				"S_CustMenu2",				"",			NULL}, /* カスタムメニュー2 */
	{F_CUSTMENU_3,				"S_CustMenu3",				"",			NULL}, /* カスタムメニュー3 */
	{F_CUSTMENU_4,				"S_CustMenu4",				"",			NULL}, /* カスタムメニュー4 */
	{F_CUSTMENU_5,				"S_CustMenu5",				"",			NULL}, /* カスタムメニュー5 */
	{F_CUSTMENU_6,				"S_CustMenu6",				"",			NULL}, /* カスタムメニュー6 */
	{F_CUSTMENU_7,				"S_CustMenu7",				"",			NULL}, /* カスタムメニュー7 */
	{F_CUSTMENU_8,				"S_CustMenu8",				"",			NULL}, /* カスタムメニュー8 */
	{F_CUSTMENU_9,				"S_CustMenu9",				"",			NULL}, /* カスタムメニュー9 */
	{F_CUSTMENU_10,				"S_CustMenu10",				"",			NULL}, /* カスタムメニュー10 */
	{F_CUSTMENU_11,				"S_CustMenu11",				"",			NULL}, /* カスタムメニュー11 */
	{F_CUSTMENU_12,				"S_CustMenu12",				"",			NULL}, /* カスタムメニュー12 */
	{F_CUSTMENU_13,				"S_CustMenu13",				"",			NULL}, /* カスタムメニュー13 */
	{F_CUSTMENU_14,				"S_CustMenu14",				"",			NULL}, /* カスタムメニュー14 */
	{F_CUSTMENU_15,				"S_CustMenu15",				"",			NULL}, /* カスタムメニュー15 */
	{F_CUSTMENU_16,				"S_CustMenu16",				"",			NULL}, /* カスタムメニュー16 */
	{F_CUSTMENU_17,				"S_CustMenu17", 			"",			NULL}, /* カスタムメニュー17 */
	{F_CUSTMENU_18,				"S_CustMenu18",				"",			NULL}, /* カスタムメニュー18 */
	{F_CUSTMENU_19,				"S_CustMenu19",				"",			NULL}, /* カスタムメニュー19 */
	{F_CUSTMENU_20,				"S_CustMenu20",				"",			NULL}, /* カスタムメニュー20 */
	{F_CUSTMENU_21,				"S_CustMenu21",				"",			NULL}, /* カスタムメニュー21 */
	{F_CUSTMENU_22,				"S_CustMenu22",				"",			NULL}, /* カスタムメニュー22 */
	{F_CUSTMENU_23,				"S_CustMenu23",				"",			NULL}, /* カスタムメニュー23 */
	{F_CUSTMENU_24,				"S_CustMenu24",				"",			NULL}, /* カスタムメニュー24 */

	/* ウィンドウ系 */
	{F_SPLIT_V,					"S_SplitWinV",				"",			NULL}, //上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	{F_SPLIT_H,					"S_SplitWinH",				"",			NULL}, //左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	{F_SPLIT_VH,				"S_SplitWinVH",				"",			NULL}, //縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	{F_WINCLOSE,				"S_WinClose",				"",			NULL}, //ウィンドウを閉じる
	{F_WIN_CLOSEALL,			"S_WinCloseAll",			"",			NULL}, //すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	{F_CASCADE,					"S_CascadeWin",				"",			NULL}, //重ねて表示
	{F_TILE_V,					"S_TileWinV",				"",			NULL}, //上下に並べて表示
	{F_TILE_H,					"S_TileWinH",				"",			NULL}, //左右に並べて表示
	{F_NEXTWINDOW,				"S_NextWindow",				"",			NULL}, //次のウィンドウ
	{F_PREVWINDOW,				"S_PrevWindow",				"",			NULL}, //前のウィンドウ
	{F_MAXIMIZE_V,				"S_MaximizeV",				"",			NULL}, //縦方向に最大化
	{F_MAXIMIZE_H,				"S_MaximizeH",				"",			NULL}, //横方向に最大化 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			"S_MinimizeAll",			"",			NULL}, //すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	{F_REDRAW,					"S_ReDraw",					"",			NULL}, //再描画
	{F_WIN_OUTPUT,				"S_ActivateWinOutput",		"",			NULL}, //アウトプットウィンドウ表示

	/* 支援 */
	{F_HOKAN,					"S_Complete",				"",			NULL}, /* 入力補完 */	//Oct. 15, 2000 JEPRO 入ってなかったので英名を付けて入れてみた
	{F_HELP_CONTENTS,			"S_HelpContents",			"",			NULL}, /* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	{F_HELP_SEARCH,				"S_HelpSearch",				"",			NULL}, /* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	{F_MENU_ALLFUNC,			"S_CommandList",			"",			NULL}, /* コマンド一覧 */
	{F_EXTHELP1,				"S_ExtHelp1",				"",			NULL}, /* 外部ヘルプ１ */
	{F_EXTHTMLHELP,				"S_ExtHtmlHelp",			"",			NULL}, /* 外部HTMLヘルプ */
	{F_ABOUT,					"S_About",					"",			NULL}, /* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	//	終端
	{0,	NULL, NULL, NULL}
};
//int	CSMacroMgr::m_nMacroFuncInfoArrNum = sizeof( CSMacroMgr::m_MacroFuncInfoArr ) / sizeof( CSMacroMgr::m_MacroFuncInfoArr[0] );

CSMacroMgr::CSMacroMgr()
{
//	m_cShareData.Init();
	m_pShareData = CShareData::getInstance()->GetShareData();
	
	if ( CPPAMacroMgr::m_cPPA.Init() ){
		//	PPA.DLLアリ
		int i;
		for (i=0; i<MAX_CUSTMACRO; i++){
			m_cSavedKeyMacro[i] = new CPPAMacroMgr;
		}
		m_cKeyMacro = new CPPAMacroMgr;
	}
	else {
		//	PPA.DLLナシ
		int i;
		for (i=0; i<MAX_CUSTMACRO; i++){
			m_cSavedKeyMacro[i] = new CKeyMacroMgr;
		}
		m_cKeyMacro = new CKeyMacroMgr;
	}
}

CSMacroMgr::~CSMacroMgr()
{
	//- 20011229 add by aroka
	ClearAll();
	
	//	PPA.DLLアリナシ共通
	int i;
	for (i=0; i<MAX_CUSTMACRO; i++){
		delete m_cSavedKeyMacro[i];
	}
	delete m_cKeyMacro;
}

/* キーマクロのバッファをクリアする */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		m_cSavedKeyMacro[i]->ClearAll();
	}
	m_cKeyMacro->ClearAll();
}

/*! @briefキーマクロのバッファにデータ追加

	@param nFuncID [in] 機能番号
	@param lParam1 [in] パラメータ。
	@param mbuf [in] 読み込み先マクロバッファ

*/
int CSMacroMgr::Append( int idx, /*CSMacroMgr::Macro1& mbuf, */ int nFuncID, LPARAM lParam1, CEditView* pcEditView )
{
	if (idx == STAND_KEYMACRO){
		m_cKeyMacro->Append( nFuncID, lParam1, pcEditView );
	}
	else {
		m_cSavedKeyMacro[idx]->Append( nFuncID, lParam1, pcEditView );
	}
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
		m_cKeyMacro->ExecKeyMacro( pcEditView );
		return TRUE;	//	必ずTRUE？
	}
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	範囲チェック
		return FALSE;

	/* 読み込み前か、毎回読み込む設定の場合は、ファイルを読み込みなおす */
	if( !m_cSavedKeyMacro[idx]->IsReady() || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareDataから、マクロファイル名を取得
		char* p = CShareData::getInstance()->GetMacroFilename( idx );
		if ( p == NULL){
			return FALSE;
		}
		char ptr[_MAX_PATH * 2];
		strcpy(ptr, p);

		Clear( idx );	//	一度クリアしてから読み込む（念のため）
		if( !Load( idx, hInstance, ptr ) )
			return FALSE;
	}

	m_cSavedKeyMacro[idx]->ExecKeyMacro(pcEditView);
	pcEditView->Redraw();	//	必要？
	return TRUE;
}

/*! キーボードマクロの読み込み

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名

	@author Norio Nakatani
*/
BOOL CSMacroMgr::Load( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	if ( idx == STAND_KEYMACRO ){
		m_cKeyMacro->ClearAll();
		return m_cKeyMacro->LoadKeyMacro(hInstance, pszPath );
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		m_cSavedKeyMacro[idx]->ClearAll();
		return m_cSavedKeyMacro[idx]->LoadKeyMacro(hInstance, pszPath );
	}
	return FALSE;
}

/*! キーボードマクロの保存

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	if ( idx == STAND_KEYMACRO ){
		return m_cKeyMacro->SaveKeyMacro(hInstance, pszPath );
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return m_cSavedKeyMacro[idx]->SaveKeyMacro(hInstance, pszPath );
	}
	return FALSE;
}

void CSMacroMgr::Clear( int idx )
{
	if ( idx == STAND_KEYMACRO ){
		m_cKeyMacro->ClearAll();
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		m_cSavedKeyMacro[idx]->ClearAll();
	}
}

/*
||  Attributes & Operations
*/
/* 機能ID→関数名，機能名日本語 */
char* CSMacroMgr::GetFuncInfoByID( HINSTANCE hInstance, int nFuncID, char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFuncID ){
			strcpy( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName );
			char *p = pszFuncName;
			while (*p){
				if (*p == '('){
					*p = '\0';
					break;
				}
				*p++;
			}
			::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			return pszFuncName;
		}
	}
	return NULL;
}

/* 関数名（S_xxxx）→機能ID，機能名日本語 */
int CSMacroMgr::GetFuncInfoByName( HINSTANCE hInstance, const char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	int		nFuncID;
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == strcmp( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName ) ){
			nFuncID = m_MacroFuncInfoArr[i].m_nFuncID;
			::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
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
//	case F_FILE_REOPEN_SJIS			://SJISで開き直す
//	case F_FILE_REOPEN_JIS			://JISで開き直す
//	case F_FILE_REOPEN_EUC			://EUCで開き直す
//	case F_FILE_REOPEN_UNICODE		://Unicodeで開き直す
//	case F_FILE_REOPEN_UTF8			://UTF-8で開き直す
//	case F_FILE_REOPEN_UTF7			://UTF-7で開き直す
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
	case F_JUMPPREV					://移動履歴: 前へ
	case F_JUMPNEXT					://移動履歴: 次へ

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

	/* 変換系 */
	case F_TOLOWER		 			://英大文字→英小文字
	case F_TOUPPER		 			://英小文字→英大文字
	case F_TOHANKAKU		 		:/* 全角→半角 */
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
	case F_GREP						://Grep
//	case F_JUMP_DIALOG				://指定行ヘジャンプ
	case F_JUMP						://指定行へジャンプ @@@ 2002.2.2 YAZAKI
//	case F_OUTLINE					://アウトライン解析
	case F_TAGJUMP					://タグジャンプ機能
	case F_TAGJUMPBACK				://タグジャンプバック機能
//	case F_COMPARE					://ファイル内容比較
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
	case F_CANCEL_MODE				://各種モードの取り消し

	/* 設定系 */
//	case F_SHOWTOOLBAR				:/* ツールバーの表示 */
//	case F_SHOWFUNCKEY				:/* ファンクションキーの表示 */
//	case F_SHOWSTATUSBAR			:/* ステータスバーの表示 */
//	case F_TYPE_LIST				:/* タイプ別設定一覧 */
//	case F_OPTION_TYPE				:/* タイプ別設定 */
//	case F_OPTION					:/* 共通設定 */
//	case F_FONT						:/* フォント設定 */
//	case F_WRAPWINDOWWIDTH			:/* 現在のウィンドウ幅で折り返し */	//Oct. 15, 2000 JEPRO

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
/*[EOF]*/
