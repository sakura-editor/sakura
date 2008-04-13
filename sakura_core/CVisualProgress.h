#pragma once

#include "doc/CDocListener.h"
class CWaitCursor;

class CVisualProgress : public CDocListenerEx, public CProgressListener{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CVisualProgress();
	virtual ~CVisualProgress();

	//���[�h�O��
	void OnBeforeLoad(const SLoadInfo& sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//�Z�[�u�O��
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnFinalSave(ESaveResult eSaveResult);

	//�v���O���X��M
	void OnProgress(int nPer);

protected:
	//�����⏕
	void _Begin();
	void _Doing(int nPer);
	void _End();
private:
	CWaitCursor* m_pcWaitCursor;
};
