/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CRULER_CF213704_1CF6_427E_AD78_D628D2D1F9029_H_
#define SAKURA_CRULER_CF213704_1CF6_427E_AD78_D628D2D1F9029_H_

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

#endif /* SAKURA_CRULER_CF213704_1CF6_427E_AD78_D628D2D1F9029_H_ */
/*[EOF]*/
