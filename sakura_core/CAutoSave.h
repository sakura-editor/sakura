//	$Id$
//
//	CAutoSave.h
/*!	@file
	�t�@�C���̎����ۑ�

	@author genta
	@version $Revision$
	@date 2000
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
#ifndef __CAUTOSAVE_H_
#define __CAUTOSAVE_H_
#include <windows.h>
#include "global.h"

//! �����~���b�ɕϊ����邽�߂̌W��
const int MSec2Min = 1000 * 60;
/*! @class CPassiveTimer CAutoSave.h
	���������̌o�ߎ��Ԃ��ݒ�Ԋu���߂������ǂ����𔻒肷��B
	�p�ɂɌĂяo�����^�C�}�[�����ɕʂ̏ꏊ�ɂ���Ƃ��A��������Ԋu���L����
	�Ԋu�̌��������v������Ȃ��p�r�ɗ��p�\�B
	�t�@�C���̎����ۑ��Ŏg���Ă���B
	@author genta
*/
class SAKURA_CORE_API CPassiveTimer {
public:
	/*!
		�����l�͊Ԋu1msec�Ń^�C�}�[�͖����B
	*/
	CPassiveTimer() : nInterval(1), bEnabled(false){ Reset(); }

	//!	���ԊԊu�̐ݒ�
	void SetInterval(int m);

	//!	���ԊԊu�̎擾
	int GetInterval(void) const {return nInterval / MSec2Min; }

	//!	������̃��Z�b�g
	void Reset(void){ nLastTick = ::GetTickCount(); }

	//!	�L���^�����̐ݒ�
	void Enable(bool flag);
	//!	�L���^�����̓ǂݏo��
	bool IsEnabled(void) const { return bEnabled; }

	//!	�K�莞�ԂɒB�������ǂ����̔���
	bool CheckAction(void);

private:
	DWORD nLastTick;	//!<	�Ō�Ƀ`�F�b�N�����Ƃ��̎��� (GetTickCount()�Ŏ擾��������)
	int  nInterval;		//!<	Action�Ԋu (��)
	bool bEnabled;	//!<	�L�����ǂ���
};
#endif


/*[EOF]*/
