#include "stdafx.h"
#include "CDrawStrategy.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_URL::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int	nUrlLen;
	
	if( pInfo->bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
	 && IsURL( &pInfo->pLine[pInfo->nPos], pInfo->nLineLen - pInfo->nPos, &nUrlLen )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_URL);	/* URL���[�h */ // 2002/03/13 novice
		pInfo->nCOMMENTEND = pInfo->nPos + nUrlLen;
		return true;
	}
	return false;
}

bool CDraw_URL::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int		nUrlLen;

	if( pInfo->bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
	 && ( IsURL( &pInfo->pLine[pInfo->nPos], pInfo->nLineLen - pInfo->nPos, &nUrlLen ) )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
	){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
		pInfo->nCOMMENTEND = pInfo->nPos + nUrlLen;
		/* ���݂̐F���w�� */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���p���l                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_Numeric::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	int	nnn;
	
	if( pInfo->bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
		&& (nnn = IsNumber( pInfo->pLine, pInfo->nPos, pInfo->nLineLen )) > 0 )		/* ���p������\������ */
	{
		/* �L�[���[�h������̏I�[���Z�b�g���� */
		int i = pInfo->nPos + nnn;
		pInfo->DrawToHere();
		/* ���݂̐F���w�� */
		pInfo->ChangeColor(COLORIDX_DIGIT);	/* ���p���l�ł��� */ // 2002/03/13 novice
		pInfo->nCOMMENTEND = i;
		return true;
	}
	return false;
}

bool CDraw_Numeric::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
	int i;
	if( pInfo->bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
		&& (i = IsNumber( pInfo->pLine, pInfo->nPos, pInfo->nLineLen )) > 0 )		/* ���p������\������ */
	{
		/* �L�[���[�h������̏I�[���Z�b�g���� */
		i = pInfo->nPos + i;
		/* ���݂̐F���w�� */
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
		pInfo->nCOMMENTEND = i;
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�Z�b�g                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_KeywordSet::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	
	if( pInfo->bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
		TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
		IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] )
	){
		//	Mar 4, 2001 genta comment out
		//	pInfo->bKeyWordTop = false;
		/* �L�[���[�h������̏I�[��T�� */
		int i;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if( IS_KEYWORD_CHAR( pInfo->pLine[i] ) ){
			}else{
				break;
			}
		}
		/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
		int j = i - pInfo->nPos;
		/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
		int nIdx = GetDllShareData().m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
			TypeDataPtr->m_nKeyWordSetIdx[0],
			&pInfo->pLine[pInfo->nPos],
			j
		);
		if( nIdx != -1 ){
			pInfo->DrawToHere();

			/* ���݂̐F���w�� */
			pInfo->ChangeColor(COLORIDX_KEYWORD1);	/* �����L�[���[�h1 */ // 2002/03/13 novice
			pInfo->nCOMMENTEND = i;
		}
		else{		//MIK START ADD 2000.12.01 second keyword & binary search
			// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
			for( int my_i = 1; my_i < 10; my_i++ )
			{
				if(TypeDataPtr->m_nKeyWordSetIdx[ my_i ] != -1 && /* �L�[���[�h�Z�b�g */							//MIK 2000.12.01 second keyword
					TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
				{																							//MIK
					/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
					nIdx = GetDllShareData().m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
						TypeDataPtr->m_nKeyWordSetIdx[ my_i ] ,													//MIK
						&pInfo->pLine[pInfo->nPos],																		//MIK
						j																					//MIK
					);																						//MIK
					if( nIdx != -1 ){																		//MIK
						pInfo->DrawToHere();
						/* ���݂̐F���w�� */																//MIK
						pInfo->ChangeColor((EColorIndexType)(COLORIDX_KEYWORD1 + my_i));	/* �����L�[���[�h2 */ // 2002/03/13 novice		//MIK
						pInfo->nCOMMENTEND = i;																	//MIK
						break;
					}																						//MIK
				}																							//MIK
				else
				{
					if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
						break;
				}
			}
		}			//MIK END
		return true;
	}
	return false;
}

bool CDraw_KeywordSet::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
		TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
		IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] )
	){
		//	Mar 4, 2001 genta comment out
		/* �L�[���[�h������̏I�[��T�� */
		int i;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if( IS_KEYWORD_CHAR( pInfo->pLine[i] ) ){
			}else{
				break;
			}
		}
		/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
		int j = i - pInfo->nPos;
		/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
		int nIdx = GetDllShareData().m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
			TypeDataPtr->m_nKeyWordSetIdx[0],
			&pInfo->pLine[pInfo->nPos],
			j
		);
		if( nIdx != -1 ){
			/* ���݂̐F���w�� */
			pInfo->nBgn = pInfo->nPos;
			pInfo->nCOMMENTMODE = COLORIDX_KEYWORD1;	/* �����L�[���[�h1 */ // 2002/03/13 novice
			pInfo->nCOMMENTEND = i;
			if( !pInfo->bSearchStringMode ){
				//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
				pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
			}
		}else{		//MIK START ADD 2000.12.01 second keyword & binary search
			// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
			for( int my_i = 1; my_i < 10; my_i++ )
			{
				if(TypeDataPtr->m_nKeyWordSetIdx[my_i] != -1 && /* �L�[���[�h�Z�b�g */							//MIK 2000.12.01 second keyword
					TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
				{																							//MIK
					/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
					nIdx = GetDllShareData().m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
						TypeDataPtr->m_nKeyWordSetIdx[my_i] ,													//MIK
						&pInfo->pLine[pInfo->nPos],																		//MIK
						j																					//MIK
					);																						//MIK
					if( nIdx != -1 ){																		//MIK
						/* ���݂̐F���w�� */																//MIK
						pInfo->nBgn = pInfo->nPos;																		//MIK
						pInfo->nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* �����L�[���[�h2 */ // 2002/03/13 novice		//MIK
						pInfo->nCOMMENTEND = i;																	//MIK
						if( !pInfo->bSearchStringMode ){															//MIK
							//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );										//MIK
							pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						}																					//MIK
						break;
					}																						//MIK
				}																							//MIK
				else
				{
					if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
						break;
				}
			}
		}			//MIK END
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ����R�[�h                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_CtrlCode::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
		&& COLORIDX_CTRLCODE != pInfo->nCOMMENTMODE // 2002/03/13 novice
		&&WCODE::IsControlCode(pInfo->pLine[pInfo->nPos]))
	{
		pInfo->DrawToHere();

		pInfo->nCOMMENTMODE_OLD = pInfo->nCOMMENTMODE;
		pInfo->nCOMMENTEND_OLD = pInfo->nCOMMENTEND;

		pInfo->ChangeColor(COLORIDX_CTRLCODE);
		/* �R���g���[���R�[�h��̏I�[��T�� */
		int i;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			if(!WCODE::IsControlCode(pInfo->pLine[i])){
				break;
			}
		}
		pInfo->nCOMMENTEND = i;
		pInfo->nCharChars = CLogicInt(1);
		return true;
	}

	return false;
}

bool CDraw_CtrlCode::EndColor(SDrawStrategyInfo* pInfo)
{
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(pInfo->nCOMMENTMODE_OLD);
		pInfo->nCOMMENTEND = pInfo->nCOMMENTEND_OLD;
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_ColorEnd::EndColor(SDrawStrategyInfo* pInfo)
{
	if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}





bool CDraw_Line::EndColor(SDrawStrategyInfo* pInfo)
{
	CTypeSupport cTextType(pInfo->pcView,COLORIDX_TEXT);
	const CLayout*	pcLayout2; //���[�N�pCLayout�|�C���^
	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	int nLineHeight = pInfo->pcView->GetTextMetrics().GetHankakuDy();  //�s�̏c���H

	if( pInfo->nPos >= pInfo->nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
		if( pInfo->DrawToHere() ){
			pInfo->nBgn = pInfo->nPos + 1;

			// �s���w�i�`��
			RECT rcClip;
			if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
				cTextType.FillBack(pInfo->hdc,rcClip);
			}

			// ���s�L���̕\��
			pInfo->pcView->GetTextDrawer().DispEOL(pInfo->hdc,pInfo->pDispPos,pcLayout2->GetLayoutEol(),pInfo->bSearchStringMode);

			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			pInfo->pcView->GetTextDrawer().DispVerticalLines(
				pInfo->hdc,
				pInfo->pDispPos->GetDrawPos().y,
				pInfo->pDispPos->GetDrawPos().y + nLineHeight,
				CLayoutInt(0),
				CLayoutInt(-1)
			);
			if( pInfo->pcView->GetSelectionInfo().IsTextSelected() ){
				/* �e�L�X�g���] */
				pInfo->pcView->DispTextSelected(
					pInfo->hdc,
					pInfo->pDispPos->GetLayoutLineRef(),
					CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
					pInfo->pDispPos->GetDrawCol()
				);
			}
		}

		return true; //goto end_of_line;
	}
	return false;
}

