#pragma once

#include "doc/CDocListener.h"
#include "util/design_template.h"

class CCodeChecker : public CDocListenerEx, public TSingleton<CCodeChecker>{
public:
	//セーブ時チェック
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo);
	void OnFinalSave(ESaveResult eSaveResult);

	//ロード時チェック
	void OnFinalLoad(ELoadResult eLoadResult);
};
