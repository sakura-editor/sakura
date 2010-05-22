#pragma once

#include "util/design_template.h" //TSingleton
#include "doc/CDocListener.h" // CDocListenerEx

class CDocLine;
class CDocLineMgr;

//! Modified�Ǘ�
class CModifyManager : public TSingleton<CModifyManager>, public CDocListenerEx{
public:
	void OnAfterSave(const SSaveInfo& sSaveInfo);

};

//! �s�ɕt������Modified���
class CLineModified{
public:
	CLineModified() : m_bModified(true) { }
	operator bool() const{ return m_bModified; }
	CLineModified& operator = (bool b)
	{
		m_bModified = b;
		return *this;
	}
private:
	bool m_bModified;
};

//! �s�S�̂�Modified���A�N�Z�T
class CModifyVisitor{
public:
	//���
	bool IsLineModified(const CDocLine* pcDocLine) const;
	void SetLineModified(CDocLine* pcDocLine, bool bModified);

	//�ꊇ����
	void ResetAllModifyFlag(CDocLineMgr* pcDocLineMgr);	// �s�ύX��Ԃ����ׂă��Z�b�g
};
