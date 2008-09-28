/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

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
	void AppendData( CLogicInt, CLayoutInt, const TCHAR*, int, int nDepth = 0 );	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2002.04.01 YAZAKI �[������*/
	void AppendData( CLogicInt, CLayoutInt, const NOT_TCHAR*, int, int nDepth = 0 );	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2002.04.01 YAZAKI �[������*/
	int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* �z��v�f����Ԃ� */
	void Empty( void );
	void DUMP( void );




public:
	SFilePath	m_szFilePath;	/*!< ��͑Ώۃt�@�C���� */
private:
	int			m_nFuncInfoArrNum;	/*!< �z��v�f�� */
	CFuncInfo**	m_ppcFuncInfoArr;	/*!< �z�� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */



