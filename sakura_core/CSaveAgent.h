#pragma once

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
