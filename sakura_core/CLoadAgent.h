// この行は文字化け対策用です。
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CLOADAGENT_780AC971_71A8_4495_BE95_C8786311D1489_H_
#define SAKURA_CLOADAGENT_780AC971_71A8_4495_BE95_C8786311D1489_H_

#include "doc/CDocListener.h"

class CLoadAgent : public CDocListenerEx{
public:
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	ELoadResult OnLoad(const SLoadInfo& sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	void OnFinalLoad(ELoadResult eLoadResult);
};

#endif /* SAKURA_CLOADAGENT_780AC971_71A8_4495_BE95_C8786311D1489_H_ */
/*[EOF]*/
