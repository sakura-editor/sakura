#include "stdafx.h"
#include "CEditView.h"
#include "dlg/CDlgCancel.h"
#include "util/window.h"
#include "util/tchar_template.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �O���R�}���h                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief	�O���R�}���h�̎��s

	@param[in] pszCmd �R�}���h���C��
	@param[in] nFlgOpt �I�v�V����
		@li	0x01	�W���o�͂𓾂�
		@li	0x02	�W���o�͂̂�_�C���N�g��i����=�A�E�g�v�b�g�E�B���h�E / �L��=�ҏW���̃E�B���h�E�j
		@li	0x04	�ҏW���t�@�C����W�����͂�
		@li	0x08	�W���o�͂�Unicode�ōs��
		@li	0x10	�W�����͂�Unicode�ōs��

	@note	�q�v���Z�X�̕W���o�͎擾�̓p�C�v���g�p����
	@note	�q�v���Z�X�̕W�����͂ւ̑��M�͈ꎞ�t�@�C�����g�p

	@author	N.Nakatani
	@date	2001/06/23
	@date	2001/06/30	GAE
	@date	2002/01/24	YAZAKI	1�o�C�g��肱�ڂ��\����������
	@date	2003/06/04	genta
	@date	2004/09/20	naoh	�����͌��₷���E�E�E
	@date	2004/01/23	genta
	@date	2004/01/28	Moca	���s�R�[�h�����������̂�h��
	@date	2007/03/18	maru	�I�v�V�����̊g��
	@date	2008/06/07	Uchi	Unidoe�̎g�p
	@date	2009/02/21	ryoji	�r���[���[�h��㏑���֎~�̂Ƃ��͕ҏW���E�B���h�E�ւ͏o�͂��Ȃ��i�w�莞�̓A�E�g�v�b�g�ցj
*/
void CEditView::ExecCmd( const TCHAR* pszCmd, int nFlgOpt )
{
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );
	CDlgCancel				cDlgCancel;

	BOOL bEditable = ( !CAppMode::Instance()->IsViewMode() && m_pcEditDoc->m_cDocLocker.IsDocWritable() );

	//	From Here 2006.12.03 maru �������g���̂���
	BOOL	bGetStdout		= nFlgOpt & 0x01 ? TRUE : FALSE;	//	�q�v���Z�X�̕W���o�͂𓾂�
	BOOL	bToEditWindow	= ((nFlgOpt & 0x02) && bEditable) ? TRUE : FALSE;	//	TRUE=�ҏW���̃E�B���h�E / FALSAE=�A�E�g�v�b�g�E�B���h�E
	BOOL	bSendStdin		= nFlgOpt & 0x04 ? TRUE : FALSE;	//	�ҏW���t�@�C�����q�v���Z�XSTDIN�ɓn��
	BOOL	bIOUnicodeGet	= nFlgOpt & 0x08 ? TRUE : FALSE;	//	�W���o�͂�Unicode�ōs��	2008/6/17 Uchi
	BOOL	bIOUnicodeSend	= nFlgOpt & 0x10 ? TRUE : FALSE;	//	�W�����͂�Unicode�ōs��	2008/6/20 Uchi
	//	To Here 2006.12.03 maru �������g���̂���

	// �ҏW���̃E�B���h�E�ɏo�͂���ꍇ�̑I��͈͏����p	/* 2007.04.29 maru */
	CLayoutInt	nLineFrom, nColmFrom;
	bool bBeforeTextSelected = GetSelectionInfo().IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom = this->GetSelectionInfo().m_sSelect.GetFrom().y; //m_nSelectLineFrom;
		nColmFrom = this->GetSelectionInfo().m_sSelect.GetFrom().x; //m_nSelectColmFrom;
	}

	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE ) {
		//�G���[�B�΍�����
		return;
	}
	//hStdOutRead�̂ق��͎q�v���Z�X�ł͎g�p����Ȃ��̂Ōp���s�\�ɂ���i�q�v���Z�X�̃��\�[�X�𖳑ʂɑ��₳�Ȃ��j
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), &hStdOutRead,					// �V�����p���s�\�n���h�����󂯎��	// 2007.01.31 ryoji
				0, FALSE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS );	// ���̌p���\�n���h���� DUPLICATE_CLOSE_SOURCE �ŕ���	// 2007.01.31 ryoji


	// From Here 2007.03.18 maru �q�v���Z�X�̕W�����̓n���h��
	// CDocLineMgr::WriteFile�ȂǊ����̃t�@�C���o�͌n�̊֐��̂Ȃ��ɂ�
	// �t�@�C���n���h����Ԃ��^�C�v�̂��̂��Ȃ��̂ŁA��U�����o���Ă���
	// �ꎞ�t�@�C�������ŃI�[�v�����邱�ƂɁB
	hStdIn = NULL;
	if(bSendStdin){	/* ���ݕҏW���̃t�@�C�����q�v���Z�X�̕W�����͂� */
		TCHAR		szPathName[MAX_PATH];
		TCHAR		szTempFileName[MAX_PATH];
		int			nFlgOpt;

		GetTempPath( MAX_PATH, szPathName );
		GetTempFileName( szPathName, TEXT("skr_"), 0, szTempFileName );
		DBPRINT( _T("CEditView::ExecCmd() TempFilename=[%ts]\n"), szTempFileName );
		
		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* �I��͈͂��o�� */
		
		if( !GetCommander().Command_PUTFILE( to_wchar(szTempFileName), bIOUnicodeSend? CODE_UNICODE : CODE_SJIS, nFlgOpt) ){	// �ꎞ�t�@�C���o��
			hStdIn = NULL;
		} else {
			// �q�v���Z�X�ւ̌p���p�Ƀt�@�C�����J��
			hStdIn = CreateFile(
				szTempFileName,
				GENERIC_READ,
				0,
				&sa,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
				NULL
			);
			if(hStdIn == INVALID_HANDLE_VALUE) hStdIn = NULL;
		}
	}
	
	if (hStdIn == NULL) {	/* �W�����͂𐧌䂵�Ȃ��ꍇ�A�܂��͈ꎞ�t�@�C���̐����Ɏ��s�����ꍇ */
		bSendStdin = FALSE;
		hStdIn = GetStdHandle( STD_INPUT_HANDLE );
	}
	// To Here 2007.03.18 maru �q�v���Z�X�̕W�����̓n���h��
	

	//CreateProcess�ɓn��STARTUPINFO���쐬
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb = sizeof(sui);
	if( bGetStdout || bSendStdin ) {
		sui.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sui.wShowWindow = bGetStdout ? SW_HIDE : SW_SHOW;
		sui.hStdInput = hStdIn;
		sui.hStdOutput = bGetStdout ? hStdOutWrite : GetStdHandle( STD_OUTPUT_HANDLE );
		sui.hStdError = bGetStdout ? hStdOutWrite : GetStdHandle( STD_ERROR_HANDLE );
	}

	//�R�}���h���C�����s
	TCHAR	cmdline[1024];
	_tcscpy( cmdline, pszCmd );
	//OS�o�[�W�����擾
	COsVersionInfo cOsVer;		// move to	2008/6/7 Uchi
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//���s�Ɏ��s�����ꍇ�A�R�}���h���C���x�[�X�̃A�v���P�[�V�����Ɣ��f����
		// command(9x) �� cmd(NT) ���Ăяo��

		//OS�o�[�W�����擾
		//COsVersionInfo cOsVer;		// move from	2008/6/7 Uchi
		//�R�}���h���C��������쐬
		auto_sprintf(
			cmdline,
			_T("%ts %ts%ts%ts"),
			( cOsVer.IsWin32NT() ? _T("cmd.exe") : _T("command.com") ),
			( bIOUnicodeGet ? _T("/U") : _T("") ),		// Unicde���[�h�ŃR�}���h���s	2008/6/17 Uchi
			( bGetStdout ? _T("/C ") : _T("/K ") ),
			pszCmd
		);
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
			MessageBox( NULL, cmdline, _T("�R�}���h���s�͎��s���܂����B"), MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
	}

	// �t�@�C���S�̂ɑ΂���t�B���^����
	//	���ݕҏW���̃t�@�C������̃f�[�^������������уf�[�^��荞�݂�
	//	�w�肳��Ă��āC���͈͑I�����s���Ă��Ȃ��ꍇ��
	//	�u���ׂđI���v����Ă�����̂Ƃ��āC�ҏW�f�[�^�S�̂�
	//	�R�}���h�̏o�͌��ʂƒu��������D
	//	2007.05.20 maru
	if(!bBeforeTextSelected && bSendStdin && bGetStdout && bToEditWindow){
		GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				CLayoutPoint(CLayoutInt(0), CLayoutInt(0)),
				CLayoutPoint(CLayoutInt(0), m_pcEditDoc->m_cLayoutMgr.GetLineCount())
			)
		);
		DeleteData( TRUE );
	}

	// hStdOutWrite �� CreateProcess() �Ōp�������̂Őe�v���Z�X�ł͗p�ς�
	// hStdIn���e�v���Z�X�ł͎g�p���Ȃ����AWin9x�n�ł͎q�v���Z�X���I�����Ă���
	// �N���[�Y����悤�ɂ��Ȃ��ƈꎞ�t�@�C���������폜����Ȃ�
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta ��dclose��h��

	if( bGetStdout ) {
		DWORD	read_cnt;
		DWORD	new_cnt;
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//���f�_�C�A���O�\��
		cDlgCancel.DoModeless( G_AppInstance(), m_hwndParent, IDD_EXECRUNNING );
		//���s�����R�}���h���C����\��
		// 2004.09.20 naoh �����͌��₷���E�E�E
		if (FALSE==bToEditWindow)	//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
		{
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CFormatManager().MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CFormatManager().MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			CShareData::getInstance()->TraceOut( _T("\r\n%ts\r\n"), _T("#============================================================") );
			CShareData::getInstance()->TraceOut( _T("#DateTime : %ts %ts\r\n"), szTextDate, szTextTime );
			CShareData::getInstance()->TraceOut( _T("#CmdLine  : %ts\r\n"), pszCmd );
			CShareData::getInstance()->TraceOut( _T("#%ts\r\n"), _T("============================================================") );
		}
		
		//char�œǂ�
		typedef char PIPE_CHAR;
		PIPE_CHAR work[1024];
		
		//���s���ʂ̎�荞��
		do {
			//�v���Z�X���I�����Ă��Ȃ����m�F
			// Jun. 04, 2003 genta CPU��������炷���߂�200msec�҂�
			// ���̊ԃ��b�Z�[�W�������؂�Ȃ��悤�ɑ҂�����WaitForSingleObject����
			// MsgWaitForMultipleObject�ɕύX
			// Jan. 23, 2004 genta
			// �q�v���Z�X�̏o�͂��ǂ�ǂ�󂯎��Ȃ��Ǝq�v���Z�X��
			// ��~���Ă��܂����߁C�҂����Ԃ�200ms����20ms�Ɍ��炷
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 20, QS_ALLEVENTS )){
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
			if( cDlgCancel.IsCanceled() ){
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
				::TerminateProcess( pi.hProcess, 0 );
				if (!bToEditWindow) {	//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
					//�Ō�Ƀe�L�X�g��ǉ�
					CShareData::getInstance()->TraceOut( _T("%ts"), _T("\r\n���f���܂����B\r\n") );
				}
				break;
			}
			new_cnt = 0;

			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
				while( new_cnt > 0 ) {												//�ҋ@���̂��̂�����

					if( new_cnt >= _countof(work)-2 - bufidx) {							//�p�C�v����ǂݏo���ʂ𒲐�
						new_cnt = _countof(work)-2 - bufidx;
					}
					::ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					read_cnt += bufidx;													//work���̎��ۂ̃T�C�Y�ɂ���

					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while�ǉ��̂��ߐ����ύX
						break;
					}
					// Unicode �� �f�[�^���󂯎�� start 2008/6/8 Uchi
					if (bIOUnicodeGet) {
						wchar_t*	workw;
						int			read_cntw;
						bool		bCarry;
						workw = (wchar_t*)work;
						read_cntw = (int)read_cnt/sizeof(wchar_t);
						workw[read_cntw] = '\0';
						bCarry = false;
						//�ǂݏo������������`�F�b�N����
						if (workw[read_cntw-1] == L'\r') {
							bCarry = true;
							read_cntw -= sizeof(wchar_t);
							workw[read_cntw] = '\0';
						}
						if (FALSE==bToEditWindow) {
							CShareData::getInstance()->TraceOut( _T("%s"), workw );
						} else {
							GetCommander().Command_INSTEXT(FALSE, workw, CLogicInt(-1), TRUE);
						}
						bufidx = 0;
						if (bCarry) {
							workw[0] = L'r';
							bufidx = sizeof(wchar_t);
							DBPRINT_A( "ExecCmd: Carry last character [CR]\n");
						}
					}
					// end 2008/6/8 Uchi
					else {
						//�ǂݏo������������`�F�b�N����
						// \r\n �� \r �����Ƃ������̑��o�C�g�������o�͂���̂�h���K�v������
						//@@@ 2002.1.24 YAZAKI 1�o�C�g��肱�ڂ��\�����������B
						//	Jan. 28, 2004 Moca �Ō�̕����͂��ƂŃ`�F�b�N����
						for( j=0; j<(int)read_cnt - 1; j++ ) {
							//	2007.09.10 ryoji
							if( CNativeA::GetSizeOfChar(work, read_cnt, j) == 2 ) {
								j++;
							} else {
								if( work[j] == _T2(PIPE_CHAR,'\r') && work[j+1] == _T2(PIPE_CHAR,'\n') ) {
									j++;
								} else if( work[j] == _T2(PIPE_CHAR,'\n') && work[j+1] == _T2(PIPE_CHAR,'\r') ) {
									j++;
								}
							}
						}
						//	From Here Jan. 28, 2004 Moca
						//	���s�R�[�h�����������̂�h��
						if( j == read_cnt - 1 ){
							if( _IS_SJIS_1(work[j]) ) {
								j = read_cnt + 1; // �҂�����o�͂ł��Ȃ����Ƃ��咣
							}else if( work[j] == _T2(PIPE_CHAR,'\r') || work[j] == _T2(PIPE_CHAR,'\n') ) {
								// CRLF�̈ꕔ�ł͂Ȃ����s�������ɂ���
								// ���̓ǂݍ��݂ŁACRLF�̈ꕔ�ɂȂ�\��������
								j = read_cnt + 1;
							}else{
								j = read_cnt;
							}
						}
						//	To Here Jan. 28, 2004 Moca
						if( j == (int)read_cnt ) {	//�҂�����o�͂ł���ꍇ
							//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
							if (FALSE==bToEditWindow) {
								work[read_cnt] = '\0';
								CShareData::getInstance()->TraceOut( _T("%hs"), work );
							} else {
								GetCommander().Command_INSTEXT(FALSE, to_wchar(work,read_cnt), CLogicInt(-1), TRUE);
							}
							bufidx = 0;
						}
						else {
							char tmp = work[read_cnt-1];
							//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
							if (FALSE==bToEditWindow) {
								work[read_cnt-1] = '\0';
								CShareData::getInstance()->TraceOut( _T("%hs"), work );
							} else {
								GetCommander().Command_INSTEXT(FALSE, to_wchar(work,read_cnt-1), CLogicInt(-1), TRUE);
							}
							work[0] = tmp;
							bufidx = 1;
							DBPRINT_A( "ExecCmd: Carry last character [%d]\n", tmp );
						}
					}
					// Jan. 23, 2004 genta
					// �q�v���Z�X�̏o�͂��ǂ�ǂ�󂯎��Ȃ��Ǝq�v���Z�X��
					// ��~���Ă��܂����߁C�o�b�t�@����ɂȂ�܂łǂ�ǂ�ǂݏo���D
					new_cnt = 0;
					if( ! PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ){
						break;
					}
					Sleep(0);
				}
			}
		} while( bLoopFlag || new_cnt > 0 );
		
		if (!bToEditWindow) {	//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
			work[bufidx] = '\0';
			CShareData::getInstance()->TraceOut( _T("%hs"), work );	/* �Ō�̕����̏��� */
			//	Jun. 04, 2003 genta	�I���R�[�h�̎擾�Əo��
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			CShareData::getInstance()->TraceOut( _T("\r\n�I���R�[�h: %d\r\n"), result );

			// 2004.09.20 naoh �I���R�[�h��1�ȏ�̎��̓A�E�g�v�b�g���A�N�e�B�u�ɂ���
			if(result > 0) ActivateFrameWindow( GetDllShareData().m_sHandles.m_hwndDebug );
		}
		else {						//	2006.12.03 maru �ҏW���̃E�B���h�E�ɏo�͎��͍Ō�ɍĕ`��
			GetCommander().Command_INSTEXT(FALSE, to_wchar(work,bufidx), CLogicInt(-1), TRUE);	/* �Ō�̕����̏��� */
			if (bBeforeTextSelected){	// �}�����ꂽ������I����Ԃ�
				GetSelectionInfo().SetSelectArea(
					CLayoutRange(
						CLayoutPoint(nColmFrom, nLineFrom),
						GetCaret().GetCaretLayoutPos()// CLayoutPoint(m_nCaretPosY, m_nCaretPosX )
					)
				);
				GetSelectionInfo().DrawSelectArea();
			}
			RedrawAll();
		}
	}


finish:
	//�I������
	if(bSendStdin) CloseHandle( hStdIn );	/* 2007.03.18 maru �W�����͂̐���̂��� */
	if(hStdOutWrite) CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread ) CloseHandle( pi.hThread );
}

