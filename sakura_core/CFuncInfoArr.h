//	$Id$
/************************************************************************

	CFuncInfoArr.h

	�A�E�g���C����́@�f�[�^�z��
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/23  �V�K�쐬

************************************************************************/

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
