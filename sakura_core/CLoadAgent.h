// ���̍s�͕��������΍��p�ł��B
#pragma once

#include "doc/CDocListener.h"

class CLoadAgent : public CDocListenerEx{
public:
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(const SLoadInfo& sLoadInfo);
	ELoadResult OnLoad(const SLoadInfo& sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	void OnFinalLoad(ELoadResult eLoadResult);
};
