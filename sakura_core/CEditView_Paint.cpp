#include "stdafx.h"
#include "CEditView_Paint.h"
#include "CEditView.h"
#include <vector>

/*
	PAINT_LINENUMBER = (1<<0), //!< �s�ԍ�
	PAINT_RULER      = (1<<1), //!< ���[���[
	PAINT_BODY       = (1<<2), //!< �{��
*/

void CEditView_Paint::Call_OnPaint(
	int nPaintFlag,   //!< �`�悷��̈��I������
	bool bUseMemoryDC //!< ������DC���g�p����
)
{
	CEditView* pView = GetEditView();

	//�e�v�f
	CMyRect rcLineNumber(0,pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaBottom());
	CMyRect rcRuler(pView->GetTextArea().GetAreaLeft(),0,pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaTop());
	CMyRect rcBody(pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaBottom());

	//�̈���쐬 -> rc
	std::vector<CMyRect> rcs;
	if(nPaintFlag & PAINT_LINENUMBER)rcs.push_back(rcLineNumber);
	if(nPaintFlag & PAINT_RULER)rcs.push_back(rcRuler);
	if(nPaintFlag & PAINT_BODY)rcs.push_back(rcBody);
	if(rcs.size()==0)return;
	CMyRect rc=rcs[0];
	for(int i=1;i<(int)rcs.size();i++)
		rc=MergeRect(rc,rcs[i]);

	//�`��
	PAINTSTRUCT	ps;
	ps.rcPaint = rc;
	HDC hdc = ::GetDC( pView->m_hWnd );
	pView->OnPaint( hdc, &ps, bUseMemoryDC );
	::ReleaseDC( pView->m_hWnd, hdc );
}


