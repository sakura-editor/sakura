//	$Id$
/*!	@file
	�A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani Visual Basic�̃A�E�g���C�����
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>
#include <commctrl.h>
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgFuncList.h"
#include "etc_uty.h"
#include "debug.h"
#include "global.h"
#include "CEditView.h"
#include "funccode.h"		//Stonee, 2001/03/12

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
	if( 0 == pcDlgFuncList->m_nSortCol){	/* �\�[�g�����ԍ� */
		return strcmp( pcFuncInfo1->m_cmemFuncName.GetPtr( NULL ), pcFuncInfo2->m_cmemFuncName.GetPtr( NULL ) );
	}
	if( 1 == pcDlgFuncList->m_nSortCol){	/* �\�[�g�����ԍ� */
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
	m_nSortCol = 0;				/* �\�[�g�����ԍ� */
	m_bLineNumIsCRLF = FALSE;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
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
	m_cmemClipText.SetDataSz( "" );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */

	if( OUTLINE_CPP == m_nListType ){	/* C++���\�b�h���X�g */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//	::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������FC++���\�b�h�c���[ */
		SetTreeCpp( m_hWnd );
		::SetWindowText( m_hWnd, "C++ ���\�b�h�c���[" );
	}else
	if( OUTLINE_TEXT == m_nListType ){ /* �e�L�X�g�E�g�s�b�N���X�g */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������F�e�L�X�g�g�s�b�N�c���[ */
		SetTreeTxt( m_hWnd );
		::SetWindowText( m_hWnd, "�e�L�X�g �g�s�b�N�c���[" );
	}else
	if( OUTLINE_JAVA == m_nListType ){ /* Java���\�b�h�c���[ */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//	::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������FJava���\�b�h�c���[ */
		SetTreeJava( m_hWnd, TRUE );
		::SetWindowText( m_hWnd, "Java ���\�b�h�c���[" );
	}else
	if( OUTLINE_COBOL == m_nListType ){ /* COBOL �A�E�g���C�� */
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//	::DestroyWindow( hwndList );
		::ShowWindow( hwndList, SW_HIDE );
		m_nViewType = 1;
		/* �c���[�R���g���[���̏������FCOBOL �A�E�g���C�� */
		SetTreeJava( m_hWnd, FALSE );
		::SetWindowText( m_hWnd, "COBOL �A�E�g���C��" );
// From Here 2001.12.03 hor
//	�u�b�N�}�[�N�̈ꗗ�𖳗������
	}else
	if( OUTLINE_BOOKMARK == m_nListType ){	/* �u�b�N�}�[�N���X�g */
		::ShowWindow( hwndTree, SW_HIDE );
		SetTreeBookMark( m_hWnd );
		m_nViewType = 0;
		::SetWindowText( m_hWnd, "�u�b�N�}�[�N" );
// To Here 2001.12.03 hor
	}else{
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
		case OUTLINE_VB:	// 2001/06/23 N.Nakatani for Visual Basic
			::SetWindowText( m_hWnd, "Visual Basic �A�E�g���C��" );
			break;
//		case OUTLINE_COBOL:
//			::SetWindowText( m_hWnd, "COBOL�A�E�g���C��" );
//			break;
		}
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//::DestroyWindow( hwndTree );
		::ShowWindow( hwndTree, SW_HIDE );
		m_nViewType = 0;
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

			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
			item.iItem = i;
			item.iSubItem = 0;
			item.lParam	= i;
			ListView_InsertItem( hwndList, &item);

			/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
			if(m_bLineNumIsCRLF ){
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
			}else{
				wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
			}
			item.mask = LVIF_TEXT;
			item.pszText = szText;
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem( hwndList, &item);

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

			// 2001/06/23 N.Nakatani for Visual Basic
			//	Jun. 26, 2001 genta ���p���ȁ��S�p��
			if( 60 == pcFuncInfo->m_nInfo ){item.pszText = "�X�e�[�g�����g�錾";}else
			if( 61 == pcFuncInfo->m_nInfo ){item.pszText = "�֐��錾";}else
			if( 62 == pcFuncInfo->m_nInfo ){item.pszText = "�X�e�[�g�����g";}else
			if( 63 == pcFuncInfo->m_nInfo ){item.pszText = "�֐�";}else

				item.pszText = "";
			}
			item.iItem = i;
			item.iSubItem = 2;
			ListView_SetItem( hwndList, &item);

			/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
			wsprintf( szText, "%s(%d): %s(%s)\r\n",
				m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
				pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
				pcFuncInfo->m_cmemFuncName.GetPtr( NULL ),	/* ���o���� */
				item.pszText								/* ���o���ʂ̎�� */
			);
//			m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
		}
		if( bSelected ){
			ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
			ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
			ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
//			ListView_GetItemRect( hwndList, nSelectedLine, &rc, LVIR_BOUNDS );
//			::PostMessage( hwndList, WM_LBUTTONDOWN, 0, MAKELONG( rc.left + 2, rc.bottom - 2) );
		}
		/* ��̕����f�[�^�ɍ��킹�Ē��� */
		ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 2, LVSCW_AUTOSIZE );
		ListView_SetColumnWidth( hwndList, 0, ListView_GetColumnWidth( hwndList, 0 ) + 16 );
		ListView_SetColumnWidth( hwndList, 1, ListView_GetColumnWidth( hwndList, 1 ) + 16 );
		ListView_SetColumnWidth( hwndList, 2, ListView_GetColumnWidth( hwndList, 2 ) + 16 );
	}
	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );
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

	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	m_pShareData->m_Common.m_bAutoCloseDlgFuncList = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList );

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
				if( -1 != tvi.lParam ){
					pcFuncInfo = m_pcFuncInfoArr->GetAt( tvi.lParam );
					nLineTo = pcFuncInfo->m_nFuncLineCRLF;
				}
			}
		}
	}
	return nLineTo;
}


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

		pWork = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );

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
				pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* ���o���� */
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
					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* ���o���� */
				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
				m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			}else{
				/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
				wsprintf( szText, "%s(%d): %s\r\n",
					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* ���o���� */
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



/* �c���[�R���g���[���̏������FJava���\�b�h�c���[ */
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
	char			szLabel[32];
//	HTREEITEM		htiGlobal;
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiItemOld;
	HTREEITEM		htiSelected;
	TV_ITEM			tvi;
	int				nClassNest;
	char			szClassArr[16][48];
//	char			szText[2048];

	::EnableWindow( ::GetDlgItem( m_hWnd , IDC_BUTTON_COPY ), TRUE );

	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );

//	tvis.hParent = TVI_ROOT;
//	tvis.hInsertAfter = TVI_LAST;
//	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
//	tvis.item.pszText = "�O���[�o��";
//	tvis.item.lParam = -1;
//	htiGlobal = TreeView_InsertItem( hwndTree, &tvis );
	nFuncLineOld = 0;
	bSelected = FALSE;
	htiItemOld = NULL;
	for( i = 0; i < m_pcFuncInfoArr->GetNum(); ++i ){
		pcFuncInfo = m_pcFuncInfoArr->GetAt( i );
		pWork = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
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
				nCharChars = CMemory::MemCharNext( pWork, nWorkLen, &pWork[k] ) - &pWork[k];
				if( 1 == nCharChars && ':' == pWork[k] ){
					break;
				}
				if( 1 == nCharChars && '\\' == pWork[k] ){
					memcpy( szClassArr[nClassNest], &pWork[m], k - m );
					szClassArr[nClassNest][k - m] = '\0';
					++nClassNest;
					m = k + 1;
				}
				if( 2 == nCharChars ){
					++k;
				}
			}
			if( 0 < k - m ){
				memcpy( szClassArr[nClassNest], &pWork[m], k - m );
				szClassArr[nClassNest][k - m] = '\0';
				++nClassNest;
			}
//			for( k = 0; k < nClassNest; ++k ){
//				MYTRACE( "%d [%s]\n", k, szClassArr[k] );
//			}
//			MYTRACE( "\n" );
			pFuncName = new char[ lstrlen( pPos + lstrlen( "::" ) ) + 1 ];
			strcpy( pFuncName, pPos + lstrlen( "::" ) );

			/* �N���X���̃A�C�e�����o�^����Ă��邩 */
			htiClass = TreeView_GetFirstVisible( hwndTree );
			HTREEITEM htiParent = TVI_ROOT;
			for( k = 0; k < nClassNest; ++k ){
				//	Apr. 1, 2001 genta
				//	�ǉ��������S�p�ɂ����̂Ń����������ꂾ���K�v
				//	6 == strlen( "�N���X" ), 1 == strlen( '\0' )
				pClassName = new char[ lstrlen( szClassArr[k] ) + 1 + 6 ];
				strcpy( pClassName, szClassArr[k] );
				if( bAddClass ){
					strcat( pClassName, "�N���X" );
				}
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
					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = pClassName;
					tvis.item.lParam = -1;
					htiClass = TreeView_InsertItem( hwndTree, &tvis );
				}else{

				}
				htiParent = htiClass;
				if( k + 1 >= nClassNest ){
					break;
				}
				htiClass = TreeView_GetChild( hwndTree, htiClass );
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
				pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* ���o���� */
			);
//			m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
			m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
//		}else{
//			/* �O���[�o���֐��̏ꍇ */
//			pClassName = NULL;
//			pFuncName = new char[ lstrlen( pWork ) + 1 ];
//			strcpy( pFuncName, pWork );
//
//			strcpy( szText, pFuncName );
//			/* �֐��錾�� */
//			if( 1 == pcFuncInfo->m_nInfo ){
//				strcat( szText, "(�錾)" );
//			}
//
//			/* �O���[�o���֐��̃A�C�e����o�^ */
//			tvis.hParent = htiGlobal;
//			tvis.hInsertAfter = TVI_LAST;
//			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
//			tvis.item.pszText = szText;
//			tvis.item.lParam = i;
//			htiItem = TreeView_InsertItem( hwndTree, &tvis );
//
//			/* �֐��錾�� */
//			if( 1 == pcFuncInfo->m_nInfo ){
//				/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
//				sprintf( szText, "%s(%d): %s(�錾)\r\n",
//					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
//					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
//					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* ���o���� */
//				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
//			}else{
//				/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
//				sprintf( szText, "%s(%d): %s\r\n",
//					m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
//					pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
//					pcFuncInfo->m_cmemFuncName.GetPtr( NULL ) 	/* ���o���� */
//				);
//				m_cmemClipText.Append( (const char *)szText, lstrlen( szText ) );	/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */
//			}
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


/* �c���[�R���g���[���̏������F�e�L�X�g �g�s�b�N�c���[ */
void CDlgFuncList::SetTreeTxt( HWND hwndDlg )
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
		pWork = (unsigned char*)pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
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
			pWork2 = (unsigned char*)pcFuncInfo2->m_cmemFuncName.GetPtr( NULL );
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



BOOL CDlgFuncList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	HWND		hwndList;
	int			nCxVScroll;
	int			nColWidthArr[3] = { 0, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST1 );
//	hwndTree = ::GetDlgItem( m_hWnd, IDC_TREE1 );
	::GetWindowRect( hwndList, &rc );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - ( nColWidthArr[1] + nColWidthArr[2] ) - nCxVScroll - 8;
	col.pszText = "�֐���";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[1];
	col.pszText = "�s *";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[2];
	col.pszText = " ";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col);

	/* ���N���X�����o */
	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgFuncList::OnBnClicked( int wID )
{
	HGLOBAL			hgClip;
	char*			pszClip;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�A�E�g���C����́v�̃w���v */
		//Apr. 5, 2001 JEPRO �C���R���ǉ� (Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���)
//		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 64 );
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE) );
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
		/* Windows�N���b�v�{�[�h�ɃR�s�[ */
		hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, m_cmemClipText.GetLength() + 1 );
		pszClip = (char*)::GlobalLock( hgClip );
		memcpy( pszClip, m_cmemClipText.GetPtr( NULL ), m_cmemClipText.GetLength() + 1 );
		::GlobalUnlock( hgClip );
		::OpenClipboard( m_hWnd );
		::EmptyClipboard();
		::SetClipboardData( CF_OEMTEXT, hgClip );
		::CloseClipboard();
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFuncList::OnNotify( WPARAM wParam, LPARAM lParam )
{
	int				idCtrl;
	LPNMHDR			pnmh;
	NM_LISTVIEW*	pnlv;
	HWND			hwndList;
	HWND			hwndTree;
	NM_TREEVIEW*	pnmtv;
//	int				nLineTo;
	LV_COLUMN		col;

	idCtrl = (int) wParam;
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
				return OnJump();
//			case NM_CLICK:
//			case NM_KILLFOCUS:
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
			if( m_nSortCol == 0 ){
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
				ListView_SetColumn( hwndList, 0, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "�s";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 1, &col );
			// From Here 2001.12.07 hor
				col.mask = LVCF_TEXT;
				col.pszText = "";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 2, &col );
			// To Here 2001.12.07 hor
				ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
			}else
			if( m_nSortCol == 1 ){
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
				ListView_SetColumn( hwndList, 0, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "�s *";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 1, &col );
			// From Here 2001.12.07 hor
				col.mask = LVCF_TEXT;
				col.pszText = "";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 2, &col );
			// To Here 2001.12.03 hor
				ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
			// From Here 2001.12.07 hor
			}else
			if( m_nSortCol == 2 ){
				col.mask = LVCF_TEXT;
				if(OUTLINE_BOOKMARK == m_nListType){
					col.pszText = "�e�L�X�g";
				}else{
					col.pszText = "�֐���";
				}
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 0, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "�s";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 1, &col );
				col.mask = LVCF_TEXT;
				col.pszText = "*";
				col.iSubItem = 0;
				ListView_SetColumn( hwndList, 2, &col );
				ListView_SortItems( hwndList, _CompareFunc_, (LPARAM)this );
			// To Here 2001.12.07 hor
			}
			return TRUE;
		case NM_DBLCLK:
			return OnJump();
		}
	}
	return FALSE;
}



BOOL CDlgFuncList::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* ���N���X�����o */
	CDialog::OnSize( wParam, lParam );

	int	Controls[] = {
		IDC_CHECK_bAutoCloseDlgFuncList,
		IDC_BUTTON_COPY,
		IDOK,
		IDCANCEL,
		IDC_BUTTON_HELP,
		IDC_LIST1,
		IDC_TREE1
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

	fwSizeType = wParam;		// resizing flag
	nWidth = LOWORD(lParam);	// width of client area
	nHeight = HIWORD(lParam);	// height of client area

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
		if( Controls[i] == IDC_CHECK_bAutoCloseDlgFuncList ){
			::SetWindowPos( hwndCtrl, NULL, rc.left, nHeight - nWork + 32 /*- nWork*//*rc.top + nExtraSize*/, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}else
		if( Controls[i] != IDC_LIST1
		 && Controls[i] != IDC_TREE1
		){
			::SetWindowPos( hwndCtrl, NULL, rc.left, nHeight - nWork + 6/*rc.top + nExtraSize*/, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
//			::InvalidateRect( hwndCtrl, NULL, TRUE );
		}else{
			::SetWindowPos( hwndCtrl, NULL, 0, 0, nWidth - 2 * rc.left, nHeight - nWork + 5/*rc.bottom - rc.top + nExtraSize*/, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}
		::InvalidateRect( hwndCtrl, NULL, TRUE );
	}
	return TRUE;
}




BOOL CDlgFuncList::OnJump( void )
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
			::SendMessage( ::GetParent( ::GetParent( m_hwndParent ) ), MYWM_SETCARETPOS, 0, 0 );
			/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
			if( m_pShareData->m_Common.m_bAutoCloseDlgFuncList ){
				::DestroyWindow( m_hWnd );
			}
		}
	}
	return TRUE;
}


// From Here 2001.12.03 hor
/* �u�b�N�}�[�N���X�g�̍쐬	*/
void CDlgFuncList::SetTreeBookMark( HWND hwndDlg )
{
	int				i;
	char			szText[2048];
	CFuncInfo*		pcFuncInfo;
	LV_ITEM			item;
	LV_COLUMN		col;
	HWND			hwndList;
	HWND			hwndTree;
	int				bSelected;
	int				nFuncLineOld;
	int				nSelectedLine;
	RECT			rc;
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST1 );
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

		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = pcFuncInfo->m_cmemFuncName.GetPtr( NULL );
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam	= i;
		ListView_InsertItem( hwndList, &item);

		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if(m_bLineNumIsCRLF ){
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineCRLF );
		}else{
			wsprintf( szText, "%d", pcFuncInfo->m_nFuncLineLAYOUT );
		}
		item.mask = LVIF_TEXT;
		item.pszText = szText;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem( hwndList, &item);

		/* �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW */
		wsprintf( szText, "%s(%d): %s\r\n",
			m_pcFuncInfoArr->m_szFilePath,				/* ��͑Ώۃt�@�C���� */
			pcFuncInfo->m_nFuncLineCRLF,				/* ���o�s�ԍ� */
			pcFuncInfo->m_cmemFuncName.GetPtr( NULL )	/* ���o���� */
		);
		m_cmemClipText.AppendSz( (const char *)szText );					/* �N���b�v�{�[�h�R�s�[�p�e�L�X�g */

	}

	if( bSelected ){
		ListView_GetItemRect( hwndList, 0, &rc, LVIR_BOUNDS );
		ListView_Scroll( hwndList, 0, nSelectedLine * ( rc.bottom - rc.top ) );
		ListView_SetItemState( hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}
	/* �񖼕ύX */
	col.mask = LVCF_TEXT;
	col.pszText = "�e�L�X�g";
	col.iSubItem = 0;
	ListView_SetColumn( hwndList, 0, &col );

	/* ��̕����f�[�^�ɍ��킹�Ē��� */
	ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE );
	ListView_SetColumnWidth( hwndList, 2, 0 );

	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgFuncList, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );
	return;
}
// To Here 2001.12.03 hor


/*[EOF]*/
