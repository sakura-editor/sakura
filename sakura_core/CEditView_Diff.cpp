//	$Id$
/*!	@file
	@brief DIFF�����\��

	@author MIK
	@date	2002/05/25 ExecCmd ���Q�l��DIFF���s���ʂ���荞�ޏ����쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, GAE, YAZAKI, hor
	Copyright (C) 2002, hor, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
//#include "CDlgCancel.h"
#include "CDlgDiff.h"
#include "CEditDoc.h"
#include "CEditView.h"
#include "CDocLine.h"
#include "CDocLineMgr.h"
#include "CWaitCursor.h"
#include "debug.h"

/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�����)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::Command_Diff_Dialog( void )
{
	CDlgDiff	cDlgDiff;
//	bool	bTmpFile1, bTmpFile2;
	char	szTmpFile1[_MAX_PATH];
	char	szTmpFile2[_MAX_PATH];

	//DIFF�����\���_�C�A���O��\������
	if( FALSE == cDlgDiff.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc,
		m_pcEditDoc->m_szFilePath,
		m_pcEditDoc->IsModified() ) )
	{
		return;
	}

	//���t�@�C��
	if( m_pcEditDoc->IsModified() )
	{
		MessageBox( NULL,  
			"�����R�}���h���s�͎��s���܂����B\n�t�@�C����ۑ����Ă���s���Ă��������B", 
			"DIFF�����\��",
			MB_OK | MB_ICONEXCLAMATION );
		return;
/*
#if 0
		if( tmpnam( szTmpFile1 ) == NULL )
		{
			MessageBox( NULL,  
				"�����R�}���h���s�͎��s���܂����B", 
				"DIFF�����\��",
				MB_OK | MB_ICONEXCLAMATION );
			return;
		}

		bTmpFile1 = true;

		// �ҏW���t�@�C�����ꎞ�t�@�C���ɕۑ�����
		if( MakeDiffTmpFile( szTmpFile1, NULL ) == FALSE )
		{
			MessageBox( NULL,  
				"�����R�}���h���s�͎��s���܂����B", 
				"DIFF�����\��",
				MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
#endif
*/
	}
	else
	{
//		bTmpFile1 = false;
		strcpy( szTmpFile1, m_pcEditDoc->m_szFilePath );
		if( strlen( szTmpFile1 ) == 0 ) return;
	}

	//����t�@�C��
	if( cDlgDiff.m_bIsModifiedDst )	/* �����t�@�C���͕ҏW�����H */
	{
		MessageBox( NULL,  
			"�����R�}���h���s�͎��s���܂����B\n�t�@�C����ۑ����Ă���s���Ă��������B", 
			"DIFF�����\��",
			MB_OK | MB_ICONEXCLAMATION );
		return;
/*
#if 0
		if( tmpnam( szTmpFile2 ) == NULL )
		{
			MessageBox( NULL,  
				"�����R�}���h���s�͎��s���܂����B", 
				"DIFF�����\��",
				MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}

		bTmpFile2 = true;

		// �����ҏW���t�@�C�����ꎞ�t�@�C���ɕۑ�����
		if( MakeDiffTmpFile( szTmpFile2, cDlgDiff.m_hWnd_Dst ) == FALSE )
		{
			MessageBox( NULL,  
				"�����R�}���h���s�͎��s���܂����B", 
				"DIFF�����\��",
				MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
#endif
*/
	}
	else
	{
//		bTmpFile2 = false;
		strcpy( szTmpFile2, cDlgDiff.m_szFile2 );
		if( strlen( szTmpFile2 ) == 0 ) return;
	}

	/* �����\�� */
	Command_Diff( szTmpFile1, szTmpFile2, cDlgDiff.m_nDiffFlgOpt );

/*
finish:;
	//�ꎞ�t�@�C�����폜����
	if( bTmpFile1 ) unlink( szTmpFile1 );
	if( bTmpFile2 ) unlink( szTmpFile2 );
*/

	return;
}

/*!	�����\��
	@param	pszFile1	[in]	���t�@�C����
	@param	pszFile2	[in]	�V�t�@�C����
//	@param	nFlgFile12	[in]	�ҏW���t�@�C����...
//									0	�t�@�C��1(���t�@�C��)
//									1	�t�@�C��2(�V�t�@�C��)
	@param	nFlgOpt		[in]	0b000000000
							      ||||+--- -i ignore-case         �啶�����������ꎋ
							      |||+---- -w ignore-all-space    �󔒖���
							      ||+----- -b ignore-space-change �󔒕ύX����
							      |+------ -B ignore-blank-lines  ��s����
							      +------- -t expand-tabs         TAB-SPACE�ϊ�
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::Command_Diff( 
	const char	*pszFile1,
	const char	*pszFile2,
	/* int			nFlgFile12, */
	int			nFlgOpt )
/*
	bool	bFlgCase,		//�啶�����������ꎋ
	bool	bFlgBlank,		//�󔒖���
	bool	bFlgWhite,		//�󔒕ύX����
	bool	bFlgBLine,		//��s����
	bool	bFlgTabSpc,		//TAB-SPACE�ϊ�
*/
{
	char	cmdline[1024];
	HANDLE	hStdOutWrite, hStdOutRead;
//	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_hWnd );
	int		nFlgFile12 = 1;

	//������DIFF��������������B
	if( m_pcEditDoc->m_cDocLineMgr.IsDiffUse() )
		Command_Diff_Reset();
		//m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(SECURITY_ATTRIBUTES) );
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = NULL;
	hStdOutRead = hStdOutWrite = 0;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE )
	{
		//�G���[�B�΍�����
		return;
	}

	//�p���s�\�ɂ���
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS );

	//CreateProcess�ɓn��STARTUPINFO���쐬
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(STARTUPINFO) );
	sui.cb          = sizeof(STARTUPINFO);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//�I�v�V�������쐬����
	char	szOption[16];	// "-cwbBt"
	strcpy( szOption, "-" );
	if( nFlgOpt & 0x0001 ) strcat( szOption, "i" );	//-i ignore-case         �啶�����������ꎋ
	if( nFlgOpt & 0x0002 ) strcat( szOption, "w" );	//-w ignore-all-space    �󔒖���
	if( nFlgOpt & 0x0004 ) strcat( szOption, "b" );	//-b ignore-space-change �󔒕ύX����
	if( nFlgOpt & 0x0008 ) strcat( szOption, "B" );	//-B ignore-blank-lines  ��s����
	if( nFlgOpt & 0x0010 ) strcat( szOption, "t" );	//-t expand-tabs         TAB-SPACE�ϊ�
	if( strcmp( szOption, "-" ) == 0 ) strcpy( szOption, "" );	//�I�v�V�����Ȃ�

	/* exe�̂���t�H���_ */
	char	szPath[_MAX_PATH + 1];
	char	szExeFolder[_MAX_PATH + 1];
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof( szPath )
	);
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, szExeFolder, NULL );

	//�R�}���h���C��������쐬(MAX:1024)
	wsprintf( cmdline, "%s\\%s %s \"%s\" \"%s\"",
			szExeFolder,	//sakura.exe�p�X
			"diff.exe",		//diff.exe
			szOption,		//diff�I�v�V����
			( nFlgFile12 ? pszFile2 : pszFile1 ),
			( nFlgFile12 ? pszFile1 : pszFile2 )
		);

	//�R�}���h���C�����s
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE )
	{
		MessageBox( NULL, cmdline, "�����R�}���h���s�͎��s���܂����B", MB_OK | MB_ICONEXCLAMATION );
		goto finish;
	}

	{
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		int		j;
		bool	bLoopFlag = true;
		bool	bLineHead = true;	//�s����
		bool	bDiffInfo = false;	//DIFF���
		int		nDiffLen = 0;		//DIFF���
		char	szDiffData[100];	//DIFF���

		//���f�_�C�A���O�\��
//		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );

		//���s���ʂ̎�荞��
		do {
			//�������̃��[�U�[������\�ɂ���
//			if( !::BlockingHook( cDlgCancel.m_hWnd ) )
//			{
//				bDiffInfo = false;
//				break;
//			}

			//���f�{�^�������`�F�b�N
//			if( cDlgCancel.IsCanceled() )
//			{
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
//				::TerminateProcess( pi.hProcess, 0 );
//				bDiffInfo = false;
//				break;
//			}

			//�v���Z�X���I�����Ă��Ȃ����m�F
			if( WaitForSingleObject( pi.hProcess, 0 ) == WAIT_OBJECT_0 )
			{
				//�I�����Ă���΃��[�v�t���O��FALSE�Ƃ���
				//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
				bLoopFlag = FALSE;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
			{
				if( new_cnt > 0 )												//�ҋ@���̂��̂�����
				{
					if( new_cnt >= sizeof(work) - 2 )							//�p�C�v����ǂݏo���ʂ𒲐�
					{
						new_cnt = sizeof(work) - 2;
					}
					ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					if( read_cnt == 0 )
					{
						continue;
					}

					//�ǂݏo������������`�F�b�N����
					for( j = 0; j < (int)read_cnt/*-1*/; j++ )
					{
						if( bLineHead )
						{
							if( work[j] != '\n' && work[j] != '\r' )
							{
								bLineHead = false;
							
								//DIFF���̎n�܂肩�H
								if( work[j] >= '0' && work[j] <= '9' )
								{
									bDiffInfo = true;
									nDiffLen = 0;
									szDiffData[nDiffLen++] = work[j];
								}
								/*
								else if( work[j] == '<' || work[j] == '>' || work[j] == '-' )
								{
									bDiffInfo = false;
									nDiffLen = 0;
								}
								*/
							}
						}
						else
						{
							//�s���ɒB�������H
							if( work[j] == '\n' || work[j] == '\r' )
							{
								//DIFF��񂪂���Ή�͂���
								if( bDiffInfo == true && nDiffLen > 0 )
								{
									szDiffData[nDiffLen] = '\0';
									AnalyzeDiffInfo( szDiffData, nFlgFile12 );
									nDiffLen = 0;
								}
								
								bDiffInfo = false;
								bLineHead = true;
							}
							else if( bDiffInfo == true )
							{
								//DIFF���ɒǉ�����
								szDiffData[nDiffLen++] = work[j];
								if( nDiffLen >= 99 )
								{
									nDiffLen = 0;
									bDiffInfo = false;
								}
							}
						}
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );

		//�c����DIFF��񂪂���Ή�͂���
		if( bDiffInfo == true && nDiffLen > 0 )
		{
			szDiffData[nDiffLen] = '\0';
			AnalyzeDiffInfo( szDiffData, nFlgFile12 );
		}
	}


finish:
	//�I������
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	//���������r���[���X�V
	for( int v = 0; v < 4; ++v )
		if( m_pcEditDoc->m_nActivePaneIndex != v )
			m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();

	return;
}

/*!	DIFF����������͂��}�[�N�o�^
	@param	pszFile2	[in]	�V�t�@�C����
	@param	nFlgFile12	[in]	�ҏW���t�@�C����...
									0	�t�@�C��1(���t�@�C��)
									1	�t�@�C��2(�V�t�@�C��)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::AnalyzeDiffInfo( 
	char	*pszDiffInfo,
	int		nFlgFile12 )
{
	/*
	 * 99a99		���t�@�C��99�s�̎��s�ɐV�t�@�C��99�s���ǉ����ꂽ�B
	 * 99a99,99		���t�@�C��99�s�̎��s�ɐV�t�@�C��99�`99�s���ǉ����ꂽ�B
	 * 99c99		���t�@�C��99�s���V�t�@�C��99�s�ɕύX���ꂽ�B
	 * 99,99c99,99	���t�@�C��99�`99�s���V�t�@�C��99�`99�s�ɕύX���ꂽ�B
	 * 99d99		���t�@�C��99�s���V�t�@�C��99�s�̎��s����폜���ꂽ�B
	 * 99,99d99		���t�@�C��99�`99�s���V�t�@�C��99�s�̎��s����폜���ꂽ�B
	 * s1,e1 mode s2,e2
	 * �擪�̏ꍇ0�̎��s�ƂȂ邱�Ƃ�����
	 */
	char	*q;
	int		s1, e1, s2, e2;
	char	mode;

	//�O���t�@�C���̊J�n�s
	s1 = 0;
	for( q = pszDiffInfo; *q; q++ )
	{
		if( *q == ',' ) break;
		if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
		//�s�ԍ��𒊏o
		if( *q >= '0' && *q <= '9' ) s1 = s1 * 10 + (*q - '0');
		else return;
	}
	if( ! *q ) return;

	//�O���t�@�C���̏I���s
	if( *q != ',' )
	{
		//�J�n�E�I���s�ԍ��͓���
		e1 = s1;
	}
	else
	{
		e1 = 0;
		for( q++; *q; q++ )
		{
			if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
			//�s�ԍ��𒊏o
			if( *q >= '0' && *q <= '9' ) e1 = e1 * 10 + (*q - '0');
			else return;
		}
	}
	if( ! *q ) return;

	//DIFF���[�h���擾
	mode = *q;

	//�㔼�t�@�C���̊J�n�s
	s2 = 0;
	for( q++; *q; q++ )
	{
		if( *q == ',' ) break;
		//�s�ԍ��𒊏o
		if( *q >= '0' && *q <= '9' ) s2 = s2 * 10 + (*q - '0');
		else return;
	}

	//�㔼�t�@�C���̏I���s
	if( *q != ',' )
	{
		//�J�n�E�I���s�ԍ��͓���
		e2 = s2;
	}
	else
	{
		e2 = 0;
		for( q++; *q; q++ )
		{
			//�s�ԍ��𒊏o
			if( *q >= '0' && *q <= '9' ) e2 = e2 * 10 + (*q - '0');
			else return;
		}
	}

	//�s���ɒB���ĂȂ���΃G���[
	if( *q ) return;

	//���o����DIFF��񂩂�s�ԍ��ɍ����}�[�N��t����
	if( 0 == nFlgFile12 )	//�ҏW���t�@�C���͋��t�@�C��
	{
		if     ( mode == 'a' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_DELETE, s1    , e1     );
		else if( mode == 'c' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_CHANGE, s1 - 1, e1 - 1 );
		else if( mode == 'd' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_APPEND, s1 - 1, e1 - 1 );
	}
	else	//�ҏW���t�@�C���͐V�t�@�C��
	{
		if     ( mode == 'a' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_APPEND, s2 - 1, e2 - 1 );
		else if( mode == 'c' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_CHANGE, s2 - 1, e2 - 1 );
		else if( mode == 'd' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_DELETE, s2    , e2     );
	}

	return;
}

/*!	���̍�����T���C����������ړ�����
*/
void CEditView::Command_Diff_Next( void )
{
	int			nX = 0;
	int			nY;
	int			nYOld;
	BOOL		bFound = FALSE;
	BOOL		bRedo = TRUE;

	nY = m_nCaretPosY_PHY;
	nYOld = nY;

re_do:;	
	if( m_pcEditDoc->m_cDocLineMgr.SearchDiffMark( nY, 1 /* ������� */, &nY ) )
	{
		bFound = TRUE;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nX, nY, &nX, &nY );
		if( m_bSelectingLock )
		{
			if( !IsTextSelected() ) BeginSelectArea();
		}
		else
		{
			if( IsTextSelected() ) DisableSelectArea( TRUE );
		}
		MoveCursor( nX, nY, TRUE );
		if( m_bSelectingLock )
		{
			ChangeSelectAreaByCurrentCursor( nX, nY );
		}
	}

	if( m_pShareData->m_Common.m_bSearchAll )
	{
		if( !bFound		// ������Ȃ�����
		 && bRedo )		// �ŏ��̌���
		{
			nY = 0 - 1;	/* 1��O���w�� */
			bRedo = FALSE;
			goto re_do;		// �擪����Č���
		}
	}
	if( bFound )
	{
		if( nYOld >= nY ) SendStatusMessage( "���擪����Č������܂���" );
	}
	else
	{
		SendStatusMessage( "��������܂���ł���" );
		if( m_pShareData->m_Common.m_bNOTIFYNOTFOUND )	/* ������Ȃ��Ƃ����b�Z�[�W��\�� */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"���(��) �ɍ�����������܂���B" );
	}

	return;
}



/*!	�O�̍�����T���C����������ړ�����
*/
void CEditView::Command_Diff_Prev( void )
{
	int			nX = 0;
	int			nY;
	int			nYOld;
	BOOL		bFound = FALSE;
	BOOL		bRedo = TRUE;

	nY = m_nCaretPosY_PHY;
	nYOld = nY;

re_do:;
	if( m_pcEditDoc->m_cDocLineMgr.SearchDiffMark( nY, 0 /* �O������ */, &nY ) )
	{
		bFound = TRUE;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nX, nY, &nX, &nY );
		if( m_bSelectingLock )
		{
			if( !IsTextSelected() ) BeginSelectArea();
		}
		else
		{
			if( IsTextSelected() ) DisableSelectArea( TRUE );
		}
		MoveCursor( nX, nY, TRUE );
		if( m_bSelectingLock )
		{
			ChangeSelectAreaByCurrentCursor( nX, nY );
		}
	}

	if( m_pShareData->m_Common.m_bSearchAll )
	{
		if( !bFound	// ������Ȃ�����
		 && bRedo )	// �ŏ��̌���
		{
			nY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 + 1;	/* 1��O���w�� */
			bRedo = FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if( bFound )
	{
		if( nYOld <= nY ) SendStatusMessage( "����������Č������܂���" );
	}
	else
	{
		SendStatusMessage( "��������܂���ł���" );
		if( m_pShareData->m_Common.m_bNOTIFYNOTFOUND )	/* ������Ȃ��Ƃ����b�Z�[�W��\�� */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"�O��(��) �ɍ�����������܂���B" );
	}

	return;
}

/*!	�����\���̑S����
	@author	MIK
	@date	2002/05/26
*/
void CEditView::Command_Diff_Reset( void )
{
	m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	//���������r���[���X�V
	for( int v = 0; v < 4; ++v )
		if( m_pcEditDoc->m_nActivePaneIndex != v )
			m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}

/*!	�ꎞ�t�@�C�����쐬����
	@author	MIK
	@date	2002/05/26
*/
/*
BOOL CEditView::MakeDiffTmpFile( char* filename, HWND hWnd )
{
#if 0
	char*	pLineData;
	int		nLineLen;
	int		y;

	// �s(���s�P��)�f�[�^�̗v��
	if( hWnd )
	{
		pLineData = m_pShareData->m_szWork;
		nLineLen = ::SendMessage( hWnd, MYWM_GETLINEDATA, y, 0 );
	}
	else
	{
		pLineData = m_pcEditDoc->m_cDocLineMgr.GetLineStr( y, &nLineLenSrc );
	}

	while( 1 )
	{
		if( hWnd && nLineLen > sizeof( m_pShareData->m_szWork ) )
		{
			// �ꎞ�o�b�t�@�𒴂��Ă���
		}

		// �s(���s�P��)�f�[�^�̗v�� 
		if( hWnd ) nLineLen = ::SendMessage( hWnd, MYWM_GETLINEDATA, y, 0 );
		else       pLineData = m_pcEditDoc->m_cDocLineMgr.GetLineStr( y, &nLineLen );
		
	}
#endif

	return FALSE;
}
*/


