#pragma once

#include "doc/CDocListener.h"
#include "util/design_template.h"

class CCodeChecker : public CDocListenerEx, public TSingleton<CCodeChecker>{
public:
	//�Z�[�u���`�F�b�N
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo);
	void OnFinalSave(ESaveResult eSaveResult);

	//���[�h���`�F�b�N
	void OnFinalLoad(ELoadResult eLoadResult);
};
