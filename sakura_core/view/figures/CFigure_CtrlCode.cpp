#include "stdafx.h"
#include "CFigure_CtrlCode.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_CtrlCode::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		//���ʂ�ASCII���䕶���iC0 Controls, IsHankaku()�Ŕ��p�����j�����𐧌䕶���\���ɂ���
		//�������Ȃ��� IsHankaku(0x0600)==false �Ȃ̂� iswcntrl(0x0600)!=0 �̂悤�ȃP�[�X�ŕ\�����������
		//U+0600: ARABIC NUMBER SIGN
		if(!(pText[0] & 0xFF80) && WCODE::IsControlCode(pText[0])){
			return true;
		}
	}
	return false;
}

CLayoutInt CFigure_CtrlCode::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �R���g���[���R�[�h�`��
void CFigure_CtrlCode::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,1))
	{
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rc,
			L"�",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}
