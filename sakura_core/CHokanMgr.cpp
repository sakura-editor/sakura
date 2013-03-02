/*!	@file
	@brief �L�[���[�h�⊮

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2004, genta, Moca, novice
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CHokanMgr.h"
#include "env/CShareData.h"
#include "view/CEditView.h"
#include "plugin/CJackManager.h"
#include "plugin/CComplementIfObj.h"
#include "util/input.h"
#include "util/os.h"
#include "sakura_rc.h"

WNDPROC			gm_wpHokanListProc;


LRESULT APIENTRY HokanList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// Modified by KEITA for WIN64 2003.9.6
	CDialog* pCDialog = ( CDialog* )::GetWindowLongPtr( ::GetParent( hwnd ), DWLP_USER );
	CHokanMgr* pCHokanMgr = (CHokanMgr*)::GetWindowLongPtr( ::GetParent( hwnd ), DWLP_USER );
	MSG* pMsg;
	int nVKey;

	switch( uMsg ){
	case WM_KEYDOWN:
		nVKey = (int) wParam;	// virtual-key code
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
			return ::PostMessageAny( ::GetParent( ::GetParent( pCDialog->m_hwndParent ) ), uMsg, wParam, lParam );
		}
		break;
	case WM_GETDLGCODE:
		pMsg = (MSG*) lParam; // pointer to an MSG structure
		if( NULL == pMsg ){
//			MYTRACE_A( "WM_GETDLGCODE  pMsg==NULL\n" );
			return 0;
		}
//		MYTRACE_A( "WM_GETDLGCODE  pMsg->message = %xh\n", pMsg->message );
		return DLGC_WANTALLKEYS;/* ���ׂẴL�[�X�g���[�N�����ɉ����� */	//	Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	}
	return CallWindowProc( gm_wpHokanListProc, hwnd, uMsg, wParam, lParam);
}



CHokanMgr::CHokanMgr()
{
	m_cmemCurWord.SetString(L"");

	m_nCurKouhoIdx = -1;
	m_bTimerFlag = TRUE;
}

CHokanMgr::~CHokanMgr()
{
}

/* ���[�h���X�_�C�A���O�̕\�� */
HWND CHokanMgr::DoModeless( HINSTANCE hInstance , HWND hwndParent, LPARAM lParam )
{
	HWND hwndWork;
	hwndWork = CDialog::DoModeless( hInstance, hwndParent, IDD_HOKAN, lParam, SW_HIDE );
	OnSize( 0, 0 );
	/* ���X�g���t�b�N */
	// Modified by KEITA for WIN64 2003.9.6
	::gm_wpHokanListProc = (WNDPROC) ::SetWindowLongPtr( ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS ), GWLP_WNDPROC, (LONG_PTR)HokanList_SubclassProc  );

	::ShowWindow( GetHwnd(), SW_HIDE );
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

	::ShowWindow( GetHwnd(), SW_HIDE );
	m_nCurKouhoIdx = -1;
	/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
	CEditView* pcEditView = (CEditView*)m_lParam;
	pcEditView->OnSetFocus();
	return;

}

/*!	������
	pcmemHokanWord == NULL�̂Ƃ��A�⊮��₪�ЂƂ�������A�⊮�E�B���h�E��\�����Ȃ��ŏI�����܂��B
	Search()�Ăяo�����Ŋm�菈����i�߂Ă��������B

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
int CHokanMgr::Search(
	POINT*			ppoWin,
	int				nWinHeight,
	int				nColmWidth,
	const wchar_t*	pszCurWord,
	const TCHAR*	pszHokanFile,
	bool			bHokanLoHiCase,	// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� 2001/06/19 asa-o
	bool			bHokanByFile,	// �ҏW���f�[�^�������T�� 2003.06.23 Moca
	int				nHokanType,
	bool			bHokanByKeyword,
	CNativeW*		pcmemHokanWord	// 2001/06/19 asa-o
)
{
	CEditView* pcEditView = (CEditView*)m_lParam;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	/*
	||  �⊮�L�[���[�h�̌���
	||
	||  �E���������������ׂĕԂ�(���s�ŋ�؂��ĕԂ�)
	||  �E�w�肳�ꂽ���̍ő吔�𒴂���Ə����𒆒f����
	||  �E������������Ԃ�
	||
	*/
	m_vKouho.clear();
	CDicMgr::HokanSearch(
		pszCurWord,
		bHokanLoHiCase,								// ��������ɕύX	2001/06/19 asa-o
		m_vKouho,
		0, //Max��␔
		pszHokanFile
	);

	// 2003.05.16 Moca �ǉ� �ҏW���f�[�^���������T��
	if( bHokanByFile ){
		pcEditView->HokanSearchByFile(
			pszCurWord,
			bHokanLoHiCase,
			m_vKouho,
			1024 // �ҏW���f�[�^����Ȃ̂Ő��𐧌����Ă���
		);
	}
	// 2012.10.13 Moca �����L�[���[�h�������T��
	if( bHokanByKeyword ){
		HokanSearchByKeyword(
			pszCurWord,
			bHokanLoHiCase,
			m_vKouho
		);
	}

	{
		int nOption = (
			  (bHokanLoHiCase ? 0x01 : 0)
			  | (bHokanByFile ? 0x02 : 0)
			);
		
		CPlug::Array plugs;
		CPlug::Array plugType;
		CJackManager::getInstance()->GetUsablePlug( PP_COMPLEMENTGLOBAL, 0, &plugs );
		if( nHokanType != 0 ){
			CJackManager::getInstance()->GetUsablePlug( PP_COMPLEMENT, nHokanType, &plugType );
			if( 0 < plugType.size() ){
				plugs.push_back( plugType[0] );
			}
		}

		for( CPlug::Array::iterator it = plugs.begin(); it != plugs.end(); ++it ){
			//�C���^�t�F�[�X�I�u�W�F�N�g����
			CWSHIfObj::List params;
			std::wstring curWord = pszCurWord;
			CComplementIfObj* objComp = new CComplementIfObj( curWord , this, nOption );
			objComp->AddRef();
			params.push_back( objComp );
			//�v���O�C���Ăяo��
			(*it)->Invoke( pcEditView, params );

			objComp->Release();
		}
	}

	if( 0 == m_vKouho.size() ){
		m_nCurKouhoIdx = -1;
		return 0;
	}

//	2001/06/19 asa-o ��₪�P�̏ꍇ�⊮�E�B���h�E�͕\�����Ȃ�(�����⊮�̏ꍇ�͏���)
	if( 1 == m_vKouho.size() ){
		if(pcmemHokanWord != NULL){
			m_nCurKouhoIdx = -1;
			pcmemHokanWord->SetString( m_vKouho[0].c_str() );
			return 1;
		}
	}



//	m_hFont = hFont;
	m_poWin.x = ppoWin->x;
	m_poWin.y = ppoWin->y;
	m_nWinHeight = nWinHeight;
	m_nColmWidth = nColmWidth;
//	m_cmemCurWord.SetData( pszCurWord, lstrlen( pszCurWord ) );
	m_cmemCurWord.SetString( pszCurWord );


	m_nCurKouhoIdx = 0;
//	SetCurKouhoStr();



//	::ShowWindow( GetHwnd(), SW_SHOWNA );







	HWND hwndList;
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS );
	List_ResetContent( hwndList );
	{
		size_t kouhoNum = m_vKouho.size();
		for( size_t i = 0; i < kouhoNum; ++i ){
			::List_AddString( hwndList, m_vKouho[i].c_str() );
		}
	}
	List_SetCurSel( hwndList, 0 );


//@@	::EnableWindow( ::GetParent( ::GetParent( m_hwndParent ) ), FALSE );


	int nX;
	int nY;
	int nCX;
	int nCY;
	RECT	rcDesktop;
	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect( GetHwnd(), &rcDesktop );

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
		nCX = std::max((int)(rcDesktop.right - nX) , 100);	// �Œ�T�C�Y��100���炢��
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
	::MoveWindow( GetHwnd(), nX, nY, nCX, nCY, TRUE );
	::ShowWindow( GetHwnd(), SW_SHOW );
//	::ShowWindow( GetHwnd(), SW_SHOWNA );
	::SetFocus( GetHwnd() );
//	::SetFocus( ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS ) );
//	::SetFocus( ::GetParent( ::GetParent( m_hwndParent ) ) );


//	2001/06/18 asa-o:
	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��

//	2003.06.25 Moca ���̃��\�b�h�Ŏg���Ă��Ȃ��̂ŁA�Ƃ肠�����폜���Ă���
	int kouhoNum = m_vKouho.size();
	m_vKouho.clear();
	return kouhoNum;
}

void CHokanMgr::HokanSearchByKeyword(
	const wchar_t*	pszCurWord,
	bool 			bHokanLoHiCase,
	vector_ex<std::wstring>& 	vKouho
){
	const CEditView* pcEditView = (const CEditView*)m_lParam;
	const STypeConfig& type = pcEditView->GetDocument()->m_cDocType.GetDocumentAttribute();
	CKeyWordSetMgr& keywordMgr = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	const int nKeyLen = wcslen(pszCurWord);
	for( int kwdset = 0; kwdset < MAX_KEYWORDSET_PER_TYPE; kwdset++ ){
		if( type.m_nKeyWordSetIdx[kwdset] == -1 ){
			continue;
		}
		const int keyCount = keywordMgr.GetKeyWordNum(kwdset);
		for(int i = 0; i < keyCount; i++){
			const wchar_t* word = keywordMgr.GetKeyWord(kwdset,i);
			int nRet;
			if( bHokanLoHiCase ){
				nRet = auto_memicmp(pszCurWord, word, nKeyLen );
			}else{
				nRet = auto_memcmp(pszCurWord, word, nKeyLen );
			}
			if( nRet != 0 ){
				continue;
			}
			std::wstring strWord = std::wstring(word);
			AddKouhoUnique(vKouho, strWord);
		}
	}
}


BOOL CHokanMgr::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
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
	int		nControls = _countof( Controls );
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

	::GetClientRect( GetHwnd(), &rcDlg );
	nWidth = rcDlg.right - rcDlg.left;  // width of client area
	nHeight = rcDlg.bottom - rcDlg.top; // height of client area

//	2001/06/18 Start by asa-o: �T�C�Y�ύX��̈ʒu��ۑ�
	m_poWin.x = rcDlg.left - 4;
	m_poWin.y = rcDlg.top - 3;
	::ClientToScreen(GetHwnd(),&m_poWin);
//	2001/06/18 End

	nWork = 48;
	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( GetHwnd(), Controls[i] );
		::GetWindowRect( hwndCtrl, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( GetHwnd(), &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( GetHwnd(), &po );
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


BOOL CHokanMgr::OnLbnSelChange( HWND hwndCtl, int wID )
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
	MYTRACE_A( "CHokanMgr::DoHokan( nVKey==%xh )\n", nVKey );
#endif
	/* �⊮��⌈��L�[ */
	if( VK_RETURN	== nVKey && !m_pShareData->m_Common.m_sHelper.m_bHokanKey_RETURN )	return FALSE;/* VK_RETURN �⊮����L�[���L��/���� */
	if( VK_TAB		== nVKey && !m_pShareData->m_Common.m_sHelper.m_bHokanKey_TAB ) 		return FALSE;/* VK_TAB    �⊮����L�[���L��/���� */
	if( VK_RIGHT	== nVKey && !m_pShareData->m_Common.m_sHelper.m_bHokanKey_RIGHT )		return FALSE;/* VK_RIGHT  �⊮����L�[���L��/���� */

	HWND hwndList;
	int nItem;
	wchar_t wszLabel[1024];
	CEditView* pcEditView;
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS );
	nItem = List_GetCurSel( hwndList );
	if( LB_ERR == nItem ){
		return FALSE;
	}
	List_GetText( hwndList, nItem, wszLabel );

 	/* �e�L�X�g��\��t�� */
	pcEditView = (CEditView*)m_lParam;
	//	Apr. 28, 2000 genta
	pcEditView->GetCommander().HandleCommand( F_WordDeleteToStart, FALSE, 0, 0, 0, 0 );
	pcEditView->GetCommander().HandleCommand( F_INSTEXT_W, TRUE, (LPARAM)wszLabel, TRUE, 0, 0 );

	// Until here
//	pcEditView->GetCommander().HandleCommand( F_INSTEXT_W, TRUE, (LPARAM)(wszLabel + m_cmemCurWord.GetLength()), TRUE, 0, 0 );
	Hide();

	m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;	//	�⊮������
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
//		MYTRACE_A( "CHokanMgr::OnCharToItem vkey=%xh\n", vkey );
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
//	MYTRACE_A( "CHokanMgr::OnVKeyToItem vkey=%xh\n", vkey );
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
		m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;
		return -2;
	}
	return -2;
}

//	2001/06/18 Start by asa-o: �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��
void CHokanMgr::ShowTip()
{
	INT			nItem,
				nTopItem,
				nItemHeight;
	WCHAR		szLabel[1024];
	POINT		point;
	CEditView*	pcEditView;
	HWND		hwndCtrl;
	RECT		rcHokanWin;

	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS );

	nItem = List_GetCurSel( hwndCtrl );
	if( LB_ERR == nItem )	return ;

	List_GetText( hwndCtrl, nItem, szLabel );	// �I�𒆂̒P����擾

	pcEditView = (CEditView*)m_lParam;

	// ���łɎ���Tip���\������Ă�����
	if( pcEditView->m_dwTipTimer == 0 )
	{
		// ����Tip������
		pcEditView -> m_cTipWnd.Hide();
		pcEditView -> m_dwTipTimer = ::GetTickCount();
	}

	// �\������ʒu������
	nTopItem = List_GetTopIndex( hwndCtrl );
	nItemHeight = List_GetItemHeight( hwndCtrl, 0 );
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

bool CHokanMgr::AddKouhoUnique(vector_ex<std::wstring>& kouhoList, const std::wstring& strWord)
{
	return kouhoList.push_back_unique(strWord);
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID CHokanMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


