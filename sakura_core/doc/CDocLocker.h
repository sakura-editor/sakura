#pragma once

#include "doc/CDocListener.h"

class CDocLocker : public CDocListenerEx{
public:
	CDocLocker();
	
	//クリア
	void Clear(void) { m_bIsDocWritable = true; }

	//ロード前後
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	
	//セーブ前後
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//状態
	bool IsDocWritable() const{ return m_bIsDocWritable; }

	//チェック
	void CheckWritable(bool bMsg);

private:
	bool m_bIsDocWritable;
};
