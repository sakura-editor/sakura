/*!	@file
	@brief ファイルの自動保存

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
#include <Windows.h>
#include "_main/global.h"
#include "doc/CDocListener.h"

//! 分→ミリ秒に変換するための係数
const int MSec2Min = 1000 * 60;

/*! @class CPassiveTimer CAutoSave.h
	基準時刻からの経過時間が設定間隔を過ぎたかどうかを判定する。
	頻繁に呼び出されるタイマーが既に別の場所にあるとき、それよりも間隔が広くて
	間隔の厳密さが要求されない用途に利用可能。
	ファイルの自動保存で使っている。
	@author genta
*/
class CPassiveTimer {
public:
	/*!
		初期値は間隔1msecでタイマーは無効。
	*/
	CPassiveTimer() : nInterval(1), bEnabled(false){ Reset(); }

	//時間間隔
	void SetInterval(int m);	//!	時間間隔の設定
	int GetInterval(void) const {return nInterval / MSec2Min; }	//!< 時間間隔の取得
	void Reset(void){ nLastTick = ::GetTickCount(); }			//!< 基準時刻のリセット

	//有効／無効
	void Enable(bool flag);							//!< 有効／無効の設定
	bool IsEnabled(void) const { return bEnabled; }	//!< 有効／無効の読み出し

	//!	規定時間に達したかどうかの判定
	bool CheckAction(void);

private:
	DWORD	nLastTick;	//!< 最後にチェックしたときの時刻 (GetTickCount()で取得したもの)
	int		nInterval;	//!< Action間隔 (分)
	bool	bEnabled;	//!< 有効かどうか
};



class CAutoSaveAgent : public CDocListenerEx{
public:
	void CheckAutoSave();
	void ReloadAutoSaveParam();	//!< 設定をSharedAreaから読み出す

private:
	CPassiveTimer m_cPassiveTimer;
};





#endif

