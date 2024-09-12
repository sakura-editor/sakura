/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CAUTORELOADAGENT_6340340B_4677_4748_8240_689427F4AE3D_H_
#define SAKURA_CAUTORELOADAGENT_6340340B_4677_4748_8240_689427F4AE3D_H_
#pragma once

#include "doc/CDocListener.h"

// ファイルが更新された場合に再読込を行うかどうかのフラグ
enum WatchUpdate {
	WU_QUERY,	//!< 再読込を行うかどうかダイアログボックスで問い合わせる
	WU_NOTIFY,	//!< 更新されたことをステータスバーで通知
	WU_NONE,	//!< 更新監視を行わない
	WU_AUTOLOAD,//!< 更新され未編集の場合に再ロード
};

class CAutoReloadAgent : public CDocListenerEx{
public:
	explicit CAutoReloadAgent(CEditDoc* pcDoc);

	void OnBeforeSave(const SSaveInfo& sSaveInfo) override;
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;

	//監視の一時停止
	void PauseWatching(){ m_nPauseCount++; }
	void ResumeWatching()
	{
		m_nPauseCount--;
		assert(m_nPauseCount>=0);
	}
	bool IsPausing() const{ return m_nPauseCount>=1; }

	bool _ToDoChecking() const;
	bool _IsFileUpdatedByOther(FILETIME* pNewFileTime) const;
	void CheckFileTimeStamp();	// ファイルのタイムスタンプのチェック処理

	WatchUpdate m_eWatchUpdate = WU_QUERY;	//!< 更新監視方法
	int         m_nPauseCount  = 0;	//これが1以上の場合は監視をしない
	int         m_nDelayCount  = 0;	//未編集で再ロード時の遅延カウンタ
};

#endif /* SAKURA_CAUTORELOADAGENT_6340340B_4677_4748_8240_689427F4AE3D_H_ */
