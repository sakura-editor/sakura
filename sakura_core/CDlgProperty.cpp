//	$Id$
/*!	@file
	�t�@�C���v���p�e�B�_�C�A���O
	
	@author Norio Nakatani
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
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgProperty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "etc_uty.h"
#include "funccode.h"		// Stonee, 2001/03/12

/* ���[�_���_�C�A���O�̕\�� */
int CDlgProperty::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return CDialog::DoModal( hInstance, hwndParent, IDD_PROPERTY_FILE, lParam );
}

BOOL CDlgProperty::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�t�@�C���̃v���p�e�B�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );
		return TRUE;
	case IDOK:			/* ������ */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgProperty::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	CMemory			cmemProp;
//	char*			pWork;
	char			szWork[100];
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS",
		"EUC",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
	int				nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );
	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;
//	SYSTEMTIME		systimeU;
	SYSTEMTIME		systimeL;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	//	Aug. 16, 2000 genta	�S�p��
	cmemProp.AppendSz( "�t�@�C����  " );
	cmemProp.AppendSz( pCEditDoc->m_szFilePath );
	cmemProp.AppendSz( "\r\n" );

	cmemProp.AppendSz( "�ݒ�̃^�C�v  " );
	cmemProp.AppendSz( pCEditDoc->GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendSz( "\r\n" );

	cmemProp.AppendSz( "�����R�[�h  " );
	cmemProp.AppendSz( pCodeNameArr[pCEditDoc->m_nCharCode] );
	cmemProp.AppendSz( "\r\n" );

	wsprintf( szWork, "�s��  %d�s\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	wsprintf( szWork, "���C�A�E�g�s��  %d�s\r\n", pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	if( pCEditDoc->m_bReadOnly ){
		cmemProp.AppendSz( "�㏑���֎~���[�h�ŊJ���Ă��܂��B\r\n" );
	}
	if( pCEditDoc->m_bIsModified ){
		cmemProp.AppendSz( "�ύX����Ă��܂��B\r\n" );
	}else{
		cmemProp.AppendSz( "�ύX����Ă��܂���B\r\n" );
	}

	wsprintf( szWork, "\r\n�R�}���h���s��    %d��\r\n", pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendSz( szWork );

	wsprintf( szWork, "--�t�@�C�����-----------------\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->m_szFilePath, (WIN32_FIND_DATA*)&wfd ) ) ){
		if( pCEditDoc->m_hLockedFile ){
			if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_DENY_WRITE ){
				wsprintf( szWork, "���Ȃ��͂��̃t�@�C�����A���v���Z�X����̏㏑���֎~���[�h�Ń��b�N���Ă��܂��B\r\n" );
			}else
			if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_EXCLUSIVE ){
				wsprintf( szWork, "���Ȃ��͂��̃t�@�C�����A���v���Z�X����̓ǂݏ����֎~���[�h�Ń��b�N���Ă��܂��B\r\n" );
			}else{
				wsprintf( szWork, "���Ȃ��͂��̃t�@�C�������b�N���Ă��܂��B\r\n" );
			}
			cmemProp.AppendSz( szWork );
		}else{
			wsprintf( szWork, "���Ȃ��͂��̃t�@�C�������b�N���Ă��܂���B\r\n" );
			cmemProp.AppendSz( szWork );
		}
		
		wsprintf( szWork, "�t�@�C������  ", pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendSz( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendSz( "/�A�[�J�C�u" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendSz( "/���k" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendSz( "/�t�H���_" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendSz( "/�B��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendSz( "/�m�[�}��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendSz( "/�I�t���C��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendSz( "/�ǂݎ���p" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendSz( "/�V�X�e��" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendSz( "/�e���|����" );
		}
		cmemProp.AppendSz( "\r\n" );


		cmemProp.AppendSz( "�쐬����  " );
		::FileTimeToLocalFileTime( &wfd.ftCreationTime, &wfd.ftCreationTime );
		::FileTimeToSystemTime( &wfd.ftCreationTime, &systimeL );
		wsprintf( szWork, "%d�N%d��%d�� %02d:%02d:%02d",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay,
			systimeL.wHour,
			systimeL.wMinute,
			systimeL.wSecond
		);
		cmemProp.AppendSz( szWork );
		cmemProp.AppendSz( "\r\n" );

		cmemProp.AppendSz( "�X�V����  " );
		::FileTimeToLocalFileTime( &wfd.ftLastWriteTime, &wfd.ftLastWriteTime );
		::FileTimeToSystemTime( &wfd.ftLastWriteTime, &systimeL );
		wsprintf( szWork, "%d�N%d��%d�� %02d:%02d:%02d",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay,
			systimeL.wHour,
			systimeL.wMinute,
			systimeL.wSecond
		);
		cmemProp.AppendSz( szWork );
		cmemProp.AppendSz( "\r\n" );


		cmemProp.AppendSz( "�A�N�Z�X��  " );
		::FileTimeToLocalFileTime( &wfd.ftLastAccessTime, &wfd.ftLastAccessTime );
		::FileTimeToSystemTime( &wfd.ftLastAccessTime, &systimeL );
		wsprintf( szWork, "%d�N%d��%d��",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay
		);
		cmemProp.AppendSz( szWork );
		cmemProp.AppendSz( "\r\n" );

		wsprintf( szWork, "MS-DOS�t�@�C����  %s\r\n", wfd.cAlternateFileName );
		cmemProp.AppendSz( szWork );

		wsprintf( szWork, "�t�@�C���T�C�Y  %d �o�C�g\r\n", wfd.nFileSizeLow );
		cmemProp.AppendSz( szWork );

		::FindClose( nFind );
	}

	
	
	
#ifdef _DEBUG/////////////////////////////////////////////////////
	int		nEUCMojiNum, nEUCCodeNum;
	int		nSJISMojiNum, nSJISCodeNum;
	int		nUNICODEMojiNum, nUNICODECodeNum;
	int		nJISMojiNum, nJISCodeNum;
	int		nUTF8MojiNum, nUTF8CodeNum;
	int		nUTF7MojiNum, nUTF7CodeNum;

	HFILE					hFile;
	HGLOBAL					hgData;
	const unsigned char*	pBuf;
	int						nBufLen;
	/* �������m�� & �t�@�C���ǂݍ��� */
	hgData = NULL;
	hFile = _lopen( pCEditDoc->m_szFilePath, OF_READ );
	if( HFILE_ERROR == hFile ){
		goto end_of_CodeTest;
	}
	nBufLen = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		_lclose( hFile );
		goto end_of_CodeTest;
	}
	pBuf = (const unsigned char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );
	
	/* 
	||�t�@�C���̓��{��R�[�h�Z�b�g����: Unicode���H
	|| �G���[�̏ꍇ�AFALSE��Ԃ�
	*/
	if( CMemory::CheckKanjiCode_UNICODE( pBuf, nBufLen, &nUNICODEMojiNum, &nUNICODECodeNum ) ){
		if( nUNICODECodeNum!=0 && nUNICODEMojiNum != 0 ){
			wsprintf( szWork, "Unicode�R�[�h�����F������%d  Unicode���L������%d (%d%%)\r\n", nUNICODEMojiNum, nUNICODECodeNum, nUNICODECodeNum*100/nUNICODEMojiNum );
		}else{
			wsprintf( szWork, "Unicode�R�[�h�����F������%d  Unicode���L������%d (0%%)\r\n", nUNICODEMojiNum, nUNICODECodeNum );
		}
		cmemProp.AppendSz( szWork );
	}
	/* 
	||�t�@�C���̓��{��R�[�h�Z�b�g����: EUC���H
	|| �G���[�̏ꍇ�AFALSE��Ԃ�
	*/
	if( CMemory::CheckKanjiCode_EUC( pBuf, nBufLen, &nEUCMojiNum, &nEUCCodeNum ) ){
		if( nEUCCodeNum!=0 && nEUCMojiNum != 0 ){
			wsprintf( szWork, "EUC�R�[�h�����F������%d  EUC���L������%d (%d%%)\r\n", nEUCMojiNum, nEUCCodeNum, nEUCCodeNum*100/nEUCMojiNum );
		}else{
			wsprintf( szWork, "EUC�R�[�h�����F������%d  EUC���L������%d (0%%)\r\n", nEUCMojiNum, nEUCCodeNum );
		}
		cmemProp.AppendSz( szWork );
	}
	/* 
	||�t�@�C���̓��{��R�[�h�Z�b�g����: SJIS���H
	|| �G���[�̏ꍇ�AFALSE��Ԃ�
	*/
	if( CMemory::CheckKanjiCode_SJIS( pBuf, nBufLen, &nSJISMojiNum, &nSJISCodeNum ) ){
		if( nSJISCodeNum!=0 && nSJISMojiNum != 0 ){
			wsprintf( szWork, "SJIS�R�[�h�����F������%d  SJIS���L������%d (%d%%)\r\n", nSJISMojiNum, nSJISCodeNum, nSJISCodeNum*100/nSJISMojiNum );
		}else{
			wsprintf( szWork, "SJIS�R�[�h�����F������%d  SJIS���L������%d (0%%)\r\n", nSJISMojiNum, nSJISCodeNum );
		}
		cmemProp.AppendSz( szWork );
	}


	/* 
	||�t�@�C���̓��{��R�[�h�Z�b�g����: JIS���H
	|| �G���[�̏ꍇ�AFALSE��Ԃ�
	*/
	if( CMemory::CheckKanjiCode_JIS( pBuf, nBufLen, &nJISMojiNum, &nJISCodeNum ) ){
		if( nJISCodeNum!=0 && nJISMojiNum != 0 ){
			wsprintf( szWork, "JIS�R�[�h�����F������%d  JIS���L������%d (%d%%)\r\n", nJISMojiNum, nJISCodeNum, nJISCodeNum*100/nJISMojiNum );
		}else{
			wsprintf( szWork, "JIS�R�[�h�����F������%d  JIS���L������%d (0%%)\r\n", nJISMojiNum, nJISCodeNum );
		}
		cmemProp.AppendSz( szWork );
	}

	/* 
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UTF-8S���H
	|| �G���[�̏ꍇ�AFALSE��Ԃ�
	*/
	if( CMemory::CheckKanjiCode_UTF8( pBuf, nBufLen, &nUTF8MojiNum, &nUTF8CodeNum ) ){
		if( nUTF8CodeNum!=0 && nUTF8MojiNum != 0 ){
			wsprintf( szWork, "UTF-8�R�[�h�����F������%d  UTF-8���L������%d (%d%%)\r\n", nUTF8MojiNum, nUTF8CodeNum, nUTF8CodeNum*100/nUTF8MojiNum );
		}else{
			wsprintf( szWork, "UTF-8�R�[�h�����F������%d  UTF-8���L������%d (0%%)\r\n", nUTF8MojiNum, nUTF8CodeNum );
		}
		cmemProp.AppendSz( szWork );
	}

	/* 
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UTF-7S���H
	|| �G���[�̏ꍇ�AFALSE��Ԃ�
	*/
	if( CMemory::CheckKanjiCode_UTF7( pBuf, nBufLen, &nUTF7MojiNum, &nUTF7CodeNum ) ){
		if( nUTF7CodeNum!=0 && nUTF7MojiNum != 0 ){
			wsprintf( szWork, "UTF-7�R�[�h�����F������%d  UTF-7���L������%d (%d%%)\r\n", nUTF7MojiNum, nUTF7CodeNum, nUTF7CodeNum*100/nUTF7MojiNum );
		}else{
			wsprintf( szWork, "UTF-7�R�[�h�����F������%d  UTF-7���L������%d (0%%)\r\n", nUTF7MojiNum, nUTF7CodeNum );
		}
		cmemProp.AppendSz( szWork );
	}
	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::SetDlgItemText( m_hWnd, IDC_EDIT1, cmemProp.GetPtr( NULL ) );

	return;
}


/*[EOF]*/
