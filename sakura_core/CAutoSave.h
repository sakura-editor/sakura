//	$Id$
//
//	CAutoSave.h
//	Copyright (C) 2000, genta
//
//	ファイルの自動保存
//
//	外部から定期的にPollingされる．
//
#ifndef __CAUTOSAVE_H_
#define __CAUTOSAVE_H_
#include <windows.h>
#include "global.h"

const int MSec2Min = 1000 * 60;

class SAKURA_CORE_API CPassiveTimer {
public:
	CPassiveTimer() : nInterval(1), bEnabled(false){ Reset(); }
	
	//	時間間隔
	void SetInterval(int m);

	int GetInterval(void) const {return nInterval / MSec2Min; }
	
	//	タイマーのリセット
	void Reset(void){ nLastTick = ::GetTickCount(); }
	
	//	有効／無効の設定
	void Enable(bool flag);
	bool IsEnabled(void) const { return bEnabled; }
	
	//	定期的に呼び出される
	bool CheckAction(void);

private:
	DWORD nLastTick;	//	最後にチェックしたときの時刻
	int  nInterval;	//	Action間隔 (分)
	bool bEnabled;	//	有効かどうか
};
#endif
