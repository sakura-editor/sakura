//	$Id$
/*!	@file
	@brief �t�@�C����r�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "sakura_rc.h"
#include "CDlgCompare.h"
#include "etc_uty.h"
#include "debug.h"
#include "CEditDoc.h"
//#include "CEditView.h"	// Oct. 10, 2000 JEPRO added	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕K�v�H
#include "global.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "mymessage.h"
//#include <locale.h>	// 2002/2/10 aroka

// �t�@�C�����e��r CDlgCompare.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12300
	IDC_BUTTON1,					HIDC_CMP_BUTTON1,			//�㉺�ɕ\��
	IDOK2,							HIDOK2_CMP,					//���E�ɕ\��
	IDOK,							HIDOK_CMP,					//OK
	IDCANCEL,						HIDCANCEL_CMP,				//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		//�w���v
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		//���E�ɕ\��
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		//�t�@�C���ꗗ
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	//�\�[�X�t�@�C��
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgCompare::CDlgCompare()
{
	m_bCompareAndTileHorz = TRUE;	/* ���E�ɕ��ׂĕ\�� */
//	m_bCompareAndTileHorz = TRUE;	/* ���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
	return;
}


/* ���[�_���_�C�A���O�̕\�� */
int CDlgCompare::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam,
	const char*	pszPath,
	BOOL		bIsModified,
	char*		pszComparePath,
	HWND*		phwndCompareWnd
)
{
	m_pszPath = pszPath;
	m_bIsModified = bIsModified;
	m_pszComparePath = pszComparePath;
	m_phwndCompareWnd = phwndCompareWnd;
	return CDialog::DoModal( hInstance, hwndParent, IDD_COMPARE, lParam );
}

BOOL CDlgCompare::OnBnClicked( int wID )
{
//	CEditView*	pcEditView = (CEditView*)m_lParam;	//	Oct. 10, 2000 JEPRO added	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕K�v�H
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u���e��r�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE) );
		return TRUE;
//	From Here Oct. 10, 2000 JEPRO added  Ref. code ��CDlgFind.cpp �� OnBnClicked
//	�`�F�b�N�{�b�N�X���{�^��������CDlgCompare.cpp�ɒ��ڏ�������ł݂������s
//	�_�C�A���O�̃{�^���͉��ɕs�������Ă����Ă���܂��B
//	�ȉ��̒ǉ��R�[�h�͑S�������Č��\�ł�����N������Ă��������B�����X�N���[��������Ă����ƂȂ����ꂵ���ł��B
//	case IDC_BUTTON1:	/* �㉺�ɕ\�� */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		return TRUE;
//	case IDOK:			/* ���E�ɕ\�� */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		HWND	hwndCompareWnd;
//		HWND*	phwndArr;
//		int		i;
//		phwndArr = new HWND[2];
//		phwndArr[0] = ::GetParent( m_hwndParent );
//		phwndArr[1] = hwndCompareWnd;
//		for( i = 0; i < 2; ++i ){
//			if( ::IsZoomed( phwndArr[i] ) ){
//				::ShowWindow( phwndArr[i], SW_RESTORE );
//			}
//		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
//		delete [] phwndArr;
//		CloseDialog( 0 );
//		return TRUE;
//	To Here Oct. 10, 2000
	case IDOK:			/* ���E�ɕ\�� */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgCompare::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	HWND			hwndList;
	int				nRowNum;
	EditNode*		pEditNodeArr;
	FileInfo*		pfi;
	int				i;
	char			szMenu[512];
	int				nItem;

	hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_FILES );

//	2002/2/10 aroka �t�@�C�����Ŕ�r���Ȃ����ߕs�p (2001.12.26 YAZAKI����)
//	//	Oct. 15, 2001 genta �t�@�C��������� stricmp��bcc�ł����Ғʂ蓮��������
//	setlocale ( LC_ALL, "C" );

	/* ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
	nRowNum = m_cShareData.GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
		for( i = 0; i < nRowNum; ++i ){
			/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
			::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
			pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

//@@@ 2001.12.26 YAZAKI �t�@�C�����Ŕ�r�����(����)�������Ƃ��ɖ�蓯�m�̔�r���ł��Ȃ�
//			if( 0 == stricmp( pfi->m_szPath, m_pszPath ) ){
			if (pEditNodeArr[i].m_hWnd == pCEditDoc->m_hwndParent){
				continue;
			}
			wsprintf( szMenu, "%s %s",
				(0 < lstrlen(pfi->m_szPath))?pfi->m_szPath:"(����)",
				pfi->m_bIsModified ? "*":" "
			);
			if( 0 != pfi->m_nCharCode ){		/* �����R�[�h��� */
				switch( pfi->m_nCharCode ){
				case CODE_JIS:	/* JIS */
					strcat( szMenu, "  [JIS]" );
					break;
				case CODE_EUC:	/* EUC */
					strcat( szMenu, "  [EUC]" );
					break;
				case CODE_UNICODE:	/* Unicode */
					strcat( szMenu, "  [Unicode]" );
					break;
				case CODE_UTF8:	/* UTF-8 */
					strcat( szMenu, "  [UTF-8]" );
					break;
				case CODE_UTF7:	/* UTF-7 */
					strcat( szMenu, "  [UTF-7]" );
					break;
				}
			}
			nItem = ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)(char*)szMenu );
			::SendMessage( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNodeArr[i].m_hWnd );
		}
		delete [] pEditNodeArr;
	}
	::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
	char	szWork[512];
	wsprintf( szWork, "%s %s",
		(0 < lstrlen( m_pszPath )?m_pszPath:"(����)" ),
		m_bIsModified?"*":""
	);
	::SetDlgItemText( m_hWnd, IDC_STATIC_COMPARESRC, szWork );
	/* ���E�ɕ��ׂĕ\�� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_TILE_H, m_bCompareAndTileHorz );
//	::CheckDlgButton( m_hWnd, IDC_CHECK_TILE_H, m_bCompareAndTileHorz );	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgCompare::GetData( void )
{
	HWND			hwndList;
	int				nItem;
//	HWND			hwndCompareFile;
	FileInfo*		pfi;
	hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_FILES );
	nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
	*m_phwndCompareWnd = (HWND)::SendMessage( hwndList, LB_GETITEMDATA, nItem, 0 );
	/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
	::SendMessage( *m_phwndCompareWnd, MYWM_GETFILEINFO, 0, 0 );
//	pfi = (FileInfo*)m_pShareData->m_szWork;
	pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

	strcpy( m_pszComparePath, pfi->m_szPath );

	/* ���E�ɕ��ׂĕ\�� */
	m_bCompareAndTileHorz = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_TILE_H );
//	m_bCompareAndTileHorz = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_TILE_H );	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
