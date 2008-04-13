#pragma once

class CDocVisitor{
public:
	CDocVisitor(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	void SetAllEol(CEol cEol); //!< 改行コードを統一する
private:
	CEditDoc* m_pcDocRef;
};
