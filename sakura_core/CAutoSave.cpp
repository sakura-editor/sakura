//	$Id$
//
//	CAutoSave.h
//	Copyright (C) 2000, genta
//
//	�t�@�C���̎����ۑ�
//
#include "CAutoSave.h"

//----------------------------------------------------------
//	class CPassiveTimer
//
//----------------------------------------------------------
void CPassiveTimer::SetInterval(int m)
{
	if( m <= 0 )
		m = 1;
	else if( m >= 35792 )	//	35792���ȏゾ�� int �ŕ\���ł��Ȃ��Ȃ�
		m = 35792;

	nInterval = m * MSec2Min;
}

void CPassiveTimer::Enable(bool flag)
{
	if( bEnabled != flag ){	//	�ύX������Ƃ�
		bEnabled = flag;
		if( flag ){	//	enabled
			Reset();
		}
	}
}

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
