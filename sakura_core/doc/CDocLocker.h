#pragma once

#include "doc/CDocListener.h"

class CDocLocker : public CDocListenerEx{
public:
	CDocLocker();

	//�N���A
	void Clear(void) { m_bIsDocWritable = true; }

	//���[�h�O��
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	
	//�Z�[�u�O��
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//���
	bool IsDocWritable() const{ return m_bIsDocWritable; }

	//�`�F�b�N
	void CheckWritable(bool bMsg);

private:
	bool m_bIsDocWritable;
};
