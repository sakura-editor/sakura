//	$Id$
//
//	CAutoSave.cpp
/*! @file
	�t�@�C���̎����ۑ�
	@author genta
	@date 2000
	$Revision$
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
