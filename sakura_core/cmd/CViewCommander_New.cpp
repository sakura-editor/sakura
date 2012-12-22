#include "StdAfx.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include <string>///	2002/2/3 aroka 
#include <vector> ///	2002/2/3 aroka
#include "CViewCommander.h"
#include "view/CEditView.h"
#include "CWaitCursor.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///	2002/2/3 aroka 
#include "doc/CLayout.h"///	2002/2/3 aroka 
#include "doc/CDocLine.h"///	2002/2/3 aroka 
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "debug/Debug.h"///	2002/2/3 aroka 
#include "_os/COsVersionInfo.h"   // 2002.04.09 minfu 
#include "dlg/CDlgCtrlCode.h"	//�R���g���[���R�[�h�̓���(�_�C�A���O)
#include "dlg/CDlgFavorite.h"	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "dlg/CDlgCancel.h"	// 2006.12.09 maru
#include "window/CEditWnd.h"
#include "io/CFileLoad.h"	// 2006.12.09 maru
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

using namespace std; // 2002/2/3 aroka to here



#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif


// 2001/06/20 Start by asa-o

// �e�L�X�g���P�s���փX�N���[��
void CViewCommander::Command_WndScrollDown( void )
{
	CLayoutInt	nCaretMarginY;

	nCaretMarginY = m_pCommanderView->GetTextArea().m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = CLayoutInt(1);

	nCaretMarginY += 2;

	if( GetCaret().GetCaretLayoutPos().GetY() > m_pCommanderView->GetTextArea().m_nViewRowNum + m_pCommanderView->GetTextArea().GetViewTopLine() - (nCaretMarginY + 1) ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV(m_pCommanderView->GetTextArea().GetViewTopLine() - CLayoutInt(1)));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( GetCaret().GetCaretLayoutPos().GetY() > m_pCommanderView->GetTextArea().m_nViewRowNum + m_pCommanderView->GetTextArea().GetViewTopLine() - nCaretMarginY )
		{
			if( GetCaret().GetCaretLayoutPos().GetY() > GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				GetCaret().Cursor_UPDOWN( (GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - GetCaret().GetCaretLayoutPos().GetY2(), FALSE );
			else
				GetCaret().Cursor_UPDOWN( CLayoutInt(-1), FALSE);
			GetCaret().ShowCaretPosInfo();
		}
	}

	GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
}

// �e�L�X�g���P�s��փX�N���[��
void CViewCommander::Command_WndScrollUp(void)
{
	CLayoutInt	nCaretMarginY;

	nCaretMarginY = m_pCommanderView->GetTextArea().m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( GetCaret().GetCaretLayoutPos().GetY2() < m_pCommanderView->GetTextArea().GetViewTopLine() + (nCaretMarginY + 1) ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( m_pCommanderView->GetTextArea().GetViewTopLine() + CLayoutInt(1) ));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( GetCaret().GetCaretLayoutPos().GetY() < m_pCommanderView->GetTextArea().GetViewTopLine() + nCaretMarginY )
		{
			if( m_pCommanderView->GetTextArea().GetViewTopLine() == 1 )
				GetCaret().Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				GetCaret().Cursor_UPDOWN( CLayoutInt(1), FALSE );
			GetCaret().ShowCaretPosInfo();
		}
	}

	GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
}

// 2001/06/20 End



/* ���̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CViewCommander::Command_GONEXTPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF�s�ł����B
		return;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			}
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	CLayoutPoint ptCaretPos_Layo;

	/* �ړ��O�̕����ʒu */
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	/* �ړ���̕����ʒu */
	CLayoutPoint ptCaretPos_Layo_CaretPointer;
	//int nCaretPosY_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + CLogicPoint(0,nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN( ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect );
}

/* �O�̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CViewCommander::Command_GOPREVPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			}
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;
		}
		else {
			nCaretPointer++;
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	CLayoutPoint ptCaretPos_Layo;

	/* �ړ��O�̕����ʒu */
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	/* �ړ���̕����ʒu */
	CLayoutPoint ptCaretPos_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + CLogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN( ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect );
}

// From Here 2001.12.03 hor

//! �u�b�N�}�[�N�̐ݒ�E�������s��(�g�O������)
void CViewCommander::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y<m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().y ){
		CLogicPoint ptFrom;
		CLogicPoint ptTo;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y),
			&ptFrom
		);
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().y  ),
			&ptTo
		);
		for(CLogicInt nY=ptFrom.GetY2();nY<=ptTo.y;nY++){
			pCDocLine=GetDocument()->m_cDocLineMgr.GetLine( nY );
			CBookmarkSetter cBookmark(pCDocLine);
			if(pCDocLine)cBookmark.SetBookmark(!cBookmark.IsBookmarked());
		}
	}
	else{
		pCDocLine=GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() );
		CBookmarkSetter cBookmark(pCDocLine);
		if(pCDocLine)cBookmark.SetBookmark(!cBookmark.IsBookmarked());
	}

	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}



//! ���̃u�b�N�}�[�N��T���C����������ړ�����
void CViewCommander::Command_BOOKMARK_NEXT(void)
{
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld=ptXY.y;					// hor

re_do:;								// hor
	if(CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_FORWARD, &tmp_y)){
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		m_pCommanderView->MoveCursorSelecting( ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// ������Ȃ�����
			bRedo			// �ŏ��̌���
		){
			ptXY.y=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// �擪����Č���
		}
	}
	if(bFound){
		if(nYOld >= ptXY.y)m_pCommanderView->SendStatusMessage(_T("���擪����Č������܂���"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("��������܂���ł���"));
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("�O��(��) �Ƀu�b�N�}�[�N��������܂���B"));
	}
	return;
}



//! �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void CViewCommander::Command_BOOKMARK_PREV(void)
{
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0,GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld=ptXY.y;						// hor

re_do:;								// hor
	if(CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_BACKWARD, &tmp_y)){
		ptXY.y = tmp_y;
		bFound = TRUE;				// hor
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		m_pCommanderView->MoveCursorSelecting( ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		){
			// 2011.02.02 m_cLayoutMgr��m_cDocLineMgr
			ptXY.y= GetDocument()->m_cDocLineMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if(bFound){
		if(nYOld <= ptXY.y)m_pCommanderView->SendStatusMessage(_T("����������Č������܂���"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("��������܂���ł���"));
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("���(��) �Ƀu�b�N�}�[�N��������܂���B") );
	}
	return;
}



//! �u�b�N�}�[�N���N���A����
void CViewCommander::Command_BOOKMARK_RESET(void)
{
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).ResetAllBookMark();
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}


//�w��p�^�[���Ɉ�v����s���}�[�N 2002.01.16 hor
//�L�[�}�N���ŋL�^�ł���悤��	2002.02.08 hor
void CViewCommander::Command_BOOKMARK_PATTERN( void )
{
	//����or�u���_�C�A���O����Ăяo���ꂽ
	if( !m_pCommanderView->ChangeCurRegexp(false) ) return;
	
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).MarkSearchWord(
		m_pCommanderView->m_strCurSearchKey.c_str(),		// ��������
		m_pCommanderView->m_sCurSearchOption,	// ��������
		&m_pCommanderView->m_CurRegexp							// ���K�\���R���p�C���f�[�^
	);
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}



/*!	�����J�n�ʒu�֖߂�
	@author	ai
	@date	02/06/26
*/
void CViewCommander::Command_JUMP_SRCHSTARTPOS(void)
{
	if( m_pCommanderView->m_ptSrchStartPos_PHY.BothNatural() )
	{
		CLayoutPoint pt;
		/* �͈͑I�𒆂� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_ptSrchStartPos_PHY,
			&pt
		);
		//	2006.07.09 genta �I����Ԃ�ۂ�
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
	else
	{
		ErrorBeep();
	}
	return;
}
