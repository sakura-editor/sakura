//	$Id$
//
//	CAutoSave.cpp
/*! @file
	�t�@�C���̎����ۑ�
	@author genta
	@version $Revision$
	@date 2000
*/
//	Copyright (C) 2000, genta
//
#include "CAutoSave.h"

//----------------------------------------------------------
//	class CPassiveTimer
//
//----------------------------------------------------------
/*!
	���ԊԊu�̐ݒ�
	@param m �Ԋu(min)
	�Ԋu��0�ȉ��ɐݒ肵���Ƃ���1�b�Ƃ݂Ȃ��B�ݒ�\�ȍő�Ԋu��35792���B
*/
void CPassiveTimer::SetInterval(int m)
{
	if( m <= 0 )
		m = 1;
	else if( m >= 35792 )	//	35792���ȏゾ�� int �ŕ\���ł��Ȃ��Ȃ�
		m = 35792;

	nInterval = m * MSec2Min;
}
/*!
	�^�C�}�[�̗L���E�����̐؂�ւ�
	@param flag true:�L�� / false: ����
	�������L���ɐ؂�ւ����Ƃ��̓��Z�b�g�����B
*/
void CPassiveTimer::Enable(bool flag)
{
	if( bEnabled != flag ){	//	�ύX������Ƃ�
		bEnabled = flag;
		if( flag ){	//	enabled
			Reset();
		}
	}
}
/*!
	�O���Œ���Ɏ��s�����Ƃ��납��Ăяo�����֐��B
	�Ăяo�����ƌo�ߎ��Ԃ��`�F�b�N����B
	
	@retval true ���莞�Ԃ��o�߂����B���̂Ƃ��͑����������I�Ƀ��Z�b�g�����B
	@retval false ����̎��ԂɒB���Ă��Ȃ��B
*/
bool CPassiveTimer::CheckAction(void)
{
	if( !IsEnabled() )	//	�L���łȂ���Ή������Ȃ�
		return false;

	//	������r
	DWORD now = ::GetTickCount();
	int diff;
	
	diff = now - nLastTick;	//	TickCount�����肵�Ă�����ł��܂������͂�...
	
	if( diff < nInterval )	//	�K�莞�ԂɒB���Ă��Ȃ�
		return false;
	
	Reset();
	return true;
}
