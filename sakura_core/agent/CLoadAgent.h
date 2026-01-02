/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CLOADAGENT_369F511F_4F3E_41A2_84AB_EDE3BB368568_H_
#define SAKURA_CLOADAGENT_369F511F_4F3E_41A2_84AB_EDE3BB368568_H_
#pragma once

#include "doc/CDocListener.h"

class CLoadAgent : public CDocListenerEx{
public:
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo) override;
	void OnBeforeLoad(SLoadInfo* sLoadInfo) override;
	ELoadResult OnLoad(const SLoadInfo& sLoadInfo) override;
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;
	void OnFinalLoad(ELoadResult eLoadResult) override;
};
#endif /* SAKURA_CLOADAGENT_369F511F_4F3E_41A2_84AB_EDE3BB368568_H_ */
