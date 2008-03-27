#pragma once

#include "CDocListener.h"

// ファイルが更新された場合に再読込を行うかどうかのフラグ
enum WatchUpdate {
	WU_QUERY,	//!< 再読込を行うかどうかダイアログボックスで問い合わせる
	WU_NOTIFY,	//!< 更新されたことをステータスバーで通知
	WU_NONE		//!< 更新監視を行わない
};

class CAutoReloadAgent : public CDocListenerEx{
public:
	CAutoReloadAgent();
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//監視の一時停止
	void PauseWatching(){ m_nPauseCount++; }
	void ResumeWatching(){ m_nPauseCount--; assert(m_nPauseCount>=0); }
	bool IsPausing() const{ return m_nPauseCount>=1; }

public://#####仮
	bool _ToDoChecking() const;
	bool _IsFileUpdatedByOther(FILETIME* pNewFileTime) const;
	void CheckFileTimeStamp();	/* ファイルのタイムスタンプのチェック処理 */

public:
	WatchUpdate		m_eWatchUpdate;	//!< 更新監視方法

private:
	int m_nPauseCount; //これが1以上の場合は監視をしない
};
