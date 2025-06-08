/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCODECHECKER_44D0ED68_9D9D_4B3E_88F5_185934F5FF0E_H_
#define SAKURA_CCODECHECKER_44D0ED68_9D9D_4B3E_88F5_185934F5FF0E_H_
#pragma once

#include "doc/CDocListener.h"
#include "util/design_template.h"

class CCodeChecker : public CDocListenerEx, public TSingleton<CCodeChecker>{
	friend class TSingleton<CCodeChecker>;
	CCodeChecker(){}

public:
	//セーブ時チェック
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo) override;
	void OnFinalSave(ESaveResult eSaveResult) override;

	//ロード時チェック
	void OnFinalLoad(ELoadResult eLoadResult) override;
};
#endif /* SAKURA_CCODECHECKER_44D0ED68_9D9D_4B3E_88F5_185934F5FF0E_H_ */
