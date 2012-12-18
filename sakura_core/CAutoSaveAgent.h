/*!	@file
	@brief �t�@�C���̎����ۑ�

	@author genta
	@date 2000
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
#ifndef __CAUTOSAVE_H_
#define __CAUTOSAVE_H_
#include <windows.h>
#include "_main/global.h"
#include "doc/CDocListener.h"

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

	//���ԊԊu
	void SetInterval(int m);	//!	���ԊԊu�̐ݒ�
	int GetInterval(void) const {return nInterval / MSec2Min; }	//!< ���ԊԊu�̎擾
	void Reset(void){ nLastTick = ::GetTickCount(); }			//!< ������̃��Z�b�g

	//�L���^����
	void Enable(bool flag);							//!< �L���^�����̐ݒ�
	bool IsEnabled(void) const { return bEnabled; }	//!< �L���^�����̓ǂݏo��

	//!	�K�莞�ԂɒB�������ǂ����̔���
	bool CheckAction(void);

private:
	DWORD	nLastTick;	//!< �Ō�Ƀ`�F�b�N�����Ƃ��̎��� (GetTickCount()�Ŏ擾��������)
	int		nInterval;	//!< Action�Ԋu (��)
	bool	bEnabled;	//!< �L�����ǂ���
};



class CAutoSaveAgent : public CDocListenerEx{
public:
	void CheckAutoSave();
	void ReloadAutoSaveParam();	//!< �ݒ��SharedArea����ǂݏo��

private:
	CPassiveTimer m_cPassiveTimer;
};





#endif

