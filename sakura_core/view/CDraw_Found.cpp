#include "stdafx.h"
#include "CDraw_Found.h"
#include "types/CTypeSupport.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)



bool CDraw_Found::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	CTypeSupport cSearchType(pInfo->pcView,COLORIDX_SEARCH);

	if( pInfo->pcView->m_bCurSrchKeyMark && cSearchType.IsDisp() ){
	}
	else{
		return false;
	}

top:
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor ���K�\���̌���������}�[�N������������
	if(!pInfo->bSearchStringMode && (!pInfo->pcView->m_sCurSearchOption.bRegularExp || (pInfo->bSearchFlg && pInfo->nSearchStart < pInfo->nPos))){
		pInfo->bSearchFlg=pInfo->pcView->IsSearchString(
			pInfo->pLine,
			pInfo->nLineLen,
			pInfo->nPos,
			&pInfo->nSearchStart,
			&pInfo->nSearchEnd
		);
	}
	//�}�b�`�����񌟏o
	if( !pInfo->bSearchStringMode && pInfo->bSearchFlg && pInfo->nSearchStart==pInfo->nPos){
		// -- -- �}�b�`�����񒼑O�܂ł�`�� -- -- //

		pInfo->DrawToHere();
		pInfo->bSearchStringMode = true;

		// ���݂̐F���w��
		pInfo->pcView->SetCurrentColor( pInfo->hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
	}
	//�}�b�`������I�����o
	else if( pInfo->bSearchStringMode && pInfo->nSearchEnd <= pInfo->nPos ){ //+ == �ł͍s�������̏ꍇ�ApInfo->nSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
		// -- -- �}�b�`�������`�� -- -- //

		// ��������������̏I���܂ł�����A�����F��W���ɖ߂�����
		pInfo->DrawToHere();
		/* ���݂̐F���w�� */
		pInfo->pcView->SetCurrentColor( pInfo->hdc, pInfo->nCOMMENTMODE );
		pInfo->bSearchStringMode = false;

		goto top;
	}
	return false;
}




bool CDraw_Found::EndColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	return false;
}
