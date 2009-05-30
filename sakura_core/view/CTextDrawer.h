#pragma once

#include "env/CShareData.h" //ColorInfo
class CTextMetrics;
class CTextArea;
class CViewFont;
class CEol;
class CEditView;
class CLayout;
#include "DispPos.h"

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

	// -- -- �s�ԍ� -- -- //
	void DispLineNumber( CGraphics& gr, CLayoutInt nLineNum, int y ) const;		// �s�ԍ��\��

public: //####��
	// -- -- �����⏕ -- -- //
	EColorIndexType _GetColorIdx(EColorIndexType nColorIdx) const;


private:
	const CEditView* m_pEditView;
};

