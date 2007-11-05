/*!	@file
	@brief GREPダイアログボックス

	@author Norio Nakatani
	@date 1998.09/07  新規作成
	@date 1999.12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgGrep;

#ifndef _CDLGGREP_H_
#define _CDLGGREP_H_

#include "CDialog.h"

//! GREPダイアログボックス
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
	int DoModal( HINSTANCE, HWND, const TCHAR* );	/* モーダルダイアログの表示 */
//	HWND DoModeless( HINSTANCE, HWND, const char* );	/* モードレスダイアログの表示 */


	BOOL		m_bSubFolder;/*!< サブフォルダからも検索する */
	BOOL		m_bFromThisText;/*!< この編集中のテキストから検索する */

	SSearchOption	m_sSearchOption;	//!< 検索オプション
	/*
	int			m_bRegularExp;	//!< 正規表現
	int			m_bLoHiCase;	//!< 英大文字と小文字を区別する
	BOOL		m_bWordOnly;	//!< 単語単位で検索
	*/

	BOOL		m_bGrepOutputLine;	/*!< 行を出力するか該当部分だけ出力するか */

	ECodeType	m_nGrepCharSet;		/*!< 文字コードセット */

	int			m_nGrepOutputStyle;			/*!< Grep: 出力形式 */
	EDIT_CHAR	m_szText[_MAX_PATH + 1];	/*!< 検索文字列 */
	SFilePath	m_szFile;					/*!< 検索ファイル */
	SFilePath	m_szFolder;					/*!< 検索フォルダ */
	SFilePath	m_szCurrentFilePath;
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGGREP_H_ */


/*[EOF]*/
