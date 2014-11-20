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
#include "CViewCommander_inline.h"

#include "dlg/CDlgCompare.h"
#include "dlg/CDlgDiff.h"
#include "charset/CCodeMediator.h"
#include "charset/CCodePage.h"
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
		szPath,
		&hwndCompareWnd
	);
	if( !bDlgCompareResult ){
		return;
	}
	/* ��r��A���E�ɕ��ׂĕ\�� */
	GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	if( FALSE != GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd
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
	for (;;) {
		if( pLineSrc == NULL &&	0 == nLineLenDes ){
			bDefferent = FALSE;
			break;
		}
		if( pLineSrc == NULL || 0 == nLineLenDes ){
			break;
		}
		if( nLineLenDes > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
			TopErrorMessage( m_pCommanderView->GetHwnd(),
				LS( STR_ERR_CMPERR ), // "��r��̃t�@�C��\n%ts\n%d�����𒴂���s������܂��B\n��r�ł��܂���B"
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
		TopInfoMessage( hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD22) );
	}
	else{
//		TopInfoMessage( hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23) );
		/* �J�[�\�����ړ�������
			��r����́A�ʃv���Z�X�Ȃ̂Ń��b�Z�[�W���΂��B
		*/
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poDes, sizeof( poDes ) );
		::SendMessageAny( hwndCompareWnd, MYWM_SETCARETPOS, 0, 0 );

		/* �J�[�\�����ړ������� */
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poSrc, sizeof( poSrc ) );
		::PostMessageAny( GetMainWindow(), MYWM_SETCARETPOS, 0, 0 );
		TopWarningMessage( hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23) );	// �ʒu��ύX���Ă��烁�b�Z�[�W	2008/4/27 Uchi
	}

	/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( GetMainWindow() );
	return;
}



static ECodeType GetFileCharCode( LPCTSTR pszFile )
{
	const STypeConfigMini* typeMini;
	CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &typeMini );
	return CCodeMediator(typeMini->m_encoding).CheckKanjiCodeOfFile( pszFile );
}



static ECodeType GetDiffCreateTempFileCode(ECodeType code)
{
	EEncodingTrait e = CCodePage::GetEncodingTrait(code);
	if( e != ENCODING_TRAIT_ASCII ){
		return CODE_UTF8;
	}
	return code;
}



/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	maru
	@date	2005/10/28 ����܂ł�Command_Diff��m_pCommanderView->ViewDiffInfo�ɖ��̕ύX
*/
void CViewCommander::Command_Diff( const WCHAR* _szDiffFile2, int nFlgOpt )
{
	const std::tstring strDiffFile2 = to_tchar(_szDiffFile2);
	const TCHAR* szDiffFile2 = strDiffFile2.c_str();

	bool	bTmpFile1 = false;
	TCHAR	szTmpFile1[_MAX_PATH * 2];

	if( !IsFileExists( szDiffFile2, true ) )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF1) );
		return;
	}

	//���t�@�C��
	// 2013.06.21 Unicode�̂Ƃ��́A�����t�@�C���o��
	ECodeType code = GetDocument()->GetDocumentEncoding();
	ECodeType saveCode = GetDiffCreateTempFileCode(code);
	ECodeType code2 = GetFileCharCode(szDiffFile2);
	ECodeType saveCode2 = GetDiffCreateTempFileCode(code2);
	// 2014.10.24 �R�[�h���Ⴄ�Ƃ��͕K��UTF-8�t�@�C���o��
	if( saveCode != saveCode2 ){
		saveCode = CODE_UTF8;
		saveCode2 = CODE_UTF8;
	}

	if( GetDocument()->m_cDocEditor.IsModified()
		|| saveCode != code
		|| !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() // 2014.06.25 Grep/�A�E�g�v�b�g���Ώۂɂ���
	){
		if( !m_pCommanderView->MakeDiffTmpFile(szTmpFile1, NULL, saveCode, GetDocument()->GetDocumentBomExist()) ){
			return;
		}
		bTmpFile1 = true;
	}else{
		_tcscpy( szTmpFile1, GetDocument()->m_cDocFile.GetFilePath() );
	}

	bool bTmpFile2 = false;
	TCHAR	szTmpFile2[_MAX_PATH * 2];
	bool bTmpFileMode = code2 != saveCode2;
	if( !bTmpFileMode ){
		_tcscpy(szTmpFile2, szDiffFile2);
	}else if( m_pCommanderView->MakeDiffTmpFile2( szTmpFile2, szDiffFile2, code2, saveCode2 ) ){
		bTmpFile2 = true;
	}else{
		if( bTmpFile1 ) _tunlink( szTmpFile1 );
		return;
	}

	bool bUTF8io = true;
	if( saveCode == CODE_SJIS ){
		bUTF8io = false;
	}

	//�����\��
	m_pCommanderView->ViewDiffInfo(szTmpFile1, szTmpFile2, nFlgOpt, bUTF8io);

	//�ꎞ�t�@�C�����폜����
	if( bTmpFile1 ) _tunlink( szTmpFile1 );
	if( bTmpFile2 ) _tunlink( szTmpFile2 );

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
		GetDocument()->m_cDocFile.GetFilePath()
	);
	if( !nDiffDlgResult ){
		return;
	}
	
	//���t�@�C��
	TCHAR	szTmpFile1[_MAX_PATH * 2];
	ECodeType code = GetDocument()->GetDocumentEncoding();
	ECodeType saveCode = GetDiffCreateTempFileCode(code);
	ECodeType code2 = cDlgDiff.m_nCodeTypeDst;
	if( CODE_ERROR == code2 ){
		if( cDlgDiff.m_szFile2[0] != _T('\0') ){
			// �t�@�C�����w��
			code2 = GetFileCharCode(cDlgDiff.m_szFile2);
		}
	}
	ECodeType saveCode2 = GetDiffCreateTempFileCode(code2);
	// 2014.10.24 �R�[�h���Ⴄ�Ƃ��͕K��UTF-8�t�@�C���o��
	if( saveCode != saveCode2 ){
		saveCode = CODE_UTF8;
		saveCode2 = CODE_UTF8;
	}
	if( GetDocument()->m_cDocEditor.IsModified()
			|| code != saveCode
			|| !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() // 2014.06.25 Grep/�A�E�g�v�b�g���Ώۂɂ���
	){
		if( !m_pCommanderView->MakeDiffTmpFile( szTmpFile1, NULL, saveCode, GetDocument()->GetDocumentBomExist() ) ){ return; }
		bTmpFile1 = true;
	}else{
		_tcscpy( szTmpFile1, GetDocument()->m_cDocFile.GetFilePath() );
	}
		
	//����t�@�C��
	// UNICODE,UNICODEBE�̏ꍇ�͏�Ɉꎞ�t�@�C����UTF-8�ɂ���
	TCHAR	szTmpFile2[_MAX_PATH * 2];
	// 2014.06.25 �t�@�C�������Ȃ�(=����,Grep,�A�E�g�v�b�g)��TmpFileMode�ɂ���
	bool bTmpFileMode = cDlgDiff.m_bIsModifiedDst || code2 != saveCode2 || cDlgDiff.m_szFile2[0] == _T('\0');
	if( !bTmpFileMode ){
		// ���ύX�Ńt�@�C�������ASCII�n�R�[�h�̏ꍇ�̂�,���̂܂܃t�@�C���𗘗p����
		_tcscpy( szTmpFile2, cDlgDiff.m_szFile2 );
	}else if( cDlgDiff.m_hWnd_Dst ){
		// �t�@�C���ꗗ����I��
		if( m_pCommanderView->MakeDiffTmpFile( szTmpFile2, cDlgDiff.m_hWnd_Dst, saveCode2, cDlgDiff.m_bBomDst ) ){
			bTmpFile2 = true;
		}else {
			if( bTmpFile1 ) _tunlink( szTmpFile1 );
			return;
		}
	}else{
		// �t�@�C�����w��Ŕ�ASCII�n�������ꍇ
		if( m_pCommanderView->MakeDiffTmpFile2( szTmpFile2, cDlgDiff.m_szFile2, code2, saveCode2 ) ){
			bTmpFile2 = true;
		}else{
			// Error
			if( bTmpFile1 ) _tunlink( szTmpFile1 );
			return;
		}
	}
	
	bool bUTF8io = true;
	if( saveCode == CODE_SJIS ){
		bUTF8io = false;
	}

	//�����\��
	m_pCommanderView->ViewDiffInfo(szTmpFile1, szTmpFile2, cDlgDiff.m_nDiffFlgOpt, bUTF8io);
	
	
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
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
		}

		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptXY_Layout );
		}
		GetCaret().MoveCursor( ptXY_Layout, true );
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
		if( nYOld_Logic >= ptXY.y ) m_pCommanderView->SendStatusMessage( LS(STR_ERR_SRNEXT1) );
	}
	else{
		m_pCommanderView->SendStatusMessage( LS(STR_ERR_SRNEXT2) );
		AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_DIFF_NEXT_NOT_FOUND) );
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
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
		}

		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptXY_Layout );
		}
		GetCaret().MoveCursor( ptXY_Layout, true );
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
		if( nYOld_Logic <= ptXY.y ) m_pCommanderView->SendStatusMessage( LS(STR_ERR_SRPREV1) );
	}
	else{
		m_pCommanderView->SendStatusMessage( LS(STR_ERR_SRPREV2) );
		AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_DIFF_PREV_NOT_FOUND) );
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
