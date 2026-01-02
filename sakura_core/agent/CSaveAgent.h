/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSAVEAGENT_9EDD4D3A_82B2_4651_82E5_AE471A54812B_H_
#define SAKURA_CSAVEAGENT_9EDD4D3A_82B2_4651_82E5_AE471A54812B_H_
#pragma once

#include "doc/CDocListener.h"

class CSaveAgent : public CDocListenerEx{
public:
	CSaveAgent();
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo) override;
	void OnBeforeSave(const SSaveInfo& sSaveInfo) override;
	void OnSave(const SSaveInfo& sSaveInfo) override;
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;
	void OnFinalSave(ESaveResult eSaveResult) override;
private:
	SSaveInfo	m_sSaveInfoForRollback;
};
#endif /* SAKURA_CSAVEAGENT_9EDD4D3A_82B2_4651_82E5_AE471A54812B_H_ */
