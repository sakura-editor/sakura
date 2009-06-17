#pragma once

#include "doc/CDocListener.h"

class CMruListener : public CDocListenerEx{
public:
	//ロード前後
//	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//セーブ前後
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//クローズ前後
	ECallbackResult OnBeforeClose();

protected:
	//ヘルパ
	void _HoldBookmarks_And_AddToMRU(); // Mar. 30, 2003 genta
};
