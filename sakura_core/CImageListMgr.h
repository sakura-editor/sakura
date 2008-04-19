/*!	@file
	@brief ImageList�̎�舵��

	@author genta
	@date Oct. 11, 2000 genta
*/
/*
	Copyright (C) 2000-2003, genta
	Copyright (C) 2003, Moca

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
//#include <commctrl.h> 2003.07.21 genta �s�v

/*! @brief ImageList�̊Ǘ�

	�A�C�R���C���[�W���Ǘ�����

	@author genta

	@date 2003.07.21 genta �C���[�W���X�g�̎g�p����߂��D�����Tool Bar��Owner Draw���g���D
		�A�N�Z�X���\�b�h�̑啔���ɉ����Ď������ύX����Ă���D

	@note �C���[�W���X�g�ւ̃r�b�g�}�b�v�̓o�^��Bitblt���s��������
		VAIO�Ƒ����������u���[�X�N���[�����������Ă����D
		�܂��C���[�W���X�g��IE3�ȑO��common component�Ɋ܂܂�Ă��Ȃ����߂�
		����Win95�ŃC���[�W�̕\�����ł��Ȃ������D������������邽�߂�ImageList�̎g�p����߂�
		�����̓Ǝ��`��ɖ߂����D
*/
class SAKURA_CORE_API CImageListMgr {
public:

	//	constructor
	CImageListMgr() : m_cx( 16 ), m_cy( 16 ), m_hIconBitmap( NULL ), m_cTrans( RGB( 0, 0, 0 )) {}
	~CImageListMgr();

	bool Create(HINSTANCE hInstance);	//	����
	
	/*! @brief �A�C�R���̕`��
	
		�w�肳�ꂽDC�̎w�肳�ꂽ���W�ɃA�C�R����`�悷��D
	
		@param index [in] �`�悷��A�C�R���ԍ�
		@param dc [in] �`�悷��Device Context
		@param x [in] �`�悷��X���W
		@param y [in] �`�悷��Y���W
		@param fstyle [in] �`��X�^�C��
		
		@date 2003.09.06 genta �w�i�̓��ߏ����ɔ����w�i�F�w��폜
	*/
	bool Draw(int index, HDC dc, int x, int y, int fstyle ) const	//	�`��
	;
	
	//! �A�C�R������Ԃ�
	int  Count(void) const;	//	�A�C�R����
	
	//! �A�C�R���̕�
	int  cx(void) const { return m_cx; }
	//! �A�C�R���̍���
	int  cy(void) const { return m_cy; }
	
	/*!
		�C���[�W��ToolBar�ւ̓o�^
	
		@param hToolBar [in] �o�^����ToolBar
		@param id [in] �o�^����擪�A�C�R���ԍ�

		@date 2003.07.21 genta �����ł͉����s��Ȃ����C�󂯎M�����c���Ă���
		@date 2003.07.21 genta �߂�^��void�ɕύX
	*/
	void  SetToolBarImages(HWND hToolBar, int id = 0) const {}

protected:
	int m_cx;			//!<	width of icon
	int m_cy;			//!<	height of icon
	/*!	@brief ���ߐF
	
		�`������O�ōs�����߁C���ߐF���o���Ă����K�v������D
		@date 2003.07.21 genta
	*/
	COLORREF m_cTrans;
	
	/*! �A�C�R���p�r�b�g�}�b�v��ێ�����
		@date 2003.07.21 genta
	*/
	HBITMAP m_hIconBitmap;

	//	�I���W�i���e�L�X�g�G�f�B�^����̕`��֐�
	//	2003.08.27 Moca �w�i�𓧉ߏ�������̂Ŕw�i�F�̎w��͕s�v��
	void MyBitBlt( HDC drawdc, int nXDest, int nYDest, 
		int nWidth, int nHeight, HBITMAP bmp, int nXSrc, int nYSrc ) const;
	void DitherBlt2( HDC drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, HBITMAP bmp, int nXSrc, int nYSrc ) const;

};

#endif



