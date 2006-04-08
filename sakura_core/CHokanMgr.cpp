//	$Id$
/*!	@file
	@brief �L�[���[�h�⊮

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2004, genta, Moca, novice

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "stdafx.h"
#include "CHokanMgr.h"
#include "debug.h"
#include "CKeyBind.h"
#include "Keycode.h"
#include "CDicMgr.h"
#include "CEditView.h"
#include "mymessage.h"
#include "sakura_rc.h"
#include "etc_uty.h"

	WNDPROC			gm_wpHokanListProc;


LRESULT APIENTRY HokanList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//#ifdef _DEBUG
//	MYTRACE( "HokanList_SubclassProc() uMsg == %xh\n", uMsg );
//#endif
	// Modified by KEITA for WIN64 2003.9.6
	CDialog* pCDialog = ( CDialog* )::GetWindowLongPtr( ::GetParent( hwnd ), DWLP_USER );
	CHokanMgr* pCHokanMgr = (CHokanMgr*)::GetWindowLongPtr( ::GetParent( hwnd ), DWLP_USER );
//	WORD vkey;
//	WORD nCaretPos;
//	LPARAM hwndLB;
//	int i;
	MSG* pMsg;
	int nVKey;
//	LPARAM lKeyData;
	switch( uMsg ){
	case WM_KEYDOWN:
		nVKey = (int) wParam;	// virtual-key code
//		lKeyData = lParam;			// key data
//		MYTRACE( "WM_KEYDOWN nVKey = %xh\n", nVKey );
		/* �L�[������U�����悤 */
		if (nVKey == VK_SPACE){	//	Space
// novice 2004/10/10
			/* Shift,Ctrl,Alt�L�[��������Ă����� */
			int nIdx = getCtrlKeyState();
			if (nIdx == _SHIFT){
				//	Shift + Space�Ł����U��
				wParam = VK_UP;
			}
			else if (nIdx == 0) {
				//	Space�݂̂Ł����U��
				wParam = VK_DOWN;
			}
		}
		/* �⊮���s�L�[�Ȃ�⊮���� */
		if( -1 != pCHokanMgr->KeyProc( wParam, lParam ) ){
			/* �L�[�X�g���[�N��e�ɓ]�� */
			::EnableWindow( ::GetParent( ::GetParent( pCDialog->m_hwndParent ) ), TRUE );
//			::ShowWindow( pCDialog->m_hWnd, SW_HIDE );
//			::SetFocus( ::GetParent( ::GetParent( pCDialog->m_hwndParent ) ) );
			return ::PostMessage( ::GetParent( ::GetParent( pCDialog->m_hwndParent ) ), uMsg, wParam, lParam );
		}
		break;
	case WM_GETDLGCODE:
		pMsg = (MSG*) lParam; // pointer to an MSG structure
		if( NULL == pMsg ){
//			MYTRACE( "WM_GETDLGCODE  pMsg==NULL\n" );
			return 0;
		}
//		MYTRACE( "WM_GETDLGCODE  pMsg->message = %xh\n", pMsg->message );
		return DLGC_WANTALLKEYS;/* ���ׂẴL�[�X�g���[�N�����ɉ����� */	//	Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	}
	return CallWindowProc( (WNDPROC)gm_wpHokanListProc, hwnd, uMsg, wParam, lParam);
}



CHokanMgr::CHokanMgr()
{
//	m_hFont = NULL;
//	m_hFontOld = NULL;
	m_cmemCurWord.SetDataSz( "" );

	m_pcmemKouho = NULL;
	m_nKouhoNum = 0;;
	m_nCurKouhoIdx = -1;
	m_pszCurKouho = NULL;
//	m_pcEditView = NULL;
	m_bTimerFlag = TRUE;
//	hm_wpHokanListProc = FALSE;
	return;
}

CHokanMgr::~CHokanMgr()
{
	if( NULL != m_pcmemKouho ){
		delete m_pcmemKouho;
		m_pcmemKouho = NULL;
	}
	m_nKouhoNum = 0;;
	if( NULL != m_pszCurKouho ){
		delete [] m_pszCurKouho;
		m_pszCurKouho = NULL;
	}

}

/* ���[�h���X�_�C�A���O�̕\�� */
HWND CHokanMgr::DoModeless( HINSTANCE hInstance , HWND hwndParent, LPARAM lParam )
{
	HWND hwndWork;
	hwndWork = CDialog::DoModeless( hInstance, hwndParent, IDD_HOKAN, lParam, SW_HIDE );
	OnSize( 0, 0 );
	/* ���X�g���t�b�N */
	// Modified by KEITA for WIN64 2003.9.6
	::gm_wpHokanListProc = (WNDPROC) ::SetWindowLongPtr( ::GetDlgItem( m_hWnd, IDC_LIST_WORDS ), GWLP_WNDPROC, (LONG_PTR)HokanList_SubclassProc  );

	::ShowWindow( m_hWnd, SW_HIDE );
	return hwndWork;
}

/* ���[�h���X���F�ΏۂƂȂ�r���[�̕ύX */
void CHokanMgr::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

void CHokanMgr::Hide( void )
{
	CEditView* pcEditView;
	::EnableWindow( ::GetParent( ::GetParent( m_hwndParent ) ), TRUE );
//	::DestroyWindow( hwnd );
	::ShowWindow( m_hWnd, SW_HIDE );
	m_nCurKouhoIdx = -1;
	/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
	pcEditView = (CEditView*)m_lParam;
	pcEditView->OnSetFocus();
//	m_nCurKouhoIdx = -1;
	return;

}

/*!	������
	pcmemHokanWord == NULL�̂Ƃ��A�⊮��₪�ЂƂ�������A�⊮�E�B���h�E��\�����Ȃ��ŏI�����܂��B
	Search()�Ăяo�����Ŋm�菈����i�߂Ă��������B

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
int CHokanMgr::Search(
//	HFONT		hFont,
	POINT*		ppoWin,
	int			nWinHeight,
	int			nColmWidth,
	const char*	pszCurWord,
	const char* pszHokanFile,
	int			bHokanLoHiCase,	// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� 2001/06/19 asa-o
	BOOL		bHokanByFile,	// �ҏW���f�[�^�������T�� 2003.06.23 Moca
	CMemory*	pcmemHokanWord	// 2001/06/19 asa-o
)
{
	CEditView* pcEditView = (CEditView*)m_lParam;
	/*
	||  �⊮�L�[���[�h�̌���
	||
	||  �E���������������ׂĕԂ�(���s�ŋ�؂��ĕԂ�)
	||  �E�w�肳�ꂽ���̍ő吔�𒴂���Ə����𒆒f����
	||  �E������������Ԃ�
	||
	*/
	if( NULL != m_pcmemKouho ){
		delete m_pcmemKouho;
		m_pcmemKouho = NULL;
	}
	m_nKouhoNum = CDicMgr::HokanSearch(
		pszCurWord,
//		m_pShareData->m_Common.m_bHokanLoHiCase,	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
		bHokanLoHiCase,								// ��������ɕύX	2001/06/19 asa-o
		&m_pcmemKouho,
		0, //Max��␔
		pszHokanFile
	);

	// 2003.05.16 Moca �ǉ� �ҏW���f�[�^���������T��
	if( bHokanByFile ){
		m_nKouhoNum = pcEditView->HokanSearchByFile(
			pszCurWord,
			bHokanLoHiCase,
			&m_pcmemKouho,
			m_nKouhoNum,
			1024 // �ҏW���f�[�^����Ȃ̂Ő��𐧌����Ă���
		);
	}
	if( 0 == m_nKouhoNum ){
		m_nCurKouhoIdx = -1;
		return 0;
	}

//	2001/06/19 asa-o ��₪�P�̏ꍇ�⊮�E�B���h�E�͕\�����Ȃ�(�����⊮�̏ꍇ�͏���)
	if( 1 == m_nKouhoNum ){
		if(pcmemHokanWord != NULL){
			m_nCurKouhoIdx = -1;
			// 2004.05.14 Moca m_pcmemKouho�̖����ɂ͉��s�R�[�h������A������폜���ăR�s�[���邷��悤��
			pcmemHokanWord->SetData( m_pcmemKouho->GetPtr(), m_pcmemKouho->GetLength() - 1 );
			return 1;
		}
	}

//	LOGFONT		lf;
//	HDC			hdc;
//	WNDCLASS	wc;
//	ATOM		atom;
	int			i;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

//	m_hFont = hFont;
	m_poWin.x = ppoWin->x;
	m_poWin.y = ppoWin->y;
	m_nWinHeight = nWinHeight;
	m_nColmWidth = nColmWidth;
//	m_cmemCurWord.SetData( pszCurWord, lstrlen( pszCurWord ) );
	m_cmemCurWord.SetDataSz( pszCurWord );


	m_nCurKouhoIdx = 0;
//	SetCurKouhoStr();



//	::ShowWindow( m_hWnd, SW_SHOWNA );







	HWND hwndList;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_WORDS );
	::SendMessage( hwndList, LB_RESETCONTENT, 0, 0 );
	char*	pszCR = "\n";
	char*	pszWork;
	char*	pszNext;
	char*	pszTest;
	pszWork = m_pcmemKouho->GetPtr();
	for( i = 0; i < m_nKouhoNum; ++i ){
		pszNext = strstr( pszWork, pszCR );
		if( NULL == pszNext ){
			break;
		}
		pszTest = new char[pszNext - pszWork + 1];
		memcpy( pszTest, pszWork, pszNext - pszWork );
		pszTest[pszNext - pszWork] = '\0';

		::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM) (LPCTSTR) pszTest );

		delete [] pszTest;
		pszTest = NULL;
		pszWork = pszNext + lstrlen( pszCR );
	}
	::SendMessage( hwndList, LB_SETCURSEL, 0, 0 );


//? ���̏����͂��Ȃ��ق������₷������?
//?	/* ���X�g�̑S�A�C�e���������X�g�Ɏ��܂鎞�́A���X�g�̃T�C�Y������������ */
//?	int nItemHeight;
//?	nItemHeight = ::SendMessage( hwndList, LB_GETITEMHEIGHT, 0, 0 );
//?	if( ( m_nKouhoNum + 1 ) * nItemHeight < m_nHeight ){
//?		m_nHeight = ( m_nKouhoNum + 1 ) * nItemHeight;
//?	}else{
//?		if( m_nHeight < 1000 ){
//?			m_nHeight = 1000;
//?		}
//?	}
//?	if( ( m_nKouhoNum + 1 ) * nItemHeight < m_nHeight ){
//?		m_nHeight = ( m_nKouhoNum + 1 ) * nItemHeight;
//?	}
//?


//@@	::EnableWindow( ::GetParent( ::GetParent( m_hwndParent ) ), FALSE );


	int nX;
	int nY;
	int nCX;
	int nCY;
	RECT	rcDesktop;
	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect( m_hWnd, &rcDesktop );

	nX = m_poWin.x - m_nColmWidth;
	nY = m_poWin.y + m_nWinHeight + 4;
	nCX = m_nWidth;
	nCY = m_nHeight;

	/* ���ɓ���Ȃ� */
	if( nY + nCY < rcDesktop.bottom ){
		/* �������Ȃ� */
	}else
	/* ��ɓ���Ȃ� */
	if( rcDesktop.top < m_poWin.y - m_nHeight - 4 ){
		/* ��ɏo�� */
		nY = m_poWin.y - m_nHeight - 4;
	}else
	/* ��ɏo�������ɏo����(�L���ق��ɏo��) */
	if(	rcDesktop.bottom - nY > m_poWin.y ){
		/* ���ɏo�� */
//		m_nHeight = rcDesktop.bottom - nY;
		nCY = rcDesktop.bottom - nY;
	}else{
		/* ��ɏo�� */
		nY = rcDesktop.top;
		nCY = m_poWin.y - 4 - rcDesktop.top;
	}

//	2001/06/19 Start by asa-o: �\���ʒu�␳

	// �E�ɓ���
	if(nX + nCX < rcDesktop.right ){
		// ���̂܂�
	}else
	// ���ɓ���
	if(rcDesktop.left < nX - nCX + 8){
		// ���ɕ\��
		nX -= nCX - 8;
	}else{
		// �T�C�Y�𒲐����ĉE�ɕ\��
		nCX = max((int)(rcDesktop.right - nX) , 100);	// �Œ�T�C�Y��100���炢��
	}

//	2001/06/19 End

//	2001/06/18 Start by asa-o: �␳��̈ʒu�E�T�C�Y��ۑ�
	m_poWin.x = nX;
	m_poWin.y = nY;
	m_nHeight = nCY;
	m_nWidth = nCX;
//	2001/06/18 End

	/* �͂ݏo���Ȃ珬�������� */
//	if( rcDesktop.bottom < nY + nCY ){
//		/* ���ɂ͂ݏo�� */
//		if( m_poWin.y - 4 - nCY < 0 ){
//			/* ��ɂ͂ݏo�� */
//			/* �������������� */
//			nCY = rcDesktop.bottom - nY - 4;
//		}else{
//
//		}
//
//	}
	::MoveWindow( m_hWnd, nX, nY, nCX, nCY, TRUE );
	::ShowWindow( m_hWnd, SW_SHOW );
//	::ShowWindow( m_hWnd, SW_SHOWNA );
	::SetFocus( m_hWnd );
//	::SetFocus( ::GetDlgItem( m_hWnd, IDC_LIST_WORDS ) );
//	::SetFocus( ::GetParent( ::GetParent( m_hwndParent ) ) );


//	2001/06/18 asa-o:
	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��

//	2003.06.25 Moca ���̃��\�b�h�Ŏg���Ă��Ȃ��̂ŁA�Ƃ肠�����폜���Ă���
	if( NULL != m_pcmemKouho ){
		delete m_pcmemKouho;
		m_pcmemKouho = NULL;
	}
	return m_nKouhoNum;
}


#if 0
///// 2003.06.25�@���g�p�̂悤���BMoca
void CHokanMgr::SetCurKouhoStr( void )
{
	char*	pszCR = "\n";
	int		i;
	char*	pszWork;
	char*	pszNext;
	char	szAdd[64];

	if( NULL != m_pszCurKouho ){
		delete [] m_pszCurKouho;
		m_pszCurKouho = NULL;
	}
	if( m_nCurKouhoIdx >= m_nKouhoNum ){
		return;
	}
	strcpy( szAdd, "" );
//	sprintf( szAdd, " (%d/%d)", m_nCurKouhoIdx + 1, m_nKouhoNum );
	pszWork = m_pcmemKouho->GetPtr();
	for( i = 0; i <= m_nCurKouhoIdx; ++i ){
		pszNext = strstr( pszWork, pszCR );
		if( NULL == pszNext ){
			return;
		}
		if( i == m_nCurKouhoIdx ){
			pszWork += m_cmemCurWord.GetLength();
			m_pszCurKouho = new char[pszNext - pszWork + 1 + lstrlen( szAdd )];
			memcpy( m_pszCurKouho, pszWork, pszNext - pszWork );
			m_pszCurKouho[pszNext - pszWork] = '\0';
			strcat( m_pszCurKouho, szAdd );
			::MoveWindow( m_hWnd,
				m_poWin.x - m_nColmWidth/*+ m_cmemCurWord.GetLength() * m_nColmWidth*/,
				m_poWin.y + m_nWinHeight,
//				m_nColmWidth * lstrlen(m_pszCurKouho) + 2,
//				m_nWinHeight + 2 + 8,
				220,
				180,
				TRUE
			);
			OnSize( 0, 0 );

			return;
		}
		pszWork = pszNext + lstrlen( pszCR );
	}
	return;

}
#endif


BOOL CHokanMgr::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	/* ���N���X�����o */
//-	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}

BOOL CHokanMgr::OnDestroy( void )
{
	/* ���N���X�����o */
	CreateSizeBox();
	return CDialog::OnDestroy();


}


BOOL CHokanMgr::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* ���N���X�����o */
	CDialog::OnSize( wParam, lParam );

	int	Controls[] = {
		IDC_LIST_WORDS
	};
	int		nControls = sizeof( Controls ) / sizeof( Controls[0] );
	int		fwSizeType;
	int		nWidth;
	int		nHeight;
	int		i;
	int		nWork;
	RECT	rc;
	HWND	hwndCtrl;
	POINT	po;
	RECT	rcDlg;

	fwSizeType = wParam;	// resizing flag

	::GetClientRect( m_hWnd, &rcDlg );
	nWidth = rcDlg.right - rcDlg.left;  // width of client area
	nHeight = rcDlg.bottom - rcDlg.top; // height of client area

//	2001/06/18 Start by asa-o: �T�C�Y�ύX��̈ʒu��ۑ�
	m_poWin.x = rcDlg.left - 4;
	m_poWin.y = rcDlg.top - 3;
	::ClientToScreen(m_hWnd,&m_poWin);
//	2001/06/18 End

	nWork = 48;
	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( m_hWnd, Controls[i] );
		::GetWindowRect( hwndCtrl, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( m_hWnd, &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( m_hWnd, &po );
		rc.right = po.x;
		rc.bottom  = po.y;
		if( Controls[i] == IDC_LIST_WORDS ){
			::SetWindowPos(
				hwndCtrl,
				NULL,
				rc.left,
				rc.top,
				nWidth - rc.left * 2,
				nHeight - rc.top * 2/* - 20*/,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
	}

//	2001/06/18 asa-o:
	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��

	return TRUE;

}

BOOL CHokanMgr::OnBnClicked( int wID )
{
	switch( wID ){
	case IDCANCEL:
//		CloseDialog( 0 );
		Hide();
		return TRUE;
	case IDOK:
//		CloseDialog( 0 );
		/* �⊮���s */
		DoHokan( VK_RETURN );
		return TRUE;
	}
	return FALSE;
	return 0;
}



BOOL CHokanMgr::OnKeyDown( WPARAM wParam, LPARAM lParam )
{
	int nVKey;
	nVKey = (int) wParam;	// virtual-key code
//	lKeyData = lParam;			// key data
	switch( nVKey ){
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		return 1;
	}
 	return 0;
}


BOOL CHokanMgr::OnCbnSelChange( HWND hwndCtl, int wID )
{
//	2001/06/18 asa-o:
	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��
	return TRUE;
}

BOOL CHokanMgr::OnLbnDblclk( int wID )
{
	/* �⊮���s */
	DoHokan( 0 );
	return TRUE;

}


BOOL CHokanMgr::OnKillFocus( WPARAM wParam, LPARAM lParam )
{
//	Hide();
	return TRUE;
}



/* �⊮���s */
BOOL CHokanMgr::DoHokan( int nVKey )
{
#ifdef _DEBUG
	MYTRACE( "CHokanMgr::DoHokan( nVKey==%xh )\n", nVKey );
#endif
	/* �⊮��⌈��L�[ */
	if( VK_RETURN	== nVKey && FALSE == m_pShareData->m_Common.m_bHokanKey_RETURN )	return FALSE;/* VK_RETURN �⊮����L�[���L��/���� */
	if( VK_TAB		== nVKey && FALSE == m_pShareData->m_Common.m_bHokanKey_TAB ) 		return FALSE;/* VK_TAB    �⊮����L�[���L��/���� */
	if( VK_RIGHT	== nVKey && FALSE == m_pShareData->m_Common.m_bHokanKey_RIGHT )		return FALSE;/* VK_RIGHT  �⊮����L�[���L��/���� */
#if 0
	if( VK_SPACE	== nVKey && FALSE == m_pShareData->m_Common.m_bHokanKey_SPACE )		return FALSE;/* VK_SPACE  �⊮����L�[���L��/���� */
#endif

	HWND hwndList;
	int nItem;
	char szLabel[1024];
	CEditView* pcEditView;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_WORDS );
	nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
	if( LB_ERR == nItem ){
		return FALSE;
	}
	::SendMessage( hwndList, LB_GETTEXT, nItem, (LPARAM)szLabel );
#if 0
	/* �X�y�[�X�Ō�⌈��̏ꍇ�̓X�y�[�X������ */
	if( VK_SPACE == nVKey ){
		strcat( szLabel, " " );
	}
#endif
//	pszWork += m_cmemCurWord.GetLength();

 	/* �e�L�X�g��\��t�� */
	pcEditView = (CEditView*)m_lParam;
	//	Apr. 28, 2000 genta
	pcEditView->HandleCommand( F_WordDeleteToStart, FALSE, 0, 0, 0, 0 );
	pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)(szLabel), TRUE, 0, 0 );

	// Until here
//	pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)(szLabel + m_cmemCurWord.GetLength()), TRUE, 0, 0 );
	Hide();

	m_pShareData->m_Common.m_bUseHokan = FALSE;	//	�⊮������
	return TRUE;
}

/*
�߂�l�� -2 �̏ꍇ�́A�A�v���P�[�V�����͍��ڂ̑I�����������A
���X�g �{�b�N�X�ł���ȏ�̓��삪�K�v�łȂ����Ƃ������܂��B

�߂�l�� -1 �̏ꍇ�́A���X�g �{�b�N�X���L�[�X�g���[�N�ɉ�����
�f�t�H���g�̓�������s���邱�Ƃ������܂��B

 �߂�l�� 0 �ȏ�̏ꍇ�́A���̒l�̓��X�g �{�b�N�X�̍��ڂ� 0 ��
��Ƃ����C���f�b�N�X���Ӗ����A���X�g �{�b�N�X�����̍��ڂł�
�L�[�X�g���[�N�ɉ����ăf�t�H���g�̓�������s���邱�Ƃ������܂��B

*/
//	int CHokanMgr::OnVKeyToItem( WPARAM wParam, LPARAM lParam )
//	{
//		return KeyProc( wParam, lParam );
//	}

/*
�߂�l�� -2 �̏ꍇ�́A�A�v���P�[�V�����͍��ڂ̑I�����������A
���X�g �{�b�N�X�ł���ȏ�̓��삪�K�v�łȂ����Ƃ������܂��B

�߂�l�� -1 �̏ꍇ�́A���X�g �{�b�N�X���L�[�X�g���[�N�ɉ�����
�f�t�H���g�̓�������s���邱�Ƃ������܂��B

 �߂�l�� 0 �ȏ�̏ꍇ�́A���̒l�̓��X�g �{�b�N�X�̍��ڂ� 0 ��
��Ƃ����C���f�b�N�X���Ӗ����A���X�g �{�b�N�X�����̍��ڂł�
�L�[�X�g���[�N�ɉ����ăf�t�H���g�̓�������s���邱�Ƃ������܂��B

*/
//	int CHokanMgr::OnCharToItem( WPARAM wParam, LPARAM lParam )
//	{
//		WORD vkey;
//		WORD nCaretPos;
//		LPARAM hwndLB;
//		vkey = LOWORD(wParam);		// virtual-key code
//		nCaretPos = HIWORD(wParam);	// caret position
//		hwndLB = lParam;			// handle to list box
//	//	switch( vkey ){
//	//	}
//
//		MYTRACE( "CHokanMgr::OnCharToItem vkey=%xh\n", vkey );
//		return -1;
//	}

int CHokanMgr::KeyProc( WPARAM wParam, LPARAM lParam )
{
	WORD vkey;
	WORD nCaretPos;
	LPARAM hwndLB;
	vkey = LOWORD(wParam);		// virtual-key code
	nCaretPos = HIWORD(wParam);	// caret position
	hwndLB = lParam;			// handle to list box
//	MYTRACE( "CHokanMgr::OnVKeyToItem vkey=%xh\n", vkey );
	switch( vkey ){
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		/* ���X�g�{�b�N�X�̃f�t�H���g�̓���������� */
		return -1;
	case VK_RETURN:
	case VK_TAB:
	case VK_RIGHT:
#if 0
	case VK_SPACE:
#endif
		/* �⊮���s */
		if( DoHokan( vkey ) ){
			return -1;
		}else{
			return -2;
		}
	case VK_ESCAPE:
	case VK_LEFT:
		m_pShareData->m_Common.m_bUseHokan = FALSE;
		return -2;
	}
	return -2;
}

//BOOL CHokanMgr::OnNextDlgCtl( WPARAM wParam, LPARAM lParam )
//{
//	/* �⊮���s */
//	DoHokan();
//	return 0;
//}


//	2001/06/18 Start by asa-o: �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��
void CHokanMgr::ShowTip()
{
	INT			nItem,
				nTopItem,
				nItemHeight;
	CHAR		szLabel[1024];
	POINT		point;
	CEditView*	pcEditView;
	HWND		hwndCtrl;
	RECT		rcHokanWin;

	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_LIST_WORDS );

	nItem = ::SendMessage( hwndCtrl, LB_GETCURSEL, 0, 0 );
	if( LB_ERR == nItem )	return ;

	::SendMessage( hwndCtrl, LB_GETTEXT, nItem, (LPARAM)szLabel );	// �I�𒆂̒P����擾

	pcEditView = (CEditView*)m_lParam;

	// ���łɎ���Tip���\������Ă�����
	if( pcEditView->m_dwTipTimer == 0 )
	{
		// ����Tip������
		pcEditView -> m_cTipWnd.Hide();
		pcEditView -> m_dwTipTimer = ::GetTickCount();
	}

	// �\������ʒu������
	nTopItem = ::SendMessage(hwndCtrl,LB_GETTOPINDEX,0,0);
	nItemHeight = ::SendMessage( hwndCtrl, LB_GETITEMHEIGHT, 0, 0 );
	point.x = m_poWin.x + m_nWidth;
	point.y = m_poWin.y + 4 + (nItem - nTopItem) * nItemHeight;
	// 2001/06/19 asa-o �I�𒆂̒P�ꂪ�⊮�E�B���h�E�ɕ\������Ă���Ȃ玫��Tip��\��
	if( point.y > m_poWin.y && point.y < m_poWin.y + m_nHeight )
	{
		::SetRect( &rcHokanWin , m_poWin.x, m_poWin.y, m_poWin.x + m_nWidth, m_poWin.y + m_nHeight );
		if( !pcEditView -> ShowKeywordHelp( point,szLabel, &rcHokanWin ) )
			pcEditView -> m_dwTipTimer = ::GetTickCount();	// �\������ׂ��L�[���[�h�w���v������
	}
}
//	2001/06/18 End

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID CHokanMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
