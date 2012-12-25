/*!	@file
@brief CViewCommander�N���X�̃R�}���h(Diff)�֐��Q

	2007.10.25 kobake CEditView_Diff���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2002, YAZAKI, genta, MIK
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, genta
	Copyright (C) 2005, maru
	Copyright (C) 2007, kobake
	Copyright (C) 2008, kobake
	Copyright (C) 2008, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CViewCommander.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "dlg/CDlgCompare.h"
#include "dlg/CDlgDiff.h"
#include "window/CEditWnd.h"
#include "util/window.h"
#include "util/os.h"


/* �t�@�C�����e��r */
void CViewCommander::Command_COMPARE( void )
{
	HWND		hwndCompareWnd;
	TCHAR		szPath[_MAX_PATH + 1];
	CMyPoint	poDes;
	CDlgCompare	cDlgCompare;
	BOOL		bDefferent;
	const wchar_t*	pLineSrc;
	CLogicInt		nLineLenSrc;
	const wchar_t*	pLineDes;
	int			nLineLenDes;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK

	/* ��r��A���E�ɕ��ׂĕ\�� */
	cDlgCompare.m_bCompareAndTileHorz = GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz;
	BOOL bDlgCompareResult = cDlgCompare.DoModal(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		(LPARAM)GetDocument(),
		GetDocument()->m_cDocFile.GetFilePath(),
		GetDocument()->m_cDocEditor.IsModified(),
		szPath,
		&hwndCompareWnd
	);
	if( !bDlgCompareResult ){
		return;
	}
	/* ��r��A���E�ɕ��ׂĕ\�� */
	GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	if( TRUE  == GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd
	 && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		hwndMsgBox = m_pCommanderView->GetHwnd();
		GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = FALSE;
	}
	else
	{
		hwndMsgBox = hwndCompareWnd;
	}


	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint	poSrc;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&poSrc
	);

	// �J�[�\���ʒu�擾 -> poDes
	{
		::SendMessageAny( hwndCompareWnd, MYWM_GETCARETPOS, 0, 0 );
		CLogicPoint* ppoCaretDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	bDefferent = TRUE;
	pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
	/* �s(���s�P��)�f�[�^�̗v�� */
	nLineLenDes = ::SendMessageAny( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	pLineDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
	while( 1 ){
		if( pLineSrc == NULL &&	0 == nLineLenDes ){
			bDefferent = FALSE;
			break;
		}
		if( pLineSrc == NULL || 0 == nLineLenDes ){
			break;
		}
		if( nLineLenDes > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
			TopErrorMessage( m_pCommanderView->GetHwnd(),
				_T("��r��̃t�@�C��\n%ts\n%d�����𒴂���s������܂��B\n")
				_T("��r�ł��܂���B"),
				szPath,
				GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>()
			);
			return;
		}
		for( ; poSrc.x < nLineLenSrc; ){
			if( poDes.x >= nLineLenDes ){
				goto end_of_compare;
			}
			if( pLineSrc[poSrc.x] != pLineDes[poDes.x] ){
				goto end_of_compare;
			}
			poSrc.x++;
			poDes.x++;
		}
		if( poDes.x < nLineLenDes ){
			goto end_of_compare;
		}
		poSrc.x = 0;
		poSrc.y++;
		poDes.x = 0;
		poDes.y++;
		pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
		/* �s(���s�P��)�f�[�^�̗v�� */
		nLineLenDes = ::SendMessageAny( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	}
end_of_compare:;
	/* ��r��A���E�ɕ��ׂĕ\�� */
//From Here Oct. 10, 2000 JEPRO	�`�F�b�N�{�b�N�X���{�^��������Έȉ��̍s(To Here �܂�)�͕s�v�̂͂�����
//	���܂������Ȃ������̂Ō��ɖ߂��Ă���c
	if( GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz ){
		HWND* phwndArr = new HWND[2];
		phwndArr[0] = GetMainWindow();
		phwndArr[1] = hwndCompareWnd;
		
		int i;	// Jan. 28, 2002 genta ���[�v�ϐ� int�̐錾��O�ɏo�����D
				// �݊����΍�Dfor��()���Ő錾����ƌÂ��K�i�ƐV�����K�i�Ŗ�������̂ŁD
		for( i = 0; i < 2; ++i ){
			if( ::IsZoomed( phwndArr[i] ) ){
				::ShowWindow( phwndArr[i], SW_RESTORE );
			}
		}
		//	�f�X�N�g�b�v�T�C�Y�𓾂� 2002.1.24 YAZAKI
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( phwndArr[0], &rcDesktop );
		int width = (rcDesktop.right - rcDesktop.left ) / 2;
		for( i = 1; i >= 0; i-- ){
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta �^�X�N�o�[�����ɂ���ꍇ���l��
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
		delete [] phwndArr;
	}
//To Here Oct. 10, 2000

	//	2002/05/11 YAZAKI �e�E�B���h�E�����܂��ݒ肵�Ă݂�B
	if( !bDefferent ){
		TopInfoMessage( hwndMsgBox, _T("�قȂ�ӏ��͌�����܂���ł����B") );
	}
	else{
//		TopInfoMessage( hwndMsgBox, _T("�قȂ�ӏ���������܂����B") );
		/* �J�[�\�����ړ�������
			��r����́A�ʃv���Z�X�Ȃ̂Ń��b�Z�[�W���΂��B
		*/
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poDes, sizeof( poDes ) );
		::SendMessageAny( hwndCompareWnd, MYWM_SETCARETPOS, 0, 0 );

		/* �J�[�\�����ړ������� */
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poSrc, sizeof( poSrc ) );
		::PostMessageAny( GetMainWindow(), MYWM_SETCARETPOS, 0, 0 );
		TopWarningMessage( hwndMsgBox, _T("�قȂ�ӏ���������܂����B") );	// �ʒu��ύX���Ă��烁�b�Z�[�W	2008/4/27 Uchi
	}

	/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( GetMainWindow() );
	return;
}



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
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("�O��(��) �ɍ�����������܂���B") );
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
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("���(��) �ɍ�����������܂���B") );
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
