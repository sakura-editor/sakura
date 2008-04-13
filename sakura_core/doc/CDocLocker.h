#pragma once

#include "doc/CDocListener.h"

class CDocLocker : public CDocListenerEx{
public:
	CDocLocker();

	//ロード前後
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	
	//セーブ前後
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	bool IsDocWritable() const{ return m_bIsDocWritable; }
private:
	bool m_bIsDocWritable;
};
