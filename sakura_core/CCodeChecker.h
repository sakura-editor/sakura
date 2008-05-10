#pragma once

#include "doc/CDocListener.h"
#include "util/design_template.h"

class CCodeChecker : public CDocListenerEx, public TSingleton<CCodeChecker>{
public:
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo);
	void OnFinishSave(ESaveResult eSaveResult);
};
