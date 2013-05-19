/*!	@file
	@brief �����̊Ǘ��_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/



#include "StdAfx.h"
#include <algorithm>
#include "CDlgFavorite.h"
#include "dlg/CDlgInput1.h"
#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/file.h"
#include "util/os.h"
#include "util/input.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_TAB_FAVORITE,				HIDC_TAB_FAVORITE,				//�^�u
	IDC_LIST_FAVORITE_FILE,			HIDC_LIST_FAVORITE_FILE,		//�t�@�C��
	IDC_LIST_FAVORITE_FOLDER,		HIDC_LIST_FAVORITE_FOLDER,		//�t�H���_
	IDC_LIST_FAVORITE_EXCEPTMRU,	HIDC_LIST_FAVORITE_EXCEPTMRU,	//MRU���O
	IDC_LIST_FAVORITE_SEARCH,		HIDC_LIST_FAVORITE_SEARCH,		//����
	IDC_LIST_FAVORITE_REPLACE,		HIDC_LIST_FAVORITE_REPLACE,		//�u��
	IDC_LIST_FAVORITE_GREP_FILE,	HIDC_LIST_FAVORITE_GREPFILE,	//GREP�t�@�C��
	IDC_LIST_FAVORITE_GREP_FOLDER,	HIDC_LIST_FAVORITE_GREPFOLDER,	//GREP�t�H���_
	IDC_LIST_FAVORITE_CMD,			HIDC_LIST_FAVORITE_CMD,			//�R�}���h
//	IDC_STATIC_BUTTONS,				-1,
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_FAVORITE_CLEAR,		//���ׂ�	// 2006.10.10 ryoji
	IDC_BUTTON_DELETE_NOFAVORATE,   HIDC_BUTTON_FAVORITE_DELETE_NOFAVORATE,  //���C�ɓ���ȊO
	IDC_BUTTON_DELETE_NOTFOUND,		HIDC_BUTTON_FAVORITE_DELETE_NOTFOUND		,  //���݂��Ȃ�����
	IDC_BUTTON_DELETE_SELECTED,     HIDC_BUTTON_FAVORITE_DELETE_SELECTED,    //�I������
	IDC_BUTTON_ADD_FAVORITE,        HIDC_BUTTON_ADD_FAVORITE,		// �ǉ�
	IDOK,							HIDC_FAVORITE_IDOK,				//����
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		//�w���v
//	IDC_STATIC_FAVORITE_MSG,		-1,
	0, 0
};

static const SAnchorList anchorList[] = {
	{IDC_TAB_FAVORITE,              ANCHOR_LEFT_RIGHT},
	{IDC_STATIC_BUTTONS,			ANCHOR_BOTTOM},
	{IDC_BUTTON_CLEAR, 				ANCHOR_BOTTOM},
	{IDC_BUTTON_DELETE_NOFAVORATE,	ANCHOR_BOTTOM},
	{IDC_BUTTON_DELETE_NOTFOUND,	ANCHOR_BOTTOM},
	{IDC_BUTTON_DELETE_SELECTED,	ANCHOR_BOTTOM},
	{IDC_BUTTON_ADD_FAVORITE, 		ANCHOR_BOTTOM},
	{IDOK, 							ANCHOR_BOTTOM},
	{IDC_BUTTON_HELP, 				ANCHOR_BOTTOM},
	{IDC_STATIC_FAVORITE_MSG, 		ANCHOR_BOTTOM},
};

//SDK�ɂ�����`����Ă��Ȃ��B
#ifndef	ListView_SetCheckState
//#if (_WIN32_IE >= 0x0300)
#define ListView_SetCheckState(hwndLV, i, fCheck) \
  ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)
//#endif
#endif

static int FormatFavoriteColm( TCHAR*, int, int , bool );
static int ListView_GetLParamInt( HWND, int );
static int CALLBACK CompareListViewFunc( LPARAM, LPARAM, LPARAM );

struct CompareListViewLParam
{
	int         nSortColumn;
	bool        bAbsOrder;
	HWND        hwndListView;
	const CRecent* pRecent;
};

/*
	CRecent�̊e�����N���X�� DLLSHAREDATA �֒��ڃA�N�Z�X���Ă���B
	�����͂ق��̃E�B���h�E������������\�������邽�߁A
	�_�C�A���O���A�N�e�B�u�ɂȂ����ۂɕύX���m�F���Ď擾����悤�ɂȂ��Ă���B
	�ҏW���͕ύX���m�F���Ă��Ȃ��̂ŁA����DLLSHAREDATA��ύX������ListView��
	DLLSHAREDATA����v���Ȃ��\��������B
*/


CDlgFavorite::CDlgFavorite()
{
	int	i;

	m_nCurrentTab = 0;
	_tcscpy( m_szMsg, _T("") );

	/* �T�C�Y�ύX���Ɉʒu�𐧌䂷��R���g���[���� */
	assert( _countof(anchorList) == _countof(m_rcItems) );

	{
		memset_raw( m_aFavoriteInfo, 0, sizeof( m_aFavoriteInfo ) );

		i = 0;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("�t�@�C��");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bFilePath  = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;
		m_aFavoriteInfo[i].m_bEditable  = false;
		m_aFavoriteInfo[i].m_bAddExcept = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("�t�H���_");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bFilePath  = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;
		m_aFavoriteInfo[i].m_bEditable  = false;
		m_aFavoriteInfo[i].m_bAddExcept = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentExceptMRU;
		m_aFavoriteInfo[i].m_pszCaption = _T("�t�@�C���E�t�H���_���O");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_EXCEPTMRU;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = true;
		m_aFavoriteInfo[i].m_bAddExcept = false;
		m_nExceptTab = i;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentSearch;
		m_aFavoriteInfo[i].m_pszCaption = _T("����");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_SEARCH;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = true;
		m_aFavoriteInfo[i].m_bAddExcept = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentReplace;
		m_aFavoriteInfo[i].m_pszCaption = _T("�u��");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_REPLACE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = true;
		m_aFavoriteInfo[i].m_bAddExcept = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREP�t�@�C��");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = true;
		m_aFavoriteInfo[i].m_bAddExcept = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREP�t�H���_");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = true;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = false;
		m_aFavoriteInfo[i].m_bAddExcept = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentCmd;
		m_aFavoriteInfo[i].m_pszCaption = _T("�R�}���h");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_CMD;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = true;
		m_aFavoriteInfo[i].m_bAddExcept = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = NULL;
		m_aFavoriteInfo[i].m_pszCaption = NULL;
		m_aFavoriteInfo[i].m_nId        = -1;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bFilePath  = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;
		m_aFavoriteInfo[i].m_bEditable  = false;
		m_aFavoriteInfo[i].m_bAddExcept = false;

		/* ����ȏ㑝�₷�Ƃ��̓e�[�u���T�C�Y�����������Ă� */
		assert( i < _countof(m_aFavoriteInfo) );
	}
	for( i = 0; i < FAVORITE_INFO_MAX; i++ ){
		m_aListViewInfo[i].hListView   = 0;
		m_aListViewInfo[i].nSortColumn = -1;
		m_aListViewInfo[i].bSortAscending = false;
	}
}

CDlgFavorite::~CDlgFavorite()
{
	for( int nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		m_aFavoriteInfo[nTab].m_pRecent->Terminate();
	}
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgFavorite::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_FAVORITE, lParam );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFavorite::SetData( void )
{
	int		nTab;

	for( nTab = 0; NULL != m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		SetDataOne( nTab, 0 );
	}

	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );

	UpdateUIState();

	return;
}

/* �_�C�A���O�f�[�^��1�̃^�u�̐ݒ�E�X�V
	@param nIndex       �^�u��Index
	@param nLvItemIndex �I���E�\��������ListView��Index�B-1�őI�����Ȃ�
*/
void CDlgFavorite::SetDataOne( int nIndex, int nLvItemIndex )
{
	HWND	hwndList;
	LV_ITEM	lvi;
	int		nNewFocus = -1;

	const CRecent*  pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

	/* ���X�g */
	hwndList = ::GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
	ListView_DeleteAllItems( hwndList );  /* ���X�g����ɂ��� */

	const int   nViewCount = pRecent->GetViewCount();
	const int   nItemCount = pRecent->GetItemCount();
	m_aFavoriteInfo[nIndex].m_nViewCount = nViewCount;

	TCHAR	tmp[1024];
	for( int i = 0; i < nItemCount; i++ )
	{
		FormatFavoriteColm( tmp, _countof(tmp), i, i < nViewCount );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = i;
		ListView_InsertItem( hwndList, &lvi );

		const TCHAR	*p;
		p = pRecent->GetItemText( i );
		auto_snprintf_s( tmp, _countof(tmp), _T("%ts"), p ? p : _T("") );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndList, &lvi );

		if( m_aFavoriteInfo[nIndex].m_bHaveFavorite )
		{
			ListView_SetCheckState( hwndList, i, (BOOL)pRecent->IsFavorite( i ) );
		}

	}

	if( -1 != m_aListViewInfo[nIndex].nSortColumn ){
		//�\�[�g���ێ�
		ListViewSort( m_aListViewInfo[nIndex], pRecent, m_aListViewInfo[nIndex].nSortColumn, false );
	}

	if( -1 != nLvItemIndex && nLvItemIndex < nItemCount )
	{
		nNewFocus = nLvItemIndex;
	}

	//�A�C�e���������Ăǂ����I���Ȃ�A�v���ɋ߂��A�C�e��(�擪������)��I��
	if( nItemCount > 0 && -1 != nLvItemIndex && nNewFocus == -1 )
	{
		nNewFocus = (0 < nLvItemIndex ? nItemCount - 1: 0);
	}

	if( -1 != nNewFocus )
	{
		ListView_SetItemState( hwndList, nNewFocus, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, nNewFocus, FALSE );
	}

	return;
}

/*! �_�C�A���O�f�[�^���擾���A���L�f�[�^�̂��C�ɓ�����X�V
	
	@retval TRUE ����(���̂Ƃ���FALSE�͕Ԃ��Ȃ�)
*/
int CDlgFavorite::GetData( void )
{
	int		nTab;

	for( nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite )
		{
			GetFavorite( nTab );

			//���X�g���X�V����B
			CRecent* pRecent = m_aFavoriteInfo[nTab].m_pRecent;
			pRecent->UpdateView();
		}
	}

	return TRUE;
}

BOOL CDlgFavorite::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	HWND		hwndBaseList;
	HWND		hwndTab;
	TCITEM		tcitem;
	LV_COLUMN	col;
	RECT		rc;
	int			nTab;
	long		lngStyle;

	_SetHwnd( hwndDlg );
	::SetWindowLongPtr( GetHwnd(), DWLP_USER, lParam );

	::GetWindowRect( hwndDlg, &rc );
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;

	for( int i = 0; i < _countof(anchorList); i++ ){
		GetItemClientRect( anchorList[i].id, m_rcItems[i] );
	}

	CreateSizeBox();
	CDialog::OnSize();

	RECT rcDialog = GetDllShareData().m_Common.m_sOthers.m_rcFavoriteDialog;
	if( rcDialog.left != 0 ||
		rcDialog.bottom != 0 ){
		m_xPos = rcDialog.left;
		m_yPos = rcDialog.top;
		m_nWidth = rcDialog.right - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}

	//���X�g�r���[�̕\���ʒu���擾����B
	m_nCurrentTab = 0;
	hwndBaseList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[0].m_nId );
	{
		rc.left = rc.top = rc.right = rc.bottom = 0;
		GetItemClientRect( m_aFavoriteInfo[0].m_nId, rc );
		m_rcListDefault = rc;
	}

	// �E�B���h�E�̃��T�C�Y
	SetDialogPosSize();

	hwndTab = ::GetDlgItem( hwndDlg, IDC_TAB_FAVORITE );
	TabCtrl_DeleteAllItems( hwndTab );

	GetItemClientRect( m_aFavoriteInfo[0].m_nId, rc );

	// ���X�g�r���[��Item/SubItem�����v�Z
	TCHAR* pszFAVORITE_TEXT = const_cast<TCHAR*>(_T("���C�ɓ���"));
	const int nListViewWidthClient = rc.right - rc.left
		 - CTextWidthCalc::WIDTH_MARGIN_SCROLLBER - ::GetSystemMetrics(SM_CXVSCROLL);
	// �����l�͏]��������%�w��
	int nItemCx = nListViewWidthClient * 16 / 100;
	int nSubItem1Cx = nListViewWidthClient * 79 / 100;
	
	{
		// �K�p����Ă���t�H���g����Z�o
		CTextWidthCalc calc( hwndBaseList );
		calc.SetTextWidthIfMax( pszFAVORITE_TEXT, CTextWidthCalc::WIDTH_LV_HEADER );
		TCHAR szBuf[200];
		for(int i = 0; i < 40; i++ ){
			// �uM (��\��)�v���̕������߂�
			FormatFavoriteColm( szBuf, _countof(szBuf), i, false);
			calc.SetTextWidthIfMax( szBuf, CTextWidthCalc::WIDTH_LV_ITEM_CHECKBOX );
		}
		
		if( 0 < calc.GetCx() ){
			nItemCx = calc.GetCx();
			nSubItem1Cx = nListViewWidthClient - nItemCx;
		}
	}

	for( nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		hwndList = GetDlgItem( hwndDlg, m_aFavoriteInfo[nTab].m_nId );
		m_aListViewInfo[nTab].hListView = hwndList;
		
		::MoveWindow( hwndList, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE );
		::ShowWindow( hwndList, SW_HIDE );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = nItemCx;
		col.pszText  = pszFAVORITE_TEXT;
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = nSubItem1Cx;
		col.pszText  = const_cast<TCHAR*>(m_aFavoriteInfo[nTab].m_pszCaption);
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		/* �s�I�� */
		lngStyle = ListView_GetExtendedListViewStyle( hwndList );
		lngStyle |= LVS_EX_FULLROWSELECT;
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite ) lngStyle |= LVS_EX_CHECKBOXES;
		ListView_SetExtendedListViewStyle( hwndList, lngStyle );

		/* �^�u���ڒǉ� */
		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = const_cast<TCHAR*>(m_aFavoriteInfo[nTab].m_pszCaption);
		TabCtrl_InsertItem( hwndTab, nTab, &tcitem );
	}

	hwndList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[m_nCurrentTab].m_nId );
	::ShowWindow( hwndList, SW_SHOW );
	TabCtrl_SetCurSel( hwndTab, m_nCurrentTab );
	//ChangeSlider( m_nCurrentTab );

	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgFavorite::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_FAVORITE ) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		// 2010.03.20 �L�����Z����p�~�BOK�Ɠ����ɂ���B
		// [X]�{�^���������ƒʉ߂���
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	// 2010.03.20 Moca �u���C�ɓ���ȊO�����ׂč폜�v�I�����b�Z�[�W�{�b�N�X��p�~��
	//     ���ꂼ��̃{�^���ɕύX
	//���ׂč폜
	case IDC_BUTTON_CLEAR:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			CRecent	*pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
			if( pRecent ){
				const int nRet = ConfirmMessage( GetHwnd(), 
					_T("�ŋߎg����%ts�̗������폜���܂��B\n��낵���ł����H\n"),
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption );
				if( IDYES == nRet ){
					pRecent->DeleteAllItem();
					RefreshListOne( m_nCurrentTab );
					UpdateUIState();
				}
			}
		}
		return TRUE;
	//���C�ɓ���ȊO�폜
	case IDC_BUTTON_DELETE_NOFAVORATE:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			if( m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite ){
				int const nRet = ConfirmMessage( GetHwnd(), 
					_T("�ŋߎg����%ts�̗����̂��C�ɓ���ȊO���폜���܂��B\n��낵���ł����H"),
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption );
				CRecent * const pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
				if( IDYES == nRet && pRecent ){
					GetFavorite( m_nCurrentTab );
					pRecent->DeleteItemsNoFavorite();
					pRecent->UpdateView();
					RefreshListOne( m_nCurrentTab );
					UpdateUIState();
				}
			}
		}
		return TRUE;
	// ���݂��Ȃ����� ���폜
	case IDC_BUTTON_DELETE_NOTFOUND:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			if( m_aFavoriteInfo[m_nCurrentTab].m_bFilePath ){
				const int nRet = ConfirmMessage( GetHwnd(), 
					_T("�ŋߎg����%ts�̑��݂��Ȃ��p�X���폜���܂��B\n\n��낵���ł����H"),
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption );
				CRecent * const pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
				if( IDYES == nRet && pRecent ){
					GetFavorite( m_nCurrentTab );

					// ���݂��Ȃ��p�X�̍폜
					for( int i = pRecent->GetItemCount() - 1; i >= 0; i-- ){
						TCHAR szPath[_MAX_PATH];
						auto_strcpy( szPath, pRecent->GetItemText(i) );
						CutLastYenFromDirectoryPath(szPath);
						if( false == IsFileExists(szPath, false ) ){
							pRecent->DeleteItem(i);
						}
					}
					pRecent->UpdateView();
					RefreshListOne( m_nCurrentTab );
					UpdateUIState();
				}
			}
		}
		return TRUE;
	//�I�����ڂ̍폜
	case IDC_BUTTON_DELETE_SELECTED:
		{
			DeleteSelected();
		}
		return TRUE;
	case IDC_BUTTON_ADD_FAVORITE:
		{
			AddItem();
		}
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFavorite::OnNotify( WPARAM wParam, LPARAM lParam )
{
	LPNMHDR	lpnmhdr;
	HWND	hwndTab;
	HWND	hwndList;

	hwndTab = ::GetDlgItem( GetHwnd(), IDC_TAB_FAVORITE );
	lpnmhdr = (LPNMHDR) lParam;
	if( lpnmhdr->hwndFrom == hwndTab )
	{
		switch( lpnmhdr->code )
		{
		case TCN_SELCHANGE:
			TabSelectChange(false);
			return TRUE;
			//break;
		}
	}else{
		hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
		if( hwndList == lpnmhdr->hwndFrom )
		{
			NM_LISTVIEW* pnlv = (NM_LISTVIEW*)lParam;
			switch( lpnmhdr->code )
			{
			case NM_DBLCLK:
				EditItem();
				return TRUE;
			case NM_RCLICK:
				{
					POINT po;
					if( 0 != GetCursorPos( &po ) ){
						RightMenu( po );
					}
				}
				return TRUE;

			// ListView�w�b�_�N���b�N:�\�[�g����
			case LVN_COLUMNCLICK:
				ListViewSort(
					m_aListViewInfo[m_nCurrentTab],
					m_aFavoriteInfo[m_nCurrentTab].m_pRecent,
					pnlv->iSubItem, true );
				return TRUE;
			
			// ListView��Delete�L�[�������ꂽ:�폜
			case LVN_KEYDOWN:
				switch( ((NMLVKEYDOWN*)lParam)->wVKey )
				{
				case VK_DELETE:
					DeleteSelected();
					return TRUE;
				case VK_APPS:
					{
						POINT po;
						RECT rc;
						hwndList = GetItemHwnd( m_aFavoriteInfo[m_nCurrentTab].m_nId );
						::GetWindowRect( hwndList, &rc );
						po.x = rc.left;
						po.y = rc.top;
						RightMenu( po );
					}
					return TRUE;
				}
				int nIdx = getCtrlKeyState();
				WORD wKey = ((NMLVKEYDOWN*)lParam)->wVKey;
				if( (wKey == VK_NEXT && nIdx == _CTRL) ){
					int next = m_nCurrentTab + 1;
					if( _countof(m_aFavoriteInfo) - 1 <= next ){
						next = 0;
					}
					TabCtrl_SetCurSel( GetItemHwnd(IDC_TAB_FAVORITE), next );
					TabSelectChange(true);
					return FALSE;
				}else if( (wKey == VK_PRIOR && nIdx == _CTRL) ){
					int prev = m_nCurrentTab - 1;
					if( prev < 0 ){
						prev = _countof(m_aFavoriteInfo) - 2;
					}
					TabCtrl_SetCurSel( GetItemHwnd(IDC_TAB_FAVORITE), prev );
					TabSelectChange(true);
					return FALSE;
				}
			}
		}
	}

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}

void CDlgFavorite::TabSelectChange(bool bSetFocus)
{
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
	HWND hwndTab = GetItemHwnd( IDC_TAB_FAVORITE );
	int nIndex = TabCtrl_GetCurSel( hwndTab );
	if( -1 != nIndex )
	{
		//�V�����\������B
		HWND hwndList = GetItemHwnd( m_aFavoriteInfo[nIndex].m_nId );
		::ShowWindow( hwndList, SW_SHOW );

		//���ݕ\�����̃��X�g���B���B
		HWND hwndList2 = GetItemHwnd( m_aFavoriteInfo[m_nCurrentTab].m_nId );
		::ShowWindow( hwndList2, SW_HIDE );

		if( bSetFocus ){
			::SetFocus( hwndList );
		}

		m_nCurrentTab = nIndex;

		UpdateUIState();

		//ChangeSlider( nIndex );
	}
}

BOOL CDlgFavorite::OnActivate( WPARAM wParam, LPARAM lParam )
{
	bool	bret;

	switch( LOWORD( wParam ) )
	{
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bret = RefreshList();
		::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, m_szMsg );
		return TRUE;
		//break;

	case WA_INACTIVE:
	default:
		break;
	}

	/* ���N���X�����o */
	return CDialog::OnActivate( wParam, lParam );
}

LPVOID CDlgFavorite::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*
	���X�g���X�V����B
*/
bool CDlgFavorite::RefreshList( void )
{
	int		nTab;
	bool	bret;
	bool	ret_val = false;
	TCHAR	msg[1024];

	_tcscpy( msg, _T("") );
	_tcscpy( m_szMsg, _T("") );

	//�S���X�g�̌��ݑI�𒆂̃A�C�e�����擾����B
	for( nTab = 0; NULL != m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		bret = RefreshListOne( nTab );
		if( bret == true )
		{
			ret_val = true;
		
			if( msg[0] != _T('\0') ) _tcscat( msg, _T("�A") );
			_tcscat( msg, m_aFavoriteInfo[nTab].m_pszCaption );
		}
	}

	if( ret_val )
	{
		auto_snprintf_s( m_szMsg, _countof(m_szMsg),
			_T("����(%ts)���X�V���ꂽ���ߕҏW������j�����ĕ\�����܂����B"),
			msg );
	}

	return ret_val;
}

/*
	������ʃ��X�g�̂���1�̃��X�g�r���[���X�V����B
*/
bool CDlgFavorite::RefreshListOne( int nIndex )
{
	HWND	hwndList;
	int		nCount;
	int		nCurrentIndex;
	int		nItemCount;
	int		i;
	BOOL	bret;
	LVITEM	lvitem;

	CRecent*	pRecent = m_aFavoriteInfo[nIndex].m_pRecent;
	nItemCount    = pRecent->GetItemCount();
	hwndList      = GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
	nCount        = ListView_GetItemCount( hwndList );
	nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
	if( -1 == nCurrentIndex ) nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_FOCUSED );

	if( nItemCount != nCount ) goto changed;	//�����ς�����̂ōč\�z

	//���C�ɓ��萔���ς�����̂ōč\�z
	if( m_aFavoriteInfo[nIndex].m_nViewCount != pRecent->GetViewCount() ) goto changed;

	for( i = 0; i < nCount; i++ )
	{
		TCHAR	szText[1024];
		auto_memset( szText, 0, _countof( szText ) );
		memset_raw( &lvitem, 0, sizeof( lvitem ) );
		lvitem.mask       = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText    = szText;
		lvitem.cchTextMax = _countof( szText );
		lvitem.iItem      = i;
		lvitem.iSubItem   = 1;
		bret = ListView_GetItem( hwndList, &lvitem );
		if( !bret ) goto changed;	//�G���[�Ȃ̂ōč\�z

		//�A�C�e�����e���ς�����̂ōč\�z
		if( lvitem.lParam != pRecent->FindItemByText( szText ) ) goto changed;
	}

	return false;

changed:
	SetDataOne( nIndex, nCurrentIndex );
	

	return true;
}

// ���C�ɓ���̃t���O�����K�p
void CDlgFavorite::GetFavorite( int nIndex )
{
	CRecent * const pRecent  = m_aFavoriteInfo[nIndex].m_pRecent;
	const HWND      hwndList = m_aListViewInfo[nIndex].hListView;
	if( m_aFavoriteInfo[nIndex].m_bHaveFavorite ){
		const int nCount = ListView_GetItemCount( hwndList );
		for( int i = 0; i < nCount; i++ ){
			const int  recIndex = ListView_GetLParamInt( hwndList, i );
			const BOOL bret = ListView_GetCheckState( hwndList, i );
			pRecent->SetFavorite( recIndex, bret ? true : false );
		}
	}
}


/*
	�I�𒆂̍��ڂ��폜
	���X�g�̍X�V������
*/
int CDlgFavorite::DeleteSelected()
{
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
	int     nDelItemCount = 0;
	CRecent *pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
	if( pRecent ){
		HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
		int nSelectedCount = ListView_GetSelectedCount(hwndList);
		if( 0 < nSelectedCount ){
			GetFavorite( m_nCurrentTab );

			int nLastSelectedItem = -1;
			std::vector<int> selRecIndexs;
			{
				int nLvItem = -1;
				while( (nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED)) != -1 )
				{
					int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
					if( 0 <= nRecIndex ){
						selRecIndexs.push_back( nRecIndex );
						nLastSelectedItem = nLvItem;
					}
				}
			}
			std::sort(selRecIndexs.rbegin(),selRecIndexs.rend());
			// �傫���ق�����폜���Ȃ��ƁACRecent����index�������
			size_t nSize = selRecIndexs.size();
			for( size_t n = 0; n < nSize; n++ )
			{
				pRecent->DeleteItem(selRecIndexs[n]);
				++nDelItemCount;
			}
			pRecent->UpdateView();
			if( 0 < nDelItemCount ){
				int nItem = nLastSelectedItem;
				if( -1 != nItem ){
					nItem += 1; // �폜�����A�C�e���̎��̃A�C�e��
					nItem -= nDelItemCount; // �V�����ʒu�́A�폜���������������
					if( pRecent->GetItemCount() <= nItem ){
						// ���f�[�^�̍Ō�̗v�f���폜����Ă���Ƃ��́A
						// �V�f�[�^�̍Ō��I��
						nItem = pRecent->GetItemCount() -1;
					}
				}
				int nLvTopIndex = ListView_GetTopIndex(hwndList);
				SetDataOne(m_nCurrentTab, nItem);
				if( 1 == nDelItemCount ){
					// 1�폜�̂Ƃ��́AY�X�N���[���ʒu��ێ�
					// 2�ȏ�͕��G�Ȃ̂�SetDataOne�ɂ��܂�������
					nLvTopIndex = t_max(0, t_min(pRecent->GetItemCount() - 1, nLvTopIndex));
					int nNowLvTopIndex = ListView_GetTopIndex(hwndList);
					if( nNowLvTopIndex != nLvTopIndex ){
						CMyRect rect;
						if( ListView_GetItemRect(hwndList, nNowLvTopIndex, &rect, LVIR_BOUNDS) ){
							// ListView_Scroll��Y���W��pixel�P�ʂŃX�N���[���ω������w��
							ListView_Scroll(hwndList, 0,
								(nLvTopIndex - nNowLvTopIndex) * (rect.bottom - rect.top) );
						}
					}
				}
				UpdateUIState();
			}
		}
	}
	return nDelItemCount;
}

void CDlgFavorite::UpdateUIState()
{
	CRecent& recent = *(m_aFavoriteInfo[m_nCurrentTab].m_pRecent);

	DlgItem_Enable( GetHwnd(), IDC_BUTTON_ADD_FAVORITE,
		m_aFavoriteInfo[m_nCurrentTab].m_bEditable && recent.GetItemCount() <= recent.GetArrayCount() );

	// �폜�̗L���E������
	DlgItem_Enable( GetHwnd(), IDC_BUTTON_CLEAR,
		0 < recent.GetItemCount() );

	DlgItem_Enable( GetHwnd(), IDC_BUTTON_DELETE_NOFAVORATE,
		m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite && 0 < recent.GetItemCount() );

	DlgItem_Enable( GetHwnd(), IDC_BUTTON_DELETE_NOTFOUND,
		m_aFavoriteInfo[m_nCurrentTab].m_bFilePath && 0 < recent.GetItemCount() );

	DlgItem_Enable( GetHwnd(), IDC_BUTTON_DELETE_SELECTED,
		0 < recent.GetItemCount() );
}

void CDlgFavorite::AddItem()
{
	if( !m_aFavoriteInfo[m_nCurrentTab].m_bEditable ){
		return;
	}
	TCHAR szAddText[_MAX_PATH];
	int max_size = _MAX_PATH;
	_tcscpy( szAddText, _T("") );

	CDlgInput1	cDlgInput1;
	if( !cDlgInput1.DoModal( G_AppInstance(), GetHwnd(), _T("�ǉ�"), _T("�ǉ����镶�������͂��Ă��������B"), max_size, szAddText ) ){
		return;
	}

	CRecent& recent = *(m_aFavoriteInfo[m_nCurrentTab].m_pRecent);
	GetFavorite(m_nCurrentTab);
	if( recent.AppendItemText(szAddText) ){
		SetDataOne(m_nCurrentTab, -1);
		UpdateUIState();
	}
}

void CDlgFavorite::EditItem()
{
	if( !m_aFavoriteInfo[m_nCurrentTab].m_bEditable ){
		return;
	}
	HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
	int nSelectedCount = ListView_GetSelectedCount(hwndList);
	if( 0 < nSelectedCount ){
		int nLvItem = -1;
		nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED);
		if( -1 != nLvItem ) {
			int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
			CRecent& recent = *(m_aFavoriteInfo[m_nCurrentTab].m_pRecent);
			TCHAR szText[_MAX_PATH];
			int max_size = _MAX_PATH;
			_tcsncpy_s(szText, max_size, recent.GetItemText(nRecIndex), _TRUNCATE);
			CDlgInput1	cDlgInput1;
			if( !cDlgInput1.DoModal(G_AppInstance(), GetHwnd(), _T("�ҏW"), _T("�ҏW���镶�������͂��Ă��������B"), max_size, szText) ){
				return;
			}
			GetFavorite(m_nCurrentTab);
			if( recent.EditItemText(nRecIndex, szText) ){
				SetDataOne(m_nCurrentTab, nRecIndex);
				UpdateUIState();
			}
		}
	}
}

void CDlgFavorite::RightMenu(POINT &menuPos)
{
	HMENU hMenu = ::CreatePopupMenu();
	const int MENU_EDIT = 100;
	const int MENU_ADD_EXCEPT = 101;
	const int MENU_ADD_NEW = 102;
	const int MENU_DELETE_ALL = 200;
	const int MENU_DELETE_NOFAVORATE = 201;
	const int MENU_DELETE_NOTFOUND = 202;
	const int MENU_DELETE_SELECTED = 203;
	CRecent& recent = *m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
	CRecent& exceptMRU = *m_aFavoriteInfo[m_nExceptTab].m_pRecent;
	
	int iPos = 0;
	int nEnable;
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bEditable && 0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_EDIT, _T("�ҏW(&E)") );
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bEditable ? 0 : MF_GRAYED);
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_ADD_NEW, _T("�V�K�ǉ�(&I)") );
	if( m_aFavoriteInfo[m_nCurrentTab].m_bAddExcept ){
		nEnable = (exceptMRU.GetItemCount() <= exceptMRU.GetArrayCount() ? 0 : MF_GRAYED);
		::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_ADD_EXCEPT, _T("���O���X�g�ɒǉ�(&I)") );
	}
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL );
	nEnable = (0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_ALL, _T("���ׂč폜(&A)") );
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite && 0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_NOFAVORATE, _T("���C�ɓ���ȊO�폜(&F)") );
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bFilePath && 0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_NOTFOUND, _T("���݂��Ȃ����ڍ폜(&N)") );
	nEnable = (0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_SELECTED, _T("�I�����ڍ폜(&D)") );

	// ���j���[��\������
	POINT pt = menuPos;
	RECT rcWork;
	GetMonitorWorkRect( pt, &rcWork );	// ���j�^�̃��[�N�G���A
	int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
								( pt.x > rcWork.left )? pt.x: rcWork.left,
								( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
								0, GetHwnd(), NULL);
	::DestroyMenu( hMenu );	// �T�u���j���[�͍ċA�I�ɔj�������

	switch( nId ){
	case MENU_EDIT:
		EditItem();
		break;
	case MENU_ADD_EXCEPT:
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			const int nLIST_SUBITEM_TEXT = 1;
			int     nDelItemCount = 0;
			CRecent *pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
			if( pRecent ){
				HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
				int nSelectedCount = ListView_GetSelectedCount(hwndList);
				if( 0 < nSelectedCount ){
					int nLvItem = -1;
					bool bAddFalse = false;
					CRecent& exceptMRU = *m_aFavoriteInfo[m_nExceptTab].m_pRecent;
					while( (nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED)) != -1 ) {
						int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
						if( exceptMRU.GetArrayCount() <= exceptMRU.GetItemCount() ){
							bAddFalse = true;
						}else{
							exceptMRU.AppendItemText(pRecent->GetItemText(nRecIndex));
						}
					}
					if( bAddFalse ){
						WarningMessage(GetHwnd(), _T("���O���X�g�������ς��Œǉ��ł��܂���ł����B") );
					}
					SetDataOne(m_nExceptTab, -1);
					UpdateUIState();
				}
			}
		}
		break;
	case MENU_ADD_NEW:
		AddItem();
		break;
	case MENU_DELETE_ALL:
		OnBnClicked( IDC_BUTTON_CLEAR );
		break;
	case MENU_DELETE_NOFAVORATE:
		OnBnClicked( IDC_BUTTON_DELETE_NOFAVORATE );
		break;
	case MENU_DELETE_NOTFOUND:
		OnBnClicked( IDC_BUTTON_DELETE_NOTFOUND );
		break;
	case MENU_DELETE_SELECTED:
		OnBnClicked( IDC_BUTTON_DELETE_SELECTED );
		break;
	}
}

int FormatFavoriteColm(TCHAR* buf, int size, int index, bool view)
{
	// 2010.03.21 Moca Text�ɘA�Ԃ�ݒ肷�邱�Ƃɂ���ăA�N�Z�X�L�[�ɂ���
	// 0 - 9 A - Z
	const int mod = index % 36;
	const TCHAR c = (TCHAR)(((mod) <= 9)?(_T('0') + mod):(_T('A') + mod - 10));
	return auto_snprintf_s( buf, size, _T("%tc%ts"), c, (view ? _T("   ") : _T(" (��\��)")) );
}


/*!
	ListView��Item(index)����LParam��int�^�Ƃ��Ď擾
*/
static int ListView_GetLParamInt( HWND hwndList, int lvIndex )
{
	LV_ITEM	lvitem;
	memset_raw( &lvitem, 0, sizeof(lvitem) );
	lvitem.mask = LVIF_PARAM;
	lvitem.iItem = lvIndex;
	lvitem.iSubItem = 0;
	if( ListView_GetItem( hwndList, &lvitem ) ){
		return (int)lvitem.lParam;
	}
	return -1;
}

/*!
	
	@param info [in,out] ���X�g�r���[�̃\�[�g��ԏ��
	@param pRecent       �\�[�g�A�C�e��
	@param colm          �\�[�g��������ԍ�
	@param bReverse      �\�[�g�ς݂̏ꍇ�ɍ~���ɐ؂�ւ���
*/
// static
void CDlgFavorite::ListViewSort(ListViewSortInfo& info, const CRecent* pRecent, int colm, bool bReverse )
{
	CompareListViewLParam lparamInfo;
	// �\�[�g���̌���
	if( info.nSortColumn != colm ){
		info.bSortAscending = true;
	}else{
		// �\�[�g�t��(�~��)
		info.bSortAscending = (bReverse ? (!info.bSortAscending): true);
	}
	
	// �w�b�_��������
	TCHAR szHeader[200];
	LV_COLUMN	col;
	if( -1 != info.nSortColumn ){
		// ���̃\�[�g�́u ���v����菜��
		col.mask = LVCF_TEXT;
		col.pszText = szHeader;
		col.cchTextMax = _countof(szHeader);
		col.iSubItem = 0;
		ListView_GetColumn( info.hListView, info.nSortColumn, &col );
		int nLen = (int)_tcslen(szHeader) - _tcslen(_T("��"));
		if( 0 <= nLen ){
			szHeader[nLen] = _T('\0');
		}
		col.mask = LVCF_TEXT;
		col.pszText = szHeader;
		col.iSubItem = 0;
		ListView_SetColumn( info.hListView, info.nSortColumn, &col );
	}
	// �u���v��t��
	col.mask = LVCF_TEXT;
	col.pszText = szHeader;
	col.cchTextMax = _countof(szHeader) - 4;
	col.iSubItem = 0;
	ListView_GetColumn( info.hListView, colm, &col );
	_tcscat(szHeader, info.bSortAscending ? _T("��") : _T("��"));
	col.mask = LVCF_TEXT;
	col.pszText = szHeader;
	col.iSubItem = 0;
	ListView_SetColumn( info.hListView, colm, &col );

	info.nSortColumn = colm;

	lparamInfo.nSortColumn = colm;
	lparamInfo.hwndListView = info.hListView;
	lparamInfo.pRecent = pRecent;
	lparamInfo.bAbsOrder = info.bSortAscending;

	ListView_SortItems( info.hListView, CompareListViewFunc, (LPARAM)&lparamInfo );
}


static int CALLBACK CompareListViewFunc( LPARAM lParamItem1, LPARAM lParamItem2, LPARAM lParamSort )
{
	CompareListViewLParam* pCompInfo = (CompareListViewLParam*)lParamSort;
	int nRet = 0;
	if(0 == pCompInfo->nSortColumn){
		nRet = lParamItem1 - lParamItem2;
	}else{
		const CRecent* p = pCompInfo->pRecent;
		nRet = auto_stricmp(p->GetItemText((int)lParamItem1), p->GetItemText((int)lParamItem2));
	}
	return pCompInfo->bAbsOrder ? nRet : -nRet;
}

INT_PTR CDlgFavorite::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );

	if( wMsg == WM_GETMINMAXINFO ){
		return OnMinMaxInfo( lParam );
	}
	return result;
}

BOOL CDlgFavorite::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* ���N���X�����o */
	CDialog::OnSize( wParam, lParam );

	::GetWindowRect( GetHwnd(), &GetDllShareData().m_Common.m_sOthers.m_rcFavoriteDialog );

	RECT rc;
	POINT ptNew;
	::GetWindowRect( GetHwnd(), &rc );
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for( int i = 0 ; i < _countof(anchorList); i++ ){
		ResizeItem( GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor );
	}

	for( int i = 0; i < FAVORITE_INFO_MAX; i++ ){
		HWND hwndList = GetItemHwnd( m_aFavoriteInfo[i].m_nId );
		ResizeItem( hwndList, m_ptDefaultSize, ptNew, m_rcListDefault, ANCHOR_ALL, (i==m_nCurrentTab) );
	}
	::InvalidateRect( GetHwnd(), NULL, TRUE );
	return TRUE;
}

BOOL CDlgFavorite::OnMove( WPARAM wParam, LPARAM lParam )
{
	::GetWindowRect( GetHwnd(), &GetDllShareData().m_Common.m_sOthers.m_rcFavoriteDialog );
	
	return CDialog::OnMove( wParam, lParam );
}

BOOL CDlgFavorite::OnMinMaxInfo( LPARAM lParam )
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y*2;
	return 0;
}
