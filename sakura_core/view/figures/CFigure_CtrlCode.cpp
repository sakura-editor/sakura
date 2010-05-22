#include "stdafx.h"
#include "view/CEditView.h" // SColorStrategyInfo
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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanBinary::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		int nLen = pText[1]? 2:1;	// �� pText �͏�ɏI�[������O
		if(CNativeW::GetKetaOfChar(pText, nLen, 0) == 1){	// ���p
			ECharSet e;
			CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
			if(e == CHARSET_BINARY){
				return true;
			}
		}
	}
	return false;
}

void CFigure_HanBinary::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
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
			L"��",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_ZenBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenBinary::Match(const wchar_t* pText) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp){
		int nLen = pText[1]? 2:1;	// �� pText �͏�ɏI�[������O
		if(CNativeW::GetKetaOfChar(pText, nLen, 0) > 1){	// �S�p
			ECharSet e;
			CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
			if(e == CHARSET_BINARY){
				return true;
			}
		}
	}
	return false;
}

void CFigure_ZenBinary::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,2))
	{
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rc,
			L"��",
			1,
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}
