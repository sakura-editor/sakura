/*!	@file
	@brief �R���g���[���v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcess��蕪��, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CControlProcess.h"
#include "CControlTray.h"
#include "env/DLLSHAREDATA.h"
#include "CCommandLine.h"
#include "env/CShareData_IO.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka �w�b�_����

#include "_os\ProcessEntryIterator.h"

#include <Psapi.h>
#if _WIN32_WINNT < _WIN32_WINNT_WIN7
	// �����J�w��
	#if PSAPI_VERSION < 2
	#pragma comment(lib, "Psapi.lib")
	#endif
#endif

//-------------------------------------------------


/*!
	@brief �R���g���[���v���Z�X������������
	
	MutexCP���쐬�E���b�N����B
	CControlTray���쐬����B
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI ���L������������������̂�CProcess�Ɉړ��B
	@date 2006/04/10 ryoji �����������C�x���g�̏�����ǉ��A�ُ펞�̌�n���̓f�X�g���N�^�ɔC����
	@date 2013.03.20 novice �R���g���[���v���Z�X�̃J�����g�f�B���N�g�����V�X�e���f�B���N�g���ɕύX
*/
bool CControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::InitializeProcess" );

	// �A�v���P�[�V�������s���o�p(�C���X�g�[���Ŏg�p)
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA );
	if( NULL == m_hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}

	std::tstring strProfileName = to_tchar(CCommandLine::getInstance()->GetProfileName());

	// �����������C�x���g���쐬����
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if( NULL == m_hEventCPInitialized )
	{
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateEvent()���s�B\n�I�����܂��B") );
		return false;
	}

	/* �R���g���[���v���Z�X�̖ڈ� */
	std::tstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += strProfileName;
	m_hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if( NULL == m_hMutexCP ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}
	
	/* ���L�������������� */
	if( !CProcess::InitializeProcess() ){
		return false;
	}

	// �R���g���[���v���Z�X�̃J�����g�f�B���N�g�����V�X�e���f�B���N�g���ɕύX
	TCHAR szDir[_MAX_PATH];
	::GetSystemDirectory( szDir, _countof(szDir) );
	::SetCurrentDirectory( szDir );

	/* ���L�f�[�^�̃��[�h */
	// 2007.05.19 ryoji �u�ݒ��ۑ����ďI������v�I�v�V���������isakuext�A�g�p�j��ǉ�
	TCHAR szIniFile[_MAX_PATH];
	CShareData_IO::LoadShareData();
	CFileNameManager::getInstance()->GetIniFileName( szIniFile, strProfileName.c_str() );	// �o��ini�t�@�C����
	if( !fexist(szIniFile) || CCommandLine::getInstance()->IsWriteQuit() ){
		/* ���W�X�g������ �쐬 */
		CShareData_IO::SaveShareData();
		if( CCommandLine::getInstance()->IsWriteQuit() ){
			return false;
		}
	}

	// �풓���Ȃ��ݒ�̏ꍇ
	if (!GetDllShareData().m_Common.m_sGeneral.m_bStayTaskTray) {
		// ��s���ċN�������G�f�B�^�v���Z�X����������
		if (!IsEditorProcess(strProfileName)) {
			return false;	// �G�f�B�^�v���Z�X��������Ȃ���ΏI��
		}
	}

	/* �����I������ */
	CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );
	RefreshString();

	MY_TRACETIME( cRunningTimer, "Before new CControlTray" );

	/* �^�X�N�g���C�ɃA�C�R���쐬 */
	m_pcTray = new CControlTray;

	MY_TRACETIME( cRunningTimer, "After new CControlTray" );

	HWND hwnd = m_pcTray->Create( GetProcessInstance() );
	if( !hwnd ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX3) );
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_sHandles.m_hwndTray = hwnd;

	// �����������C�x���g���V�O�i����Ԃɂ���
	if( !::SetEvent( m_hEventCPInitialized ) ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX4) );
		return false;
	}

	return true;
}

/*!
	@brief �R���g���[���v���Z�X�̃��b�Z�[�W���[�v
	
	@author aroka
	@date 2002/01/07
*/
bool CControlProcess::MainLoop()
{
	if( m_pcTray && GetMainWindow() ){
		m_pcTray->MessageLoop();	/* ���b�Z�[�W���[�v */
		return true;
	}
	return false;
}

/*!
	@brief �R���g���[���v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji ���L�f�[�^�ۑ��� CControlTray �ֈړ�
*/
void CControlProcess::OnExitProcess()
{
	GetDllShareData().m_sHandles.m_hwndTray = NULL;
}

CControlProcess::~CControlProcess()
{
	delete m_pcTray;

	if( m_hEventCPInitialized ){
		::ResetEvent( m_hEventCPInitialized );
	}
	::CloseHandle( m_hEventCPInitialized );
	if( m_hMutexCP ){
		::ReleaseMutex( m_hMutexCP );
	}
	::CloseHandle( m_hMutexCP );
	// ���o�[�W�����i1.2.104.1�ȑO�j�Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	if( m_hMutex ){
		::ReleaseMutex( m_hMutex );
	}
	::CloseHandle( m_hMutex );
};

//���v���Z�X�̃R�}���h���C�����擾����
std::wstring getCommandLine(_In_ HANDLE hProcess);

/*!
 * @brief �R���g���[���v���Z�X���N�������G�f�B�^�v���Z�X���擾����
 *
 * @param [in] profileName �v���t�@�C����
 * @date 2017/06/25 berryzplus		�V�K�쐬
 */
bool CControlProcess::IsEditorProcess(const std::wstring &profileName) const
{
	// ���v���Z�X�̃t���p�X���擾����
	TCHAR szMyPath[MAX_PATH];
	::GetModuleFileName(NULL, szMyPath, _countof(szMyPath) - 1);

	// ���v���Z�X�̃t�@�C�������擾����
	TCHAR szMyFilename[MAX_PATH];
	::SplitPath_FolderAndFile(szMyPath, NULL, szMyFilename);

	// ���v���Z�X�̃v���Z�XID���擾����
	DWORD myProcessId = ::GetCurrentProcessId();

	// �G�f�B�^�v���Z�X�����t���邩�A�S�v���Z�X�̃`�F�b�N���I���܂Ń��[�v
	bool editorExists = false;
	ProcessEntryIterator processIter(myProcessId);
	ProcessEntryIterator processEnd;
	for (; processIter != processEnd; ++processIter) {
		// �v���Z�X�����擾
		PROCESSENTRY32 &pe = *processIter;

		// ���v���Z�X�̓X�L�b�v
		if (myProcessId == pe.th32ProcessID)continue;

		// �t�@�C�������قȂ���̂̓X�L�b�v
		if (szMyFilename[0] != pe.szExeFile[0] || 0 != _tcsicmp(szMyFilename, pe.szExeFile)) continue;

		// �v���Z�XID���g���ăv���Z�X�n���h�����J���B
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);

		// �v���Z�X���J���Ȃ���΃X�L�b�v(�����炭�����G���[)
		if (!hProcess) continue;

		// �v���Z�X�̃t���p�X���擾
		TCHAR szFullname[MAX_PATH];
		if (!::GetModuleFileNameEx(hProcess, NULL, szFullname, _countof(szFullname) - 1)) {
			::CloseHandle(hProcess);
			continue;
		}

		// �p�X����v���Ȃ���΃X�L�b�v
		if (0 != _tcsicmp(szMyPath, szFullname))
		{
			::CloseHandle(hProcess);
			continue;
		}

		// �R�}���h���C�����擾
		std::wstring cmdline;
		try {
			cmdline = getCommandLine(hProcess);
			::CloseHandle(hProcess);
		}
		catch (const std::exception& /* ex */) {
			::CloseHandle(hProcess);
			return false;
		}

		// �R�}���h���C�������
		CCommandLine cmdlineObj;
		cmdlineObj.ParseCommandLine(cmdline.c_str(), false);

		// �v���t�@�C���w�肪��v
		if (0 == _tcsicmp(profileName.c_str(), cmdlineObj.GetProfileName())) {
			return true; // �G�f�B�^�v���Z�X�����t�����I
		}
	}
	return false; // �G�f�B�^�v���Z�X�͌�����Ȃ�����
}

#include "_os\NtQueryProcessInformation.h"


//���v���Z�X�̐��R�}���h���C�����擾����
std::wstring getRawCommandLine(_In_ HANDLE hProcess);

//���v���Z�X�̃R�}���h���C�����擾����
std::wstring getCommandLine(_In_ HANDLE hProcess)
{
	// CommandLine
	std::wstring rawCmdline(getRawCommandLine(hProcess));

	// WinMain��MinGW�����R�[�h����p�N���Ă����R�[�h
	LPCWSTR pszCommandLine = rawCmdline.c_str();
	{
		// ���s�t�@�C�������X�L�b�v����
		if (_T('\"') == *pszCommandLine) {
			pszCommandLine++;
			while (_T('\"') != *pszCommandLine && _T('\0') != *pszCommandLine) {
				pszCommandLine++;
			}
			if (_T('\"') == *pszCommandLine) {
				pszCommandLine++;
			}
		}
		else {
			while (_T(' ') != *pszCommandLine && _T('\t') != *pszCommandLine
				&& _T('\0') != *pszCommandLine) {
				pszCommandLine++;
			}
		}
		// ���̃g�[�N���܂Ői�߂�
		while (_T(' ') == *pszCommandLine || _T('\t') == *pszCommandLine) {
			pszCommandLine++;
		}
	}
	return std::wstring(pszCommandLine);
}


//���v���Z�X�̊��u���b�N���擾����
void readProcessEnvironmentBlock(_In_ HANDLE hProcess, _Out_ PPEB ppeb);

//���v���Z�X�̃v���Z�X�p�����[�^���擾����
void readUserProcessParameters(_In_ HANDLE hProcess, _Out_ PRTL_USER_PROCESS_PARAMETERS pupp);

//���v���Z�X�̐��R�}���h���C�����擾����
std::wstring getRawCommandLine(_In_ HANDLE hProcess)
{
	// ���v���Z�X�̃R�}���h���C���ɃA�N�Z�X����ɂ́A
	// �v���Z�X�̊��u���b�N(PEB)�ɂ���p�����[�^���Q�Ƃ���K�v������B

	// User Process Parameters
	RTL_USER_PROCESS_PARAMETERS upp;

	// read User Process Parameters
	readUserProcessParameters(hProcess, &upp);

	if (!upp.CommandLine.Length)
	{
		throw std::exception("upp.CommandLine is empty.");
	}

	// CommandLine
	std::wstring rawCmdline(upp.CommandLine.Length, '\0');

	// Try to read CommandLine
	SIZE_T cbRead = 0;
	if (!ReadProcessMemory(hProcess, upp.CommandLine.Buffer, &*rawCmdline.begin(), upp.CommandLine.Length, &cbRead))
	{
		throw std::exception("failed to ReadProcessMemory for CommandLine.");
	}

	return std::move(rawCmdline);
}


//���v���Z�X�̃v���Z�X�p�����[�^���擾����
void readUserProcessParameters(_In_ HANDLE hProcess, _Out_ PRTL_USER_PROCESS_PARAMETERS pupp)
{
	// Process Environment Block(PEB)
	PEB peb;

	// read Process Environment Block (PEB)
	readProcessEnvironmentBlock(hProcess, &peb);

	// try to read User Process Parameters
	SIZE_T cbRead = 0;
	if (!ReadProcessMemory(hProcess, peb.ProcessParameters, pupp, sizeof(pupp), &cbRead))
	{
		throw std::exception("failed to ReadProcessMemory for USER_PROCESS_PARAMETERS.");
	}
}


//���v���Z�X�̊��u���b�N���擾����
void readProcessEnvironmentBlock(_In_ HANDLE hProcess, _Out_ PPEB ppeb)
{
	NtQueryInformationProcessT NtQueryInformationProcess;

	std::unique_ptr<BYTE> pbiBuf(new BYTE[sizeof(PROCESS_BASIC_INFORMATION)]);
	for (DWORD size = sizeof(PROCESS_BASIC_INFORMATION);;) {
		ULONG sizeNeeded = 0;
		NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, (PPROCESS_BASIC_INFORMATION) &*pbiBuf, size, &sizeNeeded);
		if (status)
		{
			if (size < sizeNeeded) {
				size = sizeNeeded;
				pbiBuf = std::unique_ptr<BYTE>(new BYTE[size]);
				continue;
			}
			throw std::exception("failed NtQueryInformationProcess.");
		}
		break;
	}

	PPROCESS_BASIC_INFORMATION pbi = (PPROCESS_BASIC_INFORMATION) &*pbiBuf;
	if (pbi->PebBaseAddress == NULL)
	{
		throw std::exception("pbi->PebBaseAddress is NULL.");
	}

	// try to read Process Environment Block (PEB)
	SIZE_T cbRead = 0;
	if (!ReadProcessMemory(hProcess, pbi->PebBaseAddress, ppeb, sizeof(*ppeb), &cbRead))
	{
		throw std::exception("failed to ReadProcessMemory for PEB.");
	}
}
