/*!	@file
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
//#include <windows.h>
//#include <commctrl.h>
#include "sakura_rc.h"
#include "CDlgTypeList.h"
#include "debug/Debug.h"
#include "func/Funccode.h"	//Stonee, 2001/03/12
#include "util/shell.h"
#include "sakura.hh"
#include "typeprop/CImpExpManager.h"	// 2010/4/24 Uchi

//�����g�p�萔
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// �^�C�v�ʐݒ�ꗗ CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12700
	IDC_BUTTON_TEMPCHANGE,	HIDC_TL_BUTTON_TEMPCHANGE,	//�ꎞ�K�p
	IDOK,					HIDOK_TL,					//�ݒ�
	IDCANCEL,				HIDCANCEL_TL,				//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_TL_BUTTON_HELP,		//�w���v
	IDC_LIST_TYPES,			HIDC_TL_LIST_TYPES,			//���X�g
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, SResult* psResult )
{
	int	nRet;
	m_nSettingType = psResult->cDocumentType;
	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, NULL );
	if( -1 == nRet ){
		return FALSE;
	}
	else{
		//����
		psResult->cDocumentType = CTypeConfig(nRet & ~PROP_TEMPCHANGE_FLAG);
		psResult->bTempChange   = ((nRet & PROP_TEMPCHANGE_FLAG) != 0);
		return TRUE;
	}
	return nRet;
}

BOOL CDlgTypeList::OnLbnDblclk( int wID )
{
	switch( wID ){
	case IDC_LIST_TYPES:
		//	Nov. 29, 2000	genta
		//	����ύX: �w��^�C�v�̐ݒ�_�C�A���O���ꎞ�I�ɕʂ̐ݒ��K�p
		::EndDialog(
			GetHwnd(),
			::SendMessageAny(
				GetDlgItem( GetHwnd(), IDC_LIST_TYPES ),
				LB_GETCURSEL,
				0,
				0
			)
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgTypeList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�^�C�v�ʐݒ�ꗗ�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TYPE_LIST) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//	Nov. 29, 2000	From Here	genta
	//	�K�p����^�̈ꎞ�I�ύX
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog(
			GetHwnd(),
			::SendMessageAny(
				GetDlgItem( GetHwnd(), IDC_LIST_TYPES ),
				LB_GETCURSEL,
				0,
				0
			)
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	//	Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog( GetHwnd(), ::SendMessageAny( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 ) );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), -1 );
		return TRUE;
	case IDC_BUTTON_IMPORT:
		Import();
		return TRUE;
	case IDC_BUTTON_EXPORT:
		Export();
		return TRUE;
	case IDC_BUTTON_INITIALIZE:
		InitializeType();
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );

}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTypeList::SetData( void )
{
	int		nIdx;
	HWND	hwndList;
	TCHAR	szText[130];
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	::SendMessageAny( hwndList, LB_RESETCONTENT, 0, 0 );	/* ���X�g����ɂ��� */
	for( nIdx = 0; nIdx < MAX_TYPES; ++nIdx ){
		STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
		if( 0 < _tcslen( types.m_szTypeExts ) ){		/* �^�C�v�����F�g���q���X�g */
			auto_sprintf( szText, _T("%ts ( %ts )"),
				types.m_szTypeName,	/* �^�C�v�����F���� */
				types.m_szTypeExts	/* �^�C�v�����F�g���q���X�g */
			);
		}else{
			auto_sprintf( szText, _T("%ts"),
				types.m_szTypeName	/* �^�C�v�����F�g�� */
			);
		}
		::List_AddString( hwndList, szText );
	}
	::SendMessageAny( hwndList, LB_SETCURSEL, (WPARAM)m_nSettingType.GetIndex(), (LPARAM)0 );
	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgTypeList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end



// �^�C�v�ʐݒ�C���|�[�g
//		2010/4/12 Uchi
bool CDlgTypeList::Import()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));

	CImpExpType	cImpExpType( nIdx, types, hwndList );

	// �C���|�[�g
	cImpExpType.SetBaseName( to_wchar( types.m_szTypeName ) );
	if (!cImpExpType.ImportUI( G_AppInstance(), GetHwnd() )) {
		// �C���|�[�g�����Ă��Ȃ�
		return false;
	}

	// ���X�g�ď�����
	SetData();
	::SendMessageAny( hwndList, LB_SETCURSEL, (WPARAM)nIdx, 0 );

	return true;
}

// �^�C�v�ʐݒ�G�N�X�|�[�g
//		2010/4/12 Uchi
bool CDlgTypeList::Export()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));

	CImpExpType	cImpExpType( nIdx, types, hwndList );

	// �G�N�X�|�[�g
	cImpExpType.SetBaseName( to_wchar( types.m_szTypeName) );
	if (!cImpExpType.ExportUI( G_AppInstance(), GetHwnd() )) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return false;
	}

	return true;
}

//void _DefaultConfig(STypeConfig* pType);

// �^�C�v�ʐݒ菉����
//		2010/4/12 Uchi
bool CDlgTypeList::InitializeType( void )
{
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int iDocType = ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 );
	if (iDocType == 0) {
		// ��{�̏ꍇ�ɂ͉������Ȃ�
		return true;
	}
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(iDocType));
	int			nRet;
	if (0 < _tcslen( types.m_szTypeExts )) { 
		nRet = ::MYMESSAGEBOX(
			GetHwnd(),
			MB_YESNO | MB_ICONQUESTION,
			GSTR_APPNAME,
			_T("%ts �����������܂��B ��낵���ł����H"),
			types.m_szTypeName );
		if (nRet != IDYES) {
			return false;
		}
	}

//	_DefaultConfig(&types);		//�K��l���R�s�[
	types = CDocTypeManager().GetTypeSetting(CTypeConfig(0));	// ��{���R�s�[

	types.m_nIdx = iDocType;
	auto_sprintf( types.m_szTypeName, _T("�ݒ�%d"), iDocType+1 );
	_tcscpy( types.m_szTypeExts, _T("") );

	// ���X�g�ď�����
	SetData();
	::SendMessageAny( hwndList, LB_SETCURSEL, (WPARAM)iDocType, 0 );

	InfoMessage( hwndDlg, _T("%ts �����������܂����B"), types.m_szTypeName );

	return true;
}
