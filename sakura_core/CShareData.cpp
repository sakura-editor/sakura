//	$Id$
/*!	@file
	プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date 1998/05/26  新規作成
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

/*! @file
	プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date May. 26, 1998
*/

//#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "CEditApp.h"
#include "mymessage.h"
#include "debug.h"
#include "global.h"
#include "etc_uty.h"
struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

//!	共有メモリのバージョン
/*!
	共有メモリのバージョン番号。共有メモリの形式を変更したときはここを1増やす。

	この値は共有メモリのバージョンフィールドに格納され、異なる構造の共有メモリを
	使うエディタが同時に起動しないようにする。

	設定可能な値は 1～unsinged intの最大値

	@sa Init()
*/
const unsigned int uShareDataVersion = 15;

/*!
	共有メモリ領域がある場合はプロセスのアドレス空間から､
	すでにマップされているファイル ビューをアンマップする。
*/
CShareData::~CShareData()
{
	if( NULL != m_pShareData ){
		/* プロセスのアドレス空間から､ すでにマップされているファイル ビューをアンマップします */
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
	return;
}


//! CShareDataクラスの初期化処理
/*!
	CShareDataクラスを利用する前に必ず呼び出すこと。

	@retval true 初期化成功
	@retval false 初期化失敗

	@note 既に存在する共有メモリのバージョンがこのエディタが使うものと
	異なる場合は致命的エラーを防ぐためにfalseを返します。WinMain()
	でInit()に失敗するとメッセージを出してエディタの起動を中止します。
*/
bool CShareData::Init( void )
{
	int		i;
	int		j;
	char	szExeFolder[_MAX_PATH + 1];
	char	szPath[_MAX_PATH + 1];
//	int		nCharChars;
//	char	szDrive[_MAX_DRIVE];
//	char	szDir[_MAX_DIR];

	/* exeのあるフォルダ */
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof( szPath )
	);
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, szExeFolder, NULL );
	strcat( szExeFolder, "\\" );

	/* ファイルマッピングオブジェクト */
	m_hFileMap = ::CreateFileMapping(
		(HANDLE)0xFFFFFFFF,
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof( DLLSHAREDATA ),
		m_pszAppName
	);
	if( NULL == m_hFileMap ){
		::MessageBox(
			NULL,
			"CreateFileMapping()に失敗しました",
			"予期せぬエラー",
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}
	if( GetLastError() != ERROR_ALREADY_EXISTS ){
		/* オブジェクトが存在していなかった場合 */
		/* ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		m_pShareData->m_vStructureVersion = uShareDataVersion;
		m_pShareData->m_CKeyMacroMgr.Clear();			/* キーワードマクロのバッファ */
		m_pShareData->m_bRecordingKeyMacro = FALSE;		/* キーボードマクロの記録中 */
		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */
		m_pShareData->m_dwProductVersionMS = 0L;
		m_pShareData->m_dwProductVersionLS = 0L;
		m_pShareData->m_hwndTray = NULL;
		m_pShareData->m_hAccel = NULL;
		m_pShareData->m_hwndDebug = NULL;
		m_pShareData->m_nSequences = 0;					/* ウィンドウ連番 */
		m_pShareData->m_nEditArrNum = 0;
		m_pShareData->m_Common.m_nMRUArrNum_MAX = 15;	/* ファイルの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
		m_pShareData->m_nMRUArrNum = 0;
		for( i = 0; i < MAX_MRU; ++i ){
			m_pShareData->m_fiMRUArr[i].m_nViewTopLine = 0;
			m_pShareData->m_fiMRUArr[i].m_nViewLeftCol = 0;
//			m_pShareData->m_fiMRUArr[i].m_nCaretPosX = 0;
//			m_pShareData->m_fiMRUArr[i].m_nCaretPosY = 0;
			m_pShareData->m_fiMRUArr[i].m_nX = 0;
			m_pShareData->m_fiMRUArr[i].m_nY = 0;
			m_pShareData->m_fiMRUArr[i].m_bIsModified = 0;
			m_pShareData->m_fiMRUArr[i].m_nCharCode = 0;
			strcpy( m_pShareData->m_fiMRUArr[i].m_szPath, "" );
		}

		m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX = 15;	/* フォルダの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
		m_pShareData->m_nOPENFOLDERArrNum = 0;
		for( i = 0; i < MAX_OPENFOLDER; ++i ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
		}

		m_pShareData->m_nSEARCHKEYArrNum = 0;
		for( i = 0; i < MAX_SEARCHKEY; ++i ){
			strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
		}
		m_pShareData->m_nREPLACEKEYArrNum = 0;
		for( i = 0; i < MAX_REPLACEKEY; ++i ){
			strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
		}
		m_pShareData->m_nGREPFILEArrNum = 0;
		for( i = 0; i < MAX_GREPFILE; ++i ){
			strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
		}
		m_pShareData->m_nGREPFILEArrNum = 1;
		strcpy( m_pShareData->m_szGREPFILEArr[0], "*.*" );

		m_pShareData->m_nGREPFOLDERArrNum = 0;
		for( i = 0; i < MAX_GREPFOLDER; ++i ){
			strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
		}
		strcpy( m_pShareData->m_szMACROFOLDER, szExeFolder );	/* マクロ用フォルダ */
		strcpy( m_pShareData->m_szIMPORTFOLDER, szExeFolder );	/* 設定インポート用フォルダ */

		i = 0;
		wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "印刷設定 %d", i + 1 );	/* 印刷設定の名前 */
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan, "ＭＳ 明朝" );				/* 印刷フォント */
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen, "ＭＳ 明朝" );				/* 印刷フォント */
		m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth = 12;  								/* 印刷フォント幅(1/10mm単位) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight = m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth * 2;	/* 印刷フォント高さ(1/10mm単位単位) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu = 1;			/* 段組の段数 */
		m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace = 70; 		/* 段と段の隙間(1/10mm) */
		m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap = TRUE;		/* 英文ワードラップする */
		m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber = FALSE;	/* 行番号を印刷する */
		m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing = 30;	/* 印刷フォント行間 文字の高さに対する割合(%) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY = 100;		/* 印刷用紙マージン 上(1/10mm単位) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY = 200;		/* 印刷用紙マージン 下(1/10mm単位) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX = 200;		/* 印刷用紙マージン 左(1/10mm単位) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX = 100;		/* 印刷用紙マージン 右(1/10mm単位) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;	/* 用紙方向 DMORIENT_PORTRAIT (1) または DMORIENT_LANDSCAPE (2) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize = DMPAPER_A4;	/* 用紙サイズ */
		/* プリンタ設定 DEVMODE用 */
		CPrint::GetDefaultPrinterInfo( &(m_pShareData->m_PrintSettingArr[i].m_mdmDevMode) );
		m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0] = TRUE;
		m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1] = FALSE;
		m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2] = FALSE;
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0], "&f" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[1], "" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[2], "" );
		m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0] = TRUE;
		m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1] = FALSE;
		m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2] = FALSE;
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0], "&C- &P -" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[1], "" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[2], "" );
		for( i = 1; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
			wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "印刷設定 %d", i + 1 );	/* 印刷設定の名前 */
		}
//キーワード：デフォルトキー割り当て
/********************/
/* 共通設定の規定値 */
/********************/
		struct KEYDATAINIT {
			short			nKeyCode;
			char*			pszKeyName;
			short			nFuncCode_0;
			short			nFuncCode_1;
			short			nFuncCode_2;
			short			nFuncCode_3;
			short			nFuncCode_4;
			short			nFuncCode_5;
			short			nFuncCode_6;
			short			nFuncCode_7;
		};
		static KEYDATAINIT	KeyDataInit[] = {
		//Sept. 1, 2000 Jepro note: key binding
		//Feb. 17, 2001 jepro note 2: 順番は2進で下位3ビット[Alt][Ctrl][Shift]の組合せの順(それに2を加えた値)
		//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)
		//		keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		//
			/* マウスボタン */
			{ 0, "ダブルクリック",F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD },
		//Feb. 19, 2001 JEPRO Altと右クリックの組合せは効かないので右クリックメニューのキー割り当てをはずした
			{ 0, "右クリック",F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, 0, 0, 0, 0 },
			/* ファンクションキー */
		//	From Here Sept. 14, 2000 JEPRO
		//	VK_F1,"F1", F_EXTHTMLHELP, 0, F_EXTHELP1, 0, 0, 0, 0, 0,
		//	Shift+F1 に「コマンド一覧」, Alt+F1 に「ヘルプ目次」, Shift+Alt+F1 に「キーワード検索」を追加	//Nov. 25, 2000 JEPRO 殺していたのを修正・復活
		//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 に「バージョン情報」を追加
		//	{ VK_F1,"F1", F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, 0, 0, 0, 0, 0 },
			{ VK_F1,"F1", F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_ABOUT, F_HELP_CONTENTS, F_HELP_SEARCH, 0, 0 },
		//	To Here Sept. 14, 2000
			{ VK_F2,"F2", 0, 0, 0, 0, 0, 0, 0, 0 },
			//Sept. 21, 2000 JEPRO	Ctrl+F3 に「検索マークのクリア」を追加
			{ VK_F3,"F3", F_SEARCH_NEXT, F_SEARCH_PREV, F_SEARCH_CLEARMARK, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Alt+F4 に「ウィンドウを閉じる」, Shift+Alt+F4 に「すべてのウィンドウを閉じる」を追加
			//	Ctrl+F4に割り当てられていた「縦横に分割」を「閉じて(無題)」に変更し Shift+Ctrl+F4 に「閉じて開く」を追加
			//Jan. 14, 2001 Ctrl+Alt+F4 に「テキストエディタの全終了」を追加
			//Jun. 2001「サクラエディタの全終了」に改称
			{ VK_F4,"F4", F_SPLIT_V, F_SPLIT_H, F_FILECLOSE, F_FILECLOSE_OPEN, F_WINCLOSE, F_WIN_CLOSEALL, F_EXITALL, 0 },
		//	From Here Sept. 20, 2000 JEPRO Ctrl+F5 に「外部コマンド実行」を追加  なおマクロ名はCMMAND からCOMMAND に変更済み
		//	{ VK_F5,"F5", F_PLSQL_COMPILE_ON_SQLPLUS, 0, F_EXECCOMMAND, 0, 0, 0, 0, 0 },
		//	To Here Sept. 20, 2000
			//Oct. 28, 2000 F5 は「再描画」に変更	//Jan. 14, 2001 Alt+F5 に「uudecodeして保存」, Ctrl+ Alt+F5 に「TAB→空白」を追加
			//	May 28, 2001 genta	S-C-A-F5にSPACE-to-TABを追加
			{ VK_F5,"F5", F_REDRAW, 0, F_EXECCOMMAND, 0, F_UUDECODE, 0, F_TABTOSPACE, F_SPACETOTAB },
			//Jan. 14, 2001 JEPRO	Ctrl+F6 に「英大文字→英小文字」, Alt+F6 に「Base64デコードして保存」を追加
			{ VK_F6,"F6", F_BEGIN_SEL, F_BEGIN_BOX, F_TOLOWER, 0, F_BASE64DECODE, 0, 0, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+F7 に「英小文字→英大文字」, Alt+F7 に「UTF-7→SJISコード変換」, Shift+Alt+F7 に「SJIS→UTF-7コード変換」, Ctrl+Alt+F7 に「UTF-7で開き直す」を追加
			{ VK_F7,"F7", F_CUT, 0, F_TOUPPER, 0, F_CODECNV_UTF72SJIS, F_CODECNV_SJIS2UTF7, F_FILE_REOPEN_UTF7, 0 },
			//Nov. 9, 2000 JEPRO	Shift+F8 に「CRLF改行でコピー」を追加
			//Jan. 14, 2001 JEPRO	Ctrl+F8 に「全角→半角」, Alt+F8 に「UTF-8→SJISコード変換」, Shift+Alt+F8 に「SJIS→UTF-8コード変換」, Ctrl+Alt+F8 に「UTF-8で開き直す」を追加
			{ VK_F8,"F8", F_COPY, F_COPY_CRLF, F_TOHANKAKU, 0, F_CODECNV_UTF82SJIS, F_CODECNV_SJIS2UTF8, F_FILE_REOPEN_UTF8, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+F9 に「半角＋全ひら→全角・カタカナ」, Alt+F9 に「Unicode→SJISコード変換」, Ctrl+Alt+F9 に「Unicodeで開き直す」を追加
			{ VK_F9,"F9", F_PASTE, F_PASTEBOX, F_TOZENKAKUKATA, 0, F_CODECNV_UNICODE2SJIS, 0, F_FILE_REOPEN_UNICODE, 0 },
			//Oct. 28, 2000 JEPRO F10 に「SQL*Plusで実行」を追加(F5からの移動)
			//Jan. 14, 2001 JEPRO	Ctrl+F10 に「半角＋全カタ→全角・ひらがな」, Alt+F10 に「EUC→SJISコード変換」, Shift+Alt+F10 に「SJIS→EUCコード変換」, Ctrl+Alt+F10 に「EUCで開き直す」を追加
			{ VK_F10,"F10", F_PLSQL_COMPILE_ON_SQLPLUS, F_DUPLICATELINE, F_TOZENKAKUHIRA, 0, F_CODECNV_EUC2SJIS, F_CODECNV_SJIS2EUC, F_FILE_REOPEN_EUC, 0 },
			//Jan. 14, 2001 JEPRO	Shift+F11 に「SQL*Plusをアクティブ表示」, Ctrl+F11 に「半角カタカナ→全角カタカナ」, Alt+F11 に「E-Mail(JIS→SJIS)コード変換」, Shift+Alt+F11 に「SJIS→JISコード変換」, Ctrl+Alt+F11 に「JISで開き直す」を追加
			{ VK_F11,"F11", F_OUTLINE, F_ACTIVATE_SQLPLUS, F_HANKATATOZENKAKUKATA, 0, F_CODECNV_EMAIL, F_CODECNV_SJIS2JIS, F_FILE_REOPEN_JIS, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+F12 に「半角カタカナ→全角ひらがな」, Alt+F12 に「自動判別→SJISコード変換」, Ctrl+Alt+F11 に「SJISで開き直す」を追加
			{ VK_F12,"F12", F_TAGJUMP, F_TAGJUMPBACK, F_HANKATATOZENKAKUHIRA, 0, F_CODECNV_AUTO2SJIS, 0, F_FILE_REOPEN_SJIS, 0 },
			{ VK_F13,"F13", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F14,"F14", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F15,"F15", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F16,"F16", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F17,"F17", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F18,"F18", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F19,"F19", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F20,"F20", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F21,"F21", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F22,"F22", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F23,"F23", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F24,"F24", 0, 0, 0, 0, 0, 0, 0, 0 },
			/* 特殊キー */
			{ VK_TAB,"Tab",F_INDENT_TAB, F_UNINDENT_TAB, F_NEXTWINDOW, F_PREVWINDOW, 0, 0, 0, 0 },
			//Sept. 1, 2000 JEPRO	Alt+Enter に「ファイルのプロパティ」を追加	//Oct. 15, 2000 JEPRO Ctrl+Enter に「ファイル内容比較」を追加
 			{ VK_RETURN,"Enter",0, 0, F_COMPARE, 0, F_PROPERTY_FILE, 0, 0, 0 },
			{ VK_ESCAPE,"Esc",F_CANCEL_MODE, 0, 0, 0, 0, 0, 0, 0 },
//			{ VK_BACK,"BackSpace",F_DELETE_BACK, 0, F_WordDeleteToStart, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO 長いので名称を簡略形に変更(BackSpace→BkSp)
			{ VK_BACK,"BkSp",F_DELETE_BACK, 0, F_WordDeleteToStart, 0, 0, 0, 0, 0 },
//			{ VK_INSERT,"Insert",F_CHGMOD_INS, F_PASTE, F_COPY, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Insert→Ins)
			{ VK_INSERT,"Ins",F_CHGMOD_INS, F_PASTE, F_COPY, 0, 0, 0, 0, 0 },
//			{ VK_DELETE,"Delete",F_DELETE, 0, F_WordDeleteToEnd, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Delete→Del)
			//Jun. 26, 2001 JEPRO	Shift+Del に「切り取り」を追加
			{ VK_DELETE,"Del",F_DELETE, F_CUT, F_WordDeleteToEnd, 0, 0, 0, 0, 0 },
			{ VK_HOME,"Home",F_GOLINETOP, F_GOLINETOP_SEL, F_GOFILETOP, F_GOFILETOP_SEL, 0, 0, 0, 0 },
			{ VK_END,"End(Help)",F_GOLINEEND, F_GOLINEEND_SEL, F_GOFILEEND, F_GOFILEEND_SEL, 0, 0, 0, 0 },
			{ VK_LEFT,"←",F_LEFT, F_LEFT_SEL/*F_GOLINETOP*/, F_WORDLEFT, F_WORDLEFT_SEL, F_BEGIN_BOX, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+↑に「縦方向に最大化」を追加
//			{ VK_UP,"↑",F_UP, F_UP_SEL, F_UP2, F_UP2_SEL, F_BEGIN_BOX, 0, 0, F_MAXIMIZE_V },
			//Jun. 27, 2001 JEPRO
			//	Ctrl+↑に割り当てられていた「カーソル上移動(２行ごと)」を「テキストを１行下へスクロール」に変更
			{ VK_UP,"↑",F_UP, F_UP_SEL, F_WndScrollDown, F_UP2_SEL, F_BEGIN_BOX, 0, 0, F_MAXIMIZE_V },
			//2001.02.10 by MIK Shift+Ctrl+Alt+→に「横方向に最大化」を追加
			{ VK_RIGHT,"→",F_RIGHT, F_RIGHT_SEL/*F_GOLINEEND*/, F_WORDRIGHT, F_WORDRIGHT_SEL, F_BEGIN_BOX, 0, 0, F_MAXIMIZE_H },
			//Sept. 14, 2000 JEPRO
			//	Ctrl+↓に割り当てられていた「右クリックメニュー」を「カーソル下移動(２行ごと)」に変更
			//	それに付随してさらに「右クリックメニュー」をCtrl＋Alt＋↓に変更
//			{ VK_DOWN,"↓",F_DOWN, F_DOWN_SEL, F_DOWN2, F_DOWN2_SEL, F_BEGIN_BOX, 0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
			//Jun. 27, 2001 JEPRO
			//	Ctrl+↓に割り当てられていた「カーソル下移動(２行ごと)」を「テキストを１行上へスクロール」に変更
			{ VK_DOWN,"↓",F_DOWN, F_DOWN_SEL, F_WndScrollUp, F_DOWN2_SEL, F_BEGIN_BOX, 0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
//			{ VK_PRIOR,"RollDown(PageUp)",F_ROLLDOWN, F_ROLLDOWN_SEL, 0, 0, 0, 0, 0, 0 },
//			{ VK_NEXT,"RollUp(PageDown)",F_ROLLUP, F_ROLLUP_SEL, 0, 0, 0, 0, 0, 0 },
			//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページダウン」, 「(選択)１ページダウン」を追加
			//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollUp→PgDn) //Oct. 10, 2000 JEPRO 名称変更
			{ VK_NEXT,"PgDn(RollUp)",F_HalfPageDown, F_HalfPageDown_Sel, F_1PageDown, F_1PageDown_Sel, 0, 0, 0, 0 },
			//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページアップ」, 「(選択)１ページアップ」を追加
			//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollDown→PgUp) //Oct. 10, 2000 JEPRO 名称変更
			{ VK_PRIOR,"PgUp(RollDn)",F_HalfPageUp, F_HalfPageUp_Sel, F_1PageUp, F_1PageUp_Sel, 0, 0, 0, 0 },
//			{ VK_SPACE,"SpaceBar",F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(SpaceBar→Space)
			{ VK_SPACE,"Space",F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, 0, 0, 0, 0, 0 },
			/* 数字 */
			//Oct. 7, 2000 JEPRO	Ctrl+0 を「タイプ別設定一覧」→「未定義」に変更
			//Jan. 13, 2001 JEPRO	Alt+0 に「カスタムメニュー10」, Shift+Alt+0 に「カスタムメニュー20」を追加
			{ '0', "0",0, 0, 0, 0, F_CUSTMENU_10, F_CUSTMENU_20, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+1 を「タイプ別設定」→「ツールバーの表示」に変更
			//Jan. 13, 2001 JEPRO	Alt+1 に「カスタムメニュー1」, Shift+Alt+1 に「カスタムメニュー11」を追加
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 に「カスタムメニュー21」を追加
			{ '1', "1",0, 0, F_SHOWTOOLBAR, F_CUSTMENU_21, F_CUSTMENU_1, F_CUSTMENU_11, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+2 を「共通設定」→「ファンクションキーの表示」に変更
			//Jan. 13, 2001 JEPRO	Alt+2 を「アウトプット」→「カスタムメニュー2」に変更し「アウトプット」は Alt+O に移動, Shift+Alt+2 に「カスタムメニュー12」を追加
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 に「カスタムメニュー22」を追加
			{ '2', "2",0, 0, F_SHOWFUNCKEY, F_CUSTMENU_22, F_CUSTMENU_2/*F_WIN_OUTPUT*/, F_CUSTMENU_12, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+3 を「フォント設定」→「ステータスバーの表示」に変更
			//Jan. 13, 2001 JEPRO	Alt+3 に「カスタムメニュー3」, Shift+Alt+3 に「カスタムメニュー13」を追加
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 に「カスタムメニュー23」を追加
			{ '3', "3",0, 0, F_SHOWSTATUSBAR, F_CUSTMENU_23, F_CUSTMENU_3, F_CUSTMENU_13, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+4 を「ツールバーの表示」→「タイプ別設定一覧」に変更
			//Jan. 13, 2001 JEPRO	Alt+4 に「カスタムメニュー4」, Shift+Alt+4 に「カスタムメニュー14」を追加
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 に「カスタムメニュー24」を追加
			{ '4', "4",0, 0, F_TYPE_LIST, F_CUSTMENU_24, F_CUSTMENU_4, F_CUSTMENU_14, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+5 を「ファンクションキーの表示」→「タイプ別設定」に変更
			//Jan. 13, 2001 JEPRO	Alt+5 に「カスタムメニュー5」, Shift+Alt+5 に「カスタムメニュー15」を追加
			{ '5', "5",0, 0, F_OPTION_TYPE, 0, F_CUSTMENU_5, F_CUSTMENU_15, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+6 を「ステータスバーの表示」→「共通設定」に変更
			//Jan. 13, 2001 JEPRO	Alt+6 に「カスタムメニュー6」, Shift+Alt+6 に「カスタムメニュー16」を追加
			{ '6', "6",0, 0, F_OPTION, 0, F_CUSTMENU_6, F_CUSTMENU_16, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+7 に「フォント設定」を追加
			//Jan. 13, 2001 JEPRO	Alt+7 に「カスタムメニュー7」, Shift+Alt+7 に「カスタムメニュー17」を追加
			{ '7', "7",0, 0, F_FONT, 0, F_CUSTMENU_7, F_CUSTMENU_17, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+8 に「カスタムメニュー8」, Shift+Alt+8 に「カスタムメニュー18」を追加
			{ '8', "8",0, 0, 0, 0, F_CUSTMENU_8, F_CUSTMENU_18, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+9 に「カスタムメニュー9」, Shift+Alt+9 に「カスタムメニュー19」を追加
			{ '9', "9",0, 0, 0, 0, F_CUSTMENU_9, F_CUSTMENU_19, 0, 0 },
			/* アルファベット */
			{ 'A', "A",0, 0, F_SELECTALL, 0, 0, 0, 0, 0 },
			//Jan. 13, 2001 JEPRO	Ctrl+B に「ブラウズ」を追加
			{ 'B', "B",0, 0, F_BROWSE, 0, 0, 0, 0, 0 },
			//Jan. 16, 2001 JEPRO	SHift+Ctrl+C に「.hと同名の.c(なければ.cpp)を開く」を追加
			//Feb. 07, 2001 JEPRO	SHift+Ctrl+C を「.hと同名の.c(なければ.cpp)を開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
			{ 'C', "C",0, 0, F_COPY, F_OPEN_HfromtoC, 0, 0, 0, 0 },
			//Jan. 16, 2001 JEPRO	Ctrl+D に「単語切り取り」, Shift+Ctrl+D に「単語削除」を追加
			{ 'D', "D",0, 0, F_WordCut, F_WordDelete, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+Alt+E に「重ねて表示」を追加
			//Jan. 16, 2001	JEPRO	Ctrl+E に「行切り取り(折り返し単位)」, Shift+Ctrl+E に「行削除(折り返し単位)」を追加
			{ 'E', "E",0, 0, F_CUT_LINE, F_DELETE_LINE, 0, 0, F_CASCADE, 0 },
			{ 'F', "F",0, 0, F_SEARCH_DIALOG, 0, 0, 0, 0, 0 },
			{ 'G', "G",0, 0, F_GREP, 0, 0, 0, 0, 0 },
			//Oct. 07, 2000 JEPRO	Ctrl+Alt+H に「上下に並べて表示」を追加
			//Jan. 16, 2001 JEPRO	Ctrl+H を「カーソル前を削除」→「カーソル行をウィンドウ中央へ」に変更し	Shift+Ctrl+H に「.cまたは.cppと同名の.hを開く」を追加
			//Feb. 07, 2001 JEPRO	SHift+Ctrl+H を「.cまたは.cppと同名の.hを開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
			{ 'H', "H",0, 0, F_CURLINECENTER, F_OPEN_HfromtoC, 0, 0, F_TILE_V, 0 },
			//Jan. 21, 2001	JEPRO	Ctrl+I に「行の二重化」を追加
			{ 'I', "I",0, 0, F_DUPLICATELINE, 0, 0, 0, 0, 0 },
			{ 'J', "J",0, 0, F_JUMP, 0, 0, 0, 0, 0 },
			//Jan. 16, 2001	JEPRO	Ctrl+K に「行末まで切り取り(改行単位)」, Shift+Ctrl+E に「行末まで削除(改行単位)」を追加
			{ 'K', "K",0, 0, F_LineCutToEnd, F_LineDeleteToEnd, 0, 0, 0, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+Alt+L に「英大文字→英小文字」, Shift+Ctrl+Alt+L に「英小文字→英大文字」を追加
			//Jan. 16, 2001 Ctrl+L を「カーソル行をウィンドウ中央へ」→「キーマクロの読み込み」に変更し「カーソル行をウィンドウ中央へ」は Ctrl+H に移動
			{ 'L', "L",0, 0, F_LOADKEYMACRO, F_EXECKEYMACRO, 0, 0, F_TOLOWER, F_TOUPPER },
			//Jan. 16, 2001 JEPRO	Ctrl+M に「キーマクロの保存」を追加
			{ 'M', "M",0, 0, F_SAVEKEYMACRO, F_RECKEYMACRO, 0, 0, 0, 0 },
			//Oct. 20, 2000 JEPRO	Alt+N に「移動履歴: 次へ」を追加
			{ 'N', "N",0, 0, F_FILENEW, 0, F_JUMPNEXT, 0, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+O に「アウトプット」を追加
			{ 'O', "O",0, 0, F_FILEOPEN, 0, F_WIN_OUTPUT, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+P に「印刷」, Shift+Ctrl+P に「印刷プレビュー」, Ctrl+Alt+P に「ページ設定」を追加
			//Oct. 20, 2000 JEPRO	Alt+P に「移動履歴: 前へ」を追加
			{ 'P', "P",0, 0, F_PRINT, F_PRINT_PREVIEW, F_JUMPPREV, 0, F_PRINT_PAGESETUP, 0 },
			//Jan. 24, 2001	JEPRO	Ctrl+Q に「キー割り当て一覧をコピー」を追加
			{ 'Q', "Q",0, 0, F_CREATEKEYBINDLIST, 0, 0, 0, 0, 0 },
			{ 'R', "R",0, 0, F_REPLACE, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+S に「名前を付けて保存」を追加
			{ 'S', "S",0, 0, F_FILESAVE, F_FILESAVEAS, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+Alt+T に「左右に並べて表示」を追加
			//Jan. 21, 2001	JEPRO	Ctrl+T に「タグジャンプ」, Shift+Ctrl+T に「タグジャンプバック」を追加
			{ 'T', "T",0, 0, F_TAGJUMP, F_TAGJUMPBACK, 0, 0, F_TILE_H, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+Alt+U に「現在のウィンドウ幅で折り返し」を追加
			//Jan. 16, 2001	JEPRO	Ctrl+U に「行頭まで切り取り(改行単位)」, Shift+Ctrl+U に「行頭まで削除(改行単位)」を追加
			{ 'U', "U",0, 0, F_LineCutToStart, F_LineDeleteToStart, 0, 0, F_WRAPWINDOWWIDTH, 0 },
			{ 'V', "V",0, 0, F_PASTE, 0, 0, 0, 0, 0 },
			{ 'W', "W",0, 0, F_SELECTWORD, 0, 0, 0, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+X を「カスタムメニュー1」→「未定義」に変更し「カスタムメニュー1」は Alt+1 に移動
			{ 'X', "X",0, 0, F_CUT, 0, 0, 0, 0, 0 },
			{ 'Y', "Y",0, 0, F_REDO, 0, 0, 0, 0, 0 },
			{ 'Z', "Z",0, 0, F_UNDO, 0, 0, 0, 0, 0 },
			/* 記号 */
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+- に「上下に分割」を追加
			{ 0x00bd, "-",0, 0, 0, F_SPLIT_V, 0, 0, 0, 0 },
			{ 0x00de, "^",0, 0, F_COPYTAG, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ に「左右に分割」を追加
			{ 0x00dc, "\\",0, 0, F_COPYPATH, F_SPLIT_H, 0, 0, 0, 0 },
			//Sept. 20, 2000 JEPRO	Ctrl+@ に「ファイル内容比較」を追加  //Oct. 15, 2000 JEPRO「選択範囲内全行コピー」に変更
			{ 0x00c0, "@",0, 0, F_COPYLINES, 0, 0, 0, 0, 0 },
			//	Aug. 16, 2000 genta
			//	反対向きの括弧にも括弧検索を追加
			{ 0x00db, "[",0, 0, F_BRACKETPAIR, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+; に「縦横に分割」を追加	//Jan. 16, 2001	Alt+; に「日付挿入」を追加
			{ 0x00bb, ";",0, 0, 0, F_SPLIT_VH, F_INS_DATE, 0, 0, 0 },
			//Sept. 14, 2000 JEPRO	Ctrl+: に「選択範囲内全行行番号付きコピー」を追加	//Jan. 16, 2001	Alt+: に「時刻挿入」を追加
			{ 0x00ba, ":",0, 0, F_COPYLINESWITHLINENUMBER, 0, F_INS_TIME, 0, 0, 0 },
			{ 0x00dd, "]",0, 0, F_BRACKETPAIR, 0, 0, 0, 0, 0 },
			{ 0x00bc, ",",0, 0, 0, 0, 0, 0, 0, 0 },
			//Sept. 14, 2000 JEPRO	Ctrl+. に「選択範囲内全行引用符付きコピー」を追加
			{ 0x00be, ".",0, 0, F_COPYLINESASPASSAGE, 0, 0, 0, 0, 0 },
			{ 0x00bf, "/",0, 0, F_HOKAN, 0, 0, 0, 0, 0 },
			//	Nov. 15, 2000 genta PC/ATキーボードに合わせてキーコードを変更
			//	PC98救済のため，従来のキーコードに対応する項目を追加．
			{ 0x00e2, "_",0, 0, F_UNDO, 0, 0, 0, 0, 0 },
			{ 0x00df, "_(PC-98)",0, 0, F_UNDO, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	長くて表示しきれない所がでてきてしまうのでアプリケーションキー→アプリキーに短縮
			{ VK_APPS, "アプリキー",F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON }
		};
		int	nKeyDataInitNum = sizeof( KeyDataInit ) / sizeof( KeyDataInit[0] );
		for( i = 0; i < nKeyDataInitNum; ++i ){
			SetKeyNameArrVal( m_pShareData, i,
				KeyDataInit[i].nKeyCode,
				KeyDataInit[i].pszKeyName,
				KeyDataInit[i].nFuncCode_0,
				KeyDataInit[i].nFuncCode_1,
				KeyDataInit[i].nFuncCode_2,
				KeyDataInit[i].nFuncCode_3,
				KeyDataInit[i].nFuncCode_4,
				KeyDataInit[i].nFuncCode_5,
				KeyDataInit[i].nFuncCode_6,
				KeyDataInit[i].nFuncCode_7
			 );
		}
		m_pShareData->m_nKeyNameArrNum = nKeyDataInitNum;




//	From Here Sept. 19, 2000 JEPRO コメントアウトになっていた初めのブロックを復活しその下をコメントアウト
//	MS ゴシック標準スタイル10ptに設定
//		/* LOGFONTの初期化 */
		memset( &m_pShareData->m_Common.m_lf, 0, sizeof( LOGFONT ) );
		m_pShareData->m_Common.m_lf.lfHeight			= -13;
		m_pShareData->m_Common.m_lf.lfWidth				= 0;
		m_pShareData->m_Common.m_lf.lfEscapement		= 0;
		m_pShareData->m_Common.m_lf.lfOrientation		= 0;
		m_pShareData->m_Common.m_lf.lfWeight			= 400;
		m_pShareData->m_Common.m_lf.lfItalic			= 0x0;
		m_pShareData->m_Common.m_lf.lfUnderline			= 0x0;
		m_pShareData->m_Common.m_lf.lfStrikeOut			= 0x0;
		m_pShareData->m_Common.m_lf.lfCharSet			= 0x80;
		m_pShareData->m_Common.m_lf.lfOutPrecision		= 0x3;
		m_pShareData->m_Common.m_lf.lfClipPrecision		= 0x2;
		m_pShareData->m_Common.m_lf.lfQuality			= 0x1;
		m_pShareData->m_Common.m_lf.lfPitchAndFamily	= 0x31;
		strcpy( m_pShareData->m_Common.m_lf.lfFaceName, "ＭＳ ゴシック" );

//		/* LOGFONTの初期化 */
//		memset( &m_pShareData->m_Common.m_lf, 0, sizeof( LOGFONT ) );
//		m_pShareData->m_Common.m_lf.lfHeight			= -19;
//		m_pShareData->m_Common.m_lf.lfWidth				= 0;
//		m_pShareData->m_Common.m_lf.lfEscapement		= 0;
//		m_pShareData->m_Common.m_lf.lfOrientation		= 0;
//		m_pShareData->m_Common.m_lf.lfWeight			= 400;
//		m_pShareData->m_Common.m_lf.lfItalic			= 0;
//		m_pShareData->m_Common.m_lf.lfUnderline			= 0;
//		m_pShareData->m_Common.m_lf.lfStrikeOut			= 0;
//		m_pShareData->m_Common.m_lf.lfCharSet			= 128;
//		m_pShareData->m_Common.m_lf.lfOutPrecision		= 1;
//		m_pShareData->m_Common.m_lf.lfClipPrecision		= 2;
//		m_pShareData->m_Common.m_lf.lfQuality			= 1;
//		m_pShareData->m_Common.m_lf.lfPitchAndFamily	= 1;
//		strcpy( m_pShareData->m_Common.m_lf.lfFaceName, "FixedSys" );
//	To Here Sept. 19,2000

		m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = TRUE;				/* 現在のフォントは固定幅フォントである */



//		m_pShareData->m_Common.m_lfPrintFont = m_pShareData->m_Common.m_lf;	/* 印刷用フォント */
//		m_pShareData->m_Common.m_lfPrintFont.lfHeight = -400;				/* 0.01mm単位 */

//		/* プリンタ設定 DEVMODE用 */
//		CPrint::GetDefaultPrinterInfo( &(m_pShareData->m_Common.m_mdmDevMode ) );



		/* バックアップ */
		m_pShareData->m_Common.m_bBackUp = FALSE;				/* バックアップの作成 */
		m_pShareData->m_Common.m_bBackUpDialog = TRUE;			/* バックアップの作成前に確認 */
		m_pShareData->m_Common.m_bBackUpFolder = FALSE;			/* 指定フォルダにバックアップを作成する */
		m_pShareData->m_Common.m_szBackUpFolder[0] = '\0';		/* バックアップを作成するフォルダ */
		m_pShareData->m_Common.m_nBackUpType = 2;				/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
		m_pShareData->m_Common.m_nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;
																/* バックアップファイル名：日付 */
		m_pShareData->m_Common.m_nBackUpType_Opt2 = ('b' << 16 ) + 10;
																/* バックアップファイル名：連番の数と先頭文字 */
		m_pShareData->m_Common.m_nBackUpType_Opt3 = 5;			/* バックアップファイル名：Option3 */
		m_pShareData->m_Common.m_nBackUpType_Opt4 = 0;			/* バックアップファイル名：Option4 */
		m_pShareData->m_Common.m_nBackUpType_Opt5 = 0;			/* バックアップファイル名：Option5 */
		m_pShareData->m_Common.m_nBackUpType_Opt6 = 0;			/* バックアップファイル名：Option6 */

		m_pShareData->m_Common.m_nFileShareMode = OF_SHARE_DENY_WRITE;/* ファイルの排他制御モード */

		m_pShareData->m_Common.m_nCaretType = 0;				/* カーソルのタイプ 0=win 1=dos */
		m_pShareData->m_Common.m_bIsINSMode = TRUE;				/* 挿入／上書きモード */
		m_pShareData->m_Common.m_bIsFreeCursorMode = FALSE;		/* フリーカーソルモードか */	//Oct. 29, 2000 JEPRO 「なし」に変更
		m_pShareData->m_Common.m_bAutoIndent = TRUE;			/* オートインデント */
		m_pShareData->m_Common.m_bAutoIndent_ZENSPACE = TRUE;	/* 日本語空白もインデント */

		//	Oct. 27, 2000 genta
		m_pShareData->m_Common.m_bRestoreCurPosition = TRUE;	//	カーソル位置復元

//		m_pShareData->m_Common.m_bEnableLineISlog = TRUE;		/* ★廃止★行番号種別  物理行／論理行 */

		m_pShareData->m_Common.m_bRegularExp = 0;				/* 1==正規表現 */
		m_pShareData->m_Common.m_bLoHiCase = 0;					/* 1==英大文字小文字の区別 */
		m_pShareData->m_Common.m_bWordOnly = 0;					/* 1==単語のみ検索 */
		m_pShareData->m_Common.m_bSelectedArea = FALSE;			/* 選択範囲内置換 */
		m_pShareData->m_Common.m_szExtHelp1[0] = '\0';			/* 外部ヘルプ１ */
		m_pShareData->m_Common.m_szExtHtmlHelp[0] = '\0';		/* 外部HTMLヘルプ */

		m_pShareData->m_Common.m_bNOTIFYNOTFOUND = TRUE;		/* 検索／置換  見つからないときメッセージを表示 */

		m_pShareData->m_Common.m_bExitConfirm = FALSE;			/* 終了時の確認をする */
		m_pShareData->m_Common.m_nRepeatedScrollLineNum = 3;	/* キーリピート時のスクロール行数 */
		m_pShareData->m_Common.m_nRepeatedScroll_Smooth = FALSE;/* キーリピート時のスクロールを滑らかにするか */

//		strcpy( m_pShareData->m_Common.m_szEMailUserName, "");		/* メールユーザー名 */
//		strcpy( m_pShareData->m_Common.m_szEMailUserAddress, "");	/* メールアドレス */
//		strcpy( m_pShareData->m_Common.m_szSMTPServer, "");			/* SMTPホスト名・アドレス */
//		m_pShareData->m_Common.m_nSMTPPort = 25;					/* SMTPポート番号(通常は25) */

		m_pShareData->m_Common.m_bAddCRLFWhenCopy = FALSE;		/* 折り返し行に改行を付けてコピー */
		m_pShareData->m_Common.m_bGrepSubFolder = TRUE;			/* Grep: サブフォルダも検索 */
		m_pShareData->m_Common.m_bGrepOutputLine = TRUE;		/* Grep: 行を出力するか該当部分だけ出力するか */
		m_pShareData->m_Common.m_nGrepOutputStyle = 1;			/* Grep: 出力形式 */

		m_pShareData->m_Common.m_bGTJW_RETURN = TRUE;			/* エンターキーでタグジャンプ */
		m_pShareData->m_Common.m_bGTJW_LDBLCLK = TRUE;			/* ダブルクリックでタグジャンプ */

//キーワード：ツールバー順序
		/* ツールバーボタン構造体 */
//Sept. 16, 2000 JEPRO
//	CShareData_new2.cppでできるだけ系ごとに集まるようにアイコンの順番を大幅に入れ替えたのに伴い以下の初期設定値を変更
		i = -1;
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 1;		//新規作成
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 2;		//ファイルを開く
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 3;		//上書き保存		//Sept. 16, 2000 JEPRO 3→11に変更	//Oct. 25, 2000 11→3
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 4;		//名前を付けて保存	//Sept. 19, 2000 JEPRO 追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 161;	//切り取り	//Sept. 16, 2000 JEPRO 4→16に変更	//Oct. 25, 2000 16→161	//Feb. 24, 2001 外した
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 162;	//コピー	//Sept. 16, 2000 JEPRO 5→17に変更	//Oct. 25, 2000 171→62	//Feb. 24, 2001 外した
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 164;	//貼り付け	//Sept. 16, 2000 JEPRO 6→18に変更	//Oct. 25, 2000 18→164	//Feb. 24, 2001 外した
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;		//Feb. 24, JEPRO 2001 外した
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 33;	//元に戻す(Undo)	//Sept. 16, 2000 JEPRO 7→19に変更	//Oct. 25, 2000 19→33
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 34;	//やり直し(Redo)	//Sept. 16, 2000 JEPRO 8→20に変更	//Oct. 25, 2000 20→34
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 87;	//移動履歴: 前へ	//Dec. 24, 2000 JEPRO 追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 88;	//移動履歴: 次へ	//Dec. 24, 2000 JEPRO 追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 225;	//検索		//Sept. 16, 2000 JEPRO 9→22に変更	//Oct. 25, 2000 22→225
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 226;	//次を検索	//Sept. 16, 2000 JEPRO 16→23に変更	//Oct. 25, 2000 23→226
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 227;	//前を検索	//Sept. 16, 2000 JEPRO 17→24に変更	//Oct. 25, 2000 24→227
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 228;	//置換		// Oct. 7, 2000 JEPRO 追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 229;	//検索マークのクリア	//Sept. 16, 2000 JEPRO 41→25に変更(Oct. 7, 2000 25→26)	//Oct. 25, 2000 25→229
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 230;	//Grep		//Sept. 16, 2000 JEPRO 14→31に変更	//Oct. 25, 2000 31→230
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 232;	//アウトライン解析	//Dec. 24, 2000 JEPRO 追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 264;	//タイプ別設定一覧	//Sept. 16, 2000 JEPRO 追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 265;	//タイプ別設定		//Sept. 16, 2000 JEPRO 18→36に変更	//Oct. 25, 2000 36→265
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 266;	//共通設定			//Sept. 16, 2000 JEPRO 10→37に変更 説明を「設定プロパティシート」から変更	//Oct. 25, 2000 37→266
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 267;	//フォント設定		//Sept. 16, 2000 JEPRO 11→38に変更	//Oct. 25, 2000 38→267	//Dec. 24, 2000 外した
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 268;	//現在のウィンドウ幅で折り返し	//Sept. 16, 2000 JEPRO 42→39に変更	//Oct. 25, 2000 39→268	//Feb. 24, 2001 外した
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 12;	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 13;	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更

//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 277;	//キーマクロの記録開始／終了	//Sept. 16, 2000 JEPRO 55→68に変更	//Oct. 25, 2000 68→277	//Dec. 24, 2000 外した
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 280;	//キーマクロの実行	//Sept. 16, 2000 JEPRO 58→71に変更	//Oct. 25, 2000 71→280	//Dec. 24, 2000 外した
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;		//Oct. 8, 2000 jepro 次行のために追加
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 346;	//コマンド一覧	//Oct. 8, 2000 JEPRO 追加


		m_pShareData->m_Common.m_nToolBarButtonNum = ++i;		/* ツールバーボタンの数 */
		m_pShareData->m_Common.m_bToolBarIsFlat = TRUE;			/* フラットツールバーにする／しない */


		m_pShareData->m_Common.m_bDispTOOLBAR = TRUE;			/* 次回ウィンドウを開いたときツールバーを表示する */
		m_pShareData->m_Common.m_bDispSTATUSBAR = TRUE;			/* 次回ウィンドウを開いたときステータスバーを表示する */
		m_pShareData->m_Common.m_bDispFUNCKEYWND = FALSE;		/* 次回ウィンドウを開いたときファンクションキーを表示する */
		m_pShareData->m_Common.m_nFUNCKEYWND_Place = 1;			/* ファンクションキー表示位置／0:上 1:下 */
		m_pShareData->m_Common.m_bSplitterWndHScroll = TRUE;	// 2001/06/20 asa-o 分割ウィンドウの水平スクロールの同期をとる
		m_pShareData->m_Common.m_bSplitterWndVScroll = TRUE;	// 2001/06/20 asa-o 分割ウィンドウの垂直スクロールの同期をとる

		/* カスタムメニュー情報 */
		wsprintf( m_pShareData->m_Common.m_szCustMenuNameArr[0], "右クリックメニュー", i );
		for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( m_pShareData->m_Common.m_szCustMenuNameArr[i], "メニュー%d", i );
			m_pShareData->m_Common.m_nCustMenuItemNumArr[i] = 0;
			for( j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
				m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j] = 0;
				m_pShareData->m_Common.m_nCustMenuItemKeyArr [i][j] = '\0';
			}
		}

		/* カスタムメニュー 規定値 */

		/* 右クリックメニュー */
		int n;
		n = 0;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_UNDO;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'U';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_REDO;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'R';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_CUT;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'T';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPY;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'C';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_PASTE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'P';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_DELETE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'D';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	//Nov. 9, 2000 JEPRO 「CRLF改行でコピー」を追加
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'L';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	//Nov. 9, 2000 JEPRO 「矩形貼り付け」を復活
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'X';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_SELECTALL;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'A';
		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOLOWER;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'L';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOUPPER;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'U';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOHANKAKU;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOZENKAKUHIRA;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOZENKAKUKATA;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_OPTION_TYPE;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '1';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_OPTION;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '2';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_FONT;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '3';
//		n++;

		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;		//Oct. 3, 2000 JEPRO 以下に「タグジャンプ」と「タグジャンプバック」を追加
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'G';		//Nov. 9, 2000 JEPRO 「コピー」とバッティングしていたアクセスキーを変更(T→G)
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'B';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;		//Oct. 15, 2000 JEPRO 以下に「選択範囲内全行コピー」と「引用符付きコピー」を追加
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPYLINES;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '@';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '.';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPYPATH;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\\';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'F';		//Nov. 9, 2000 JEPRO 「やり直し」とバッティングしていたアクセスキーを変更(R→F)
		n++;
		m_pShareData->m_Common.m_nCustMenuItemNumArr[0] = n;

		/* カスタムメニュー１ */
		m_pShareData->m_Common.m_nCustMenuItemNumArr[1] = 7;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][0] = 'O';		//Sept. 14, 2000 JEPRO できるだけ標準設定値に合わせるように変更 (F→O)
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][1] = F_FILESAVE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][1] = 'S';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][2] = 'N';		//Sept. 14, 2000 JEPRO できるだけ標準設定値に合わせるように変更 (O→N)
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][3] = F_TOLOWER;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][3] = 'L';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][4] = F_TOUPPER;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][4] = 'U';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][6] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][6] = '\0';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][6] = 'C';
		/* 見出し記号 */
		strcpy( m_pShareData->m_Common.m_szMidashiKigou, "１２３４５６７８９０（(［[「『【■□▲△▼▽◆◇○◎●§・※☆★第①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ一二三四五六七八九十壱弐参伍" );
		/* 引用符 */
		strcpy( m_pShareData->m_Common.m_szInyouKigou, "> " );		/* 引用符 */
		m_pShareData->m_Common.m_bUseHokan = FALSE;					/* 入力補完機能を使用する */

		// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除
//		strcpy( m_pShareData->m_Common.m_szHokanFile, "" );			/* 入力補完 単語ファイル */
//		m_pShareData->m_Common.m_bUseKeyWordHelp = FALSE;			/* キーワードヘルプを使用する */
//		strcpy( m_pShareData->m_Common.m_szKeyWordHelpFile, "" );	/* 辞書ファイル */
		m_pShareData->m_Common.m_bGrepKanjiCode_AutoDetect = FALSE;	/* Grep: 文字コード自動判別 */
		// 2001/06/19 asa-o タイプ別に移動したので削除
//		m_pShareData->m_Common.m_bHokanLoHiCase = FALSE;			/* 入力補完機能：英大文字小文字を同一視する */
		m_pShareData->m_Common.m_bSaveWindowSize = TRUE;			/* ウィンドウサイズ継承 */
		m_pShareData->m_Common.m_nWinSizeType = SIZE_RESTORED;
		m_pShareData->m_Common.m_nWinSizeCX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_nWinSizeCY = 0;
		m_pShareData->m_Common.m_bUseTaskTray = TRUE;				/* タスクトレイのアイコンを使う */
		m_pShareData->m_Common.m_bStayTaskTray = TRUE;				/* タスクトレイのアイコンを常駐 */
		m_pShareData->m_Common.m_wTrayMenuHotKeyCode = 'Z';			/* タスクトレイ左クリックメニュー キー */
		m_pShareData->m_Common.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* タスクトレイ左クリックメニュー キー */
		m_pShareData->m_Common.m_bUseOLE_DragDrop = TRUE;			/* OLEによるドラッグ & ドロップを使う */
		m_pShareData->m_Common.m_bUseOLE_DropSource = TRUE;			/* OLEによるドラッグ元にするか */
		m_pShareData->m_Common.m_bDispExitingDialog = FALSE;		/* 終了ダイアログを表示する */
		m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite = FALSE;/* 無変更でも上書きするか */
		m_pShareData->m_Common.m_bSelectClickedURL = TRUE;			/* URLがクリックされたら選択するか */
		m_pShareData->m_Common.m_bGrepExitConfirm = FALSE;			/* Grepモードで保存確認するか */
//		m_pShareData->m_Common.m_bRulerDisp = TRUE;					/* ルーラー表示 */
		m_pShareData->m_Common.m_nRulerHeight = 13;					/* ルーラーの高さ */
		m_pShareData->m_Common.m_nRulerBottomSpace = 0;				/* ルーラーとテキストの隙間 */
		m_pShareData->m_Common.m_nRulerType = 0;					/* ルーラーのタイプ */
		m_pShareData->m_Common.m_bCopyAndDisablSelection = FALSE;	/* コピーしたら選択解除 */
		m_pShareData->m_Common.m_bHtmlHelpIsSingle = TRUE;			/* HtmlHelpビューアはひとつ */
		m_pShareData->m_Common.m_bCompareAndTileHorz = TRUE;		/* 文書比較後、左右に並べて表示 */
//		m_pShareData->m_Common.m_bCompareAndTileHorz = TRUE;		/* 文書比較後、左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
		/* 1999.11.15 */
		m_pShareData->m_Common.m_bDropFileAndClose = FALSE;			/* ファイルをドロップしたときは閉じて開く */
		m_pShareData->m_Common.m_nDropFileNumMax = 8;				/* 一度にドロップ可能なファイル数 */
		m_pShareData->m_Common.m_bCheckFileTimeStamp = TRUE;		/* 更新の監視 */
		m_pShareData->m_Common.m_bNotOverWriteCRLF = TRUE;			/* 改行は上書きしない */
		::SetRect( &m_pShareData->m_Common.m_rcOpenDialog, 0, 0, 0, 0 );	/* 「開く」ダイアログのサイズと位置 */
		m_pShareData->m_Common.m_bAutoCloseDlgFind = TRUE;			/* 検索ダイアログを自動的に閉じる */
		m_pShareData->m_Common.m_bScrollBarHorz = TRUE;				/* 水平スクロールバーを使う */
		m_pShareData->m_Common.m_bAutoCloseDlgFuncList = FALSE;		/* アウトライン ダイアログを自動的に閉じる */	//Nov. 18, 2000 JEPRO TRUE→FALSE に変更
		m_pShareData->m_Common.m_bAutoCloseDlgReplace = TRUE;		/* 置換 ダイアログを自動的に閉じる */
		m_pShareData->m_Common.m_bAutoColmnPaste = TRUE;			/* 矩形コピーのテキストは常に矩形貼り付け */

		m_pShareData->m_Common.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN 補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_bHokanKey_TAB		= FALSE;		/* VK_TAB   補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_bHokanKey_RIGHT	= TRUE;			/* VK_RIGHT 補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_bHokanKey_SPACE	= FALSE;		/* VK_SPACE 補完決定キーが有効/無効 */


/***********
書式指定子 意味
d 年月日の日。先頭に 0 は付きません。
dd 年月日の日。1 桁の場合、先頭に 0 が付きます。
ddd 曜日。ロケールの LOCALE_SABBREVDAYNAME が使われます。
dddd 曜日。ロケールの LOCALE_SDAYNAME が使われます。
M 年月日の月。先頭に 0 は付きません。
MM 年月日の月。1 桁の場合、先頭に 0 が付きます。
MMM 年月日の月。ロケールの LOCALE_SABBREVMONTHNAME が使われます。
MMMM 年月日の月。ロケールの LOCALE_SMONTHNAME が使われます。
y 年月日の年。先頭に 0 は付きません。
yy 年月日の年。1 桁の場合、先頭に 0 が付きます。
yyyy 年月日の年。4 桁で表されます。
gg 「平成」などの時代を示す文字列。ロケールの CAL_SERASTRING が使われます。日付がこの情報を持たないときは、無視されます。
************/

/***********
書式指定子 意味
h 12 時間制の時間。先頭に 0 は付きません。
hh 12 時間制の時間。必要に応じて、先頭に 0 が付きます。
H 24 時間制の時間。先頭に 0 は付きません。
HH 24 時間制の時間。必要に応じて、先頭に 0 が付きます。
m 分。先頭に 0 は付きません。
mm 分。必要に応じて、先頭に 0 が付きます。
s 秒。先頭に 0 は付きません。
ss 秒。必要に応じて、先頭に 0 が付きます。
t 時刻マーカー。「 A 」「 P 」など。
tt 時刻マーカー。「 AM 」「 PM 」「午前」「午後」など。
************/

		m_pShareData->m_Common.m_nDateFormatType = 0;	//日付書式のタイプ
		strcpy( m_pShareData->m_Common.m_szDateFormat, "yyyy\'年\'M\'月\'d\'日(\'dddd\')\'" );	//日付書式
		m_pShareData->m_Common.m_nTimeFormatType = 0;	//時刻書式のタイプ
		strcpy( m_pShareData->m_Common.m_szTimeFormat, "tthh\'時\'mm\'分\'ss\'秒\'"  );			//時刻書式

		m_pShareData->m_Common.m_bMenuIcon = TRUE;		/* メニューにアイコンを表示する */

		//	Nov. 12, 2000 genta
		m_pShareData->m_Common.m_bAutoMIMEdecode = TRUE;	//ファイル読み込み時にMIMEのデコードを行うか	//Jul. 13, 2001 JEPRO

//		/* MRU 外部コマンド */
//		m_pShareData->m_Common.m_MRU_ExtCmd.m_nExtCmdArrNum = 0;
//		for( i = 0; i < MAX_EXTCMDMRUNUM; i++ ){
//			/* 初期化 */
//			m_pShareData->m_Common.m_MRU_ExtCmd.m_ExtCmdArr[i].Init();
//		}
		for( i = 0; i < MAX_CMDARR; i++ ){
			/* 初期化 */
			m_pShareData->m_szCmdArr[i][0] = '\0';
		}
		m_pShareData->m_nCmdArrNum = 0;

//キーワード：デフォルトカラー設定
/************************/
/* タイプ別設定の規定値 */
/************************/
		int nIdx = 0;
		m_pShareData->m_Types[nIdx].m_nMaxLineSize = 10240;				/* 折り返し文字数 */
		m_pShareData->m_Types[nIdx].m_nColmSpace = 0;					/* 文字と文字の隙間 */
		m_pShareData->m_Types[nIdx].m_nLineSpace = 1;					/* 行間のすきま */
		m_pShareData->m_Types[nIdx].m_nTabSpace = 4;					/* TABの文字数 */
		m_pShareData->m_Types[nIdx].m_nKeyWordSetIdx = -1;				/* キーワードセット */
		m_pShareData->m_Types[nIdx].m_nKeyWordSetIdx2 = -1;				/* キーワードセット2 */	//Dec. 4, 2000, MIK
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		strcpy( m_pShareData->m_Types[nIdx].m_szTabViewString, "^       " );	/* TAB表示文字列 */
//#endif
		strcpy( m_pShareData->m_Types[nIdx].m_szLineComment,  "" );		/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[nIdx].m_szLineComment2, "" );		/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[nIdx].m_szLineComment3, "" );		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentFrom, "" );	/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentTo, "" );	/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentFrom2, "" );/* ブロックコメントデリミタ2(From) */
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentTo2, "" );	/* ブロックコメントデリミタ2(To) */
//#endif
		m_pShareData->m_Types[nIdx].m_nLineCommentPos = -1;				/* 行コメント1 桁位置 */
		m_pShareData->m_Types[nIdx].m_nLineCommentPos2 = -1;			/* 行コメント2 桁位置 */
		m_pShareData->m_Types[nIdx].m_nLineCommentPos3 = -1;			/* 行コメント3 桁位置 */	//Jun. 01, 2001 JEPRO 追加
		m_pShareData->m_Types[nIdx].m_nStringType = 0;					/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
		strcpy( m_pShareData->m_Types[nIdx].m_szIndentChars, "" );		/* その他のインデント対象文字 */

		m_pShareData->m_Types[nIdx].m_nColorInfoArrNum = COLORIDX_LAST;

		// 2001/06/14 Start by asa-o
		strcpy( m_pShareData->m_Types[nIdx].m_szHokanFile, "" );		/* 入力補完 単語ファイル */
		m_pShareData->m_Types[nIdx].m_bUseKeyWordHelp = FALSE;			/* キーワードヘルプを使用する */
		strcpy( m_pShareData->m_Types[nIdx].m_szKeyWordHelpFile, "" );	/* 辞書ファイル */
		// 2001/06/14 End

		// 2001/06/19 asa-o
		m_pShareData->m_Types[nIdx].m_bHokanLoHiCase = FALSE;			/* 入力補完機能：英大文字小文字を同一視する */

/**
		static const char* ppszTypeName[] = {
			"テキスト",							// CI[00]
			"行番号",							// CI[01]
			"改行記号",							// CI[02]
			"TAB記号",							// CI[03]
			"日本語空白",						// CI[04]
			"EOF記号",							// CI[05]
			"強調キーワード",					// CI[06]
			"コメント",							// CI[07]
			"シングルクォーテーション文字列",	// CI[08]
			"ダブルクォーテーション文字列",		// CI[09]
			"カーソル行アンダーライン",			// CI[10]
			"折り返し記号",						// CI[11]
			"コントロールコード",				// CI[12]
			"URL",								// CI[13]
			"検索文字列",						// CI[14]
			"行番号(変更行)",					// CI[15]
			"ルーラー"							// CI[16]
		};
**/

/*	From Here Sept. 18, 2000 JEPRO 順番を大幅に入れ替えた
		static ColorInfoIni ColorInfo_DEFAULT[] = {
			"テキスト",				TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 255 ),
			"行番号",				TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"改行記号",				TRUE , FALSE, FALSE, RGB( 0, 128, 255 )		, RGB( 255, 255, 255 ),
			"TAB記号",				TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 255, 255 ),
			"日本語空白",			TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 255, 255 ),
			"EOF記号",				TRUE , FALSE, FALSE, RGB( 0, 255, 255 )		, RGB( 0, 0, 0 ) },
			"強調キーワード",		TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 255, 255 ),
			"コメント",				TRUE , FALSE, FALSE, RGB( 0, 128, 0 )		, RGB( 255, 255, 255 ),
		//	From Here Sept. 4, 2000 JEPRO
		//	シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
		//	"シングルクォーテーション文字列", FALSE, FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 255, 255 ),
			"シングルクォーテーション文字列", FALSE, FALSE, FALSE, RGB( 64, 128, 128 )	, RGB( 255, 255, 255 ),
		//	To Here Sept. 4, 2000
			"ダブルクォーテーション文字列", TRUE, FALSE, FALSE, RGB( 128, 0, 64 ),RGB( 255, 255, 255 ),
			"カーソル行アンダーライン",  TRUE, FALSE, FALSE, RGB( 0, 0, 255 )	, RGB( 255, 255, 255 ),
			"折り返し記号",			TRUE , FALSE, FALSE, RGB( 255, 0, 255 )		, RGB( 255, 255, 255 ),
			"コントロールコード",	TRUE , FALSE, FALSE, RGB( 255, 255, 0 )		, RGB( 255, 255, 255 ),
			"URL",					TRUE , FALSE, TRUE , RGB( 0, 0, 255 )		, RGB( 255, 255, 255 ),
			"検索文字列",			TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 0 ),
			"行番号(変更行)",		TRUE , TRUE , FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"ルーラー",				TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 239, 239, 239 )
		};
*/


/**
		static const char* ppszTypeName[] = {	//改修前のColor Index
//Dec.26, 2000 jepro 補追 UR1.2.24から強調キーワードが1つ増えたが以下はそれ以前のもの
			"テキスト",							// CI[00]
			"ルーラー",							// CI[16]
			"カーソル行アンダーライン",			// CI[10]
			"行番号",							// CI[01]
			"行番号(変更行)",					// CI[15]
			"TAB記号",							// CI[03]
			"日本語空白",						// CI[04]
			"コントロールコード",				// CI[12]
			"改行記号",							// CI[02]
			"折り返し記号",						// CI[11]
			"EOF記号",							// CI[05]
			"検索文字列",						// CI[14]
			"強調キーワード",					// CI[06]
			"コメント",							// CI[07]
			"シングルクォーテーション文字列",	// CI[08]
			"ダブルクォーテーション文字列",		// CI[09]
			"URL"								// CI[13]
		};
**/

		static ColorInfoIni ColorInfo_DEFAULT[] = {
		//	Nov. 9, 2000 Jepro note: color setting (詳細は CshareData.h を参照のこと)
		//	0,							1(Disp),	 2(FatFont),3(UnderLIne) , 4(colTEXT),	5(colBACK),
		//	szName(項目名),				色分け／表示, 太字,		下線,		文字色,		背景色,
		//
		//Oct. 8, 2000 JEPRO 背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
			"テキスト",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 251, 240 ),
			"ルーラー",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 239, 239, 239 ),
			"カーソル行アンダーライン",			TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
			"行番号",							TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"行番号(変更行)",					TRUE , TRUE , FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"TAB記号",							TRUE , FALSE, FALSE, RGB( 128, 128, 128 )	, RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)より濃いグレーに変更
			"日本語空白",						TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ),
			"コントロールコード",				TRUE , FALSE, FALSE, RGB( 255, 255, 0 )		, RGB( 255, 251, 240 ),
			"改行記号",							TRUE , FALSE, FALSE, RGB( 0, 128, 255 )		, RGB( 255, 251, 240 ),
			"折り返し記号",						TRUE , FALSE, FALSE, RGB( 255, 0, 255 )		, RGB( 255, 251, 240 ),
			"EOF記号",							TRUE , FALSE, FALSE, RGB( 0, 255, 255 )		, RGB( 0, 0, 0 ),
//#ifdef COMPILE_COLOR_DIGIT
			"半角数値",							FALSE, FALSE, FALSE, RGB( 235, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)を変更  Mar.10, 2001 標準は色なしに
//#endif
			"検索文字列",						TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 0 ),
			"強調キーワード1",					TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
			"強調キーワード2",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
			"コメント",							TRUE , FALSE, FALSE, RGB( 0, 128, 0 )		, RGB( 255, 251, 240 ),
		//Sept. 4, 2000 JEPRO シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
		//Oct. 17, 2000 JEPRO 色分け表示するように変更(最初のFALSE→TRUE)
		//"シングルクォーテーション文字列", FALSE, FALSE, FALSE, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
			"シングルクォーテーション文字列",	TRUE , FALSE, FALSE, RGB( 64, 128, 128 )	, RGB( 255, 251, 240 ),
			"ダブルクォーテーション文字列",		TRUE , FALSE, FALSE, RGB( 128, 0, 64 )		, RGB( 255, 251, 240 ),
			"URL",								TRUE , FALSE, TRUE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
  		};
//	To Here Sept. 18, 2000


		for( i = 0; i < COLORIDX_LAST; ++i ){
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_nColorIdx		= i;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bDisp			= ColorInfo_DEFAULT[i].m_bDisp;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bFatFont		= ColorInfo_DEFAULT[i].m_bFatFont;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bUnderLine		= ColorInfo_DEFAULT[i].m_bUnderLine;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colTEXT			= ColorInfo_DEFAULT[i].m_colTEXT;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colBACK			= ColorInfo_DEFAULT[i].m_colBACK;
//			strcpy( m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_szName, ppszTypeName[i] );
			strcpy( m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_szName, ColorInfo_DEFAULT[i].m_pszName );
		}
		m_pShareData->m_Types[nIdx].m_bLineNumIsCRLF = TRUE;				/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		m_pShareData->m_Types[nIdx].m_nLineTermType = 1;					/* 行番号区切り 0=なし 1=縦線 2=任意 */
		m_pShareData->m_Types[nIdx].m_cLineTermChar = ':';					/* 行番号区切り文字 */
		m_pShareData->m_Types[nIdx].m_bWordWrap = FALSE;					/* 英文ワードラップをする */
		m_pShareData->m_Types[nIdx].m_nCurrentPrintSetting = 0;				/* 現在選択している印刷設定 */
		m_pShareData->m_Types[nIdx].m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
		m_pShareData->m_Types[nIdx].m_nSmartIndent = SMARTINDENT_NONE;		/* スマートインデント種別 */
		m_pShareData->m_Types[nIdx].m_nImeState = IME_CMODE_NOCONVERSION;	/* IME入力 */


		static char* pszTypeNameArr[] = {
			"基本",
			"テキスト",
			"C/C++",
			"HTML",
			"PL/SQL",
			"COBOL",
			"Java",
			"アセンブラ",
			"AWK",
			"MS-DOSバッチファイル",
			"Pascal",
			"TeX",				//Oct. 31, 2000 JEPRO TeX  ユーザに贈る
			"Perl",				//Jul. 08, 2001 JEPRO Perl ユーザに贈る
			"Visual Basic",		//JUl. 10, 2001 JEPRO VB   ユーザに贈る
			"リッチテキスト",	//JUl. 10, 2001 JEPRO WinHelp作るのにいるケンね
			"設定ファイル",		//Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
			"設定17",			//From Here Jul. 12, 2001 JEPRO タイプ別設定の設定数を16→20に増やした
			"設定18",
			"設定19",
			"設定20"			//To Here Jul. 12, 2001
		};
		static char* pszTypeExts[] = {
			"",
			"txt,doc,1st,err,ps",	//Nov. 15, 2000 JEPRO PostScriptファイルも読めるようにする //Jan. 12, 2001 JEPRO readme.1st も読めるようにする //Feb. 12, 2001 JEPRO .err エラーメッセージ
//			"c,cpp,cxx,h",
			"c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,dsw,dsp,dep,mak,hm",	//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読めるようにする
				//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++を追加	//Mar. 15, 2001 JEPRO .hmを追加
//			"html,htm,shtml",
			"html,htm,shtml,plg",	//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読み込めるようにする
			"sql,plsql",
			"cbl,cpy,pco,cob",	//Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加
			"java,jav",
			"asm",
			"awk",
			"bat",
			"dpr,pas",
			"tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo",		//Oct. 31, 2000 JEPRO TeX ユーザに贈る	//Mar. 10, 2001 JEPRO 追加
			"cgi,pl,pm",			//Jul. 08, 2001 JEPRO 追加
			"bas,frm,cls,ctl,pag",	//Jul. 09, 2001 JEPRO 追加
			"rtf",					//Jul. 10, 2001 JEPRO 追加
			"ini,inf,cnf,kwd,col",	//Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
			"",						//From Here Jul. 12, 2001 JEPRO タイプ別設定の設定数を16→20に増やした
			"",
			"",
			""						//To Here Jul. 12, 2001
		};

		m_pShareData->m_Types[0].m_nIdx = 0;
		strcpy( m_pShareData->m_Types[0].m_szTypeName, pszTypeNameArr[0] );				/* タイプ属性：名称 */
		strcpy( m_pShareData->m_Types[0].m_szTypeExts, pszTypeExts[0] );				/* タイプ属性：拡張子リスト */
		for( nIdx = 1; nIdx < MAX_TYPES; ++nIdx ){
			m_pShareData->m_Types[nIdx] = m_pShareData->m_Types[0];
			m_pShareData->m_Types[nIdx].m_nIdx = nIdx;
			strcpy( m_pShareData->m_Types[nIdx].m_szTypeName, pszTypeNameArr[nIdx] );	/* タイプ属性：名称 */
			strcpy( m_pShareData->m_Types[nIdx].m_szTypeExts, pszTypeExts[nIdx] );		/* タイプ属性：拡張子リスト */
			m_pShareData->m_Types[nIdx].m_nKeyWordSetIdx2 = -1;	//Dec. 4, 2000 MIK
		}


		/* 基本 */
		strcpy( m_pShareData->m_Types[0].m_szLineComment, "" );			/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[0].m_szLineComment2, "" );		/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[0].m_szBlockCommentFrom, "" );	/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[0].m_szBlockCommentTo, "" );		/* ブロックコメントデリミタ(To) */
		m_pShareData->m_Types[0].m_nMaxLineSize = 10240;				/* 折り返し文字数 */
//		m_pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_UNKNOWN;	/* アウトライン解析方法 */	//Jul. 08, 2001 JEPRO 使わないように変更
		m_pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
		//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;

		nIdx = 0;
		/* テキスト */
		strcpy( m_pShareData->m_Types[1].m_szLineComment, "" );			/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[1].m_szLineComment2, "" );		/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[1].m_szBlockCommentFrom, "" );	/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[1].m_szBlockCommentTo, "" );		/* ブロックコメントデリミタ(To) */
		//From Here Sept. 20, 2000 JEPRO テキストの規定値を80→120に変更(不具合一覧.txtがある程度読みやすい桁数)
//		m_pShareData->m_Types[1].m_nMaxLineSize = 80;					/* 折り返し文字数 */
		m_pShareData->m_Types[1].m_nMaxLineSize = 120;					/* 折り返し文字数 */
		//To Here Sept. 20, 2000
		m_pShareData->m_Types[1].m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
		//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;


		// nIdx = 1;
		/* C/C++ */
		strcpy( m_pShareData->m_Types[2].m_szLineComment, "//" );			/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[2].m_szLineComment2, "" );			/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentFrom, "/*" );		/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentTo, "*/" );		/* ブロックコメントデリミタ(To) */
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentFrom2, "#if 0" );	/* ブロックコメントデリミタ2(From) */	//Jul. 11, 2001 JEPRO
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentTo2, "#endif" );	/* ブロックコメントデリミタ2(To) */
		m_pShareData->m_Types[2].m_nKeyWordSetIdx = 0;						/* キーワードセット */
		m_pShareData->m_Types[2].m_nDefaultOutline = OUTLINE_CPP;			/* アウトライン解析方法 */
		m_pShareData->m_Types[2].m_nSmartIndent = SMARTINDENT_CPP;			/* スマートインデント種別 */
		//Mar. 10, 2001 JEPRO	半角数値を色分け表示
		m_pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;


		/* HTML */
		strcpy( m_pShareData->m_Types[3].m_szLineComment, "" );				/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[3].m_szLineComment2, "" );			/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[3].m_szBlockCommentFrom, "<!--" );	/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[3].m_szBlockCommentTo, "-->" );		/* ブロックコメントデリミタ(To) */
		m_pShareData->m_Types[3].m_nStringType = 0;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		m_pShareData->m_Types[3].m_nKeyWordSetIdx = 1;						/* キーワードセット */

		// nIdx = 3;
		/* PL/SQL */
		strcpy( m_pShareData->m_Types[4].m_szLineComment, "--" );			/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[4].m_szLineComment2, "" );			/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[4].m_szBlockCommentFrom, "/*" );		/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[4].m_szBlockCommentTo, "*/" );		/* ブロックコメントデリミタ(To) */
		m_pShareData->m_Types[4].m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		strcpy( m_pShareData->m_Types[4].m_szIndentChars, "|★" );			/* その他のインデント対象文字 */
		m_pShareData->m_Types[4].m_nKeyWordSetIdx = 2;						/* キーワードセット */
		m_pShareData->m_Types[4].m_nDefaultOutline = OUTLINE_PLSQL;			/* アウトライン解析方法 */

		/* COBOL */
//		strcpy( m_pShareData->m_Types[5].m_szLineComment, "      *" );		/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[5].m_szLineComment, "*" );			/* 行コメントデリミタ */	//Jun. 02, 2001 JEPRO 修正
		strcpy( m_pShareData->m_Types[5].m_szLineComment2, "D" );			/* 行コメントデリミタ2 */	//Jun. 04, 2001 JEPRO 追加
		strcpy( m_pShareData->m_Types[5].m_szBlockCommentFrom, "" );		/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[5].m_szBlockCommentTo, "" );			/* ブロックコメントデリミタ(To) */
		m_pShareData->m_Types[5].m_nLineCommentPos = 6;						/* 行コメント1 桁位置 */	//Jun. 02, 2001 JEPRO 追加
		m_pShareData->m_Types[5].m_nLineCommentPos2 = 6;					/* 行コメント2 桁位置 */	//Jun. 04, 2001 JEPRO 追加
		m_pShareData->m_Types[5].m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		strcpy( m_pShareData->m_Types[5].m_szIndentChars, "*" );			/* その他のインデント対象文字 */
		m_pShareData->m_Types[5].m_nKeyWordSetIdx = 3;						/* キーワードセット */		//Jul. 10, 2001 JEPRO
		m_pShareData->m_Types[5].m_nDefaultOutline = OUTLINE_COBOL;			/* アウトライン解析方法 */


		/* Java */
		strcpy( m_pShareData->m_Types[6].m_szLineComment, "//" );			/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[6].m_szLineComment2, "" );			/* 行コメントデリミタ2 */
		strcpy( m_pShareData->m_Types[6].m_szBlockCommentFrom, "/*" );		/* ブロックコメントデリミタ(From) */
		strcpy( m_pShareData->m_Types[6].m_szBlockCommentTo, "*/" );		/* ブロックコメントデリミタ(To) */
		m_pShareData->m_Types[6].m_nKeyWordSetIdx = 4;						/* キーワードセット */
		m_pShareData->m_Types[6].m_nDefaultOutline = OUTLINE_JAVA;			/* アウトライン解析方法 */
		m_pShareData->m_Types[6].m_nSmartIndent = SMARTINDENT_CPP;			/* スマートインデント種別 */
		//Mar. 10, 2001 JEPRO	半角数値を色分け表示
		m_pShareData->m_Types[6].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

		/* アセンブラ */
		m_pShareData->m_Types[7].m_nDefaultOutline = OUTLINE_ASM;			/* アウトライン解析方法 */
		//Mar. 10, 2001 JEPRO	半角数値を色分け表示
		m_pShareData->m_Types[7].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

		/* awk */
		strcpy( m_pShareData->m_Types[8].m_szLineComment, "#" );			/* 行コメントデリミタ */
		m_pShareData->m_Types[8].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
		m_pShareData->m_Types[8].m_nKeyWordSetIdx = 6;						/* キーワードセット */

		/* MS-DOSバッチファイル */
		strcpy( m_pShareData->m_Types[9].m_szLineComment, "REM " );			/* 行コメントデリミタ */
//		strcpy( m_pShareData->m_Types[9].m_szLineComment2, ":" );			/* 行コメントデリミタ2 */	//Oct. 31, 2000 JEPRO ':' を追加(却下された)
		m_pShareData->m_Types[9].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
		m_pShareData->m_Types[9].m_nKeyWordSetIdx = 7;						/* キーワードセット */

		/* Pascal */
		strcpy( m_pShareData->m_Types[10].m_szLineComment, "//" );			/* 行コメントデリミタ */				//Nov. 5, 2000 JEPRO 追加
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentFrom, "{" );		/* ブロックコメントデリミタ(From) */	//Nov. 5, 2000 JEPRO 追加
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentTo, "}" );		/* ブロックコメントデリミタ(To) */		//Nov. 5, 2000 JEPRO 追加
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentFrom2, "(*" );	/* ブロックコメントデリミタ2(From) */
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentTo2, "*)" );		/* ブロックコメントデリミタ2(To) */
//#endif
		m_pShareData->m_Types[10].m_nStringType = 1;						/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO 追加
		m_pShareData->m_Types[10].m_nKeyWordSetIdx = 8;						/* キーワードセット */
		//Mar. 10, 2001 JEPRO	半角数値を色分け表示
		m_pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

		//From Here Oct. 31, 2000 JEPRO
		/* TeX */
		strcpy( m_pShareData->m_Types[11].m_szLineComment, "%" );			/* 行コメントデリミタ */
		m_pShareData->m_Types[11].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
		m_pShareData->m_Types[11].m_nKeyWordSetIdx  = 9;					/* キーワードセット */
		m_pShareData->m_Types[11].m_nKeyWordSetIdx2 = 10;					/* キーワードセット2 */	//Jan. 19, 2001 JEPRO
		//シングルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//ダブルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
		//URLにアンダーラインを引かない(やっぱりやめた)
//		m_pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
		//To Here Oct. 31, 2000

		//From Here Jul. 08, 2001 JEPRO
		/* Perl */
		strcpy( m_pShareData->m_Types[12].m_szLineComment, "#" );			/* 行コメントデリミタ */
		m_pShareData->m_Types[12].m_nDefaultOutline = OUTLINE_PERL;			/* アウトライン解析方法 */
		m_pShareData->m_Types[12].m_nKeyWordSetIdx  = 11;					/* キーワードセット */
		m_pShareData->m_Types[12].m_nKeyWordSetIdx2 = 12;					/* キーワードセット2 */
		m_pShareData->m_Types[12].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* 半角数値を色分け表示 */
		//To Here Jul. 08, 2001

		//From Here Jul. 10, 2001 JEPRO
		/* Visual Basic */
		strcpy( m_pShareData->m_Types[13].m_szLineComment, "'" );			/* 行コメントデリミタ */
		m_pShareData->m_Types[13].m_nDefaultOutline = OUTLINE_VB;			/* アウトライン解析方法 */
		m_pShareData->m_Types[13].m_nKeyWordSetIdx  = 13;					/* キーワードセット */
		m_pShareData->m_Types[13].m_nKeyWordSetIdx2 = 14;					/* キーワードセット2 */
		m_pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* 半角数値を色分け表示 */
		m_pShareData->m_Types[13].m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		//シングルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;

		/* リッチテキスト */
		m_pShareData->m_Types[14].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
		m_pShareData->m_Types[14].m_nKeyWordSetIdx  = 15;					/* キーワードセット */
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* 半角数値を色分け表示 */
		m_pShareData->m_Types[14].m_nStringType = 0;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		//シングルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//ダブルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
		//URLにアンダーラインを引かない
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
		//To Here Jul. 10, 2001

		//From Here Nov. 9, 2000 JEPRO
		/* 設定ファイル */
		strcpy( m_pShareData->m_Types[15].m_szLineComment, "//" );			/* 行コメントデリミタ */
		strcpy( m_pShareData->m_Types[15].m_szLineComment2, ";" );			/* 行コメントデリミタ2 */
		m_pShareData->m_Types[15].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
		//シングルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//ダブルクォーテーション文字列を色分け表示しない
		m_pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
		//To Here Nov. 9, 2000


		/* 強調キーワードのテストデータ */
		m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "C/C++", TRUE );			/* セット 0の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "HTML", FALSE );			/* セット 1の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "PL/SQL", FALSE );		/* セット 2の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "COBOL", TRUE );			/* セット 3の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Java", TRUE );			/* セット 4の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "CORBA IDL", TRUE );		/* セット 5の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "AWK", TRUE );			/* セット 6の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "MS-DOS batch", FALSE );	/* セット 7の追加 */	//Oct. 31, 2000 JEPRO 'バッチファイル'→'batch' に短縮
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Pascal", FALSE );		/* セット 8の追加 */	//Nov. 5, 2000 JEPRO 大・小文字の区別を'しない'に変更
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "TeX", TRUE );			/* セット 9の追加 */	//Sept. 2, 2000 jepro Tex →TeX に修正 Bool値は大・小文字の区別
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "TeX2", TRUE );			/* セット10の追加 */	//Jan. 19, 2001 JEPRO 追加
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Perl", TRUE );			/* セット11の追加 */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Perl2", TRUE );			/* セット12の追加 */	//Jul. 10, 2001 JEPRO Perlから変数を分離・独立
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Visual Basic", FALSE );	/* セット13の追加 */	//Jul. 10, 2001 JEPRO
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Visual Basic2", FALSE );	/* セット14の追加 */	//Jul. 10, 2001 JEPRO
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "リッチテキスト", TRUE );	/* セット15の追加 */	//Jul. 10, 2001 JEPRO



		/* ｎ番目のセットにキーワードを追加 */
		static const char*	ppszKeyWordsCPP[] = {
			"#define",
			"#elif",
			"#else",
			"#endif",
			"#error",
			"#if",
			"#ifdef",
			"#ifndef",
			"#include",
			"#line",
			"#pragma",
			"#undef",
			"__declspec",
			"__FILE__",
			"asm",
			"auto",
			"break",
			"bool",
			"case",
			"catch",
			"char",
			"class",
			"const",
			"continue",
			"default",
			"define",
			"delete",
			"do",
			"double",
			"elif",
			"else",
			"endif",
			"enum",
			"error",
			"explicit",
			"extern",
			"false",
			"float",
			"for",
			"friend",
			"goto",
			"if",
			"ifdef",
			"ifndef",
			"include",
			"inline",
			"int",
			"line",
			"long",
			"mutable",
			"namespace",
			"new",
			"operator",
			"pragma",
			"private",
			"protected",
			"public",
			"register",
			"return",
			"short",
			"signed",
			"sizeof",
			"static",
			"struct",
			"switch",
			"template",
			"this",
			"throw",
			"try",
			"true",
			"typedef",
			"typename",
			"undef",
			"union",
			"unsigned",
			"using",
			"virtual",
			"void",
			"volatile",
			"while"
		};
		static int nKeyWordsCPP_Num = sizeof( ppszKeyWordsCPP ) / sizeof( ppszKeyWordsCPP[0] );
		for( i = 0; i < nKeyWordsCPP_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 0, ppszKeyWordsCPP[i] );
		}


		static const char*	ppszKeyWordsHTML[] = {
			"_blank",
			"_parent",
			"_self",
			"_top",
			"A",
			"ABBR",
			"ABOVE",
			"absbottom",
			"absmiddle",
			"ACCESSKEY",
			"ACRONYM",
			"ACTION",
			"ADDRESS",
			"ALIGN",
			"all",
			"APPLET",
			"AREA",
			"AUTOPLAY",
			"AUTOSTART",
			"B",
			"BACKGROUND",
			"BASE",
			"BASEFONT",
			"baseline",
			"BEHAVIOR",
			"BELOW",
			"BGCOLOR",
			"BGSOUND",
			"BIG",
			"BLINK",
			"BLOCKQUOTE",
			"BODY",
			"BORDER",
			"BORDERCOLOR",
			"BORDERCOLORDARK",
			"BORDERCOLORLIGHT",
			"BOTTOM",
			"box",
			"BR",
			"BUTTON",
			"CAPTION",
			"CELLPADDING",
			"CELLSPACING",
			"CENTER",
			"CHALLENGE",
			"char",
			"checkbox",
			"CHECKED",
			"CITE",
			"CLEAR",
			"CLIP",
			"CODE",
			"CODEBASE",
			"CODETYPE",
			"COL",
			"COLGROUP",
			"COLOR",
			"COLS",
			"COLSPAN",
			"COMMENT",
			"CONTROLS",
			"DATA",
			"DD",
			"DECLARE",
			"DEFER",
			"DEL",
			"DELAY",
			"DFN",
			"DIR",
			"DIRECTION",
			"DISABLED",
			"DIV",
			"DL",
			"DOCTYPE",
			"DT",
			"EM",
			"EMBED",
			"ENCTYPE",
			"FACE",
			"FIELDSET",
			"file",
			"FONT",
			"FOR",
			"FORM",
			"FRAME",
			"FRAMEBORDER",
			"FRAMESET",
			"GET",
			"groups",
			"GROUPS",
			"GUTTER",
			"H1",
			"H2",
			"H3",
			"H4",
			"H5",
			"H6",
			"H7",
			"HEAD",
			"HEIGHT",
			"HIDDEN",
			"Hn",
			"HR",
			"HREF",
			"hsides",
			"HSPACE",
			"HTML",
			"I",
			"ID",
			"IFRAME",
			"ILAYER",
			"image",
			"IMG",
			"INDEX",
			"inherit",
			"INPUT",
			"INS",
			"ISINDEX",
			"JavaScript",
			"justify",
			"KBD",
			"KEYGEN",
			"LABEL",
			"LANGUAGE",
			"LAYER",
			"LEFT",
			"LEGEND",
			"lhs",
			"LI",
			"LINK",
			"LISTING",
			"LOOP",
			"MAP",
			"MARQUEE",
			"MAXLENGTH",
			"MENU",
			"META",
			"METHOD",
			"METHODS",
			"MIDDLE",
			"MULTICOL",
			"MULTIPLE",
			"NAME",
			"NEXT",
			"NEXTID",
			"NOBR",
			"NOEMBED",
			"NOFRAMES",
			"NOLAYER",
			"none",
			"NOSAVE",
			"NOSCRIPT",
			"NOTAB",
			"NOWRAP",
			"OBJECT",
			"OL",
			"onBlur",
			"onChange",
			"onClick",
			"onFocus",
			"onLoad",
			"onMouseOut",
			"onMouseOver",
			"onReset",
			"onSelect",
			"onSubmit",
			"OPTION",
			"P",
			"PAGEX",
			"PAGEY",
			"PALETTE",
			"PANEL",
			"PARAM",
			"PARENT",
			"password",
			"PLAINTEXT",
			"PLUGINSPAGE",
			"POST",
			"PRE",
			"PREVIOUS",
			"Q",
			"radio",
			"REL",
			"REPEAT",
			"reset",
			"REV",
			"rhs",
			"RIGHT",
			"rows",
			"ROWSPAN",
			"RULES",
			"S",
			"SAMP",
			"SAVE",
			"SCRIPT",
			"SCROLLAMOUNT",
			"SCROLLDELAY",
			"SELECT",
			"SELECTED",
			"SERVER",
			"SHAPES",
			"show",
			"SIZE",
			"SMALL",
			"SONG",
			"SPACER",
			"SPAN",
			"SRC",
			"STANDBY",
			"STRIKE",
			"STRONG",
			"STYLE",
			"SUB",
			"submit",
			"SUMMARY",
			"SUP",
			"TABINDEX",
			"TABLE",
			"TARGET",
			"TBODY",
			"TD",
			"TEXT",
			"TEXTAREA",
			"textbottom",
			"TEXTFOCUS",
			"textmiddle",
			"texttop",
			"TFOOT",
			"TH",
			"THEAD",
			"TITLE",
			"TOP",
			"TR",
			"TT",
			"TXTCOLOR",
			"TYPE",
			"U",
			"UL",
			"URN",
			"USEMAP",
			"VALIGN",
			"VALUE",
			"VALUETYPE",
			"VAR",
			"VISIBILITY",
			"void",
			"vsides",
			"VSPACE",
			"WBR",
			"WIDTH",
			"WRAP",
			"XMP"
		};
		static int nKeyWordsHTML_Num = sizeof( ppszKeyWordsHTML ) / sizeof( ppszKeyWordsHTML[0] );
		for( i = 0; i < nKeyWordsHTML_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 1, ppszKeyWordsHTML[i] );
		}


		static const char*	ppszKeyWordsPLSQL[] = {
			"AND",
			"AS",
			"BEGIN",
			"BINARY_INTEGER",
			"BODY",
			"BOOLEAN",
			"BY",
			"CHAR",
			"CHR",
			"COMMIT",
			"COUNT",
			"CREATE",
			"CURSOR",
			"DATE",
			"DECLARE",
			"DEFAULT",
			"DELETE",
			"ELSE",
			"ELSIF",
			"END",
			"ERRORS",
			"EXCEPTION",
			"FALSE",
			"FOR",
			"FROM",
			"FUNCTION",
			"GOTO",
			"HTP",
			"IDENT_ARR",
			"IF",
			"IN",
			"INDEX",
			"INTEGER",
			"IS",
			"LOOP",
			"NOT",
			"NO_DATA_FOUND",
			"NULL",
			"NUMBER",
			"OF",
			"OR",
			"ORDER",
			"OUT",
			"OWA_UTIL",
			"PACKAGE",
			"PRAGMA",
			"PRN",
			"PROCEDURE",
			"REPLACE",
			"RESTRICT_REFERENCES",
			"RETURN",
			"ROWTYPE",
			"SELECT",
			"SHOW",
			"SUBSTR",
			"TABLE",
			"THEN",
			"TRUE",
			"TYPE",
			"UPDATE",
			"VARCHAR",
			"VARCHAR2",
			"WHEN",
			"WHERE",
			"WHILE",
			"WNDS",
			"WNPS",
			"RAISE",
			"INSERT",
			"INTO",
			"VALUES",
			"SET",
			"SYSDATE",
			"RTRIM",
			"LTRIM",
			"TO_CHAR",
			"DUP_VAL_ON_INDEX",
			"ROLLBACK",
			"OTHERS",
			"SQLCODE"
		};
		static int nKeyWordsPLSQL_Num = sizeof( ppszKeyWordsPLSQL ) / sizeof( ppszKeyWordsPLSQL[0] );
		for( i = 0; i < nKeyWordsPLSQL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 2, ppszKeyWordsPLSQL[i] );
		}


//Jul. 10, 2001 JEPRO 追加
		static const char*	ppszKeyWordsCOBOL[] = {
			"ACCEPT",
			"ADD",
			"ADVANCING",
			"AFTER",
			"ALL",
			"AND",
			"ARGUMENT",
			"ASSIGN",
			"AUTHOR",
			"BEFORE",
			"BLOCK",
			"BY",
			"CALL",
			"CHARACTERS",
			"CLOSE",
			"COMP",
			"COMPILED",
			"COMPUTE",
			"COMPUTER",
			"CONFIGURATION",
			"CONSOLE",
			"CONTAINS",
			"CONTINUE",
			"CONTROL",
			"COPY",
			"DATA",
			"DELETE",
			"DISPLAY",
			"DIVIDE",
			"DIVISION",
			"ELSE",
			"END",
			"ENVIRONMENT",
			"EVALUATE",
			"EXAMINE",
			"EXIT",
			"EXTERNAL",
			"FD",
			"FILE",
			"FILLER",
			"FROM",
			"GIVING",
			"GO",
			"GOBACK",
			"HIGH-VALUE",
			"IDENTIFICATION"
			"IF",
			"INITIALIZE",
			"INPUT",
			"INTO",
			"IS",
			"LABEL",
			"LINKAGE",
			"LOW-VALUE",
			"MODE",
			"MOVE",
			"NOT",
			"OBJECT",
			"OCCURS",
			"OF",
			"ON",
			"OPEN",
			"OR",
			"OTHER",
			"OUTPUT",
			"PERFORM",
			"PIC",
			"PROCEDURE",
			"PROGRAM",
			"READ",
			"RECORD",
			"RECORDING",
			"REDEFINES",
			"REMAINDER",
			"REMARKS",
			"REPLACING",
			"REWRITE",
			"ROLLBACK",
			"SECTION",
			"SELECT",
			"SOURCE",
			"SPACE",
			"STANDARD",
			"STOP",
			"STORAGE",
			"SYSOUT",
			"TEST",
			"THEN",
			"TO",
			"TODAY",
			"TRANSFORM",
			"UNTIL",
			"UPON",
			"USING",
			"VALUE",
			"VARYING",
			"WHEN",
			"WITH",
			"WORKING",
			"WRITE",
			"WRITTEN",
			"ZERO"
		};
		static int nKeyWordsCOBOL_Num = sizeof( ppszKeyWordsCOBOL ) / sizeof( ppszKeyWordsCOBOL[0] );
		for( i = 0; i < nKeyWordsCOBOL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 3, ppszKeyWordsCOBOL[i] );
		}


		static const char*	ppszKeyWordsJAVA[] = {
			"abstract",
			"boolean",
			"break",
			"byte",
			"case",
			"catch",
			"char",
			"class",
			"const",
			"continue",
			"default",
			"do",
			"double",
			"else",
			"extends",
			"final",
			"finally",
			"float",
			"for",
			"goto",
			"if",
			"implements",
			"import",
			"instanceof",
			"int",
			"interface",
			"long",
			"native",
			"new",
			"package",
			"private",
			"protected",
			"public",
			"return",
			"short",
			"static",
			"super",
			"switch",
			"synchronized",
			"this",
			"throw",
			"throws",
			"transient",
			"try",
			"void",
			"volatile",
			"while"
		};
		static int nKeyWordsJAVA_Num = sizeof( ppszKeyWordsJAVA ) / sizeof( ppszKeyWordsJAVA[0] );
		for( i = 0; i < nKeyWordsJAVA_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 4, ppszKeyWordsJAVA[i] );
		}


		static const char*	ppszKeyWordsCORBA_IDL[] = {
			"any",
			"attribute",
			"boolean",
			"case",
			"char",
			"const",
			"context",
			"default",
			"double",
			"enum",
			"exception",
			"FALSE",
			"fixed",
			"float",
			"in",
			"inout",
			"interface",
			"long",
			"module",
			"Object",
			"octet",
			"oneway",
			"out",
			"raises",
			"readonly",
			"sequence",
			"short",
			"string",
			"struct",
			"switch",
			"TRUE",
			"typedef",
			"unsigned",
			"union",
			"void",
			"wchar",
			"wstring"
		};
		static int nKeyWordsCORBA_IDL_Num = sizeof( ppszKeyWordsCORBA_IDL ) / sizeof( ppszKeyWordsCORBA_IDL[0] );
		for( i = 0; i < nKeyWordsCORBA_IDL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 5, ppszKeyWordsCORBA_IDL[i] );
		}


		static const char*	ppszKeyWordsAWK[] = {
			"BEGIN",
			"END",
			"next",
			"exit",
			"func",
			"function",
			"return",
			"if",
			"else",
			"for",
			"in",
			"do",
			"while",
			"break",
			"continue",
			"$0",
			"$1",
			"$2",
			"$3",
			"$4",
			"$5",
			"$6",
			"$7",
			"$8",
			"$9",
			"$10",
			"$11",
			"$12",
			"$13",
			"$14",
			"$15",
			"$16",
			"$17",
			"$18",
			"$19",
			"$20",
			"FS",
			"OFS",
			"NF",
			"RS",
			"ORS",
			"NR",
			"FNR",
			"ARGV",
			"ARGC",
			"ARGIND",
			"FILENAME",
			"ENVIRON",
			"ERRNO",
			"OFMT",
			"CONVFMT",
			"FIELDWIDTHS",
			"IGNORECASE",
			"RLENGTH",
			"RSTART",
			"SUBSEP",
			"delete",
			"index",
			"jindex",
			"length",
			"jlength",
			"substr",
			"jsubstr",
			"match",
			"split",
			"sub",
			"gsub",
			"sprintf",
			"tolower",
			"toupper",
			"print",
			"printf",
			"getline",
			"system",
			"close",
			"sin",
			"cos",
			"atan2",
			"exp",
			"log",
			"int",
			"sqrt",
			"srand",
			"rand",
			"strftime",
			"systime"
		};
		static int nKeyWordsAWK_Num = sizeof( ppszKeyWordsAWK ) / sizeof( ppszKeyWordsAWK[0] );
		for( i = 0; i < nKeyWordsAWK_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 6, ppszKeyWordsAWK[i] );
		}


		static const char*	ppszKeyWordsBAT[] = {
			"PATH",
			"PROMPT",
			"TEMP",
			"TMP",
			"TZ",
			"CONFIG",
			"COMSPEC",
			"DIRCMD",
			"COPYCMD",
			"winbootdir",
			"windir",
			"DIR",
			"CALL",
			"CHCP",
			"RENAME",
			"REN",
			"ERASE",
			"DEL",
			"TYPE",
			"REM",
			"COPY",
			"PAUSE",
			"DATE",
			"TIME",
			"VER",
			"VOL",
			"CD",
			"CHDIR",
			"MD",
			"MKDIR",
			"RD",
			"RMDIR",
			"BREAK",
			"VERIFY",
			"SET",
//			"PROMPT",	//Oct. 31, 2000 JEPRO この2つは既に登録されているので省略
//			"PATH",
			"EXIT",
			"CTTY",
			"ECHO",
			"@ECHO",	//Oct. 31, 2000 JEPRO '@' を強調可能にしたので追加
			"LOCK",
			"UNLOCK",
			"GOTO",
			"SHIFT",
			"IF",
			"FOR",
			"DO",	//Nov. 2, 2000 JEPRO 追加
			"IN",	//Nov. 2, 2000 JEPRO 追加
			"ELSE",	//Nov. 2, 2000 JEPRO 追加 Win2000で使える
			"CLS",
			"TRUENAME",
			"LOADHIGH",
			"LH",
			"LFNFOR",
			"ON",
			"OFF",
			"NOT",
			"ERRORLEVEL",
			"EXIST",
			"NUL",
			"CON",
			"AUX",
			"COM1",
			"COM2",
			"COM3",
			"COM4",
			"PRN",
			"LPT1",
			"LPT2",
			"LPT3",
			"CLOCK",
			"CLOCK$",
			"CONFIG$"
		};
		static int nKeyWordsBAT_Num = sizeof( ppszKeyWordsBAT ) / sizeof( ppszKeyWordsBAT[0] );
		for( i = 0; i < nKeyWordsBAT_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 7, ppszKeyWordsBAT[i] );
		}


		static const char*	ppszKeyWordsPASCAL[] = {
			"and",
			"exports",
			"mod",
			"shr",
			"array",
			"file",
			"nil",
			"string",
			"as",
			"finalization",
			"not",
			"stringresource",
			"asm",
			"finally",
			"object",
			"then",
			"begin",
			"for",
			"of",
			"case",
			"function",
			"or",
			"to",
			"class",
			"goto",
			"out",
			"try",
			"const",
			"if",
			"packed",
			"type",
			"constructor",
			"implementation",
			"procedure",
			"unit",
			"destructor",
			"in",
			"program",
			"until",
			"dispinterface",
			"inherited",
			"property",
			"uses",
			"div",
			"initialization",
			"raise",
			"var",
			"do",
			"inline",
			"record",
			"while",
			"downto",
			"interface",
			"repeat",
			"with",
			"else",
			"is",
			"resourcestring",
			"xor",
			"end",
			"label",
			"set",
			"except",
			"library",
			"shl",
			"private",
			"public",
			"published",
			"protected",
			"override"
		};
		static int nKeyWordsPASCAL_Num = sizeof( ppszKeyWordsPASCAL ) / sizeof( ppszKeyWordsPASCAL[0] );
		for( i = 0; i < nKeyWordsPASCAL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 8, ppszKeyWordsPASCAL[i] );
		}


		static const char*	ppszKeyWordsTEX[] = {
//Nov. 20, 2000 JEPRO	大幅追加 & 若干修正・削除 --ほとんどコマンドのみ
			"error",
			"Warning",
//			"center",
//			"document",
//			"enumerate",
//			"eqnarray",
//			"figure",
//			"itemize",
//			"minipage",
//			"tabbing",
//			"table",
//			"tabular",
//			"\\!",
//			"\\#",
//			"\\%",
//			"\\&",
//			"\\(",
//			"\\)",
//			"\\+",
//			"\\,",
//			"\\-",
//			"\\:",
//			"\\;",
//			"\\<",
//			"\\=",
//			"\\>",
			"\\aa",
			"\\AA",
			"\\acute",
			"\\addcontentsline",
			"\\addtocounter",
			"\\addtolength",
			"\\ae",
			"\\AE",
			"\\aleph",
			"\\alpha",
			"\\alph",
			"\\Alph",
			"\\and",
			"\\angle",
			"\\appendix",
			"\\approx",
			"\\arabic",
			"\\arccos",
			"\\arctan",
			"\\arg",
			"\\arrayrulewidth",
			"\\arraystretch",
			"\\ast",
			"\\atop",
			"\\author",
			"\\b",
			"\\backslash",
			"\\bar",
			"\\baselineskip",
			"\\baselinestretch",
			"\\begin",
			"\\beta",
			"\\bf",
			"\\bibitem",
			"\\bibliography",
			"\\bibliographystyle",
			"\\big",
			"\\Big",
			"\\bigcap",
			"\\bigcirc",
			"\\bigcup",
			"\\bigg",
			"\\Bigg",
			"\\Biggl",
			"\\Biggm",
			"\\biggl",
			"\\biggm",
			"\\biggr",
			"\\Biggr",
			"\\bigl",
			"\\bigm",
			"\\Bigm",
			"\\Bigl",
			"\\bigodot",
			"\\bigoplus",
			"\\bigotimes",
			"\\bigr",
			"\\Bigr",
			"\\bigskip",
			"\\bigtriangledown",
			"\\bigtriangleup",
			"\\boldmath",
			"\\bot",
			"\\Box",
			"\\brace",
			"\\breve",
			"\\bullet",
			"\\bye",
			"\\c",
			"\\cal",
			"\\cap",
			"\\caption",
			"\\cc",
			"\\cdot",
			"\\cdots",
			"\\centering",
			"\\chapter",
			"\\check",
			"\\chi",
			"\\choose",
			"\\circ",
			"\\circle",
			"\\cite",
			"\\clearpage",
			"\\cline",
			"\\closing",
			"\\clubsuit",
			"\\colon",
			"\\columnsep",
			"\\columnseprule",
			"\\cong",
			"\\cot",
			"\\coth",
			"\\cr",
			"\\cup",
			"\\d",
			"\\dag",
			"\\dagger",
			"\\date",
			"\\dashbox",
			"\\ddag",
			"\\ddot",
			"\\ddots",
			"\\def",
			"\\deg",
			"\\delta",
			"\\Delta",
			"\\det",
			"\\diamond",
			"\\diamondsuit",
			"\\dim",
			"\\displaystyle",
			"\\documentclass",
			"\\documentstyle",
			"\\dot",
			"\\doteq",
			"\\dotfill",
			"\\Downarrow",
			"\\downarrow",
			"\\ell",
			"\\em",
			"\\emptyset",
			"\\encl",
			"\\end",
			"\\enspace",
			"\\enskip",
			"\\epsilon",
			"\\eqno",
			"\\equiv",
			"\\evensidemargin",
			"\\eta",
			"\\exists",
			"\\exp",
			"\\fbox",
			"\\fboxrule",
			"\\flat",
			"\\footnote",
			"\\footnotesize",
			"\\forall",
			"\\frac",
			"\\frame",
			"\\framebox",
			"\\gamma",
			"\\Gamma",
			"\\gcd",
			"\\ge",
			"\\geq",
			"\\gets",
			"\\gg",
			"\\grave",
			"\\gt",
			"\\H",
			"\\hat",
			"\\hbar",
			"\\hbox",
			"\\headsep",
			"\\heartsuit",
			"\\hfil",
			"\\hfill",
			"\\hline",
			"\\hom",
			"\\hrulefill",
			"\\hskip",
			"\\hspace",
			"\\hspace*",
			"\\huge",
			"\\Huge",
			"\\i",
			"\\Im",
			"\\imath",
			"\\in",
			"\\include",
			"\\includegraphics",
			"\\includeonly",
			"\\indent",
			"\\index",
			"\\inf",
			"\\infty",
			"\\input",
			"\\int",
			"\\iota",
			"\\it",
			"\\item",
			"\\itemsep",
			"\\j",
			"\\jmath",
			"\\kappa",
			"\\ker",
			"\\kern",
			"\\kill",
			"\\l",
			"\\L",
			"\\label",
			"\\lambda",
			"\\Lambda",
			"\\land",
			"\\langle",
			"\\large",
			"\\Large",
			"\\LARGE",
			"\\LaTeX",
			"\\LaTeXe",
			"\\lceil",
			"\\ldots",
			"\\le",
			"\\leftarrow",
			"\\Leftarrow",
			"\\lefteqn",
			"\\leftharpoondown",
			"\\leftharpoonup",
			"\\leftmargin",
			"\\leftrightarrow",
			"\\Leftrightarrow",
			"\\leq",
			"\\leqno",
			"\\lfloor",
			"\\lg",
			"\\lim",
			"\\liminf",
			"\\limsup",
			"\\line",
			"\\linebreak",
			"\\linewidth",
			"\\listoffigures",
			"\\listoftables",
			"\\ll",
			"\\llap",
			"\\ln",
			"\\lnot",
			"\\log",
			"\\longleftarrow",
			"\\Longleftarrow",
			"\\longleftrightarrow",
			"\\Longleftrightarrow",
			"\\longrightarrow",
			"\\Longrightarrow",
			"\\lor",
			"\\lower",
			"\\magstep",
			"\\makeatletter",
			"\\makeatother",
			"\\makebox",
			"\\makeindex",
			"\\maketitle",
			"\\makelabels",
			"\\mathop",
			"\\mapsto",
			"\\markboth",
			"\\markright",
			"\\mathstrut",
			"\\max",
			"\\mbox",
			"\\mc",
			"\\medskip",
			"\\mid",
			"\\min",
			"\\mit",
			"\\mp",
			"\\mu",
			"\\multicolumn",
			"\\multispan",
			"\\multiput",
			"\\nabla",
			"\\natural",
			"\\ne",
			"\\neg",
			"\\nearrow",
			"\\nwarrow",
			"\\neq",
			"\\newblock",
			"\\newcommand",
			"\\newenvironment",
			"\\newfont",
			"\\newlength",
			"\\newline",
			"\\newpage",
			"\\newtheorem",
			"\\ni",
			"\\noalign",
			"\\noindent",
			"\\nolimits",
			"\\nolinebreak",
			"\\nonumber",
			"\\nopagebreak",
			"\\normalsize",
			"\\not",
			"\\notice",
			"\\notin",
			"\\nu",
			"\\o",
			"\\O",
			"\\oddsidemargin",
			"\\odot",
			"\\oe",
			"\\OE",
			"\\oint",
			"\\Omega",
			"\\omega",
			"\\ominus",
			"\\oplus",
			"\\opening",
			"\\otimes",
			"\\owns",
			"\\overleftarrow",
			"\\overline",
			"\\overrightarrow",
			"\\overvrace",
			"\\oval",
			"\\P",
			"\\pagebreak",
			"\\pagenumbering",
			"\\pageref",
			"\\pagestyle",
			"\\par",
			"\\parallel",
			"\\paragraph",
			"\\parbox",
			"\\parindent",
			"\\parskip",
			"\\partial",
			"\\perp",
			"\\phi",
			"\\Phi",
			"\\pi",
			"\\Pi",
			"\\pm",
			"\\Pr",
			"\\prime",
			"\\printindex",
			"\\prod",
			"\\propto",
			"\\ps",
			"\\psi",
			"\\Psi",
			"\\put",
			"\\qquad",
			"\\quad",
			"\\raisebox",
			"\\rangle",
			"\\rceil",
			"\\Re",
			"\\ref",
			"\\renewcommand",
			"\\renewenvironment",
			"\\rfloor",
			"\\rho",
			"\\right",
			"\\rightarrow",
			"\\Rightarrow",
			"\\rightharpoondown",
			"\\rightharpoonup",
			"\\rightleftharpoonup",
			"\\rightmargin",
			"\\rm",
			"\\rule",
			"\\roman",
			"\\Roman",
			"\\S",
			"\\samepage",
			"\\sb",
			"\\sc",
			"\\scriptsize",
			"\\scriptscriptstyle",
			"\\scriptstyle",
			"\\searrow",
			"\\sec",
			"\\section",
			"\\setcounter",
			"\\setlength",
			"\\settowidth",
			"\\setminus",
			"\\sf",
			"\\sharp",
			"\\sigma",
			"\\Sigma",
			"\\signature",
			"\\sim",
			"\\simeq",
			"\\sin",
			"\\sinh",
			"\\sl",
			"\\sloppy",
			"\\small",
			"\\smash",
			"\\smallskip",
			"\\sp",
			"\\spadesuit",
			"\\special",
			"\\sqrt",
			"\\ss",
			"\\star",
			"\\stackrel",
			"\\strut",
			"\\subparagraph",
			"\\subsection",
			"\\subset",
			"\\subseteq",
			"\\subsubsection",
			"\\sum",
			"\\sup",
			"\\supset",
			"\\supseteq",
			"\\swarrow",
			"\\t",
			"\\tableofcontents",
			"\\tan",
			"\\tanh",
			"\\tau",
			"\\TeX",
			"\\textbf",
			"\\textgreater",
			"\\textgt",
			"\\textheight",
			"\\textit",
			"\\textless",
			"\\textmc",
			"\\textrm",
			"\\textsc",
			"\\textsf",
			"\\textsl",
			"\\textstyle",
			"\\texttt",
			"\\textwidth",
			"\\thanks",
			"\\thebibliography",
			"\\theequation",
			"\\thepage",
			"\\thesection",
			"\\theta",
			"\\Theta",
			"\\thicklines",
			"\\thinlines",
			"\\thinspace",
			"\\thisepage",
			"\\thisepagestyle",
			"\\tie",
			"\\tilde",
			"\\times",
			"\\tiny",
			"\\title",
			"\\titlepage",
			"\\to",
			"\\toaddress",
			"\\topmargin",
			"\\triangle",
			"\\tt",
			"\\twocolumn",
			"\\u",
			"\\underline",
			"\\undervrace",
			"\\unitlength",
			"\\Uparrow",
			"\\uparrow",
			"\\updownarrow",
			"\\Updownarrow",
			"\\uplus",
			"\\upsilon",
			"\\Upsilon",
			"\\usepackage",
			"\\v",
			"\\varepsilon",
			"\\varphi",
			"\\varpi",
			"\\varrho",
			"\\varsigma",
			"\\vartheta",
			"\\vbox",
			"\\vcenter",
			"\\vec",
			"\\vector",
			"\\vee",
			"\\verb",
			"\\verb*",
			"\\verbatim",
			"\\vert",
			"\\Vert",
			"\\vfil",
			"\\vfill",
			"\\vrule",
			"\\vskip",
			"\\vspace",
			"\\vspace*",
			"\\wedge",
			"\\widehat",
			"\\widetilde",
			"\\wp",
			"\\wr",
			"\\wrapfigure",
			"\\xi",
			"\\Xi",
			"\\zeta"//,
//			"\\[",
//			"\\\"",
//			"\\\'",
//			"\\\\",
//			"\\]",
//			"\\^",
//			"\\_",
//			"\\`",
//			"\\{",
//			"\\|",
//			"\\}",
//			"\\~",
		};
		static int nKeyWordsTEX_Num = sizeof( ppszKeyWordsTEX ) / sizeof( ppszKeyWordsTEX[0] );
		for( i = 0; i < nKeyWordsTEX_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 9, ppszKeyWordsTEX[i] );
		}


//Jan. 19, 2001 JEPRO	TeX のキーワード2として新規追加 & 一部復活 --環境コマンドとオプション名が中心
		static const char*	ppszKeyWordsTEX2[] = {
//	環境コマンド
//Jan. 19, 2001 JEPRO 本当は{}付きでキーワードにしたかったが単語として認識してくれないので止めた
			"abstract",
			"array"
			"center",
			"description",
			"document",
			"displaymath",
			"em",
			"enumerate",
			"eqnarray",
			"eqnarray*",
			"equation",
			"figure",
			"figure*",
			"floatingfigure",
			"flushleft",
			"flushright",
			"itemize",
			"letter",
			"list",
			"math",
			"minipage",
			"multicols",
			"namelist",
			"picture",
			"quotation",
			"quote",
			"sloppypar",
			"subeqnarray",
			"subeqnarray*",
			"subequations",
			"subfigure",
			"tabbing",
			"table",
			"table*",
			"tabular",
			"tabular*",
			"tatepage",
			"thebibliography",
			"theindex",
			"titlepage",
			"trivlist",
			"verbatim",
			"verbatim*",
			"verse",
			"wrapfigure",
//
//	スタイルオプション
			"a4",
			"a4j",
			"a5",
			"a5j",
			"Alph",
			"alph",
			"annote",
			"arabic",
			"b4",
			"b4j",
			"b5",
			"b5j",
			"bezier",
			"booktitle",
			"boxedminipage",
			"boxit",
//			"bp",
//			"cm",
			"dbltopnumber",
//			"dd",
			"eclepsf",
			"eepic",
			"enumi",
			"enumii",
			"enumiii",
			"enumiv",
			"epic",
			"epsbox",
			"epsf",
			"fancybox",
			"fancyheadings",
			"fleqn",
			"footnote",
			"howpublished",
			"jabbrv",
			"jalpha",
//			"article",
			"jarticle",
			"jsarticle",
//			"book",
			"jbook",
			"jsbook",
//			"letter",
			"jletter",
//			"plain",
			"jplain",
//			"report",
			"jreport",
			"jtwocolumn",
			"junsrt",
			"leqno",
			"makeidx",
			"markboth",
			"markright",
//			"mm",
			"multicol",
			"myheadings",
			"openbib",
//			"pc",
//			"pt",
			"secnumdepth",
//			"sp",
			"titlepage",
			"tjarticle",
			"topnumber",
			"totalnumber",
			"twocolumn",
			"twoside",
			"yomi"//,
//			"zh",
//			"zw"
		};
		static int nKeyWordsTEX2_Num = sizeof( ppszKeyWordsTEX2 ) / sizeof( ppszKeyWordsTEX2[0] );
		for( i = 0; i < nKeyWordsTEX2_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 10, ppszKeyWordsTEX2[i] );
		}


		static const char*	ppszKeyWordsPERL[] = {
//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
			"break",
			"continue",
			"do",
			"elsif",
			"else",
			"for",
			"foreach",
			"goto",
			"if",
			"last",
			"next",
			"return",
			"sub",
			"undef",
			"unless",
			"until",
			"while",
			"abs",
			"accept",
			"alarm",
			"atan2",
			"bind",
			"binmode",
			"bless",
			"caller",
			"chdir",
			"chmod",
			"chomp",
			"chop",
			"chown",
			"chr",
			"chroot",
			"close",
			"closedir",
			"connect",
			"continue",
			"cos",
			"crypt",
			"dbmclose",
			"dbmopen",
			"defined",
			"delete",
			"die",
			"do",
			"dump",
			"each",
			"eof",
			"eval",
			"exec",
			"exists",
			"exit",
			"exp",
			"fcntl",
			"fileno",
			"flock",
			"fork",
			"format",
			"formline",
			"getc",
			"getlogin",
			"getpeername",
			"getpgrp",
			"getppid",
			"getpriority",
			"getpwnam",
			"getgrnam",
			"gethostbyname",
			"getnetbyname",
			"getprotobyname",
			"getpwuid",
			"getgrgid",
			"getservbyname",
			"gethostbyaddr",
			"getnetbyaddr",
			"getprotobynumber",
			"getservbyport",
			"getpwent",
			"getgrent",
			"gethostent",
			"getnetent",
			"getprotoent",
			"getservent",
			"setpwent",
			"setgrent",
			"sethostent",
			"setnetent",
			"setprotoent",
			"setservent",
			"endpwent",
			"endgrent",
			"endhostent",
			"endnetent",
			"endprotoent",
			"endservent",
			"getsockname",
			"getsockopt",
			"glob",
			"gmtime",
			"goto",
			"grep",
			"hex",
			"import",
			"index",
			"int",
			"ioctl",
			"join",
			"keys",
			"kill",
			"last",
			"lc",
			"lcfirst",
			"length",
			"link",
			"listen",
			"local",
			"localtime",
			"log",
			"lstat",
//			"//m",
			"map",
			"mkdir",
			"msgctl",
			"msgget",
			"msgsnd",
			"msgrcv",
			"my",
			"next",
			"no",
			"oct",
			"open",
			"opendir",
			"ord",
			"pack",
			"package",
			"pipe",
			"pop",
			"pos",
			"print",
			"printf",
			"prototype",
			"push",
//			"//q",
			"qq",
			"qr",
			"qx",
			"qw",
			"quotemeta",
			"rand",
			"read",
			"readdir",
			"readline",
			"readlink",
			"readpipe",
			"recv",
			"redo",
			"ref",
			"rename",
			"require",
			"reset",
			"return",
			"reverse",
			"rewinddir",
			"rindex",
			"rmdir",
//			"//s",
			"scalar",
			"seek",
			"seekdir",
			"select",
			"semctl",
			"semget",
			"semop",
			"send",
			"setpgrp",
			"setpriority",
			"setsockopt",
			"shift",
			"shmctl",
			"shmget",
			"shmread",
			"shmwrite",
			"shutdown",
			"sin",
			"sleep",
			"socket",
			"socketpair",
			"sort",
			"splice",
			"split",
			"sprintf",
			"sqrt",
			"srand",
			"stat",
			"study",
			"sub",
			"substr",
			"symlink",
			"syscall",
			"sysopen",
			"sysread",
			"sysseek",
			"system",
			"syswrite",
			"tell",
			"telldir",
			"tie",
			"tied",
			"time",
			"times",
			"tr",
			"truncate",
			"uc",
			"ucfirst",
			"umask",
			"undef",
			"unlink",
			"unpack",
			"untie",
			"unshift",
			"use",
			"utime",
			"values",
			"vec",
			"wait",
			"waitpid",
			"wantarray",
			"warn",
			"write"
		};
		static int nKeyWordsPERL_Num = sizeof( ppszKeyWordsPERL ) / sizeof( ppszKeyWordsPERL[0] );
		for( i = 0; i < nKeyWordsPERL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 11, ppszKeyWordsPERL[i] );
		}


		static const char*	ppszKeyWordsPERL2[] = {
//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
			"$ARGV",
			"$_",
			"$1",
			"$2",
			"$3",
			"$4",
			"$5",
			"$6",
			"$7",
			"$8",
			"$9",
			"$0",
			"$MATCH",
			"$&",
			"$PREMATCH",
			"$`",
			"$POSTMATCH",
			"$'",
			"$LAST_PAREN_MATCH",
			"$+",
			"$MULTILINE_MATCHING",
			"$*",
			"$INPUT_LINE_NUMBER",
			"$NR",
			"$.",
			"$INPUT_RECORD_SEPARATOR",
			"$RS",
			"$/",
			"$OUTPUT_AUTOFLUSH",
			"$|",
			"$OUTPUT_FIELD_SEPARATOR",
			"$OFS",
			"$,",
			"$OUTPUT_RECORD_SEPARATOR",
			"$ORS",
			"$\\",
			"$LIST_SEPARATOR",
			"$\"",
			"$SUBSCRIPT_SEPARATOR",
			"$SUBSEP",
			"$;",
			"$OFMT",
			"$#",
			"$FORMAT_PAGE_NUMBER",
			"$%",
			"$FORMAT_LINES_PER_PAGE",
			"$=",
			"$FORMAT_LINES_LEFT",
			"$-",
			"$FORMAT_NAME",
			"$~",
			"$FORMAT_TOP_NAME",
			"$^",
			"$FORMAT_LINE_BREAK_CHARACTERS",
			"$:",
			"$FORMAT_FORMFEED",
			"$^L",
			"$ACCUMULATOR",
			"$^A",
			"$CHILD_ERROR",
			"$?",
			"$OS_ERROR",
			"$ERRNO",
			"$!",
			"$EVAL_ERROR",
			"$@",
			"$PROCESS_ID",
			"$PID",
			"$$",
			"$REAL_USER_ID",
			"$UID",
			"$<",
			"$EFFECTIVE_USER_ID",
			"$EUID",
			"$>",
			"$REAL_GROUP_ID",
			"$GID",
			"$(",
			"$EFFECTIVE_GROUP_ID",
			"$EGID",
			"$)",
			"$PROGRAM_NAME",
			"$0",
			"$[",
			"$PERL_VERSION",
			"$]",
			"$DEBUGGING",
			"$^D",
			"$SYSTEM_FD_MAX",
			"$^F",
			"$INPLACE_EDIT",
			"$^I",
			"$PERLDB",
			"$^P",
			"$BASETIME",
			"$^T",
			"$WARNING",
			"$^W",
			"$EXECUTABLE_NAME",
			"$^X",
			"$ARGV",
			"$ENV",
			"$SIG"
		};
		static int nKeyWordsPERL2_Num = sizeof( ppszKeyWordsPERL2 ) / sizeof( ppszKeyWordsPERL2[0] );
		for( i = 0; i < nKeyWordsPERL2_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 12, ppszKeyWordsPERL2[i] );
		}


//Jul. 10, 2001 JEPRO 追加
		static const char*	ppszKeyWordsVB[] = {
			"And",
			"As",
			"Attribute",
			"Begin",
			"BeginProperty",
			"Boolean",
			"ByVal",
			"Byte",
			"Call",
			"Case",
			"Const",
			"Currency",
			"Date",
			"Declare",
			"Dim",
			"Do",
			"Double",
			"Each",
			"Else",
			"ElseIf",
			"Empty",
			"End",
			"EndProperty",
			"Error",
			"Eqv",
			"Exit",
			"False",
			"For",
			"Friend",
			"Function",
			"Get",
			"GoTo",
			"If",
			"Imp",
			"Integer",
			"Is",
			"Let",
			"Like",
			"Long",
			"Loop",
			"Me",
			"Mod",
			"New",
			"Next",
			"Not",
			"Null",
			"Object",
			"On",
			"Option",
			"Or",
			"Private",
			"Property",
			"Public",
			"RSet",
			"ReDim",
			"Rem",
			"Resume",
			"Select",
			"Set",
			"Single",
			"Static",
			"Step",
			"Stop",
			"String",
			"Sub",
			"Then",
			"To",
			"True",
			"Type",
			"Wend",
			"While",
			"With",
			"Xor",
			"#If",
			"#Else",
			"#End",
			"#Const",
			"AddressOf",
			"Alias",
			"Append",
			"Array",
			"ByRef",
			"Explicit",
			"Global",
			"In",
			"Lib",
			"Nothing",
			"Optional",
			"Output",
			"Terminate",
			"Until",
//=========================================================
// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
//=========================================================
			"DefBool",
			"DefByte",
			"DefCur",
			"DefDate",
			"DefDbl",
			"DefInt",
			"DefLng",
			"DefObj",
			"DefSng",
			"DefStr",
			"DefVar",
			"LSet",
			"GoSub",
			"Return",
			"Variant",
//			"Option Base
//			"As Any
//=========================================================
// 以下はVB.NET用キーワードです
//=========================================================
			//BitAnd
			//BitOr
			//BitNot
			//BitXor
			//Delegate
			//Short
			//Structure
		};
		static int nKeyWordsVB_Num = sizeof( ppszKeyWordsVB ) / sizeof( ppszKeyWordsVB[0] );
		for( i = 0; i < nKeyWordsVB_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 13, ppszKeyWordsVB[i] );
		}


//Jul. 10, 2001 JEPRO 追加
		static const char*	ppszKeyWordsVB2[] = {
			"AppActivate",
			"Beep",
			"BeginTrans",
			"ChDir",
			"ChDrive",
			"Close",
			"CommitTrans",
			"CompactDatabase",
			"Date",
			"DeleteSetting",
			"Erase",
			"FileCopy",
			"FreeLocks",
			"Input",
			"Kill",
			"Load",
			"Lock",
			"Mid",
			"MidB",
			"MkDir",
			"Name",
			"Open",
			"Print",
			"Put",
			"Randomize",
			"RegisterDatabase",
			"RepairDatabase",
			"Reset",
			"RmDir",
			"Rollback",
			"SavePicture",
			"SaveSetting",
			"Seek",
			"SendKeys",
			"SetAttr",
			"SetDataAccessOption",
			"SetDefaultWorkspace",
			"Time",
			"Unload",
			"Unlock",
			"Width",
			"Write",
			"Array",
			"Asc",
			"AscB",
			"Atn",
			"CBool",
			"CByte",
			"CCur",
			"CDate",
			"CDbl",
			"CInt",
			"CLng",
			"CSng",
			"CStr",
			"CVErr",
			"CVar",
			"Choose",
			"Chr",
			"ChrB",
			"Command",
			"Cos",
			"CreateDatabase",
			"CreateObject",
			"CurDir",
			"DDB",
			"Date",
			"DateAdd",
			"DateDiff",
			"DatePart",
			"DateSerial",
			"DateValue",
			"Day",
			"Dir",
			"DoEvents",
			"EOF",
			"Environ",
			"Error",
			"Exp",
			"FV",
			"FileAttr",
			"FileDateTime",
			"FileLen",
			"Fix",
			"Format",
			"FreeFile",
			"GetAllSettings",
			"GetAttr",
			"GetObject",
			"GetSetting",
			"Hex",
			"Hour",
			"IIf",
			"IMEStatus",
			"IPmt",
			"IRR",
			"InStr",
			"Input",
			"Int",
			"IsArray",
			"IsDate",
			"IsEmpty",
			"IsError",
			"IsMissing",
			"IsNull",
			"IsNumeric",
			"IsObject",
			"LBound",
			"LCase",
			"LOF",
			"LTrim",
			"Left",
			"LeftB",
			"Len",
			"LoadPicture",
			"Loc",
			"Log",
			"MIRR",
			"Mid",
			"MidB",
			"Minute",
			"Month",
			"MsgBox",
			"NPV",
			"NPer",
			"Now",
			"Oct",
			"OpenDatabase",
			"PPmt",
			"PV",
			"Partition",
			"Pmt",
			"QBColor",
			"RGB",
			"RTrim",
			"Rate",
			"ReadProperty",
			"Right",
			"RightB",
			"Rnd",
			"SLN",
			"SYD",
			"Second",
			"Seek",
			"Sgn",
			"Shell",
			"Sin",
			"Space",
			"Spc",
			"Sqr",
			"Str",
			"StrComp",
			"StrConv",
			"Switch",
			"Tab",
			"Tan",
			"Time",
			"TimeSerial",
			"TimeValue",
			"Timer",
			"Trim",
			"TypeName",
			"UBound",
			"UCase",
			"Val",
			"VarType",
			"Weekday",
			"Year",
			"Hide",
			"Line",
			"Refresh",
			"Show",
//=========================================================
// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
//=========================================================
//$付き関数各種
			"Dir$",
			"LCase$",
			"Left$",
			"LeftB$",
			"Mid$",
			"MidB$",
			"RightB$",
			"Right$",
			"Space$",
			"Str$",
			"String$",
			"Trim$",
			"UCase$",
//VB5,6の隠し関数
			"VarPtr",
			"StrPtr",
			"ObjPtr",
			"VarPrtArray",
			"VarPtrStringArray"
		};
		static int nKeyWordsVB2_Num = sizeof( ppszKeyWordsVB2 ) / sizeof( ppszKeyWordsVB2[0] );
		for( i = 0; i < nKeyWordsVB2_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 14, ppszKeyWordsVB2[i] );
		}


//Jul. 10, 2001 JEPRO 追加
		static const char*	ppszKeyWordsRTF[] = {
			"\\ansi",
			"\\b",
			"\\bin",
			"\\box",
			"\\brdrb",
			"\\brdrbar",
			"\\brdrdb",
			"\\brdrdot",
			"\\brdrl",
			"\\brdrr",
			"\\brdrs",
			"\\brdrsh",
			"\\brdrt",
			"\\brdrth",
			"\\cell",
			"\\cellx",
			"\\cf",
			"\\chftn",
			"\\clmgf",
			"\\clmrg",
			"\\colortbl",
			"\\deff",
			"\\f",
			"\\fi",
			"\\field",
			"\\fldrslt",
			"\\fonttbl",
			"\\footnote",
			"\\fs",
			"\\i"
			"\\intbl",
			"\\keep",
			"\\keepn",
			"\\li",
			"\\line",
			"\\mac",
			"\\page",
			"\\par",
			"\\pard",
			"\\pc",
			"\\pich",
			"\\pichgoal",
			"\\picscalex",
			"\\picscaley",
			"\\pict",
			"\\picw",
			"\\picwgoal",
			"\\plain",
			"\\qc",
			"\\ql",
			"\\qr",
			"\\ri",
			"\\row",
			"\\rtf",
			"\\sa",
			"\\sb",
			"\\scaps",
			"\\sect",
			"\\sl",
			"\\strike",
			"\\tab",
			"\\tqc",
			"\\tqr",
			"\\trgaph",
			"\\trleft",
			"\\trowd",
			"\\trqc",
			"\\trql",
			"\\tx",
			"\\ul",
			"\\uldb",
			"\\v",
			"\\wbitmap",
			"\\wbmbitspixel",
			"\\wbmplanes",
			"\\wbmwidthbytes",
			"\\wmetafile",
			"bmc",
			"bml",
			"bmr",
			"emc",
			"eml",
			"emr"
		};
		static int nKeyWordsRTF_Num = sizeof( ppszKeyWordsRTF ) / sizeof( ppszKeyWordsRTF[0] );
		for( i = 0; i < nKeyWordsRTF_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 15, ppszKeyWordsRTF[i] );
		}

		//	From Here Sep. 14, 2001 genta
		//	Macro登録の初期化
		MacroRec *mptr = m_pShareData->m_MacroTable;
		for( i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
		//	mptr->m_bEnabled = FALSE;	// Oct 4. 2001 deleted by genta
			mptr->m_szName[0] = '\0';
			mptr->m_szFile[0] = '\0';
		}
		//	To Here Sep. 14, 2001 genta
		

	}else{
		/* オブジェクトがすでに存在する場合 */
		/* ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		//	From Here Oct. 27, 2000 genta
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ){
			//	この共有データ領域は使えない．
			//	ハンドルを解放する
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta
	}
	return true;
}




//	/*
//	|| この実行ファイルのあるディレクトリを返します(例)"c:\\Program Files\\caspy\\"
//	||
//	*/
//	long CShareData::GetModuleDir( char* pszDir, long nSize )
//	{
//		char	szDrive[_MAX_DRIVE];
//		char	szDir[_MAX_DIR];
//		::GetModuleFileName(
//			::GetModuleHandle( NULL ),
//			pszDir, nSize
//		);
//		_splitpath( pszDir, szDrive, szDir, NULL, NULL );
//		strcpy( pszDir, szDrive );
//		strcat( pszDir, szDir );
//		return strlen( pszDir );
//	}




/* KEYDATA配列にデータをセット */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	char*			pszKeyName,
	short			nFuncCode_0,
	short			nFuncCode_1,
	short			nFuncCode_2,
	short			nFuncCode_3,
	short			nFuncCode_4,
	short			nFuncCode_5,
	short			nFuncCode_6,
	short			nFuncCode_7
 )
 {
	pShareData->m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	strcpy( pShareData->m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
 	return;
 }


/* KEYDATA配列にデータをセット */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	char*			pszKeyName
 )
 {
	pShareData->m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	strcpy( pShareData->m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	return;
 }




/* TBBUTTON構造体にデータをセット */
void CShareData::SetTBBUTTONVal(
	TBBUTTON*	ptb,
	int			iBitmap,
	int			idCommand,
	BYTE		fsState,
	BYTE		fsStyle,
	DWORD		dwData,
	int			iString
)
{
/*
typedef struct _TBBUTTON {
	int iBitmap;	// ボタン イメージの 0 から始まるインデックス
	int idCommand;	// ボタンが押されたときに送られるコマンド
	BYTE fsState;	// ボタンの状態--以下を参照
	BYTE fsStyle;	// ボタン スタイル--以下を参照
	DWORD dwData;	// アプリケーション-定義された値
	int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
} TBBUTTON;
*/

 	ptb->iBitmap	= iBitmap;
 	ptb->idCommand	= idCommand;
 	ptb->fsState	= fsState;
 	ptb->fsStyle	= fsStyle;
 	ptb->dwData		= dwData;
 	ptb->iString	= iString;
	return;
 }




/* 共有データ構造体のアドレスを返す */
DLLSHAREDATA* CShareData::GetShareData( const char* pszFilePath, int* pnSettingType )
{

	char	szExt[_MAX_EXT];
	char	szText[256];
	int		i;
	char*	pszToken;
	char*	pszSeps = " ;,";
	BOOL	bFound;
	if( NULL != pnSettingType ){
		*pnSettingType = 0;
		if( NULL != pszFilePath && 0 < (int)strlen( pszFilePath ) ){
			_splitpath( pszFilePath, NULL, NULL, NULL, szExt );
			if( szExt[0] == '.' ){
				char	szExt2[_MAX_EXT];
				strcpy( szExt2, szExt );
				strcpy( szExt, &szExt2[1] );
			}

			bFound = FALSE;
			for( i = 0; i < MAX_TYPES; ++i ){
				strcpy( szText, m_pShareData->m_Types[i].m_szTypeExts );
				pszToken = strtok( szText, pszSeps );
				while( NULL != pszToken ){
					if( 0 == _stricmp( szExt, pszToken ) ){
						bFound = TRUE;
						break;
					}
					pszToken = strtok( NULL, pszSeps );
				}
				if( bFound ){
					break;
				}
			}
			if( i < MAX_TYPES ){
				*pnSettingType = i;
			}else{
				*pnSettingType = 0;
			}
		}
	}
	return m_pShareData;
}





/* 編集ウィンドウリストへの登録 */
BOOL CShareData::AddEditWndList( HWND hWnd )
{
	int		i;
	int		j;
	/* 同じウィンドウハンドルがある場合は先頭に持ってくる */
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( hWnd == m_pShareData->m_pEditArr[i].m_hWnd ){
			break;
		}
	}
	if( i < m_pShareData->m_nEditArrNum ){
		for( j = i; j > 0; j-- ){
			m_pShareData->m_pEditArr[j] = m_pShareData->m_pEditArr[j - 1];
		}
	}else{
		if( m_pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
			/* これ以上登録できない */
			return FALSE;
		}

		for( j = MAX_EDITWINDOWS - 1; j > 0; j-- ){
			m_pShareData->m_pEditArr[j] = m_pShareData->m_pEditArr[j - 1];
		}
		m_pShareData->m_nEditArrNum++;
		if( m_pShareData->m_nEditArrNum > MAX_EDITWINDOWS ){
			m_pShareData->m_nEditArrNum = MAX_EDITWINDOWS;
//#ifdef _DEBUG
//			/* デバッグモニタに出力 */
//			TraceOut( "%s(%d): m_nEditArrNum=%d\n", __FILE__, __LINE__, hWnd, m_pShareData->m_nEditArrNum );
//#endif
			/* これ以上登録できない */
			return FALSE;
		}
	}
	m_pShareData->m_pEditArr[0].m_hWnd = hWnd;

	/* ウィンドウ連番 */

	if( 0 == ::GetWindowLong( hWnd, 4 ) ){
		m_pShareData->m_nSequences++;
		::SetWindowLong( hWnd, 4, (LONG)m_pShareData->m_nSequences );
	}
	return TRUE;
}





/* 編集ウィンドウリストからの削除 */
void CShareData::DeleteEditWndList( HWND hWnd )
{
	int		i;
	int		j;

	/* ウィンドウハンドルの検索 */
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( hWnd == m_pShareData->m_pEditArr[i].m_hWnd ){
			break;
		}
	}
	if( i >= m_pShareData->m_nEditArrNum ){
		return;
	}
	for( j = i; j < m_pShareData->m_nEditArrNum - 1; ++j ){
		m_pShareData->m_pEditArr[j] = m_pShareData->m_pEditArr[j + 1];
	}
	m_pShareData->m_nEditArrNum--;
	return;
}

//!	MRUリストへの登録
/*!
	@param pfi [in] 追加するファイルの情報

	該当ファイルがリムーバブルディスク上にある場合にはMRU Listへの登録は行わない。

	@par History
	2001.03.29 リムーバブルディスク上のファイルを登録しないようにした。(by みく)
*/
void CShareData::AddMRUList( FileInfo* pfi )
{
	if( 0 == strlen( pfi->m_szPath ) ){
		return;
	}
	int		i;
	int		j;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];

	_splitpath( pfi->m_szPath, szDrive, szDir, NULL, NULL );

	//@@@ 2001.03.29 Start by MIK
	char	szDriveType[_MAX_DRIVE+1];	// "A:\"
	long	lngRet;
	char	c;

	c = szDrive[0];
	if( c >= 'a' && c <= 'z' ){
		c = c - ('a' - 'A');
	}
	if( c >= 'A' && c <= 'Z' ){
		sprintf( szDriveType, "%c:\\", c );
		lngRet = GetDriveType( szDriveType );
		if( DRIVE_REMOVABLE	== lngRet
		 || DRIVE_CDROM		== lngRet){
			return;
		}
	}

	char*	pszFolder = new char[_MAX_PATH + 1];
	//@@@ 2001.03.29 End by MIK
	strcpy( pszFolder, szDrive );
	strcat( pszFolder, szDir );
	if( 0 < strlen( pszFolder ) ){
		/* 開いたフォルダ リストへの登録 */
		AddOPENFOLDERList( pszFolder );
	}
	/* 同じPATHがある場合は先頭へ移動させる */
	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		if( 0 == _stricmp( pfi->m_szPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nMRUArrNum ){
		for( j = i; j > 0; j-- ){
			m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];
		}
		m_pShareData->m_fiMRUArr[0] = *pfi;
	}else{
		for( j = MAX_MRU - 1; j > 0; j-- ){
			m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];
		}
		m_pShareData->m_fiMRUArr[0] = *pfi;
		m_pShareData->m_nMRUArrNum++;
		if( m_pShareData->m_nMRUArrNum > MAX_MRU ){
			m_pShareData->m_nMRUArrNum = MAX_MRU;
		}
	}
	delete [] pszFolder;
	return;
}





/* 開いたフォルダ リストへの登録 */
void CShareData::AddOPENFOLDERList( const char* pszFolder )
{
	if( 0 == strlen( pszFolder ) ){
		return;
	}

	int		i;
	int		j;
	char*	pszWork = new char[_MAX_PATH + 1];
	/* 同じPATHがある場合は先頭へ移動させる */
	for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
		if( 0 == _stricmp( pszFolder, m_pShareData->m_szOPENFOLDERArr[i] ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nOPENFOLDERArrNum ){
		for( j = i; j > 0; j-- ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );
		}
		strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );
	}else{
		for( j = MAX_OPENFOLDER - 1; j > 0; j-- ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );
		}
		strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );
		m_pShareData->m_nOPENFOLDERArrNum++;
		if( m_pShareData->m_nOPENFOLDERArrNum > MAX_OPENFOLDER ){
			m_pShareData->m_nOPENFOLDERArrNum = MAX_OPENFOLDER;
		}
	}
	delete [] pszWork;
	return;
}


/*!
	指定された名前のファイルがMRUリストに存在するか調べる。存在するならばファイル情報を返す。

	@param pszPath [in] 検索するファイル名
	@param pfi [out] データが見つかったときにファイル情報を格納する領域。
		呼び出し側で領域をあらかじめ用意する必要がある。
	@retval TRUE  ファイルが見つかった。pfiにファイル情報が格納されている。
	@retval FALSE 指定されたファイルはMRU Listに無い。
*/
BOOL CShareData::IsExistInMRUList( const char* pszPath, FileInfo* pfi )
{
	int		i;
	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		if( 0 == _stricmp( pszPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nMRUArrNum ){
		*pfi = m_pShareData->m_fiMRUArr[i];
		return TRUE;
	}else{
		return FALSE;
	}
}




/* 共有データのロード */
BOOL CShareData::LoadShareData( void )
{
	return ShareData_IO_2( TRUE );
}




/* 共有データの保存 */
void CShareData::SaveShareData( void )
{
	ShareData_IO_2( FALSE );
	return;
}




/* 全編集ウィンドウへ終了要求を出す */
BOOL CShareData::RequestCloseAllEditor( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;

	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return TRUE;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( IsEditWnd( phWndArr[i] ) ){
			/* ウィンドウをアクティブにする */
			/* アクティブにする */
			ActivateFrameWindow( phWndArr[i] );
			/* トレイからエディタへの終了要求 */
			if( !::SendMessage( phWndArr[i], MYWM_CLOSE, 0, 0 ) ){
				delete [] phWndArr;
				return FALSE;
			}
		}
	}
	delete [] phWndArr;
	return TRUE;
}




/* 指定ファイルが開かれているか調べる */
/* 開かれている場合は開いているウィンドウのハンドルも返す */
BOOL CShareData::IsPathOpened( const char* pszPath, HWND* phwndOwner )
{
	int			i;
	FileInfo*	pfi;
	*phwndOwner = NULL;

	/* 現在の編集ウィンドウの数を調べる */
	if( 0 ==  GetEditorWindowsNum() ){
		return FALSE;
	}
	//	Oct. 11, 2001 genta ファイル名判定の stricmpをbccでも期待通り動かすため
	setlocale ( LC_ALL, "C" );
	
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessage( m_pShareData->m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
//			pfi = (FileInfo*)m_pShareData->m_szWork;
			pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
			/* 同一パスのファイルが既に開かれているか */
			if( 0 == _stricmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_pEditArr[i].m_hWnd;
				return TRUE;
			}
		}
	}
	return FALSE;
}





/* 現在の編集ウィンドウの数を調べる */
int CShareData::GetEditorWindowsNum( void )
{
	int		i;
	int		j;
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			j++;
		}
	}
	return j;

}



/* 全編集ウィンドウへメッセージをポストする */
BOOL CShareData::PostMessageToAllEditors(
	UINT		uMsg,		/* ポストするメッセージ */
	WPARAM		wParam,		/* 第1メッセージ パラメータ */
	LPARAM		lParam,		/* 第2メッセージ パラメータ */
	HWND		m_hWndLast	/* 最後に送りたいウィンドウ */
 )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return TRUE;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( NULL != m_hWndLast && phWndArr[i] != m_hWndLast ){
			if( IsEditWnd( phWndArr[i] ) ){
				/* トレイからエディタへメッセージをポスト */
				::PostMessage( phWndArr[i], uMsg, wParam, lParam );
			}
		}
	}
	if( NULL != m_hWndLast && IsEditWnd( m_hWndLast ) ){
		/* トレイからエディタへメッセージをポスト */
		::PostMessage( m_hWndLast, uMsg, wParam, lParam );
	}
	delete [] phWndArr;
	return TRUE;
}


/* 全編集ウィンドウへメッセージを送る */
BOOL CShareData::SendMessageToAllEditors(
	UINT		uMsg,		/* ポストするメッセージ */
	WPARAM		wParam,		/* 第1メッセージ パラメータ */
	LPARAM		lParam,		/* 第2メッセージ パラメータ */
	HWND		m_hWndLast	/* 最後に送りたいウィンドウ */
 )
{
	HWND*	phWndArr;
	int		i;
	int		j;

	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return TRUE;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( NULL != m_hWndLast && phWndArr[i] != m_hWndLast ){
			if( IsEditWnd( phWndArr[i] ) ){
				/* トレイからエディタへメッセージをポスト */
				::SendMessage( phWndArr[i], uMsg, wParam, lParam );
			}
		}
	}
	if( NULL != m_hWndLast && IsEditWnd( m_hWndLast ) ){
		/* トレイからエディタへメッセージをポスト */
		::SendMessage( m_hWndLast, uMsg, wParam, lParam );
	}
	delete [] phWndArr;
	return TRUE;
}


/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL CShareData::IsEditWnd( HWND hWnd )
{
	char	szClassName[64];
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, sizeof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == strcmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}

}

/* 現在開いている編集ウィンドウの配列を返す */
/* 配列の要素数を返す 要素数>0 の場合は呼び出し側で配列をdeleteしてください */
int CShareData::GetOpenedWindowArr( EditNode** ppEditNode, BOOL bSort )
{
	int			nRowNum;
	int			i;
	int			j;
	int			k;
	int			nMinIdx;
	int			nMin;
	HWND*		phWndArr;

	nRowNum = 0;
	// phWndArr = NULL;
	*ppEditNode = NULL;
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			j++;
		}
	}
	if( j > 0 ){
		phWndArr = new HWND[j];
		*ppEditNode = new EditNode[j];
		nRowNum = 0;
		for( i = 0;i < j; ++i ){
			phWndArr[i] = NULL;
		}
		k = 0;
		for( i = 0; i < m_pShareData->m_nEditArrNum && k < j; ++i ){
			if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
				phWndArr[k] = m_pShareData->m_pEditArr[i].m_hWnd;
				k++;
			}
		}
		if( bSort ){
			while( 1 ){
				nMinIdx = 99999;
				nMin = 99999;
				for( i = 0; i < j; ++i ){
					if( phWndArr[i] != NULL &&
						nMin > ::GetWindowLong( phWndArr[i], 4 )
					){
						nMinIdx = i;
						nMin = ::GetWindowLong( phWndArr[i], 4 );
					}
				}
				if( nMinIdx != 99999 ){
					i = nMinIdx;
					(*ppEditNode)[nRowNum].m_nIndex = i;
					(*ppEditNode)[nRowNum].m_hWnd = m_pShareData->m_pEditArr[i].m_hWnd;
					nRowNum++;
					phWndArr[i] = NULL;
				}else{
					break;
				}
			}
		}else{
			for( i = 0; i < k; ++i ){
				(*ppEditNode)[i].m_nIndex = i;
				(*ppEditNode)[i].m_hWnd = phWndArr[i];
			}
			nRowNum = k;
		}

//		LRESULT	lSuccrss;
//		DWORD	dwResult;
//		for( i = 0; i < nRowNum; ++i ){
//			(*ppEditNode)[i].m_szGrepKey[0] = '\0';
//			lSuccrss = ::SendMessageTimeout(
//				(*ppEditNode)[i].m_hWnd, MYWM_AREYOUGREP, 0, 0,
//				SMTO_ABORTIFHUNG | SMTO_NORMAL,
//				3000,
//				&dwResult
//			);
//			if( !lSuccrss ){
//				(*ppEditNode)[i].m_bIsGrep = -1;				/* Grepのウィンドウか */
//			}else{
//				(*ppEditNode)[i].m_bIsGrep = (BOOL)dwResult;	/* Grepのウィンドウか */
//				strcpy( (*ppEditNode)[i].m_szGrepKey, m_szWork );
//			}
//		}
		delete [] phWndArr;
	}
	return nRowNum;
}


/*!
	アウトプットウインドウに出力

	アウトプットウインドウが無ければオープンする
	@param lpFmt [in] 書式指定文字列
*/
void CShareData::TraceOut( LPCTSTR lpFmt, ... )
{

	if( NULL == m_pShareData->m_hwndDebug
	|| !IsEditWnd( m_pShareData->m_hwndDebug )
	){
		CEditApp::OpenNewEditor( NULL, NULL, "-DEBUGMODE", CODE_SJIS, FALSE, true );
#if 0
		//	Jun. 25, 2001 genta OpenNewEditorの同期機能を利用するように変更
		//	2001/06/23 N.Nakatani 窓が出るまでウエイトをかけるように修正
		//アウトプットウインドウが出来るまで5秒ぐらい待つ。
		CRunningTimer wait_timer( NULL );
		while( NULL == m_pShareData->m_hwndDebug && 5000 > wait_timer.Read() ){
			Sleep(1);
		}
		Sleep(10);
		if( NULL == m_pShareData->m_hwndDebug ){
			return;
		}
#endif
		/* 開いているウィンドウをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( m_pShareData->m_hwndDebug );
	}
	va_list argList;
	va_start( argList, lpFmt );
	wvsprintf( m_pShareData->m_szWork, lpFmt, argList );
	va_end( argList );
	::SendMessage( m_pShareData->m_hwndDebug, MYWM_ADDSTRING, 0, 0 );
	return;
}

/*!
	MRUとOPENFOLDERリストの存在チェックなど
	存在しないファイルやフォルダはMRUやOPENFOLDERリストから削除する

	@note 現在は使われていないようだ。
*/
void CShareData::CheckMRUandOPENFOLDERList( void )
{
	int		i;
	int		j;
	/* MRUリスト */
	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		/* 存在チェック */
		if( -1 == _access( m_pShareData->m_fiMRUArr[i].m_szPath, 0 ) ){
			for( j = i + 1; j < m_pShareData->m_nMRUArrNum; ++j ){
				m_pShareData->m_fiMRUArr[j - 1] = m_pShareData->m_fiMRUArr[j];
			}
			i--;
			m_pShareData->m_nMRUArrNum--;
		}
	}
	/* OPENFOLDERリスト */
	for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
		/* 存在チェック */
		if( -1 == _access( m_pShareData->m_szOPENFOLDERArr[i], 0 ) ){
			for( j = i + 1; j < m_pShareData->m_nOPENFOLDERArrNum; ++j ){
				strcpy( m_pShareData->m_szOPENFOLDERArr[j - 1], m_pShareData->m_szOPENFOLDERArr[j] );
			}
			i--;
			m_pShareData->m_nOPENFOLDERArrNum--;
		}
	}
	return;
}

//	/* キー名称のセット */
//	void CShareData::SetKeyNames( DLLSHAREDATA* pShareData )
//	{
//	int		i;
//	i = 0;
//	SetKeyNameArrVal( pShareData, i++, 0,"ダブルクリック" );
//	SetKeyNameArrVal( pShareData, i++, 0,"右クリック" );
//	/* ファンクションキー */
//	SetKeyNameArrVal( pShareData, i++, VK_F1,"F1" );
//	SetKeyNameArrVal( pShareData, i++, VK_F2,"F2" );
//	SetKeyNameArrVal( pShareData, i++, VK_F3,"F3" );
//	SetKeyNameArrVal( pShareData, i++, VK_F4,"F4" );
//	SetKeyNameArrVal( pShareData, i++, VK_F5,"F5" );
//	SetKeyNameArrVal( pShareData, i++, VK_F6,"F6" );
//	SetKeyNameArrVal( pShareData, i++, VK_F7,"F7" );
//	SetKeyNameArrVal( pShareData, i++, VK_F8,"F8" );
//	SetKeyNameArrVal( pShareData, i++, VK_F9,"F9" );
//	SetKeyNameArrVal( pShareData, i++, VK_F10,"F10" );
//	SetKeyNameArrVal( pShareData, i++, VK_F11,"F11" );
//	SetKeyNameArrVal( pShareData, i++, VK_F12,"F12" );
//	SetKeyNameArrVal( pShareData, i++, VK_F13,"F13" );
//	SetKeyNameArrVal( pShareData, i++, VK_F14,"F14" );
//	SetKeyNameArrVal( pShareData, i++, VK_F15,"F15" );
//	SetKeyNameArrVal( pShareData, i++, VK_F16,"F16" );
//	SetKeyNameArrVal( pShareData, i++, VK_F17,"F17" );
//	SetKeyNameArrVal( pShareData, i++, VK_F18,"F18" );
//	SetKeyNameArrVal( pShareData, i++, VK_F19,"F19" );
//	SetKeyNameArrVal( pShareData, i++, VK_F20,"F20" );
//	SetKeyNameArrVal( pShareData, i++, VK_F21,"F21" );
//	SetKeyNameArrVal( pShareData, i++, VK_F22,"F22" );
//	SetKeyNameArrVal( pShareData, i++, VK_F23,"F23" );
//	SetKeyNameArrVal( pShareData, i++, VK_F24,"F24" );
//	/* 特殊キー */
//	SetKeyNameArrVal( pShareData, i++, VK_TAB,"Tab" );
//	SetKeyNameArrVal( pShareData, i++, VK_RETURN,"Enter" );
//	SetKeyNameArrVal( pShareData, i++, VK_ESCAPE,"Esc" );
//	SetKeyNameArrVal( pShareData, i++, VK_BACK,"BackSpace" );
//	SetKeyNameArrVal( pShareData, i++, VK_INSERT,"Insert" );
//	SetKeyNameArrVal( pShareData, i++, VK_DELETE,"Delete" );
//	SetKeyNameArrVal( pShareData, i++, VK_HOME,"Home" );
//	SetKeyNameArrVal( pShareData, i++, VK_END,"End(Help)" );
//	SetKeyNameArrVal( pShareData, i++, VK_LEFT,"←" );
//	SetKeyNameArrVal( pShareData, i++, VK_UP,"↑" );
//	SetKeyNameArrVal( pShareData, i++, VK_RIGHT,"→" );
//	SetKeyNameArrVal( pShareData, i++, VK_DOWN,"↓" );
//	SetKeyNameArrVal( pShareData, i++, VK_PRIOR,"RollDown(PageUp)" );
//	SetKeyNameArrVal( pShareData, i++, VK_NEXT,"RollUp(PageDown)" );
//	SetKeyNameArrVal( pShareData, i++, VK_SPACE,"SpaceBar" );
//	/* 数字 */
//	SetKeyNameArrVal( pShareData, i++, '0', "0" );
//	SetKeyNameArrVal( pShareData, i++, '1', "1" );
//	SetKeyNameArrVal( pShareData, i++, '2', "2" );
//	SetKeyNameArrVal( pShareData, i++, '3', "3" );
//	SetKeyNameArrVal( pShareData, i++, '4', "4" );
//	SetKeyNameArrVal( pShareData, i++, '5', "5" );
//	SetKeyNameArrVal( pShareData, i++, '6', "6" );
//	SetKeyNameArrVal( pShareData, i++, '7', "7" );
//	SetKeyNameArrVal( pShareData, i++, '8', "8" );
//	SetKeyNameArrVal( pShareData, i++, '9', "9" );
//	/* アルファベット */
//	SetKeyNameArrVal( pShareData, i++, 'A', "A" );
//	SetKeyNameArrVal( pShareData, i++, 'B', "B" );
//	SetKeyNameArrVal( pShareData, i++, 'C', "C" );
//	SetKeyNameArrVal( pShareData, i++, 'D', "D" );
//	SetKeyNameArrVal( pShareData, i++, 'E', "E" );
//	SetKeyNameArrVal( pShareData, i++, 'F', "F" );
//	SetKeyNameArrVal( pShareData, i++, 'G', "G" );
//	SetKeyNameArrVal( pShareData, i++, 'H', "H" );
//	SetKeyNameArrVal( pShareData, i++, 'I', "I" );
//	SetKeyNameArrVal( pShareData, i++, 'J', "J" );
//	SetKeyNameArrVal( pShareData, i++, 'K', "K" );
//	SetKeyNameArrVal( pShareData, i++, 'L', "L" );
//	SetKeyNameArrVal( pShareData, i++, 'M', "M" );
//	SetKeyNameArrVal( pShareData, i++, 'N', "N" );
//	SetKeyNameArrVal( pShareData, i++, 'O', "O" );
//	SetKeyNameArrVal( pShareData, i++, 'P', "P" );
//	SetKeyNameArrVal( pShareData, i++, 'Q', "Q" );
//	SetKeyNameArrVal( pShareData, i++, 'R', "R" );
//	SetKeyNameArrVal( pShareData, i++, 'S', "S" );
//	SetKeyNameArrVal( pShareData, i++, 'T', "T" );
//	SetKeyNameArrVal( pShareData, i++, 'U', "U" );
//	SetKeyNameArrVal( pShareData, i++, 'V', "V" );
//	SetKeyNameArrVal( pShareData, i++, 'W', "W" );
//	SetKeyNameArrVal( pShareData, i++, 'X', "X" );
//	SetKeyNameArrVal( pShareData, i++, 'Y', "Y" );
//	SetKeyNameArrVal( pShareData, i++, 'Z', "Z" );
//	/* 記号 */
//	SetKeyNameArrVal( pShareData, i++, 0x00bd, "-" );
//	SetKeyNameArrVal( pShareData, i++, 0x00de, "^" );
//	SetKeyNameArrVal( pShareData, i++, 0x00dc, "\\" );
//	SetKeyNameArrVal( pShareData, i++, 0x00c0, "@" );
//	SetKeyNameArrVal( pShareData, i++, 0x00db, "[" );
//	SetKeyNameArrVal( pShareData, i++, 0x00bb, ";" );
//	SetKeyNameArrVal( pShareData, i++, 0x00ba, ":" );
//	SetKeyNameArrVal( pShareData, i++, 0x00dd, "]" );
//	SetKeyNameArrVal( pShareData, i++, 0x00bc, "," );
//	SetKeyNameArrVal( pShareData, i++, 0x00be, "." );
//	SetKeyNameArrVal( pShareData, i++, 0x00bf, "/" );
//	SetKeyNameArrVal( pShareData, i++, 0x00df, "_" );
//	return;
//	}


/*[EOF]*/
