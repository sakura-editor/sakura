#pragma once

class CEditView;

//!�i����̓N���X
class CViewParser{
public:
	CViewParser(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CViewParser(){}

	//! �J�[�\�����O�̒P����擾
	int GetLeftWord( CNativeW2* pcmemWord, int nMaxWordLen ) const;

	//! �L�����b�g�ʒu�̒P����擾
	// 2006.03.24 fon
	BOOL GetCurrentWord( CNativeW2* pcmemWord ) const;

private:
	const CEditView* m_pEditView;
};
