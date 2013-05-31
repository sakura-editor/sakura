#include "StdAfx.h"
#include "CEditView.h"
#include "_main/CAppMode.h"
#include "_os/COsVersionInfo.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFormatManager.h"
#include "dlg/CDlgCancel.h"
#include "charset/CCodeFactory.h"
#include "charset/CUtf8.h"
#include "util/window.h"
#include "util/tchar_template.h"
#include "sakura_rc.h" // IDD_EXECRUNNING

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �O���R�}���h                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class OutputAdapter
{
public:
	OutputAdapter(CEditView* view, BOOL bToEditWindow) : m_view(view),m_bWindow(bToEditWindow)
		,pcCodeBase(CCodeFactory::CreateCodeBase(CODE_UTF8,0))
	{
		m_pCShareData = CShareData::getInstance();
		m_pCommander  = &(view->GetCommander());
	}

	void OutputW(const WCHAR* pBuf, int size = -1);
	void OutputA(const ACHAR* pBuf, int size = -1);
	void OutputUTF8(const ACHAR* pBuf, int size = -1);
	void Output(const WCHAR* pBuf, int size = -1){ OutputW(pBuf, size); }
	void Output(const ACHAR* pBuf, int size = -1){ OutputA(pBuf, size); }

private:
	BOOL m_bWindow;
	CEditView* m_view;
	CShareData* m_pCShareData;
	CViewCommander* m_pCommander;
	std::auto_ptr<CCodeBase> pcCodeBase;
};

/*!	@brief	�O���R�}���h�̎��s

	@param[in] pszCmd �R�}���h���C��
	@param[in] nFlgOpt �I�v�V����
		@li	0x01	�W���o�͂𓾂�
		@li	0x02	�W���o�͂̃��_�C���N�g��i����=�A�E�g�v�b�g�E�B���h�E / �L��=�ҏW���̃E�B���h�E�j
		@li	0x04	�ҏW���t�@�C����W�����͂�
		@li	0x08	�W���o�͂�Unicode�ōs��
		@li	0x10	�W�����͂�Unicode�ōs��
		@li	0x20	���o�͂���
		@li	0x40	���o�͂��Ȃ�
		@li	0x80	�W���o�͂�UTF-8�ōs��
		@li	0x100	�W�����͂�UTF-8�ōs��
		@li	0x200	�J�����g�f�B���N�g�����w��

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
	@date	2010/04/12	Moca	nFlgOpt��0x20,0x40�ǉ��B�����o�͑΍�BWM_QUIT�΍�BUnicode��Carry����̏C��

	TODO:	�W�����́E�W���G���[�̎捞�I���B�J�����g�f�B���N�g���BUTF-8���ւ̑Ή�
*/
void CEditView::ExecCmd( const TCHAR* pszCmd, int nFlgOpt, const TCHAR* pszCurDir )
{
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );
	CDlgCancel				cDlgCancel;

	bool bEditable = m_pcEditDoc->IsEditable();

	//	From Here 2006.12.03 maru �������g���̂���
	BOOL	bGetStdout		= nFlgOpt & 0x01 ? TRUE : FALSE;	//	�q�v���Z�X�̕W���o�͂𓾂�
	BOOL	bToEditWindow	= ((nFlgOpt & 0x02) && bEditable) ? TRUE : FALSE;	//	TRUE=�ҏW���̃E�B���h�E / FALSAE=�A�E�g�v�b�g�E�B���h�E
	BOOL	bSendStdin		= nFlgOpt & 0x04 ? TRUE : FALSE;	//	�ҏW���t�@�C�����q�v���Z�XSTDIN�ɓn��
	// BOOL	bIOUnicodeGet	= nFlgOpt & 0x08 ? TRUE : FALSE;	//	�W���o�͂�Unicode�ōs��	2008/6/17 Uchi
	// BOOL	bIOUnicodeSend	= nFlgOpt & 0x10 ? TRUE : FALSE;	//	�W�����͂�Unicode�ōs��	2008/6/20 Uchi
	ECodeType outputEncoding;
	if( nFlgOpt & 0x08 ){
		outputEncoding = CODE_UNICODE;
	}else if( nFlgOpt & 0x80 ){
		outputEncoding = CODE_UTF8;
	}else{
		outputEncoding = CODE_SJIS;
	}
	ECodeType sendEncoding;
	if( nFlgOpt & 0x10 ){
		sendEncoding = CODE_UNICODE;
	}else if( nFlgOpt & 0x100 ){
		sendEncoding = CODE_UTF8;
	}else{
		sendEncoding = CODE_SJIS;
	}
	//	To Here 2006.12.03 maru �������g���̂���
	// 2010.04.12 Moca ���o��
	BOOL	bOutputExtInfo	= !bToEditWindow;
	if( nFlgOpt & 0x20 ) bOutputExtInfo = TRUE;
	if( nFlgOpt & 0x40 ) bOutputExtInfo = FALSE;
	bool	bCurDir = (nFlgOpt & 0x200) == 0x200;

	// �ҏW���̃E�B���h�E�ɏo�͂���ꍇ�̑I��͈͏����p	/* 2007.04.29 maru */
	CLayoutInt	nLineFrom(0), nColumnFrom(0);
	bool bBeforeTextSelected = GetSelectionInfo().IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom   = this->GetSelectionInfo().m_sSelect.GetFrom().y; //m_nSelectLineFrom;
		nColumnFrom = this->GetSelectionInfo().m_sSelect.GetFrom().x; //m_nSelectColumnFrom;
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
		DEBUG_TRACE( _T("CEditView::ExecCmd() TempFilename=[%ts]\n"), szTempFileName );

		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* �I��͈͂��o�� */

		if( !GetCommander().Command_PUTFILE( to_wchar(szTempFileName), sendEncoding, nFlgOpt) ){	// �ꎞ�t�@�C���o��
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
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, bCurDir ? pszCurDir : NULL, &sui, &pi ) == FALSE ) {
		//���s�Ɏ��s�����ꍇ�A�R�}���h���C���x�[�X�̃A�v���P�[�V�����Ɣ��f����
		// command(9x) �� cmd(NT) ���Ăяo��

		// 2010.08.27 Moca �V�X�e���f�B���N�g���t��
		TCHAR szCmdDir[_MAX_PATH];
		if( IsWin32NT() ){
			::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
		}else{
			::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
		}

		//�R�}���h���C��������쐬
		auto_sprintf(
			cmdline,
			_T("\"%ts\\%ts\" %ts%ts%ts"),
			szCmdDir,
			( IsWin32NT() ? _T("cmd.exe") : _T("command.com") ),
			( outputEncoding == CODE_UNICODE ? _T("/U") : _T("") ),		// Unicde���[�h�ŃR�}���h���s	2008/6/17 Uchi
			( bGetStdout ? _T("/C ") : _T("/K ") ),
			pszCmd
		);
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, bCurDir ? pszCurDir : NULL, &sui, &pi ) == FALSE ) {
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
		DeleteData( true );
	}

	// hStdOutWrite �� CreateProcess() �Ōp�������̂Őe�v���Z�X�ł͗p�ς�
	// hStdIn���e�v���Z�X�ł͎g�p���Ȃ����AWin9x�n�ł͎q�v���Z�X���I�����Ă���
	// �N���[�Y����悤�ɂ��Ȃ��ƈꎞ�t�@�C���������폜����Ȃ�
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta ��dclose��h��

	if( bGetStdout ) {
		DWORD	new_cnt;
		int		bufidx = 0;
		bool	bLoopFlag = true;
		bool	bCancelEnd = false; // �L�����Z���Ńv���Z�X��~
		OutputAdapter oa(this, bToEditWindow );

		//���f�_�C�A���O�\��
		cDlgCancel.DoModeless( G_AppInstance(), m_hwndParent, IDD_EXECRUNNING );
		// �_�C�A���O�ɃR�}���h��\��
		::DlgItem_SetText( cDlgCancel.GetHwnd(), IDC_STATIC_CMD, pszCmd );
		//���s�����R�}���h���C����\��
		// 2004.09.20 naoh �����͌��₷���E�E�E
		// 2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
		if (bOutputExtInfo)
		{
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CFormatManager().MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CFormatManager().MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			WCHAR szOutTemp[1024*2+100];
			oa.OutputW( L"\r\n" );
			oa.OutputW( L"#============================================================\r\n" );
			int len = auto_snprintf_s( szOutTemp, _countof(szOutTemp),
				L"#DateTime : %ts %ts\r\n", szTextDate, szTextTime );
			oa.OutputW( szOutTemp, len );
			len = auto_snprintf_s( szOutTemp, _countof(szOutTemp),
				L"#CmdLine  : %ts\r\n", pszCmd );
			oa.OutputW( szOutTemp, len );
			oa.OutputW( L"#============================================================\r\n" );
		}
		
		//char�œǂ�
		typedef char PIPE_CHAR;
		const int WORK_NULL_TERMS = sizeof(wchar_t); // �o�͗p\0�̕�
		const int MAX_BUFIDX = 10; // bufidx�̕�
		const int MAX_WORK_READ = 1024*5; // 5KiB ReadFile�œǂݍ��ތ��E�l
		// 2010.04.13 Moca �o�b�t�@�T�C�Y�̒��� 1022 Byte �ǂݎ��� 5KiB�ɕύX
		// �{�g���l�b�N�̓A�E�g�v�b�g�E�B���h�E�ւ̓]��
		// ����̃v���O������VC9��stdout�ł̓f�t�H���g��4096�BVC6,VC8��WinXP��type�R�}���h�ł�1024
		// �e�L�X�g���[�h���� new_cnt�����s��\r�������������������̐ݒ�l��葽����x�ɑ����Ă���
		// 4KB���� 4096 -> 100 -> 4096 -> 100 �̂悤�ɓǂݎ�邱�ƂɂȂ�̂�5KB�ɂ���
		PIPE_CHAR work[MAX_WORK_READ + MAX_BUFIDX + WORK_NULL_TERMS];
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
					//�I�����Ă���΃��[�v�t���O��false�Ƃ���
					//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
					bLoopFlag = false;
					break;
				case WAIT_OBJECT_0 + 1:
					//�������̃��[�U�[������\�ɂ���
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						// WM_QUIT��M�B�������ɏI������
						::TerminateProcess( pi.hProcess, 0 );
						goto finish;
					}
					break;
				default:
					break;
			}
			//���f�{�^�������`�F�b�N
			if( cDlgCancel.IsCanceled() ){
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
				::TerminateProcess( pi.hProcess, 0 );
				bCancelEnd  = true;
				break;
			}
			new_cnt = 0;

			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
				while( new_cnt > 0 ) {												//�ҋ@���̂��̂�����

					if( new_cnt > MAX_WORK_READ) {							//�p�C�v����ǂݏo���ʂ𒲐�
						new_cnt = MAX_WORK_READ;
					}
					DWORD	read_cnt = 0;
					::ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					read_cnt += bufidx;													//work���̎��ۂ̃T�C�Y�ɂ���

					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while�ǉ��̂��ߐ����ύX
						break;
					}
					// Unicode �� �f�[�^���󂯎�� start 2008/6/8 Uchi
					if( outputEncoding == CODE_UNICODE ){
						wchar_t*	workw;
						int			read_cntw;
						bool		bCarry;
						char		byteCarry = 0;
						workw = (wchar_t*)work;
						read_cntw = (int)read_cnt/sizeof(wchar_t);
						if( read_cnt % (int)sizeof(wchar_t) ){
							byteCarry = work[read_cnt-1];
						}
						if(read_cntw){
							workw[read_cntw] = L'\0';
							bCarry = false;
							//�ǂݏo������������`�F�b�N����
							if (workw[read_cntw-1] == L'\r') {
								bCarry = true;
								read_cntw -= 1; // 2010.04.12 1�����]���ɏ�����Ă�
								workw[read_cntw] = L'\0';
							}
							oa.OutputW( workw, read_cntw );
							bufidx = 0;
							if (bCarry) {
								workw[0] = L'\r'; // 2010.04.12 'r' -> '\r'
								bufidx = sizeof(wchar_t);
								DEBUG_TRACE( _T("ExecCmd: Carry last character [CR]\n") );
							}
						}
						if( read_cnt % (int)sizeof(wchar_t) ){
							// ���m����0���Ǝv����1���ƍ���
							DEBUG_TRACE( _T("ExecCmd: Carry Unicode 1byte [%x]\n"), byteCarry );
							work[bufidx] = byteCarry;
							bufidx += 1;
						}
					}
					// end 2008/6/8 Uchi
					else if (outputEncoding == CODE_SJIS) {
						//�ǂݏo������������`�F�b�N����
						// \r\n �� \r �����Ƃ������̑��o�C�g�������o�͂���̂�h���K�v������
						//@@@ 2002.1.24 YAZAKI 1�o�C�g��肱�ڂ��\�����������B
						//	Jan. 28, 2004 Moca �Ō�̕����͂��ƂŃ`�F�b�N����
						int		j;
						for( j=0; j<(int)read_cnt - 1; j++ ) {
							//	2007.09.10 ryoji
							if( CNativeA::GetSizeOfChar(work, read_cnt, j) == 2 ) {
								j++;
							} else {
								if( work[j] == _T2(PIPE_CHAR,'\r') && work[j+1] == _T2(PIPE_CHAR,'\n') ) {
									j++;
								}
							}
						}
						//	From Here Jan. 28, 2004 Moca
						//	���s�R�[�h�����������̂�h��
						if( j == read_cnt - 1 ){
							if( _IS_SJIS_1(work[j]) ) {
								j = read_cnt + 1; // �҂�����o�͂ł��Ȃ����Ƃ��咣
							}else if( work[j] == _T2(PIPE_CHAR,'\r') ) {
								// CRLF�̈ꕔ�ł͂Ȃ����s�������ɂ���
								// ���̓ǂݍ��݂ŁACRLF�̈ꕔ�ɂȂ�\��������
								j = read_cnt + 1;
							}else{
								j = read_cnt;
							}
						}
						//	To Here Jan. 28, 2004 Moca
						if( j == (int)read_cnt ) {	//�҂�����o�͂ł���ꍇ
							work[read_cnt] = '\0';
							//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
							oa.OutputA( work, read_cnt );
							bufidx = 0;
						}
						else {
							char tmp = work[read_cnt-1];
							work[read_cnt-1] = '\0';
							//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
							oa.OutputA( work, read_cnt-1 );
							work[0] = tmp;
							bufidx = 1;
							DEBUG_TRACE( _T("ExecCmd: Carry last character [%x]\n"), tmp );
						}
					}
					else if (outputEncoding == CODE_UTF8) {
						int		j;
						int checklen = 0;
						for( j = 0; j < (int)read_cnt;){
							ECharSet echarset;
							checklen = CheckUtf8Char2(work + j , read_cnt - j, &echarset, true, 0);
							if( echarset == CHARSET_BINARY2 ){
								break;
							}else if( read_cnt - 1 == j && work[j] == _T2(PIPE_CHAR,'\r') ){
								// CRLF�̈ꕔ�ł͂Ȃ����s�������ɂ���
								// ���̓ǂݍ��݂ŁACRLF�̈ꕔ�ɂȂ�\��������
								break;
							}else{
								j += checklen;
							}
						}
						if( j == (int)read_cnt ) {	//�҂�����o�͂ł���ꍇ
							work[read_cnt] = '\0';
							//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
							oa.OutputUTF8(work, read_cnt);
							bufidx = 0;
						}
						else {
							DEBUG_TRACE(_T("read_cnt %d j %d\n"), read_cnt, j);
							char tmp[5];
							int len = read_cnt - j;
							memcpy(tmp, &work[j], len);
							work[j] = '\0';
							//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�Eor�ҏW���̃E�B���h�E����ǉ�
							oa.OutputUTF8(work, j);
							memcpy(work, tmp, len);
							bufidx = len;
							DEBUG_TRACE(_T("ExecCmd: Carry last character [%x]\n"), tmp[0]);
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
					
					// 2010.04.12 Moca ���肪�o�͂��Â��Ă���Ǝ~�߂��Ȃ�����
					// BlockingHook�ƃL�����Z���m�F��ǎ惋�[�v���ł��s��
					// bLoopFlag �������Ă��Ȃ��Ƃ��́A���łɃv���Z�X�͏I�����Ă��邩��Terminate���Ȃ�
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						if( bLoopFlag ){
							::TerminateProcess( pi.hProcess, 0 );
						}
						goto finish;
					}
					if( cDlgCancel.IsCanceled() ){
						//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
						if( bLoopFlag ){
							::TerminateProcess( pi.hProcess, 0 );
						}
						bCancelEnd = true;
						goto user_cancel;
					}
				}
			}
		} while( bLoopFlag || new_cnt > 0 );

user_cancel:

		// �Ō�̕����̏o��(�����Ă�CR)
		if( 0 < bufidx ){
			if( outputEncoding == CODE_UNICODE ){
				if( bufidx % (int)sizeof(wchar_t) ){
					DEBUG_TRACE( _T("ExecCmd: Carry last Unicode byte [%x]\n"), work[bufidx-1] );
					// UTF-16�Ȃ̂Ɋ�o�C�g������
					work[bufidx] = 0x00; // ��ʃo�C�g��0�ɂ��Ă��܂���
					bufidx += 1;
				}
				wchar_t* workw = (wchar_t*)work;
				int bufidxw = bufidx / (int)sizeof(wchar_t);
				workw[bufidxw] = L'\0';
				oa.OutputW( workw, bufidxw );
			}else if( outputEncoding == CODE_SJIS ) {
				work[bufidx] = '\0';
				oa.OutputA( work, bufidx );
			}else if( outputEncoding == CODE_UTF8 ) {
				work[bufidx] = '\0';
				oa.OutputUTF8( work, bufidx );
			}
		}

		if( bCancelEnd && bOutputExtInfo ){
			//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
			//�Ō�Ƀe�L�X�g��ǉ�
			oa.OutputW( L"\r\n���f���܂����B\r\n" );
		}
		
		{
			//	2006.12.03 maru �A�E�g�v�b�g�E�B���h�E�ɂ̂ݏo��
			//	Jun. 04, 2003 genta	�I���R�[�h�̎擾�Əo��
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			if( bOutputExtInfo ){
				WCHAR endCode[128];
				auto_sprintf( endCode, L"\r\n�I���R�[�h: %d\r\n", result );
				oa.OutputW( endCode );
			}
			// 2004.09.20 naoh �I���R�[�h��1�ȏ�̎��̓A�E�g�v�b�g���A�N�e�B�u�ɂ���
			if(!bToEditWindow && result > 0) ActivateFrameWindow( GetDllShareData().m_sHandles.m_hwndDebug );
		}
		if (bToEditWindow) {
			if (bBeforeTextSelected){	// �}�����ꂽ������I����Ԃ�
				GetSelectionInfo().SetSelectArea(
					CLayoutRange(
						CLayoutPoint(nColumnFrom, nLineFrom),
						GetCaret().GetCaretLayoutPos()// CLayoutPoint(m_nCaretPosY, m_nCaretPosX )
					)
				);
				GetSelectionInfo().DrawSelectArea();
			}
			//	2006.12.03 maru �ҏW���̃E�B���h�E�ɏo�͎��͍Ō�ɍĕ`��
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

/*!
	@param pBuf size���w��Ȃ�vNUL�I�[
	@param size WCHAR�P�� 
*/
void OutputAdapter::OutputW(const WCHAR* pBuf, int size)
{
	if( m_bWindow ){
		m_pCommander->Command_INSTEXT( false, pBuf, CLogicInt(size), TRUE);
	}else{
		m_pCShareData->TraceOutString( pBuf , size );
	}
}

/*
	@param pBuf size���w��Ȃ�vNUL�I�[
	@param size ACHAR�P�� 
*/
void OutputAdapter::OutputA(const ACHAR* pBuf, int size)
{
	if( m_bWindow ){
		CNativeW buf;
		if( -1 == size ){
			buf.SetStringOld(pBuf);
		}else{
			buf.SetStringOld(pBuf,size);
		}
		m_pCommander->Command_INSTEXT( false, buf.GetStringPtr(), buf.GetStringLength(), TRUE);
	}else{
		// TraceOutString����ANSI�ł�������ق�������
		CNativeW buf;
		if( -1 == size ){
			buf.SetStringOld(pBuf);
		}else{
			buf.SetStringOld(pBuf,size);
		}
		m_pCShareData->TraceOutString( buf.GetStringPtr(), (int)buf.GetStringLength() );
	}
}

/*
	@param pBuf size���w��Ȃ�vNUL�I�[
	@param size ACHAR�P�� 
*/
void OutputAdapter::OutputUTF8(const ACHAR* pBuf, int size)
{
	CMemory input;
	CNativeW buf;
	if( -1 == size ){
		input.SetRawData(pBuf, strlen(pBuf));
	}else{
		input.SetRawData(pBuf, size);
	}
	pcCodeBase->CodeToUnicode(input, &buf);
	if( m_bWindow ){
		m_pCommander->Command_INSTEXT(FALSE, buf.GetStringPtr(), buf.GetStringLength(), TRUE);
	}else{
		m_pCShareData->TraceOutString( buf.GetStringPtr(), (int)buf.GetStringLength() );
	}
}
