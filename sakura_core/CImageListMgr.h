//	$Id$
/*!	@file
	CImageListMgr ImageListを扱うクラス

	@author genta
	@date Oct. 11, 2000 genta
	$Revision$

*/
/*
	Copyright (C) 2000-2001, genta

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _CIMAGELIST_MGR_H_
#define _CIMAGELIST_MGR_H_

#include "global.h"
#include "commctrl.h"

class SAKURA_CORE_API CImageListMgr {
public:

	//	constructor
	CImageListMgr() : m_hList( NULL ), m_cx( 16 ), m_cy( 16 ){}
	~CImageListMgr();

	bool Create(HINSTANCE hInstance, HWND hWnd);	//	生成
	bool Draw(int index, HDC dc, int x, int y, int fstyle) const	//	描画
		{ return m_hList == NULL ? false : (ImageList_Draw( m_hList, index, dc, x, y, fstyle) != 0 ); }
	int  Count(void) const	//	アイコン数
		{ return m_hList == NULL ? 0 : ImageList_GetImageCount( m_hList ); }
	int  cx(void) const { return m_cx; }
	int  cy(void) const { return m_cy; }
	HIMAGELIST  SetToolBarImages(HWND hToolBar, int id = 0) const {
		return (HIMAGELIST)::SendMessage( hToolBar, TB_SETIMAGELIST,
			(WPARAM)id, (LPARAM)m_hList );
	}

	//	IMAGE LISTへのハンドルが必要なAPIのために(ToolBarなど)
	HIMAGELIST GetHandle(void) const { return m_hList; }

protected:
	HIMAGELIST m_hList;	//	ImageListのハンドル
	int m_cx;				//	width of icon
	int m_cy;				//	height of icon

};

#endif


/*[EOF]*/
