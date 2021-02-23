/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
