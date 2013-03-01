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
#ifndef SAKURA_CTEXTDRAWER_AECD0708_CE8E_489C_A9ED_484CBAB9523E9_H_
#define SAKURA_CTEXTDRAWER_AECD0708_CE8E_489C_A9ED_484CBAB9523E9_H_

class CTextMetrics;
class CTextArea;
class CViewFont;
class CEol;
class CEditView;
class CLayout;
#include "DispPos.h"

class CGraphics;

class CTextDrawer{
public:
	CTextDrawer(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CTextDrawer(){}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �O���ˑ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�̈�̃C���X�^���X�����߂�
	const CTextArea* GetTextArea() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �C���^�[�t�F�[�X                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
	//���ۂɂ� pX �� nX ���X�V�����B
	void DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength, bool bTransparent = false ) const; // �e�L�X�g�\�� (�V������)
	void DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength, bool bTransparent = false ) const;      // �e�L�X�g�\�� (�Â���)

	// -- -- �w�茅�c���`�� -- -- //
	//!	�w�茅�c���`��֐�	// 2005.11.08 Moca
	void DispVerticalLines( CGraphics& gr, int nTop, int nBottom, CLayoutInt nLeftCol, CLayoutInt nRightCol ) const;

	// -- -- �܂�Ԃ����c���`�� -- -- //
	void DispWrapLine( CGraphics& gr, int nTop, int nBottom ) const;

	// -- -- �s�ԍ� -- -- //
	void DispLineNumber( CGraphics& gr, CLayoutInt nLineNum, int y ) const;		// �s�ԍ��\��

public: //####��
	// -- -- �����⏕ -- -- //
	EColorIndexType _GetColorIdx(EColorIndexType nColorIdx) const;


private:
	const CEditView* m_pEditView;
};

#endif /* SAKURA_CTEXTDRAWER_AECD0708_CE8E_489C_A9ED_484CBAB9523E9_H_ */
/*[EOF]*/
