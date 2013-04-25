/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�^�O�W�����v)�֐��Q

	2012/12/17	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, MIK
	Copyright (C) 2003, MIK, genta, ���イ��
	Copyright (C) 2004, Moca, novice
	Copyright (C) 2005, MIK
	Copyright (C) 2006, genta
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, MIK
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "uiparts/CWaitCursor.h"
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "dlg/CDlgTagJumpList.h"
#include "dlg/CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "CEditApp.h"
#include "_os/COsVersionInfo.h"
#include "util/window.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "env/CSakuraEnvironment.h"
#include "CGrepAgent.h"
#include "sakura_rc.h"


/*! �^�O�W�����v

	@param bClose [in] true:���E�B���h�E�����

	@date 2003.04.03 genta ���E�B���h�E����邩�ǂ����̈�����ǉ�
	@date 2004.05.13 Moca �s���ʒu�̎w�肪�����ꍇ�́A�s�����ړ����Ȃ�
*/
bool CViewCommander::Command_TAGJUMP( bool bClose )
{
	//	2004.05.13 Moca �����l��1�ł͂Ȃ����̈ʒu���p������悤��
	// 0�ȉ��͖��w�舵���B(1�J�n)
	int			nJumpToLine;
	int			nJumpToColm;
	nJumpToLine = 0;
	nJumpToColm = 0;

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint ptXY;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&ptXY
	);
	/* ���ݍs�̃f�[�^���擾 */
	CLogicInt		nLineLen;
	const wchar_t*	pLine;
	pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
	if( NULL == pLine ){
		goto can_not_tagjump_end;
	}

	//�t�@�C�����o�b�t�@
	wchar_t		szJumpToFile[1024];
	int			nBgn;
	int			nPathLen;
	wmemset( szJumpToFile, 0, _countof(szJumpToFile) );

	/* WZ���̃^�O���X�g�� */
	if( 0 == wmemcmp( pLine, L"��\"", 2 ) ){
		if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
			wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
			GetLineColm( &pLine[2] + nPathLen, &nJumpToLine, &nJumpToColm );
		}
		else{
			goto can_not_tagjump;
		}
	}
	else if(0 == wmemcmp( pLine, L"�E", 1 )){
		GetLineColm( &pLine[1], &nJumpToLine, &nJumpToColm );
		ptXY.y--;

		for( ; 0 <= ptXY.y; ptXY.y-- ){
			pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
			if( NULL == pLine ){
				goto can_not_tagjump;
			}
			if( 0 == wmemcmp( pLine, L"�E", 1 ) ){
				continue;
			}
			else if( 0 == wmemcmp( pLine, L"��\"", 2 ) ){
				if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
					wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
					break;
				}
				else{
					goto can_not_tagjump;
				}
			}
			else{
				goto can_not_tagjump;
			}
		}
	}
	else{
		//@@@ 2001.12.31 YAZAKI
		const wchar_t *p = pLine;
		const wchar_t *p_end = p + nLineLen;

		//	From Here Aug. 27, 2001 genta
		//	Borland �`���̃��b�Z�[�W�����TAG JUMP
		while( p < p_end ){
			//	skip space
			for( ; p < p_end && ( *p == L' ' || *p == L'\t' || *p == L'\n' ); ++p )
				;
			if( p >= p_end )
				break;
		
			//	Check Path
			if( IsFilePath( p, &nBgn, &nPathLen ) ){
				wmemcpy( szJumpToFile, &p[nBgn], nPathLen );
				GetLineColm( &p[nBgn + nPathLen], &nJumpToLine, &nJumpToColm );
				break;
			}
			//	Jan. 04, 2001 genta Directory��ΏۊO�ɂ����̂ŕ�����ɂ͏_��ɑΉ�
			//	break;	//@@@ 2001.12.31 YAZAKI �uworking ...�v���ɑΏ�
			//	skip non-space
			for( ; p < p_end && ( *p != L' ' && *p != L'\t' ); ++p )
				;
		}
		if( szJumpToFile[0] == L'\0' ){
			if( !Command_TagJumpByTagsFile() )	//@@@ 2003.04.13
				goto can_not_tagjump;
			return true;
		}
		//	From Here Aug. 27, 2001 genta
	}

	//	Apr. 21, 2003 genta bClose�ǉ�
	if( !m_pCommanderView->TagJumpSub( to_tchar(szJumpToFile), CMyPoint(nJumpToColm, nJumpToLine), bClose ) )	//@@@ 2003.04.13
		goto can_not_tagjump;

	return true;

can_not_tagjump:;
can_not_tagjump_end:;
	m_pCommanderView->SendStatusMessage(_T("�^�O�W�����v�ł��܂���"));	//@@@ 2003.04.13
	return false;
}



/* �^�O�W�����v�o�b�N */
void CViewCommander::Command_TAGJUMPBACK( void )
{
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	TagJump tagJump;

	/* �^�O�W�����v���̎Q�� */
	if( !CTagJumpManager().PopTagJump(&tagJump) || !IsSakuraMainWindow(tagJump.hwndReferer) ){
		m_pCommanderView->SendStatusMessage(_T("�^�O�W�����v�o�b�N�ł��܂���"));
		// 2004.07.10 Moca m_TagJumpNum��0�ɂ��Ȃ��Ă������Ǝv��
		// GetDllShareData().m_TagJumpNum = 0;
		return;
	}

	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( tagJump.hwndReferer );

	/* �J�[�\�����ړ������� */
	memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &(tagJump.point), sizeof( tagJump.point ) );
	::SendMessageAny( tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0 );

	return;
}



/*
	�^�O�t�@�C�����쐬����B

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.05.12	�_�C�A���O�\���Ńt�H���_�����ׂ����w��ł���悤�ɂ����B
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
bool CViewCommander::Command_TagsMake( void )
{
#define	CTAGS_COMMAND	_T("ctags.exe")

	TCHAR	szTargetPath[1024 /*_MAX_PATH+1*/ ];
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() )
	{
		_tcscpy( szTargetPath, GetDocument()->m_cDocFile.GetFilePath() );
		szTargetPath[ _tcslen( szTargetPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');
	}
	else
	{
		// 20100722 Moca �T�N���̃t�H���_����J�����g�f�B���N�g���ɕύX
		::GetCurrentDirectory( _countof(szTargetPath), szTargetPath );
	}

	//�_�C�A���O��\������
	CDlgTagsMake	cDlgTagsMake;
	if( !cDlgTagsMake.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0, szTargetPath ) ) return false;

	TCHAR	cmdline[1024];
	/* exe�̂���t�H���_ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	GetExedir( cmdline, CTAGS_COMMAND );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//ctags.exe�̑��݃`�F�b�N
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T( "�^�O�쐬�R�}���h���s�͎��s���܂����B\n\nCTAGS.EXE ��������܂���B" ) );
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_pCommanderView->GetHwnd() );

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
		//�G���[
		return false;
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

	//	To Here Dec. 28, 2002 MIK

	TCHAR	options[1024];
	_tcscpy( options, _T("--excmd=n") );	//�f�t�H���g�̃I�v�V����
	if( cDlgTagsMake.m_nTagsOpt & 0x0001 ) _tcscat( options, _T(" -R") );	//�T�u�t�H���_���Ώ�
	if( _tcslen( cDlgTagsMake.m_szTagsCmdLine ) )	//�ʎw��̃R�}���h���C��
	{
		_tcscat( options, _T(" ") );
		_tcscat( options, cDlgTagsMake.m_szTagsCmdLine );
	}
	_tcscat( options, _T(" *") );	//�z���̂��ׂẴt�@�C��

	//�R�}���h���C��������쐬(MAX:1024)
	if (IsWin32NT())
	{
		// 2010.08.28 Moca �V�X�e���f�B���N�g���t��
		TCHAR szCmdDir[_MAX_PATH];
		::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
		//	2006.08.04 genta add /D to disable autorun
		auto_sprintf( cmdline, _T("\"%ts\\cmd.exe\" /D /C \"\"%ts\\%ts\" %ts\""),
				szCmdDir,
				szExeFolder,	//sakura.exe�p�X
				CTAGS_COMMAND,	//ctags.exe
				options			//ctags�I�v�V����
			);
	}
	else
	{
		// 2010.08.28 Moca �V�X�e���f�B���N�g���t��
		TCHAR szCmdDir[_MAX_PATH];
		::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
		auto_sprintf( cmdline, _T("\"%ts\\command.com\" /C \"%ts\\%ts\" %ts"),
				szCmdDir,
				szExeFolder,	//sakura.exe�p�X
				CTAGS_COMMAND,	//ctags.exe
				options			//ctags�I�v�V����
			);
	}

	//�R�}���h���C�����s
	BOOL bProcessResult = CreateProcess(
		NULL, cmdline, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, cDlgTagsMake.m_szPath, &sui, &pi
	);
	if( !bProcessResult )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T("�^�O�쐬�R�}���h���s�͎��s���܂����B\n\n%ts"), cmdline );
		goto finish;
	}

	{
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		bool	bLoopFlag = true;

		//���f�_�C�A���O�\��
		HWND	hwndCancel;
		HWND	hwndMsg;
		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->m_hwndParent, IDD_EXECRUNNING );
		hwndMsg = ::GetDlgItem( hwndCancel, IDC_STATIC_CMD );
		::SendMessage( hwndMsg, WM_SETTEXT, 0, (LPARAM)L"�^�O�t�@�C�����쐬���ł��B" );

		//���s���ʂ̎�荞��
		do {
			// Jun. 04, 2003 genta CPU��������炷���߂�200msec�҂�
			// ���̊ԃ��b�Z�[�W�������؂�Ȃ��悤�ɑ҂�����WaitForSingleObject����
			// MsgWaitForMultipleObject�ɕύX
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 200, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//�I�����Ă���΃��[�v�t���O��FALSE�Ƃ���
					//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//�������̃��[�U�[������\�ɂ���
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						break;
					}
					break;
				default:
					break;
			}

			//���f�{�^�������`�F�b�N
			if( cDlgCancel.IsCanceled() )
			{
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
				::TerminateProcess( pi.hProcess, 0 );
				break;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
			{
				if( new_cnt > 0 )												//�ҋ@���̂��̂�����
				{
					if( new_cnt >= _countof(work) - 2 )							//�p�C�v����ǂݏo���ʂ𒲐�
					{
						new_cnt = _countof(work) - 2;
					}
					::ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					if( read_cnt == 0 )
					{
						continue;
					}
					// 2003.11.09 ���イ��
					//	����I���̎��̓��b�Z�[�W���o�͂���Ȃ��̂�
					//	�����o�͂��ꂽ��G���[���b�Z�[�W�ƌ��Ȃ��D
					else {
						//�I������
						CloseHandle( hStdOutWrite );
						CloseHandle( hStdOutRead  );
						if( pi.hProcess ) CloseHandle( pi.hProcess );
						if( pi.hThread  ) CloseHandle( pi.hThread  );

						cDlgCancel.CloseDialog( TRUE );

						work[ read_cnt ] = L'\0';	// Nov. 15, 2003 genta �\���p��0�I�[����
						WarningMessage( m_pCommanderView->GetHwnd(), _T("�^�O�쐬�R�}���h���s�͎��s���܂����B\n\n%hs"), work ); // 2003.11.09 ���イ��

						return true;
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );

	}


finish:
	//�I������
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	cDlgCancel.CloseDialog( TRUE );

	InfoMessage(m_pCommanderView->GetHwnd(), _T("�^�O�t�@�C���̍쐬���I�����܂����B"));

	return true;
}



/*
	�_�C���N�g�^�O�W�����v(���b�Z�[�W�t)

	@date	2010.07.22	�V�K�쐬
*/
bool CViewCommander::Command_TagJumpByTagsFileMsg( bool bMsg )
{
	bool ret = Command_TagJumpByTagsFile();
	if( false == ret && bMsg ){
		m_pCommanderView->SendStatusMessage(_T("�^�O�W�����v�ł��܂���"));
	}
	return ret;
}



/*
	�_�C���N�g�^�O�W�����v

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.05.12	�t�H���_�K�w���l�����ĒT��
	@date	
*/
bool CViewCommander::Command_TagJumpByTagsFile( void )
{
	CNativeW	cmemKeyW;
	m_pCommanderView->GetCurrentTextForSearch( cmemKeyW, true, true );
	if( 0 == cmemKeyW.GetStringLength() ){
		return false;
	}
	
	TCHAR	szDirFile[1024];
	if( false == Sub_PreProcTagJumpByTagsFile( szDirFile, _countof(szDirFile) ) ){
		return false;
	}
	CDlgTagJumpList	cDlgTagJumpList(true);	//�^�O�W�����v���X�g
	
	cDlgTagJumpList.SetFileName( szDirFile );
	cDlgTagJumpList.SetKeyword(cmemKeyW.GetStringPtr());

	int nMatchAll = cDlgTagJumpList.FindDirectTagJump();

	//��������ΑI�����Ă��炤�B
	if( 1 < nMatchAll ){
		if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)0 ) ){
			nMatchAll = 0;
			return true;	//�L�����Z��
		}
	}

	//�^�O�W�����v����B
	if( 0 < nMatchAll ){
		//	@@ 2005.03.31 MIK �K�w�p�����[�^�ǉ�
		TCHAR fileName[1024];
		int   fileLine;

		if( false == cDlgTagJumpList.GetSelectedFullPathAndLine( fileName, _countof(fileName), &fileLine , NULL ) ){
			return false;
		}
		return m_pCommanderView->TagJumpSub( fileName, CMyPoint(0, fileLine) );
	}

	return false;
}



/*!
	�L�[���[�h���w�肵�ă^�O�W�����v(�_�C�A���O)
	@param keyword NULL���e
	@author MIK
	@date 2005.03.31 �V�K�쐬
	@date 2010.04.02 Moca ����ł��g����悤��
*/
bool CViewCommander::Command_TagJumpByTagsFileKeyword( const wchar_t* keyword )
{
	CDlgTagJumpList	cDlgTagJumpList(false);
	TCHAR	fileName[1024];
	int		fileLine;	// �s�ԍ�
	TCHAR	szCurrentPath[1024];

	if( false == Sub_PreProcTagJumpByTagsFile( szCurrentPath, _countof(szCurrentPath) ) ){
		return false;
	}

	cDlgTagJumpList.SetFileName( szCurrentPath );
	cDlgTagJumpList.SetKeyword( keyword );

	if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ) 
	{
		return true;	//�L�����Z��
	}

	//�^�O�W�����v����B
	if( false == cDlgTagJumpList.GetSelectedFullPathAndLine( fileName, _countof(fileName), &fileLine, NULL ) )
	{
		return false;
	}

	return m_pCommanderView->TagJumpSub( fileName, CMyPoint(0, fileLine) );
}



/*!
	�^�O�W�����v�̑O����
	���s�\�m�F�ƁA��t�@�C�����̐ݒ�
*/
bool CViewCommander::Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, int count )
{
	if( count ) szCurrentPath[0] = _T('\0');

	// ���s�\�m�F
	if( ! GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		// 2010.04.02 (����)�ł��^�O�W�����v�ł���悤��
		// Grep�A�A�E�g�v�b�g�͍s�ԍ��^�O�W�����v������̂Ŗ����ɂ���(�v����)
		if( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode ||
		    CAppMode::getInstance()->IsDebugMode() ){
		    return false;
		}
	}
	
	// ��t�@�C�����̐ݒ�
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		auto_strcpy( szCurrentPath, GetDocument()->m_cDocFile.GetFilePath() );
	}else{
		if( 0 == ::GetCurrentDirectory( count - _countof(_T("\\dmy")) - MAX_TYPES_EXTS, szCurrentPath ) ){
			return false;
		}
		// (����)�ł��t�@�C������v�����Ă���̂Ń_�~�[������
		// ���݂̃^�C�v�ʂ�1�Ԗڂ̊g���q��q��
		TCHAR szExts[MAX_TYPES_EXTS];
		TCHAR* pszExt = szExts;
		auto_strcpy( szExts, GetDocument()->m_cDocType.GetDocumentAttribute().m_szTypeExts );
		if( szExts[0] != '\0' ){
			// strtok ����
			pszExt = _tcstok( szExts, _T(" ;,") );
		}
		int nExtLen = 0;
		if( NULL != pszExt ){
			nExtLen = auto_strlen( pszExt );
		}
		_tcscat( szCurrentPath, _T("\\dmy") );
		if( nExtLen ){
			_tcscat( szCurrentPath, _T(".") );
			_tcscat( szCurrentPath, pszExt );
		}
	}
	return true;
}
