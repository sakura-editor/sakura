//	$Id$
/*!	@file
	@brief DIFF�����\���_�C�A���O�{�b�N�X

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include <string.h>
#include "sakura_rc.h"
#include "CDlgOpenFile.h"
#include "CDlgDiff.h"
#include "etc_uty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"

#include "sakura.hh"
const DWORD p_helpids[] = {	//13200
	IDC_BUTTON_DIFF_DST,		HIDC_BUTTON_DIFF_DST,
	IDC_CHECK_DIFF_OPT_BLINE,	HIDC_CHECK_DIFF_OPT_BLINE,
	IDC_CHECK_DIFF_OPT_CASE,	HIDC_CHECK_DIFF_OPT_CASE,
	IDC_CHECK_DIFF_OPT_SPACE,	HIDC_CHECK_DIFF_OPT_SPACE,
	IDC_CHECK_DIFF_OPT_SPCCHG,	HIDC_CHECK_DIFF_OPT_SPCCHG,
	IDC_CHECK_DIFF_OPT_TABSPC,	HIDC_CHECK_DIFF_OPT_TABSPC,
	IDC_EDIT_DIFF_DST,			HIDC_EDIT_DIFF_DST,
	IDC_FRAME_DIFF_FILE12,		HIDC_RADIO_DIFF_FILE1,
	IDC_RADIO_DIFF_FILE1,		HIDC_RADIO_DIFF_FILE1,
	IDC_RADIO_DIFF_FILE2,		HIDC_RADIO_DIFF_FILE2,
	IDC_FRAME_DIFF_DST,			HIDC_RADIO_DIFF_DST1,
	IDC_RADIO_DIFF_DST1,		HIDC_RADIO_DIFF_DST1,
	IDC_RADIO_DIFF_DST2,		HIDC_RADIO_DIFF_DST2,
	IDC_LIST_DIFF_FILES,		HIDC_LIST_DIFF_FILES,
	IDC_STATIC_DIFF_SRC,		HIDC_STATIC_DIFF_SRC,
	IDOK,						HIDC_DIFF_IDOK,
	IDCANCEL,					HIDC_DIFF_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_DIFF_HELP,
	IDC_CHECK_DIFF_EXEC_STATE,	HIDC_CHECK_DIFF_EXEC_STATE,	//DIFF������������Ȃ��Ƃ��Ƀ��b�Z�[�W��\��  2003.05.12 MIK
//	IDC_FRAME_DIFF_SEARCH_MSG,	HIDC_FRAME_DIFF_SEARCH_MSG,
//	IDC_STATIC,						-1,
	0, 0
};

CDlgDiff::CDlgDiff()
{
	strcpy( m_szFile1, "" );
	strcpy( m_szFile2, "" );
	//m_nDiffFlgFile12 = 1;
	m_nDiffFlgOpt    = 0;
	m_bIsModified    = FALSE;
	m_bIsModifiedDst = FALSE;
	m_hWnd_Dst       = NULL;
	return;
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgDiff::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam,
	const char*	pszPath,		//���t�@�C��
	BOOL		bIsModified		//���t�@�C���ҏW���H
)
{
	strcpy( m_szFile1, pszPath );
	m_bIsModified = bIsModified;

	return CDialog::DoModal( hInstance, hwndParent, IDD_DIFF, lParam );
}

BOOL CDlgDiff::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DIFF_DIALOG) );
		return TRUE;

	case IDC_BUTTON_DIFF_DST:	/* �Q�� */
		{
			CDlgOpenFile	cDlgOpenFile;
			char*			pszMRU = NULL;;
			char*			pszOPENFOLDER = NULL;;
			char			szPath[_MAX_PATH];
			strcpy( szPath, m_szFile2 );
			/* �t�@�C���I�[�v���_�C�A���O�̏����� */
			cDlgOpenFile.Create(
				m_hInstance,
				m_hWnd,
				"*.*",
				m_szFile1 /*m_szFile2*/,
				(const char **)&pszMRU,
				(const char **)&pszOPENFOLDER
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) )
			{
				strcpy( m_szFile2, szPath );
				::SetDlgItemText( m_hWnd, IDC_EDIT_DIFF_DST, m_szFile2 );
				//�O���t�@�C����I����Ԃ�
				::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
				::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
				::SendMessage( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
			}
		}
		return TRUE;

	case IDOK:			/* ���E�ɕ\�� */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;

	case IDC_RADIO_DIFF_DST1:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST ), TRUE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_DIFF_DST ), TRUE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), FALSE );
		::SendMessage( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES), LB_SETCURSEL, (WPARAM)-1, 0 );
		return TRUE;

	case IDC_RADIO_DIFF_DST2:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_DIFF_DST ), FALSE );
		//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), TRUE );
		return TRUE;

	case IDC_RADIO_DIFF_FILE1:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE2, FALSE );
		return TRUE;

	case IDC_RADIO_DIFF_FILE2:
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE1, FALSE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgDiff::SetData( void )
{
	//���t�@�C��
	::SetDlgItemText( m_hWnd, IDC_STATIC_DIFF_SRC, m_szFile1 );

	//�I�v�V����
	m_nDiffFlgOpt = m_pShareData->m_nDiffFlgOpt;
	if( m_nDiffFlgOpt & 0x0001 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_CASE,   TRUE );
	if( m_nDiffFlgOpt & 0x0002 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_SPACE,  TRUE );
	if( m_nDiffFlgOpt & 0x0004 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_SPCCHG, TRUE );
	if( m_nDiffFlgOpt & 0x0008 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_BLINE,  TRUE );
	if( m_nDiffFlgOpt & 0x0010 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_OPT_TABSPC, TRUE );

	//�V���t�@�C��
	if( m_nDiffFlgOpt & 0x0020 )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE1, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE2, TRUE );
	}
	else
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE1, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_FILE2, FALSE );
	}
	//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_FRAME_DIFF_FILE12 ), FALSE );
	//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_FILE1 ), FALSE );
	//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_FILE2 ), FALSE );

	//DIFF������������Ȃ��Ƃ��Ƀ��b�Z�[�W��\�� 2003.05.12 MIK
	if( m_nDiffFlgOpt & 0x0040 ) ::CheckDlgButton( m_hWnd, IDC_CHECK_DIFF_EXEC_STATE, TRUE );

	/* ����t�@�C���̑I�� */
	::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
	::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );

	/* ������Ȃ��Ƃ����b�Z�[�W��\�� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_pShareData->m_Common.m_bNOTIFYNOTFOUND );
	
	/* �擪�i�����j����Č��� */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	/* �ҏW���̃t�@�C���ꗗ���쐬���� */
	{
		HWND		hwndList;
		int			nRowNum;
		EditNode	*pEditNode;
		FileInfo	*pFileInfo;
		int			i;
		int			nItem;
		char		szName[_MAX_PATH];
		CEditDoc*	pCEditDoc = (CEditDoc*)m_lParam;
		int			count = 0;

		/* ���X�g�̃n���h���擾 */
		hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );

		/* ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
		nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNode, TRUE );
		if( nRowNum > 0 )
		{
			for( i = 0; i < nRowNum; i++ )
			{
				/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
				::SendMessage( pEditNode[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
				pFileInfo = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

				/* �����Ȃ�X�L�b�v */
				if ( pEditNode[i].m_hWnd == pCEditDoc->m_hwndParent )
				{
					continue;
				}

				/* �t�@�C�������쐬���� */
				wsprintf( szName, "%s %s",
					( strlen( pFileInfo->m_szPath ) ) ? pFileInfo->m_szPath : "(����)",
					pFileInfo->m_bIsModified ? "*" : " "
				);

				// gm_pszCodeNameArr_3 ����R�s�[����悤�ɕύX
				if( 0 < pFileInfo->m_nCharCode && pFileInfo->m_nCharCode < CODE_CODEMAX ){
					strcat( szName, gm_pszCodeNameArr_3[pFileInfo->m_nCharCode] );
				}
#if 0
				/* �����R�[�h��t�^���� */
				switch( pFileInfo->m_nCharCode )
				{
				case CODE_JIS:	/* JIS */
					strcat( szName, "  [JIS]" );
					break;
				case CODE_EUC:	/* EUC */
					strcat( szName, "  [EUC]" );
					break;
				case CODE_UNICODE:	/* Unicode */
					strcat( szName, "  [Unicode]" );
					break;
				case CODE_UTF8:	/* UTF-8 */
					strcat( szName, "  [UTF-8]" );
					break;
				case CODE_UTF7:	/* UTF-7 */
					strcat( szName, "  [UTF-7]" );
					break;
				default:	/* SJIS */
					break;
				}
#endif

				/* ���X�g�ɓo�^���� */
				nItem = ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)(char*)szName );
				::SendMessage( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNode[i].m_hWnd );
				count++;
			}

			delete [] pEditNode;
			// 2002/11/01 Moca �ǉ� ���X�g�r���[�̉�����ݒ�B��������Ȃ��Ɛ����X�N���[���o�[���g���Ȃ�
			::SendMessage( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)1000, 0 );

			/* �ŏ���I�� */
			//::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
		}

		if( count == 0 )
		{
			/* ���̑��̕ҏW�����X�g�͂Ȃ� */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_DIFF_DST2 ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES ), FALSE );
		}
	}

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgDiff::GetData( void )
{
	BOOL	ret = TRUE;

	//DIFF�I�v�V����
	m_nDiffFlgOpt = 0;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_CASE   ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0001;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_SPACE  ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0002;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_SPCCHG ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0004;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_BLINE  ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0008;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_OPT_TABSPC ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0010;
	//�t�@�C���V��
	if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_FILE2      ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0020;
	//DIFF������������Ȃ��Ƃ��Ƀ��b�Z�[�W��\�� 2003.05.12 MIK
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_DIFF_EXEC_STATE ) == BST_CHECKED ) m_nDiffFlgOpt |= 0x0040;
	m_pShareData->m_nDiffFlgOpt = m_nDiffFlgOpt;

	//����t�@�C����
	strcpy( m_szFile2, "" );
	m_hWnd_Dst = NULL;
	m_bIsModifiedDst = FALSE;
	if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_DST1 ) == BST_CHECKED )
	{
		::GetDlgItemText( m_hWnd, IDC_EDIT_DIFF_DST, m_szFile2, sizeof( m_szFile2 ) );
	}
	else if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_DIFF_DST2 ) == BST_CHECKED )
	{
		HWND		hwndList;
		int			nItem;
		FileInfo	*pFileInfo;

		/* ���X�g���瑊��̃E�C���h�E�n���h�����擾 */
		hwndList = :: GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );
		nItem = ::SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
		if( nItem != LB_ERR )
		{
			m_hWnd_Dst = (HWND)::SendMessage( hwndList, LB_GETITEMDATA, nItem, 0 );

			/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
			::SendMessage( m_hWnd_Dst, MYWM_GETFILEINFO, 0, 0 );
			pFileInfo = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

			strcpy( m_szFile2, pFileInfo->m_szPath );
			m_bIsModifiedDst = pFileInfo->m_bIsModified;
		}
		else
		{
			ret = FALSE;
		}
	}
	else
	{
		ret = FALSE;
	}

	/* ������Ȃ��Ƃ����b�Z�[�W��\�� */
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND ) == BST_CHECKED )
		m_pShareData->m_Common.m_bNOTIFYNOTFOUND = TRUE;
	else
		m_pShareData->m_Common.m_bNOTIFYNOTFOUND = FALSE;

	/* �擪�i�����j����Č��� */
	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL ) == BST_CHECKED )
		m_pShareData->m_Common.m_bSearchAll = TRUE;
	else
		m_pShareData->m_Common.m_bSearchAll = FALSE;

	//����t�@�C�����w�肳��ĂȂ���΃L�����Z��
	if( m_szFile2[0] == '\0' ) ret = FALSE;

	return ret;
}

BOOL CDlgDiff::OnCbnSelChange( HWND hwndCtl, int wID )
{
	HWND	hwndList;

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_DIFF_FILES );

	if( hwndList == hwndCtl )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, TRUE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

BOOL CDlgDiff::OnEditChange( HWND hwndCtl, int wID )
{
	HWND	hwndEdit;

	hwndEdit = GetDlgItem( m_hWnd, IDC_EDIT_DIFF_DST );

	if( hwndEdit == hwndCtl )
	{
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST1, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_DIFF_DST2, FALSE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

LPVOID CDlgDiff::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*[EOF]*/
