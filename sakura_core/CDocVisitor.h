#pragma once

class CDocVisitor{
public:
	CDocVisitor(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	void SetAllEol(CEol cEol); //!< ���s�R�[�h�𓝈ꂷ��
private:
	CEditDoc* m_pcDocRef;
};
