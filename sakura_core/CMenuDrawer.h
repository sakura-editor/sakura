//	$Id$
/*!	@file
	@brief メニュー管理＆表示

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CMenuDrawer;

#ifndef _CMENUDRAWER_H_
#define _CMENUDRAWER_H_

#include <windows.h>
#include "CShareData.h"
#include "CMemory.h"// 2002/2/10 aroka
class CImageListMgr;// 2002/2/10 aroka

//#define MAX_MENUPOS	10
#define MAX_MENUITEMS	300

//ツールバーの拡張	//@@@ 2002.06.15 MIK
#define TBSTYLE_COMBOBOX	((BYTE)0x40)	//ツールバーにコンボボックス
#ifndef TBSTYLE_DROPDOWN	//IE3以上
	#define TBSTYLE_DROPDOWN	0x0008
#endif
#ifndef TB_SETEXTENDEDSTYLE	//IE4以上
	#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)  // For TBSTYLE_EX_*
#endif
#ifndef TBSTYLE_EX_DRAWDDARROWS	//IE4以上
	#define TBSTYLE_EX_DRAWDDARROWS 0x00000001
#endif

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief メニュー表示＆管理

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class SAKURA_CORE_API CMenuDrawer
{
public:
	/*
	||  Constructors
	*/
	CMenuDrawer();
	~CMenuDrawer();
	void Create( HINSTANCE, HWND, CImageListMgr* );


	/*
	||  Attributes & Operations
	*/
	void ResetContents( void );
	//void MyAppendMenu( HMENU , int , int , const char*, BOOL = TRUE );	/* メニュー項目を追加 */
	void MyAppendMenu( HMENU , int , int , const char*, BOOL = TRUE, int = -1 );	/* メニュー項目を追加 */	//お気に入り	//@@@ 2003.04.08 MIK
	int MeasureItem( int, int* );	/* メニューアイテムの描画サイズを計算 */
	void DrawItem( DRAWITEMSTRUCT* );	/* メニューアイテム描画 */
	int Find( int nFuncID );
	const char* GetLabel( int nFuncID );
	char GetAccelCharFromLabel( const char* pszLabel );
	LRESULT OnMenuChar( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
//@@@ 2002.01.03 YAZAKI 不使用のため
//	static void MyBitBlt( HDC drawdc, int nXDest, int nYDest, int nWidth,
//							int nHeight, HBITMAP bmp, int nXSrc, int nYSrc, COLORREF, COLORREF);
//	void DitherBlt2( HDC drawdc, int nXDest, int nYDest, int nWidth,
//						int nHeight, HBITMAP bmp, int nXSrc, int nYSrc);


	DLLSHAREDATA*	m_pShareData;

	HINSTANCE		m_hInstance;
	HWND			m_hWndOwner;

	int				m_nMenuItemNum;

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	TBBUTTON		m_tbMyButton[MAX_TOOLBARBUTTONS+1];	/* ツールバーのボタン +1はセパレータ */
	int				m_nMyButtonNum;

	int				m_nMenuItemBitmapIdxArr[MAX_MENUITEMS];
	int				m_nMenuItemFuncArr[MAX_MENUITEMS];
	CMemory			m_cmemMenuItemStrArr[MAX_MENUITEMS];
	int				m_nMenuHeight;
	HFONT			m_hFontMenu;
	HFONT			m_hFontMenuUndelLine;
//@@@ 2002.01.03 YAZAKI 不使用のため
//	int				m_nMaxTab;
//	int				m_nMaxTabLen;

	//	Oct. 16, 2000 genta
	CImageListMgr	*m_pcIcons;	//	Image List

protected:
	/*
	||  実装ヘルパ関数
	*/
	int GetData( void );	/* ダイアログデータの取得 */

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	void SetTBBUTTONVal( TBBUTTON*, int, int, BYTE, BYTE, DWORD_PTR, INT_PTR );	/* TBBUTTON構造体にデータをセット */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CMENUDRAWER_H_ */


/*[EOF]*/
