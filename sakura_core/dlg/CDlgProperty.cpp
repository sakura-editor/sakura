/*!	@file
	@brief �t�@�C���v���p�e�B�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, Moca, MIK, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "dlg/CDlgProperty.h"
#include "debug/Debug.h"
#include "doc/CEditDoc.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "global.h"		// Moca, 2002/05/26
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "charset/charcode.h"	// rastiv, 2006/06/28
#include "io/CBinaryStream.h"
#include "util/shell.h"
#include "charset/CESI.h"
#include "CAppMode.h"

// �v���p�e�B CDlgProperty.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
//	IDCANCEL,				HIDCANCEL_PROP,			// ���g�p del 2008/7/4 Uchi
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT_PROPERTY,		HIDC_PROP_EDIT1,		// IDC_EDIT1->IDC_EDIT_PROPERTY	2008/7/3 Uchi
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* ���[�_���_�C�A���O�̕\�� */
int CDlgProperty::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PROPERTY_FILE, lParam );
}

BOOL CDlgProperty::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�t�@�C���̃v���p�e�B�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDOK:			/* ������ */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
//	case IDCANCEL:							// ���g�p del 2008/7/4 Uchi
//		::EndDialog( GetHwnd(), FALSE );
//		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/*! �_�C�A���O�f�[�^�̐ݒ�

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
void CDlgProperty::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	CNativeT		cmemProp;
	TCHAR			szWork[500];

	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	//	Aug. 16, 2000 genta	�S�p��
	cmemProp.AppendString( _T("�t�@�C����  ") );
	cmemProp.AppendString( pCEditDoc->m_cDocFile.GetFilePath() );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( _T("�ݒ�̃^�C�v  ") );
	cmemProp.AppendString( pCEditDoc->m_cDocType.GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( _T("�����R�[�h  ") );
	cmemProp.AppendString( CCodeTypeName(pCEditDoc->GetDocumentEncoding()).Normal() );
	//	From Here  2008/4/27 Uchi
	if (pCEditDoc->m_cDocFile.IsBomExist()) {
		cmemProp.AppendString( _T(" with BOM") );
	}
	//	To Here  2008/4/27 Uchi
	cmemProp.AppendString( _T("\r\n") );

	auto_sprintf( szWork, _T("�s��  %d�s\r\n"), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	auto_sprintf( szWork, _T("���C�A�E�g�s��  %d�s\r\n"), pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( CAppMode::Instance()->IsViewMode() ){
		cmemProp.AppendString( _T("�r���[���[�h�ŊJ���Ă��܂��B\r\n") );	// 2009.04.11 ryoji �u�㏑���֎~���[�h�v���u�r���[���[�h�v
	}
	if( pCEditDoc->m_cDocEditor.IsModified() ){
		cmemProp.AppendString( _T("�ύX����Ă��܂��B\r\n") );
	}else{
		cmemProp.AppendString( _T("�ύX����Ă��܂���B\r\n") );
	}

	auto_sprintf( szWork, _T("\r\n�R�}���h���s��    %d��\r\n"), pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendString( szWork );

	auto_sprintf( szWork, _T("--�t�@�C�����-----------------\r\n"), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->m_cDocFile.GetFilePath(), &wfd ) ) ){
		if( pCEditDoc->m_cDocFile.IsFileLocking() ){
			if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ){
				auto_sprintf( szWork, _T("���Ȃ��͂��̃t�@�C�����A���v���Z�X����̏㏑���֎~���[�h�Ń��b�N���Ă��܂��B\r\n") );
			}
			else if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
				auto_sprintf( szWork, _T("���Ȃ��͂��̃t�@�C�����A���v���Z�X����̓ǂݏ����֎~���[�h�Ń��b�N���Ă��܂��B\r\n") );
			}
			else{
				auto_sprintf( szWork, _T("���Ȃ��͂��̃t�@�C�������b�N���Ă��܂��B\r\n") );
			}
			cmemProp.AppendString( szWork );
		}
		else{
			auto_sprintf( szWork, _T("���Ȃ��͂��̃t�@�C�������b�N���Ă��܂���B\r\n") );
			cmemProp.AppendString( szWork );
		}

		auto_sprintf( szWork, _T("�t�@�C������  "), pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendString( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendString( _T("/�A�[�J�C�u") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendString( _T("/���k") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendString( _T("/�t�H���_") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendString( _T("/�B��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendString( _T("/�m�[�}��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendString( _T("/�I�t���C��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendString( _T("/�ǂݎ���p") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendString( _T("/�V�X�e��") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendString( _T("/�e���|����") );
		}
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( _T("�쐬����  ") );
		CFileTime ctimeCreation = wfd.ftCreationTime;
		auto_sprintf( szWork, _T("%d�N%d��%d�� %02d:%02d:%02d"),
			ctimeCreation->wYear,
			ctimeCreation->wMonth,
			ctimeCreation->wDay,
			ctimeCreation->wHour,
			ctimeCreation->wMinute,
			ctimeCreation->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		cmemProp.AppendString( _T("�X�V����  ") );
		CFileTime ctimeLastWrite = wfd.ftLastWriteTime;
		auto_sprintf( szWork, _T("%d�N%d��%d�� %02d:%02d:%02d"),
			ctimeLastWrite->wYear,
			ctimeLastWrite->wMonth,
			ctimeLastWrite->wDay,
			ctimeLastWrite->wHour,
			ctimeLastWrite->wMinute,
			ctimeLastWrite->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( _T("�A�N�Z�X��  ") );
		CFileTime ctimeLastAccess = wfd.ftLastAccessTime;
		auto_sprintf( szWork, _T("%d�N%d��%d��"),
			ctimeLastAccess->wYear,
			ctimeLastAccess->wMonth,
			ctimeLastAccess->wDay
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		auto_sprintf( szWork, _T("MS-DOS�t�@�C����  %ts\r\n"), wfd.cAlternateFileName );
		cmemProp.AppendString( szWork );

		auto_sprintf( szWork, _T("�t�@�C���T�C�Y  %d �o�C�g\r\n"), wfd.nFileSizeLow );
		cmemProp.AppendString( szWork );

		::FindClose( nFind );
	}




#ifdef _DEBUG/////////////////////////////////////////////////////
	HGLOBAL					hgData;
	char*					pBuf;
	int						nBufLen;
	CNativeT				ctext;
	/* �������m�� & �t�@�C���ǂݍ��� */
	hgData = NULL;
	CBinaryInputStream in(pCEditDoc->m_cDocFile.GetFilePath());
	if(!in){
		goto end_of_CodeTest;
	}
	nBufLen = in.GetLength();
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		in.Close();
		goto end_of_CodeTest;
	}
	pBuf = GlobalLockChar( hgData );
	in.Read( pBuf, nBufLen );
	in.Close();

	//CESI�̃f�o�b�O���
	CESI::GetDebugInfo(pBuf,nBufLen,&ctext);
	cmemProp.AppendNativeData(ctext);

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_PROPERTY, cmemProp.GetStringPtr() );

	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end
