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
	Copyright (C) 2010, Uchi, Beta.Ito, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "types/CType.h" // use CDlgTypeList��`

#include "typeprop/CDlgTypeList.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/24 Uchi
#include "env/CDocTypeManager.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "util/RegKey.h"

typedef std::basic_string<TCHAR> tstring;

#define BUFFER_SIZE 1024
#define ACTION_NAME	(_T("SakuraEditor"))
#define PROGID_BACKUP_NAME	(_T("SakuraEditorBackup"))
#define ACTION_BACKUP_PATH	(_T("\\ShellBackup"))

//�֐��v���g�^�C�v
int CopyRegistry(HKEY srcRoot, const tstring srcPath, HKEY destRoot, const tstring destPath);
int RegistExt(LPCTSTR sExt, bool bDefProg);
int UnregistExt(LPCTSTR sExt);
int CheckExt(LPCTSTR sExt, bool *pbRMenu, bool *pbDblClick);

//�����g�p�萔
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// �^�C�v�ʐݒ�ꗗ CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12700
	IDC_BUTTON_TEMPCHANGE,	HIDC_TL_BUTTON_TEMPCHANGE,	//�ꎞ�K�p
	IDOK,					HIDOK_TL,					//�ݒ�
	IDCANCEL,				HIDCANCEL_TL,				//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_TL_BUTTON_HELP,		//�w���v
	IDC_LIST_TYPES,			HIDC_TL_LIST_TYPES,			//���X�g
	IDC_BUTTON_IMPORT,		HIDC_TL_BUTTON_IMPORT,		//�C���|�[�g
	IDC_BUTTON_EXPORT,		HIDC_TL_BUTTON_EXPORT,		//�G�N�X�|�[�g
	IDC_BUTTON_INITIALIZE,	HIDC_TL_BUTTON_INIT,		//������
	IDC_CHECK_EXT_RMENU,	HIDC_TL_CHECK_RMENU,		//�E�N���b�N���j���[�ɒǉ�
	IDC_CHECK_EXT_DBLCLICK,	HIDC_TL_CHECK_DBLCLICK,		//�_�u���N���b�N�ŊJ��
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, SResult* psResult )
{
	int	nRet;
	m_nSettingType = psResult->cDocumentType;
	m_bAlertFileAssociation = true;
	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, (LPARAM)NULL );
	if( -1 == nRet ){
		return FALSE;
	}
	else{
		//����
		psResult->cDocumentType = CTypeConfig(nRet & ~PROP_TEMPCHANGE_FLAG);
		psResult->bTempChange   = ((nRet & PROP_TEMPCHANGE_FLAG) != 0);
		return TRUE;
	}
}


BOOL CDlgTypeList::OnLbnDblclk( int wID )
{
	switch( wID ){
	case IDC_LIST_TYPES:
		//	Nov. 29, 2000	genta
		//	����ύX: �w��^�C�v�̐ݒ�_�C�A���O���ꎞ�I�ɕʂ̐ݒ��K�p
		::EndDialog(
			GetHwnd(),
			List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) )
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
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TYPE_LIST) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//	Nov. 29, 2000	From Here	genta
	//	�K�p����^�̈ꎞ�I�ύX
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog(
			GetHwnd(),
 			List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) )
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	//	Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog( GetHwnd(), List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) ) );
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


INT_PTR CDlgTypeList::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*	pNMHDR;
	HWND	hwndList;

	pNMHDR = (NMHDR*)lParam;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
	HWND hwndRMenu = GetDlgItem( GetHwnd(), IDC_CHECK_EXT_RMENU );
	HWND hwndDblClick = GetDlgItem( GetHwnd(), IDC_CHECK_EXT_DBLCLICK );

	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_COMMAND:
		if( LOWORD(wParam) == IDC_LIST_TYPES )
		{
			switch( HIWORD(wParam) )
			{
			case LBN_SELCHANGE:
				if( types.m_szTypeExts[0] == '\0' ){
					::EnableWindow( hwndRMenu, FALSE );
					::EnableWindow( hwndDblClick, FALSE );
				}else{
					::EnableWindow( GetDlgItem( GetHwnd(), IDC_CHECK_EXT_RMENU ), TRUE );
					if( !m_bRegistryChecked[ nIdx ] ){
						TCHAR exts[_countof(types.m_szTypeExts)] = {0};
						_tcscpy( exts, types.m_szTypeExts );
						const TCHAR	pszSeps[] = _T(" ;,");	// separator
						TCHAR *ext = _tcstok( exts, pszSeps );

						m_bExtRMenu[ nIdx ] = true;
						m_bExtDblClick[ nIdx ] = true;
						while( NULL != ext ){
							bool bRMenu;
							bool bDblClick;
							CheckExt( ext, &bRMenu, &bDblClick );
							m_bExtRMenu[ nIdx ] &= bRMenu;
							m_bExtDblClick[ nIdx ] &= bDblClick;
							ext = _tcstok( NULL, pszSeps );
						}
						m_bRegistryChecked[ nIdx ] = true;
					}
					BtnCtl_SetCheck( hwndRMenu, m_bExtRMenu[ nIdx ] );
					::EnableWindow( hwndDblClick, m_bExtRMenu[ nIdx ] );
					BtnCtl_SetCheck( hwndDblClick, m_bExtDblClick[ nIdx ] );
				}
				return TRUE;
			}
		}
		else if( LOWORD(wParam) == IDC_CHECK_EXT_RMENU && HIWORD(wParam) == BN_CLICKED )
		{
			bool checked = ( BtnCtl_GetCheck( hwndRMenu ) == TRUE ? true : false );
			if( ! AlertFileAssociation() ){		//���W�X�g���ύX�m�F
				BtnCtl_SetCheck( hwndRMenu, !checked );
				break;
			}
			TCHAR exts[_countof(types.m_szTypeExts)] = {0};
			_tcscpy( exts, types.m_szTypeExts );
			const TCHAR	pszSeps[] = _T(" ;,");	// separator
			TCHAR *ext = _tcstok( exts, pszSeps );
			int nRet;
			while( NULL != ext ){
				if( checked ){	//�u�E�N���b�N�v�`�F�b�NON
					if( (nRet = RegistExt( ext, true )) != 0 )
					{
						TCHAR buf[BUFFER_SIZE] = {0};
						::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL ); 
						::MessageBox( GetHwnd(), (tstring(_T("�֘A�t���Ɏ��s���܂���\n")) + buf).c_str(), GSTR_APPNAME, MB_OK );
						break;
					}
				}else{			//�u�E�N���b�N�v�`�F�b�NOFF
					if( (nRet = UnregistExt( ext )) != 0 )
					{
						TCHAR buf[BUFFER_SIZE] = {0};
						::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL ); 
						::MessageBox( GetHwnd(), (tstring(_T("�֘A�t�������Ɏ��s���܂���\n")) + buf).c_str(), GSTR_APPNAME, MB_OK );
						break;
					}
				}
				m_bExtRMenu[ nIdx ] = checked;
				::EnableWindow( hwndDblClick, checked );
				m_bExtDblClick[ nIdx ] = checked;
				BtnCtl_SetCheck( hwndDblClick, checked );
				ext = _tcstok( NULL, pszSeps );
			}
			return TRUE;
		}
		else if( LOWORD(wParam) == IDC_CHECK_EXT_DBLCLICK && HIWORD(wParam) == BN_CLICKED )
		{
			bool checked = ( BtnCtl_GetCheck( hwndDblClick ) == TRUE ? true : false );
			if( ! AlertFileAssociation() ){		//���W�X�g���ύX�m�F
				BtnCtl_SetCheck( hwndDblClick, !checked );
				break;
			}
			TCHAR exts[_countof(types.m_szTypeExts)] = {0};
			_tcscpy( exts, types.m_szTypeExts );
			const TCHAR	pszSeps[] = _T(" ;,");	// separator
			TCHAR *ext = _tcstok( exts, pszSeps );
			int nRet;
			while( NULL != ext ){
				if( (nRet = RegistExt( ext, checked )) != 0 )
				{
					TCHAR buf[BUFFER_SIZE] = {0};
					::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL ); 
					::MessageBox( GetHwnd(), (tstring(_T("�֘A�t���Ɏ��s���܂���\n")) + buf).c_str(), GSTR_APPNAME, MB_OK );
					break;
				}
				m_bExtDblClick[ nIdx ] = checked;
				ext = _tcstok( NULL, pszSeps );
			}
			return TRUE;
		}
	}
	return CDialog::OnNotify( wParam, lParam );
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTypeList::SetData( void )
{
	int		nIdx;
	HWND	hwndList;
	TCHAR	szText[64 + MAX_TYPES_EXTS + 10];
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	List_ResetContent( hwndList );	/* ���X�g����ɂ��� */
	for( nIdx = 0; nIdx < MAX_TYPES; ++nIdx ){
		STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
		if( types.m_szTypeExts[0] != _T('\0') ){		/* �^�C�v�����F�g���q���X�g */
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
		m_bRegistryChecked[ nIdx ] = FALSE;
		m_bExtRMenu[ nIdx ] = FALSE;
		m_bExtDblClick[ nIdx ] = FALSE;
	}
	List_SetCurSel( hwndList, m_nSettingType.GetIndex() );
	::SendMessageAny( GetHwnd(), WM_COMMAND, MAKEWPARAM(IDC_LIST_TYPES, LBN_SELCHANGE), 0 );
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
	int nIdx = List_GetCurSel( hwndList );
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
	List_SetCurSel( hwndList, nIdx );

	return true;
}

// �^�C�v�ʐݒ�G�N�X�|�[�g
//		2010/4/12 Uchi
bool CDlgTypeList::Export()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
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

// �^�C�v�ʐݒ菉����
//		2010/4/12 Uchi
bool CDlgTypeList::InitializeType( void )
{
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int iDocType = List_GetCurSel( hwndList );
	if (iDocType == 0) {
		// ��{�̏ꍇ�ɂ͉������Ȃ�
		return true;
	}
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(iDocType));
	int			nRet;
	if ( types.m_szTypeExts[0] != _T('\0') ) { 
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
	List_SetCurSel( hwndList, iDocType );

	InfoMessage( hwndDlg, _T("%ts �����������܂����B"), types.m_szTypeName );

	return true;
}

/*! �ċA�I���W�X�g���R�s�[ */
int CopyRegistry(HKEY srcRoot, const tstring srcPath, HKEY destRoot, const tstring destPath)
{
	int errorCode;
	CRegKey keySrc;
	if((errorCode = keySrc.Open(srcRoot, srcPath.c_str(), KEY_READ)) != 0){ return errorCode; }

	CRegKey keyDest;
	if((errorCode = keyDest.Open(destRoot, destPath.c_str(), KEY_READ | KEY_WRITE)) != 0)
	{
		if( (errorCode = keyDest.Create(destRoot, destPath.c_str())) != 0 ){ return errorCode; }
	}

	int index = 0;
	for (;;)
	{
		TCHAR szValue[ BUFFER_SIZE ] = {0};
		BYTE data[ BUFFER_SIZE ] = {0};
		DWORD dwDataLen;
		DWORD dwType;

		errorCode = keySrc.EnumValue(index, szValue, _countof(szValue), &dwType, data, _countof(data), &dwDataLen );
		if( errorCode == ERROR_NO_MORE_ITEMS ){
			errorCode = 0;
			break;
		}else if( errorCode ){
			return errorCode;
		}else{
			// �蔲���F�f�[�^�̃T�C�Y��BUFFER_SIZE(=1024)�𒴂���ꍇ���l�����Ă��Ȃ�
			if( (errorCode = keyDest.SetValue(szValue, data, dwDataLen, dwType)) != 0 ){ return errorCode; }
			index++;
		}
	}

	index = 0;
	TCHAR szSubKey[ BUFFER_SIZE ] = {0};
	for (;;)
	{
		errorCode = keySrc.EnumKey(index, szSubKey, _countof(szSubKey));
		if( errorCode == ERROR_NO_MORE_ITEMS ){
			errorCode = 0;
			break;
		}else if( errorCode ){
			return errorCode;
		}else{
			if( errorCode = CopyRegistry(srcRoot, srcPath + _T("\\") + szSubKey, destRoot, destPath + _T("\\") + szSubKey) ){ return errorCode; }
			index++;
		}
	}

	return errorCode;
}

/*! �ċA�I���W�X�g���폜 */
int DeleteRegistry(HKEY root, const tstring path)
{
	int errorCode;
	CRegKey keySrc;
	if((errorCode = keySrc.Open(root, path.c_str(), KEY_READ | KEY_WRITE)) != 0){ return ERROR_SUCCESS; }

	int index = 0;
	index = 0;
	TCHAR szSubKey[ BUFFER_SIZE ] = {0};
	for (;;)
	{
		errorCode = keySrc.EnumKey(index, szSubKey, _countof(szSubKey));
		if( errorCode == ERROR_NO_MORE_ITEMS ){
			errorCode = 0;
			break;
		}else if( errorCode ){
			return errorCode;
		}else{
			if( (errorCode = DeleteRegistry(root, path + _T("\\") + szSubKey)) != 0 ){ return errorCode; }
		}
	}
	keySrc.Close();
	if( (errorCode = CRegKey::DeleteKey(root, path.c_str())) != 0 ){ return errorCode; }

	return errorCode;
}

/*!
	@brief �g���q���Ƃ̊֘A�t�����W�X�g���ݒ���s��
	@param sExt	�g���q
	@param bDefProg [in]����t���O�i�_�u���N���b�N�ŋN�������邩�j
	���W�X�g���A�N�Z�X���j
	�E�Ǘ��Ҍ����Ȃ��Ŏ��{���������߁AHKLM�͓ǂݍ��݂݂̂Ƃ��A�������݂�HKCU�ɍs���B
	�����̗���
	�E[HKCU\Software\Classes\.(�g���q)]�̑��݃`�F�b�N
		���݂��Ȃ����
		�E[HKCU\Software\Classes\.(�g���q)]���쐬�B�l�́uSakuraEditor_(�g���q)�v
	�E[HKCU\Software\Classes\.(�g���q)]�̒l���uSakuraEditor_(�g���q)�v�ȊO�̏ꍇ�A
		[HKCU\Software\Classes\.(�g���q)\SakuraEditorBackup]�ɒl���R�s�[����
		�l�ɁuSakuraEditor_(�g���q)�v��ݒ肷��
	�EProgID <- [HKCR\Software\Classes\.(�g���q)]�̒l
	�E[HKCU\Software\Classes\(ProgID)]�̑��݃`�F�b�N
		���݂��Ȃ����
		�E[HKLM\Software\Classes\(HKLM��ProgID)]�̑��݃`�F�b�N
		���݂����
			�E[HKLM\Software\Classes\(ProgID)]�̍\����[HKCU\Software\Classes\(ProgID)]�ɃR�s�[����
	�E[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\command]���쐬�B�l�́u"(�T�N��EXE�p�X)" "%1"�v
	�E[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]�̒l���uSakura &Editor�v�Ƃ���
	�E����t���O����
		true�Ȃ�
			�E[HKCU\Software\Classes\(ProgID)\shell]�̒l����łȂ����[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]�ɑޔ�����
			�E[HKCU\Software\Classes\(ProgID)\shell]�̒l���uSakuraEditor�v�Ƃ���
		false�Ȃ�
			�E[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]�̑��݃`�F�b�N
				���݂���΁A�ޔ������l��[HKCU\Software\Classes\(ProgID)\shell]�ɐݒ�
				���݂��Ȃ���΁A[HKCU\Software\Classes\(ProgID)\shell]�̒l���폜
*/
int RegistExt(LPCTSTR sExt, bool bDefProg)
{
	int errorCode = ERROR_SUCCESS;
	tstring sBasePath = tstring( _T("Software\\Classes\\") );

	//��������
	TCHAR szLowerExt[MAX_PATH] = {0};
	_tcsncpy_s(szLowerExt, sizeof(szLowerExt) / sizeof(szLowerExt[0]), sExt, _tcslen(sExt));
	CharLower(szLowerExt);

	tstring sDotExt = sBasePath + _T(".") + szLowerExt;
	tstring sGenProgID = tstring() + _T("SakuraEditor_") + szLowerExt;

	CRegKey keyExt_HKLM;
	TCHAR szProgID_HKLM[ BUFFER_SIZE ] = {0};
	if( ( errorCode = keyExt_HKLM.Open(HKEY_LOCAL_MACHINE, sDotExt.c_str(), KEY_READ) ) == 0 )
	{
		keyExt_HKLM.GetValue(NULL, szProgID_HKLM, _countof(szProgID_HKLM));
	}

	CRegKey keyExt;
	if((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ | KEY_WRITE)) != 0)
	{
		if( (errorCode = keyExt.Create(HKEY_CURRENT_USER, sDotExt.c_str())) != 0 ){ return errorCode; }
	}

	TCHAR szProgID[ BUFFER_SIZE ] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if(_tcscmp( sGenProgID.c_str(), szProgID ) != 0) {
		if( szProgID[0] != _T('\0') )
		{
			if( (errorCode = keyExt.SetValue(PROGID_BACKUP_NAME, szProgID)) != 0 ){ return errorCode; }
		} 
		if( (errorCode = keyExt.SetValue(NULL, sGenProgID.c_str())) != 0 ){ return errorCode; }
	}

	tstring sProgIDPath = sBasePath + sGenProgID;
	if( ! CRegKey::ExistsKey(HKEY_CURRENT_USER, sProgIDPath.c_str()) )
	{
		if( szProgID_HKLM[0] != _T('\0') )
		{
			if( (errorCode = CopyRegistry(HKEY_LOCAL_MACHINE, (sBasePath + szProgID_HKLM).c_str(), HKEY_CURRENT_USER, sProgIDPath.c_str())) != 0 ){ return errorCode; }
		}
	}

	tstring sShellPath = sProgIDPath + _T("\\shell");
	tstring sShellActionPath = sShellPath + _T("\\") + ACTION_NAME;
	tstring sShellActionCommandPath = sShellActionPath + _T("\\command");
	tstring sBackupPath = sShellActionPath + ACTION_BACKUP_PATH;

	CRegKey keyShellActionCommand;
	if( (errorCode = keyShellActionCommand.Open(HKEY_CURRENT_USER, sShellActionCommandPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
	{
		if( (errorCode = keyShellActionCommand.Create(HKEY_CURRENT_USER, sShellActionCommandPath.c_str())) != 0 ){ return errorCode; }
	}

	TCHAR sExePath[_MAX_PATH] = {0};
	::GetModuleFileName( NULL, sExePath, _countof(sExePath) );
	tstring sCommandPathArg = tstring() + _T("\"") + sExePath + _T("\" \"%1\"");
	if( (errorCode = keyShellActionCommand.SetValue(NULL, sCommandPathArg.c_str())) != 0 ){ return errorCode; }

	CRegKey keyShellAction;
	if( (errorCode = keyShellAction.Open(HKEY_CURRENT_USER, sShellActionPath.c_str(), KEY_READ | KEY_WRITE)) !=0 ){ return errorCode; }
	if( (errorCode = keyShellAction.SetValue(NULL, _T("Sakura &Editor"))) != 0 ){ return errorCode; }

	CRegKey keyShell;
	if( (errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ | KEY_WRITE)) != 0 ){ return errorCode; }
	TCHAR szShellValue[ BUFFER_SIZE ] = {0};
	keyShell.GetValue(NULL, szShellValue, _countof(szShellValue));
	if(bDefProg)
	{
		if( _tcscmp(szShellValue, ACTION_NAME) != 0 )
		{
			if( szShellValue[0] != '\0')
			{
				CRegKey keyBackup;
				if( (errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
				{
					if( (errorCode = keyBackup.Create(HKEY_CURRENT_USER, sBackupPath.c_str())) != 0 ){ return errorCode; }
				}
				keyBackup.SetValue(NULL, szShellValue);
			}
			keyShell.SetValue(NULL, ACTION_NAME);
		}
	}
	else
	{
		CRegKey keyBackup;
		if( (errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
		{
			keyShell.DeleteValue(_T(""));
		}
		else
		{
			TCHAR sBackupValue[ BUFFER_SIZE ] = {0};
			keyBackup.GetValue(NULL, sBackupValue, _countof(sBackupValue));
			keyShell.SetValue(NULL, sBackupValue);
		}
	}

	return ERROR_SUCCESS;
}

/*!
	@brief �g���q���Ƃ̊֘A�t�����W�X�g���ݒ���폜����
	@param sExt	[in]�g���q
	�����̗���
	�E[HKCU\Software\Classes\.(�g���q)]�̑��݃`�F�b�N
		���݂��Ȃ���ΏI��
	�EProgID <- [HKCU\Software\Classes\.(�g���q)]�̒l
	�E[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]�̑��݃`�F�b�N
		���݂��Ȃ���ΏI��
	�E[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]�̑��݃`�F�b�N
		���݂���΁A�ޔ������l��[HKCU\Software\Classes\(ProgID)\shell]�ɐݒ�
		���݂��Ȃ���΁A[HKCU\Software\Classes\(ProgID)\shell]�̒l���폜
	�EProgID�̐擪��"SakuraEditor_"���H
		�����Ȃ�[HKCU\Software\Classes\(ProgID)]��[HKCU\Software\Classes\.(�g���q)]���폜
	�@�@�����łȂ����[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]���폜
*/
int UnregistExt(LPCTSTR sExt)
{
	int errorCode = ERROR_SUCCESS;
	tstring sBasePath = tstring( _T("Software\\Classes\\") );

	//��������
	TCHAR szLowerExt[MAX_PATH] = {0};
	_tcsncpy_s(szLowerExt, sizeof(szLowerExt) / sizeof(szLowerExt[0]), sExt, _tcslen(sExt));
	CharLower(szLowerExt);

	tstring sDotExt = sBasePath + _T(".") + szLowerExt;
	tstring sGenProgID = tstring() + szLowerExt + _T("file");

	CRegKey keyExt;
	if((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ | KEY_WRITE)) != 0)
	{
		return errorCode;
	}

	TCHAR szProgID[ BUFFER_SIZE ] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if( szProgID[0] == _T('\0') )
	{
		return ERROR_SUCCESS;
	}

	tstring sProgIDPath = sBasePath + szProgID;
	tstring sShellPath = sProgIDPath + _T("\\shell");
	tstring sShellActionPath = sShellPath + _T("\\") + ACTION_NAME;
	tstring sShellActionCommandPath = sShellActionPath + _T("\\command");
	tstring sBackupPath = sShellActionPath + ACTION_BACKUP_PATH;

	CRegKey keyShellAction;
	if( (errorCode = keyShellAction.Open(HKEY_CURRENT_USER, sShellActionPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
	{
		return ERROR_SUCCESS;
	}

	CRegKey keyShell;
	if( (errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ | KEY_WRITE)) != 0 ){ return errorCode; }
	CRegKey keyBackup;
	if( (errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
	{
		keyShell.DeleteValue(_T(""));
	}
	else
	{
		TCHAR szBackupValue[ BUFFER_SIZE ] = {0};
		keyBackup.GetValue(NULL, szBackupValue, _countof(szBackupValue));
		keyShell.SetValue(NULL, szBackupValue);
	}

	keyBackup.Close();
	keyShellAction.Close();
	if( _tcsncmp(szProgID, _T("SakuraEditor_"), 13) == 0)
	{
		if( (errorCode = DeleteRegistry(HKEY_CURRENT_USER, sProgIDPath)) != 0 ){ return errorCode; }

		TCHAR szBackupValue[ BUFFER_SIZE ] = {0};
		keyExt.GetValue(PROGID_BACKUP_NAME, szBackupValue, _countof(szBackupValue));
		if( szBackupValue[0] != _T('\0') ){
			keyExt.SetValue(NULL, szBackupValue);
		}else{
			if( (errorCode = DeleteRegistry(HKEY_CURRENT_USER, sDotExt)) != 0 ){ return errorCode; }
		}
	}else{
		if( (errorCode = DeleteRegistry(HKEY_CURRENT_USER, sShellActionPath)) != 0 ){ return errorCode; }
	}

	return ERROR_SUCCESS;
}

/*!
	@brief �g���q���Ƃ̊֘A�t�����W�X�g���ݒ���m�F����
	@param sExt			[in]�g���q
	@param pbRMenu		[out]�֘A�t���ݒ�
	@param pbDblClick	[out]����ݒ�
	�����̗���
	�EpbRMenu <- false, pbDblClick <- false
	�E[HKCU\Software\Classes\.(�g���q)]�̑��݃`�F�b�N
		���݂��Ȃ���ΏI��
	�EProgID <- [HKCU\Software\Classes\.(�g���q)]�̒l
	�E[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]�̑��݃`�F�b�N
		���݂��Ȃ���ΏI��
	�EpbRMenu <- true
	�E[HKCU\Software\Classes\(ProgID)\shell]�̒l���`�F�b�N
		�uSakuraEditor�v�Ȃ�ApbDblClick <- true
*/
int CheckExt(LPCTSTR sExt, bool *pbRMenu, bool *pbDblClick)
{
	int errorCode = ERROR_SUCCESS;
	tstring sBasePath = tstring( _T("Software\\Classes\\") );

	*pbRMenu = false;
	*pbDblClick = false;

	//��������
	TCHAR szLowerExt[MAX_PATH] = {0};
	_tcsncpy_s(szLowerExt, sizeof(szLowerExt) / sizeof(szLowerExt[0]), sExt, _tcslen(sExt));
	CharLower(szLowerExt);

	tstring sDotExt = sBasePath + _T(".") + szLowerExt;
	tstring sGenProgID = tstring() + szLowerExt + _T("file");

	CRegKey keyExt;
	if((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ)) != 0)
	{
		return ERROR_SUCCESS;
	}

	TCHAR szProgID[ BUFFER_SIZE ] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if(szProgID[0] == _T('\0'))
	{
		return ERROR_SUCCESS;
	}

	tstring sShellPath = tstring() + _T("Software\\Classes\\") + szProgID + _T("\\shell");
	tstring sShellActionPath = sShellPath + _T("\\") + ACTION_NAME;
	if( ! CRegKey::ExistsKey(HKEY_CURRENT_USER, sShellActionPath.c_str()) )
	{
		return ERROR_SUCCESS;
	}
	*pbRMenu = true;

	CRegKey keyShell;
	if( (errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ)) != 0 ){ return errorCode; }
	TCHAR szShellValue[ BUFFER_SIZE ] = {0};
	keyShell.GetValue(NULL, szShellValue, _countof(szShellValue));
	if( _tcscmp( szShellValue, ACTION_NAME ) == 0 )
	{
		*pbDblClick = true;
	}

	return ERROR_SUCCESS;
}

/*!
	@brief ���W�X�g���ύX�̌x�����b�Z�[�W��\������
*/
bool CDlgTypeList::AlertFileAssociation()
{
	if( m_bAlertFileAssociation ){
		if( IDYES == ::MYMESSAGEBOX( 
						NULL, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
						GSTR_APPNAME,
						_T("Windows�̊֘A�t���ݒ��ύX���悤�Ƃ��Ă��܂��B\n���̑���͓����ݒ�𗘗p���鑼�̃\�t�g�ɂ��e����^����\��������܂��B\n���{���܂����H"))
					)
		{
			m_bAlertFileAssociation = false;	//�u�͂��v�Ȃ�ŏ��̈�x�����m�F����
			return true;
		}else{
			return false;
		}
	}
	return true;
}