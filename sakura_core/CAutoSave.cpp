//	$Id$
//
//	CAutoSave.cpp
/*! @file
	ファイルの自動保存
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
	時間間隔の設定
	@param m 間隔(min)
	間隔を0以下に設定したときは1秒とみなす。設定可能な最大間隔は35792分。
*/
void CPassiveTimer::SetInterval(int m)
{
	if( m <= 0 )
		m = 1;
	else if( m >= 35792 )	//	35792分以上だと int で表現できなくなる
		m = 35792;

	nInterval = m * MSec2Min;
}
/*!
	タイマーの有効・無効の切り替え
	@param flag true:有効 / false: 無効
	無効→有効に切り替えたときはリセットされる。
*/
void CPassiveTimer::Enable(bool flag)
{
	if( bEnabled != flag ){	//	変更があるとき
		bEnabled = flag;
		if( flag ){	//	enabled
			Reset();
		}
	}
}
/*!
	外部で定期に実行されるところから呼び出される関数。
	呼び出されると経過時間をチェックする。
	
	@retval true 所定時間が経過した。このときは測定基準が自動的にリセットされる。
	@retval false 所定の時間に達していない。
*/
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
