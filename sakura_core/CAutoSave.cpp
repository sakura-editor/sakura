//	$Id$
//
//	CAutoSave.h
//	Copyright (C) 2000, genta
//
//	ファイルの自動保存
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
	else if( m >= 35792 )	//	35792分以上だと int で表現できなくなる
		m = 35792;

	nInterval = m * MSec2Min;
}

void CPassiveTimer::Enable(bool flag)
{
	if( bEnabled != flag ){	//	変更があるとき
		bEnabled = flag;
		if( flag ){	//	enabled
			Reset();
		}
	}
}

bool CPassiveTimer::CheckAction(void)
{
	if( !IsEnabled() )	//	有効でなければ何もしない
		return false;

	//	時刻比較
	DWORD now = ::GetTickCount();
	int diff;
	
	diff = now - nLastTick;	//	TickCountが一回りしてもこれでうまくいくはず...
	
	if( diff < nInterval )	//	規定時間に達していない
		return false;
	
	Reset();
	return true;
}
