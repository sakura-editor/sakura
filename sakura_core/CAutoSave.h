//	$Id$
//
//	CAutoSave.h
/*!	@file
	ファイルの自動保存

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

//! 分→ミリ秒に変換するための係数
const int MSec2Min = 1000 * 60;
/*! @class CPassiveTimer CAutoSave.h
	基準時刻からの経過時間が設定間隔を過ぎたかどうかを判定する。
	頻繁に呼び出されるタイマーが既に別の場所にあるとき、それよりも間隔が広くて
	間隔の厳密さが要求されない用途に利用可能。
	ファイルの自動保存で使っている。
	@author genta
*/
class SAKURA_CORE_API CPassiveTimer {
public:
	/*!
		初期値は間隔1msecでタイマーは無効。
	*/
	CPassiveTimer() : nInterval(1), bEnabled(false){ Reset(); }

	//!	時間間隔の設定
	void SetInterval(int m);

	//!	時間間隔の取得
	int GetInterval(void) const {return nInterval / MSec2Min; }

	//!	基準時刻のリセット
	void Reset(void){ nLastTick = ::GetTickCount(); }

	//!	有効／無効の設定
	void Enable(bool flag);
	//!	有効／無効の読み出し
	bool IsEnabled(void) const { return bEnabled; }

	//!	規定時間に達したかどうかの判定
	bool CheckAction(void);

private:
	DWORD nLastTick;	//!<	最後にチェックしたときの時刻 (GetTickCount()で取得したもの)
	int  nInterval;		//!<	Action間隔 (分)
	bool bEnabled;	//!<	有効かどうか
};
#endif


/*[EOF]*/
