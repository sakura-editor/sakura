#pragma once

class CTextArea;
class CEditView;
class CEditDoc;
class CTextMetrics;

class CRuler{
public:
	CRuler(const CEditView* pEditView, const CEditDoc* pEditDoc);
	virtual ~CRuler();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �C���^�[�t�F�[�X                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! ���[���[�`�� (�w�i�ƃL�����b�g)
	void DispRuler( HDC );

	//! ���[���[�̔w�i�̂ݕ`�� 2007.08.29 kobake �ǉ�
	void DrawRulerBg(CGraphics& gr);

	void SetRedrawFlag(){ m_bRedrawRuler = true; }
	bool GetRedrawFlag(){ return m_bRedrawRuler; }

private:
	//! ���[���[�̃L�����b�g�̂ݕ`�� 2002.02.25 Add By KK
	void DrawRulerCaret( CGraphics& gr );

	void _DrawRulerCaret( CGraphics& gr, int nCaretDrawX, int nCaretWidth );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	//�Q��
	const CEditView*	m_pEditView;
	const CEditDoc*		m_pEditDoc;

	//���
	bool	m_bRedrawRuler;		// ���[���[�S�̂�`�������� = true      2002.02.25 Add By KK
	int		m_nOldRulerDrawX;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK  2007.08.26 kobake ���O�ύX
	int		m_nOldRulerWidth;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK  2007.08.26 kobake ���O�ύX
};
