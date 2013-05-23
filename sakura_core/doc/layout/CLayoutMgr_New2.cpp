/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "CLayoutMgr.h"
#include "CLayout.h" // 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h" // 2002/2/10 aroka
#include "charset/charcode.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "CSearchAgent.h"



/* ������u�� */
void CLayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg*	pArg
)
{
	CLayoutInt	nWork_nLines = m_nLines;	//�ύX�O�̑S�s���̕ۑ�	@@@ 2002.04.19 MIK

	/* �u���擪�ʒu�̃��C�A�E�g��� */
	CLayout*		pLayout = SearchLineByLayoutY( pArg->sDelRange.GetFrom().GetY2() );
	EColorIndexType	nCurrentLineType = COLORIDX_DEFAULT;
	CLayoutInt		nLineWork = pArg->sDelRange.GetFrom().GetY2();

	CLayout*		pLayoutWork = pLayout;
	if( pLayoutWork ){
		while( 0 != pLayoutWork->GetLogicOffset() ){
			pLayoutWork = pLayoutWork->GetPrevLayout();
			nLineWork--;
		}
		nCurrentLineType = pLayoutWork->GetColorTypePrev();
	}else if( GetLineCount() == pArg->sDelRange.GetFrom().GetY2() ){
		// 2012.01.05 �ŏI�s��Redo/Undo�ł̐F�������������Ȃ��̂��C��
		nCurrentLineType = m_nLineTypeBot;
	}


	/*
	||  �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu) ��
	||  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint ptFrom;
	CLogicPoint ptTo;
	LayoutToLogic( pArg->sDelRange.GetFrom(), &ptFrom );
	LayoutToLogic( pArg->sDelRange.GetTo(), &ptTo );

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	DocLineReplaceArg DLRArg;
	DLRArg.sDelRange.SetFrom(ptFrom);	//�폜�͈�from
	DLRArg.sDelRange.SetTo(ptTo);		//�폜�͈�to
	DLRArg.pcmemDeleted = pArg->pcmemDeleted;	// �폜���ꂽ�f�[�^��ۑ�
	DLRArg.pInsData = pArg->pInsData;			// �}������f�[�^
	DLRArg.nInsDataLen = pArg->nInsDataLen;		// �}������f�[�^�̒���
	CSearchAgent(m_pcDocLineMgr).ReplaceData(
		&DLRArg
	);


	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */

	CLayoutInt	nModifyLayoutLinesOld = CLayoutInt(0);
	CLayout* pLayoutPrev;
	CLogicInt nWork;
	nWork = t_max( DLRArg.nDeletedLineNum, DLRArg.nInsLineNum );


	if( pLayoutWork ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			ptFrom.GetY2(),
			ptFrom.GetY2() + nWork,
			ptFrom,
			&nModifyLayoutLinesOld
		);

		/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
		/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
		/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
		if( 0 != DLRArg.nInsLineNum - DLRArg.nDeletedLineNum ){
			ShiftLogicalLineNum(
				pLayoutPrev,
				DLRArg.nInsLineNum - DLRArg.nDeletedLineNum
			);
		}
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	CLogicInt	nRowNum;
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->GetLogicLineNo();
		}
	}
	else{
		if( NULL == pLayoutPrev->GetNextLayout() ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->GetLogicLineNo() - CLogicInt(1);
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->GetLogicLineNo() -
				pLayoutPrev->GetLogicLineNo() - CLogicInt(1);
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = pArg->sDelRange.GetFrom().GetY2();		// �ҏW�J�n�s
	ctwArg.nColumnFrom  = pArg->sDelRange.GetFrom().GetX2();		// �ҏW�J�n��
	ctwArg.nDelLines    = pArg->sDelRange.GetTo().GetY2() - pArg->sDelRange.GetFrom().GetY2();	// �폜�s�� - 1
	ctwArg.nAllLinesOld = nWork_nLines;								// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = pArg->nInsDataLen ? TRUE : FALSE;			// �ǉ�������̗L��

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	CLayoutInt nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		ptFrom,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	pArg->nAddLineNum = nWork_nLines - m_nLines;	//�ύX��̑S�s���Ƃ̍���	@@@ 2002.04.19 MIK
	if( 0 == pArg->nAddLineNum )
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
	pArg->nModLineFrom = pArg->sDelRange.GetFrom().GetY2();	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom */
	pArg->nModLineTo += ( pArg->nModLineFrom - CLayoutInt(1) ) ;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo */

	//2007.10.18 kobake LayoutReplaceArg::ptLayoutNew�͂����ŎZ�o����̂�������
	LogicToLayout(DLRArg.ptNewPos, &pArg->ptLayoutNew); // �}�����ꂽ�����̎��̈ʒu
}



