//	$Id$
/*!	@file
	@brief ImageList�̎�舵��

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

//! ImageList�̊Ǘ�
class SAKURA_CORE_API CImageListMgr {
public:

	//	constructor
	CImageListMgr() : m_hList( NULL ), m_cx( 16 ), m_cy( 16 ){}
	~CImageListMgr();

	bool Create(HINSTANCE hInstance, HWND hWnd);	//	����
	
	/*! @brief �A�C�R���̕`��
	
		�w�肳�ꂽDC�̎w�肳�ꂽ���W�ɃA�C�R����`�悷��D
	
		@param index [in] �`�悷��A�C�R���ԍ�
		@param dc [in] �`�悷��Device Context
		@param x [in] �`�悷��X���W
		@param y [in] �`�悷��Y���W
		@param fstyle [in] �`��X�^�C��
	*/
	bool Draw(int index, HDC dc, int x, int y, int fstyle) const	//	�`��
		{ return m_hList == NULL ? false : (ImageList_Draw( m_hList, index, dc, x, y, fstyle) != 0 ); }
	
	//! �A�C�R������Ԃ�
	int  Count(void) const	//	�A�C�R����
		{ return m_hList == NULL ? 0 : ImageList_GetImageCount( m_hList ); }
	
	//! �A�C�R���̕�
	int  cx(void) const { return m_cx; }
	//! �A�C�R���̍���
	int  cy(void) const { return m_cy; }
	
	/*!
		�C���[�W��ToolBar�ւ̓o�^
	
		@param hToolBar [in] �o�^����ToolBar
		@param id [in] �o�^����擪�A�C�R���ԍ�
	*/
	HIMAGELIST  SetToolBarImages(HWND hToolBar, int id = 0) const {
		return (HIMAGELIST)::SendMessage( hToolBar, TB_SETIMAGELIST,
			(WPARAM)id, (LPARAM)m_hList );
	}

	/*!	@brief Image List�̃n���h��

		IMAGE LIST�ւ̃n���h�����K�v��API�̂��߂�(ToolBar�Ȃ�)
	*/
	HIMAGELIST GetHandle(void) const { return m_hList; }

protected:
	HIMAGELIST m_hList;	//!<	ImageList�̃n���h��
	int m_cx;			//!<	width of icon
	int m_cy;			//!<	height of icon

};

#endif


/*[EOF]*/
