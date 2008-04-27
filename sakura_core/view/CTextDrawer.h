#pragma once

#include "CShareData.h" //ColorInfo
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
	void DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength ) const; // �e�L�X�g�\�� (�V������)
	void DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength ) const;      // �e�L�X�g�\�� (�Â���)

	//2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
	//2007.08.25 kobake �������� nCharWidth, nLineHeight ���폜
	//���ۂɂ� pX �� nX ���X�V�����B
	//	EOF�`��֐�	//	2004.05.29 genta
	//2007.08.28 kobake ���� fuOptions ���폜
	void DispEOF( HDC hdc, DispPos* pDispPos) const;

	//May 23, 2000 genta
	//��ʕ`��⏕�֐�
	//@@@ 2001.12.21 YAZAKI ���s�L���̏����������ς������̂ŏC��
	void _DrawEOL(HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight,
		CEol cEol, bool bBold, COLORREF pColor ) const;
	//2007.08.30 kobake �ǉ�
	void DispEOL(HDC hdc, DispPos* pDispPos, CEol cEol, bool bSearchStringMode) const;

	// -- -- �^�u�`�� -- -- //
	//2007.08.28 kobake �ǉ�
	void DispTab( HDC hdc, DispPos* pDispPos, EColorIndexType nColorIdx ) const;
	//�^�u���`��֐�	//@@@ 2003.03.26 MIK
	void _DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor ) const;

	// -- -- �X�y�[�X�`�� -- -- //
	void DispZenkakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode) const;
	void DispHankakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode) const;

	// -- -- �܂�Ԃ��`�� -- -- //
	void DispWrap(HDC hdc, DispPos* pDispPos) const;

	// -- -- ��(����)�s�`�� -- -- //
	bool DispEmptyLine(HDC hdc, DispPos* pDispPos) const;

	// -- -- �w�茅�c���`�� -- -- //
	//!	�w�茅�c���`��֐�	// 2005.11.08 Moca
	void DispVerticalLines( HDC hdc, int nTop, int nBottom, CLayoutInt nLeftCol, CLayoutInt nRightCol ) const;

	// -- -- �s�ԍ� -- -- //
	void DispLineNumber( HDC hdc, const CLayout* pcLayout, int nLineNum, int y ) const;		// �s�ԍ��\��

protected:
	// -- -- �����⏕ -- -- //
	EColorIndexType _GetColorIdx(EColorIndexType nColorIdx,bool bSearchStringMode) const;


private:
	const CEditView* m_pEditView;
};

