/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani Visual Basic�̃A�E�g���C�����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, JEPRO, genta, hor
	Copyright (C) 2002, MIK, aroka, hor, genta, YAZAKI, Moca, frozen
	Copyright (C) 2003, zenryaku, Moca, naoh, little YOSHI, genta,
	Copyright (C) 2004, zenryaku, Moca, novice
	Copyright (C) 2005, genta, zenryaku, ������, D.S.Koba
	Copyright (C) 2006, genta, aroka, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include "sakura_rc.h"
#include "CDlgFuncList.h"
#include "etc_uty.h"
#include "debug.h"
#include "global.h"
#include "CEditView.h"
#include "funccode.h"		//Stonee, 2001/03/12
#include "CFuncInfoArr.h"// 2002/2/3 aroka
#include "mymessage.h"// 2002/2/3 aroka
#include "Keycode.h"// 2002/2/10 aroka �w�b�_����
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "CEditWnd.h"	//	2006/2/11 aroka �ǉ�

//�A�E�g���C����� CDlgFuncList.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12200
	IDC_BUTTON_COPY,				HIDC_FL_BUTTON_COPY,	//�R�s�[
	IDOK,							HIDOK_FL,				//�W�����v
	IDCANCEL,						HIDCANCEL_FL,			//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_FL_BUTTON_HELP,	//�w���v
	IDC_CHECK_bAutoCloseDlgFuncList,	HIDC_FL_CHECK_bAutoCloseDlgFuncList,	//�����I�ɕ���
	IDC_LIST1,						HIDC_FL_LIST1,			//�g�s�b�N���X�g
	IDC_TREE1,						HIDC_FL_TREE1,			//�g�s�b�N�c���[
	IDC_CHECK_bFunclistSetFocusOnJump	,HIDC_FL_CHECK_bFunclistSetFocusOnJump,	//�W�����v�Ńt�H�[�J�X�ړ�����
	IDC_CHECK_bMarkUpBlankLineEnable	,HIDC_FL_CHECK_bMarkUpBlankLineEnable,	//��s�𖳎�����
	IDC_COMBO_nSortType,			HIDC_COMBO_nSortType,	//����
	IDC_BUTTON_WINSIZE,				HIDC_FL_BUTTON_WINSIZE,	//�E�B���h�E�ʒu�ۑ�	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/*! �\�[�g��r�p�v���V�[�W�� */
int CALLBACK _CompareFunc_( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	CFuncInfo*		pcFuncInfo1;
	CFuncInfo*		pcFuncInfo2;
	CDlgFuncList*	pcDlgFuncList;
	pcDlgFuncList = (CDlgFuncList*)lParamSort;

	pcFuncInfo1 = pcDlgFuncList->m_pcFuncInfoArr->GetAt( lParam1 );
	if( NULL == pcFuncInfo1 ){
		return -1;
	}
	pcFuncInfo2 = pcDlgFuncList->m_pcFuncInfoArr->GetAt( lParam2 );
	if( NULL == pcFuncInfo2 ){
		return -1;
	}
	//	Apr. 23, 2005 genta �s�ԍ������[��
	if( 1 == pcDlgFuncList->m_nSortCol){	/* �\�[�g�����ԍ� */
		return strcmp( pcFuncInfo1->m_cmemFuncName.GetPtr(), pcFuncInfo2->m_cmemFuncName.GetPtr() );
	}
	//	Apr. 23, 2005 genta �s�ԍ������[��
	if( 0 == pcDlgFuncList->m_nSortCol){	/* �\�[�g�����ԍ� */
		if( pcFuncInfo1->m_nFuncLineCRLF < pcFuncInfo2->m_nFuncLineCRLF ){
			return -1;
		}else
		if( pcFuncInfo1->m_nFuncLineCRLF == pcFuncInfo2->m_nFuncLineCRLF ){
			return 0;
		}else{
			return 1;
		}
	}
	// From Here 2001.12.07 hor
	if( 2 == pcDlgFuncList->m_nSortCol){	/* �\�[�g�����ԍ� */
		if( pcFuncInfo1->m_nInfo < pcFuncInfo2->m_nInfo ){
			return -1;
		}else
		if( pcFuncInfo1->m_nInfo == pcFuncInfo2->m_nInfo ){
			return 0;
		}else{
			return 1;
		}
	}
	// To Here 2001.12.07 hor
	return -1;
}

CDlgFuncList::CDlgFuncList()
{
	m_pcFuncInfoArr = NULL;		/* �֐����z�� */
	m_nCurLine = 0;				/* ���ݍs */
	//	Apr. 23, 2005 genta �s�ԍ������[��
	m_nSortCol = 0;				/* �\�[�g�����ԍ� 2004.04.06 zenryaku �W���͍s�ԍ�(1���) */
	m_bLineNumIsCRLF = FALSE;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	m_bWaitTreeProcess = false;	// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 2/4
	m_nSortType = 0;
	return;
}



//	/* ���[�_���_�C�A���O�̕\�� */
//	int CDlgFuncList::DoModal(
//		HINSTANCE		hInstance,
//		HWND			hwndParent,
//		LPARAM			lParam,
//		CFuncInfoArr*	pcFuncInfoArr,
//		int				nCurLine,
//		int				nListType,
//		int				bLineNumIsCRLF	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//	)
//	{
//		m_pcFuncInfoArr = pcFuncInfoArr;	/* �֐����z�� */
//		m_nCurLine = nCurLine;				/* ���ݍs */
//		m_nListType = nListType;			/* �ꗗ�̎�� */
//		m_bLineNumIsCRLF = bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//		return CDialog::DoModal( hInstance, hwndParent, IDD_FUNCLIST, lParam );
//	}


/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDlgFuncList::DoModeless(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	CFuncInfoArr*	pcFuncInfoArr,
	int				nCurLine,
	int				nListType,
	int				bLineNumIsCRLF		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
)
{
	m_pcFuncInfoArr = pcFuncInfoArr;	/* �֐����z�� */
	m_nCurLine = nCurLine;				/* ���ݍs */
	m_nListType = nListType;			/* �ꗗ�̎�� */
	m_bLineNumIsCRLF = bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FUNCLIST, lParam, SW_SHOW );
}

/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
void CDlgFuncList::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

/*! �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFuncList::SetData( void/*HWND hwndDlg*/ )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	//2002.02.08 hor �B���Ƃ��ăA�C�e���폜�����Ƃŕ\��
	::ShowWindow( hwndList, SW_HIDE );
	::ShowWindow( hwndTree, SW_HIDE );
	ListView_DeleteAllItems( hwndList );
	TreeView_DeleteAllItems( hwndTree );

	m_cmemClipText.SetDataSz( "" );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */

	if( OUTLINE_CPP == m_nListType ){	/* C++���\�b�h���X�g */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//	::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������FC++���\�b�h�c���[ */
		//SetTreeCpp( m_hWnd );
		//	Jan. 04, 2002 genta Java Method Tree�ɓ���

		SetTreeJava( m_hWnd, TRUE );

		::SetWindowText( m_hWnd, "C++ ���\�b�h�c���[" );
	}else
	if( OUTLINE_FILE == m_nListType ){	//@@@ 2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C������
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, "���[���t�@�C��" );
	}else
	if( OUTLINE_WZTXT == m_nListType ){ //@@@ 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
		m_nViewType = 1;
		SetTree();
		//	2003.06.22 Moca ���O�ύX
		::SetWindowText( m_hWnd, "WZ�K�w�t�e�L�X�g" );
	}else
	if( OUTLINE_HTML == m_nListType ){ //@@@ 2003.05.20 zenryaku HTML�A�E�g���C�����
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, "HTML" );
	}else
	if( OUTLINE_TEX == m_nListType ){ //@@@ 2003.07.20 naoh TeX�A�E�g���C�����
		m_nViewType = 1;
		SetTree();
		::SetWindowText( m_hWnd, "TeX" );
	}else
	if( OUTLINE_TEXT == m_nListType ){ /* �e�L�X�g�E�g�s�b�N���X�g */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������F�e�L�X�g�g�s�b�N�c���[ */
//		SetTreeTxt();
		SetTree();	//@@@ 2002.04.01 YAZAKI �e�L�X�g�g�s�b�N�c���[���A�ėpSetTree���ĂԂ悤�ɕύX�B
		::SetWindowText( m_hWnd, "�e�L�X�g �g�s�b�N�c���[" );
	}else
	if( OUTLINE_JAVA == m_nListType ){ /* Java���\�b�h�c���[ */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//	::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������FJava���\�b�h�c���[ */
		SetTreeJava( m_hWnd, TRUE );
		::SetWindowText( m_hWnd, "Java ���\�b�h�c���[" );
	}else
	if( OUTLINE_COBOL == m_nListType ){ /* COBOL �A�E�g���C�� */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//	::DestroyWindow( hwndList );
//		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������FCOBOL �A�E�g���C�� */
		SetTreeJava( m_hWnd, FALSE );
		::SetWindowText( m_hWnd, "COBOL �A�E�g���C��" );
	}else
	if( OUTLINE_VB == m_nListType ){	/* VisualBasic �A�E�g���C�� */
		// Jul 10, 2003  little YOSHI   �����������Ȃ����̂œƗ�
		m_nViewType = 0;
		/* ���X�g�r���[�R���g���[���̏������FVisual Basic �A�E�g���C�� */
		SetListVB();
		::SetWindowText( m_hWnd, "Visual Basic �A�E�g���C��" );
	}else{
		m_nViewType = 0;
		switch( m_nListType ){
		case OUTLINE_C:
			::SetWindowText( m_hWnd, "C �֐��ꗗ" );
			break;
		case OUTLINE_PLSQL:
			::SetWindowText( m_hWnd, "PL/SQL �֐��ꗗ" );
			break;
		case OUTLINE_ASM:
			::SetWindowText( m_hWnd, "�A�Z���u�� �A�E�g���C��" );
			break;
		case OUTLINE_PERL:	//	Sep. 8, 2000 genta
			::SetWindowText( m_hWnd, "Perl �֐��ꗗ" );
			break;
// Jul 10, 2003  little YOSHI  ��Ɉړ����܂���--->>
//		case OUTLINE_VB:	// 2001/06/23 N.Nakatani for Visual Basic
//			::SetWindowText( m_hWnd, "Visual Basic �A�E�g���C��" );
//			break;
// <<---�����܂�
		case OUTLINE_BOOKMARK:
			LV_COLUMN col;
			col.mask = LVCF_TEXT;
			col.pszText = "�e�L�X�g";
			col.iSubItem = 0;
			//	Apr. 23, 2005 genta �s�ԍ������[��
			ListView_SetColumn( hwndList, 1, &col );
			::SetWindowText( m_hWnd, "�u�b�N�}�[�N" );
			break;
//		case OUTLINE_COBOL:
//			::SetWindowText( m_hWnd, "COBOL�A�E�g���C��" );
//			break;
		}
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//::DestroyWindow( hwndTree );
//		::ShowWindow( hwndTree, SW_HIDE );
		::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );
		nFuncLineOld = 0;
		bSelected = FALSE;
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
			if( !bSelected ){
				if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
					bSelected = TRUE;
					nSelectedLine = i;
				}else
				if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
					bSelected = TRUE;
					nSelectedLine = i - 1;
				}
			}
			nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		}
		if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
			bSelected = TRUE;
			nSelectedLine =  m_pcFuncInfoArr->GetNum() - 1;
		}
		for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
			/* ���݂̉�͌��ʗv�f */
			pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

			//	From Here Apr. 23, 2005 genta �s�ԍ������[��
			/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
			if(m_bLineNumIsCRLF ){
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
			}else{
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
			}
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = szText;
			item.iItem = i;
			item.lParam	= i;
			item.iSubItem = 0;
			ListView_InsertItem( hwndList, &item);

			item.mask = LVIF_TEXT;
			item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr();
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem( hwndList, &item);
			//	To Here Apr. 23, 2005 genta �s�ԍ������[��

			item.mask = LVIF_TEXT;
			if(  1 == pcFuncInfo->m_nInfo ){item.pszText = "�錾";}else
			if( 10 == pcFuncInfo->m_nInfo ){item.pszText = "�֐��錾";}else
			if( 20 == pcFuncInfo->m_nInfo ){item.pszText = "�v���V�[�W���錾";}else
			if( 11 == pcFuncInfo->m_nInfo ){item.pszText = "�֐�";}else
			if( 21 == pcFuncInfo->m_nInfo ){item.pszText = "�v���V�[�W��";}else
			if( 31 == pcFuncInfo->m_nInfo ){item.pszText = "���p�b�P�[�W�d�l��";}else
			if( 41 == pcFuncInfo->m_nInfo ){item.pszText = "���p�b�P�[�W�{�̕�";}else
			if( 50 == pcFuncInfo->m_nInfo ){item.pszText = "PROC";}else
			if( 51 == pcFuncInfo->m_nInfo ){item.pszText = "���x��";}else
			if( 52 == pcFuncInfo->m_nInfo ){item.pszText = "ENDP";}else{
				// Jul 10, 2003  little YOSHI
				// �����ɂ�����VB�֌W�̏�����SetListVB()���\�b�h�Ɉړ����܂����B

				item.pszText = "";
			}
			item.iItem = i;
			item.iSubItem = 2;
			ListView_SetItem( hwndList, &item);

			/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
			if(lstrlen(item.pszText)){
				// ���o���ʂ̎��(�֐�,,,)������Ƃ�
				wsprintf( szText, "%s(%d): %s(%s)\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_cmemFuncName.GetPtr(),	/* ���o���� */
					item.pszText								/* ���o���ʂ̎�� */
				);
			}else{
				// ���o���ʂ̎��(�֐�,,,)���Ȃ��Ƃ�
				wsprintf( szText, "%s(%d): %s\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_cmemFuncName.GetPtr()	/* ���o���� */
				);
			}
			m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		}
		//2002.02.08 hor List�͗񕝒����Ƃ������s����O�ɕ\�����Ƃ��Ȃ��ƕςɂȂ�
		::ShowWindow( hwndList, SW_SHOW );
		/* ��̕����f�[�^�ɍ��킹�Ē��� */
		ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 0, ListView_GetColumnWidth( hwndList, 0 ) + 16 );
		ListView_SetColumnWidth( hwndList, 1, ListView_GetColumnWidth( hwndList, 1 ) + 16 );
		ListView_SetColumnWidth( hwndList, 2, ListView_GetColumnWidth( hwndList, 2 ) + 16 );

		// 2005.07.05 ������
		DWORD dwExStyle  = ListView_GetExtendedListViewStyle( hwndList );
		dwExStyle |= LVS_EX_FULLROWSELECT;
		ListView_SetExtendedListViewStyle( hwndList, dwExStyle );

		if( bSelected ){
			ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
			ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
			ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		}
	}
	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );
	/* �A�E�g���C�� �u�b�N�}�[�N�ꗗ�ŋ�s�𖳎����� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable, m_pShareData->m_Common.m_bMarkUpBlankLineEnable );
	/* �A�E�g���C�� �W�����v������t�H�[�J�X���ڂ� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump, m_pShareData->m_Common.m_bFunclistSetFocusOnJump );

	/* �A�E�g���C�� ���ʒu�ƃT�C�Y���L������ */ // 20060201 aroka
	::CheckDlgButton( m_hWnd, IDC_BUTTON_WINSIZE, m_pShareData->m_Common.m_bRememberOutlineWindowPos );

	/* �_�C�A���O�������I�ɕ���Ȃ�t�H�[�J�X�ړ��I�v�V�����͊֌W�Ȃ� */
	if(m_pShareData->m_Common.m_bAutoCloseDlgFuncList){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
	}

	//2002.02.08 hor
	//�iIDC_LIST1��IDC_TREE1����ɑ��݂��Ă��āAm_nViewType�ɂ���āA�ǂ����\�����邩��I��ł���j
	if(m_nViewType){
		::ShowWindow( hwndTree, SW_SHOW );
		::SetFocus( hwndTree );
	}else{
//		::ShowWindow( hwndList, SW_SHOW );
		::SetFocus( hwndList );
	}
	//2002.02.08 hor
	//��s���ǂ��������̃`�F�b�N�{�b�N�X�̓u�b�N�}�[�N�ꗗ�̂Ƃ������\������
	if(OUTLINE_BOOKMARK == m_nListType){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), TRUE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), SW_SHOW );
	}else{
		::ShowWindow( GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), SW_HIDE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable ), FALSE );
	}
	// 2002/11/1 frozen ���ڂ̃\�[�g���ݒ肷��R���{�{�b�N�X�̓u�b�N�}�[�N�ꗗ�̈ȊO�̎��ɕ\������
	// Nov. 5, 2002 genta �c���[�\���̎������\�[�g��R���{�{�b�N�X��\��
	if( m_nViewType == 1 ){
		HWND hWnd_Combo_Sort = ::GetDlgItem( m_hWnd, IDC_COMBO_nSortType );
		::EnableWindow( hWnd_Combo_Sort , TRUE );
		::ShowWindow( hWnd_Combo_Sort , SW_SHOW );
		::SendMessage( hWnd_Combo_Sort , CB_RESETCONTENT, 0, 0 ); // 2002.11.10 Moca �ǉ�
		::SendMessage( hWnd_Combo_Sort , CB_ADDSTRING, 0, (LPARAM)(_T("�f�t�H���g")));
		::SendMessage( hWnd_Combo_Sort , CB_ADDSTRING, 0, (LPARAM)(_T("�A���t�@�x�b�g��")));
		::SendMessage( hWnd_Combo_Sort , CB_SETCURSEL, m_nSortType, 0L);
		::ShowWindow( GetDlgItem( m_hWnd, IDC_STATIC_nSortType ), SW_SHOW );
		// 2002.11.10 Moca �ǉ� �\�[�g����
		if( 1 == m_nSortType ){
			SortTree(::GetDlgItem( m_hWnd , IDC_TREE1),TVI_ROOT);
		}
	}
	else {
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_nSortType ), FALSE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_COMBO_nSortType ), SW_HIDE );
		::ShowWindow( GetDlgItem( m_hWnd, IDC_STATIC_nSortType ), SW_HIDE );
		//ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );  // 2005.04.05 zenryaku �\�[�g��Ԃ�ێ�
		SortListView( hwndList, m_nSortCol );	// 2005.04.23 genta �֐���(�w�b�_���������̂���)
	}

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* 0==����������   0���傫��==����   0��菬����==���̓G���[ */
int CDlgFuncList::GetData( void )
{
	HWND			hwndList;
	HWND			hwndTree;
	int				nItem;
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	int				nLineTo;
	HTREEITEM		htiItem;
	TV_ITEM			tvi;
	char			szLabel[32];

//2002.02.08 hor OnBnClicked���Ɉړ�
//	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
//	m_pShareData->m_Common.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );

	nLineTo = -1;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	if( m_nViewType == 0 ){
		//	List
		nItem = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
		if( -1 == nItem ){
			return -1;
		}
		item.mask = LVIF_PARAM;
		item.iItem = nItem;
		item.iSubItem = 0;
		ListView_GetItem( hwndList, &item );
		pcFuncInfo = m_pcFuncInfoArr->GetAt( item.lParam );
		nLineTo = pcFuncInfo->m_nFuncLineCRLF;
	}else{
		hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
		if( NULL != hwndTree ){
			htiItem = TreeView_GetSelection( hwndTree );

			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.hItem = htiItem;
			tvi.pszText = szLabel;
			tvi.cchTextMax = sizeof( szLabel );
			if( TreeView_GetItem( hwndTree, &tvi ) ){
				// lParam��-1�ȉ��� pcFuncInfoArr�ɂ͊܂܂�Ȃ�����
				if( 0 <= tvi.lParam ){
					pcFuncInfo = m_pcFuncInfoArr->GetAt( tvi.lParam );
					nLineTo = pcFuncInfo->m_nFuncLineCRLF;
				}
			}
		}
	}
	return nLineTo;
}

#if 0
Jan. 04, 2001 genta Java Tree�ɓ���
/*! �c���[�R���g���[���̏������FC++���\�b�h�c���[ */
void CDlgFuncList::SetTreeCpp( HWND hwndDlg )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	TV_INSERTSTRUCT	tvis;
	char*			pWork;
	char*			pPos;
	char*			pClassName;
	char*			pFuncName;
	//	Jul. 7, 2001 genta
	char			szLabel[64];
	HTREEITEM		htiGlobal;
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
//	char			szText[2048];

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = "�O���[�o��";
	tvis.item.lParam = -1;
	htiGlobal = TreeView_InsertItem( hwndTree, &tvis );

	nFuncLineOld = 0;
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

		pWork = pcFuncInfo->m_cmemFuncName.GetPtr();

		/* �N���X��::���\�b�h�̏ꍇ */
		if( NULL != ( pPos = strstr( pWork, "::" ) ) ){
			//	Apr. 1, 2000 genta
			//	�ǉ��������S�p�ɂ����̂Ń����������ꂾ���K�v
			//	6 == strlen( "�N���X" ), 1 == strlen( '\0' )
			pClassName = new char[pPos - pWork + 6 + 1 ];
			memcpy( pClassName, pWork, pPos - pWork );
			strcpy( &pClassName[pPos - pWork], "�N���X" );
			pFuncName = new char[ lstrlen( pPos + lstrlen( "::" ) ) + 1 ];
			strcpy( pFuncName, pPos + lstrlen( "::" ) );

			/* �N���X���̃A�C�e�����o�^����Ă��邩 */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			while( NULL != htiClass ){
				tvi.mask = TVIF_HANDLE | TVIF_TEXT;
				tvi.hItem = htiClass;
				tvi.pszText = szLabel;
				tvi.cchTextMax = sizeof(szLabel);
				if( TreeView_GetItem( hwndTree, &tvi ) ){
					if( 0 == strcmp( pClassName, szLabel ) ){
						break;
					}
				}
				htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
			}
			/* �N���X���̃A�C�e�����o�^����Ă��Ȃ��̂œo�^ */
			if( NULL == htiClass ){
				tvis.hParent = TVI_ROOT;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
				tvis.item.pszText = pClassName;
				tvis.item.lParam = -1;
				htiClass = TreeView_InsertItem( hwndTree, &tvis );
			}

			/* �Y���N���X���̃A�C�e���̎q�Ƃ��āA���\�b�h�̃A�C�e����o�^ */
			tvis.hParent = htiClass;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = pFuncName;
			tvis.item.lParam = i;
			htiItem = TreeView_InsertItem( hwndTree, &tvis );

			/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
			wsprintf( szText, "%s(%d): %s\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
				pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
				pcFuncInfo->m_cmemFuncName.GetPtr() 	/* ���o���� */
			);
//			m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		}else{
			/* �O���[�o���֐��̏ꍇ */
			pClassName = NULL;
			pFuncName = new char[ lstrlen( pWork ) + 1 ];
			strcpy( pFuncName, pWork );

			strcpy( szText, pFuncName );
			/* �֐��錾�� */
			if( 1 == pcFuncInfo->m_nInfo ){
				strcat( szText, "(�錾)" );
			}

			/* �O���[�o���֐��̃A�C�e����o�^ */
			tvis.hParent = htiGlobal;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = szText;
			tvis.item.lParam = i;
			htiItem = TreeView_InsertItem( hwndTree, &tvis );

			/* �֐��錾�� */
			if( 1 == pcFuncInfo->m_nInfo ){
				/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
				wsprintf( szText, "%s(%d): %s(�錾)\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_cmemFuncName.GetPtr() 	/* ���o���� */
				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
				m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			}else{
				/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
				wsprintf( szText, "%s(%d): %s\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_cmemFuncName.GetPtr() 	/* ���o���� */
				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
				m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			}
		}
		/* ���݃J�[�\���ʒu�̃��\�b�h���ǂ������ׂ� */
		if( !bSelected ){
			if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i;
				htiSelected = htiItem;
			}else
			if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
				htiSelected = htiItemOld;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;

		if( NULL != pClassName ){
			delete [] pClassName;
			pClassName = NULL;
		}
		if( NULL != pFuncName ){
			delete [] pFuncName;
			pFuncName = NULL;
		}
		htiItemOld = htiItem;
	}
	/* �\�[�g�A�m�[�h�̓W�J������ */
//	TreeView_SortChildren( hwndTree, TVI_ROOT, 0 );
	htiClass = TreeView_GetFirstVisible( hwndTree );
	while( NULL != htiClass ){
//		TreeView_SortChildren( hwndTree, htiClass, 0 );
		TreeView_Expand( hwndTree, htiClass, TVE_EXPAND );
		htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
	}
	/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
	if( bSelected ){
		TreeView_SelectItem( hwndTree, htiSelected );
	}else{
		if( NULL != htiItemOld ){
			TreeView_SelectItem( hwndTree, htiItemOld );
		}
	}
//	GetTreeTextNext( hwndTree, NULL, 0 );
	return;
}
#endif


/*! �c���[�R���g���[���̏������FJava���\�b�h�c���[

	Java Method Tree�̍\�z: �֐����X�g������TreeControl������������B

	@date 2002.01.04 genta C++�c���[�𓝍�
*/
void CDlgFuncList::SetTreeJava( HWND hwndDlg, BOOL bAddClass )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	TV_INSERTSTRUCT	tvis;
	char*			pWork;
	char*			pPos;
	char*			pClassName;
	char*			pFuncName;
    char            szLabel[64+6];  // Jan. 07, 2001 genta �N���X���G���A�̊g��
	HTREEITEM		htiGlobal = NULL;	// Jan. 04, 2001 genta C++�Ɠ���
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
	int				nClassNest;
	int				nDummylParam = -64000;	// 2002.11.10 Moca �N���X���̃_�~�[lParam �\�[�g�̂���
	char			szClassArr[16][64];	// Jan. 04, 2001 genta �N���X���G���A�̊g��

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	nFuncLineOld = 0;
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		pWork = pcFuncInfo->m_cmemFuncName.GetPtr();
		/* �N���X��::���\�b�h�̏ꍇ */
		if( NULL != ( pPos = strstr( pWork, "::" ) ) ){
			/* �C���i�[�N���X�̃l�X�g���x���𒲂ׂ� */
			int	k, m;
			int	nWorkLen;
			int	nCharChars;
			nClassNest = 0;
			m = 0;
			nWorkLen = lstrlen( pWork );
			for( k = 0; k < nWorkLen; ++k ){
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pWork, nWorkLen, k );
				if( 1 == nCharChars && ':' == pWork[k] ){
					//	Jan. 04, 2001 genta
					//	C++�̓����̂��߁A\�ɉ�����::���N���X��؂�Ƃ݂Ȃ��悤��
					if( k < nWorkLen - 1 && ':' == pWork[k+1] ){
						memcpy( szClassArr[nClassNest], &pWork[m], k - m );
						szClassArr[nClassNest][k - m] = '\0';
						++nClassNest;
						m = k + 2;
						++k;
					}
					else 
						break;
				}
				else if( 1 == nCharChars && '\\' == pWork[k] ){
					memcpy( szClassArr[nClassNest], &pWork[m], k - m );
					szClassArr[nClassNest][k - m] = '\0';
					++nClassNest;
					m = k + 1;
				}
				if( 2 == nCharChars ){
					++k;
				}
			}
			//	Jan. 04, 2001 genta
			//	::���N���X��؂�Ƃ݂Ȃ��̂ŁA�Ō�̕�����͊֐����Ƃ��Ďc���Ă���
			/*
			if( 0 < k - m ){
				memcpy( szClassArr[nClassNest], &pWork[m], k - m );
				szClassArr[nClassNest][k - m] = '\0';
				++nClassNest;
			}
			*/
//			for( k = 0; k < nClassNest; ++k ){
//				MYTRACE( "%d [%s]\n", k, szClassArr[k] );
//			}
//			MYTRACE( "\n" );
			//	Jan. 04, 2001 genta
			//	�֐��擪�̃Z�b�g(�c���[�\�z�Ŏg��)
			pWork = pWork + m; // 2 == lstrlen( "::" );

			/* �N���X���̃A�C�e�����o�^����Ă��邩 */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			HTREEITEM htiParent = TVI_ROOT;
			for( k = 0; k < nClassNest; ++k ){
				//	Apr. 1, 2001 genta
				//	�ǉ��������S�p�ɂ����̂Ń����������ꂾ���K�v
				//	6 == strlen( "�N���X" ), 1 == strlen( '\0' )

				// 2002/10/30 frozen
				// bAddClass == true �̏ꍇ�̎d�l�ύX
				// �����̍��ڂ́@�u(�N���X��)(���p�X�y�[�X���)(�ǉ�������)�v
				// �ƂȂ��Ă���Ƃ݂Ȃ��AszClassArr[k] �� �u�N���X���v�ƈ�v����΁A�����e�m�[�h�ɐݒ�B
				// �������A��v���鍀�ڂ���������ꍇ�͍ŏ��̍��ڂ�e�m�[�h�ɂ���B
				// ��v���Ȃ��ꍇ�́u(�N���X��)(���p�X�y�[�X���)�N���X�v�̃m�[�h���쐬����B
				size_t nClassNameLen = strlen( szClassArr[k] );
				for( ; NULL != htiClass ; htiClass = TreeView_GetNextSibling( hwndTree, htiClass ))
				{
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;
					tvi.pszText = szLabel;
					tvi.cchTextMax = sizeof(szLabel);
					if( TreeView_GetItem( hwndTree, &tvi ) )
					{
						if( 0 == strncmp( szClassArr[k],szLabel,nClassNameLen) )
						{
							if(sizeof(szLabel) < (nClassNameLen +1) )
								break;// �o�b�t�@�s���ł͖������Ƀ}�b�`����
							else
							{
								if(bAddClass)
								{
									if(szLabel[nClassNameLen]==' ')
										break;
								}
								else
								{
									if(szLabel[nClassNameLen]=='\0')
										break;
								}
							}
						}
					}
				}
//				pClassName = new char[ lstrlen( szClassArr[k] ) + 1 + 6 ];
//				strcpy( pClassName, szClassArr[k] );
//				if( bAddClass ){
//					strcat( pClassName, "�N���X" );
//				}
//				while( NULL != htiClass ){
//					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
//					tvi.hItem = htiClass;
//					tvi.pszText = szLabel;
//					tvi.cchTextMax = sizeof(szLabel);
//					if( TreeView_GetItem( hwndTree, &tvi ) ){
//						if( 0 == strcmp( pClassName, szLabel ) ){
//							break;
//						}
//					}
//					htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
//				}
				/* �N���X���̃A�C�e�����o�^����Ă��Ȃ��̂œo�^ */
				if( NULL == htiClass ){
					// 2002/10/28 frozen �ォ�炱���ֈړ�
					pClassName = new char[ lstrlen( szClassArr[k] ) + 1 + 9 ]; // 2002/10/28 frozen +9�͒ǉ����镶����̍ő咷�i" ���O���"���ő�j
					strcpy( pClassName, szClassArr[k] );

					tvis.item.lParam = -1;
					if( bAddClass )
					{
						if( pcFuncInfo->m_nInfo == 7 )
						{
							strcat( pClassName, " ���O���" );
							tvis.item.lParam = i;
						}
						else
							strcat( pClassName, " �N���X" );
							tvis.item.lParam = nDummylParam;
							nDummylParam++;
					}

					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = pClassName;

					htiClass = TreeView_InsertItem( hwndTree, &tvis );
					//	Jan. 04, 2001 genta
					//	�s�v�ɂȂ����炳�����ƍ폜
					delete [] pClassName; // 2002/10/28 frozen �����炱���ֈړ�

				}else{
					//none
				}
				htiParent = htiClass;
				//if( k + 1 >= nClassNest ){
				//	break;
				//}
				htiClass = TreeView_GetChild( hwndTree, htiClass );
			}
			htiClass = htiParent;
		}else{
			//	Jan. 04, 2001 genta
			//	Global��Ԃ̏ꍇ (C++�̂�)

			// 2002/10/27 frozen ��������
			if( 3 <= pcFuncInfo->m_nInfo  && pcFuncInfo->m_nInfo <= 7 )
				htiClass = TVI_ROOT;
			else
			{
			// 2002/10/27 frozen �����܂�
				if( htiGlobal == NULL ){
					TV_INSERTSTRUCT	tvg;
					
					::ZeroMemory( &tvg, sizeof(tvg));
					tvg.hParent = TVI_ROOT;
					tvg.hInsertAfter = TVI_LAST;
					tvg.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvg.item.pszText = "�O���[�o��";
//					tvg.item.lParam = -1;
					tvg.item.lParam = nDummylParam;
					htiGlobal = TreeView_InsertItem( hwndTree, &tvg );
					nDummylParam++;
				}
				htiClass = htiGlobal;
			}
		}
		pFuncName = new char[ strlen(pWork) + 1 + 7 ];	// +6 �͒ǉ����镶����̍ő咷
		strcpy( pFuncName, pWork );

		// 2002/10/27 frozen �ǉ�������̎�ނ𑝂₵��
		switch(pcFuncInfo->m_nInfo)
		{// case 4�ȏ�̊e�ǉ�������̍ŏ��ɂ��锼�p�X�y�[�X���ȗ����邱�Ƃ͂ł��Ȃ��B
		case 1: strcat( pFuncName, "(�錾)" );break;
		case 3: strcat( pFuncName, " �N���X" );break;
		case 4: strcat( pFuncName, " �\����" );break;
		case 5: strcat( pFuncName, " �񋓑�" );break;
		case 6: strcat( pFuncName, " ���p��" );break;
//		case 7: strcat( pFuncName, " ���O���" );break;
		};
//		}
		/* �Y���N���X���̃A�C�e���̎q�Ƃ��āA���\�b�h�̃A�C�e����o�^ */
		tvis.hParent = htiClass;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = pFuncName;
		tvis.item.lParam = i;
		htiItem = TreeView_InsertItem( hwndTree, &tvis );

		/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
		wsprintf( szText, "%s(%d): %s %s\r\n",
			m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
			pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
			pcFuncInfo->m_cmemFuncName.GetPtr(), 	/* ���o���� */
			( 1 == pcFuncInfo->m_nInfo ? "(�錾)" : "" ) 	//	Jan. 04, 2001 genta C++�Ŏg�p
		);
		m_cmemClipText.AppendSz( (const char *)szText ); /* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		delete [] pFuncName;

		/* ���݃J�[�\���ʒu�̃��\�b�h���ǂ������ׂ� */
		if( !bSelected ){
			if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i;
				htiSelected = htiItem;
			}else
			if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
				htiSelected = htiItemOld;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
		htiItemOld = htiItem;
		//	Jan. 04, 2001 genta
		//	delete�͂��̓s�x�s���̂ł����ł͕s�v
		}
	/* �\�[�g�A�m�[�h�̓W�J������ */
//	TreeView_SortChildren( hwndTree, TVI_ROOT, 0 );
	htiClass = TreeView_GetFirstVisible( hwndTree );
	while( NULL != htiClass ){
//		TreeView_SortChildren( hwndTree, htiClass, 0 );
		TreeView_Expand( hwndTree, htiClass, TVE_EXPAND );
		htiClass = TreeView_GetNextSibling( hwndTree, htiClass );
	}
	/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
	if( bSelected ){
		TreeView_SelectItem( hwndTree, htiSelected );
	}else{
		if( NULL != htiItemOld ){
			TreeView_SelectItem( hwndTree, htiItemOld );
		}
	}
//	GetTreeTextNext( hwndTree, NULL, 0 );
	return;
}


/*! ���X�g�r���[�R���g���[���̏������FVisualBasic

  �����Ȃ����̂œƗ������܂����B

  @date Jul 10, 2003  little YOSHI
*/
void CDlgFuncList::SetListVB (void)
{
	int				i;
	char			szText[2048], szType[128], szOption[256];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	HWND			hwndList;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );

	nFuncLineOld = 0;
	bSelected = FALSE;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		if( !bSelected ){
			if( i == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i;
			}else
			if( i > 0 && nFuncLineOld <= m_nCurLine && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
				bSelected = TRUE;
				nSelectedLine = i - 1;
			}
		}
		nFuncLineOld = pcFuncInfo->m_nFuncLineLAYOUT;
	}
	if( 0 < m_pcFuncInfoArr->GetNum() && !bSelected ){
		bSelected = TRUE;
		nSelectedLine =  m_pcFuncInfoArr->GetNum() - 1;
	}
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		/* ���݂̉�͌��ʗv�f */
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );

		//	From Here Apr. 23, 2005 genta �s�ԍ������[��
		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if(m_bLineNumIsCRLF ){
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
		}else{
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
		}
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam	= i;
		ListView_InsertItem( hwndList, &item);

		item.mask = LVIF_TEXT;
		item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr();
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem( hwndList, &item);
		//	To Here Apr. 23, 2005 genta �s�ԍ������[��

		item.mask = LVIF_TEXT;

		// 2001/06/23 N.Nakatani for Visual Basic
		//	Jun. 26, 2001 genta ���p���ȁ��S�p��
		memset(szText, '\0', sizeof(szText));
		memset(szType, '\0', sizeof(szType));
		memset(szOption, '\0', sizeof(szOption));
		if( 1 == ((pcFuncInfo->m_nInfo >> 8) & 0x01) ){
			// �X�^�e�B�b�N�錾(Static)
			strcpy(szOption, "�ÓI");
		}
		switch ((pcFuncInfo->m_nInfo >> 4) & 0x0f) {
			case 2  :	// �v���C�x�[�g(Private)
				strncat(szOption, "�v���C�x�[�g", sizeof(szText) - strlen(szText));
				break;

			case 3  :	// �t�����h(Friend)
				strncat(szOption, "�t�����h", sizeof(szText) - strlen(szText));
				break;

			default :	// �p�u���b�N(Public)
				strncat(szOption, "�p�u���b�N", sizeof(szText) - strlen(szText));
		}
		switch (pcFuncInfo->m_nInfo & 0x0f) {
			case 1:		// �֐�(Function)
				strcpy(szType, "�֐�");
				break;

			case 2:		// �X�e�[�^�X(Sub)
				strcpy(szType, "�X�e�[�^�X");
				break;

			case 3:		// �v���p�e�B �擾(Property Get)
				strcpy(szType, "�v���p�e�B �擾");
				break;

			case 4:		// �v���p�e�B �ݒ�(Property Let)
				strcpy(szType, "�v���p�e�B �ݒ�");
				break;

			case 5:		// �v���p�e�B �Q��(Property Set)
				strcpy(szType, "�v���p�e�B �Q��");
				break;

			case 6:		// �萔(Const)
				strcpy(szType, "�萔");
				break;

			case 7:		// �񋓌^(Enum)
				strcpy(szType, "�񋓌^");
				break;

			case 8:		// ���[�U��`�^(Type)
				strcpy(szType, "���[�U��`�^");
				break;

			case 9:		// �C�x���g(Event)
				strcpy(szType, "�C�x���g");
				break;

			default:	// ����`�Ȃ̂ŃN���A
				pcFuncInfo->m_nInfo	= 0;

		}
		if ( 2 == ((pcFuncInfo->m_nInfo >> 8) & 0x02) ) {
			// �錾(Declare�Ȃ�)
			strncat(szType, "�錾", sizeof(szType) - strlen(szType));
		}

		if ( 0 == pcFuncInfo->m_nInfo ) {
			memset(szText, '\0', sizeof(szText));
		} else
		if ( 0 == strlen(szOption) ) {
			wsprintf(szText, "%s", szType);
		} else {
			wsprintf(szText, "%s�i%s�j", szType, szOption);
		}
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem( hwndList, &item);

		/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
		if(lstrlen(item.pszText)){
			// ���o���ʂ̎��(�֐�,,,)������Ƃ�
			wsprintf( szText, "%s(%d): %s(%s)\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
				pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
				pcFuncInfo->m_cmemFuncName.GetPtr(),		/* ���o���� */
				item.pszText								/* ���o���ʂ̎�� */
			);
		}else{
			// ���o���ʂ̎��(�֐�,,,)���Ȃ��Ƃ�
			wsprintf( szText, "%s(%d): %s\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
				pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
				pcFuncInfo->m_cmemFuncName.GetPtr()			/* ���o���� */
			);
		}
		m_cmemClipText.AppendSz( (const char *)szText );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
	}

	//2002.02.08 hor List�͗񕝒����Ƃ������s����O�ɕ\�����Ƃ��Ȃ��ƕςɂȂ�
	::ShowWindow( hwndList, SW_SHOW );
	/* ��̕����f�[�^�ɍ��킹�Ē��� */
	ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 0, ListView_GetColumnWidth( hwndList, 0 ) + 16 );
	ListView_SetColumnWidth( hwndList, 1, ListView_GetColumnWidth( hwndList, 1 ) + 16 );
	ListView_SetColumnWidth( hwndList, 2, ListView_GetColumnWidth( hwndList, 2 ) + 16 );
	if( bSelected ){
		ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
		ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
		ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	return;
}

#if 0
2002.04.01 YAZAKI ���Ƃ��Ǝg�p����Ă��Ȃ���������
void CDlgFuncList::GetTreeTextNext(
		HWND		hwndTree,
		HTREEITEM	htiParent,
		int			nDepth
)
{
	HTREEITEM		htiItem;
	TV_ITEM			tvi;
	int				i;
	char			szWork[1024];

	if( NULL == htiParent ){
		htiItem = TreeView_GetRoot( hwndTree );
	}else{
		htiItem = TreeView_GetChild( hwndTree, htiParent );
	}
	while( NULL != htiItem ){
		tvi.mask = TVIF_HANDLE | TVIF_TEXT;
		tvi.hItem = htiItem;
		tvi.pszText = (LPSTR)szWork;
		tvi.cchTextMax = sizeof( szWork );
		TreeView_GetItem( hwndTree, &tvi );
		for( i = 0; i < nDepth; ++i ){
//			m_cmemClipText.Append( "  ", 2 );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			m_cmemClipText.AppendSz( "  " );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		}
//		m_cmemClipText.Append( (const char *)tvi.pszText, lstrlen( (const char *)tvi.pszText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		m_cmemClipText.AppendSz( (const char *)tvi.pszText );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
//		m_cmemClipText.Append( (const char *)"\r\n", 2 );		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		m_cmemClipText.AppendSz( (const char *)"\r\n" );		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		GetTreeTextNext( hwndTree, htiItem, nDepth + 1 );

		htiItem = TreeView_GetNextSibling( hwndTree, htiItem );
	}
	return;
}
#endif

/*! �ėp�c���[�R���g���[���̏������FCFuncInfo::m_nDepth�𗘗p���Đe�q��ݒ�

	@date 2002.04.01 YAZAKI
	@date 2002.11.10 Moca �K�w�̐������Ȃ�����
*/
void CDlgFuncList::SetTree()
{
	HTREEITEM hItemSelected = NULL;
	HWND hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	int i;
	int nFuncInfoArrNum = m_pcFuncInfoArr->GetNum();
	int nStackPointer = 0;
	int nStackDepth = 32; // phParentStack �̊m�ۂ��Ă��鐔
	HTREEITEM* phParentStack;
	phParentStack = (HTREEITEM*)malloc( nStackDepth * sizeof( HTREEITEM ) );
	phParentStack[ nStackPointer ] = TVI_ROOT;

	for (i = 0; i < nFuncInfoArrNum; i++){
		CFuncInfo* pcFuncInfo = m_pcFuncInfoArr->GetAt(i);

		/*	�V�����A�C�e�����쐬
			���݂̐e�̉��ɂԂ牺����`�ŁA�Ō�ɒǉ�����B
		*/
		HTREEITEM hItem;
		TV_INSERTSTRUCT cTVInsertStruct;
		cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		cTVInsertStruct.hInsertAfter = TVI_LAST;	//	�K���Ō�ɒǉ��B
		cTVInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		cTVInsertStruct.item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr();
		cTVInsertStruct.item.lParam = i;	//	���Ƃł��̐��l�i��m_pcFuncInfoArr�̉��Ԗڂ̃A�C�e�����j�����āA�ړI�n�ɃW�����v���邺!!�B

		/*	�e�q�֌W���`�F�b�N
		*/
		if (nStackPointer != pcFuncInfo->m_nDepth){
			//	���x�����ς��܂���!!
			//	�����A2�i�K�[���Ȃ邱�Ƃ͍l�����Ă��Ȃ��̂Œ��ӁB
			//	�@�������A2�i�K�ȏ�󂭂Ȃ邱�Ƃ͍l���ς݁B

			// 2002.11.10 Moca �ǉ� �m�ۂ����T�C�Y�ł͑���Ȃ��Ȃ����B�Ċm��
			if( nStackDepth <= pcFuncInfo->m_nDepth + 1 ){
				nStackDepth = pcFuncInfo->m_nDepth + 4; // ���߂Ɋm�ۂ��Ă���
				HTREEITEM* phTi;
				phTi = (HTREEITEM*)realloc( phParentStack, nStackDepth * sizeof( HTREEITEM ) );
				if( NULL != phTi ){
					phParentStack = phTi;
				}else{
					goto end_of_func;
				}
			}
			nStackPointer = pcFuncInfo->m_nDepth;
			cTVInsertStruct.hParent = phParentStack[ nStackPointer ];
		}
		hItem = TreeView_InsertItem( hwndTree, &cTVInsertStruct );
		phParentStack[ nStackPointer+1 ] = hItem;

		/*	pcFuncInfo�ɓo�^����Ă���s�����m�F���āA�I������A�C�e�����l����
		*/
		if ( pcFuncInfo->m_nFuncLineLAYOUT <= m_nCurLine ){
			hItemSelected = hItem;
		}

		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g���쐬���� */
		//	2003.06.22 Moca dummy�v�f�̓c���[�ɓ���邪TAGJUMP�ɂ͉����Ȃ�
		if( pcFuncInfo->IsAddClipText() ){
			int j;
			for( j = 0; j < nStackPointer; ++j ){
				m_cmemClipText.AppendSz( "  " );
			}
			m_cmemClipText.AppendSz( (const char *)pcFuncInfo->m_cmemFuncName.GetPtr() );
			m_cmemClipText.AppendSz( (const char *)"\r\n" );
		}
	}

end_of_func:;

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	if( NULL != hItemSelected ){
		/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
		TreeView_SelectItem( hwndTree, hItemSelected );
	}

	free( phParentStack );
}

#if 0
2002.04.01 YAZAKI SetTreeTxt()�ASetTreeTxtNest()�͔p�~�B�ėp��SetTree()���g���悤�ɂ��܂����B
/* �c���[�R���g���[���̏������F�e�L�X�g �g�s�b�N�c���[ */
void CDlgFuncList::SetTreeTxt()
{
	HWND			hwndTree;
	int				nBgn;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemSelected;
	TV_INSERTSTRUCT	tvis;
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = "�g�s�b�N";
	tvis.item.lParam = -1;
	htiItem = TreeView_InsertItem( hwndTree, &tvis );
	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );
	nBgn = 0;
	htiItemSelected = NULL;
	SetTreeTxtNest( hwndTree, htiItem, 0, m_pcFuncInfoArr->GetNum(), &htiItemSelected, 0 );
//	TreeView_Expand( hwndTree, htiItem, TVE_EXPAND );
	if( NULL != htiItemSelected ){
		/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
		TreeView_SelectItem( hwndTree, htiItemSelected );
	}
	return;
}





/* �c���[�R���g���[���̏������F�e�L�X�g �g�s�b�N�c���[  �ċA�T�u�֐� */
int CDlgFuncList::SetTreeTxtNest(
	HWND			hwndTree,
	HTREEITEM		htiParent,
	int				nBgn,
	int				nEnd,
	HTREEITEM*		phtiItemSelected,
	int				nDepth
)
{
	CFuncInfo*		pcFuncInfoNext;
	CFuncInfo*		pcFuncInfo;
	CFuncInfo*		pcFuncInfo2;
	unsigned char*	pWork;
	unsigned char*	pWork2;
	int				nCharChars;
	int				nCharChars2;
	int				i;
	TV_INSERTSTRUCT	tvis;
	HTREEITEM		htiItem;
	while( nBgn < nEnd ){
		/* �g�s�b�N���擾 */
		pcFuncInfo = m_pcFuncInfoArr->GetAt( nBgn );
		pWork = (unsigned char*)pcFuncInfo->m_cmemFuncName.GetPtr();
		nCharChars = CMemory::MemCharNext( (char*)pWork, lstrlen( (char*)pWork ), (char*)pWork ) - (char*)pWork;

		tvis.hParent = htiParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = (char*)pWork;
		tvis.item.lParam = nBgn;
		htiItem = TreeView_InsertItem( hwndTree, &tvis );

		for( i = 0; i < nDepth; ++i ){
//			m_cmemClipText.Append( "  ", 2 );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			m_cmemClipText.AppendSz( "  " );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		}
//		m_cmemClipText.Append( (const char *)pWork, lstrlen( (const char *)pWork ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		m_cmemClipText.AppendSz( (const char *)pWork );		/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
//		m_cmemClipText.Append( (const char *)"\r\n", 2 );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		m_cmemClipText.AppendSz( (const char *)"\r\n" );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */

		/* ���݃J�[�\���ʒu�̃��\�b�h���ǂ������ׂ� */
		if( nBgn == 0 && m_nCurLine < pcFuncInfo->m_nFuncLineLAYOUT ){
			/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
//			TreeView_SelectItem( hwndTree, htiItem );
			*phtiItemSelected = htiItem;
		}else{
			if( nBgn + 1 < m_pcFuncInfoArr->GetNum() ){
				pcFuncInfoNext = m_pcFuncInfoArr->GetAt( nBgn + 1 );
				if( m_nCurLine >= pcFuncInfo->m_nFuncLineLAYOUT &&
					m_nCurLine < pcFuncInfoNext->m_nFuncLineLAYOUT
				){
					/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
//					TreeView_SelectItem( hwndTree, htiItem );
					*phtiItemSelected = htiItem;
				}
			}else{
				if( m_nCurLine >= pcFuncInfo->m_nFuncLineLAYOUT ){
					/* ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ��� */
//					TreeView_SelectItem( hwndTree, htiItem );
					*phtiItemSelected = htiItem;
				}
			}
		}
		for( i = nBgn + 1; i < nEnd; ++i ){
			pcFuncInfo2 = m_pcFuncInfoArr->GetAt( i );
			pWork2 = (unsigned char*)pcFuncInfo2->m_cmemFuncName.GetPtr();
			nCharChars2 = CMemory::MemCharNext( (char*)pWork2, lstrlen( (char*)pWork2 ), (char*)pWork2 ) - (char*)pWork2;
			if( nCharChars == nCharChars2 ){
				if( nCharChars == 1 ){
					/* ���p���� */
					if( pWork[0] >= '0' && pWork[0] <= '9' &&
						pWork2[0] >= '0' && pWork2[0] <= '9' ){
						break;
					}
				}else
				if( nCharChars == 2 ){
					/* �S�p���� */
					if( pWork[0] == 0x82 && ( pWork[1] >= 0x4f && pWork[1] <= 0x58 ) &&
						pWork2[0] == 0x82 && ( pWork2[1] >= 0x4f && pWork2[1] <= 0x58 ) ){
						break;
					}
					/* �@�`�S */
					if( pWork[0] == 0x87 && ( pWork[1] >= 0x40 && pWork[1] <= 0x53 ) &&
						pWork2[0] == 0x87 && ( pWork2[1] >= 0x40 && pWork2[1] <= 0x53 ) ){
						break;
					}
					/* �T�`�] */
					if( pWork[0] == 0x87 && ( pWork[1] >= 0x54 && pWork[1] <= 0x5d ) &&
						pWork2[0] == 0x87 && ( pWork2[1] >= 0x54 && pWork2[1] <= 0x5d ) ){
						break;
					}
				}
				if( 0 == memcmp( pWork, pWork2, nCharChars ) ){
					break;
				}
			}
		}
		SetTreeTxtNest( hwndTree, htiItem, nBgn + 1, i, phtiItemSelected, nDepth + 1 );
//		TreeView_Expand( hwndTree, htiItem, TVE_EXPAND );
		nBgn = i;
	}
	return 1;
}
#endif


BOOL CDlgFuncList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	HWND		hwndList;
	int			nCxVScroll;
	int			nColWidthArr[3] = { 0, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST1 );
	// 2005.10.21 zenryaku 1�s�I��
	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		SendMessage(hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) | LVS_EX_FULLROWSELECT);

	::GetWindowRect( hwndList, &rc );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - ( nColWidthArr[1] + nColWidthArr[2] ) - nCxVScroll - 8;
	//	Apr. 23, 2005 genta �s�ԍ������[��
	col.pszText = "�s *";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[1];
	//	Apr. 23, 2005 genta �s�ԍ������[��
	col.pszText = "�֐���";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[2];
	col.pszText = " ";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col);

	/* �A�E�g���C���ʒu�ƃT�C�Y������������ */ // 20060201 aroka
	if( m_lParam != NULL ){
		CEditView* pcEditView=(CEditView*)m_lParam;
		if( m_pShareData->m_Common.m_bRememberOutlineWindowPos ){
			WINDOWPLACEMENT cWindowPlacement;
			cWindowPlacement.length = sizeof( WINDOWPLACEMENT );
			if (::GetWindowPlacement( pcEditView->m_pcEditDoc->m_pcEditWnd->m_hWnd, &cWindowPlacement )){
				/* �E�B���h�E�ʒu�E�T�C�Y��-1�ȊO�̒l�ɂ��Ă����ƁACDialog�Ŏg�p�����D */
				m_xPos = m_pShareData->m_Common.m_xOutlineWindowPos + cWindowPlacement.rcNormalPosition.left;
				m_yPos = m_pShareData->m_Common.m_yOutlineWindowPos + cWindowPlacement.rcNormalPosition.top;
				m_nWidth =  m_pShareData->m_Common.m_widthOutlineWindow;
				m_nHeight = m_pShareData->m_Common.m_heightOutlineWindow;
			}
		}
	}
	/* ���N���X�����o */
	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgFuncList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�A�E�g���C����́v�̃w���v */
		//Apr. 5, 2001 JEPRO �C���R���ǉ� (Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���)
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDOK:
		return OnJump();
	case IDCANCEL:
		if( m_bModal ){		/* ���[�_�� �_�C�A���O�� */
			::EndDialog( m_hWnd, 0 );
		}else{
			::DestroyWindow( m_hWnd );
		}
		return TRUE;
	case IDC_BUTTON_COPY:
		// Windows�N���b�v�{�[�h�ɃR�s�[ 
		// 2004.02.17 Moca �֐���
		SetClipboardText( m_hWnd, m_cmemClipText.GetPtr(), m_cmemClipText.GetLength() );
		return TRUE;
	case IDC_BUTTON_WINSIZE:
		{// �E�B���h�E�̈ʒu�ƃT�C�Y���L�� // 20060201 aroka
			m_pShareData->m_Common.m_bRememberOutlineWindowPos = ::IsDlgButtonChecked( m_hWnd, IDC_BUTTON_WINSIZE );
		}
		return TRUE;
	//2002.02.08 �I�v�V�����ؑ֌�List/Tree�Ƀt�H�[�J�X�ړ�
	case IDC_CHECK_bAutoCloseDlgFuncList:
	case IDC_CHECK_bMarkUpBlankLineEnable:
	case IDC_CHECK_bFunclistSetFocusOnJump:
		m_pShareData->m_Common.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );
		m_pShareData->m_Common.m_bMarkUpBlankLineEnable = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bMarkUpBlankLineEnable );
		m_pShareData->m_Common.m_bFunclistSetFocusOnJump = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump );
		if(m_pShareData->m_Common.m_bAutoCloseDlgFuncList){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_bFunclistSetFocusOnJump ), TRUE );
		}
		if(wID==IDC_CHECK_bMarkUpBlankLineEnable&&m_nListType==OUTLINE_BOOKMARK){
			CEditView* pcEditView=(CEditView*)m_lParam;
			pcEditView->HandleCommand( F_BOOKMARK_VIEW, TRUE, TRUE, 0, 0, 0 );
			m_nCurLine=pcEditView->m_nCaretPosY + 1;
			SetData();
		}else
		if(m_nViewType){
			::SetFocus( ::GetDlgItem( m_hWnd, IDC_TREE1 ) );
		}else{
			::SetFocus( ::GetDlgItem( m_hWnd, IDC_LIST1 ) );
		}
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFuncList::OnNotify( WPARAM wParam, LPARAM lParam )
{
//	int				idCtrl;
	LPNMHDR			pnmh;
	NM_LISTVIEW*	pnlv;
	HWND			hwndList;
	HWND			hwndTree;
	NM_TREEVIEW*	pnmtv;
//	int				nLineTo;

//	idCtrl = (int) wParam;
	pnmh = (LPNMHDR) lParam;
	pnlv = (NM_LISTVIEW*)lParam;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

	if( hwndTree == pnmh->hwndFrom ){
		pnmtv = (NM_TREEVIEW *) lParam;
//		switch( pnmh->code ){
//		case TVN_BEGINDRAG:
//		case TVN_BEGINLABELEDIT:
//		case TVN_BEGINRDRAG:
//		case TVN_DELETEITEM:
//		case TVN_ENDLABELEDIT:
//		case TVN_GETDISPINFO:
//		case TVN_ITEMEXPANDED:
//		case TVN_ITEMEXPANDING:
//		case TVN_KEYDOWN:
//		case TVN_SELCHANGED:
//		case TVN_SELCHANGING:
//		case TVN_SETDISPINFO:
//			break;
//		default:
			switch( pnmtv->hdr.code ){
			case NM_DBLCLK:
				// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 3/4
				OnJump();
				m_bWaitTreeProcess=true;
				return TRUE;
				//return OnJump();
			case TVN_KEYDOWN:
				Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
				return TRUE;
			case TVN_SELCHANGED:
				// 2005.11.04 zenryaku ��ʊO�̑I�����ڂ�ǂ�
				::SendMessage(hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)((NMTREEVIEW*)lParam)->itemNew.hItem);
				break;
//			case NM_CLICK:
			case NM_KILLFOCUS:
				// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 4/4
				if(m_bWaitTreeProcess){
					if(m_pShareData->m_Common.m_bFunclistSetFocusOnJump){
						::SetFocus(m_hwndParent);
					}
					m_bWaitTreeProcess=false;
				}
				return TRUE;
//			case NM_OUTOFMEMORY:
//			case NM_RCLICK:
//			case NM_RDBLCLK:
//			case NM_RETURN:
//			case NM_SETFOCUS:
//			default:
//				break;
			}
//			break;
//		}
	}else
	if( hwndList == pnmh->hwndFrom ){
		switch( pnmh->code ){
		case LVN_COLUMNCLICK:
//				MYTRACE( "LVN_COLUMNCLICK\n" );
			m_nSortCol =  pnlv->iSubItem;
			//	Apr. 23, 2005 genta �֐��Ƃ��ēƗ�������
			SortListView( hwndList, m_nSortCol );
			return TRUE;
		case NM_DBLCLK:
			return OnJump();
		case LVN_KEYDOWN:
			Key2Command( ((LV_KEYDOWN *)lParam)->wVKey );
			return TRUE;
		}
	}
	return FALSE;
}
/*!
	�w�肳�ꂽ�J�����Ń��X�g�r���[���\�[�g����D
	�����Ƀw�b�_������������D

	�\�[�g��̓t�H�[�J�X����ʓ��Ɍ����悤�ɕ\���ʒu�𒲐�����D

	@par �\���ʒu�����̏��Z
	EnsureVisible�̌��ʂ́C��X�N���[���̏ꍇ�͏�[�ɁC���X�N���[���̏ꍇ��
	���[�ɖړI�̍��ڂ������D�[���班�����������ꍇ�̓I�t�Z�b�g��^����K�v��
	���邪�C�X�N���[���������킩��Ȃ��Ɓ}���킩��Ȃ�
	���̂��ߍŏ��Ɉ�ԉ��Ɉ��X�N���[�������邱�Ƃ�EnsureVisible�ł�
	���Ȃ炸��X�N���[���ɂȂ�悤�ɂ��邱�ƂŁC�\�[�g��̕\���ʒu��
	�Œ肷��

	@param[in] hwndList	���X�g�r���[�̃E�B���h�E�n���h��
	@param[in] sortcol	�\�[�g����J�����ԍ�(0-2)

	@date 2005.04.23 genta �֐��Ƃ��ēƗ�������
	@date 2005.04.29 genta �\�[�g��̕\���ʒu����
*/
void CDlgFuncList::SortListView(HWND hwndList, int sortcol)
{
	LV_COLUMN		col;

	//	Apr. 23, 2005 genta �s�ԍ������[��
	if( sortcol == 1 ){
		col.mask = LVCF_TEXT;
	// From Here 2001.12.03 hor
	//	col.pszText = "�֐��� *";
		if(OUTLINE_BOOKMARK == m_nListType){
			col.pszText = "�e�L�X�g *";
		}else{
			col.pszText = "�֐��� *";
		}
	// To Here 2001.12.03 hor
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 1, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "�s";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 0, &col );
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = "";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 2, &col );
	// To Here 2001.12.07 hor
		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	}else
	if( sortcol == 0 ){
		col.mask = LVCF_TEXT;
	// From Here 2001.12.03 hor
	//	col.pszText = "�֐���";
		if(OUTLINE_BOOKMARK == m_nListType){
			col.pszText = "�e�L�X�g";
		}else{
			col.pszText = "�֐���";
		}
	// To Here 2001.12.03 hor
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 1, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "�s *";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 0, &col );
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = "";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 2, &col );
	// To Here 2001.12.03 hor
		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	// From Here 2001.12.07 hor
	}else
	if( sortcol == 2 ){
		col.mask = LVCF_TEXT;
		if(OUTLINE_BOOKMARK == m_nListType){
			col.pszText = "�e�L�X�g";
		}else{
			col.pszText = "�֐���";
		}
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 1, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "�s";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 0, &col );
		col.mask = LVCF_TEXT;
		col.pszText = "*";
		col.iSubItem = 0;
		ListView_SetColumn( hwndList, 2, &col );
		ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
	// To Here 2001.12.07 hor
	}
	//	2005.04.23 zenryaku �I�����ꂽ���ڂ�������悤�ɂ���

	//	Apr. 29, 2005 genta ��U��ԉ��ɃX�N���[��������
	ListView_EnsureVisible( hwndList,
		ListView_GetItemCount(hwndList) - 1,
		FALSE );
	
	//	Jan.  9, 2006 genta �擪����1�ڂ�2�ڂ̊֐���
	//	�I�����ꂽ�ꍇ�ɃX�N���[������Ȃ�����
	int keypos = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED) - 2;
	ListView_EnsureVisible( hwndList,
		keypos >= 0 ? keypos : 0,
		FALSE );
}

/*!	�E�B���h�E�T�C�Y���ύX���ꂽ

	@date 2003.06.22 Moca �R�[�h�̐���(�R���g���[���̏������@���e�[�u���Ɏ�������)
	@date 2003.08.16 genta �z���static��(���ʂȏ��������s��Ȃ�����)
*/
BOOL CDlgFuncList::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* ���N���X�����o */
	CDialog::OnSize( wParam, lParam );

	static const int Controls[][2] = {
		{IDC_CHECK_bFunclistSetFocusOnJump, 1},
		{IDC_CHECK_bMarkUpBlankLineEnable , 1},
		{IDC_CHECK_bAutoCloseDlgFuncList, 1},
		{IDC_BUTTON_WINSIZE, 2}, // 20060201 aroka
		{IDC_BUTTON_COPY, 2},
		{IDOK, 2},
		{IDCANCEL, 2},
		{IDC_BUTTON_HELP, 2},
		{IDC_LIST1, 3},
		{IDC_TREE1, 3},
	};
	int		nControls = sizeof( Controls ) / sizeof( Controls[0] );
//	int		fwSizeType;
	int		nWidth;
	int		nHeight;
	int		i;
	int		nHeightCheckBox;
	int		nHeightButton;
	const int	nHeightMargin = 3;
	RECT	rc;
	HWND	hwndCtrl;
	POINT	po;

	nWidth = LOWORD(lParam);	// width of client area
	nHeight = HIWORD(lParam);	// height of client area


	::GetWindowRect( ::GetDlgItem( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList ), &rc );
	nHeightCheckBox = rc.bottom -  rc.top;
	::GetWindowRect( ::GetDlgItem( m_hWnd, IDOK ), &rc );	
	nHeightButton = rc.bottom - rc.top;

	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( m_hWnd, Controls[i][0] );
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
		//	2003.06.22 Moca �e�[�u����̎�ʂɂ���ď������@��ς���
		switch( Controls[i][1] ){
		case 1:
			::SetWindowPos( hwndCtrl, NULL, 
				rc.left,
				nHeight - nHeightCheckBox - nHeightMargin,
				0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
// 2002/11/1 frozen ��������
		case 2:
			::SetWindowPos( hwndCtrl, NULL,
				rc.left,
				nHeight - nHeightCheckBox - nHeightButton - nHeightMargin * 2,
				0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
		case 3:
			::SetWindowPos( hwndCtrl, NULL, 0, 0, 
				nWidth - 2 * rc.left,
				nHeight - rc.top - nHeightCheckBox - nHeightButton - 3 * nHeightMargin,
				SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			break;
		}
// 2002/11/1 frozen �����܂�
		::InvalidateRect( hwndCtrl, NULL, TRUE );
	}
	return TRUE;
}

int CALLBACK Compare_by_ItemData(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if( lParam1< lParam2 )
		return -1;
	if( lParam1 > lParam2 )
		return 1;
	else
		return 0;
}

BOOL CDlgFuncList::OnDestroy( void )
{
	CDialog::OnDestroy();

	/* �A�E�g���C�� ���ʒu�ƃT�C�Y���L������ */ // 20060201 aroka
	// �O������Fm_lParam �� CDialog::OnDestroy �ŃN���A����Ȃ�����
	CEditView* pcEditView=(CEditView*)m_lParam;
	if( m_pShareData->m_Common.m_bRememberOutlineWindowPos ){
		/* �e�̃E�B���h�E�ʒu�E�T�C�Y���L�� */
		WINDOWPLACEMENT cWindowPlacement;
		cWindowPlacement.length = sizeof( WINDOWPLACEMENT );
		if (::GetWindowPlacement( pcEditView->m_pcEditDoc->m_pcEditWnd->m_hWnd, &cWindowPlacement )){
			/* �E�B���h�E�ʒu�E�T�C�Y���L�� */
			m_pShareData->m_Common.m_xOutlineWindowPos = m_xPos - cWindowPlacement.rcNormalPosition.left;
			m_pShareData->m_Common.m_yOutlineWindowPos = m_yPos - cWindowPlacement.rcNormalPosition.top;
			m_pShareData->m_Common.m_widthOutlineWindow = m_nWidth;
			m_pShareData->m_Common.m_heightOutlineWindow = m_nHeight;
		}

	}
	return TRUE;
}


BOOL CDlgFuncList::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int nSelect = ::SendMessage(hwndCtl,CB_GETCURSEL, 0, 0L);
	switch(wID)
	{
	case IDC_COMBO_nSortType:
		if( m_nSortType != nSelect )
		{
			m_nSortType = nSelect;
			SortTree(::GetDlgItem( m_hWnd , IDC_TREE1),TVI_ROOT);
		}
		return TRUE;
	};
	return FALSE;

}
void  CDlgFuncList::SortTree(HWND hWndTree,HTREEITEM htiParent)
{
	if( m_nSortType == 1 )
		TreeView_SortChildren(hWndTree,htiParent,TRUE);
	else
	{
		TVSORTCB sort;
		sort.hParent =  htiParent;
		sort.lpfnCompare = Compare_by_ItemData;
		sort.lParam = 0;
		TreeView_SortChildrenCB(hWndTree , &sort , TRUE);
	}
	for(HTREEITEM htiItem = TreeView_GetChild( hWndTree, htiParent ); NULL != htiItem ; htiItem = TreeView_GetNextSibling( hWndTree, htiItem ))
		SortTree(hWndTree,htiItem);
}



BOOL CDlgFuncList::OnJump( bool bCheckAutoClose )	//2002.02.08 hor �����ǉ�
{
	int				nLineTo;
	/* �_�C�A���O�f�[�^�̎擾 */
	if( 0 < ( nLineTo = GetData() ) ){
		if( m_bModal ){		/* ���[�_�� �_�C�A���O�� */
			::EndDialog( m_hWnd, nLineTo );
		}else{
			/* �J�[�\�����ړ������� */
			POINT	poCaret;
			poCaret.x = 0;
			poCaret.y = nLineTo - 1;
			memcpy( m_pShareData->m_szWork, (void*)&poCaret, sizeof(poCaret) );
			//	2006.07.09 genta �ړ����ɑI����Ԃ�ێ�����悤��
			::SendMessage( ::GetParent( ::GetParent( m_hwndParent ) ),
				MYWM_SETCARETPOS, 0, PM_SETCARETPOS_KEEPSELECT );
			if( bCheckAutoClose ){
				/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
				if( m_pShareData->m_Common.m_bAutoCloseDlgFuncList ){
					::DestroyWindow( m_hWnd );
				}else
				if( m_pShareData->m_Common.m_bFunclistSetFocusOnJump ){
					::SetFocus( m_hwndParent );
				}
			}
		}
	}
	return TRUE;
}


//@@@ 2002.01.18 add start
LPVOID CDlgFuncList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


/*!	�L�[������R�}���h�ɕϊ�����w���p�[�֐�
	
*/
void CDlgFuncList::Key2Command(WORD KeyCode)
{
	CEditView*	pcEditView;
	int nIdx, nFuncCode;
// novice 2004/10/10
	/* Shift,Ctrl,Alt�L�[��������Ă����� */
	nIdx = getCtrlKeyState();
	nFuncCode=CKeyBind::GetFuncCode(
			((WORD)(((BYTE)(KeyCode)) | ((WORD)((BYTE)(nIdx))) << 8)),
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr
	);
	switch( nFuncCode ){
	case F_REDRAW:
		nFuncCode=(m_nListType==OUTLINE_BOOKMARK)?F_BOOKMARK_VIEW:F_OUTLINE;
		/*FALLTHROUGH*/
	case F_OUTLINE:
	case F_OUTLINE_TOGGLE: // 20060201 aroka �t�H�[�J�X������Ƃ��̓����[�h
	case F_BOOKMARK_VIEW:
		pcEditView=(CEditView*)m_lParam;
		pcEditView->HandleCommand( nFuncCode, TRUE, SHOW_RELOAD, 0, 0, 0 ); // �����̕ύX 20060201 aroka

		// 2002.11.11 Moca CEditView::HandleCommand��Readraw�Ɖ���Ă��邽�ߍX�V���Ȃ��Ă悢
//		m_nListType=(nFuncCode==F_BOOKMARK_VIEW)?OUTLINE_BOOKMARK:pcEditView->m_pcEditDoc->GetDocumentAttribute().m_nDefaultOutline;
//		m_nCurLine=pcEditView->m_nCaretPosY + 1;
//		SetData(); 
		break;
	case F_BOOKMARK_SET:
		OnJump( false );
		pcEditView=(CEditView*)m_lParam;
		pcEditView->HandleCommand( nFuncCode, TRUE, 0, 0, 0, 0 );
	//	if( m_pShareData->m_Common.m_bAutoCloseDlgFuncList ){
	//		OnBnClicked( IDCANCEL );
	//	}else
	//	if( m_pShareData->m_Common.m_bFunclistSetFocusOnJump ){
	//		::SetFocus( m_hwndParent );
	//	}
		break;
	case F_COPY:
	case F_CUT:
		OnBnClicked( IDC_BUTTON_COPY );
		break;
	}
}

/*!
	@date 2002.10.05 genta
*/
void CDlgFuncList::Redraw( int nOutLineType, CFuncInfoArr* pcFuncInfoArr, int nCurLine )
{
	m_nListType = nOutLineType;
	m_pcFuncInfoArr = pcFuncInfoArr;	/* �֐����z�� */
	m_nCurLine = nCurLine;				/* ���ݍs */
	SetData();
}

/*[EOF]*/
