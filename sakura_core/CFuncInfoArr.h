//	$Id$
/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CFuncInfoArr;

#ifndef _CFUNCINFOARR_H_
#define _CFUNCINFOARR_H_

#include "CFuncInfo.h"

//! �A�E�g���C����� �f�[�^�z��
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



		char		m_szFilePath[_MAX_PATH + 1];	/*!< ��͑Ώۃt�@�C���� */
	private:
		int			m_nFuncInfoArrNum;	/*!< �z��v�f�� */
		CFuncInfo**	m_ppcFuncInfoArr;	/*!< �z�� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */


/*[EOF]*/
