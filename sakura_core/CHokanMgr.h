//	$Id$
/************************************************************************

	CHokanMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CHokanMgr;

#ifndef _CHOKANMGR_H_
#define _CHOKANMGR_H_

#include "CDialog.h"
#include <windows.h>
#include "CMemory.h"
#include "CShareData.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CHokanMgr : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CHokanMgr();
	~CHokanMgr();

	HWND DoModeless( HINSTANCE, HWND, LPARAM );/* モードレスダイアログの表示 */
	void Hide( void );
	/* 初期化 */
	int CHokanMgr::Search( 
	//	HFONT		hFont,
		POINT*		ppoWin,
		int			nWinHeight,
		int			nColmWidth,
		const char*	pszCurWord,
	//	void*		pcEditView,
		const char* pszHokanFile
	);
	void SetCurKouhoStr( void );
	BOOL DoHokan( int );
	void ChangeView( LPARAM );/* モードレス時：対象となるビューの変更 */


	BOOL OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL OnDestroy( void );
	BOOL OnSize( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int wID );
	BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnLbnDblclk( int wID );
	BOOL OnKillFocus( WPARAM wParam, LPARAM lParam );
//	int OnVKeyToItem( WPARAM wParam, LPARAM lParam );
//	int OnCharToItem( WPARAM wParam, LPARAM lParam );
//	BOOL OnNextDlgCtl( WPARAM, LPARAM );

	int KeyProc( WPARAM, LPARAM );

    
//	HFONT			m_hFont;
//	HFONT			m_hFontOld;
	CMemory			m_cmemCurWord;
	CMemory*		m_pcmemKouho;
	int				m_nKouhoNum;

	int				m_nCurKouhoIdx;
	char*			m_pszCurKouho;

	POINT			m_poWin;
	int				m_nWinHeight;
	int				m_nColmWidth;
	int				m_pnDx[256 + 1];		/* 文字列描画用文字幅配列 */
//	void*			m_pcEditView;
	int				m_bTimerFlag;

protected:
	/*
	||  実装ヘルパ関数
	*/

};



///////////////////////////////////////////////////////////////////////
#endif /* _CHOKANMGR_H_ */

/*[EOF]*/
