//	$Id$
/*!	@file
	@brief ImageListの取り扱い

	@author genta
	@date Oct. 11, 2000 genta
	$Revision$

*/
/*
	Copyright (C) 2000-2003, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _CIMAGELIST_MGR_H_
#define _CIMAGELIST_MGR_H_

#include "global.h"
//#include <commctrl.h> 2003.07.21 genta 不要

/*! @brief ImageListの管理

	アイコンイメージを管理する

	@author genta

	@date 2003.07.21 genta イメージリストの使用をやめた．代わりにTool BarのOwner Drawを使う．
		アクセスメソッドの大部分に於いて実装が変更されている．

	@note イメージリストへのビットマップの登録でBitbltを行う部分が
		VAIOと相性が悪くブルースクリーンが発生していた．
		またイメージリストがIE3以前のcommon componentに含まれていないために
		初期Win95でイメージの表示ができなかった．それらを回避するためにImageListの使用をやめて
		当初の独自描画に戻した．
*/
class SAKURA_CORE_API CImageListMgr {
public:

	//	constructor
	CImageListMgr() : m_cx( 16 ), m_cy( 16 ), m_hIconBitmap( NULL ), m_cTrans( RGB( 0, 0, 0 )) {}
	~CImageListMgr();

	bool Create(HINSTANCE hInstance, HWND hWnd);	//	生成
	
	/*! @brief アイコンの描画
	
		指定されたDCの指定された座標にアイコンを描画する．
	
		@param index [in] 描画するアイコン番号
		@param dc [in] 描画するDevice Context
		@param x [in] 描画するX座標
		@param y [in] 描画するY座標
		@param fstyle [in] 描画スタイル
	*/
	bool Draw(int index, HDC dc, int x, int y, int fstyle) const	//	描画
	;
	
	//! アイコン数を返す
	int  Count(void) const;	//	アイコン数
	
	//! アイコンの幅
	int  cx(void) const { return m_cx; }
	//! アイコンの高さ
	int  cy(void) const { return m_cy; }
	
	/*!
		イメージのToolBarへの登録
	
		@param hToolBar [in] 登録するToolBar
		@param id [in] 登録する先頭アイコン番号

		@date 2003.07.21 genta ここでは何も行わないが，受け皿だけ残しておく
		@date 2003.07.21 genta 戻り型をvoidに変更
	*/
	void  SetToolBarImages(HWND hToolBar, int id = 0) const {}

protected:
	int m_cx;			//!<	width of icon
	int m_cy;			//!<	height of icon
	/*!	@brief 透過色
	
		描画を自前で行うため，透過色を覚えておく必要がある．
		@date 2003.07.21 genta
	*/
	COLORREF m_cTrans;
	
	/*! アイコン用ビットマップを保持する
		@date 2003.07.21 genta
	*/
	HBITMAP m_hIconBitmap;

	//	オリジナルテキストエディタからの描画関数
	void MyBitBlt( HDC drawdc, int nXDest, int nYDest, 
		int nWidth, int nHeight, HBITMAP bmp, int nXSrc, int nYSrc,
		COLORREF colBkColor ) const;
	void DitherBlt2( HDC drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, HBITMAP bmp, int nXSrc, int nYSrc) const;

};

#endif


/*[EOF]*/
