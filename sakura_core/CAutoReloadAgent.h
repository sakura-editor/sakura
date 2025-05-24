/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
	CAutoReloadAgent();
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

public://#####仮
	bool _ToDoChecking() const;
	bool _IsFileUpdatedByOther(FILETIME* pNewFileTime) const;
	void CheckFileTimeStamp();	// ファイルのタイムスタンプのチェック処理

public:
	WatchUpdate		m_eWatchUpdate;	//!< 更新監視方法

private:
	int m_nPauseCount;	//これが1以上の場合は監視をしない
	int m_nDelayCount;	//未編集で再ロード時の遅延カウンタ
};
#endif /* SAKURA_CAUTORELOADAGENT_6340340B_4677_4748_8240_689427F4AE3D_H_ */
