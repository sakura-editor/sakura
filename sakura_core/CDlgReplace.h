//	$Id$
/************************************************************************

	CDlgReplace.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/10/2  新規作成
************************************************************************/

class CDlgReplace;

#ifndef _CDLGREPLACE_H_
#define _CDLGREPLACE_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgReplace : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgReplace();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( HINSTANCE, HWND, LPARAM, BOOL );	/* モーダルダイアログの表示 */
	void ChangeView( LPARAM );	/* モードレス時：置換・検索対象となるビューの変更 */

	int				m_bLoHiCase;	/* 英大文字と英小文字を区別する */
	int				m_bWordOnly;	/* 一致する単語のみ検索する */
	int				m_bRegularExp;	/* 正規表現 */
	char			m_szText[_MAX_PATH + 1];	/* 検索文字列 */
	char			m_szText2[_MAX_PATH + 1];	/* 置換後文字列 */
	BOOL			m_bSelectedArea;	/* 選択範囲内置換 */
	int				m_bNOTIFYNOTFOUND;				/* 検索／置換  見つからないときメッセージを表示 */
	int				m_nSettingType;
	BOOL			m_bSelected;	/* テキスト選択中か */
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
#endif /* _CDLGREPLACE_H_ */

/*[EOF]*/
