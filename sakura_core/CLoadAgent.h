// ‚±‚Ìs‚Í•¶š‰»‚¯‘Îô—p‚Å‚·B
#pragma once

#include "doc/CDocListener.h"

class CLoadAgent : public CDocListenerEx{
public:
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	ELoadResult OnLoad(const SLoadInfo& sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	void OnFinalLoad(ELoadResult eLoadResult);
};
