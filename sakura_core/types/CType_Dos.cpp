/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"

/* MS-DOSバッチファイル */
void CType_Dos::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("MS-DOSバッチファイル") );
	wcscpy( pType->m_szTypeExts, LTEXT("bat") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("REM "), -1 );	/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 7;					/* キーワードセット */
}

const WCHAR* g_ppszKeywordsBAT[] = {
	LTEXT("PATH"),
	LTEXT("PROMPT"),
	LTEXT("TEMP"),
	LTEXT("TMP"),
	LTEXT("TZ"),
	LTEXT("CONFIG"),
	LTEXT("COMSPEC"),
	LTEXT("DIRCMD"),
	LTEXT("COPYCMD"),
	LTEXT("winbootdir"),
	LTEXT("windir"),
	LTEXT("DIR"),
	LTEXT("CALL"),
	LTEXT("CHCP"),
	LTEXT("RENAME"),
	LTEXT("REN"),
	LTEXT("ERASE"),
	LTEXT("DEL"),
	LTEXT("TYPE"),
	LTEXT("REM"),
	LTEXT("COPY"),
	LTEXT("PAUSE"),
	LTEXT("DATE"),
	LTEXT("TIME"),
	LTEXT("VER"),
	LTEXT("VOL"),
	LTEXT("CD"),
	LTEXT("CHDIR"),
	LTEXT("MD"),
	LTEXT("MKDIR"),
	LTEXT("RD"),
	LTEXT("RMDIR"),
	LTEXT("BREAK"),
	LTEXT("VERIFY"),
	LTEXT("SET"),
	LTEXT("EXIT"),
	LTEXT("CTTY"),
	LTEXT("ECHO"),
	LTEXT("@ECHO"),	//Oct. 31, 2000 JEPRO '@' を強調可能にしたので追加
	LTEXT("LOCK"),
	LTEXT("UNLOCK"),
	LTEXT("GOTO"),
	LTEXT("SHIFT"),
	LTEXT("IF"),
	LTEXT("FOR"),
	LTEXT("DO"),	//Nov. 2, 2000 JEPRO 追加
	LTEXT("IN"),	//Nov. 2, 2000 JEPRO 追加
	LTEXT("ELSE"),	//Nov. 2, 2000 JEPRO 追加 Win2000で使える
	LTEXT("CLS"),
	LTEXT("TRUENAME"),
	LTEXT("LOADHIGH"),
	LTEXT("LH"),
	LTEXT("LFNFOR"),
	LTEXT("ON"),
	LTEXT("OFF"),
	LTEXT("NOT"),
	LTEXT("ERRORLEVEL"),
	LTEXT("EXIST"),
	LTEXT("NUL"),
	LTEXT("CON"),
	LTEXT("AUX"),
	LTEXT("COM1"),
	LTEXT("COM2"),
	LTEXT("COM3"),
	LTEXT("COM4"),
	LTEXT("PRN"),
	LTEXT("LPT1"),
	LTEXT("LPT2"),
	LTEXT("LPT3"),
	LTEXT("CLOCK"),
	LTEXT("CLOCK$"),
	LTEXT("CONFIG$"),
	LTEXT("SETLOCAL"),
	LTEXT("ENDLOCAL")
};
int g_nKeywordsBAT = int(std::size(g_ppszKeywordsBAT));
