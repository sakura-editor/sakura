/*!	@file
	@brief �����v�J�[�\��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CWaitCursor;

#ifndef _CWAITCURSOR_H_
#define _CWAITCURSOR_H_


#include <windows.h>


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//!	�����v�J�[�\���N���X
/*!
	�I�u�W�F�N�g�̑������Ă���ԃJ�[�\���`��������v�ɂ���D
	�I�u�W�F�N�g���j�������ƃJ�[�\���`��͌��ɖ߂�
*/
class CWaitCursor
{
public:
	/*
	||  Constructors
	*/
	CWaitCursor( HWND );
	~CWaitCursor();
private: // 2002/2/10 aroka
	HCURSOR	m_hCursor;
	HCURSOR	m_hCursorOld;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CWAITCURSOR_H_ */



