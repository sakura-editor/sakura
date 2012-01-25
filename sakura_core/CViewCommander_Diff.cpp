#include "StdAfx.h"
#include "CViewCommander.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "dlg/CDlgDiff.h"
#include "window/CEditWnd.h"

/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	maru
	@date	2005/10/28 ����܂ł�Command_Diff��m_pCommanderView->ViewDiffInfo�ɖ��̕ύX
*/
void CViewCommander::Command_Diff( const WCHAR* _szTmpFile2, int nFlgOpt )
{
	const TCHAR* szTmpFile2 = to_tchar(_szTmpFile2);

	bool	bTmpFile1 = false;
	TCHAR	szTmpFile1[_MAX_PATH * 2];

	if( -1 == ::GetFileAttributes( szTmpFile2 ) )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T("�����R�}���h���s�͎��s���܂����B\n\n��r����t�@�C����������܂���B") );
		return;
	}

	//���t�@�C��
	if (!GetDocument()->m_cDocEditor.IsModified()) _tcscpy( szTmpFile1, GetDocument()->m_cDocFile.GetFilePath());
	else if (m_pCommanderView->MakeDiffTmpFile ( szTmpFile1, NULL )) bTmpFile1 = true;
	else return;

	//�����\��
	m_pCommanderView->ViewDiffInfo(szTmpFile1, szTmpFile2, nFlgOpt);

	//�ꎞ�t�@�C�����폜����
	if( bTmpFile1 ) _tunlink( szTmpFile1 );

	return;

}

/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�����)
	@author	MIK
	@date	2002/05/25
	@date	2002/11/09 �ҏW���t�@�C��������
	@date	2005/10/29 maru �ꎞ�t�@�C���쐬������m_pCommanderView->MakeDiffTmpFile�ֈړ�
*/
void CViewCommander::Command_Diff_Dialog( void )
{
	CDlgDiff	cDlgDiff;
	bool	bTmpFile1 = false, bTmpFile2 = false;

	//DIFF�����\���_�C�A���O��\������
	int nDiffDlgResult = cDlgDiff.DoModal(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		(LPARAM)GetDocument(),
		GetDocument()->m_cDocFile.GetFilePath(),
		GetDocument()->m_cDocEditor.IsModified()
	);
	if( !nDiffDlgResult ){
		return;
	}
	
	//���t�@�C��
	TCHAR	szTmpFile1[_MAX_PATH * 2];
	if (!GetDocument()->m_cDocEditor.IsModified()) _tcscpy( szTmpFile1, GetDocument()->m_cDocFile.GetFilePath());
	else if (m_pCommanderView->MakeDiffTmpFile ( szTmpFile1, NULL )) bTmpFile1 = true;
	else return;
		
	//����t�@�C��
	TCHAR	szTmpFile2[_MAX_PATH * 2];
	if (!cDlgDiff.m_bIsModifiedDst ) _tcscpy( szTmpFile2, cDlgDiff.m_szFile2);
	else if (m_pCommanderView->MakeDiffTmpFile ( szTmpFile2, cDlgDiff.m_hWnd_Dst )) bTmpFile2 = true;
	else 
	{
		if( bTmpFile1 ) _tunlink( szTmpFile1 );
		return;
	}
	
	//�����\��
	m_pCommanderView->ViewDiffInfo(szTmpFile1, szTmpFile2, cDlgDiff.m_nDiffFlgOpt);
	
	
	//�ꎞ�t�@�C�����폜����
	if( bTmpFile1 ) _tunlink( szTmpFile1 );
	if( bTmpFile2 ) _tunlink( szTmpFile2 );

	return;
}


/*!	���̍�����T���C����������ړ�����
*/
void CViewCommander::Command_Diff_Next( void )
{
	BOOL		bFound = FALSE;
	BOOL		bRedo = TRUE;

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int			nYOld_Logic = ptXY.y;
	CLogicInt tmp_y;

re_do:;	
	if( CDiffLineMgr(&GetDocument()->m_cDocLineMgr).SearchDiffMark( ptXY.GetY2(), SEARCH_FORWARD, &tmp_y ) ){
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptXY_Layout;
		GetDocument()->m_cLayoutMgr.LogicToLayout( ptXY, &ptXY_Layout );
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
		else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}

		GetCaret().MoveCursor( ptXY_Layout, TRUE );
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptXY_Layout );
		}
	}


	if( GetDllShareData().m_Common.m_sSearch.m_bSearchAll ){
		// ������Ȃ������B���A�ŏ��̌���
		if( !bFound	&& bRedo ){
			ptXY.y = 0 - 1;	// 1��O���w��
			bRedo = FALSE;
			goto re_do;		// �擪����Č���
		}
	}

	if( bFound ){
		if( nYOld_Logic >= ptXY.y ) m_pCommanderView->SendStatusMessage( _T("���擪����Č������܂���") );
	}
	else{
		m_pCommanderView->SendStatusMessage( _T("��������܂���ł���") );
		if( GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND )	/* ������Ȃ��Ƃ����b�Z�[�W��\�� */
			InfoMessage( m_pCommanderView->GetHwnd(), _T("�O��(��) �ɍ�����������܂���B") );
	}

	return;
}



/*!	�O�̍�����T���C����������ړ�����
*/
void CViewCommander::Command_Diff_Prev( void )
{
	BOOL		bFound = FALSE;
	BOOL		bRedo = TRUE;

	CLogicPoint	ptXY(0,GetCaret().GetCaretLogicPos().y);
	int			nYOld_Logic = ptXY.y;
	CLogicInt tmp_y;

re_do:;
	if( CDiffLineMgr(&GetDocument()->m_cDocLineMgr).SearchDiffMark( ptXY.GetY2(), SEARCH_BACKWARD, &tmp_y) ){
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptXY_Layout;
		GetDocument()->m_cLayoutMgr.LogicToLayout( ptXY, &ptXY_Layout );
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
		else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}

		GetCaret().MoveCursor( ptXY_Layout, TRUE );
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptXY_Layout );
		}
	}

	if( GetDllShareData().m_Common.m_sSearch.m_bSearchAll ){
		// ������Ȃ������A���A�ŏ��̌���
		if( !bFound	&& bRedo ){
			// 2011.02.02 m_cLayoutMgr��m_cDocLineMgr
			ptXY.y = GetDocument()->m_cDocLineMgr.GetLineCount();	// 1��O���w��
			bRedo = FALSE;
			goto re_do;	// ��������Č���
		}
	}

	if( bFound ){
		if( nYOld_Logic <= ptXY.y ) m_pCommanderView->SendStatusMessage( _T("����������Č������܂���") );
	}
	else{
		m_pCommanderView->SendStatusMessage( _T("��������܂���ł���") );
		if( GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND )	/* ������Ȃ��Ƃ����b�Z�[�W��\�� */
			InfoMessage( m_pCommanderView->GetHwnd(), _T("���(��) �ɍ�����������܂���B") );
	}

	return;
}

/*!	�����\���̑S����
	@author	MIK
	@date	2002/05/26
*/
void CViewCommander::Command_Diff_Reset( void )
{
	CDiffLineMgr(&GetDocument()->m_cDocLineMgr).ResetAllDiffMark();

	//���������r���[���X�V
	GetEditWindow()->Views_Redraw();
	return;
}
