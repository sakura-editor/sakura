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
#ifndef SAKURA_CSAVEAGENT_441713C1_707B_4B6D_8DBE_D398BA4EBFA2_H_
#define SAKURA_CSAVEAGENT_441713C1_707B_4B6D_8DBE_D398BA4EBFA2_H_

class CSaveAgent : public CDocListenerEx{
public:
	CSaveAgent();
	ECallbackResult OnCheckSave(SSaveInfo* pSaveInfo);
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);
	void OnFinalSave(ESaveResult eSaveResult);
private:
	SSaveInfo	m_sSaveInfoForRollback;
};

#endif /* SAKURA_CSAVEAGENT_441713C1_707B_4B6D_8DBE_D398BA4EBFA2_H_ */
/*[EOF]*/
