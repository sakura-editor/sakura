/*!	@file
	@brief DIFF�����\��

	@author MIK
	@date	2002/05/25 ExecCmd ���Q�l��DIFF���s���ʂ���荞�ޏ����쐬
 	@date	2005/10/29	maru Diff�����\�������𕪗����A�_�C�A���O����ŁE�_�C�A���O�Ȃ��ł̗�������R�[��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, GAE, YAZAKI, hor
	Copyright (C) 2002, hor, MIK
	Copyright (C) 2003, MIK, ryoji, genta
	Copyright (C) 2004, genta
	Copyright (C) 2005, maru
	Copyright (C) 2007, ryoji, kobake
	Copyright (C) 2008, kobake

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "sakura_rc.h"
#include "global.h"
#include "dlg/CDlgDiff.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "doc/CDocLine.h"
#include "doc/CDocLineMgr.h"
#include "CWaitCursor.h"
#include "COsVersionInfo.h"
#include "debug/Debug.h"
#include "util/module.h"
#include "util/file.h"
#include "window/CEditWnd.h"
#include "io/CTextStream.h"
#include "CWriteManager.h"

#define	SAKURA_DIFF_TEMP_PREFIX	_T("sakura_diff_")

/*!	�����\��
	@param	pszFile1	[in]	���t�@�C����
	@param	pszFile2	[in]	����t�@�C����
    @param  nFlgOpt     [in]    0b000000000
                                    ||||||+--- -i ignore-case         �啶�����������ꎋ
                                    |||||+---- -w ignore-all-space    �󔒖���
                                    ||||+----- -b ignore-space-change �󔒕ύX����
                                    |||+------ -B ignore-blank-lines  ��s����
                                    ||+------- -t expand-tabs         TAB-SPACE�ϊ�
                                    |+--------    (�ҏW���̃t�@�C�������t�@�C��)
                                    +---------    (DIFF�������Ȃ��Ƃ��Ƀ��b�Z�[�W�\��)
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	MIK
	@date	2002/05/25
	@date	2005/10/28	��Command_Diff����֐����̕ύX�B
						GetCommander().Command_Diff_Dialog�����łȂ��VCommand_Diff
						������Ă΂��֐��Bmaru
*/
void CEditView::ViewDiffInfo( 
	const TCHAR*	pszFile1,
	const TCHAR*	pszFile2,
	int				nFlgOpt
)
/*
	bool	bFlgCase,		//�啶�����������ꎋ
	bool	bFlgBlank,		//�󔒖���
	bool	bFlgWhite,		//�󔒕ύX����
	bool	bFlgBLine,		//��s����
	bool	bFlgTabSpc,		//TAB-SPACE�ϊ�
	bool	bFlgFile12,		//�ҏW���̃t�@�C�������t�@�C��
*/
{
	HANDLE	hStdOutWrite, hStdOutRead;

	CWaitCursor	cWaitCursor( this->GetHwnd() );
	int		nFlgFile12 = 1;

	/* exe�̂���t�H���_ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	TCHAR	cmdline[1024];
	GetExedir( cmdline, _T("diff.exe") );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//	From Here Dec. 28, 2002 MIK
	//	diff.exe�̑��݃`�F�b�N
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		WarningMessage( GetHwnd(), _T( "�����R�}���h���s�͎��s���܂����B\n\nDIFF.EXE ��������܂���B" ) );
		return;
	}

	//������DIFF��������������B
	if( CDiffManager::Instance()->IsDiffUse() )
		GetCommander().Command_Diff_Reset();
		//m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(sa) );
	sa.nLength              = sizeof(sa);
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
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb          = sizeof(sui);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//�I�v�V�������쐬����
	TCHAR	szOption[16];	// "-cwbBt"
	_tcscpy( szOption, _T("-") );
	if( nFlgOpt & 0x0001 ) _tcscat( szOption, _T("i") );	//-i ignore-case         �啶�����������ꎋ
	if( nFlgOpt & 0x0002 ) _tcscat( szOption, _T("w") );	//-w ignore-all-space    �󔒖���
	if( nFlgOpt & 0x0004 ) _tcscat( szOption, _T("b") );	//-b ignore-space-change �󔒕ύX����
	if( nFlgOpt & 0x0008 ) _tcscat( szOption, _T("B") );	//-B ignore-blank-lines  ��s����
	if( nFlgOpt & 0x0010 ) _tcscat( szOption, _T("t") );	//-t expand-tabs         TAB-SPACE�ϊ�
	if( _tcscmp( szOption, _T("-") ) == 0 ) _tcscpy( szOption, _T("") );	//�I�v�V�����Ȃ�
	if( nFlgOpt & 0x0020 ) nFlgFile12 = 0;
	else                   nFlgFile12 = 1;

	//	To Here Dec. 28, 2002 MIK

	//OS�o�[�W�����擾
	{
		COsVersionInfo cOsVer;
		//�R�}���h���C��������쐬(MAX:1024)
		if (cOsVer.IsWin32NT()){
			auto_sprintf(
				cmdline,
				_T("cmd.exe /C \"\"%ts\\%ts\" %ts \"%ts\" \"%ts\"\""),
				szExeFolder,	//sakura.exe�p�X
				_T("diff.exe"),		//diff.exe
				szOption,		//diff�I�v�V����
				( nFlgFile12 ? pszFile2 : pszFile1 ),
				( nFlgFile12 ? pszFile1 : pszFile2 )
			);
		}
		else{
			auto_sprintf(
				cmdline,
				_T("command.com /C \"%ts\\%ts\" %ts \"%ts\" \"%ts\""),
				szExeFolder,	//sakura.exe�p�X
				_T("diff.exe"),		//diff.exe
				szOption,		//diff�I�v�V����
				( nFlgFile12 ? pszFile2 : pszFile1 ),
				( nFlgFile12 ? pszFile1 : pszFile2 )
			);
		}
	}

	//�R�}���h���C�����s
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE )
	{
			WarningMessage( NULL, _T("�����R�}���h���s�͎��s���܂����B\n\n%ls"), cmdline );
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
		bool	bFirst = true;	//�擪���H	//@@@ 2003.05.31 MIK

		//���f�_�C�A���O�\��
//		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );

		//���s���ʂ̎�荞��
		do {
			//�v���Z�X���I�����Ă��Ȃ����m�F
			// Jul. 04, 2003 genta CPU��100%�g���ʂ����̂�h������ 200msec�x��
			// Jan. 23, 2004 genta
			// �q�v���Z�X�̏o�͂��ǂ�ǂ�󂯎��Ȃ��Ǝq�v���Z�X��
			// ��~���Ă��܂����߁C�҂����Ԃ�200ms����20ms�Ɍ��炷
			if( WaitForSingleObject( pi.hProcess, 20 ) == WAIT_OBJECT_0 )
			{
				//�I�����Ă���΃��[�v�t���O��FALSE�Ƃ���
				//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
				bLoopFlag = FALSE;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )
			{
				while( new_cnt > 0 )												//�ҋ@���̂��̂�����
				{
					if( new_cnt >= _countof(work) - 2 )							//�p�C�v����ǂݏo���ʂ𒲐�
					{
						new_cnt = _countof(work) - 2;
					}
					::ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					if( read_cnt == 0 )
					{
						// Jan. 23, 2004 genta while�ǉ��̂��ߐ����ύX
						break;
					}

					//@@@ 2003.05.31 MIK
					//	�擪��Binary files�Ȃ�o�C�i���t�@�C���̂��߈Ӗ��̂��鍷��������Ȃ�����
					if( bFirst )
					{
						bFirst = false;
						if( strncmp( work, "Binary files ", strlen( "Binary files " ) ) == 0 )
						{
							WarningMessage( NULL, _T("DIFF�������s�����Ƃ����t�@�C���̓o�C�i���t�@�C���ł��B") );
							goto finish;
						}
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
					// Jan. 23, 2004 genta
					// �q�v���Z�X�̏o�͂��ǂ�ǂ�󂯎��Ȃ��Ǝq�v���Z�X��
					// ��~���Ă��܂����߁C�o�b�t�@����ɂȂ�܂łǂ�ǂ�ǂݏo���D
					new_cnt = 0;
					if( ! PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ){
						break;
					}
					Sleep(0); // Jan. 23, 2004 genta �^�X�N�X�C�b�`�𑣂�
				}
			}
		} while( bLoopFlag || new_cnt > 0 );

		//�c����DIFF��񂪂���Ή�͂���
		if( bDiffInfo == true && nDiffLen > 0 )
		{
			szDiffData[nDiffLen] = '\0';
			AnalyzeDiffInfo( szDiffData, nFlgFile12 );
		}
	}


	//DIFF������������Ȃ������Ƃ��Ƀ��b�Z�[�W�\��
	if( nFlgOpt & 0x0040 )
	{
		if( !CDiffManager::Instance()->IsDiffUse() )
		{
			InfoMessage( this->GetHwnd(), _T("DIFF�����͌�����܂���ł����B") );
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
		if( m_pcEditWnd->m_nActivePaneIndex != v )
			m_pcEditWnd->m_pcEditViewArr[v]->Redraw();
	Redraw();

	return;
}

/*!	DIFF����������͂��}�[�N�o�^
	@param	pszDiffInfo	[in]	�V�t�@�C����
	@param	nFlgFile12	[in]	�ҏW���t�@�C����...
									0	�t�@�C��1(���t�@�C��)
									1	�t�@�C��2(�V�t�@�C��)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::AnalyzeDiffInfo( 
	const char*	pszDiffInfo,
	int			nFlgFile12
)
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
	const char	*q;
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
		if     ( mode == 'a' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s1    ), CLogicInt(e1    ) );
		else if( mode == 'c' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
		else if( mode == 'd' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
	}
	else	//�ҏW���t�@�C���͐V�t�@�C��
	{
		if     ( mode == 'a' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'c' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'd' ) CDiffLineMgr(&m_pcEditDoc->m_cDocLineMgr).SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s2    ), CLogicInt(e2    ) );
	}

	return;
}

/*!	�ꎞ�t�@�C�����쐬����
	@author	MIK
	@date	2002/05/26
	@date	2005/10/29	�����ύXconst char* �� char*
						�ꎞ�t�@�C�����̎擾�����������ł����Ȃ��Bmaru
	@date	2007/08/??	kobake �@�B�I��UNICODE��
	@date	2008/01/26	kobake �o�͌`���C��
*/
BOOL CEditView::MakeDiffTmpFile( TCHAR* filename, HWND hWnd )
{
	//�ꎞ
	TCHAR* pszTmpName = _ttempnam( NULL, SAKURA_DIFF_TEMP_PREFIX );
	if( NULL == pszTmpName ){
		WarningMessage( NULL, _T("�����R�}���h���s�͎��s���܂����B") );
		return FALSE;
	}

	_tcscpy( filename, pszTmpName );
	free( pszTmpName );

	//�������H
	if( NULL == hWnd )
	{
		EConvertResult eWriteResult = CWriteManager().WriteFile_From_CDocLineMgr(
			m_pcEditDoc->m_cDocLineMgr,
			SSaveInfo(
				filename,
				m_pcEditDoc->GetDocumentEncoding(),
				EOL_NONE,
				m_pcEditDoc->m_cDocFile.IsBomExist()
			)
		);
		return RESULT_FAILURE != eWriteResult;
	}

	CTextOutputStream out(filename, CODE_SJIS);
	if(!out){
		WarningMessage( NULL, _T("�����R�}���h���s�͎��s���܂����B\n\n�ꎞ�t�@�C�����쐬�ł��܂���B") );
		return FALSE;
	}

	CLogicInt y = CLogicInt(0);

	while(1){
		// �s(���s�P��)�f�[�^�̗v�� 
		const wchar_t*	pLineData;
		CLogicInt		nLineLen;
		if( hWnd ){
			pLineData = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
			nLineLen = CLogicInt(::SendMessageAny( hWnd, MYWM_GETLINEDATA, y, 0 ));

			// �ꎞ�o�b�t�@�𒴂���ꍇ�̓G���[�I��
			if( nLineLen > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
				out.Close();
				_tunlink( filename );	//�֐��̎��s�Ɏ��s�����Ƃ��A�ꎞ�t�@�C���̍폜�͊֐����ōs���B2005.10.29
				WarningMessage( NULL, _T("�����R�}���h���s�͎��s���܂����B\n\n�s���������܂��B") );
				return FALSE;
			}
		}
		else{
			pLineData = m_pcEditDoc->m_cDocLineMgr.GetLine(y)->GetDocLineStrWithEOL(&nLineLen);
		}

		if( 0 == nLineLen || NULL == pLineData ) break;

		try{
			out.WriteString(pLineData,nLineLen);
		}
		catch(...){
			out.Close();
			_tunlink( filename );	//�֐��̎��s�Ɏ��s�����Ƃ��A�ꎞ�t�@�C���̍폜�͊֐����ōs���B2005.10.29
			WarningMessage( NULL, _T("�����R�}���h���s�͎��s���܂����B\n\n�ꎞ�t�@�C�����쐬�ł��܂���B") );
		}

		y++;
	}

	//fclose( fp );

	return TRUE;
}



