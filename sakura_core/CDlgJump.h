//	$Id$
/************************************************************************

	CDlgJump.h

    UPDATE: 1999.12/05  再作成
    CREATE: 1998.05/31  新規作成
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

class CDlgJump;

#ifndef _CDLGJUMP_H_
#define _CDLGJUMP_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgJump : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgJump();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, BOOL );	/* モーダルダイアログの表示 */

	int			m_nLineNum;		/* 行番号 */
	BOOL		m_bPLSQL;		/* PL/SQLソースの有効行か */
	int			m_nPLSQL_E1;
	int			m_nPLSQL_E2;
	BOOL		m_bLineNumIsCRLF;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnNotify( WPARAM,  LPARAM );	//	Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGJUMP_H_ */

/*[EOF]*/
