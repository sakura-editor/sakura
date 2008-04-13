#pragma once

#include "doc/CDocListener.h"

class CDocLocker : public CDocListenerEx{
public:
	CDocLocker();

	//���[�h�O��
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	
	//�Z�[�u�O��
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	bool IsDocWritable() const{ return m_bIsDocWritable; }
private:
	bool m_bIsDocWritable;
};
