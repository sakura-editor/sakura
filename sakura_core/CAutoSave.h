//	$Id$
//
//	CAutoSave.h
/*!	@file
	�t�@�C���̎����ۑ�

	@author genta
	@version $Revision$
	@date 2000
*/
//	Copyright (C) 2000, genta
//	
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
