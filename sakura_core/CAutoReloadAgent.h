/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CAUTORELOADAGENT_5B64C473_C8AB_4660_AAA9_3A999953008B_H_
#define SAKURA_CAUTORELOADAGENT_5B64C473_C8AB_4660_AAA9_3A999953008B_H_

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
	void CheckFileTimeStamp();	// ファイルのタイムスタンプのチェック処理

public:
	WatchUpdate		m_eWatchUpdate;	//!< 更新監視方法

private:
	int m_nPauseCount;	//これが1以上の場合は監視をしない
	int m_nDelayCount;	//未編集で再ロード時の遅延カウンタ
};

#endif /* SAKURA_CAUTORELOADAGENT_5B64C473_C8AB_4660_AAA9_3A999953008B_H_ */
/*[EOF]*/
