//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "debug.h"
#include <commctrl.h>
#include <stdlib.h>
#include "CLayout.h" // 2002/2/10 aroka
#include "CDocLineMgr.h" // 2002/2/10 aroka



/* ������u�� */
void CLayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg*	pArg
)
{
//	int nDeletedLineNum;	/* �폜�����s�̑��� */
//	int nInsLineNum;		/* �}���ɂ���đ������s�̐� */
//	int nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int nNewPos;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	int	nxFrom;
	int	nyFrom;
	int	nxTo;
	int	nyTo;
	CLayout* pLayout;
	CLayout* pLayoutWork;
	int nCurrentLineType;
	int nLineWork;

	int	nWork_nLines = m_nLines;	//�ύX�O�̑S�s���̕ۑ�	@@@ 2002.04.19 MIK

	/* �u���擪�ʒu�̃��C�A�E�g��� */
	pLayout = (CLayout*)Search( pArg->nDelLineFrom );
	nCurrentLineType = 0;
	pLayoutWork = pLayout;
	nLineWork = pArg->nDelLineFrom;

	if( NULL != pLayoutWork ){
		while( 0 != pLayoutWork->m_nOffset ){
			pLayoutWork = pLayoutWork->m_pPrev;
			nLineWork--;
		}
		nCurrentLineType = pLayoutWork->m_nTypePrev;
	}


	/*
	||  �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu) ��
	||  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CaretPos_Log2Phys( pArg->nDelColmFrom, pArg->nDelLineFrom, &nxFrom, &nyFrom );
	CaretPos_Log2Phys( pArg->nDelColmTo, pArg->nDelLineTo, &nxTo, &nyTo );

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	DocLineReplaceArg DLRArg;
	DLRArg.nDelLineFrom = nyFrom;			/* �폜�͈͍s  From ���s�P�ʂ̍s�ԍ� 0�J�n) */
	DLRArg.nDelPosFrom = nxFrom;			/* �폜�͈͈ʒuFrom ���s�P�ʂ̍s������̃o�C�g�ʒu 0�J�n) */
	DLRArg.nDelLineTo = nyTo;				/* �폜�͈͍s  To   ���s�P�ʂ̍s�ԍ� 0�J�n) */
	DLRArg.nDelPosTo = nxTo;				/* �폜�͈͈ʒuTo   ���s�P�ʂ̍s������̃o�C�g�ʒu 0�J�n) */
	DLRArg.pcmemDeleted = pArg->pcmemDeleted;	/* �폜���ꂽ�f�[�^��ۑ� */
//	DLRArg.nDeletedLineNum = 0;					/* �폜�����s�̑��� */
	DLRArg.pInsData = pArg->pInsData;			/* �}������f�[�^ */
	DLRArg.nInsDataLen = pArg->nInsDataLen;		/* �}������f�[�^�̒��� */
//	DLRArg.nInsLineNum = 0;						/* �}���ɂ���đ������s�̐� */
//	DLRArg.nNewLine = 0;						/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	DLRArg.nNewPos = 0;							/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	m_pcDocLineMgr->ReplaceData(
		&DLRArg
#if 0
		nyFrom,					/* �폜�͈͍s  From ���s�P�ʂ̍s�ԍ� 0�J�n) */
		nxFrom,					/* �폜�͈͈ʒuFrom ���s�P�ʂ̍s������̃o�C�g�ʒu 0�J�n) */
		nyTo,					/* �폜�͈͍s  To   ���s�P�ʂ̍s�ԍ� 0�J�n) */
		nxTo,					/* �폜�͈͈ʒuTo   ���s�P�ʂ̍s������̃o�C�g�ʒu 0�J�n) */
		pArg->pcmemDeleted,		/* �폜���ꂽ�f�[�^��ۑ� */
		&nDeletedLineNum,		/* �폜�����s�̑��� */

		pArg->pInsData,			/* �}������f�[�^ */
		pArg->nInsDataLen,		/* �}������f�[�^�̒��� */
		&nInsLineNum,			/* �}���ɂ���đ������s�̐� */
		&pArg->nNewLine,		/* �}�����ꂽ�����̎��̈ʒu�̍s */
		&pArg->nNewPos			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
#endif
	);
//	nDeletedLineNum = DLRArg.nDeletedLineNum;	/* �폜�����s�̑��� */
//	pArg->nInsLineNum = DLRArg.nInsLineNum;		/* �}���ɂ���đ������s�̐� */
	pArg->nNewLine = DLRArg.nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	pArg->nNewPos = DLRArg.nNewPos;				/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */



//	DUMP();

	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */

	int nAllLinesOld = m_nLines;
	int	nModifyLayoutLinesOld = 0;
	CLayout* pLayoutPrev;
	int nWork;
	nWork = __max( DLRArg.nDeletedLineNum, DLRArg.nInsLineNum );
//	if( 0 < nWork ){
//		--nWork;
//	}


//	DUMP();

	if( NULL != pLayoutWork ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,

			nyFrom,
			nyFrom + nWork,
			nyFrom, nxFrom,
			&nModifyLayoutLinesOld
		);

//		DUMP();

		/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
		/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
		/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
		if( 0 != DLRArg.nInsLineNum - DLRArg.nDeletedLineNum ){
			ShiftLogicalLineNum( pLayoutPrev,
				DLRArg.nInsLineNum - DLRArg.nDeletedLineNum
			);
		}
	}else{
		pLayoutPrev = m_pLayoutBot;
	}
//	DUMP();

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int	nRowNum;
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_nLinePhysical - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_nLinePhysical -
				pLayoutPrev->m_nLinePhysical - 1;
		}
	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	int nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nyFrom, nxFrom,
		nCurrentLineType,
		&nAddInsLineNum
	);

//	DUMP();

	pArg->nAddLineNum = nWork_nLines - m_nLines;	//�ύX��̑S�s���Ƃ̍���	@@@ 2002.04.19 MIK
	if( 0 == pArg->nAddLineNum )
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;/* nAddInsLineNum;*/	/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
	pArg->nModLineFrom = pArg->nDelLineFrom;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom */
	pArg->nModLineTo += ( pArg->nModLineFrom - 1 ) ;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo */

#ifdef _DEBUG
//	MYTRACE( "�폜�����s�̑���( DLRArg.nDeletedLineNum ) == %d \n" , DLRArg.nDeletedLineNum );/* �폜�����s�̑��� */
//	MYTRACE( "�}���ɂ���đ������s�̐�( DLRArg.nInsLineNum ) == %d \n" , DLRArg.nInsLineNum );/* �}���ɂ���đ������s�̐� */
//	MYTRACE( "�}�����ꂽ�����̎��̈ʒu�̍s( nNewLine ) == %d \n" , nNewLine );/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	MYTRACE( "�}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu( nNewPos ) == %d \n" , nNewPos );/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
//	MYTRACE( "nModifyLayoutLinesOld == %d \n" , nModifyLayoutLinesOld );

//	MYTRACE( "\n\n����������������������������\n" );
//	MYTRACE( "�ĕ`��q���g ���C�A�E�g�s�̑���(pArg->nAddLineNum) == %d \n" , pArg->nAddLineNum );
//	MYTRACE( "�ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(pArg->nModLineFrom) == %d \n" , pArg->nModLineFrom );
//	MYTRACE( "�ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo(pArg->nModLineTo) == %d \n" , pArg->nModLineTo );
#endif

//	DoLayout( NULL, bDispSSTRING, bDispWSTRING );

	/* ���C�A�E�g�ʒu�ւ̕ϊ� */
	CaretPos_Phys2Log( pArg->nNewPos, pArg->nNewLine, &pArg->nNewPos, &pArg->nNewLine );

	return;
}


/*[EOF]*/
