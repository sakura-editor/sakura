//	$Id$
/*
	CImageListMgr	ImageList�������N���X

	Author: genta
	Date:	Oct. 11, 2000
	Copyright (C) 1998-2000, genta

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
	
	bool Create(HINSTANCE hInstance, HWND hWnd);	//	����
	bool Draw(int index, HDC dc, int x, int y, int fstyle) const	//	�`��
		{ return m_hList == NULL ? false : (ImageList_Draw( m_hList, index, dc, x, y, fstyle) != 0 ); }
	int  Count(void) const	//	�A�C�R����
		{ return m_hList == NULL ? 0 : ImageList_GetImageCount( m_hList ); }
	int  cx(void) const { return m_cx; }
	int  cy(void) const { return m_cy; }
	HIMAGELIST  SetToolBarImages(HWND hToolBar, int id = 0) const {
		return (HIMAGELIST)::SendMessage( hToolBar, TB_SETIMAGELIST,
			(WPARAM)id, (LPARAM)m_hList );
	}

	//	IMAGE LIST�ւ̃n���h�����K�v��API�̂��߂�(ToolBar�Ȃ�)
	HIMAGELIST GetHandle(void) const { return m_hList; }

protected:
	HIMAGELIST m_hList;	//	ImageList�̃n���h��
	int m_cx;				//	width of icon
	int m_cy;				//	height of icon

};

#endif	/* EOF */
