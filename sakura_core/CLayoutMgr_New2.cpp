//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include <stdlib.h>



/* ������u�� */
void CLayoutMgr::ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
#if 0
		int			nDelLineFrom,			/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
		int			nDelColmFrom,			/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
		int			nDelLineTo,				/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
		int			nDelColmTo,				/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
		CMemory*	pcmemDeleted,			/* �폜���ꂽ�f�[�^ */
		const char*	pInsData,				/* �}������f�[�^ */
		int			nInsDataLen,			/* �}������f�[�^�̒��� */

		int*		pnAddLineNum,			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
		int*		pnModLineFrom,			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
		int*		pnModLineTo,			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */

		int*		pnNewLine,				/* �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
		int*		pnNewPos,				/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */

		BOOL		bDispSSTRING,			/* �V���O���N�H�[�e�[�V�����������\������ */
		BOOL		bDispWSTRING,			/* �_�u���N�H�[�e�[�V�����������\������ */
		BOOL		bUndo					/* Undo���삩�ǂ��� */
#endif
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
	int	nModifyLayoutLinesOld;
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

//	if( NULL != pLayoutPrev ){
//		pLayoutNext	= pLayoutPrev->m_pNext;
//	}else{
//		pLayoutNext	= m_pLayoutBot;
//	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
//	*pnModifyLayoutLinesNew = DoLayout3(
//		pLayoutPrev, nRowNum,
//		nDelStartLogicalLine, nDelStartLogicalPos
//	);

//	MYTRACE( "pLayoutPrev=[%xh]\n", pLayoutPrev );
//	MYTRACE( "pLayoutNext=[%xh]\n", pLayoutNext );
	int nAddInsLineNum;
	pArg->nModLineTo = DoLayout3_New(
		pLayoutPrev,
//		pLayoutNext,
		nRowNum,
		nyFrom, nxFrom,
		nCurrentLineType,
		&nAddInsLineNum,
		pArg->bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
		pArg->bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
	);


//	DUMP();

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
