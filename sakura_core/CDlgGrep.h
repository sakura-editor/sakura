//	$Id$
/************************************************************************

	CDlgGrep.h

    UPDATE: 1999.12/05 再作成
    CREATE: 1998.09/07  新規作成
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgGrep;

#ifndef _CDLGGREP_H_
#define _CDLGGREP_H_

#include "CDialog.h"
//#include <windows.h>
//#include "CShareData.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgGrep : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgGrep();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, const char* );	/* モーダルダイアログの表示 */
//	HWND DoModeless( HINSTANCE, HWND, const char* );	/* モードレスダイアログの表示 */


	BOOL		m_bSubFolder;/* サブフォルダからも検索する */
	BOOL		m_bFromThisText;/* この編集中のテキストから検索する */
	int			m_bLoHiCase;	/* 英大文字と小文字を区別する */
	int			m_bRegularExp;	/* 正規表現 */
	BOOL		m_bKanjiCode_AutoDetect;	/* 文字コード自動判別 */
	BOOL		m_bGrepOutputLine;	/* 行を出力するか該当部分だけ出力するか */
	int			m_nGrepOutputStyle;				/* Grep: 出力形式 */
	char		m_szText[_MAX_PATH + 1];	/* 検索文字列 */
	char		m_szFile[_MAX_PATH + 1];	/* 検索ファイル */
	char		m_szFolder[_MAX_PATH + 1];	/* 検索フォルダ */
	char		m_szCurrentFilePath[_MAX_PATH + 1];
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );

	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGGREP_H_ */

/*[EOF]*/
