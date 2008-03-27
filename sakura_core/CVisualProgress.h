#pragma once

#include "CDocListener.h"
class CWaitCursor;

class CVisualProgress : public CDocListenerEx, public CProgressListener{
public:
	//コンストラクタ・デストラクタ
	CVisualProgress();
	virtual ~CVisualProgress();

	//ロード前後
	void OnBeforeLoad(const SLoadInfo& sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//セーブ前後
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnFinalSave(ESaveResult eSaveResult);

	//プログレス受信
	void OnProgress(int nPer);

protected:
	//実装補助
	void _Begin();
	void _Doing(int nPer);
	void _End();
private:
	CWaitCursor* m_pcWaitCursor;
};
