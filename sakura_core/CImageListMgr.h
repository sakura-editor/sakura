//	$Id$
/*!	@file
	@brief ImageListの取り扱い

	@author genta
	@date Oct. 11, 2000 genta
	$Revision$

*/
/*
	Copyright (C) 2000-2001, genta

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
#include "commctrl.h"

//! ImageListの管理
class SAKURA_CORE_API CImageListMgr {
public:

	//	constructor
	CImageListMgr() : m_hList( NULL ), m_cx( 16 ), m_cy( 16 ){}
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
		{ return m_hList == NULL ? false : (ImageList_Draw( m_hList, index, dc, x, y, fstyle) != 0 ); }
	
	//! アイコン数を返す
	int  Count(void) const	//	アイコン数
		{ return m_hList == NULL ? 0 : ImageList_GetImageCount( m_hList ); }
	
	//! アイコンの幅
	int  cx(void) const { return m_cx; }
	//! アイコンの高さ
	int  cy(void) const { return m_cy; }
	
	/*!
		イメージのToolBarへの登録
	
		@param hToolBar [in] 登録するToolBar
		@param id [in] 登録する先頭アイコン番号
	*/
	HIMAGELIST  SetToolBarImages(HWND hToolBar, int id = 0) const {
		return (HIMAGELIST)::SendMessage( hToolBar, TB_SETIMAGELIST,
			(WPARAM)id, (LPARAM)m_hList );
	}

	/*!	@brief Image Listのハンドル

		IMAGE LISTへのハンドルが必要なAPIのために(ToolBarなど)
	*/
	HIMAGELIST GetHandle(void) const { return m_hList; }

protected:
	HIMAGELIST m_hList;	//!<	ImageListのハンドル
	int m_cx;			//!<	width of icon
	int m_cy;			//!<	height of icon

};

#endif


/*[EOF]*/
