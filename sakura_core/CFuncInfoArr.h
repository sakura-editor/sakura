//	$Id$
/*!	@file
	�A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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

class CFuncInfoArr;

#ifndef _CFUNCINFOARR_H_
#define _CFUNCINFOARR_H_

#include "CFuncInfo.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �A���h�D�E���h�D�o�b�t�@ */
class CFuncInfoArr {
	public:
		CFuncInfoArr();	/* CFuncInfoArr�N���X�\�z */
		~CFuncInfoArr();	/* CFuncInfoArr�N���X���� */
		CFuncInfo* GetAt( int );	/* 0<=�̎w��ԍ��̃f�[�^��Ԃ� */
		void AppendData( CFuncInfo* );	/* �z��̍Ō�Ƀf�[�^��ǉ����� */
		void AppendData( int, int, char*, int );	/* �z��̍Ō�Ƀf�[�^��ǉ����� */
		int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* �z��v�f����Ԃ� */
		void Empty( void );
		void DUMP( void );



		char		m_szFilePath[_MAX_PATH + 1];	/* ��͑Ώۃt�@�C���� */
	private:
		int			m_nFuncInfoArrNum;	/* �z��v�f�� */
		CFuncInfo**	m_ppcFuncInfoArr;	/* �z�� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */


/*[EOF]*/
