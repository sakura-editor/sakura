//	$Id$
/*!	@file
	キーボードマクロ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CMacro.h"
#include "funccode.h"

struct MacroFuncInfo {
	int		m_nFuncID;
	char*	m_pszFuncName;
};

MacroFuncInfo m_MacroFuncInfoArr[] =
{
//	F_DISABLE					0	//未使用

	/* ファイル操作系 */
	F_FILENEW					, "FileNew",			//新規作成
	F_FILEOPEN					, "FileOpen",			//開く
	F_FILESAVE					, "FileSave",			//上書き保存
	F_FILESAVEAS				, "FileSaveAs",			//名前を付けて保存
	F_FILECLOSE					, "FileClose",			//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	F_FILECLOSE_OPEN			, "FileCloseOpen",		//閉じて開く
	F_FILE_REOPEN_SJIS			, "FileReopenSJIS",		//SJISで開き直す
	F_FILE_REOPEN_JIS			, "FileReopenJIS",		//JISで開き直す
	F_FILE_REOPEN_EUC			, "FileReopenEUC",		//EUCで開き直す
	F_FILE_REOPEN_UNICODE		, "FileReopenUNICODE",	//Unicodeで開き直す
	F_FILE_REOPEN_UTF8			, "FileReopenUTF8",		//UTF-8で開き直す
	F_FILE_REOPEN_UTF7			, "FileReopenUTF7",		//UTF-7で開き直す
	F_PRINT						, "Print",				//印刷
//	F_PRINT_DIALOG				, "PrintDialog",		//印刷ダイアログ
	F_PRINT_PREVIEW				, "PrintPreview",		//印刷プレビュー
	F_PRINT_PAGESETUP			, "PrintPageSetup",		//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	F_OPEN_HfromtoC				, "OpenHfromtoC",		//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	F_OPEN_HHPP					, "OpenHHpp",			//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	F_OPEN_CCPP					, "OpenCCpp",			//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	F_ACTIVATE_SQLPLUS			, "ActivateSQLPLUS",	/* Oracle SQL*Plusをアクティブ表示 */
	F_PLSQL_COMPILE_ON_SQLPLUS	, "ExecSQLPLUS",		/* Oracle SQL*Plusで実行 */
	F_BROWSE					, "Browse",				//ブラウズ
	F_PROPERTY_FILE				, "PropertyFile",		//ファイルのプロパティ
	F_EXITALL					, "ExitAll",			//サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加

	/* 編集系 */
	F_CHAR						, "Char",				//文字入力
	F_IME_CHAR					, "CharIme",			//全角文字入力
	F_UNDO						, "Undo",				//元に戻す(Undo)
	F_REDO						, "Redo",				//やり直し(Redo)
	F_DELETE					, "Delete",				//削除
	F_DELETE_BACK				, "DeleteBack",			//カーソル前を削除
	F_WordDeleteToStart			, "WordDeleteToStart",	//単語の左端まで削除
	F_WordDeleteToEnd			, "WordDeleteToEnd",	//単語の右端まで削除
	F_WordCut					, "WordCut",			//単語切り取り
	F_WordDelete				, "WordDleete",			//単語削除
	F_LineCutToStart			, "LineCutToStart",		//行頭まで切り取り(改行単位)
	F_LineCutToEnd				, "LineCutToEnd",		//行末まで切り取り(改行単位)
	F_LineDeleteToStart			, "LineDeleteToStart",	//行頭まで削除(改行単位)
	F_LineDeleteToEnd			, "LineDeleteToEnd",	//行末まで削除(改行単位)
	F_CUT_LINE					, "CutLine",			//行切り取り(折り返し単位)
	F_DELETE_LINE				, "DeleteLine",			//行削除(折り返し単位)
	F_DUPLICATELINE				, "DuplicateLine",		//行の二重化(折り返し単位)
	F_INDENT_TAB				, "IndentTab",			//TABインデント
	F_UNINDENT_TAB				, "UnindentTab",		//逆TABインデント
	F_INDENT_SPACE				, "IndentSpace",		//SPACEインデント
	F_UNINDENT_SPACE			, "UnindentSpace",		//逆SPACEインデント
	F_WORDSREFERENCE			, "WordReference",		//単語リファレンス

	/* カーソル移動系 */
	F_UP						, "Up",				//カーソル上移動
	F_DOWN						, "Down",			//カーソル下移動
	F_LEFT						, "Left",			//カーソル左移動
	F_RIGHT						, "Right",			//カーソル右移動
	F_UP2						, "Up2",			//カーソル上移動(２行ごと)
	F_DOWN2						, "Down2",			//カーソル下移動(２行ごと)
	F_WORDLEFT					, "WordLeft",		//単語の左端に移動
	F_WORDRIGHT					, "WordRight",		//単語の右端に移動
	F_GOLINETOP					, "GoLineTop",		//行頭に移動(折り返し単位)
	F_GOLINEEND					, "GoLineEnd",		//行末に移動(折り返し単位)
//	F_ROLLDOWN					, "RollDown",		//スクロールダウン
//	F_ROLLUP					, "RollUp",			//スクロールアップ
	F_HalfPageUp				, "HalfPageUp",		//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_HalfPageDown				, "HalfPageDown",	//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_1PageUp					, "1PageUp",		//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	F_1PageDown					, "1PageDown",		//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	F_GOFILETOP					, "GoFileTop",		//ファイルの先頭に移動
	F_GOFILEEND					, "GoFileEnd",		//ファイルの最後に移動
	F_CURLINECENTER				, "CurLineCenter",	//カーソル行をウィンドウ中央へ
	F_JUMPPREV					, "MoveHistPrev",	//移動履歴: 前へ
	F_JUMPNEXT					, "MoveHistNext",	//移動履歴: 次へ
	F_WndScrollDown				, "F_WndScrollDown",//テキストを１行下へスクロール	// 2001/06/20 asa-o
	F_WndScrollUp				, "F_WndScrollUp",	//テキストを１行上へスクロール	// 2001/06/20 asa-o

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	F_SELECTWORD				, "SelectWord",			//現在位置の単語選択
	F_SELECTALL					, "SelectAll",			//すべて選択
	F_BEGIN_SEL					, "BeginSelect",		//範囲選択開始 Mar. 5, 2001 genta 名称修正
	F_UP_SEL					, "Up_Sel",				//(範囲選択)カーソル上移動
	F_DOWN_SEL					, "Down_Sel",			//(範囲選択)カーソル下移動
	F_LEFT_SEL					, "Left_Sel",			//(範囲選択)カーソル左移動
	F_RIGHT_SEL					, "Right_Sel",			//(範囲選択)カーソル右移動
	F_UP2_SEL					, "Up2_Sel",			//(範囲選択)カーソル上移動(２行ごと)
	F_DOWN2_SEL					, "Down2_Sel",			//(範囲選択)カーソル下移動(２行ごと)
	F_WORDLEFT_SEL				, "WordLeft_Sel",		//(範囲選択)単語の左端に移動
	F_WORDRIGHT_SEL				, "WordRight_Sel",		//(範囲選択)単語の右端に移動
	F_GOLINETOP_SEL				, "GoLineTop_Sel",		//(範囲選択)行頭に移動(折り返し単位)
	F_GOLINEEND_SEL				, "GoLineEnd_Sel",		//(範囲選択)行末に移動(折り返し単位)
//	F_ROLLDOWN_SEL				, "RollDown_Sel",		//(範囲選択)スクロールダウン
//	F_ROLLUP_SEL				, "RollUp_Sel",			//(範囲選択)スクロールアップ
	F_HalfPageUp_Sel			, "HalfPageUp_Sel",		//(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_HalfPageDown_Sel			, "HalfPageDown_Sel",	//(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_1PageUp_Sel				, "1PageUp_Sel",		//(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	F_1PageDown_Sel				, "1PageDown_Sel",		//(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	F_GOFILETOP_SEL				, "GoFileTop_Sel",		//(範囲選択)ファイルの先頭に移動
	F_GOFILEEND_SEL				, "GoFileEnd_Sel",		//(範囲選択)ファイルの最後に移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
//	case F_BOXSELALL			, "BoxSelectAll",		//矩形ですべて選択
	F_BEGIN_BOX					, "BeginBoxSelect",		//矩形範囲選択開始
/*
	F_UP_BOX					, "Up_Box", 			//(矩形選択)カーソル上移動
	F_DOWN_BOX					, "Down_Box",			//(矩形選択)カーソル下移動
	F_LEFT_BOX					, "Left_Box",			//(矩形選択)カーソル左移動
	F_RIGHT_BOX					, "Right_Box",			//(矩形選択)カーソル右移動
	F_UP2_BOX					, "Up2_Box",			//(矩形選択)カーソル上移動(２行ごと)
	F_DOWN2_BOX					, "Down2_Box",			//(矩形選択)カーソル下移動(２行ごと)
	F_WORDLEFT_BOX				, "WordLeft_Box",		//(矩形選択)単語の左端に移動
	F_WORDRIGHT_BOX				, "WordRight_Box",		//(矩形選択)単語の右端に移動
	F_GOLINETOP_BOX				, "GoLineTop_Box",		//(矩形選択)行頭に移動(折り返し単位)
	F_GOLINEEND_BOX				, "GoLineEnd_Box",		//(矩形選択)行末に移動(折り返し単位)
	F_HalfPageUp_Box			, "HalfPageUp_Box",		//(矩形選択)半ページアップ
	F_HalfPageDown_Box			, "HalfPAgeDown_Box",	//(矩形選択)半ページダウン
	F_1PageUp_Box				, "1PageUp_Box",		//(矩形選択)１ページアップ
	F_1PageDown_Box				, "1PageDown_Box",		//(矩形選択)１ページダウン
	F_GOFILETOP_BOX				, "GoFileTop_Box",		//(矩形選択)ファイルの先頭に移動
	F_GOFILEEND_BOX				, "GoFileEnd_Box",		//(矩形選択)ファイルの最後に移動
*/

	/* クリップボード系 */
	F_CUT						, "Cut",					//切り取り(選択範囲をクリップボードにコピーして削除)
	F_COPY						, "Copy",					//コピー(選択範囲をクリップボードにコピー)
	F_PASTE						, "Paste",					//貼り付け(クリップボードから貼り付け)
	F_COPY_CRLF					, "CopyCRLF",				//CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	F_PASTEBOX					, "PasteBox",				//矩形貼り付け(クリップボードから矩形貼り付け)
	F_INSTEXT					, "InsText",				// テキストを貼り付け
	F_ADDTAIL					, "AddTail",				// 最後にテキストを追加
	F_COPYLINES					, "CopyLines",				//選択範囲内全行コピー
	F_COPYLINESASPASSAGE		, "CopyLinesAsPassage",		//選択範囲内全行引用符付きコピー
	F_COPYLINESWITHLINENUMBER 	, "CopyLinesWithLineNumber",//選択範囲内全行行番号付きコピー
	F_COPYPATH					, "CopyPath",				//このファイルのパス名をクリップボードにコピー
	F_COPYTAG					, "CopyTag",				//このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	F_CREATEKEYBINDLIST			, "CopyKeyBindList",		//キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加 //Dec. 25, 2000 復活

	/* 挿入系 */
	F_INS_DATE					, "InsertDate",			// 日付挿入
	F_INS_TIME					, "InsertTime",			// 時刻挿入

	/* 変換系 */
	F_TOLOWER		 			, "ToLower",			//英大文字→英小文字
	F_TOUPPER		 			, "ToUpper",			//英小文字→英大文字
	F_TOHANKAKU		 			, "ToHankaku",			/* 全角→半角 */
	F_TOZENEI		 			, "ToZenEi",			/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	F_TOZENKAKUKATA	 			, "ToZenKata",			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	F_TOZENKAKUHIRA	 			, "ToZenHira",			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	F_HANKATATOZENKAKUKATA		, "HanKataToZenKata",	/* 半角カタカナ→全角カタカナ */
	F_HANKATATOZENKAKUHIRA		, "HanKataToZenHira",	/* 半角カタカナ→全角ひらがな */
	F_TABTOSPACE				, "TABToSPACE",			/* TAB→空白 */
	F_SPACETOTAB				, "SPACEToTAB",			/* 空白→TAB */ //#### Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS			, "AutoToSJIS",			/* 自動判別→SJISコード変換 */
	F_CODECNV_EMAIL				, "JIStoSJIS",			//E-Mail(JIS→SJIS)コード変換
	F_CODECNV_EUC2SJIS			, "EUCtoSJIS",			//EUC→SJISコード変換
	F_CODECNV_UNICODE2SJIS		, "CodeCnvUNICODEtoJIS",//Unicode→SJISコード変換
	F_CODECNV_UTF82SJIS			, "UTF8toSJIS",			/* UTF-8→SJISコード変換 */
	F_CODECNV_UTF72SJIS			, "UTF7toSJIS",			/* UTF-7→SJISコード変換 */
	F_CODECNV_SJIS2JIS			, "SJIStoJIS",			/* SJIS→JISコード変換 */
	F_CODECNV_SJIS2EUC			, "SJIStoEUC",			/* SJIS→EUCコード変換 */
	F_CODECNV_SJIS2UTF8			, "SJIStoUTF8",			/* SJIS→UTF-8コード変換 */
	F_CODECNV_SJIS2UTF7			, "SJIStoUTF7",			/* SJIS→UTF-7コード変換 */
	F_BASE64DECODE	 			, "Base64Decode",		//Base64デコードして保存
	F_UUDECODE		 			, "Uudecode",			//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


	/* 検索系 */
	F_SEARCH_DIALOG				, "SearchDialog",		//検索(単語検索ダイアログ)
	F_SEARCH_NEXT				, "SearchNext",			//次を検索
	F_SEARCH_PREV				, "SearchPrev",			//前を検索
	F_REPLACE					, "Replace",			//置換(置換ダイアログ)
	F_SEARCH_CLEARMARK			, "SearchClearMark",	//検索マークのクリア
	F_GREP						, "Grep",				//Grep
	F_JUMP						, "Jump",				//指定行ヘジャンプ
	F_OUTLINE					, "Outline",			//アウトライン解析
	F_TAGJUMP					, "TagJump",			//タグジャンプ機能
	F_TAGJUMPBACK				, "TagJumpBack",		//タグジャンプバック機能
	F_COMPARE					, "Compare",			//ファイル内容比較
	F_BRACKETPAIR				, "BracketPair",		//対括弧の検索

	/* モード切り替え系 */
	F_CHGMOD_INS				, "ChgmodINS",		//挿入／上書きモード切り替え
	F_CANCEL_MODE				, "CancelMode",		//各種モードの取り消し

	/* 設定系 */
	F_SHOWTOOLBAR				, "ShowToolbar",	/* ツールバーの表示 */
	F_SHOWFUNCKEY				, "ShowFunckey",	/* ファンクションキーの表示 */
	F_SHOWSTATUSBAR				, "ShowStatusbar",	/* ステータスバーの表示 */
	F_TYPE_LIST					, "TypeList",		/* タイプ別設定一覧 */
	F_OPTION_TYPE				, "OptionType",		/* タイプ別設定 */
	F_OPTION					, "OptionCommon",	/* 共通設定 */
	F_FONT						, "SelectFont",		/* フォント設定 */
	F_WRAPWINDOWWIDTH			, "WrapWindowWidth",/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更

	/* カスタムメニュー */
	F_MENU_RBUTTON				, "RMenu",			/* 右クリックメニュー */
	F_CUSTMENU_1				, "CustMenu1",		/* カスタムメニュー1 */
	F_CUSTMENU_2				, "CustMenu2",		/* カスタムメニュー2 */
	F_CUSTMENU_3				, "CustMenu3",		/* カスタムメニュー3 */
	F_CUSTMENU_4				, "CustMenu4",		/* カスタムメニュー4 */
	F_CUSTMENU_5				, "CustMenu5",		/* カスタムメニュー5 */
	F_CUSTMENU_6				, "CustMenu6",		/* カスタムメニュー6 */
	F_CUSTMENU_7				, "CustMenu7",		/* カスタムメニュー7 */
	F_CUSTMENU_8				, "CustMenu8",		/* カスタムメニュー8 */
	F_CUSTMENU_9				, "CustMenu9",		/* カスタムメニュー9 */
	F_CUSTMENU_10				, "CustMenu10",		/* カスタムメニュー10 */
	F_CUSTMENU_11				, "CustMenu11",		/* カスタムメニュー11 */
	F_CUSTMENU_12				, "CustMenu12",		/* カスタムメニュー12 */
	F_CUSTMENU_13				, "CustMenu13",		/* カスタムメニュー13 */
	F_CUSTMENU_14				, "CustMenu14",		/* カスタムメニュー14 */
	F_CUSTMENU_15				, "CustMenu15",		/* カスタムメニュー15 */
	F_CUSTMENU_16				, "CustMenu16",		/* カスタムメニュー16 */
	F_CUSTMENU_17				, "CustMenu17", 	/* カスタムメニュー17 */
	F_CUSTMENU_18				, "CustMenu18",		/* カスタムメニュー18 */
	F_CUSTMENU_19				, "CustMenu19",		/* カスタムメニュー19 */
	F_CUSTMENU_20				, "CustMenu20",		/* カスタムメニュー20 */
	F_CUSTMENU_21				, "CustMenu21",		/* カスタムメニュー21 */
	F_CUSTMENU_22				, "CustMenu22",		/* カスタムメニュー22 */
	F_CUSTMENU_23				, "CustMenu23",		/* カスタムメニュー23 */
	F_CUSTMENU_24				, "CustMenu24",		/* カスタムメニュー24 */

	/* ウィンドウ系 */
	F_SPLIT_V					, "SplitWinV",			//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	F_SPLIT_H					, "SplitWinH",			//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	F_SPLIT_VH					, "SplitWinVH",			//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	F_WINCLOSE					, "WinClose",			//ウィンドウを閉じる
	F_WIN_CLOSEALL				, "WinCloseAll",		//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	F_CASCADE					, "CascadeWin",			//重ねて表示
	F_TILE_V					, "TileWinV",			//上下に並べて表示
	F_TILE_H					, "TileWinH",			//左右に並べて表示
	F_NEXTWINDOW				, "NextWindow",			//次のウィンドウ
	F_PREVWINDOW				, "PrevWindow",			//前のウィンドウ
	F_MAXIMIZE_V				, "MaximizeV",			//縦方向に最大化
	F_MAXIMIZE_H				, "MaximizeH",			//横方向に最大化 //2001.02.10 by MIK
	F_MINIMIZE_ALL				, "MinimizeAll",		//すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	F_REDRAW					, "ReDraw",				//再描画
	F_WIN_OUTPUT				, "ActivateWinOutput",	//アウトプットウィンドウ表示

	/* 支援 */
	F_HOKAN						, "Complete",		/* 入力補完 */	//Oct. 15, 2000 JEPRO 入ってなかったので英名を付けて入れてみた
	F_HELP_CONTENTS				, "HelpContents",	/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	F_HELP_SEARCH				, "HelpSearch",		/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	F_MENU_ALLFUNC				, "CommandList",	/* コマンド一覧 */
	F_EXTHELP1					, "ExtHelp1",		/* 外部ヘルプ１ */
	F_EXTHTMLHELP				, "ExtHtmlHelp",	/* 外部HTMLヘルプ */
	F_ABOUT						, "About",			/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */
//	F_SENDMAIL					, "SendMail",		/* メール送信 */	//Oct. 17, 2000 JEPRO メール機能は死んでいるのでコメントアウトにした

};
int	m_nMacroFuncInfoArrNum = sizeof( m_MacroFuncInfoArr ) / sizeof( m_MacroFuncInfoArr[0] );



CMacro::CMacro( void )
{
	return;
}


CMacro::~CMacro( void )
{
	return;
}

/*
||  Attributes & Operations
*/
/* 機能ID→関数名，機能名日本語 */
char* CMacro::GetFuncInfoByID( HINSTANCE hInstance, int nFincID, char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	for( i = 0; i < m_nMacroFuncInfoArrNum; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFincID ){
			strcpy( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName );
			::LoadString( hInstance, nFincID, pszFuncNameJapanese, 255 );
			return pszFuncName;
		}
	}
	return NULL;
}

/* 関数名→機能ID，機能名日本語 */
int CMacro::GetFuncInfoByName( HINSTANCE hInstance, const char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	int		nFincID;
	for( i = 0; i < m_nMacroFuncInfoArrNum; ++i ){
		if( 0 == strcmp( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName ) ){
			nFincID = m_MacroFuncInfoArr[i].m_nFuncID;
			::LoadString( hInstance, nFincID, pszFuncNameJapanese, 255 );
			return nFincID;
		}
	}
	return -1;
}

/* キーマクロに記録可能な機能かどうかを調べる */
BOOL CMacro::CanFuncIsKeyMacro( int nFuncID )
{
	switch( nFuncID ){
	/* ファイル操作系 */
//	case F_FILENEW					://新規作成
//	case F_FILEOPEN					://開く
//	case F_FILESAVE					://上書き保存
//	case F_FILESAVEAS				://名前を付けて保存
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
	case F_CREATEKEYBINDLIST		://キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加	//Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE					:// 日付挿入
	case F_INS_TIME					:// 時刻挿入

	/* 変換系 */
	case F_TOLOWER		 			://英大文字→英小文字
	case F_TOUPPER		 			://英小文字→英大文字
	case F_TOHANKAKU		 		:/* 全角→半角 */
	case F_TOZENEI			 		:/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
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
//	case F_REPLACE					://置換(置換ダイアログ)
	case F_SEARCH_CLEARMARK			://検索マークのクリア
//	case F_GREP						://Grep
//	case F_JUMP						://指定行ヘジャンプ
//	case F_OUTLINE					://アウトライン解析
	case F_TAGJUMP					://タグジャンプ機能
	case F_TAGJUMPBACK				://タグジャンプバック機能
//	case F_COMPARE					://ファイル内容比較
	case F_BRACKETPAIR				://対括弧の検索

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
//	case F_REDRAW					://再描画
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
