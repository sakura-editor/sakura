/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�ݒ�n)�֐��Q

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "typeprop/CDlgTypeList.h"
#include "dlg/CDlgFavorite.h"	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "CEditApp.h"
#include "util/shell.h"
#include "CPropertyManager.h"
#include "util/window.h"


/*! �c�[���o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutToolBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CViewCommander::Command_SHOWTOOLBAR( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispTOOLBAR = ((NULL == pCEditWnd->m_cToolbar.GetToolbarHwnd())? TRUE: FALSE);	/* �c�[���o�[�\�� */
	pCEditWnd->LayoutToolBar();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TOOLBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! �t�@���N�V�����L�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutFuncKey(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CViewCommander::Command_SHOWFUNCKEY( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispFUNCKEYWND = ((NULL == pCEditWnd->m_CFuncKeyWnd.GetHwnd())? TRUE: FALSE);	/* �t�@���N�V�����L�[�\�� */
	pCEditWnd->LayoutFuncKey();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_FUNCKEY,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! �^�u(�E�C���h�E)�̕\��/��\��

	@author MIK
	@date 2003.06.10 �V�K�쐬
	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutTabBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
	@date 2007.06.20 ryoji �O���[�vID���Z�b�g
 */
void CViewCommander::Command_SHOWTAB( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd = ((NULL == pCEditWnd->m_cTabWnd.GetHwnd())? TRUE: FALSE);	/* �^�u�o�[�\�� */
	pCEditWnd->LayoutTabBar();
	pCEditWnd->EndLayoutBars();

	// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����	// 2007.05.18 ryoji
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		GetDocument()->m_pcEditWnd->WindowTopMost(
			( (DWORD)::GetWindowLongPtr( GetDocument()->m_pcEditWnd->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
		);
	}

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeManager::getInstance()->ResetGroupId();
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TAB,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/*! �X�e�[�^�X�o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutStatusBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CViewCommander::Command_SHOWSTATUSBAR( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispSTATUSBAR = ((NULL == pCEditWnd->m_cStatusBar.GetStatusHwnd())? TRUE: FALSE);	/* �X�e�[�^�X�o�[�\�� */
	pCEditWnd->LayoutStatusBar();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_STATUSBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}



/* �^�C�v�ʐݒ�ꗗ */
void CViewCommander::Command_TYPE_LIST( void )
{
	CDlgTypeList			cDlgTypeList;
	CDlgTypeList::SResult	sResult;
	sResult.cDocumentType = GetDocument()->m_cDocType.GetDocumentType();
	if( cDlgTypeList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), &sResult ) ){
		//	Nov. 29, 2000 genta
		//	�ꎞ�I�Ȑݒ�K�p�@�\�𖳗���ǉ�
		if( sResult.bTempChange ){
			GetDocument()->m_cDocType.SetDocumentType( sResult.cDocumentType, true );
			GetDocument()->m_cDocType.LockDocumentType();
			GetDocument()->OnChangeType();
		}
		else{
			/* �^�C�v�ʐݒ� */
			CEditApp::getInstance()->m_pcPropertyManager->OpenPropertySheetTypes( -1, sResult.cDocumentType );
		}
	}
	return;
}



/* �^�C�v�ʐݒ� */
void CViewCommander::Command_OPTION_TYPE( void )
{
	CEditApp::getInstance()->m_pcPropertyManager->OpenPropertySheetTypes( -1, GetDocument()->m_cDocType.GetDocumentType() );
}



/* ���ʐݒ� */
void CViewCommander::Command_OPTION( void )
{
	/* �ݒ�v���p�e�B�V�[�g �e�X�g�p */
	CEditApp::getInstance()->m_pcPropertyManager->OpenPropertySheet( -1/*, -1*/ );
}



/* �t�H���g�ݒ� */
void CViewCommander::Command_FONT( void )
{
	HWND	hwndFrame;
	hwndFrame = GetMainWindow();

	/* �t�H���g�ݒ�_�C�A���O */
	LOGFONT lf = GetDllShareData().m_Common.m_sView.m_lf;
	INT nPointSize;
#ifdef USE_UNFIXED_FONT
	bool bFixedFont = false;
#else
	bool bFixedFont = true;
#endif
	if( MySelectFont( &lf, &nPointSize, CEditWnd::getInstance()->m_cSplitterWnd.GetHwnd(), bFixedFont ) ){
		GetDllShareData().m_Common.m_sView.m_lf = lf;
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;

		// �������L���b�V���̏�����	// 2008/5/15 Uchi
		InitCharWidthCache(lf);

//		/* �ύX�t���O �t�H���g */
//		GetDllShareData().m_bFontModify = TRUE;

		if( GetDllShareData().m_Common.m_sView.m_lf.lfPitchAndFamily & FIXED_PITCH  ){
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		}else{
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = FALSE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		}
		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONT, hwndFrame
		);

		/* �L�����b�g�̕\�� */
//		::HideCaret( GetHwnd() );
//		::ShowCaret( GetHwnd() );

//		/* �A�N�e�B�u�ɂ��� */
//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndFrame );
	}
	return;
}



/*! �t�H���g�T�C�Y�ݒ�
	@param fontSize �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj
	@param shift �t�H���g�T�C�Y���g��or�k�����邽�߂̕ύX��(fontSize=0�̂Ƃ��L��)

	@note TrueType�̂݃T�|�[�g

	@date 2013.04.10 novice �V�K�쐬
*/
void CViewCommander::Command_SETFONTSIZE( int fontSize, int shift )
{
	// The point sizes recommended by "The Windows Interface: An Application Design Guide", 1/10�|�C���g�P��
	static const INT sizeTable[] = { 8*10, 9*10, 10*10, (INT)(10.5*10), 11*10, 12*10, 14*10, 16*10, 18*10, 20*10, 22*10, 24*10, 26*10, 28*10, 36*10, 48*10, 72*10 };
	LOGFONT& lf = GetDllShareData().m_Common.m_sView.m_lf;
	INT nPointSize;

	// TrueType�̂ݑΉ�
	if( OUT_STROKE_PRECIS != lf.lfOutPrecision) {
		return;
	}

	if( 0 != fontSize ){
		// �t�H���g�T�C�Y�𒼐ڑI������ꍇ
		nPointSize = t_max(sizeTable[0], t_min(sizeTable[_countof(sizeTable)-1], fontSize));

		// �V�����t�H���g�T�C�Y�ݒ�
		lf.lfHeight = DpiPointsToPixels(-nPointSize, 10);
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;
	} else if( 0 != shift ) {
		// ���݂̃t�H���g�ɑ΂��āA�k��or�g�債���t�H���g�I������ꍇ
		nPointSize = GetDllShareData().m_Common.m_sView.m_nPointSize;

		// �t�H���g�̊g��or�k�����邽�߂̃T�C�Y����
		int i;
		for( i = 0; i < _countof(sizeTable); i++) {
			if( nPointSize <= sizeTable[i] ){
				int index = t_max(0, t_min((int)_countof(sizeTable) - 1, (int)(i + shift)));
				nPointSize = sizeTable[index];
				break;
			}
		}

		// �V�����t�H���g�T�C�Y�ݒ�
		lf.lfHeight = DpiPointsToPixels(-nPointSize, 10);
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;
	} else {
		// �t�H���g�T�C�Y���ς��Ȃ��̂ŏI��
		return;
	}

	HWND	hwndFrame;
	hwndFrame = GetMainWindow();

	/* �ݒ�ύX�𔽉f������ */
	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_CHANGESETTING,
		(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONT, hwndFrame
	);
}



/*! ���݂̃E�B���h�E���Ő܂�Ԃ�

	@date 2002.01.14 YAZAKI ���݂̃E�B���h�E���Ő܂�Ԃ���Ă���Ƃ��́A�ő�l�ɂ���悤��
	@date 2002.04.08 YAZAKI �Ƃ��ǂ��E�B���h�E���Ő܂�Ԃ���Ȃ����Ƃ�����o�O�C���B
	@date 2005.08.14 genta �����ł̐ݒ�͋��ʐݒ�ɔ��f���Ȃ��D
	@date 2005.10.22 aroka ���݂̃E�B���h�E�����ő�l�������^�C�v�̏����l ���g�O���ɂ���

	@note �ύX���鏇����ύX�����Ƃ���CEditWnd::InitMenu()���ύX���邱��
	@sa CEditWnd::InitMenu()
*/
void CViewCommander::Command_WRAPWINDOWWIDTH( void )	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
{
	// Jan. 8, 2006 genta ���菈����m_pCommanderView->GetWrapMode()�ֈړ�
	CEditView::TOGGLE_WRAP_ACTION nWrapMode;
	CLayoutInt newKetas;
	
	nWrapMode = m_pCommanderView->GetWrapMode( &newKetas );
	GetDocument()->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
	GetDocument()->m_bTextWrapMethodCurTemp = !( GetDocument()->m_nTextWrapMethodCur == GetDocument()->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod );
	if( nWrapMode == CEditView::TGWRAP_NONE ){
		return;	// �܂�Ԃ����͌��̂܂�
	}

	GetEditWindow()->ChangeLayoutParam( true, GetDocument()->m_cLayoutMgr.GetTabSpace(), newKetas );
	

	//	Aug. 14, 2005 genta ���ʐݒ�ւ͔��f�����Ȃ�
//	GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas = m_nViewColNum;

	m_pCommanderView->GetTextArea().SetViewLeftCol( CLayoutInt(0) );		/* �\����̈�ԍ��̌�(0�J�n) */

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();
	return;
}



//	from CViewCommander_New.cpp
/*!	�����̊Ǘ�(�_�C�A���O)
	@author	MIK
	@date	2003/04/07
*/
void CViewCommander::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//�_�C�A���O��\������
	if( !cDlgFavorite.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		return;
	}

	return;
}



/*!
	@brief �e�L�X�g�̐܂�Ԃ����@��ύX����
	
	@param[in] nWrapMethod �܂�Ԃ����@
		WRAP_NO_TEXT_WRAP  : �܂�Ԃ��Ȃ�
		WRAP_SETTING_WIDTH ; �w�茅�Ő܂�Ԃ�
		WRAP_WINDOW_WIDTH  ; �E�[�Ő܂�Ԃ�
	
	@note �E�B���h�E�����E�ɕ�������Ă���ꍇ�A�����̃E�B���h�E����܂�Ԃ����Ƃ���B
	
	@date 2008.05.31 nasukoji	�V�K�쐬
	@date 2009.08.28 nasukoji	�e�L�X�g�̍ő啝���Z�o����
*/
void CViewCommander::Command_TEXTWRAPMETHOD( int nWrapMethod )
{
	CEditDoc* pcDoc = GetDocument();

	// ���݂̐ݒ�l�Ɠ����Ȃ牽�����Ȃ�
	if( pcDoc->m_nTextWrapMethodCur == nWrapMethod )
		return;

	int nWidth;

	switch( nWrapMethod ){
	case WRAP_NO_TEXT_WRAP:		// �܂�Ԃ��Ȃ�
		nWidth = MAXLINEKETAS;	// �A�v���P�[�V�����̍ő啝�Ő܂�Ԃ�
		break;

	case WRAP_SETTING_WIDTH:	// �w�茅�Ő܂�Ԃ�
		nWidth = (Int)pcDoc->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas;
		break;

	case WRAP_WINDOW_WIDTH:		// �E�[�Ő܂�Ԃ�
		// �E�B���h�E�����E�ɕ�������Ă���ꍇ�͍����̃E�B���h�E�����g�p����
		nWidth = (Int)m_pCommanderView->ViewColNumToWrapColNum( GetEditWindow()->GetView(0).GetTextArea().m_nViewColNum );
		break;

	default:
		return;		// �s���Ȓl�̎��͉������Ȃ�
	}

	pcDoc->m_nTextWrapMethodCur = nWrapMethod;	// �ݒ���L��

	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�^�ꎞ�ݒ�K�p����	// 2008.06.08 ryoji
	pcDoc->m_bTextWrapMethodCurTemp = !( pcDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod == nWrapMethod );

	// �܂�Ԃ��ʒu��ύX
	GetEditWindow()->ChangeLayoutParam( false, pcDoc->m_cLayoutMgr.GetTabSpace(), (CLayoutInt)nWidth );

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( pcDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		pcDoc->m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
		pcDoc->m_pcEditWnd->RedrawAllViews( NULL );		// �X�N���[���o�[�̍X�V���K�v�Ȃ̂ōĕ\�������s����
	}else{
		pcDoc->m_cLayoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}



/*!
	@brief �����J�E���g���@��ύX����
	
	@param[in] nMode �����J�E���g���@
		SELECT_COUNT_TOGGLE  : �����J�E���g���@���g�O��
		SELECT_COUNT_BY_CHAR ; �������ŃJ�E���g
		SELECT_COUNT_BY_BYTE ; �o�C�g���ŃJ�E���g
*/
void CViewCommander::Command_SELECT_COUNT_MODE( int nMode )
{
	//�ݒ�ɂ͕ۑ������AView���Ɏ��t���O��ݒ�
	//BOOL* pbDispSelCountByByte = &GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte;
	ESelectCountMode* pnSelectCountMode = &GetEditWindow()->m_nSelectCountMode;

	if( nMode == SELECT_COUNT_TOGGLE ){
		//�������̃o�C�g���g�O��
		ESelectCountMode nCurrentMode;
		if( *pnSelectCountMode == SELECT_COUNT_TOGGLE ){
			nCurrentMode = ( GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte ?
								SELECT_COUNT_BY_BYTE :
								SELECT_COUNT_BY_CHAR );
		}else{
			nCurrentMode = *pnSelectCountMode;
		}
		*pnSelectCountMode = ( nCurrentMode == SELECT_COUNT_BY_BYTE ?
								SELECT_COUNT_BY_CHAR :
								SELECT_COUNT_BY_BYTE );
	}else if( nMode == SELECT_COUNT_BY_BYTE || nMode == SELECT_COUNT_BY_CHAR ){
		*pnSelectCountMode = ( ESelectCountMode )nMode;
	}
}



/*!	@brief ���p���̐ݒ�
	@date Jan. 29, 2005 genta �V�K�쐬
*/
void CViewCommander::Command_SET_QUOTESTRING( const wchar_t* quotestr )
{
	if( quotestr == NULL )
		return;

	wcsncpy( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou, quotestr,
		_countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ));
	
	GetDllShareData().m_Common.m_sFormat.m_szInyouKigou[ _countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ) - 1 ] = L'\0';
}
