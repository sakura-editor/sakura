//	$Id$
/************************************************************************

	CDlgCompare.h
	Copyright (C) 1998-2000, Norio Nakatani

663552
************************************************************************/

class CDlgCompare;

#ifndef _CDLGCOMPARE_H_
#define _CDLGCOMPARE_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgCompare : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCompare();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const char*, BOOL, char*, HWND* );	/* モーダルダイアログの表示 */

	const char*		m_pszPath;
	BOOL			m_bIsModified;
	char*			m_pszComparePath;
	HWND*			m_phwndCompareWnd;
	BOOL			m_bCompareAndTileHorz;/* 左右に並べて表示 */
//	BOOL			m_bCompareAndTileHorz;/* 左右に並べて表示 */	//Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked( int );

	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCOMPARE_H_ */

/*[EOF]*/
