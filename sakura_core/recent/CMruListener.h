#pragma once

#include "doc/CDocListener.h"

class CMruListener : public CDocListenerEx{
public:
	//���[�h�O��
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(const SLoadInfo& sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//�Z�[�u�O��
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//�N���[�Y�O��
	ECallbackResult OnBeforeClose();

protected:
	//�w���p
	void _HoldBookmarks_And_AddToMRU(); // Mar. 30, 2003 genta
};
