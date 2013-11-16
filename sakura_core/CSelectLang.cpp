/*!	@file
	@brief �e���ꃁ�b�Z�[�W���\�[�X�Ή�

	@author nasukoji
	@date 2011.04.10	�V�K�쐬
*/
/*
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CSelectLang.h"
#include "util/os.h"
#include "util/module.h"
#include "_os/COsVersionInfo.h"

#include <new>

CSelectLang::SELLANG_INFO* CSelectLang::m_psLangInfo = NULL;	// ���b�Z�[�W���\�[�X�p�\����
CSelectLang::PSELLANG_INFO_LIST CSelectLang::m_psLangInfoList;

/*!
	@brief �f�X�g���N�^

	@note �ǂݍ��񂾃��b�Z�[�W���\�[�XDLL���������

	@date 2011.04.10 nasukoji	�V�K�쐬
*/
CSelectLang::~CSelectLang( void )
{
	for (PSELLANG_INFO_LIST::iterator it = m_psLangInfoList.begin(); it != m_psLangInfoList.end(); it++) {
		if( (*it)->hInstance ){
			FreeLibrary( (*it)->hInstance );
			(*it)->hInstance = NULL;
		}
		delete *it;
	}
	m_psLangInfoList.clear();
}

/*!
	@brief ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h����Ԃ�

	@retval ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��

	@note ���b�Z�[�W���\�[�XDLL�����[�h���Ă��Ȃ��ꍇexe�̃C���X�^���X�n���h�����Ԃ�

	@date 2011.04.10 nasukoji	�V�K�쐬
*/
HINSTANCE CSelectLang::getLangRsrcInstance( void )
{
	return m_psLangInfo->hInstance;
}

/*!
	@brief ���b�Z�[�W���\�[�XDLL���ǂݍ��ݎ��̃f�t�H���g����̕������Ԃ�

	@retval �f�t�H���g����̕�����i"(Japanese)" �܂��� "(English(United States))"�j

	@note �A�v���P�[�V�������\�[�X���ǂݍ��� "(Japanese)" �܂��� "(English(United States))"

	@date 2011.04.10 nasukoji	�V�K�쐬
*/
LPCTSTR CSelectLang::getDefaultLangString( void )
{
	return m_psLangInfo->szLangName;
}

/*!
	@brief �����������������
	
	@retval ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��

	@note ���b�Z�[�W���\�[�XDLL�����w��A�܂��͓ǂݍ��݃G���[�����̎���exe�̃C���X�^���X�n���h�����Ԃ�
	@note �iLoadString()�̈����Ƃ��Ă��̂܂܎g�p���邽�߁j
	@note �f�t�H���g����̕�����̓ǂݍ��݂��s��
	@note �v���Z�X����CShareData����1�񂾂��Ă΂��

	@date 2011.04.10 nasukoji	�V�K�쐬
*/
HINSTANCE CSelectLang::InitializeLanguageEnvironment( void )
{
	int nCount;
	SELLANG_INFO *psLangInfo;

	if ( m_psLangInfoList.size() == 0 ) {
		// �f�t�H���g�����쐬����
		psLangInfo = new SELLANG_INFO();
		psLangInfo->hInstance = GetModuleHandle(NULL);

		// ������_�C�A���O�� "System default" �ɕ\�����镶������쐬����
		nCount = ::LoadString( GetModuleHandle(NULL), STR_SELLANG_NAME, psLangInfo->szLangName, _countof(psLangInfo->szLangName) );

		m_psLangInfoList.push_back( psLangInfo );
	}

	if( m_psLangInfo != NULL && m_psLangInfo->hInstance && m_psLangInfo->hInstance != GetModuleHandle(NULL) ){
		// �ǂݍ��ݍς݂�DLL���������
		::FreeLibrary( m_psLangInfo->hInstance );
		m_psLangInfo->hInstance = NULL;
		m_psLangInfo = NULL;
	}

	//�J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CCurrentDirectoryBackupPoint cCurDirBackup;
	ChangeCurrentDirectoryToExeDir();
// ��ini�܂���exe�t�H���_�ƂȂ�悤�ɉ������K�v

	WIN32_FIND_DATA w32fd;
	TCHAR szPath[] = _T("sakura_lang_*.dll");			// �T�[�`���郁�b�Z�[�W���\�[�XDLL
	HANDLE handle = FindFirstFile( szPath, &w32fd );
	BOOL result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;

	while( result ){
		if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){		//�t�H���_�łȂ�
			// �o�b�t�@�ɓo�^����B
			psLangInfo = new SELLANG_INFO();
			_tcscpy( psLangInfo->szDllName, w32fd.cFileName );
			psLangInfo->hInstance = CSelectLang::LoadLangRsrcLibrary( *psLangInfo );

			if( psLangInfo->hInstance ){
				if ( !psLangInfo->bValid ){
					// ���b�Z�[�W���\�[�XDLL�Ƃ��Ă͖���
					::FreeLibrary( psLangInfo->hInstance );
					psLangInfo->hInstance = NULL;
					delete psLangInfo;
				} else {
					// �L���ȃ��b�Z�[�W���\�[�XDLL
					// ��UDLL��������A���ChangeLang�ōēǂݍ��݂���
					m_psLangInfoList.push_back( psLangInfo );
					::FreeLibrary( psLangInfo->hInstance );
					psLangInfo->hInstance = NULL;
				}
			}
		}

		result = FindNextFile( handle, &w32fd );
	}

	if( INVALID_HANDLE_VALUE != handle ){
		FindClose( handle );
		handle = INVALID_HANDLE_VALUE;
	}

	// ���̎��_�ł�exe�̃C���X�^���X�n���h���ŋN�����A���L���������������ChangeLang����
	m_psLangInfo = *m_psLangInfoList.begin();

	return m_psLangInfo->hInstance;
}

/*!
	@brief ���b�Z�[�W���\�[�XDLL�����[�h����
	
	@retval ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��

	@note ���b�Z�[�W���\�[�XDLL�����w��A�܂��͓ǂݍ��݃G���[�����̎���NULL���Ԃ�

	@date 2011.04.10 nasukoji	�V�K�쐬
*/
HINSTANCE CSelectLang::LoadLangRsrcLibrary( SELLANG_INFO& lang )
{
	if( lang.szDllName[0] == _T('\0') )
		return NULL;		// DLL���w�肳��Ă��Ȃ����NULL��Ԃ�

	int nCount;

	lang.bValid  = FALSE;
	lang.szLangName[0] = _T('\0');
	lang.wLangId = 0;

	HINSTANCE hInstance = LoadLibraryExedir( lang.szDllName );

	if( hInstance ){
		// ���ꖼ���擾
		nCount = ::LoadString( hInstance, STR_SELLANG_NAME, lang.szLangName, _countof(lang.szLangName) );

		if( nCount > 0 ){
			// ����ID���擾
			TCHAR szBuf[7];		// "0x" + 4�� + �ԕ�
			nCount = ::LoadString( hInstance, STR_SELLANG_LANGID, szBuf, _countof(szBuf) );
			szBuf[_countof(szBuf) - 1] = _T('\0');

			if( nCount > 0 ){
				lang.wLangId = (WORD)_tcstoul( szBuf, NULL, 16 );		// ����ID�𐔒l��

				if( lang.wLangId > 0 )
					lang.bValid = TRUE;		// ���b�Z�[�W���\�[�XDLL�Ƃ��ėL��
			}
		}
	}

	return hInstance;
}



// �����񃊃\�[�X�ǂݍ��ݗp�O���[�o��
CLoadString::CLoadStrBuffer CLoadString::m_acLoadStrBufferTemp[];	// ������ǂݍ��݃o�b�t�@�̔z��iCLoadString::LoadStringSt() ���g�p����j
int CLoadString::m_nDataTempArrayIndex = 0;							// �Ō�Ɏg�p�����o�b�t�@�̃C���f�b�N�X�iCLoadString::LoadStringSt() ���g�p����j


/*!
	@brief �ÓI�o�b�t�@�ɕ����񃊃\�[�X��ǂݍ��ށi�e���ꃁ�b�Z�[�W���\�[�X�Ή��j

	@param[in] uid ���\�[�XID

	@retval �ǂݍ��񂾕�����i�����񖳂��̎� "" ���Ԃ�j

	@note �ÓI�o�b�t�@�im_acLoadStrBufferTemp[?]�j�ɕ����񃊃\�[�X��ǂݍ��ށB
	@note �o�b�t�@�͕����������Ă��邪�A�Ăяo�����ɍX�V����̂Ńo�b�t�@����
	@note �����ČĂяo���Ə������e�������Ă����B
	@note �Ăяo������ł̎g�p��֐��̈����Ȃǂł̎g�p��z�肵�Ă���A�O��l��
	@note ���o�����Ƃ͂ł��Ȃ��B
	@note �g�p��j::SetWindowText( m_hWnd, CLoadString::LoadStringSt(STR_ERR_DLGSMCLR1) );
	@note �A�v���P�[�V�������̊֐��ւ̈����Ƃ���ꍇ�A���̊֐����{�֐����g�p
	@note ���Ă��邩�ӎ�����K�v������i����𒴂���Γ��e���X�V����邽�߁j
	@note ���e��ێ��������ꍇ�� CLoadString::LoadString() ���g�p����B

	@date 2011.06.01 nasukoji	�V�K�쐬
*/
LPCTSTR CLoadString::LoadStringSt( UINT uid )
{
	// �g�p����o�b�t�@�̌��݈ʒu��i�߂�
	m_nDataTempArrayIndex = ( m_nDataTempArrayIndex >= _countof(m_acLoadStrBufferTemp) - 1) ?
															0 : ( m_nDataTempArrayIndex + 1 );

	m_acLoadStrBufferTemp[m_nDataTempArrayIndex].LoadString( uid );

	return /* CLoadString:: */ m_acLoadStrBufferTemp[m_nDataTempArrayIndex].GetStringPtr();
}

/*!
	@brief �����񃊃\�[�X��ǂݍ��ށi�e���ꃁ�b�Z�[�W���\�[�X�Ή��j

	@param[in] uid ���\�[�XID

	@retval �ǂݍ��񂾕�����i�����񖳂��̎� "" ���Ԃ�j

	@note �����o�ϐ����ɋL������邽��  CLoadString::LoadStringSt() �̗l��
	@note �s�p�ӂɔj�󂳂�邱�Ƃ͂Ȃ��B
	@note �������A�ϐ�����������K�v������̂��s�ցB
	@note �g�p��j
	@note   CLoadString cStr[2];
	@note   cDlgInput1.DoModal( m_hInstance, m_hWnd,
	@note       cStr[0].LoadString(STR_ERR_DLGPRNST1),
	@note       cStr[1].LoadString(STR_ERR_DLGPRNST2),
	@note       sizeof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1, szWork ) )

	@date 2011.06.01 nasukoji	�V�K�쐬
*/
LPCTSTR CLoadString::LoadString( UINT uid )
{
	m_cLoadStrBuffer.LoadString( uid );

	return /* this-> */ m_cLoadStrBuffer.GetStringPtr();
}

/*!
	@brief �����񃊃\�[�X��ǂݍ��ށi�ǂݍ��ݎ��s���j

	@param[in] uid  ���\�[�XID

	@retval �ǂݍ��񂾕������iTCHAR�P�ʁj

	@note ���b�Z�[�W���\�[�X��蕶�����ǂݍ��ށB���b�Z�[�W���\�[�XDLL�Ɏw���
	@note ���\�[�X�����݂��Ȃ��A�܂��̓��b�Z�[�W���\�[�XDLL���̂��ǂݍ��܂��
	@note ���Ȃ��ꍇ�A�������\�[�X��蕶�����ǂݍ��ށB
	@note �ŏ��͐ÓI�o�b�t�@�ɓǂݍ��ނ��o�b�t�@�s���ƂȂ�����o�b�t�@���g��
	@note ���ēǂݒ����B
	@note �擾�����o�b�t�@�̓f�X�g���N�^�ŉ������B
	@note ANSI�ł�2�o�C�g�����̓s���ɂ��i�o�b�t�@ - 2�j�o�C�g�܂ł����ǂ܂Ȃ�
	@note �ꍇ������̂�1�o�C�g���Ȃ��l�Ńo�b�t�@�g���𔻒肷��B

	@date 2011.06.01 nasukoji	�V�K�쐬
*/
int CLoadString::CLoadStrBuffer::LoadString( UINT uid )
{
	if( !m_pszString ){
		// �o�b�t�@�|�C���^���ݒ肳��Ă��Ȃ��ꍇ����������i���ʂ͂��蓾�Ȃ��j
		m_pszString = m_szString;					// �ϐ����ɏ��������o�b�t�@��ڑ�
		m_nBufferSize = _countof(m_szString);		// �z���
		m_szString[m_nBufferSize - 1] = 0;
		m_nLength = _tcslen(m_szString);			// ������
	}

	HINSTANCE hRsrc = CSelectLang::getLangRsrcInstance();		// ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��

	if( !hRsrc ){
		// ���b�Z�[�W���\�[�XDLL�Ǎ������O�͓������\�[�X���g��
		hRsrc = ::GetModuleHandle(NULL);
	}

	int nRet = 0;

	while(1){
		nRet = ::LoadString( hRsrc, uid, m_pszString, m_nBufferSize );

		// ���\�[�X������
		if( nRet == 0 ){
			if( hRsrc != ::GetModuleHandle(NULL) ){
				hRsrc = ::GetModuleHandle(NULL);	// �������\�[�X���g��
			}else{
				// �������\�[�X������ǂ߂Ȃ���������߂�i���ʂ͂��蓾�Ȃ��j
				m_pszString[0] = 0;
				break;
			}
#ifdef UNICODE
		}else if( nRet >= m_nBufferSize - 1 ){
#else
		}else if( nRet >= m_nBufferSize - 2 ){		// ANSI�ł�1�����������ōēǂݍ��݂𔻒肷��
#endif
			// �ǂ݂���Ȃ������ꍇ�A�o�b�t�@���g�����ēǂݒ���
			int nTemp = m_nBufferSize + LOADSTR_ADD_SIZE;		// �g�������T�C�Y
			LPTSTR pTemp;

			try{
				pTemp = new TCHAR[nTemp];
			}
			catch(std::bad_alloc){
				// ���������蓖�ė�O�i��O�̔���������̏ꍇ�ł������̏����ɂ���j
				pTemp = NULL;
			}

			if( pTemp ){
				if( m_pszString != m_szString ){
					delete[] m_pszString;
				}

				m_pszString = pTemp;
				m_nBufferSize = nTemp;
			}else{
				// �������擾�Ɏ��s�����ꍇ�͒��O�̓��e�Œ��߂�
				nRet = _tcslen( m_pszString );
				break;
			}
		}else{
			break;		// �����񃊃\�[�X������Ɏ擾�ł���
		}
	}

	m_nLength = nRet;	// �ǂݍ��񂾕�����

	return nRet;
}

HINSTANCE CSelectLang::ChangeLang( UINT nIndex )
{
	if ( m_psLangInfoList.size() <= nIndex || m_psLangInfoList.at( nIndex ) == m_psLangInfo ) {
		return m_psLangInfo->hInstance;
	}

	SELLANG_INFO *psLangInfo = m_psLangInfoList.at( nIndex );
	if ( psLangInfo->hInstance != GetModuleHandle(NULL) ) {
		psLangInfo->hInstance = LoadLangRsrcLibrary( *psLangInfo );
		if ( psLangInfo->hInstance == NULL ) {
			return m_psLangInfo->hInstance;
		} else if ( !psLangInfo->bValid ) {
			::FreeLibrary( psLangInfo->hInstance );
			psLangInfo->hInstance = NULL;
			return m_psLangInfo->hInstance;
		}
	}

	if ( m_psLangInfo->hInstance != GetModuleHandle(NULL) ) {
		::FreeLibrary( m_psLangInfo->hInstance );
		m_psLangInfo->hInstance = NULL;
	}
	m_psLangInfo = psLangInfo;

	// ���P�[����ݒ�
	// SetThreadUILanguage�̌Ăяo�������݂�
	bool isSuccess = false;
	if( COsVersionInfo()._IsWinVista_or_later() ) {
		HMODULE hDll = LoadLibrary( _T("kernel32") );
		if ( hDll ) {
			typedef short (CALLBACK* SetThreadUILanguageType)(LANGID);
			SetThreadUILanguageType _SetThreadUILanguage = (SetThreadUILanguageType)
					GetProcAddress(hDll, "SetThreadUILanguage");
			isSuccess = _SetThreadUILanguage && _SetThreadUILanguage( m_psLangInfo->wLangId );
			FreeLibrary( hDll );
		}
	}
	if ( !isSuccess ) {
		SetThreadLocale(MAKELCID( m_psLangInfo->wLangId, SORT_DEFAULT ));
	}

	return m_psLangInfo->hInstance;
}



/*[EOF]*/
