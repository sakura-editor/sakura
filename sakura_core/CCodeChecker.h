#pragma once

#include "CDocListener.h"

class CCodeChecker : public CDocListenerEx{
public:
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo);
	void OnFinishSave(ESaveResult eSaveResult);
};
