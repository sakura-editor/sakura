/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta, MIK
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, ai, Moca, MIK, YAZAKI, hor, KK, aroka
	Copyright (C) 2003, Moca, aroka, MIK, genta, wmlhq, sui
	Copyright (C) 2004, Moca, novice, genta, isearch, MIK
	Copyright (C) 2005, Moca, MIK, genta, ryoji, ���, aroka
	Copyright (C) 2006, aroka, ryoji, genta
	Copyright (C) 2007, ryoji, genta, maru

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CShareData.h"
#include "CControlTray.h"
#include "mymessage.h"
#include "debug/Debug.h"
#include "global.h"
#include "debug/CRunningTimer.h"
#include "charcode.h"
#include <tchar.h>
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/file.h"
#include "util/os.h"

struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

//!	���L�������̃o�[�W����
/*!
	���L�������̃o�[�W�����ԍ��B���L�������̌`����ύX�����Ƃ��͂�����1���₷�B

	���̒l�͋��L�������̃o�[�W�����t�B�[���h�Ɋi�[����A�قȂ�\���̋��L��������
	�g���G�f�B�^�������ɋN�����Ȃ��悤�ɂ���B

	�ݒ�\�Ȓl�� 1�`unsinged int�̍ő�l

	@sa Init()

	Version 1�`24:
	���낢��
	
	Version 25:
	m_bStopsBothEndsWhenSearchWord�ǉ�
	
	Version 26:
	MacroRec�ɁAm_bReloadWhenExecute��ǉ� 2002/03/11 YAZAKI
	EditInfo�ɁAm_szDocType�ǉ� Mar. 7, 2002 genta
	
	Version 27:
	STypeConfig�ɁAm_szOutlineRuleFilename��ǉ� 2002.04.01 YAZAKI 
	
	Version 28:
	PRINTSETTING�ɁAm_bPrintKinsokuHead�Am_bPrintKinsokuTail��ǉ� 2002.04.09 MIK
	STypeConfig�ɁAm_bKinsokuHead�Am_bKinsokuTail�Am_szKinsokuHead�Am_szKinsokuTail��ǉ� 2002.04.09 MIK

	Version 29:
	PRINTSETTING�ɁAm_bPrintKinsokuRet��ǉ� 2002.04.13 MIK
	STypeConfig�ɁAm_bKinsokuRet��ǉ� 2002.04.13 MIK

	Version 30:
	PRINTSETTING�ɁAm_bPrintKinsokuKuto��ǉ� 2002.04.17 MIK
	STypeConfig�ɁAm_bKinsokuKuto��ǉ� 2002.04.17 MIK

	Version 31:
	Common�ɁAm_bStopsBothEndsWhenSearchParagraph��ǉ� 2002/04/26 YAZAKI

	Version 32:
	Common����STypeConfig�ցAm_bAutoIndent�Am_bAutoIndent_ZENSPACE���ړ� 2002/04/30 YAZAKI

	Version 33:
	Common�ɁAm_lf_kh��ǉ� 2002/05/21 ai
	m_nDiffFlgOpt��ǉ� 2002.05.27 MIK
	STypeConfig-Color��COLORIDX_DIFF_APPEND,COLORIDX_DIFF_CHANGE,COLORIDX_DIFF_DELETE��ǉ�

	Version 34:
	STypeConfig��m_bUseDocumentIcon �ǉ��D 2002.09.10 genta
	
	Version 35:
	Common��m_nLineNumRightSpace �ǉ��D2002.09.18 genta

	Version 36:
	Common��m_bGrepKanjiCode_AutoDetect���폜�Am_nGrepCharSet��ǉ� 2002/09/21 Moca

	Version 37:
	STypeConfig��LineComment�֘A��m_cLineComment�ɕύX�D  @@@ 2002.09.23 YAZAKI
	STypeConfig��BlockComment�֘A��m_cBlockComment�ɕύX�D@@@ 2002.09.23 YAZAKI

	Version 38:
	STypeConfig��m_bIndentLayout��ǉ�. @@@ 2002.09.29 YAZAKI
	2002.10.01 genta m_nIndentLayout�ɖ��O�ύX

	Version 39:
	Common��m_nFUNCKEYWND_GroupNum��ǉ��D 2002/11/04 Moca

	Version 40:
	�t�@�C�����ȈՕ\�L�֘A��ǉ��D 2002/12/08�`2003/01/15 Moca

	Version 41:
	STypeConfig��m_szTabViewString�T�C�Y�g��
	m_nWindowSizeX/Y m_nWindowOriginX/Y�ǉ� 2003.01.26 aroka

	Version 42:
	STypeConfig�ɓƎ�TAB�}�[�N�t���O�ǉ� 2003.03.28 MIK

	Version 43:
	�ŋߎg�����t�@�C���E�t�H���_�ɂ��C�ɓ����ǉ� 2003.04.08 MIK

	Version 44:
	Window Caption������̈��Common�ɒǉ� 2003.04.05 genta

	Version 45:
	�^�O�t�@�C���쐬�p�R�}���h�I�v�V�����ۑ��̈�(m_nTagsOpt,m_szTagsCmdLine)��ǉ� 2003.05.12 MIK

	Version 46:
	�ҏW�E�C���h�E���C���A�^�u�E�C���h�E�p���ǉ�

	Version 47:
	�t�@�C������̕⊮��STypeConfig�ɒǉ� 2003.06.28 Moca

	Version 48:
	Grep���A���^�C���\���ǉ� 2003.06.28 Moca

	Version 49:
	�t�@�C������IsDebug�ǉ� (�^�u�\���p) 2003.10.13 MIK
	
	Version 50:
	�E�B���h�E�ʒu�Œ�E�p����ǉ� 2004.05.13 Moca

	Version 51:
	�^�O�W�����v�@�\�ǉ� 2004/06/21 novice

	Version 52:
	�O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O�ǉ�	2004.10.03 genta

	Version 53:
	���݂��Ȃ��t�@�C�����J�����Ƃ����ꍇ�Ɍx������t���O�̒ǉ�	2004.10.09 genta

	Version 54:
	�}�E�X�T�C�h�{�^���Ή� 2004/10/10 novice

	Version 55:
	�}�E�X���{�^���Ή� 2004/10/11 novice

	Version 56:
	�C���N�������^���T�[�`(Migemo path�p) 2004/10/13 isearch
	
	Version 57:
	�����L�[���[�h�w��g�� 2005/01/13 MIK
	
	Version 58:
	�����L�[���[�h�Z�b�g�ϒ����蓖�� 2005/01/25 Moca

	Version 59:
	�}�N�����𑝂₵�� 2005/01/30 genta
	
	Version 60:
	�L�[���[�h�w��^�O�W�����v����ۑ� 2005/04/03 MIK

	Version 61:
	���s�ōs���̋󔒂��폜����I�v�V����(�^�C�v�ʐݒ�) 2005/10/11 ryoji

	Version 62:
	�o�b�N�A�b�v�t�H���_ 2005.11.07 aroka

	Version 63:
	�w�茅�c���\���ǉ� 2005.11.08 Moca

	Version 64:
	�^�u�̋@�\�g��(�����A�A�C�R���\��) 2006/01/28 ryoji
	�A�E�g�v�b�g�E�B���h�E�ʒu 2006.02.01 aroka

	Version 65:
	�^�u�ꗗ���\�[�g���� 2006/05/10 ryoji

	Version 66:
	�L�[���[�h�w���v�@�\�g�� 2006.04.10 fon
		�L�[���[�h�w���v�@�\�ݒ��ʃ^�u�Ɉړ�, �����̕������ɑΉ�, �L�����b�g�ʒu�L�[���[�h�����ǉ�

	Version 67:
	�L�����b�g�F�w���ǉ� 2006.12.07 ryoji

	Version 68:
	�t�@�C���_�C�A���O�̃t�B���^�ݒ� 2006.11.16 ryoji

	Version 69:
	�u���ׂĒu���v�͒u���̌J�Ԃ� 2006.11.16 ryoji

	Version 70:
	[���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� 2006.12.25 ryoji

	Version 71:
	�^�u����铮��𐧌䂷��I�v�V����2��ǉ� 2007.02.11 genta
		m_bRetainEmptyTab, m_bCloseOneWinInTabMode

	Version 72:
	�^�u�ꗗ���t���p�X�\������ 2007.02.28 ryoji

	Version 73:
	�ҏW�E�B���h�E�ؑ֒� 2007.04.03 ryoji

	Version 74:
	�J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� 2006.08.23 ryoji

	Version 75:
	�}�E�X�z�C�[���ŃE�B���h�E�؂�ւ� 2006.03.26 ryoji

	Version 76:
	�^�u�̃O���[�v�� 2007.06.20 ryoji

	Version 77:
	ini�t�H���_�ݒ� 2007.05.31 ryoji

	Version 78:
	�G�f�B�^�|�g���C�Ԃł�UI���������m�F�̂��߂̃o�[�W�������킹 2007.06.07 ryoji

	Version 79:
	�O���R�}���h���s�̃I�v�V�����g�� 2007.04.29 maru

	Version 80:
	���K�\�����C�u�����̐؂�ւ� 2007.07.22 genta

	Version 81:
	�}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji

	Version 82:
	���C�����[�h�\��t�����\�ɂ��� 2007.10.08 ryoji

	Version 83:
	�I���Ȃ��ŃR�s�[���\�ɂ��� 2007.11.18 ryoji

	Version 1000:
	�o�[�W����1000�ȍ~��{�Ɠ����܂ł̊ԁA�g�킹�Ă��������B���Ȃ�p�ɂɍ\�����ύX�����Ǝv����̂ŁBby kobake 2008.03.02
*/

const unsigned int uShareDataVersion = 1008;

/*
||	Singleton��
*/
CShareData* CShareData::_instance = NULL;

/*! @brief CShareData::m_pEditArr�ی�pMutex

	�����̃G�f�B�^���񓯊��Ɉ�ē��삵�Ă���Ƃ��ł��ACShareData::m_pEditArr��
	���S�ɑ���ł���悤���쒆��Mutex��Lock()����B

	@par�i�񓯊���ē���̗�j
		�����̃E�B���h�E��\�����Ă��ăO���[�v����L���ɂ����^�X�N�o�[�Łu�O���[�v�����v����������Ƃ�

	@par�i�ی삷��ӏ��̗�j
		CShareData::AddEditWndList(): �G���g���̒ǉ��^���ёւ�
		CShareData::DeleteEditWndList(): �G���g���̍폜
		CShareData::GetOpenedWindowArr(): �z��̃R�s�[�쐬

	����ɂǂ��ɂł������ƃf�b�h���b�N����댯������̂œ����Ƃ��͐T�d�ɁB
	�iLock()���Ԓ���SendMessage()�Ȃǂő��E�B���h�E�̑��������Ɗ댯����j
	CShareData::m_pEditArr�𒼐ڎQ�Ƃ�����ύX����悤�ȉӏ��ɂ͐��ݓI�Ȋ댯�����邪�A
	�Θb�^�ŏ������삵�Ă���͈͂ł���΂܂����͋N���Ȃ��B

	@date 2007.07.05 ryoji �V�K����
	@date 2007.07.07 genta CShareData�̃����o�ֈړ�
*/
CMutex CShareData::g_cEditArrMutex( FALSE, GSTR_MUTEX_SAKURA_EDITARR );

CShareData* CShareData::getInstance()
{
	return _instance;
}

/*!
	���L�������̈悪����ꍇ�̓v���Z�X�̃A�h���X��Ԃ���
	���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v����B
*/
CShareData::~CShareData()
{
	if( NULL != m_pShareData ){
		/* �v���Z�X�̃A�h���X��Ԃ��� ���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v���܂� */
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
	return;
}


//! CShareData�N���X�̏���������
/*!
	CShareData�N���X�𗘗p����O�ɕK���Ăяo�����ƁB

	@retval true ����������
	@retval false ���������s

	@note ���ɑ��݂��鋤�L�������̃o�[�W���������̃G�f�B�^���g�����̂�
	�قȂ�ꍇ�͒v���I�G���[��h�����߂�false��Ԃ��܂��BCProcess::Initialize()
	��Init()�Ɏ��s����ƃ��b�Z�[�W���o���ăG�f�B�^�̋N���𒆎~���܂��B
*/
bool CShareData::Init( void )
{
	MY_RUNNINGTIMER(cRunningTimer,"CShareData::Init" );

//	MessageBoxA(NULL,"share","init",MB_OK);

	if (CShareData::_instance == NULL)	//	Singleton��
		CShareData::_instance = this;

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	/* �t�@�C���}�b�s���O�I�u�W�F�N�g */
	m_hFileMap = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,	//	Sep. 6, 2003 wmlhq
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof( DLLSHAREDATA ),
		GSTR_CSHAREDATA
	);
	if( NULL == m_hFileMap ){
		::MessageBoxA(
			NULL,
			"CreateFileMapping()�Ɏ��s���܂���",
			"�\�����ʃG���[",
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}
	if( GetLastError() != ERROR_ALREADY_EXISTS ){
		/* �I�u�W�F�N�g�����݂��Ă��Ȃ������ꍇ */
		/* �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂� */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		m_pShareData->OnInit();

		// 2007.05.19 ryoji ���s�t�@�C���t�H���_->�ݒ�t�@�C���t�H���_�ɕύX
		TCHAR	szIniFolder[_MAX_PATH];
		m_pShareData->m_IniFolder.m_bInit = false;
		GetInidir( szIniFolder );
		AddLastChar( szIniFolder, _MAX_PATH, _T('\\') );

		m_pShareData->m_vStructureVersion = uShareDataVersion;
		_tcscpy(m_pShareData->m_szKeyMacroFileName, _T(""));	/* �L�[���[�h�}�N���̃t�@�C���� */ //@@@ 2002.1.24 YAZAKI
		m_pShareData->m_bRecordingKeyMacro = FALSE;		/* �L�[�{�[�h�}�N���̋L�^�� */
		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

		// 2004.05.13 Moca ���\�[�X���琻�i�o�[�W�����̎擾
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&m_pShareData->m_dwProductVersionMS, &m_pShareData->m_dwProductVersionLS );
		m_pShareData->m_hwndTray = NULL;
		m_pShareData->m_hAccel = NULL;
		m_pShareData->m_hwndDebug = NULL;
		m_pShareData->m_nSequences = 0;					/* �E�B���h�E�A�� */
		m_pShareData->m_nGroupSequences = 0;			/* �^�u�O���[�v�A�� */	// 2007.06.20 ryoji
		m_pShareData->m_nEditArrNum = 0;

		m_pShareData->m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji

		m_pShareData->m_Common.m_sGeneral.m_nMRUArrNum_MAX = 15;	/* �t�@�C���̗���MAX */	//Oct. 14, 2000 JEPRO �������₵��(10��15)
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
		CMRU cMRU;
		cMRU.ClearAll();
		m_pShareData->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = 15;	/* �t�H���_�̗���MAX */	//Oct. 14, 2000 JEPRO �������₵��(10��15)
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
		CMRUFolder cMRUFolder;
		cMRUFolder.ClearAll();

		m_pShareData->m_aSearchKeys.clear();
		m_pShareData->m_aReplaceKeys.clear();
		m_pShareData->m_aGrepFiles.clear();
		m_pShareData->m_aGrepFiles.push_back(_T("*.*"));
		m_pShareData->m_aGrepFolders.clear();

		_tcscpy( m_pShareData->m_szMACROFOLDER, szIniFolder );	/* �}�N���p�t�H���_ */
		_tcscpy( m_pShareData->m_szIMPORTFOLDER, szIniFolder );	/* �ݒ�C���|�[�g�p�t�H���_ */

		for( int i = 0; i < MAX_TRANSFORM_FILENAME; ++i ){
			_tcscpy( m_pShareData->m_szTransformFileNameFrom[i], _T("") );
			_tcscpy( m_pShareData->m_szTransformFileNameTo[i], _T("") );
		}
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[0], _T("%DeskTop%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[0],   _T("�f�X�N�g�b�v\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[1], _T("%Personal%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[1],   _T("�}�C�h�L�������g\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[2],   _T("IE�L���b�V��\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[3], _T("%TEMP%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[3],   _T("TEMP\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[4],   _T("���L�f�X�N�g�b�v\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[5], _T("%Common Documents%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[5],   _T("���L�h�L�������g\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[6], _T("%AppData%\\") );	// 2007.05.19 ryoji �ǉ�
		_tcscpy( m_pShareData->m_szTransformFileNameTo[6],   _T("�A�v���f�[�^\\") );	// 2007.05.19 ryoji �ǉ�
		m_pShareData->m_nTransformFileNameArrNum = 7;
		
		/* m_PrintSettingArr[0]��ݒ肵�āA�c���1�`7�ɃR�s�[����B
			�K�v�ɂȂ�܂Œx�点�邽�߂ɁACPrint�ɁACShareData�𑀍삷�錠����^����B
			YAZAKI.
		*/
		{
			/*
				2006.08.16 Moca �������P�ʂ� PRINTSETTING�ɕύX�BCShareData�ɂ͈ˑ����Ȃ��B
			*/
			TCHAR szSettingName[64];
			int i = 0;
			auto_sprintf( szSettingName, _T("����ݒ� %d"), i + 1 );
			CPrint::SettingInitialize( m_pShareData->m_PrintSettingArr[0], szSettingName );	//	���������߁B
		}
		for( int i = 1; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
			auto_sprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, _T("����ݒ� %d"), i + 1 );	/* ����ݒ�̖��O */
		}

		//	Jan. 30, 2005 genta �֐��Ƃ��ēƗ�
		//	2007.11.04 genta �߂�l�`�F�b�N�Dfalse�Ȃ�N�����f�D
		if( ! InitKeyAssign( m_pShareData )){
			return false;
		}

//	From Here Sept. 19, 2000 JEPRO �R�����g�A�E�g�ɂȂ��Ă������߂̃u���b�N�𕜊������̉����R�����g�A�E�g
//	MS �S�V�b�N�W���X�^�C��10pt�ɐݒ�
//		/* LOGFONT�̏����� */
		memset_raw( &m_pShareData->m_Common.m_sView.m_lf, 0, sizeof( m_pShareData->m_Common.m_sView.m_lf ) );
		m_pShareData->m_Common.m_sView.m_lf.lfHeight			= -13;
		m_pShareData->m_Common.m_sView.m_lf.lfWidth				= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfEscapement		= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfOrientation		= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfWeight			= 400;
		m_pShareData->m_Common.m_sView.m_lf.lfItalic			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfUnderline			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfStrikeOut			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfCharSet			= 0x80;
		m_pShareData->m_Common.m_sView.m_lf.lfOutPrecision		= 0x3;
		m_pShareData->m_Common.m_sView.m_lf.lfClipPrecision		= 0x2;
		m_pShareData->m_Common.m_sView.m_lf.lfQuality			= 0x1;
		m_pShareData->m_Common.m_sView.m_lf.lfPitchAndFamily	= 0x31;
		_tcscpy( m_pShareData->m_Common.m_sView.m_lf.lfFaceName, _T("�l�r �S�V�b�N") );

		// �L�[���[�h�w���v�̃t�H���g ai 02/05/21 Add S
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&m_pShareData->m_Common.m_sHelper.m_lf_kh,	// depends on action to be taken
			NULL									// user profile update flag
		);
		// ai 02/05/21 Add E

//	To Here Sept. 19,2000

		m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;				/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */

//		m_pShareData->m_Common.m_bUseCaretKeyWord = FALSE;		/* �L�����b�g�ʒu�̒P�����������-�@�\OFF */	// 2006.03.24 fon sakura�N������FALSE�Ƃ��A���������Ȃ�


		/* �o�b�N�A�b�v */
		CommonSetting_Backup& sBackup = m_pShareData->m_Common.m_sBackup;
		sBackup.m_bBackUp = false;										/* �o�b�N�A�b�v�̍쐬 */
		sBackup.m_bBackUpDialog = true;									/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		sBackup.m_bBackUpFolder = false;								/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		sBackup.m_szBackUpFolder[0] = L'\0';							/* �o�b�N�A�b�v���쐬����t�H���_ */
		sBackup.m_nBackUpType = 2;										/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		sBackup.m_nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;	/* �o�b�N�A�b�v�t�@�C�����F���t */
		sBackup.m_nBackUpType_Opt2 = ('b' << 16 ) + 10;					/* �o�b�N�A�b�v�t�@�C�����F�A�Ԃ̐��Ɛ擪���� */
		sBackup.m_nBackUpType_Opt3 = 5;									/* �o�b�N�A�b�v�t�@�C�����FOption3 */
		sBackup.m_nBackUpType_Opt4 = 0;									/* �o�b�N�A�b�v�t�@�C�����FOption4 */
		sBackup.m_nBackUpType_Opt5 = 0;									/* �o�b�N�A�b�v�t�@�C�����FOption5 */
		sBackup.m_nBackUpType_Opt6 = 0;									/* �o�b�N�A�b�v�t�@�C�����FOption6 */
		sBackup.m_bBackUpDustBox = false;								/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
		sBackup.m_bBackUpPathAdvanced = false;							/* 20051107 aroka �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� */
		sBackup.m_szBackUpPathAdvanced[0] = _T('\0');					/* 20051107 aroka �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� */

		m_pShareData->m_Common.m_sGeneral.m_nCaretType = 0;					/* �J�[�\���̃^�C�v 0=win 1=dos */
		m_pShareData->m_Common.m_sGeneral.m_bIsINSMode = true;				/* �}���^�㏑�����[�h */
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = false;		/* �t���[�J�[�\�����[�h�� */	//Oct. 29, 2000 JEPRO �u�Ȃ��v�ɕύX

		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord = FALSE;	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */
		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph = FALSE;	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */

		m_pShareData->m_Common.m_sSearch.m_sSearchOption.Reset();			// �����I�v�V����
		m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll = 0;			// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
		m_pShareData->m_Common.m_sSearch.m_bSelectedArea = FALSE;			// �I��͈͓��u��
		m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] = L'\0';			// �O���w���v�P
		m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] = L'\0';		// �O��HTML�w���v
		
		m_pShareData->m_Common.m_sHelper.m_szMigemoDll[0] = L'\0';			/* migemo dll */
		m_pShareData->m_Common.m_sHelper.m_szMigemoDict[0] = L'\0';		/* migemo dict */

		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = TRUE;		/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */

		m_pShareData->m_Common.m_sGeneral.m_bCloseAllConfirm = FALSE;		/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
		m_pShareData->m_Common.m_sGeneral.m_bExitConfirm = FALSE;			/* �I�����̊m�F������ */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(3);	/* �L�[���s�[�g���̃X�N���[���s�� */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScroll_Smooth = FALSE;/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */

		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy = FALSE;			/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
		m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = TRUE;			/* Grep: �T�u�t�H���_������ */
		m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = TRUE;			/* Grep: �s���o�͂��邩�Y�����������o�͂��邩 */
		m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = 1;			/* Grep: �o�͌`�� */
		m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder=FALSE;		/* Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ��� */
		m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = CODE_AUTODETECT;	/* Grep: �����R�[�h�Z�b�g */
		m_pShareData->m_Common.m_sSearch.m_bGrepRealTimeView = FALSE;		/* 2003.06.28 Moca Grep���ʂ̃��A���^�C���\�� */
		m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch = TRUE;		/* 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� */
		m_pShareData->m_Common.m_sSearch.m_szRegexpLib[0] =_T('\0');		/* 2007.08.12 genta ���K�\��DLL */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_RETURN = TRUE;				/* �G���^�[�L�[�Ń^�O�W�����v */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK = TRUE;			/* �_�u���N���b�N�Ń^�O�W�����v */

//�L�[���[�h�F�c�[���o�[����
		//	Jan. 30, 2005 genta �֐��Ƃ��ēƗ�
		InitToolButtons( m_pShareData );

		m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR = TRUE;			/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
		m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR = TRUE;			/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
		m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND = FALSE;		/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place = 1;			/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = 4;			// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

		m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd = FALSE;			//�^�u�E�C���h�E�\��	//@@@ 2003.05.31 MIK
		m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin = FALSE;	//�^�u�E�C���h�E�\��	//@@@ 2003.05.31 MIK
		wcscpy( m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption,
	L"${w?�yGrep�z$h$:�y�A�E�g�v�b�g�z$:$f$}${U?(�X�V)$}${R?(�r���[���[�h)$:(�㏑���֎~)$}${M?�y�L�[�}�N���̋L�^���z$}" );	//@@@ 2003.06.13 MIK
		m_pShareData->m_Common.m_sTabBar.m_bSameTabWidth = FALSE;			//�^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon = FALSE;			//�^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bSortTabList = TRUE;			//�^�u�ꗗ���\�[�g����		//@@@ 2006.05.10 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bTab_RetainEmptyWin = TRUE;	// �Ō�̃t�@�C��������ꂽ�Ƃ�(����)���c��	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin = FALSE;	// �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_ListFull = FALSE;			//�^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bChgWndByWheel = FALSE;		//�}�E�X�z�C�[���ŃE�B���h�E�ؑ�	//@@@ 2006.03.26 ryoji

		m_pShareData->m_Common.m_sWindow.m_bSplitterWndHScroll = TRUE;	// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�
		m_pShareData->m_Common.m_sWindow.m_bSplitterWndVScroll = TRUE;	// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�

		/* �J�X�^�����j���[��� */
		auto_sprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0], LTEXT("�E�N���b�N���j���[") );
		for( int i = 1; i < MAX_CUSTOM_MENU; ++i ){
			auto_sprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i], LTEXT("���j���[%d"), i );
			m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] = 0;
			for( int j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[i][j] = F_0;
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr [i][j] = '\0';
			}
		}
		auto_sprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND], LTEXT("�^�u���j���[") );	//@@@ 2003.06.13 MIK


		/* ���o���L�� */
		wcscpy( m_pShareData->m_Common.m_sFormat.m_szMidashiKigou, L"�P�Q�R�S�T�U�V�W�X�O�i(�m[�u�w�y�������������������������E��������@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]���O�l�ܘZ������\���Q��" );
		/* ���p�� */
		wcscpy( m_pShareData->m_Common.m_sFormat.m_szInyouKigou, L"> " );		/* ���p�� */
		m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;					/* ���͕⊮�@�\���g�p���� */

		// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜
		//	2004.05.13 Moca �E�B���h�E�T�C�Y�Œ�w��ǉ��ɔ����w����@�ύX
		m_pShareData->m_Common.m_sWindow.m_nSaveWindowSize = WINSIZEMODE_SAVE;	// �E�B���h�E�T�C�Y�p��
		m_pShareData->m_Common.m_sWindow.m_nWinSizeType = SIZE_RESTORED;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCY = 0;
		
		//	2004.05.13 Moca �E�B���h�E�ʒu
		m_pShareData->m_Common.m_sWindow.m_nSaveWindowPos = WINSIZEMODE_DEF;		// �E�B���h�E�ʒu�Œ�E�p��
		m_pShareData->m_Common.m_sWindow.m_nWinPosX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinPosY = 0;

		m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray = TRUE;				/* �^�X�N�g���C�̃A�C�R�����g�� */
		m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray = TRUE;				/* �^�X�N�g���C�̃A�C�R�����풓 */
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode = L'Z';		/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop = TRUE;			/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource = TRUE;			/* OLE�ɂ��h���b�O���ɂ��邩 */
		m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog = FALSE;		/* �I���_�C�A���O��\������ */
		m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL = TRUE;			/* URL���N���b�N���ꂽ��I�����邩 */
		m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm = FALSE;			/* Grep���[�h�ŕۑ��m�F���邩 */
//		m_pShareData->m_Common.m_bRulerDisp = TRUE;					/* ���[���[�\�� */
		m_pShareData->m_Common.m_sWindow.m_nRulerHeight = 13;					/* ���[���[�̍��� */
		m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace = 0;				/* ���[���[�ƃe�L�X�g�̌��� */
		m_pShareData->m_Common.m_sWindow.m_nRulerType = 0;					/* ���[���[�̃^�C�v */
		//	Sep. 18, 2002 genta
		m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace = 0;			/* �s�ԍ��̉E�̌��� */
		m_pShareData->m_Common.m_sWindow.m_nVertLineOffset = -1;			// 2005.11.10 Moca �w�茅�c��
		m_pShareData->m_Common.m_sEdit.m_bCopyAndDisablSelection = FALSE;	/* �R�s�[������I������ */
		m_pShareData->m_Common.m_sEdit.m_bEnableNoSelectCopy = TRUE;		/* �I���Ȃ��ŃR�s�[���\�ɂ��� */	// 2007.11.18 ryoji
		m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste = TRUE;		/* ���C�����[�h�\��t�����\�ɂ��� */	// 2007.10.08 ryoji
		m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle = TRUE;		/* HtmlHelp�r���[�A�͂ЂƂ� */
		m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz = TRUE;		/* ������r��A���E�ɕ��ׂĕ\�� */

		//[�t�@�C��]�^�u
		{
			//�t�@�C���̔r������
			m_pShareData->m_Common.m_sFile.m_nFileShareMode = SHAREMODE_DENY_WRITE;	// �t�@�C���̔r�����䃂�[�h
			m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp = true;			// �X�V�̊Ď�

			//�t�@�C���̕ۑ�
			m_pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite = false;	// ���ύX�ł��㏑�����邩

			// �u���O��t���ĕۑ��v�Ńt�@�C���̎�ނ�[���[�U�w��]�̂Ƃ��̃t�@�C���ꗗ�\��	//�t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
			m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew = true;		// �V�K����ۑ����͑S�t�@�C���\��
			m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile = true;		// �V�K�ȊO����ۑ����͑S�t�@�C���\��

			//�t�@�C���I�[�v��
			m_pShareData->m_Common.m_sFile.m_bDropFileAndClose = false;		// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
			m_pShareData->m_Common.m_sFile.m_nDropFileNumMax = 8;			// ��x�Ƀh���b�v�\�ȃt�@�C����
			m_pShareData->m_Common.m_sFile.m_bRestoreCurPosition = true;	// �J�[�\���ʒu����	//	Oct. 27, 2000 genta
			m_pShareData->m_Common.m_sFile.m_bRestoreBookmarks = true;		// �u�b�N�}�[�N����	//2002.01.16 hor
			m_pShareData->m_Common.m_sFile.m_bAutoMIMEdecode = false;		// �t�@�C���ǂݍ��ݎ���MIME�̃f�R�[�h���s����	//Jul. 13, 2001 JEPRO
			m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange = true;		// �O��ƈقȂ镶���R�[�h�̎��ɖ₢���킹���s����	Oct. 03, 2004 genta
			m_pShareData->m_Common.m_sFile.m_bAlertIfFileNotExist = false;	// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������	Oct. 09, 2004 genta
		}

		m_pShareData->m_Common.m_sEdit.m_bNotOverWriteCRLF = TRUE;			/* ���s�͏㏑�����Ȃ� */
		::SetRect( &m_pShareData->m_Common.m_sOthers.m_rcOpenDialog, 0, 0, 0, 0 );	/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = TRUE;			/* �����_�C�A���O�������I�ɕ��� */
		m_pShareData->m_Common.m_sSearch.m_bSearchAll		 = FALSE;			/* �����^�u���^�u�b�N�}�[�N  �擪�i�����j����Č��� 2002.01.26 hor */
		m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz = TRUE;				/* �����X�N���[���o�[���g�� */
		m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList = FALSE;		/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */	//Nov. 18, 2000 JEPRO TRUE��FALSE �ɕύX
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace = TRUE;		/* �u�� �_�C�A���O�������I�ɕ��� */
		m_pShareData->m_Common.m_sEdit.m_bAutoColmnPaste = TRUE;			/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */
		m_pShareData->m_Common.m_sGeneral.m_bNoCaretMoveByActivation = FALSE;	/* �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji (add by genta) */

		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_TAB		= FALSE;		/* VK_TAB   �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RIGHT	= TRUE;			/* VK_RIGHT �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_SPACE	= FALSE;		/* VK_SPACE �⊮����L�[���L��/���� */

		m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable	=	FALSE;	//�A�E�g���C���_�C�A���O�Ńu�b�N�}�[�N�̋�s�𖳎�			2002.02.08 aroka,hor
		m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump	=	FALSE;	//�A�E�g���C���_�C�A���O�ŃW�����v������t�H�[�J�X���ڂ�	2002.02.08 hor

		/*
			�����w��q�̈Ӗ���Windows SDK��GetDateFormat(), GetTimeFormat()���Q�Ƃ̂���
		*/

		m_pShareData->m_Common.m_sFormat.m_nDateFormatType = 0;	//���t�����̃^�C�v
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szDateFormat, _T("yyyy\'�N\'M\'��\'d\'��(\'dddd\')\'") );	//���t����
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType = 0;	//���������̃^�C�v
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szTimeFormat, _T("tthh\'��\'mm\'��\'ss\'�b\'")  );			//��������

		m_pShareData->m_Common.m_sWindow.m_bMenuIcon = TRUE;		/* ���j���[�ɃA�C�R����\������ */


		m_pShareData->m_aCommands.clear();

		InitKeyword( m_pShareData );
		InitTypeConfigs( m_pShareData );
		InitPopupMenu( m_pShareData );

		//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̏����l
		//	Aug. 16, 2003 genta $N(�t�@�C�����ȗ��\��)���f�t�H���g�ɕύX
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionActive, 
			_T("${w?$h$:�A�E�g�v�b�g$:${I?$f$:$N$}$}${U?(�X�V)$} -")
			_T(" $A $V ${R?(�r���[���[�h)$:�i�㏑���֎~�j$}${M?  �y�L�[�}�N���̋L�^���z$}") );
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionInactive, 
			_T("${w?$h$:�A�E�g�v�b�g$:$f$}${U?(�X�V)$} -")
			_T(" $A $V ${R?(�r���[���[�h)$:�i�㏑���֎~�j$}${M?  �y�L�[�}�N���̋L�^���z$}") );

		//	From Here Sep. 14, 2001 genta
		//	Macro�o�^�̏�����
		MacroRec *mptr = m_pShareData->m_MacroTable;
		for( int i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
			mptr->m_szName[0] = L'\0';
			mptr->m_szFile[0] = L'\0';
			mptr->m_bReloadWhenExecute = FALSE;
		}
		//	To Here Sep. 14, 2001 genta

		// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
		m_pShareData->m_TagJumpNum = 0;
		// 2004.06.22 Moca �^�O�W�����v�̐擪
		m_pShareData->m_TagJumpTop = 0;
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
//		m_pShareData->m_bGetStdout = TRUE;	/* �O���R�}���h���s�́u�W���o�͂𓾂�v */
		m_pShareData->m_nExecFlgOpt = 1;	/* �O���R�}���h���s�́u�W���o�͂𓾂�v */	// 2006.12.03 maru �I�v�V�����̊g���̂���
		m_pShareData->m_bLineNumIsCRLF = TRUE;	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */

		m_pShareData->m_nDiffFlgOpt = 0;	/* DIFF�����\�� */	//@@@ 2002.05.27 MIK

		m_pShareData->m_nTagsOpt = 0;	/* CTAGS */	//@@@ 2003.05.12 MIK
		_tcscpy( m_pShareData->m_szTagsCmdLine, _T("") );	/* CTAGS */	//@@@ 2003.05.12 MIK
		//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v��History�ۊ�
		m_pShareData->m_aTagJumpKeywords.clear();
		m_pShareData->m_bTagJumpICase = FALSE;
		m_pShareData->m_bTagJumpAnyWhere = FALSE;
		//To Here 2005.04.03 MIK 

	}else{
		/* �I�u�W�F�N�g�����łɑ��݂���ꍇ */
		/* �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂� */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		m_pShareData->OnInit();

		//	From Here Oct. 27, 2000 genta
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ){
			//	���̋��L�f�[�^�̈�͎g���Ȃ��D
			//	�n���h�����������
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta
	}
	return true;
}




/* KEYDATA�z��Ƀf�[�^���Z�b�g */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	TCHAR*			pszKeyName,
	EFunctionCode	nFuncCode_0,
	EFunctionCode	nFuncCode_1,
	EFunctionCode	nFuncCode_2,
	EFunctionCode	nFuncCode_3,
	EFunctionCode	nFuncCode_4,
	EFunctionCode	nFuncCode_5,
	EFunctionCode	nFuncCode_6,
	EFunctionCode	nFuncCode_7
)
{
	pShareData->m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	_tcscpy( pShareData->m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
	return;
}



/*!
	�t�@�C��������A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszFilePath [in] �t�@�C����
	
	�g���q��؂�o���� GetDocumentTypeExt �ɓn�������D
*/
CTypeConfig CShareData::GetDocumentType( const TCHAR* pszFilePath )
{
	TCHAR	szExt[_MAX_EXT];

	if( NULL != pszFilePath && 0 < (int)_tcslen( pszFilePath ) ){
		_tsplitpath( pszFilePath, NULL, NULL, NULL, szExt );
		if( szExt[0] == _T('.') )
			return GetDocumentTypeExt( szExt + 1 );
		else
			return GetDocumentTypeExt( szExt );
	}
	return CTypeConfig(0);
}


/*!
	�g���q����A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszExt [in] �g���q (�擪��,�͊܂܂Ȃ�)
	
	�w�肳�ꂽ�g���q�̑����镶���^�C�v�ԍ���Ԃ��D
	�Ƃ肠�������̂Ƃ���̓^�C�v�͊g���q�݂̂Ɉˑ�����Ɖ��肵�Ă���D
	�t�@�C���S�̂̌`���ɑΉ�������Ƃ��́C�܂��l�������D
*/
CTypeConfig CShareData::GetDocumentTypeExt( const TCHAR* pszExt )
{
	const TCHAR	pszSeps[] = _T(" ;,");	// separator

	int		i;
	TCHAR*	pszToken;
	TCHAR	szText[256];

	for( i = 0; i < MAX_TYPES; ++i ){
		CTypeConfig nType(i);
		_tcscpy( szText, m_pShareData->GetTypeSetting(nType).m_szTypeExts );
		pszToken = _tcstok( szText, pszSeps );
		while( NULL != pszToken ){
			if( 0 == _tcsicmp( pszExt, pszToken ) ){
				return CTypeConfig(i);	//	�ԍ�
			}
			pszToken = _tcstok( NULL, pszSeps );
		}
	}
	return CTypeConfig(0);	//	�n�Y��
}





/** �ҏW�E�B���h�E���X�g�ւ̓o�^

	@param hWnd   [in] �o�^����ҏW�E�B���h�E�̃n���h��
	@param nGroup [in] �V�K�o�^�̏ꍇ�̃O���[�vID

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji �V�K�E�B���h�E�ɂ̓O���[�vID��t�^����
*/
BOOL CShareData::AddEditWndList( HWND hWnd, int nGroup/* = 0*/ )
{
	int		nSubCommand = TWNT_ADD;
	int		nIndex;
	EditNode	sMyEditNode;
	EditNode	*p;

	memset_raw( &sMyEditNode, 0, sizeof( sMyEditNode ) );
	sMyEditNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock�̈�
	LockGuard<CMutex> guard( g_cEditArrMutex );

	CRecentEditNode	cRecentEditNode;

	//�o�^�ς݂��H
	nIndex = cRecentEditNode.FindItemByHwnd( hWnd );
	if( -1 != nIndex )
	{
		//��������ȏ�o�^�ł��Ȃ����H
		if( cRecentEditNode.GetItemCount() >= cRecentEditNode.GetArrayCount() )
		{
			cRecentEditNode.Terminate();
			return FALSE;
		}
		nSubCommand = TWNT_ORDER;

		//�ȑO�̏����R�s�[����B
		p = cRecentEditNode.GetItem( nIndex );
		if( p )
		{
			memcpy_raw( &sMyEditNode, p, sizeof( sMyEditNode ) );
		}
	}

	/* �E�B���h�E�A�� */

	if( 0 == ::GetWindowLongPtr( hWnd, sizeof(LONG_PTR) ) )
	{
		m_pShareData->m_nSequences++;
		::SetWindowLongPtr( hWnd, sizeof(LONG_PTR) , (LONG_PTR)m_pShareData->m_nSequences );

		//�A�Ԃ��X�V����B
		sMyEditNode.m_nIndex = m_pShareData->m_nSequences;

		/* �^�u�O���[�v�A�� */
		if( nGroup > 0 )
		{
			sMyEditNode.m_nGroup = nGroup;	// �w��̃O���[�v
		}
		else
		{
			p = cRecentEditNode.GetItem( 0 );
			if( NULL == p )
				sMyEditNode.m_nGroup = ++m_pShareData->m_nGroupSequences;	// �V�K�O���[�v
			else
				sMyEditNode.m_nGroup = p->m_nGroup;	// �ŋ߃A�N�e�B�u�̃O���[�v
		}

		sMyEditNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		sMyEditNode.m_bClosing = FALSE;
	}

	//�ǉ��܂��͐擪�Ɉړ�����B
	cRecentEditNode.AppendItem( &sMyEditNode );
	cRecentEditNode.Terminate();
	}	// 2007.07.07 genta Lock�̈�I���

	//�E�C���h�E�o�^���b�Z�[�W���u���[�h�L���X�g����B
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)nSubCommand, (LPARAM)hWnd, hWnd, GetGroupId( hWnd ) );

	return TRUE;
}





/** �ҏW�E�B���h�E���X�g����̍폜

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.07.05 ryoji mutex�ŕی�
*/
void CShareData::DeleteEditWndList( HWND hWnd )
{
	int nGroup = GetGroupId( hWnd );

	//�E�C���h�E�����X�g����폜����B
	{	// 2007.07.07 genta Lock�̈�
		LockGuard<CMutex> guard( g_cEditArrMutex );

		CRecentEditNode	cRecentEditNode;
DBPRINT_A("cnt %d",cRecentEditNode.GetItemCount());
DBPRINT_A("DeleteItemByHwnd %08X",hWnd);
		cRecentEditNode.DeleteItemByHwnd( hWnd );
DBPRINT_A("/cnt %d",cRecentEditNode.GetItemCount());
		cRecentEditNode.Terminate();
	}

	//�E�C���h�E�폜���b�Z�[�W���u���[�h�L���X�g����B
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_DEL, (LPARAM)hWnd, hWnd, nGroup );

	return;
}

/** �O���[�v��ID���Z�b�g����

	@date 2007.06.20 ryoji
*/
void CShareData::ResetGroupId( void )
{
	int nGroup;
	int	i;

	nGroup = ++m_pShareData->m_nGroupSequences;
	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
		{
			m_pShareData->m_pEditArr[i].m_nGroup = nGroup;
		}
	}
}

/** �ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji

	@warning ���̊֐���m_pEditArr���̗v�f�ւ̃|�C���^��Ԃ��D
	m_pEditArr���ύX���ꂽ��ł̓A�N�Z�X���Ȃ��悤���ӂ��K�v�D
*/
EditNode* CShareData::GetEditNode( HWND hWnd )
{
	int	i;

	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( hWnd == m_pShareData->m_pEditArr[i].m_hWnd )
		{
			if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
				return &m_pShareData->m_pEditArr[i];
		}
	}

	return NULL;
}

/** �O���[�vID���擾����

	@date 2007.06.20 ryoji
*/
int CShareData::GetGroupId( HWND hWnd )
{
	EditNode* pEditNode;
	pEditNode = GetEditNode( hWnd );
	return (pEditNode != NULL)? pEditNode->m_nGroup: -1;
}

/** ����O���[�v���ǂ����𒲂ׂ�

	@param[in] hWnd1 ��r����E�B���h�E1
	@param[in] hWnd2 ��r����E�B���h�E2
	
	@return 2�̃E�B���h�E������O���[�v�ɑ����Ă����true

	@date 2007.06.20 ryoji
*/
bool CShareData::IsSameGroup( HWND hWnd1, HWND hWnd2 )
{
	int nGroup1;
	int nGroup2;

	if( hWnd1 == hWnd2 )
		return true;

	nGroup1 = GetGroupId( hWnd1 );
	if( nGroup1 < 0 )
		return false;

	nGroup2 = GetGroupId( hWnd2 );
	if( nGroup2 < 0 )
		return false;

	return ( nGroup1 == nGroup2 );
}

/** �w��ʒu�̕ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetEditNodeAt( int nGroup, int nIndex )
{
	int	i;
	int iIndex;

	iIndex = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( nGroup == 0 || nGroup == m_pShareData->m_pEditArr[i].m_nGroup )
		{
			if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
			{
				if( iIndex == nIndex )
					return &m_pShareData->m_pEditArr[i];
				iIndex++;
			}
		}
	}

	return NULL;
}

/** �擪�̕ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetTopEditNode( HWND hWnd )
{
	int nGroup;

	nGroup = GetGroupId( hWnd );
	return GetEditNodeAt( nGroup, 0 );
}

/** �擪�̕ҏW�E�B���h�E���擾����

	@return �^����ꂽ�G�f�B�^�E�B���h�E�Ɠ���O���[�v�ɑ���
	�擪�E�B���h�E�̃n���h��

	@date 2007.06.20 ryoji
*/
HWND CShareData::GetTopEditWnd( HWND hWnd )
{
	EditNode* p = GetTopEditNode( hWnd );

	return ( p != NULL )? p->m_hWnd: NULL;
}

/* ���L�f�[�^�̃��[�h */
BOOL CShareData::LoadShareData( void )
{
	return ShareData_IO_2( true );
}




/* ���L�f�[�^�̕ۑ� */
void CShareData::SaveShareData( void )
{
	ShareData_IO_2( false );
	return;
}




/** �S�ҏW�E�B���h�E�֏I���v�����o��

	@param bExit [in] TRUE: �ҏW�̑S�I�� / FALSE: ���ׂĕ���
	@param nGroup [in] �O���[�v�w��i0:�S�O���[�v�j

	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.22 ryoji nGroup������ǉ�
*/
BOOL CShareData::RequestCloseAllEditor( BOOL bExit, int nGroup )
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	for( i = 0; i < n; ++i ){
		if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
			if( IsEditWnd( pWndArr[i].m_hWnd ) ){
				/* �A�N�e�B�u�ɂ��� */
				ActivateFrameWindow( pWndArr[i].m_hWnd );
				/* �g���C����G�f�B�^�ւ̏I���v�� */
				if( !::SendMessageAny( pWndArr[i].m_hWnd, MYWM_CLOSE, bExit, 0 ) ){	// 2007.02.13 ryoji bExit�������p��
					delete []pWndArr;
					return FALSE;
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}



/*!
	@brief	�w��t�@�C�����J����Ă��邩���ׂ�
	
	�w��̃t�@�C�����J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h����Ԃ�

	@retval	TRUE ���łɊJ���Ă���
	@retval	FALSE �J���Ă��Ȃ�����
*/
BOOL CShareData::IsPathOpened( const TCHAR* pszPath, HWND* phwndOwner )
{
	EditInfo*	pfi;
	*phwndOwner = NULL;

	//	2007.10.01 genta ���΃p�X���΃p�X�ɕϊ�
	//	�ϊ����Ȃ���IsPathOpened�Ő��������ʂ�����ꂸ�C
	//	����t�@�C���𕡐��J�����Ƃ�����D
	TCHAR	szBuf[_MAX_PATH];
	if( GetLongFileName( pszPath, szBuf )){
		pszPath = szBuf;
	}

	// ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�
	if( 0 ==  GetEditorWindowsNum( 0 ) ){
		return FALSE;
	}
	
	for( int i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessageAny( m_pShareData->m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;

			// ����p�X�̃t�@�C�������ɊJ����Ă��邩
			if( 0 == _tcsicmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_pEditArr[i].m_hWnd;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*!
	@brief	�w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F

	�������łɊJ���Ă���΃A�N�e�B�u�ɂ��āA�E�B���h�E�̃n���h����Ԃ��B
	����ɁA�����R�[�h���قȂ�Ƃ��̃��[�j���O����������B
	���������ɎU��΂������d�I�[�v���������W��������̂��ړI�B

	@retval	�J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h��

	@note	CEditDoc::FileLoad�ɐ旧���Ď��s����邱�Ƃ����邪�A
			CEditDoc::FileLoad��������s�����K�v�����邱�Ƃɒ��ӁB
			(�t�H���_�w��̏ꍇ��CEditDoc::FileLoad�����ڎ��s�����ꍇ�����邽��)

	@retval	TRUE ���łɊJ���Ă���
	@retval	FALSE �J���Ă��Ȃ�����

	@date 2007.03.12 maru �V�K�쐬
*/
BOOL CShareData::ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, ECodeType nCharCode )
{
	if( IsPathOpened( pszPath, phwndOwner ) ){
		
		//�����R�[�h�̈�v�m�F
		EditInfo*		pfi;
		::SendMessageAny( *phwndOwner, MYWM_GETFILEINFO, 0, 0 );
		pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;
		if(nCharCode != CODE_AUTODETECT){
			LPCTSTR pszCodeNameNew = CCodeTypeName(nCharCode).Normal();
			LPCTSTR pszCodeNameCur = CCodeTypeName(pfi->m_nCharCode).Normal();

			if(pszCodeNameCur && pszCodeNameNew){
				if(nCharCode != pfi->m_nCharCode){
					TopWarningMessage( *phwndOwner,
						_T("%ts\n\n\n���ɊJ���Ă���t�@�C�����Ⴄ�����R�[�h�ŊJ���ꍇ�́A\n")
						_T("�t�@�C�����j���[����u�J�������v���g�p���Ă��������B\n")
						_T("\n")
						_T("���݂̕����R�[�h�Z�b�g=[%ts]\n")
						_T("�V���������R�[�h�Z�b�g=[%ts]"),
						pszPath,
						pszCodeNameCur,
						pszCodeNameNew
					);
				}
			}
			else{
				TopWarningMessage( *phwndOwner,
					_T("%ts\n\n���d�I�[�v���̊m�F�ŕs���ȕ����R�[�h���w�肳��܂����B\n")
					_T("\n")
					_T("���݂̕����R�[�h�Z�b�g=%d [%ts]\n�V���������R�[�h�Z�b�g=%d [%ts]"),
					pszPath,
					pfi->m_nCharCode,
					NULL==pszCodeNameCur?_T("�s��"):pszCodeNameCur,
					nCharCode,
					NULL==pszCodeNameNew?_T("�s��"):pszCodeNameNew
				);
			}
		}

		// �J���Ă���E�B���h�E���A�N�e�B�u�ɂ���
		ActivateFrameWindow( *phwndOwner );

		// MRU���X�g�ւ̓o�^
		CMRU().Add( pfi );
		return TRUE;
	}
	else {
		return FALSE;
	}

}




/** ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�

	@param nGroup [in] �O���[�v�w��i0:�S�O���[�v�j

	@date 2007.06.22 ryoji nGroup������ǉ�
*/
int CShareData::GetEditorWindowsNum( int nGroup )
{
	int		i;
	int		j;

	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			if( nGroup != 0 && nGroup != GetGroupId( m_pShareData->m_pEditArr[i].m_hWnd ) )
				continue;
			j++;
		}
	}
	return j;

}



/** �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����

	@param nGroup [in] �O���[�v�w��i0:�S�O���[�v�j

	@date 2005.01.24 genta hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
BOOL CShareData::PostMessageToAllEditors(
	UINT		uMsg,		/*!< �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/*!< ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/*!< ��2���b�Z�[�W �p�����[�^ */
	HWND		hWndLast,	/*!< �Ō�ɑ��肽���E�B���h�E */
	int			nGroup/* = 0*/	/*!< ���肽���O���[�v */
 )
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W���|�X�g */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W���|�X�g */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}


/** �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂�

	@param nGroup [in] �O���[�v�w��i0:�S�O���[�v�j

	@date 2005.01.24 genta m_hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
BOOL CShareData::SendMessageToAllEditors(
	UINT		uMsg,		/* �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/* ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/* ��2���b�Z�[�W �p�����[�^ */
	HWND		hWndLast,	/* �Ō�ɑ��肽���E�B���h�E */
	int			nGroup/* = 0*/	/*!< ���肽���O���[�v */
 )
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W�𑗂� */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* ���b�Z�[�W�𑗂� */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}


/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
BOOL CShareData::IsEditWnd( HWND hWnd )
{
	TCHAR	szClassName[64];
	if( hWnd == NULL ){	// 2007.06.20 ryoji �����ǉ�
		return FALSE;
	}
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, _countof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == _tcscmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}

}

// GetOpenedWindowArr�p�ÓI�ϐ��^�\����
static BOOL s_bSort;	// �\�[�g�w��
static BOOL s_bGSort;	// �O���[�v�w��
struct EditNodeEx{	// �g���\����
	EditNode* p;	// �ҏW�E�B���h�E�z��v�f�ւ̃|�C���^
	int nGroupMru;	// �O���[�v�P�ʂ�MRU�ԍ�
};

// GetOpenedWindowArr�p�\�[�g�֐�
static int __cdecl cmpGetOpenedWindowArr(const void *e1, const void *e2)
{
	// �قȂ�O���[�v�̂Ƃ��̓O���[�v��r����
	int nGroup1;
	int nGroup2;

	if( s_bGSort )
	{
		// �I���W�i���̃O���[�v�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->p->m_nGroup;
		nGroup2 = ((EditNodeEx*)e2)->p->m_nGroup;
	}
	else
	{
		// �O���[�v��MRU�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->nGroupMru;
		nGroup2 = ((EditNodeEx*)e2)->nGroupMru;
	}
	if( nGroup1 != nGroup2 )
	{
		return nGroup1 - nGroup2;	// �O���[�v��r
	}

	// �O���[�v��r���s���Ȃ������Ƃ��̓E�B���h�E��r����
	if( s_bSort )
		return ( ((EditNodeEx*)e1)->p->m_nIndex - ((EditNodeEx*)e2)->p->m_nIndex );	// �E�B���h�E�ԍ���r
	return ( ((EditNodeEx*)e1)->p - ((EditNodeEx*)e2)->p );	// �E�B���h�EMRU��r�i�\�[�g���Ȃ��j
}

/** ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

	@param[out] ppEditNode �z����󂯎��|�C���^
		�߂�l��0�̏ꍇ��NULL���Ԃ���邪�C��������҂��Ȃ����ƁD
		�܂��C�s�v�ɂȂ�����delete []���Ȃ��Ă͂Ȃ�Ȃ��D
	@param[in] bSort TRUE: �\�[�g���� / FALSE: �\�[�g����
	@param[in]bGSort TRUE: �O���[�v�\�[�g���� / FALSE: �O���[�v�\�[�g����

	���Ƃ̕ҏW�E�B���h�E���X�g�̓\�[�g���Ȃ���΃E�B���h�E��MRU���ɕ���ł���
	-------------------------------------------------
	bSort	bGSort	��������
	-------------------------------------------------
	FALSE	FALSE	�O���[�vMRU���|�E�B���h�EMRU��
	TRUE	FALSE	�O���[�vMRU���|�E�B���h�E�ԍ���
	FALSE	TRUE	�O���[�v�ԍ����|�E�B���h�EMRU��
	TRUE	TRUE	�O���[�v�ԍ����|�E�B���h�E�ԍ���
	-------------------------------------------------

	@return �z��̗v�f����Ԃ�
	@note �v�f��>0 �̏ꍇ�͌Ăяo�����Ŕz���delete []���Ă�������

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji bGroup�����ǉ��A�\�[�g���������O�̂��̂���qsort�ɕύX
*/
int CShareData::GetOpenedWindowArr( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	int nRet;

	LockGuard<CMutex> guard( g_cEditArrMutex );
	nRet = GetOpenedWindowArrCore( ppEditNode, bSort, bGSort );

	return nRet;
}

// GetOpenedWindowArr�֐��R�A������
int CShareData::GetOpenedWindowArrCore( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	//�ҏW�E�C���h�E�����擾����B
	EditNodeEx *pNode;	// �\�[�g�����p�̊g�����X�g
	int		nRowNum;	//�ҏW�E�C���h�E��
	int		i;

	//�ҏW�E�C���h�E�����擾����B
	*ppEditNode = NULL;
	if( m_pShareData->m_nEditArrNum <= 0 )
		return 0;

	//�ҏW�E�C���h�E���X�g�i�[�̈���쐬����B
	*ppEditNode = new EditNode[ m_pShareData->m_nEditArrNum ];
	if( NULL == *ppEditNode )
		return 0;

	// �g�����X�g���쐬����
	pNode = new EditNodeEx[ m_pShareData->m_nEditArrNum ];
	if( NULL == pNode )
	{
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// �g�����X�g�̊e�v�f�ɕҏW�E�B���h�E���X�g�̊e�v�f�ւ̃|�C���^���i�[����
	nRowNum = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( IsEditWnd( m_pShareData->m_pEditArr[ i ].m_hWnd ) )
		{
			pNode[ nRowNum ].p = &m_pShareData->m_pEditArr[ i ];	// �|�C���^�i�[
			pNode[ nRowNum ].nGroupMru = -1;	// �O���[�v�P�ʂ�MRU�ԍ�������
			nRowNum++;
		}
	}
	if( nRowNum <= 0 )
	{
		delete []pNode;
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// �g�����X�g��ŃO���[�v�P�ʂ�MRU�ԍ�������
	if( !bGSort )
	{
		int iGroupMru = 0;	// �O���[�v�P�ʂ�MRU�ԍ�
		int nGroup = -1;
		for( i = 0; i < nRowNum; i++ )
		{
			if( pNode[ i ].nGroupMru == -1 && nGroup != pNode[ i ].p->m_nGroup )
			{
				nGroup = pNode[ i ].p->m_nGroup;
				iGroupMru++;
				pNode[ i ].nGroupMru = iGroupMru;	// MRU�ԍ��t�^

				// ����O���[�v�̃E�B���h�E�ɓ���MRU�ԍ�������
				int j;
				for( j = i + 1; j < nRowNum; j++ )
				{
					if( pNode[ j ].p->m_nGroup == nGroup )
						pNode[ j ].nGroupMru = iGroupMru;
				}
			}
		}
	}

	// �g�����X�g���\�[�g����
	// Note. �O���[�v���P�����̏ꍇ�͏]���ibGSort ���������j�Ɠ������ʂ�������
	//       �i�O���[�v������ݒ�łȂ���΃O���[�v�͂P�j
	s_bSort = bSort;
	s_bGSort = bGSort;
	qsort( pNode, nRowNum, sizeof(EditNodeEx), cmpGetOpenedWindowArr );

	// �g�����X�g�̃\�[�g���ʂ����ƂɕҏW�E�C���h�E���X�g�i�[�̈�Ɍ��ʂ��i�[����
	for( i = 0; i < nRowNum; i++ )
	{
		(*ppEditNode)[i] = *pNode[i].p;

		//�C���f�b�N�X��t����B
		//���̃C���f�b�N�X�� m_pEditArr �̔z��ԍ��ł��B
		(*ppEditNode)[i].m_nIndex = pNode[i].p - m_pShareData->m_pEditArr;	// �|�C���^���Z���z��ԍ�
	}

	delete []pNode;

	return nRowNum;
}

/** �E�B���h�E�̕��ёւ�

	@param[in] hSrcTab �ړ�����E�B���h�E
	@param[in] hSrcTab �ړ���E�B���h�E

	@author ryoji
	@date 2007.07.07 genta �E�B���h�E�z�񑀍암��CTabWnd���ړ�
*/
bool CShareData::ReorderTab( HWND hwndSrc, HWND hwndDst )
{
	EditNode	*p = NULL;
	int			nCount;
	int			i;

	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<CMutex> guard( g_cEditArrMutex );
	nCount = GetOpenedWindowArrCore( &p, TRUE );	// ���b�N�͎����ł���Ă���̂Œ��ڃR�A���Ăяo��
	for( i = 0; i < nCount; i++ )
	{
		if( hwndSrc == p[i].m_hWnd )
			nSrcTab = i;
		if( hwndDst == p[i].m_hWnd )
			nDstTab = i;
	}

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
	{
		if( p ) delete []p;
		return false;
	}

	// �^�u�̏��������ւ��邽�߂ɃE�B���h�E�̃C���f�b�N�X�����ւ���
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[ nDstTab ].m_nIndex;
	nIndex = m_pShareData->m_pEditArr[ nArr0 ].m_nIndex;
	if( nSrcTab < nDstTab )
	{
		// �^�u���������[�e�[�g
		for( i = nDstTab - 1; i >= nSrcTab; i-- )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	else
	{
		// �^�u�E�������[�e�[�g
		for( i = nDstTab + 1; i <= nSrcTab; i++ )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = nIndex;

	if( p ) delete []p;
	return true;
}

/** �^�u�ړ��ɔ����E�B���h�E����

	@param[in] hwndSrc �ړ�����E�B���h�E
	@param[in] hwndDst �ړ���E�B���h�E�D�V�K�Ɨ�����NULL�D
	@param[in] bSrcIsTop �ړ�����E�B���h�E�����E�B���h�E�Ȃ�true
	@param[in] notifygroups �^�u�̍X�V���K�v�ȃO���[�v�̃O���[�vID�Dint[2]���Ăяo�����ŗp�ӂ���D

	@return �X�V���ꂽhwndDst (�ړ��悪���ɕ���ꂽ�ꍇ�Ȃǂ�NULL�ɕύX����邱�Ƃ�����)

	@author ryoji
	@date 2007.07.07 genta CTabWnd::SeparateGroup()���Ɨ�
*/
HWND CShareData::SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] )
{
	LockGuard<CMutex> guard( g_cEditArrMutex );

	EditNode* pSrcEditNode = GetEditNode( hwndSrc );
	EditNode* pDstEditNode = GetEditNode( hwndDst );
	int nSrcGroup = pSrcEditNode->m_nGroup;
	int nDstGroup;
	if( pDstEditNode == NULL )
	{
		hwndDst = NULL;
		nDstGroup = ++m_pShareData->m_nGroupSequences;	// �V�K�O���[�v
	}
	else
	{
		nDstGroup = pDstEditNode->m_nGroup;	// �����O���[�v
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++m_pShareData->m_nSequences;	// �^�u���т̍Ō�i�N�����̍Ō�j�ɂ����Ă���

	// ��\���̃^�u�������O���[�v�Ɉړ�����Ƃ��͔�\���̂܂܂ɂ���̂�
	// ���������擪�ɂ͂Ȃ�Ȃ��悤�A�K�v�Ȃ�擪�E�B���h�E�ƈʒu����������B
	if( !bSrcIsTop && pDstEditNode != NULL )
	{
		if( pSrcEditNode < pDstEditNode )
		{
			EditNode en = *pDstEditNode;
			*pDstEditNode = *pSrcEditNode;
			*pSrcEditNode = en;
		}
	}
	
	notifygroups[0] = nSrcGroup;
	notifygroups[1] = nDstGroup;
	
	return hwndDst;
}

/*!
	�A�E�g�v�b�g�E�C���h�E�ɏo��

	�A�E�g�v�b�g�E�C���h�E��������΃I�[�v������
	@param lpFmt [in] �����w�蕶����
*/
void CShareData::TraceOut( LPCTSTR lpFmt, ... )
{

	if( NULL == m_pShareData->m_hwndDebug
	|| !IsEditWnd( m_pShareData->m_hwndDebug )
	){
		// 2007.06.26 ryoji
		// �A�E�g�v�b�g�E�B���h�E���쐬���Ɠ����O���[�v�ɍ쐬���邽�߂� m_hwndTraceOutSource ���g���Ă��܂�
		// �im_hwndTraceOutSource �� CEditWnd::Create() �ŗ\�ߐݒ�j
		// ������ƕs���D�����ǁATraceOut() �̈����ɂ��������N�������w�肷��̂��D�D�D
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_SJIS;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( NULL, m_hwndTraceOutSource, sLoadInfo, _T("-DEBUGMODE"), true );
		//	2001/06/23 N.Nakatani �����o��܂ŃE�G�C�g��������悤�ɏC��
		//�A�E�g�v�b�g�E�C���h�E���o����܂�5�b���炢�҂B
		//	Jun. 25, 2001 genta OpenNewEditor�̓����@�\�𗘗p����悤�ɕύX

		/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( m_pShareData->m_hwndDebug );
	}
	va_list argList;
	va_start( argList, lpFmt );
	auto_vsprintf( m_pShareData->GetWorkBuffer<EDIT_CHAR>(), to_wchar(lpFmt), argList );
	va_end( argList );
	::SendMessage( m_pShareData->m_hwndDebug, MYWM_ADDSTRING, 0, 0 );
	return;
}


/*
	CShareData::CheckMRUandOPENFOLDERList
	MRU��OPENFOLDER���X�g�̑��݃`�F�b�N�Ȃ�
	���݂��Ȃ��t�@�C����t�H���_��MRU��OPENFOLDER���X�g����폜����

	@note ���݂͎g���Ă��Ȃ��悤���B
	@par History
	2001.12.26 �폜�����B�iYAZAKI�j
	
*/
/*!	idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����D

	@param pszPath [in]	�p�X���̏o�͐�D�����݂̂�m�肽���Ƃ���NULL������D
	@param idx [in]		�}�N���ԍ�
	@param nBufLen [in]	pszPath�Ŏw�肳�ꂽ�o�b�t�@�̃o�b�t�@�T�C�Y

	@retval >0 : �p�X���̒����D
	@retval  0 : �G���[�C���̃}�N���͎g���Ȃ��C�t�@�C�������w�肳��Ă��Ȃ��D
	@retval <0 : �o�b�t�@�s���D�K�v�ȃo�b�t�@�T�C�Y�� -(�߂�l)+1

	@author YAZAKI
	@date 2003.06.08 Moca ���[�J���ϐ��ւ̃|�C���^��Ԃ��Ȃ��悤�Ɏd�l�ύX
	@date 2003.06.14 genta �����񒷁C�|�C���^�̃`�F�b�N��ǉ�
	@date 2003.06.24 Moca idx��-1�̂Ƃ��A�L�[�}�N���̃t���p�X��Ԃ�.
	
	@note idx�͐��m�Ȃ��̂łȂ���΂Ȃ�Ȃ��B(�����Ő������`�F�b�N���s���Ă��Ȃ�)
*/
int CShareData::GetMacroFilename( int idx, TCHAR *pszPath, int nBufLen )
{
	if( -1 != idx && !m_pShareData->m_MacroTable[idx].IsEnabled() )
		return 0;
	TCHAR *ptr;
	TCHAR *pszFile;

	if( -1 == idx ){
		pszFile = _T("RecKey.mac");
	}else{
		pszFile = m_pShareData->m_MacroTable[idx].m_szFile;
	}
	if( pszFile[0] == _T('\0') ){	//	�t�@�C����������
		if( pszPath != NULL ){
			pszPath[0] = _T('\0');
		}
		return 0;
	}
	ptr = pszFile;
	int nLen = _tcslen( ptr ); // Jul. 21, 2003 genta wcslen�Ώۂ�����Ă������߃}�N�����s���ł��Ȃ�

	if( !_IS_REL_PATH( pszFile )	// ��΃p�X
		|| m_pShareData->m_szMACROFOLDER[0] == _T('\0') ){	//	�t�H���_�w��Ȃ�
		if( pszPath == NULL || nBufLen <= nLen ){
			return -nLen;
		}
		_tcscpy( pszPath, pszFile );
		return nLen;
	}
	else {	//	�t�H���_�w�肠��
		//	���΃p�X����΃p�X
		int nFolderSep = AddLastChar( m_pShareData->m_szMACROFOLDER, _countof2(m_pShareData->m_szMACROFOLDER), _T('\\') );
		int nAllLen;
		TCHAR *pszDir;

		 // 2003.06.24 Moca �t�H���_�����΃p�X�Ȃ���s�t�@�C������̃p�X
		// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		if( _IS_REL_PATH( m_pShareData->m_szMACROFOLDER ) ){
			TCHAR szDir[_MAX_PATH + _countof2( m_pShareData->m_szMACROFOLDER )];
			GetInidirOrExedir( szDir, m_pShareData->m_szMACROFOLDER );
			pszDir = szDir;
		}else{
			pszDir = m_pShareData->m_szMACROFOLDER;
		}

		int nDirLen = _tcslen( pszDir );
		nAllLen = nDirLen + nLen + ( -1 == nFolderSep ? 1 : 0 );
		if( pszPath == NULL || nBufLen <= nAllLen ){
			return -nAllLen;
		}

		_tcscpy( pszPath, pszDir );
		ptr = pszPath + nDirLen;
		if( -1 == nFolderSep ){
			*ptr++ = _T('\\');
		}
		_tcscpy( ptr, pszFile );
		return nAllLen;
	}

}

/*!	idx�Ŏw�肵���}�N����m_bReloadWhenExecute���擾����B
	idx�͐��m�Ȃ��̂łȂ���΂Ȃ�Ȃ��B
	YAZAKI
*/
bool CShareData::BeReloadWhenExecuteMacro( int idx )
{
	if( !m_pShareData->m_MacroTable[idx].IsEnabled() )
		return false;

	return ( m_pShareData->m_MacroTable[idx].m_bReloadWhenExecute == TRUE );
}

/*!	m_aSearchKeys��pszSearchKey��ǉ�����B
	YAZAKI
*/
void CShareData::AddToSearchKeyArr( const wchar_t* pszSearchKey )
{
	CRecentSearch	cRecentSearchKey;
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
}

/*!	m_aReplaceKeys��pszReplaceKey��ǉ�����
	YAZAKI
*/
void CShareData::AddToReplaceKeyArr( const wchar_t* pszReplaceKey )
{
	CRecentReplace	cRecentReplaceKey;
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();

	return;
}

/*!	m_aGrepFiles��pszGrepFile��ǉ�����
	YAZAKI
*/
void CShareData::AddToGrepFileArr( const TCHAR* pszGrepFile )
{
	CRecentGrepFile	cRecentGrepFile;
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_aGrepFolders.size()��pszGrepFolder��ǉ�����
	YAZAKI
*/
void CShareData::AddToGrepFolderArr( const TCHAR* pszGrepFolder )
{
	CRecentGrepFolder	cRecentGrepFolder;
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}

/*!	�O��Win�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool CShareData::ExtWinHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != L'\0'){
		return true;	//	���ʐݒ�ɐݒ肳��Ă���
	}
	if (!nTypeNo.IsValid()) {
		return false;	//	���ʐݒ�ɐݒ肳��Ă��Ȃ���nTypeNo���͈͊O�B
	}
	if (m_pShareData->GetTypeSetting(nTypeNo).m_szExtHelp[0] != L'\0'){
		return true;	//	�^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
const TCHAR* CShareData::GetExtWinHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && m_pShareData->GetTypeSetting(nTypeNo).m_szExtHelp[0] != _T('\0')){
		return m_pShareData->GetTypeSetting(nTypeNo).m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}

/*!	�O��HTML�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool CShareData::ExtHTMLHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	���ʐݒ�ɐݒ肳��Ă���
	}
	if (!nTypeNo.IsValid()){
		return false;	//	���ʐݒ�ɐݒ肳��Ă��Ȃ���nTypeNo���͈͊O�B
	}
	if (nTypeNo->m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	�^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
const TCHAR* CShareData::GetExtHTMLHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && m_pShareData->GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != _T('\0')){
		return m_pShareData->GetTypeSetting(nTypeNo).m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}

/*!	�r���[�A�𕡐��N�����Ȃ���ON����Ԃ��B
*/
bool CShareData::HTMLHelpIsSingle( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && m_pShareData->GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != L'\0'){
		return (m_pShareData->GetTypeSetting(nTypeNo).m_bHtmlHelpIsSingle != FALSE);
	}
	
	return (m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle != FALSE);
}

/*! ���t���t�H�[�}�b�g
	systime�F�����f�[�^
	
	pszDest�F�t�H�[�}�b�g�ς݃e�L�X�g�i�[�p�o�b�t�@
	nDestLen�FpszDest�̒���
	
	pszDateFormat�F
		�J�X�^���̂Ƃ��̃t�H�[�}�b�g
*/
const TCHAR* CShareData::MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen )
{
	return MyGetDateFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nDateFormatType,
		m_pShareData->m_Common.m_sFormat.m_szDateFormat
	);
}

const TCHAR* CShareData::MyGetDateFormat(
	const SYSTEMTIME&		systime,
	TCHAR*		pszDest,
	int				nDestLen,
	int				nDateFormatType,
	const TCHAR*	szDateFormat
)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}



/* �������t�H�[�}�b�g */
const TCHAR* CShareData::MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen )
{
	return MyGetTimeFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType,
		m_pShareData->m_Common.m_sFormat.m_szTimeFormat
	);
}

/* �������t�H�[�}�b�g */
const TCHAR* CShareData::MyGetTimeFormat(
	const SYSTEMTIME&	systime,
	TCHAR*			pszDest,
	int					nDestLen,
	int					nTimeFormatType,
	const TCHAR*		szTimeFormat
)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}



/*!	���L�f�[�^�̐ݒ�ɏ]���ăp�X���k���\�L�ɕϊ�����
	@param pszSrc   [in]  �t�@�C����
	@param pszDest  [out] �ϊ���̃t�@�C�����̊i�[��
	@param nDestLen [in]  �I�[��NULL���܂�pszDest��TCHAR�P�ʂ̒��� _MAX_PATH �܂�
	@date 2003.01.27 Moca �V�K�쐬
	@note �A�����ČĂяo���ꍇ�̂��߁A�W�J�ς݃��^��������L���b�V�����č��������Ă���B
*/
LPTSTR CShareData::GetTransformFileNameFast( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen )
{
	int i;
	TCHAR szBuf[_MAX_PATH + 1];

	if( -1 == m_nTransformFileNameCount ){
		TransformFileName_MakeCache();
	}

	if( 0 < m_nTransformFileNameCount ){
		GetFilePathFormat( pszSrc, pszDest, nDestLen,
			m_szTransformFileNameFromExp[0],
			m_pShareData->m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]]
		);
		for( i = 1; i < m_nTransformFileNameCount; i++ ){
			_tcscpy( szBuf, pszDest );
			GetFilePathFormat( szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				m_pShareData->m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]] );
		}
	}else{
		// �ϊ�����K�v���Ȃ� �R�s�[��������
		_tcsncpy( pszDest, pszSrc, nDestLen - 1 );
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}

/*!	�W�J�ς݃��^������̃L���b�V�����쐬�E�X�V����
	@retval �L���ȓW�J�ςݒu���O������̐�
	@date 2003.01.27 Moca �V�K�쐬
	@date 2003.06.23 Moca �֐����ύX
*/
int CShareData::TransformFileName_MakeCache( void ){
	int i;
	int nCount = 0;
	for( i = 0; i < m_pShareData->m_nTransformFileNameArrNum; i++ ){
		if( L'\0' != m_pShareData->m_szTransformFileNameFrom[i][0] ){
			if( ExpandMetaToFolder( m_pShareData->m_szTransformFileNameFrom[i],
			 m_szTransformFileNameFromExp[nCount], _MAX_PATH ) ){
				// m_szTransformFileNameTo��m_szTransformFileNameFromExp�̔ԍ�������邱�Ƃ�����̂ŋL�^���Ă���
				m_nTransformFileNameOrgId[nCount] = i;
				nCount++;
			}
		}
	}
	m_nTransformFileNameCount = nCount;
	return nCount;
}


/*!	�t�@�C���E�t�H���_����u�����āA�ȈՕ\�������擾����
	@date 2002.11.27 Moca �V�K�쐬
	@note �召��������ʂ��Ȃ��BnDestLen�ɒB�����Ƃ��͌���؂�̂Ă���
*/
LPCTSTR CShareData::GetFilePathFormat( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen, LPCTSTR pszFrom, LPCTSTR pszTo )
{
	int i, j;
	int nSrcLen;
	int nFromLen, nToLen;
	int nCopy;

	nSrcLen  = _tcslen( pszSrc );
	nFromLen = _tcslen( pszFrom );
	nToLen   = _tcslen( pszTo );

	nDestLen--;

	for( i = 0, j = 0; i < nSrcLen && j < nDestLen; i++ ){
#if defined(_MBCS)
		if( 0 == strnicmp( &pszSrc[i], pszFrom, nFromLen ) )
#else
		if( 0 == _tcsncicmp( &pszSrc[i], pszFrom, nFromLen ) )
#endif
		{
			nCopy = __min( nToLen, nDestLen - j );
			memcpy( &pszDest[j], pszTo, nCopy * sizeof( TCHAR ) );
			j += nCopy;
			i += nFromLen - 1;
		}else{
#if defined(_MBCS)
// SJIS ��p����
			if( _IS_SJIS_1( (unsigned char)pszSrc[i] ) && i + 1 < nSrcLen && _IS_SJIS_2( (unsigned char)pszSrc[i + 1] ) ){
				if( j + 1 < nDestLen ){
					pszDest[j] = pszSrc[i];
					j++;
					i++;
				}else{
					// SJIS�̐�s�o�C�g�����R�s�[�����̂�h��
					break;// goto end_of_func;
				}
			}
#endif
			pszDest[j] = pszSrc[i];
			j++;
		}
	}
// end_of_func:;
	pszDest[j] = '\0';
	return pszDest;
}


/*!	%MYDOC%�Ȃǂ̃p�����[�^�w������ۂ̃p�X���ɕϊ�����

	@param pszSrc  [in]  �ϊ��O������
	@param pszDes  [out] �ϊ��㕶����
	@param nDesLen [in]  pszDes��NULL���܂�TCHAR�P�ʂ̒���
	@retval true  ����ɕϊ��ł���
	@retval false �o�b�t�@������Ȃ������C�܂��̓G���[�BpszDes�͕s��
	@date 2002.11.27 Moca �쐬�J�n
*/
bool CShareData::ExpandMetaToFolder( LPCTSTR pszSrc, LPTSTR pszDes, int nDesLen )
{
	LPCTSTR ps;
	LPTSTR  pd, pd_end;

#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias{
		LPTSTR szAlias;
		int nLenth;
		LPTSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  _T("COMDESKTOP"), 10, _T("Common Desktop") },
		{  _T("COMMUSIC"), 8, _T("CommonMusic") },
		{  _T("COMVIDEO"), 8, _T("CommonVideo") },
		{  _T("MYMUSIC"),  7, _T("My Music") },
		{  _T("MYVIDEO"),  7, _T("Video") },
		{  _T("COMPICT"),  7, _T("CommonPictures") },
		{  _T("MYPICT"),   6, _T("My Pictures") },
		{  _T("COMDOC"),   6, _T("Common Documents") },
		{  _T("MYDOC"),    5, _T("Personal") },
		{ NULL, 0 , NULL }
	};
#endif

	pd_end = pszDes + ( nDesLen - 1 );
	for( ps = pszSrc, pd = pszDes; _T('\0') != *ps; ps++ ){
		if( pd_end <= pd ){
			if( pd_end == pd ){
				*pd = _T('\0');
			}
			return false;
		}

		if( _T('%') != *ps ){
			*pd = *ps;
			pd++;
			continue;
		}

		// %% �� %
		if( _T('%') == ps[1] ){
			*pd = _T('%');
			pd++;
			ps++;
			continue;
		}

		if( _T('\0') != ps[1] ){
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			LPCTSTR  pStr;
			ps++;
			// %SAKURA%
			if( 0 == auto_strnicmp( _T("SAKURA%"), ps, 7 ) ){
				// exe�̂���t�H���_
				GetExedir( szPath );
				nMetaLen = 6;
			}
			// %SAKURADATA%	// 2007.06.06 ryoji
			else if( 0 == auto_strnicmp( _T("SAKURADATA%"), ps, 11 ) ){
				// ini�̂���t�H���_
				GetInidir( szPath );
				nMetaLen = 10;
			}
			// ���^��������ۂ�
			else if( NULL != (pStr = _tcschr( ps, _T('%') ) )){
				nMetaLen = pStr - ps;
				if( nMetaLen < _MAX_PATH ){
					auto_memcpy( szMeta, ps, nMetaLen );
					szMeta[nMetaLen] = _T('\0');
				}
				else{
					*pd = _T('\0');
					return false;
				}
#ifdef _USE_META_ALIAS
				// ���^�����񂪃G�C���A�X���Ȃ珑��������
				const MetaAlias* pAlias;
				for( pAlias = &AliasList[0]; nMetaLen < pAlias->nLenth; pAlias++ )
					; // �ǂݔ�΂�
				for( ; nMetaLen == pAlias->nLenth; pAlias++ ){
					if( 0 == auto_stricmp( pAlias->szAlias, szMeta ) ){
						_tcscpy( szMeta, pAlias->szOrig );
						break;
					}
				}
#endif
				// ���ڃ��W�X�g���Œ��ׂ�
				szPath[0] = _T('\0');
				bFolderPath = ReadRegistry( HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, _countof( szPath ) );
				if( false == bFolderPath || _T('\0') == szPath[0] ){
					bFolderPath = ReadRegistry( HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, _countof( szPath ) );
				}
				if( false == bFolderPath || _T('\0') == szPath[0] ){
					pStr = _tgetenv( szMeta );
					// ���ϐ�
					if( NULL != pStr ){
						nPathLen = _tcslen( pStr );
						if( nPathLen < _MAX_PATH ){
							_tcscpy( szPath, pStr );
						}else{
							*pd = _T('\0');
							return false;
						}
					}
					// ����`�̃��^������� ���͂��ꂽ%...%���C���̂܂ܕ����Ƃ��ď�������
					else if(  pd + ( nMetaLen + 2 ) < pd_end ){
						*pd = _T('%');
						auto_memcpy( &pd[1], ps, nMetaLen );
						pd[nMetaLen + 1] = _T('%');
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else{
						*pd = _T('\0');
						return false;
					}
				}
			}else{
				// %...%�̏I����%���Ȃ� �Ƃ肠�����C%���R�s�[
				*pd = _T('%');
				pd++;
				ps--; // ���ps++���Ă��܂����̂Ŗ߂�
				continue;
			}

			// �����O�t�@�C�����ɂ���
			nPathLen = _tcslen( szPath );
			LPTSTR pStr2 = szPath;
			if( nPathLen < _MAX_PATH && 0 != nPathLen ){
				if( FALSE != GetLongFileName( szPath, szMeta ) ){
					pStr2 = szMeta;
				}
			}

			// �Ō�̃t�H���_��؂�L�����폜����
			// [A:\]�Ȃǂ̃��[�g�ł����Ă��폜
			for(nPathLen = 0; pStr2[nPathLen] != _T('\0'); nPathLen++ ){
#ifdef _MBCS
				if( _IS_SJIS_1( (unsigned char)pStr2[nPathLen] ) && _IS_SJIS_2( (unsigned char)pStr2[nPathLen + 1] ) ){
					// SJIS�ǂݔ�΂�
					nPathLen++; // 2003/01/17 sui
				}else
#endif
				if( _T('\\') == pStr2[nPathLen] && _T('\0') == pStr2[nPathLen + 1] ){
					pStr2[nPathLen] = _T('\0');
					break;
				}
			}

			if( pd + nPathLen < pd_end && 0 != nPathLen ){
				auto_memcpy( pd, pStr2, nPathLen );
				pd += nPathLen;
				ps += nMetaLen;
			}else{
				*pd = _T('\0');
				return false;
			}
		}else{
			// �Ō�̕�����%������
			*pd = *ps;
			pd++;
		}
	}
	*pd = _T('\0');
	return true;
}

static const wchar_t* const	ppszKeyWordsCPP[] = {
	L"#define",
	L"#elif",
	L"#else",
	L"#endif",
	L"#error",
	L"#if",
	L"#ifdef",
	L"#ifndef",
	L"#include",
	L"#line",
	L"#pragma",
	L"#undef",
	L"__FILE__",
	L"__declspec",
	L"asm",
	L"auto",
	L"bool",
	L"break",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"const_cast",
	L"continue",
	L"default",
	L"define",
	L"defined",
	L"delete",
	L"do",
	L"double",
	L"dynamic_cast",
	L"elif",
	L"else",
	L"endif",
	L"enum",
	L"error",
	L"explicit",
	L"export",
	L"extern",
	L"false",
	L"float",
	L"for",
	L"friend",
	L"goto",
	L"if",
	L"ifdef",
	L"ifndef",
	L"include",
	L"inline",
	L"int",
	L"line",
	L"long",
	L"mutable",
	L"namespace",
	L"new",
	L"operator",
	L"pragma",
	L"private",
	L"protected",
	L"public",
	L"register",
	L"reinterpret_cast",
	L"return",
	L"short",
	L"signed",
	L"sizeof",
	L"static",
	L"static_cast",
	L"struct",
	L"switch",
	L"template",
	L"this",
	L"throw",
	L"true",
	L"try",
	L"typedef",
	L"typeid",
	L"typename",
	L"undef",
	L"union",
	L"unsigned",
	L"using",
	L"virtual",
	L"void",
	L"volatile",
	L"wchar_t",
	L"while"
};

static const wchar_t* const	ppszKeyWordsHTML[] = {
	L"_blank",
	L"_parent",
	L"_self",
	L"_top",
	L"A",
	L"ABBR",
	L"ABOVE",
	L"absbottom",
	L"absmiddle",
	L"ACCESSKEY",
	L"ACRONYM",
	L"ACTION",
	L"ADDRESS",
	L"ALIGN",
	L"all",
	L"APPLET",
	L"AREA",
	L"AUTOPLAY",
	L"AUTOSTART",
	L"B",
	L"BACKGROUND",
	L"BASE",
	L"BASEFONT",
	L"baseline",
	L"BEHAVIOR",
	L"BELOW",
	L"BGCOLOR",
	L"BGSOUND",
	L"BIG",
	L"BLINK",
	L"BLOCKQUOTE",
	L"BODY",
	L"BORDER",
	L"BORDERCOLOR",
	L"BORDERCOLORDARK",
	L"BORDERCOLORLIGHT",
	L"BOTTOM",
	L"box",
	L"BR",
	L"BUTTON",
	L"CAPTION",
	L"CELLPADDING",
	L"CELLSPACING",
	L"CENTER",
	L"CHALLENGE",
	L"char",
	L"checkbox",
	L"CHECKED",
	L"CITE",
	L"CLEAR",
	L"CLIP",
	L"CODE",
	L"CODEBASE",
	L"CODETYPE",
	L"COL",
	L"COLGROUP",
	L"COLOR",
	L"COLS",
	L"COLSPAN",
	L"COMMENT",
	L"CONTROLS",
	L"DATA",
	L"DD",
	L"DECLARE",
	L"DEFER",
	L"DEL",
	L"DELAY",
	L"DFN",
	L"DIR",
	L"DIRECTION",
	L"DISABLED",
	L"DIV",
	L"DL",
	L"DOCTYPE",
	L"DT",
	L"EM",
	L"EMBED",
	L"ENCTYPE",
	L"FACE",
	L"FIELDSET",
	L"file",
	L"FONT",
	L"FOR",
	L"FORM",
	L"FRAME",
	L"FRAMEBORDER",
	L"FRAMESET",
	L"GET",
	L"groups",
	L"GROUPS",
	L"GUTTER",
	L"H1",
	L"H2",
	L"H3",
	L"H4",
	L"H5",
	L"H6",
	L"H7",
	L"HEAD",
	L"HEIGHT",
	L"HIDDEN",
	L"Hn",
	L"HR",
	L"HREF",
	L"hsides",
	L"HSPACE",
	L"HTML",
	L"I",
	L"ID",
	L"IFRAME",
	L"ILAYER",
	L"image",
	L"IMG",
	L"INDEX",
	L"inherit",
	L"INPUT",
	L"INS",
	L"ISINDEX",
	L"JavaScript",
	L"justify",
	L"KBD",
	L"KEYGEN",
	L"LABEL",
	L"LANGUAGE",
	L"LAYER",
	L"LEFT",
	L"LEGEND",
	L"lhs",
	L"LI",
	L"LINK",
	L"LISTING",
	L"LOOP",
	L"MAP",
	L"MARQUEE",
	L"MAXLENGTH",
	L"MENU",
	L"META",
	L"METHOD",
	L"METHODS",
	L"MIDDLE",
	L"MULTICOL",
	L"MULTIPLE",
	L"NAME",
	L"NEXT",
	L"NEXTID",
	L"NOBR",
	L"NOEMBED",
	L"NOFRAMES",
	L"NOLAYER",
	L"none",
	L"NOSAVE",
	L"NOSCRIPT",
	L"NOTAB",
	L"NOWRAP",
	L"OBJECT",
	L"OL",
	L"onBlur",
	L"onChange",
	L"onClick",
	L"onFocus",
	L"onLoad",
	L"onMouseOut",
	L"onMouseOver",
	L"onReset",
	L"onSelect",
	L"onSubmit",
	L"OPTION",
	L"P",
	L"PAGEX",
	L"PAGEY",
	L"PALETTE",
	L"PANEL",
	L"PARAM",
	L"PARENT",
	L"password",
	L"PLAINTEXT",
	L"PLUGINSPAGE",
	L"POST",
	L"PRE",
	L"PREVIOUS",
	L"Q",
	L"radio",
	L"REL",
	L"REPEAT",
	L"reset",
	L"REV",
	L"rhs",
	L"RIGHT",
	L"rows",
	L"ROWSPAN",
	L"RULES",
	L"S",
	L"SAMP",
	L"SAVE",
	L"SCRIPT",
	L"SCROLLAMOUNT",
	L"SCROLLDELAY",
	L"SELECT",
	L"SELECTED",
	L"SERVER",
	L"SHAPES",
	L"show",
	L"SIZE",
	L"SMALL",
	L"SONG",
	L"SPACER",
	L"SPAN",
	L"SRC",
	L"STANDBY",
	L"STRIKE",
	L"STRONG",
	L"STYLE",
	L"SUB",
	L"submit",
	L"SUMMARY",
	L"SUP",
	L"TABINDEX",
	L"TABLE",
	L"TARGET",
	L"TBODY",
	L"TD",
	L"TEXT",
	L"TEXTAREA",
	L"textbottom",
	L"TEXTFOCUS",
	L"textmiddle",
	L"texttop",
	L"TFOOT",
	L"TH",
	L"THEAD",
	L"TITLE",
	L"TOP",
	L"TR",
	L"TT",
	L"TXTCOLOR",
	L"TYPE",
	L"U",
	L"UL",
	L"URN",
	L"USEMAP",
	L"VALIGN",
	L"VALUE",
	L"VALUETYPE",
	L"VAR",
	L"VISIBILITY",
	L"void",
	L"vsides",
	L"VSPACE",
	L"WBR",
	L"WIDTH",
	L"WRAP",
	L"XMP"
};

static const wchar_t* const	ppszKeyWordsPLSQL[] = {
	L"AND",
	L"AS",
	L"BEGIN",
	L"BINARY_INTEGER",
	L"BODY",
	L"BOOLEAN",
	L"BY",
	L"CHAR",
	L"CHR",
	L"COMMIT",
	L"COUNT",
	L"CREATE",
	L"CURSOR",
	L"DATE",
	L"DECLARE",
	L"DEFAULT",
	L"DELETE",
	L"ELSE",
	L"ELSIF",
	L"END",
	L"ERRORS",
	L"EXCEPTION",
	L"FALSE",
	L"FOR",
	L"FROM",
	L"FUNCTION",
	L"GOTO",
	L"HTP",
	L"IDENT_ARR",
	L"IF",
	L"IN",
	L"INDEX",
	L"INTEGER",
	L"IS",
	L"LOOP",
	L"NOT",
	L"NO_DATA_FOUND",
	L"NULL",
	L"NUMBER",
	L"OF",
	L"OR",
	L"ORDER",
	L"OUT",
	L"OWA_UTIL",
	L"PACKAGE",
	L"PRAGMA",
	L"PRN",
	L"PROCEDURE",
	L"REPLACE",
	L"RESTRICT_REFERENCES",
	L"RETURN",
	L"ROWTYPE",
	L"SELECT",
	L"SHOW",
	L"SUBSTR",
	L"TABLE",
	L"THEN",
	L"TRUE",
	L"TYPE",
	L"UPDATE",
	L"VARCHAR",
	L"VARCHAR2",
	L"WHEN",
	L"WHERE",
	L"WHILE",
	L"WNDS",
	L"WNPS",
	L"RAISE",
	L"INSERT",
	L"INTO",
	L"VALUES",
	L"SET",
	L"SYSDATE",
	L"RTRIM",
	L"LTRIM",
	L"TO_CHAR",
	L"DUP_VAL_ON_INDEX",
	L"ROLLBACK",
	L"OTHERS",
	L"SQLCODE"
};

//Jul. 10, 2001 JEPRO �ǉ�
static const wchar_t* const	ppszKeyWordsCOBOL[] = {
	L"ACCEPT",
	L"ADD",
	L"ADVANCING",
	L"AFTER",
	L"ALL",
	L"AND",
	L"ARGUMENT",
	L"ASSIGN",
	L"AUTHOR",
	L"BEFORE",
	L"BLOCK",
	L"BY",
	L"CALL",
	L"CHARACTERS",
	L"CLOSE",
	L"COMP",
	L"COMPILED",
	L"COMPUTE",
	L"COMPUTER",
	L"CONFIGURATION",
	L"CONSOLE",
	L"CONTAINS",
	L"CONTINUE",
	L"CONTROL",
	L"COPY",
	L"DATA",
	L"DELETE",
	L"DISPLAY",
	L"DIVIDE",
	L"DIVISION",
	L"ELSE",
	L"END",
	L"ENVIRONMENT",
	L"EVALUATE",
	L"EXAMINE",
	L"EXIT",
	L"EXTERNAL",
	L"FD",
	L"FILE",
	L"FILLER",
	L"FROM",
	L"GIVING",
	L"GO",
	L"GOBACK",
	L"HIGH-VALUE",
	L"IDENTIFICATION"
	L"IF",
	L"INITIALIZE",
	L"INPUT",
	L"INTO",
	L"IS",
	L"LABEL",
	L"LINKAGE",
	L"LOW-VALUE",
	L"MODE",
	L"MOVE",
	L"NOT",
	L"OBJECT",
	L"OCCURS",
	L"OF",
	L"ON",
	L"OPEN",
	L"OR",
	L"OTHER",
	L"OUTPUT",
	L"PERFORM",
	L"PIC",
	L"PROCEDURE",
	L"PROGRAM",
	L"READ",
	L"RECORD",
	L"RECORDING",
	L"REDEFINES",
	L"REMAINDER",
	L"REMARKS",
	L"REPLACING",
	L"REWRITE",
	L"ROLLBACK",
	L"SECTION",
	L"SELECT",
	L"SOURCE",
	L"SPACE",
	L"STANDARD",
	L"STOP",
	L"STORAGE",
	L"SYSOUT",
	L"TEST",
	L"THEN",
	L"TO",
	L"TODAY",
	L"TRANSFORM",
	L"UNTIL",
	L"UPON",
	L"USING",
	L"VALUE",
	L"VARYING",
	L"WHEN",
	L"WITH",
	L"WORKING",
	L"WRITE",
	L"WRITTEN",
	L"ZERO"
};

static const wchar_t*	ppszKeyWordsJAVA[] = {
	L"abstract",
	L"assert",	// Mar. 8, 2003 genta
	L"boolean",
	L"break",
	L"byte",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"continue",
	L"default",
	L"do",
	L"double",
	L"else",
	L"extends",
	L"final",
	L"finally",
	L"float",
	L"for",
	L"goto",
	L"if",
	L"implements",
	L"import",
	L"instanceof",
	L"int",
	L"interface",
	L"long",
	L"native",
	L"new",
	L"package",
	L"private",
	L"protected",
	L"public",
	L"return",
	L"short",
	L"static",
	L"strictfp",	// Mar. 8, 2003 genta
	L"super",
	L"switch",
	L"synchronized",
	L"this",
	L"throw",
	L"throws",
	L"transient",
	L"try",
	L"void",
	L"volatile",
	L"while"
};

static const wchar_t* const	ppszKeyWordsCORBA_IDL[] = {
	L"any",
	L"attribute",
	L"boolean",
	L"case",
	L"char",
	L"const",
	L"context",
	L"default",
	L"double",
	L"enum",
	L"exception",
	L"FALSE",
	L"fixed",
	L"float",
	L"in",
	L"inout",
	L"interface",
	L"long",
	L"module",
	L"Object",
	L"octet",
	L"oneway",
	L"out",
	L"raises",
	L"readonly",
	L"sequence",
	L"short",
	L"string",
	L"struct",
	L"switch",
	L"TRUE",
	L"typedef",
	L"unsigned",
	L"union",
	L"void",
	L"wchar_t",
	L"wstring"
};

static const wchar_t* const	ppszKeyWordsAWK[] = {
	L"BEGIN",
	L"END",
	L"next",
	L"exit",
	L"func",
	L"function",
	L"return",
	L"if",
	L"else",
	L"for",
	L"in",
	L"do",
	L"while",
	L"break",
	L"continue",
	L"$0",
	L"$1",
	L"$2",
	L"$3",
	L"$4",
	L"$5",
	L"$6",
	L"$7",
	L"$8",
	L"$9",
	L"$10",
	L"$11",
	L"$12",
	L"$13",
	L"$14",
	L"$15",
	L"$16",
	L"$17",
	L"$18",
	L"$19",
	L"$20",
	L"FS",
	L"OFS",
	L"NF",
	L"RS",
	L"ORS",
	L"NR",
	L"FNR",
	L"ARGV",
	L"ARGC",
	L"ARGIND",
	L"FILENAME",
	L"ENVIRON",
	L"ERRNO",
	L"OFMT",
	L"CONVFMT",
	L"FIELDWIDTHS",
	L"IGNORECASE",
	L"RLENGTH",
	L"RSTART",
	L"SUBSEP",
	L"delete",
	L"index",
	L"jindex",
	L"length",
	L"jlength",
	L"substr",
	L"jsubstr",
	L"match",
	L"split",
	L"sub",
	L"gsub",
	L"sprintf",
	L"tolower",
	L"toupper",
	L"print",
	L"printf",
	L"getline",
	L"system",
	L"close",
	L"sin",
	L"cos",
	L"atan2",
	L"exp",
	L"log",
	L"int",
	L"sqrt",
	L"srand",
	L"rand",
	L"strftime",
	L"systime"
};

static const wchar_t*	ppszKeyWordsBAT[] = {
	L"PATH",
	L"PROMPT",
	L"TEMP",
	L"TMP",
	L"TZ",
	L"CONFIG",
	L"COMSPEC",
	L"DIRCMD",
	L"COPYCMD",
	L"winbootdir",
	L"windir",
	L"DIR",
	L"CALL",
	L"CHCP",
	L"RENAME",
	L"REN",
	L"ERASE",
	L"DEL",
	L"TYPE",
	L"REM",
	L"COPY",
	L"PAUSE",
	L"DATE",
	L"TIME",
	L"VER",
	L"VOL",
	L"CD",
	L"CHDIR",
	L"MD",
	L"MKDIR",
	L"RD",
	L"RMDIR",
	L"BREAK",
	L"VERIFY",
	L"SET",
	L"EXIT",
	L"CTTY",
	L"ECHO",
	L"@ECHO",	//Oct. 31, 2000 JEPRO '@' �������\�ɂ����̂Œǉ�
	L"LOCK",
	L"UNLOCK",
	L"GOTO",
	L"SHIFT",
	L"IF",
	L"FOR",
	L"DO",	//Nov. 2, 2000 JEPRO �ǉ�
	L"IN",	//Nov. 2, 2000 JEPRO �ǉ�
	L"ELSE",	//Nov. 2, 2000 JEPRO �ǉ� Win2000�Ŏg����
	L"CLS",
	L"TRUENAME",
	L"LOADHIGH",
	L"LH",
	L"LFNFOR",
	L"ON",
	L"OFF",
	L"NOT",
	L"ERRORLEVEL",
	L"EXIST",
	L"NUL",
	L"CON",
	L"AUX",
	L"COM1",
	L"COM2",
	L"COM3",
	L"COM4",
	L"PRN",
	L"LPT1",
	L"LPT2",
	L"LPT3",
	L"CLOCK",
	L"CLOCK$",
	L"CONFIG$"
};

static const wchar_t*	ppszKeyWordsPASCAL[] = {
	L"and",
	L"exports",
	L"mod",
	L"shr",
	L"array",
	L"file",
	L"nil",
	L"string",
	L"as",
	L"finalization",
	L"not",
	L"stringresource",
	L"asm",
	L"finally",
	L"object",
	L"then",
	L"begin",
	L"for",
	L"of",
	L"case",
	L"function",
	L"or",
	L"to",
	L"class",
	L"goto",
	L"out",
	L"try",
	L"const",
	L"if",
	L"packed",
	L"type",
	L"constructor",
	L"implementation",
	L"procedure",
	L"unit",
	L"destructor",
	L"in",
	L"program",
	L"until",
	L"dispinterface",
	L"inherited",
	L"property",
	L"uses",
	L"div",
	L"initialization",
	L"raise",
	L"var",
	L"do",
	L"inline",
	L"record",
	L"while",
	L"downto",
	L"interface",
	L"repeat",
	L"with",
	L"else",
	L"is",
	L"resourcestring",
	L"xor",
	L"end",
	L"label",
	L"set",
	L"except",
	L"library",
	L"shl",
	L"private",
	L"public",
	L"published",
	L"protected",
	L"override"
};

static const wchar_t*	ppszKeyWordsTEX[] = {
//Nov. 20, 2000 JEPRO	�啝�ǉ� & �኱�C���E�폜 --�قƂ�ǃR�}���h�̂�
	L"error",
	L"Warning",
//			"center",
//			"document",
//			"enumerate",
//			"eqnarray",
//			"figure",
//			"itemize",
//			"minipage",
//			"tabbing",
//			"table",
//			"tabular",
//			"\\!",
//			"\\#",
//			"\\%",
//			"\\&",
//			"\\(",
//			"\\)",
//			"\\+",
//			"\\,",
//			"\\-",
//			"\\:",
//			"\\;",
//			"\\<",
//			"\\=",
//			"\\>",
	L"\\aa",
	L"\\AA",
	L"\\acute",
	L"\\addcontentsline",
	L"\\addtocounter",
	L"\\addtolength",
	L"\\ae",
	L"\\AE",
	L"\\aleph",
	L"\\alpha",
	L"\\alph",
	L"\\Alph",
	L"\\and",
	L"\\angle",
	L"\\appendix",
	L"\\approx",
	L"\\arabic",
	L"\\arccos",
	L"\\arctan",
	L"\\arg",
	L"\\arrayrulewidth",
	L"\\arraystretch",
	L"\\ast",
	L"\\atop",
	L"\\author",
	L"\\b",
	L"\\backslash",
	L"\\bar",
	L"\\baselineskip",
	L"\\baselinestretch",
	L"\\begin",
	L"\\beta",
	L"\\bf",
	L"\\bibitem",
	L"\\bibliography",
	L"\\bibliographystyle",
	L"\\big",
	L"\\Big",
	L"\\bigcap",
	L"\\bigcirc",
	L"\\bigcup",
	L"\\bigg",
	L"\\Bigg",
	L"\\Biggl",
	L"\\Biggm",
	L"\\biggl",
	L"\\biggm",
	L"\\biggr",
	L"\\Biggr",
	L"\\bigl",
	L"\\bigm",
	L"\\Bigm",
	L"\\Bigl",
	L"\\bigodot",
	L"\\bigoplus",
	L"\\bigotimes",
	L"\\bigr",
	L"\\Bigr",
	L"\\bigskip",
	L"\\bigtriangledown",
	L"\\bigtriangleup",
	L"\\boldmath",
	L"\\bot",
	L"\\Box",
	L"\\brace",
	L"\\breve",
	L"\\bullet",
	L"\\bye",
	L"\\c",
	L"\\cal",
	L"\\cap",
	L"\\caption",
	L"\\cc",
	L"\\cdot",
	L"\\cdots",
	L"\\centering",
	L"\\chapter",
	L"\\check",
	L"\\chi",
	L"\\choose",
	L"\\circ",
	L"\\circle",
	L"\\cite",
	L"\\clearpage",
	L"\\cline",
	L"\\closing",
	L"\\clubsuit",
	L"\\colon",
	L"\\columnsep",
	L"\\columnseprule",
	L"\\cong",
	L"\\cot",
	L"\\coth",
	L"\\cr",
	L"\\cup",
	L"\\d",
	L"\\dag",
	L"\\dagger",
	L"\\date",
	L"\\dashbox",
	L"\\ddag",
	L"\\ddot",
	L"\\ddots",
	L"\\def",
	L"\\deg",
	L"\\delta",
	L"\\Delta",
	L"\\det",
	L"\\diamond",
	L"\\diamondsuit",
	L"\\dim",
	L"\\displaystyle",
	L"\\documentclass",
	L"\\documentstyle",
	L"\\dot",
	L"\\doteq",
	L"\\dotfill",
	L"\\Downarrow",
	L"\\downarrow",
	L"\\ell",
	L"\\em",
	L"\\emptyset",
	L"\\encl",
	L"\\end",
	L"\\enspace",
	L"\\enskip",
	L"\\epsilon",
	L"\\eqno",
	L"\\equiv",
	L"\\evensidemargin",
	L"\\eta",
	L"\\exists",
	L"\\exp",
	L"\\fbox",
	L"\\fboxrule",
	L"\\flat",
	L"\\footnote",
	L"\\footnotesize",
	L"\\forall",
	L"\\frac",
	L"\\frame",
	L"\\framebox",
	L"\\gamma",
	L"\\Gamma",
	L"\\gcd",
	L"\\ge",
	L"\\geq",
	L"\\gets",
	L"\\gg",
	L"\\grave",
	L"\\gt",
	L"\\H",
	L"\\hat",
	L"\\hbar",
	L"\\hbox",
	L"\\headsep",
	L"\\heartsuit",
	L"\\hfil",
	L"\\hfill",
	L"\\hline",
	L"\\hom",
	L"\\hrulefill",
	L"\\hskip",
	L"\\hspace",
	L"\\hspace*",
	L"\\huge",
	L"\\Huge",
	L"\\i",
	L"\\Im",
	L"\\imath",
	L"\\in",
	L"\\include",
	L"\\includegraphics",
	L"\\includeonly",
	L"\\indent",
	L"\\index",
	L"\\inf",
	L"\\infty",
	L"\\input",
	L"\\int",
	L"\\iota",
	L"\\it",
	L"\\item",
	L"\\itemsep",
	L"\\j",
	L"\\jmath",
	L"\\kappa",
	L"\\ker",
	L"\\kern",
	L"\\kill",
	L"\\l",
	L"\\L",
	L"\\label",
	L"\\lambda",
	L"\\Lambda",
	L"\\land",
	L"\\langle",
	L"\\large",
	L"\\Large",
	L"\\LARGE",
	L"\\LaTeX",
	L"\\LaTeXe",
	L"\\lceil",
	L"\\ldots",
	L"\\le",
	L"\\leftarrow",
	L"\\Leftarrow",
	L"\\lefteqn",
	L"\\leftharpoondown",
	L"\\leftharpoonup",
	L"\\leftmargin",
	L"\\leftrightarrow",
	L"\\Leftrightarrow",
	L"\\leq",
	L"\\leqno",
	L"\\lfloor",
	L"\\lg",
	L"\\lim",
	L"\\liminf",
	L"\\limsup",
	L"\\line",
	L"\\linebreak",
	L"\\linewidth",
	L"\\listoffigures",
	L"\\listoftables",
	L"\\ll",
	L"\\llap",
	L"\\ln",
	L"\\lnot",
	L"\\log",
	L"\\longleftarrow",
	L"\\Longleftarrow",
	L"\\longleftrightarrow",
	L"\\Longleftrightarrow",
	L"\\longrightarrow",
	L"\\Longrightarrow",
	L"\\lor",
	L"\\lower",
	L"\\magstep",
	L"\\makeatletter",
	L"\\makeatother",
	L"\\makebox",
	L"\\makeindex",
	L"\\maketitle",
	L"\\makelabels",
	L"\\mathop",
	L"\\mapsto",
	L"\\markboth",
	L"\\markright",
	L"\\mathstrut",
	L"\\max",
	L"\\mbox",
	L"\\mc",
	L"\\medskip",
	L"\\mid",
	L"\\min",
	L"\\mit",
	L"\\mp",
	L"\\mu",
	L"\\multicolumn",
	L"\\multispan",
	L"\\multiput",
	L"\\nabla",
	L"\\natural",
	L"\\ne",
	L"\\neg",
	L"\\nearrow",
	L"\\nwarrow",
	L"\\neq",
	L"\\newblock",
	L"\\newcommand",
	L"\\newenvironment",
	L"\\newfont",
	L"\\newlength",
	L"\\newline",
	L"\\newpage",
	L"\\newtheorem",
	L"\\ni",
	L"\\noalign",
	L"\\noindent",
	L"\\nolimits",
	L"\\nolinebreak",
	L"\\nonumber",
	L"\\nopagebreak",
	L"\\normalsize",
	L"\\not",
	L"\\notice",
	L"\\notin",
	L"\\nu",
	L"\\o",
	L"\\O",
	L"\\oddsidemargin",
	L"\\odot",
	L"\\oe",
	L"\\OE",
	L"\\oint",
	L"\\Omega",
	L"\\omega",
	L"\\ominus",
	L"\\oplus",
	L"\\opening",
	L"\\otimes",
	L"\\owns",
	L"\\overleftarrow",
	L"\\overline",
	L"\\overrightarrow",
	L"\\overvrace",
	L"\\oval",
	L"\\P",
	L"\\pagebreak",
	L"\\pagenumbering",
	L"\\pageref",
	L"\\pagestyle",
	L"\\par",
	L"\\parallel",
	L"\\paragraph",
	L"\\parbox",
	L"\\parindent",
	L"\\parskip",
	L"\\partial",
	L"\\perp",
	L"\\phi",
	L"\\Phi",
	L"\\pi",
	L"\\Pi",
	L"\\pm",
	L"\\Pr",
	L"\\prime",
	L"\\printindex",
	L"\\prod",
	L"\\propto",
	L"\\ps",
	L"\\psi",
	L"\\Psi",
	L"\\put",
	L"\\qquad",
	L"\\quad",
	L"\\raisebox",
	L"\\rangle",
	L"\\rceil",
	L"\\Re",
	L"\\ref",
	L"\\renewcommand",
	L"\\renewenvironment",
	L"\\rfloor",
	L"\\rho",
	L"\\right",
	L"\\rightarrow",
	L"\\Rightarrow",
	L"\\rightharpoondown",
	L"\\rightharpoonup",
	L"\\rightleftharpoonup",
	L"\\rightmargin",
	L"\\rm",
	L"\\rule",
	L"\\roman",
	L"\\Roman",
	L"\\S",
	L"\\samepage",
	L"\\sb",
	L"\\sc",
	L"\\scriptsize",
	L"\\scriptscriptstyle",
	L"\\scriptstyle",
	L"\\searrow",
	L"\\sec",
	L"\\section",
	L"\\setcounter",
	L"\\setlength",
	L"\\settowidth",
	L"\\setminus",
	L"\\sf",
	L"\\sharp",
	L"\\sigma",
	L"\\Sigma",
	L"\\signature",
	L"\\sim",
	L"\\simeq",
	L"\\sin",
	L"\\sinh",
	L"\\sl",
	L"\\sloppy",
	L"\\small",
	L"\\smash",
	L"\\smallskip",
	L"\\sp",
	L"\\spadesuit",
	L"\\special",
	L"\\sqrt",
	L"\\ss",
	L"\\star",
	L"\\stackrel",
	L"\\strut",
	L"\\subparagraph",
	L"\\subsection",
	L"\\subset",
	L"\\subseteq",
	L"\\subsubsection",
	L"\\sum",
	L"\\sup",
	L"\\supset",
	L"\\supseteq",
	L"\\swarrow",
	L"\\t",
	L"\\tableofcontents",
	L"\\tan",
	L"\\tanh",
	L"\\tau",
	L"\\TeX",
	L"\\textbf",
	L"\\textgreater",
	L"\\textgt",
	L"\\textheight",
	L"\\textit",
	L"\\textless",
	L"\\textmc",
	L"\\textrm",
	L"\\textsc",
	L"\\textsf",
	L"\\textsl",
	L"\\textstyle",
	L"\\texttt",
	L"\\textwidth",
	L"\\thanks",
	L"\\thebibliography",
	L"\\theequation",
	L"\\thepage",
	L"\\thesection",
	L"\\theta",
	L"\\Theta",
	L"\\thicklines",
	L"\\thinlines",
	L"\\thinspace",
	L"\\thisepage",
	L"\\thisepagestyle",
	L"\\tie",
	L"\\tilde",
	L"\\times",
	L"\\tiny",
	L"\\title",
	L"\\titlepage",
	L"\\to",
	L"\\toaddress",
	L"\\topmargin",
	L"\\triangle",
	L"\\tt",
	L"\\twocolumn",
	L"\\u",
	L"\\underline",
	L"\\undervrace",
	L"\\unitlength",
	L"\\Uparrow",
	L"\\uparrow",
	L"\\updownarrow",
	L"\\Updownarrow",
	L"\\uplus",
	L"\\upsilon",
	L"\\Upsilon",
	L"\\usepackage",
	L"\\v",
	L"\\varepsilon",
	L"\\varphi",
	L"\\varpi",
	L"\\varrho",
	L"\\varsigma",
	L"\\vartheta",
	L"\\vbox",
	L"\\vcenter",
	L"\\vec",
	L"\\vector",
	L"\\vee",
	L"\\verb",
	L"\\verb*",
	L"\\verbatim",
	L"\\vert",
	L"\\Vert",
	L"\\vfil",
	L"\\vfill",
	L"\\vrule",
	L"\\vskip",
	L"\\vspace",
	L"\\vspace*",
	L"\\wedge",
	L"\\widehat",
	L"\\widetilde",
	L"\\wp",
	L"\\wr",
	L"\\wrapfigure",
	L"\\xi",
	L"\\Xi",
	L"\\zeta"//,
//			"\\[",
//			"\\\"",
//			"\\\'",
//			"\\\\",
//			"\\]",
//			"\\^",
//			"\\_",
//			"\\`",
//			"\\{",
//			"\\|",
//			"\\}",
//			"\\~",
};

//Jan. 19, 2001 JEPRO	TeX �̃L�[���[�h2�Ƃ��ĐV�K�ǉ� & �ꕔ���� --���R�}���h�ƃI�v�V�����������S
static const wchar_t*	ppszKeyWordsTEX2[] = {
	//	���R�}���h
	//Jan. 19, 2001 JEPRO �{����{}�t���ŃL�[���[�h�ɂ������������P��Ƃ��ĔF�����Ă���Ȃ��̂Ŏ~�߂�
	L"abstract",
	L"array"
	L"center",
	L"description",
	L"document",
	L"displaymath",
	L"em",
	L"enumerate",
	L"eqnarray",
	L"eqnarray*",
	L"equation",
	L"figure",
	L"figure*",
	L"floatingfigure",
	L"flushleft",
	L"flushright",
	L"itemize",
	L"letter",
	L"list",
	L"math",
	L"minipage",
	L"multicols",
	L"namelist",
	L"picture",
	L"quotation",
	L"quote",
	L"sloppypar",
	L"subeqnarray",
	L"subeqnarray*",
	L"subequations",
	L"subfigure",
	L"tabbing",
	L"table",
	L"table*",
	L"tabular",
	L"tabular*",
	L"tatepage",
	L"thebibliography",
	L"theindex",
	L"titlepage",
	L"trivlist",
	L"verbatim",
	L"verbatim*",
	L"verse",
	L"wrapfigure",
	//	�X�^�C���I�v�V����
	L"a4",
	L"a4j",
	L"a5",
	L"a5j",
	L"Alph",
	L"alph",
	L"annote",
	L"arabic",
	L"b4",
	L"b4j",
	L"b5",
	L"b5j",
	L"bezier",
	L"booktitle",
	L"boxedminipage",
	L"boxit",
//		"bp",
//		"cm",
	L"dbltopnumber",
//		"dd",
	L"eclepsf",
	L"eepic",
	L"enumi",
	L"enumii",
	L"enumiii",
	L"enumiv",
	L"epic",
	L"epsbox",
	L"epsf",
	L"fancybox",
	L"fancyheadings",
	L"fleqn",
	L"footnote",
	L"howpublished",
	L"jabbrv",
	L"jalpha",
//		"article",
	L"jarticle",
	L"jsarticle",
//		"book",
	L"jbook",
	L"jsbook",
//		"letter",
	L"jletter",
//		"plain",
	L"jplain",
//		"report",
	L"jreport",
	L"jtwocolumn",
	L"junsrt",
	L"leqno",
	L"makeidx",
	L"markboth",
	L"markright",
//		"mm",
	L"multicol",
	L"myheadings",
	L"openbib",
//		"pc",
//		"pt",
	L"secnumdepth",
//		"sp",
	L"titlepage",
	L"tjarticle",
	L"topnumber",
	L"totalnumber",
	L"twocolumn",
	L"twoside",
	L"yomi"//,
//		"zh",
//		"zw"
};

static const wchar_t*	ppszKeyWordsPERL[] = {
	//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
	L"break",
	L"continue",
	L"do",
	L"elsif",
	L"else",
	L"for",
	L"foreach",
	L"goto",
	L"if",
	L"last",
	L"next",
	L"return",
	L"sub",
	L"undef",
	L"unless",
	L"until",
	L"while",
	L"abs",
	L"accept",
	L"alarm",
	L"atan2",
	L"bind",
	L"binmode",
	L"bless",
	L"caller",
	L"chdir",
	L"chmod",
	L"chomp",
	L"chop",
	L"chown",
	L"chr",
	L"chroot",
	L"close",
	L"closedir",
	L"connect",
	L"continue",
	L"cos",
	L"crypt",
	L"dbmclose",
	L"dbmopen",
	L"defined",
	L"delete",
	L"die",
	L"do",
	L"dump",
	L"each",
	L"eof",
	L"eval",
	L"exec",
	L"exists",
	L"exit",
	L"exp",
	L"fcntl",
	L"fileno",
	L"flock",
	L"fork",
	L"format",
	L"formline",
	L"getc",
	L"getlogin",
	L"getpeername",
	L"getpgrp",
	L"getppid",
	L"getpriority",
	L"getpwnam",
	L"getgrnam",
	L"gethostbyname",
	L"getnetbyname",
	L"getprotobyname",
	L"getpwuid",
	L"getgrgid",
	L"getservbyname",
	L"gethostbyaddr",
	L"getnetbyaddr",
	L"getprotobynumber",
	L"getservbyport",
	L"getpwent",
	L"getgrent",
	L"gethostent",
	L"getnetent",
	L"getprotoent",
	L"getservent",
	L"setpwent",
	L"setgrent",
	L"sethostent",
	L"setnetent",
	L"setprotoent",
	L"setservent",
	L"endpwent",
	L"endgrent",
	L"endhostent",
	L"endnetent",
	L"endprotoent",
	L"endservent",
	L"getsockname",
	L"getsockopt",
	L"glob",
	L"gmtime",
	L"goto",
	L"grep",
	L"hex",
	L"import",
	L"index",
	L"int",
	L"ioctl",
	L"join",
	L"keys",
	L"kill",
	L"last",
	L"lc",
	L"lcfirst",
	L"length",
	L"link",
	L"listen",
	L"local",
	L"localtime",
	L"log",
	L"lstat",
//			"//m",
	L"map",
	L"mkdir",
	L"msgctl",
	L"msgget",
	L"msgsnd",
	L"msgrcv",
	L"my",
	L"next",
	L"no",
	L"oct",
	L"open",
	L"opendir",
	L"ord",
	L"our",	// 2006.04.20 genta
	L"pack",
	L"package",
	L"pipe",
	L"pop",
	L"pos",
	L"print",
	L"printf",
	L"prototype",
	L"push",
//			"//q",
	L"qq",
	L"qr",
	L"qx",
	L"qw",
	L"quotemeta",
	L"rand",
	L"read",
	L"readdir",
	L"readline",
	L"readlink",
	L"readpipe",
	L"recv",
	L"redo",
	L"ref",
	L"rename",
	L"require",
	L"reset",
	L"return",
	L"reverse",
	L"rewinddir",
	L"rindex",
	L"rmdir",
//			"//s",
	L"scalar",
	L"seek",
	L"seekdir",
	L"select",
	L"semctl",
	L"semget",
	L"semop",
	L"send",
	L"setpgrp",
	L"setpriority",
	L"setsockopt",
	L"shift",
	L"shmctl",
	L"shmget",
	L"shmread",
	L"shmwrite",
	L"shutdown",
	L"sin",
	L"sleep",
	L"socket",
	L"socketpair",
	L"sort",
	L"splice",
	L"split",
	L"sprintf",
	L"sqrt",
	L"srand",
	L"stat",
	L"study",
	L"sub",
	L"substr",
	L"symlink",
	L"syscall",
	L"sysopen",
	L"sysread",
	L"sysseek",
	L"system",
	L"syswrite",
	L"tell",
	L"telldir",
	L"tie",
	L"tied",
	L"time",
	L"times",
	L"tr",
	L"truncate",
	L"uc",
	L"ucfirst",
	L"umask",
	L"undef",
	L"unlink",
	L"unpack",
	L"untie",
	L"unshift",
	L"use",
	L"utime",
	L"values",
	L"vec",
	L"wait",
	L"waitpid",
	L"wantarray",
	L"warn",
	L"write"
};

//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
static const wchar_t*	ppszKeyWordsPERL2[] = {
	L"$ARGV",
	L"$_",
	L"$1",
	L"$2",
	L"$3",
	L"$4",
	L"$5",
	L"$6",
	L"$7",
	L"$8",
	L"$9",
	L"$0",
	L"$MATCH",
	L"$&",
	L"$PREMATCH",
	L"$`",
	L"$POSTMATCH",
	L"$'",
	L"$LAST_PAREN_MATCH",
	L"$+",
	L"$MULTILINE_MATCHING",
	L"$*",
	L"$INPUT_LINE_NUMBER",
	L"$NR",
	L"$.",
	L"$INPUT_RECORD_SEPARATOR",
	L"$RS",
	L"$/",
	L"$OUTPUT_AUTOFLUSH",
	L"$|",
	L"$OUTPUT_FIELD_SEPARATOR",
	L"$OFS",
	L"$,",
	L"$OUTPUT_RECORD_SEPARATOR",
	L"$ORS",
	L"$\\",
	L"$LIST_SEPARATOR",
	L"$\"",
	L"$SUBSCRIPT_SEPARATOR",
	L"$SUBSEP",
	L"$;",
	L"$OFMT",
	L"$#",
	L"$FORMAT_PAGE_NUMBER",
	L"$%",
	L"$FORMAT_LINES_PER_PAGE",
	L"$=",
	L"$FORMAT_LINES_LEFT",
	L"$-",
	L"$FORMAT_NAME",
	L"$~",
	L"$FORMAT_TOP_NAME",
	L"$^",
	L"$FORMAT_LINE_BREAK_CHARACTERS",
	L"$:",
	L"$FORMAT_FORMFEED",
	L"$^L",
	L"$ACCUMULATOR",
	L"$^A",
	L"$CHILD_ERROR",
	L"$?",
	L"$OS_ERROR",
	L"$ERRNO",
	L"$!",
	L"$EVAL_ERROR",
	L"$@",
	L"$PROCESS_ID",
	L"$PID",
	L"$$",
	L"$REAL_USER_ID",
	L"$UID",
	L"$<",
	L"$EFFECTIVE_USER_ID",
	L"$EUID",
	L"$>",
	L"$REAL_GROUP_ID",
	L"$GID",
	L"$(",
	L"$EFFECTIVE_GROUP_ID",
	L"$EGID",
	L"$)",
	L"$PROGRAM_NAME",
	L"$0",
	L"$[",
	L"$PERL_VERSION",
	L"$]",
	L"$DEBUGGING",
	L"$^D",
	L"$SYSTEM_FD_MAX",
	L"$^F",
	L"$INPLACE_EDIT",
	L"$^I",
	L"$PERLDB",
	L"$^P",
	L"$BASETIME",
	L"$^T",
	L"$WARNING",
	L"$^W",
	L"$EXECUTABLE_NAME",
	L"$^X",
	L"$ARGV",
	L"$ENV",
	L"$SIG"
};

//Jul. 10, 2001 JEPRO �ǉ�
static const wchar_t*	ppszKeyWordsVB[] = {
	L"And",
	L"As",
	L"Attribute",
	L"Begin",
	L"BeginProperty",
	L"Boolean",
	L"ByVal",
	L"Byte",
	L"Call",
	L"Case",
	L"Const",
	L"Currency",
	L"Date",
	L"Declare",
	L"Dim",
	L"Do",
	L"Double",
	L"Each",
	L"Else",
	L"ElseIf",
	L"Empty",
	L"End",
	L"EndProperty",
	L"Error",
	L"Eqv",
	L"Exit",
	L"False",
	L"For",
	L"Friend",
	L"Function",
	L"Get",
	L"GoTo",
	L"If",
	L"Imp",
	L"Integer",
	L"Is",
	L"Let",
	L"Like",
	L"Long",
	L"Loop",
	L"Me",
	L"Mod",
	L"New",
	L"Next",
	L"Not",
	L"Null",
	L"Object",
	L"On",
	L"Option",
	L"Or",
	L"Private",
	L"Property",
	L"Public",
	L"RSet",
	L"ReDim",
	L"Rem",
	L"Resume",
	L"Select",
	L"Set",
	L"Single",
	L"Static",
	L"Step",
	L"Stop",
	L"String",
	L"Sub",
	L"Then",
	L"To",
	L"True",
	L"Type",
	L"Wend",
	L"While",
	L"With",
	L"Xor",
	L"#If",
	L"#Else",
	L"#End",
	L"#Const",
	L"AddressOf",
	L"Alias",
	L"Append",
	L"Array",
	L"ByRef",
	L"Explicit",
	L"Global",
	L"In",
	L"Lib",
	L"Nothing",
	L"Optional",
	L"Output",
	L"Terminate",
	L"Until",
	//=========================================================
	// �ȉ���VB.NET(VB7)�ł̔p�~�����肵�Ă���L�[���[�h�ł�
	//=========================================================
	L"DefBool",
	L"DefByte",
	L"DefCur",
	L"DefDate",
	L"DefDbl",
	L"DefInt",
	L"DefLng",
	L"DefObj",
	L"DefSng",
	L"DefStr",
	L"DefVar",
	L"LSet",
	L"GoSub",
	L"Return",
	L"Variant",
	//			"Option Base
	//			"As Any
	//=========================================================
	// �ȉ���VB.NET�p�L�[���[�h�ł�
	//=========================================================
	//BitAnd
	//BitOr
	//BitNot
	//BitXor
	//Delegate
	//Short
	//Structure
};

//Jul. 10, 2001 JEPRO �ǉ�
static const wchar_t*	ppszKeyWordsVB2[] = {
	L"AppActivate",
	L"Beep",
	L"BeginTrans",
	L"ChDir",
	L"ChDrive",
	L"Close",
	L"CommitTrans",
	L"CompactDatabase",
	L"Date",
	L"DeleteSetting",
	L"Erase",
	L"FileCopy",
	L"FreeLocks",
	L"Input",
	L"Kill",
	L"Load",
	L"Lock",
	L"Mid",
	L"MidB",
	L"MkDir",
	L"Name",
	L"Open",
	L"Print",
	L"Put",
	L"Randomize",
	L"RegisterDatabase",
	L"RepairDatabase",
	L"Reset",
	L"RmDir",
	L"Rollback",
	L"SavePicture",
	L"SaveSetting",
	L"Seek",
	L"SendKeys",
	L"SetAttr",
	L"SetDataAccessOption",
	L"SetDefaultWorkspace",
	L"Time",
	L"Unload",
	L"Unlock",
	L"Width",
	L"Write",
	L"Array",
	L"Asc",
	L"AscB",
	L"Atn",
	L"CBool",
	L"CByte",
	L"CCur",
	L"CDate",
	L"CDbl",
	L"CInt",
	L"CLng",
	L"CSng",
	L"CStr",
	L"CVErr",
	L"CVar",
	L"Choose",
	L"Chr",
	L"ChrB",
	L"Command",
	L"Cos",
	L"CreateDatabase",
	L"CreateObject",
	L"CurDir",
	L"DDB",
	L"Date",
	L"DateAdd",
	L"DateDiff",
	L"DatePart",
	L"DateSerial",
	L"DateValue",
	L"Day",
	L"Dir",
	L"DoEvents",
	L"EOF",
	L"Environ",
	L"Error",
	L"Exp",
	L"FV",
	L"FileAttr",
	L"FileDateTime",
	L"FileLen",
	L"Fix",
	L"Format",
	L"FreeFile",
	L"GetAllSettings",
	L"GetAttr",
	L"GetObject",
	L"GetSetting",
	L"Hex",
	L"Hour",
	L"IIf",
	L"IMEStatus",
	L"IPmt",
	L"IRR",
	L"InStr",
	L"Input",
	L"Int",
	L"IsArray",
	L"IsDate",
	L"IsEmpty",
	L"IsError",
	L"IsMissing",
	L"IsNull",
	L"IsNumeric",
	L"IsObject",
	L"LBound",
	L"LCase",
	L"LOF",
	L"LTrim",
	L"Left",
	L"LeftB",
	L"Len",
	L"LoadPicture",
	L"Loc",
	L"Log",
	L"MIRR",
	L"Mid",
	L"MidB",
	L"Minute",
	L"Month",
	L"MsgBox",
	L"NPV",
	L"NPer",
	L"Now",
	L"Oct",
	L"OpenDatabase",
	L"PPmt",
	L"PV",
	L"Partition",
	L"Pmt",
	L"QBColor",
	L"RGB",
	L"RTrim",
	L"Rate",
	L"ReadProperty",
	L"Right",
	L"RightB",
	L"Rnd",
	L"SLN",
	L"SYD",
	L"Second",
	L"Seek",
	L"Sgn",
	L"Shell",
	L"Sin",
	L"Space",
	L"Spc",
	L"Sqr",
	L"Str",
	L"StrComp",
	L"StrConv",
	L"Switch",
	L"Tab",
	L"Tan",
	L"Time",
	L"TimeSerial",
	L"TimeValue",
	L"Timer",
	L"Trim",
	L"TypeName",
	L"UBound",
	L"UCase",
	L"Val",
	L"VarType",
	L"Weekday",
	L"Year",
	L"Hide",
	L"Line",
	L"Refresh",
	L"Show",
	//=========================================================
	// �ȉ���VB.NET(VB7)�ł̔p�~�����肵�Ă���L�[���[�h�ł�
	//=========================================================
	//$�t���֐��e��
	L"Dir$",
	L"LCase$",
	L"Left$",
	L"LeftB$",
	L"Mid$",
	L"MidB$",
	L"RightB$",
	L"Right$",
	L"Space$",
	L"Str$",
	L"String$",
	L"Trim$",
	L"UCase$",
	//VB5,6�̉B���֐�
	L"VarPtr",
	L"StrPtr",
	L"ObjPtr",
	L"VarPrtArray",
	L"VarPtrStringArray"
};

//Jul. 10, 2001 JEPRO �ǉ�
static const wchar_t*	ppszKeyWordsRTF[] = {
	L"\\ansi",
	L"\\b",
	L"\\bin",
	L"\\box",
	L"\\brdrb",
	L"\\brdrbar",
	L"\\brdrdb",
	L"\\brdrdot",
	L"\\brdrl",
	L"\\brdrr",
	L"\\brdrs",
	L"\\brdrsh",
	L"\\brdrt",
	L"\\brdrth",
	L"\\cell",
	L"\\cellx",
	L"\\cf",
	L"\\chftn",
	L"\\clmgf",
	L"\\clmrg",
	L"\\colortbl",
	L"\\deff",
	L"\\f",
	L"\\fi",
	L"\\field",
	L"\\fldrslt",
	L"\\fonttbl",
	L"\\footnote",
	L"\\fs",
	L"\\i"
	L"\\intbl",
	L"\\keep",
	L"\\keepn",
	L"\\li",
	L"\\line",
	L"\\mac",
	L"\\page",
	L"\\par",
	L"\\pard",
	L"\\pc",
	L"\\pich",
	L"\\pichgoal",
	L"\\picscalex",
	L"\\picscaley",
	L"\\pict",
	L"\\picw",
	L"\\picwgoal",
	L"\\plain",
	L"\\qc",
	L"\\ql",
	L"\\qr",
	L"\\ri",
	L"\\row",
	L"\\rtf",
	L"\\sa",
	L"\\sb",
	L"\\scaps",
	L"\\sect",
	L"\\sl",
	L"\\strike",
	L"\\tab",
	L"\\tqc",
	L"\\tqr",
	L"\\trgaph",
	L"\\trleft",
	L"\\trowd",
	L"\\trqc",
	L"\\trql",
	L"\\tx",
	L"\\ul",
	L"\\uldb",
	L"\\v",
	L"\\wbitmap",
	L"\\wbmbitspixel",
	L"\\wbmplanes",
	L"\\wbmwidthbytes",
	L"\\wmetafile",
	L"bmc",
	L"bml",
	L"bmr",
	L"emc",
	L"eml",
	L"emr"
};

/*!	@brief ���L������������/�����L�[���[�h

	�����L�[���[�h�֘A�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
		�L�[���[�h��`���֐��̊O�ɏo���C�o�^���}�N�������ĊȌ��ɁD
*/
void CShareData::InitKeyword(DLLSHAREDATA* pShareData)
{
	/* �����L�[���[�h�̃e�X�g�f�[�^ */
	pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,ary) \
	pShareData->m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, _countof(ary), (ary) );
	
	PopulateKeyword( L"C/C++", TRUE, ppszKeyWordsCPP );			/* �Z�b�g 0�̒ǉ� */
	PopulateKeyword( L"HTML", FALSE, ppszKeyWordsHTML );			/* �Z�b�g 1�̒ǉ� */
	PopulateKeyword( L"PL/SQL", FALSE, ppszKeyWordsPLSQL );		/* �Z�b�g 2�̒ǉ� */
	PopulateKeyword( L"COBOL", TRUE ,ppszKeyWordsCOBOL );		/* �Z�b�g 3�̒ǉ� */
	PopulateKeyword( L"Java", TRUE, ppszKeyWordsJAVA );			/* �Z�b�g 4�̒ǉ� */
	PopulateKeyword( L"CORBA IDL", TRUE, ppszKeyWordsCORBA_IDL );/* �Z�b�g 5�̒ǉ� */
	PopulateKeyword( L"AWK", TRUE, ppszKeyWordsAWK );			/* �Z�b�g 6�̒ǉ� */
	PopulateKeyword( L"MS-DOS batch", FALSE, ppszKeyWordsBAT );	/* �Z�b�g 7�̒ǉ� */	//Oct. 31, 2000 JEPRO '�o�b�`�t�@�C��'��'batch' �ɒZ�k
	PopulateKeyword( L"Pascal", FALSE, ppszKeyWordsPASCAL );		/* �Z�b�g 8�̒ǉ� */	//Nov. 5, 2000 JEPRO ��E�������̋�ʂ�'���Ȃ�'�ɕύX
	PopulateKeyword( L"TeX", TRUE, ppszKeyWordsTEX );			/* �Z�b�g 9�̒ǉ� */	//Sept. 2, 2000 jepro Tex ��TeX �ɏC�� Bool�l�͑�E�������̋��
	PopulateKeyword( L"TeX2", TRUE, ppszKeyWordsTEX2 );			/* �Z�b�g10�̒ǉ� */	//Jan. 19, 2001 JEPRO �ǉ�
	PopulateKeyword( L"Perl", TRUE, ppszKeyWordsPERL );			/* �Z�b�g11�̒ǉ� */
	PopulateKeyword( L"Perl2", TRUE, ppszKeyWordsPERL2 );		/* �Z�b�g12�̒ǉ� */	//Jul. 10, 2001 JEPRO Perl����ϐ��𕪗��E�Ɨ�
	PopulateKeyword( L"Visual Basic", FALSE, ppszKeyWordsVB );	/* �Z�b�g13�̒ǉ� */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"Visual Basic2", FALSE, ppszKeyWordsVB2 );	/* �Z�b�g14�̒ǉ� */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"���b�`�e�L�X�g", TRUE, ppszKeyWordsRTF );	/* �Z�b�g15�̒ǉ� */	//Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}

/*!	@brief ���L������������/�L�[���蓖��

	�f�t�H���g�L�[���蓖�Ċ֘A�̏���������

	@date 2005.01.30 genta CShareData::Init()���番��
	@date 2007.11.04 genta �L�[�ݒ萔��DLLSHARE�̗̈�𒴂�����N���ł��Ȃ��悤��
*/
bool CShareData::InitKeyAssign(DLLSHAREDATA* pShareData)
{
	/********************/
	/* ���ʐݒ�̋K��l */
	/********************/
	struct KEYDATAINIT {
		short			nKeyCode;		//!< Key Code (0 for non-keybord button)
		TCHAR*			pszKeyName;		//!< Key Name (for display)
		EFunctionCode	nFuncCode_0;	//!<                      Key
		EFunctionCode	nFuncCode_1;	//!< Shift +              Key
		EFunctionCode	nFuncCode_2;	//!<         Ctrl +       Key
		EFunctionCode	nFuncCode_3;	//!< Shift + Ctrl +       Key
		EFunctionCode	nFuncCode_4;	//!<                Alt + Key
		EFunctionCode	nFuncCode_5;	//!< Shift +        Alt + Key
		EFunctionCode	nFuncCode_6;	//!<         Ctrl + Alt + Key
		EFunctionCode	nFuncCode_7;	//!< Shift + Ctrl + Alt + Key
	};
	static KEYDATAINIT	KeyDataInit[] = {
	//Sept. 1, 2000 Jepro note: key binding
	//Feb. 17, 2001 jepro note 2: ���Ԃ�2�i�ŉ���3�r�b�g[Alt][Ctrl][Shift]�̑g�����̏�(�����2���������l)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)
	//		keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
	//
		/* �}�E�X�{�^�� */
		{ 0, _T("�_�u���N���b�N"),		F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD }, //Feb. 19, 2001 JEPRO Alt�ƉE�N���b�N�̑g�����͌����Ȃ��̂ŉE�N���b�N���j���[�̃L�[���蓖�Ă��͂�����
		{ 0, _T("�E�N���b�N"),			F_MENU_RBUTTON,	F_MENU_RBUTTON,	F_MENU_RBUTTON,	F_MENU_RBUTTON,	F_0,			F_0,			F_0,			F_0 },
		{ 0, _T("���N���b�N"),			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0 }, // novice 2004/10/11 �}�E�X���{�^���Ή�
		{ 0, _T("���T�C�h�N���b�N"),	F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0 }, // novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
		{ 0, _T("�E�T�C�h�N���b�N"),	F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0 },
	// 2007.11.15 nasukoji	�g���v���N���b�N�E�N�A�h���v���N���b�N�Ή�
		{ 0, _T("�g���v���N���b�N"), F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE },
		{ 0, _T("�N�A�h���v���N���b�N"), F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL },

		/* �t�@���N�V�����L�[ */
	//	From Here Sept. 14, 2000 JEPRO
	//	VK_F1,_T("F1"), F_EXTHTMLHELP, F_0, F_EXTHELP1, 0, 0, 0, 0, 0,
	//	Shift+F1 �Ɂu�R�}���h�ꗗ�v, Alt+F1 �Ɂu�w���v�ڎ��v, Shift+Alt+F1 �Ɂu�L�[���[�h�����v��ǉ�	//Nov. 25, 2000 JEPRO �E���Ă����̂��C���E����
	//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 �Ɂu�o�[�W�������v��ǉ�
	//	{ VK_F1,_T("F1"), F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F1,_T("F1"), F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_ABOUT, F_HELP_CONTENTS, F_HELP_SEARCH, F_0, F_0 },
	//	To Here Sept. 14, 2000
	// From Here 2001.12.03 hor F2�Ƀu�b�N�}�[�N�֘A������
	//	{ VK_F2,_T("F2"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F2,_T("F2"), F_BOOKMARK_NEXT, F_BOOKMARK_PREV, F_BOOKMARK_SET, F_BOOKMARK_RESET, F_BOOKMARK_VIEW, F_0, F_0, F_0 },
	// To Here 2001.12.03 hor
		//Sept. 21, 2000 JEPRO	Ctrl+F3 �Ɂu�����}�[�N�̃N���A�v��ǉ�
		//Aug. 12, 2002 ai	Ctrl+Shift+F3 �Ɂu�����J�n�ʒu�֖߂�v��ǉ�
		{ VK_F3,_T("F3"), F_SEARCH_NEXT, F_SEARCH_PREV, F_SEARCH_CLEARMARK, F_JUMP_SRCHSTARTPOS, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Alt+F4 �Ɂu�E�B���h�E�����v, Shift+Alt+F4 �Ɂu���ׂẴE�B���h�E�����v��ǉ�
		//	Ctrl+F4�Ɋ��蓖�Ă��Ă����u�c���ɕ����v���u����(����)�v�ɕύX�� Shift+Ctrl+F4 �Ɂu���ĊJ���v��ǉ�
		//Jan. 14, 2001 Ctrl+Alt+F4 �Ɂu�e�L�X�g�G�f�B�^�̑S�I���v��ǉ�
		//Jun. 2001�u�T�N���G�f�B�^�̑S�I���v�ɉ���
		//2006.10.21 ryoji Alt+F4 �ɂ͉������蓖�ĂȂ��i�f�t�H���g�̃V�X�e���R�}���h�u����v�����s�����悤�Ɂj
		//2007.02.13 ryoji Shift+Ctrl+F4��F_WIN_CLOSEALL����F_EXITALLEDITORS�ɕύX
		//2007.02.22 ryoji Ctrl+F4 �ւ̊��蓖�Ă��폜�i�f�t�H���g�̃R�}���h�����s�j
		{ VK_F4,_T("F4"), F_SPLIT_V, F_SPLIT_H, F_0, F_FILECLOSE_OPEN, F_0, F_EXITALLEDITORS, F_EXITALL, F_0 },
	//	From Here Sept. 20, 2000 JEPRO Ctrl+F5 �Ɂu�O���R�}���h���s�v��ǉ�  �Ȃ��}�N������CMMAND ����COMMAND �ɕύX�ς�
	//	{ VK_F5,_T("F5"), F_PLSQL_COMPILE_ON_SQLPLUS, F_0, F_EXECCOMMAND_DIALOG, F_0, F_0, F_0, F_0, F_0 },
	//	To Here Sept. 20, 2000
		//Oct. 28, 2000 F5 �́u�ĕ`��v�ɕύX	//Jan. 14, 2001 Alt+F5 �Ɂuuudecode���ĕۑ��v, Ctrl+ Alt+F5 �ɁuTAB���󔒁v��ǉ�
		//	May 28, 2001 genta	S-C-A-F5��SPACE-to-TAB��ǉ�
		{ VK_F5,_T("F5"), F_REDRAW, F_0, F_EXECCOMMAND_DIALOG, F_0, F_UUDECODE, F_0, F_TABTOSPACE, F_SPACETOTAB },
		//Jan. 14, 2001 JEPRO	Ctrl+F6 �Ɂu�������v, Alt+F6 �ɁuBase64�f�R�[�h���ĕۑ��v��ǉ�
		{ VK_F6,_T("F6"), F_BEGIN_SEL, F_BEGIN_BOX, F_TOLOWER, F_0, F_BASE64DECODE, F_0, F_0, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+F7 �Ɂu�啶���v, Alt+F7 �ɁuUTF-7��SJIS�R�[�h�ϊ��v, Shift+Alt+F7 �ɁuSJIS��UTF-7�R�[�h�ϊ��v, Ctrl+Alt+F7 �ɁuUTF-7�ŊJ�������v��ǉ�
		{ VK_F7,_T("F7"), F_CUT, F_0, F_TOUPPER, F_0, F_CODECNV_UTF72SJIS, F_CODECNV_SJIS2UTF7, F_FILE_REOPEN_UTF7, F_0 },
		//Nov. 9, 2000 JEPRO	Shift+F8 �ɁuCRLF���s�ŃR�s�[�v��ǉ�
		//Jan. 14, 2001 JEPRO	Ctrl+F8 �Ɂu�S�p�����p�v, Alt+F8 �ɁuUTF-8��SJIS�R�[�h�ϊ��v, Shift+Alt+F8 �ɁuSJIS��UTF-8�R�[�h�ϊ��v, Ctrl+Alt+F8 �ɁuUTF-8�ŊJ�������v��ǉ�
		{ VK_F8,_T("F8"), F_COPY, F_COPY_CRLF, F_TOHANKAKU, F_0, F_CODECNV_UTF82SJIS, F_CODECNV_SJIS2UTF8, F_FILE_REOPEN_UTF8, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+F9 �Ɂu���p�{�S�Ђ灨�S�p�E�J�^�J�i�v, Alt+F9 �ɁuUnicode��SJIS�R�[�h�ϊ��v, Ctrl+Alt+F9 �ɁuUnicode�ŊJ�������v��ǉ�
		{ VK_F9,_T("F9"), F_PASTE, F_PASTEBOX, F_TOZENKAKUKATA, F_0, F_CODECNV_UNICODE2SJIS, F_0, F_FILE_REOPEN_UNICODE, F_0 },
		//Oct. 28, 2000 JEPRO F10 �ɁuSQL*Plus�Ŏ��s�v��ǉ�(F5����̈ړ�)
		//Jan. 14, 2001 JEPRO	Ctrl+F10 �Ɂu���p�{�S�J�^���S�p�E�Ђ炪�ȁv, Alt+F10 �ɁuEUC��SJIS�R�[�h�ϊ��v, Shift+Alt+F10 �ɁuSJIS��EUC�R�[�h�ϊ��v, Ctrl+Alt+F10 �ɁuEUC�ŊJ�������v��ǉ�
		{ VK_F10,_T("F10"), F_PLSQL_COMPILE_ON_SQLPLUS, F_DUPLICATELINE, F_TOZENKAKUHIRA, F_0, F_CODECNV_EUC2SJIS, F_CODECNV_SJIS2EUC, F_FILE_REOPEN_EUC, F_0 },
		//Jan. 14, 2001 JEPRO	Shift+F11 �ɁuSQL*Plus���A�N�e�B�u�\���v, Ctrl+F11 �Ɂu���p�J�^�J�i���S�p�J�^�J�i�v, Alt+F11 �ɁuE-Mail(JIS��SJIS)�R�[�h�ϊ��v, Shift+Alt+F11 �ɁuSJIS��JIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuJIS�ŊJ�������v��ǉ�
		{ VK_F11,_T("F11"), F_OUTLINE, F_ACTIVATE_SQLPLUS, F_HANKATATOZENKAKUKATA, F_0, F_CODECNV_EMAIL, F_CODECNV_SJIS2JIS, F_FILE_REOPEN_JIS, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+F12 �Ɂu���p�J�^�J�i���S�p�Ђ炪�ȁv, Alt+F12 �Ɂu�������ʁ�SJIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuSJIS�ŊJ�������v��ǉ�
		{ VK_F12,_T("F12"), F_TAGJUMP, F_TAGJUMPBACK, F_HANKATATOZENKAKUHIRA, F_0, F_CODECNV_AUTO2SJIS, F_0, F_FILE_REOPEN_SJIS, F_0 },
		{ VK_F13,_T("F13"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F14,_T("F14"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F15,_T("F15"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F16,_T("F16"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F17,_T("F17"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F18,_T("F18"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F19,_T("F19"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F20,_T("F20"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F21,_T("F21"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F22,_T("F22"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F23,_T("F23"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F24,_T("F24"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		/* ����L�[ */
		{ VK_TAB,_T("Tab"),F_INDENT_TAB, F_UNINDENT_TAB, F_NEXTWINDOW, F_PREVWINDOW, F_0, F_0, F_0, F_0 },
		//Sept. 1, 2000 JEPRO	Alt+Enter �Ɂu�t�@�C���̃v���p�e�B�v��ǉ�	//Oct. 15, 2000 JEPRO Ctrl+Enter �Ɂu�t�@�C�����e��r�v��ǉ�
			{ VK_RETURN,_T("Enter"),F_0, F_0, F_COMPARE, F_0, F_PROPERTY_FILE, F_0, F_0, F_0 },
		{ VK_ESCAPE,_T("Esc"),F_CANCEL_MODE, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_BACK,_T("BackSpace"),F_DELETE_BACK, F_0, F_WordDeleteToStart, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO �����̂Ŗ��̂��ȗ��`�ɕύX(BackSpace��BkSp)
		{ VK_BACK,_T("BkSp"),F_DELETE_BACK, F_0, F_WordDeleteToStart, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_INSERT,_T("Insert"),F_CHGMOD_INS, F_PASTE, F_COPY, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Insert��Ins)
		{ VK_INSERT,_T("Ins"),F_CHGMOD_INS, F_PASTE, F_COPY, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_DELETE,_T("Delete"),F_DELETE, F_0, F_WordDeleteToEnd, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Delete��Del)
		//Jun. 26, 2001 JEPRO	Shift+Del �Ɂu�؂���v��ǉ�
		{ VK_DELETE,_T("Del"),F_DELETE, F_CUT, F_WordDeleteToEnd, F_0, F_0, F_0, F_0, F_0 },
		{ VK_HOME,_T("Home"),F_GOLINETOP, F_GOLINETOP_SEL, F_GOFILETOP, F_GOFILETOP_SEL, F_0, F_0, F_0, F_0 },
		{ VK_END,_T("End(Help)"),F_GOLINEEND, F_GOLINEEND_SEL, F_GOFILEEND, F_GOFILEEND_SEL, F_0, F_0, F_0, F_0 },
		{ VK_LEFT,_T("��"),F_LEFT, F_LEFT_SEL/*F_GOLINETOP*/, F_WORDLEFT, F_WORDLEFT_SEL, F_BEGIN_BOX, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+���Ɂu�c�����ɍő剻�v��ǉ�
//			{ VK_UP,_T("��"),F_UP, F_UP_SEL, F_UP2, F_UP2_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_V },
		//Jun. 27, 2001 JEPRO
		//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\����ړ�(�Q�s����)�v���u�e�L�X�g���P�s���փX�N���[���v�ɕύX
		{ VK_UP,_T("��"),F_UP, F_UP_SEL, F_WndScrollDown, F_UP2_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_V },
		//2001.02.10 by MIK Shift+Ctrl+Alt+���Ɂu�������ɍő剻�v��ǉ�
		{ VK_RIGHT,_T("��"),F_RIGHT, F_RIGHT_SEL/*F_GOLINEEND*/, F_WORDRIGHT, F_WORDRIGHT_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_H },
		//Sept. 14, 2000 JEPRO
		//	Ctrl+���Ɋ��蓖�Ă��Ă����u�E�N���b�N���j���[�v���u�J�[�\�����ړ�(�Q�s����)�v�ɕύX
		//	����ɕt�����Ă���Ɂu�E�N���b�N���j���[�v��Ctrl�{Alt�{���ɕύX
//			{ VK_DOWN,_T("��"),F_DOWN, F_DOWN_SEL, F_DOWN2, F_DOWN2_SEL, F_BEGIN_BOX, F_0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
		//Jun. 27, 2001 JEPRO
		//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\�����ړ�(�Q�s����)�v���u�e�L�X�g���P�s��փX�N���[���v�ɕύX
		{ VK_DOWN,_T("��"),F_DOWN, F_DOWN_SEL, F_WndScrollUp, F_DOWN2_SEL, F_BEGIN_BOX, F_0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
//			{ VK_PRIOR,_T("RollDown(PageUp)"),F_ROLLDOWN, F_ROLLDOWN_SEL, F_0, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_NEXT,_T("RollUp(PageDown)"),F_ROLLUP, F_ROLLUP_SEL, F_0, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�_�E���v, �u(�I��)�P�y�[�W�_�E���v��ǉ�
		//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollUp��PgDn) //Oct. 10, 2000 JEPRO ���̕ύX
		//2001.12.03 hor 1Page/HalfPage ���ւ�
		{ VK_NEXT,_T("PgDn(RollUp)"), F_1PageDown, F_1PageDown_Sel,F_HalfPageDown, F_HalfPageDown_Sel, F_0, F_0, F_0, F_0 },
		//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�A�b�v�v, �u(�I��)�P�y�[�W�A�b�v�v��ǉ�
		//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollDown��PgUp) //Oct. 10, 2000 JEPRO ���̕ύX
		//2001.12.03 hor 1Page/HalfPage ���ւ�
		{ VK_PRIOR,_T("PgUp(RollDn)"), F_1PageUp, F_1PageUp_Sel,F_HalfPageUp, F_HalfPageUp_Sel, F_0, F_0, F_0, F_0 },
//			{ VK_SPACE,_T("SpaceBar"),F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(SpaceBar��Space)
		{ VK_SPACE,_T("Space"),F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		/* ���� */
		//Oct. 7, 2000 JEPRO	Ctrl+0 ���u�^�C�v�ʐݒ�ꗗ�v���u����`�v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+0 �Ɂu�J�X�^�����j���[10�v, Shift+Alt+0 �Ɂu�J�X�^�����j���[20�v��ǉ�
		{ '0', _T("0"),F_0, F_0, F_0, F_0, F_CUSTMENU_10, F_CUSTMENU_20, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+1 ���u�^�C�v�ʐݒ�v���u�c�[���o�[�̕\���v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+1 �Ɂu�J�X�^�����j���[1�v, Shift+Alt+1 �Ɂu�J�X�^�����j���[11�v��ǉ�
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 �Ɂu�J�X�^�����j���[21�v��ǉ�
		{ '1', _T("1"),F_0, F_0, F_SHOWTOOLBAR, F_CUSTMENU_21, F_CUSTMENU_1, F_CUSTMENU_11, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+2 ���u���ʐݒ�v���u�t�@���N�V�����L�[�̕\���v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+2 ���u�A�E�g�v�b�g�v���u�J�X�^�����j���[2�v�ɕύX���u�A�E�g�v�b�g�v�� Alt+O �Ɉړ�, Shift+Alt+2 �Ɂu�J�X�^�����j���[12�v��ǉ�
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 �Ɂu�J�X�^�����j���[22�v��ǉ�
		{ '2', _T("2"),F_0, F_0, F_SHOWFUNCKEY, F_CUSTMENU_22, F_CUSTMENU_2/*F_WIN_OUTPUT*/, F_CUSTMENU_12, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+3 ���u�t�H���g�ݒ�v���u�X�e�[�^�X�o�[�̕\���v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+3 �Ɂu�J�X�^�����j���[3�v, Shift+Alt+3 �Ɂu�J�X�^�����j���[13�v��ǉ�
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 �Ɂu�J�X�^�����j���[23�v��ǉ�
		{ '3', _T("3"),F_0, F_0, F_SHOWSTATUSBAR, F_CUSTMENU_23, F_CUSTMENU_3, F_CUSTMENU_13, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+4 ���u�c�[���o�[�̕\���v���u�^�C�v�ʐݒ�ꗗ�v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+4 �Ɂu�J�X�^�����j���[4�v, Shift+Alt+4 �Ɂu�J�X�^�����j���[14�v��ǉ�
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 �Ɂu�J�X�^�����j���[24�v��ǉ�
		{ '4', _T("4"),F_0, F_0, F_TYPE_LIST, F_CUSTMENU_24, F_CUSTMENU_4, F_CUSTMENU_14, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+5 ���u�t�@���N�V�����L�[�̕\���v���u�^�C�v�ʐݒ�v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+5 �Ɂu�J�X�^�����j���[5�v, Shift+Alt+5 �Ɂu�J�X�^�����j���[15�v��ǉ�
		{ '5', _T("5"),F_0, F_0, F_OPTION_TYPE, F_0, F_CUSTMENU_5, F_CUSTMENU_15, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+6 ���u�X�e�[�^�X�o�[�̕\���v���u���ʐݒ�v�ɕύX
		//Jan. 13, 2001 JEPRO	Alt+6 �Ɂu�J�X�^�����j���[6�v, Shift+Alt+6 �Ɂu�J�X�^�����j���[16�v��ǉ�
		{ '6', _T("6"),F_0, F_0, F_OPTION, F_0, F_CUSTMENU_6, F_CUSTMENU_16, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+7 �Ɂu�t�H���g�ݒ�v��ǉ�
		//Jan. 13, 2001 JEPRO	Alt+7 �Ɂu�J�X�^�����j���[7�v, Shift+Alt+7 �Ɂu�J�X�^�����j���[17�v��ǉ�
		{ '7', _T("7"),F_0, F_0, F_FONT, F_0, F_CUSTMENU_7, F_CUSTMENU_17, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+8 �Ɂu�J�X�^�����j���[8�v, Shift+Alt+8 �Ɂu�J�X�^�����j���[18�v��ǉ�
		{ '8', _T("8"),F_0, F_0, F_0, F_0, F_CUSTMENU_8, F_CUSTMENU_18, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+9 �Ɂu�J�X�^�����j���[9�v, Shift+Alt+9 �Ɂu�J�X�^�����j���[19�v��ǉ�
		{ '9', _T("9"),F_0, F_0, F_0, F_0, F_CUSTMENU_9, F_CUSTMENU_19, F_0, F_0 },
		/* �A���t�@�x�b�g */
		//2001.12.06 hor Alt+A ���uSORT_ASC�v�Ɋ���
		{ 'A', _T("A"),F_0, F_0, F_SELECTALL, F_0, F_SORT_ASC, F_0, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Ctrl+B �Ɂu�u���E�Y�v��ǉ�
		{ 'B', _T("B"),F_0, F_0, F_BROWSE, F_0, F_0, F_0, F_0, F_0 },
		//Jan. 16, 2001 JEPRO	SHift+Ctrl+C �Ɂu.h�Ɠ�����.c(�Ȃ����.cpp)���J���v��ǉ�
		//Feb. 07, 2001 JEPRO	SHift+Ctrl+C ���u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
		{ 'C', _T("C"),F_0, F_0, F_COPY, F_OPEN_HfromtoC, F_0, F_0, F_0, F_0 },
		//Jan. 16, 2001 JEPRO	Ctrl+D �Ɂu�P��؂���v, Shift+Ctrl+D �Ɂu�P��폜�v��ǉ�
		//2001.12.06 hor Alt+D ���uSORT_DESC�v�Ɋ���
		{ 'D', _T("D"),F_0, F_0, F_WordCut, F_WordDelete, F_SORT_DESC, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+Alt+E �Ɂu�d�˂ĕ\���v��ǉ�
		//Jan. 16, 2001	JEPRO	Ctrl+E �Ɂu�s�؂���(�܂�Ԃ��P��)�v, Shift+Ctrl+E �Ɂu�s�폜(�܂�Ԃ��P��)�v��ǉ�
		{ 'E', _T("E"),F_0, F_0, F_CUT_LINE, F_DELETE_LINE, F_0, F_0, F_CASCADE, F_0 },
		{ 'F', _T("F"),F_0, F_0, F_SEARCH_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'G', _T("G"),F_0, F_0, F_GREP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 07, 2000 JEPRO	Ctrl+Alt+H �Ɂu�㉺�ɕ��ׂĕ\���v��ǉ�
		//Jan. 16, 2001 JEPRO	Ctrl+H ���u�J�[�\���O���폜�v���u�J�[�\���s���E�B���h�E�����ցv�ɕύX��	Shift+Ctrl+H �Ɂu.c�܂���.cpp�Ɠ�����.h���J���v��ǉ�
		//Feb. 07, 2001 JEPRO	SHift+Ctrl+H ���u.c�܂���.cpp�Ɠ�����.h���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
		{ 'H', _T("H"),F_0, F_0, F_CURLINECENTER, F_OPEN_HfromtoC, F_0, F_0, F_TILE_V, F_0 },
		//Jan. 21, 2001	JEPRO	Ctrl+I �Ɂu�s�̓�d���v��ǉ�
		{ 'I', _T("I"),F_0, F_0, F_DUPLICATELINE, F_0, F_0, F_0, F_0, F_0 },
		{ 'J', _T("J"),F_0, F_0, F_JUMP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		//Jan. 16, 2001	JEPRO	Ctrl+K �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+E �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
		{ 'K', _T("K"),F_0, F_0, F_LineCutToEnd, F_LineDeleteToEnd, F_0, F_0, F_0, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+Alt+L �Ɂu�������v, Shift+Ctrl+Alt+L �Ɂu�啶���v��ǉ�
		//Jan. 16, 2001 Ctrl+L ���u�J�[�\���s���E�B���h�E�����ցv���u�L�[�}�N���̓ǂݍ��݁v�ɕύX���u�J�[�\���s���E�B���h�E�����ցv�� Ctrl+H �Ɉړ�
		//2001.12.03 hor Alt+L ���uLTRIM�v�Ɋ���
		{ 'L', _T("L"),F_0, F_0, F_LOADKEYMACRO, F_EXECKEYMACRO, F_LTRIM, F_0, F_TOLOWER, F_TOUPPER },
		//Jan. 16, 2001 JEPRO	Ctrl+M �Ɂu�L�[�}�N���̕ۑ��v��ǉ�
		//2001.12.06 hor Alt+M ���uMERGE�v�Ɋ���
		{ 'M', _T("M"),F_0, F_0, F_SAVEKEYMACRO, F_RECKEYMACRO, F_MERGE, F_0, F_0, F_0 },
		//Oct. 20, 2000 JEPRO	Alt+N �Ɂu�ړ�����: ���ցv��ǉ�
		{ 'N', _T("N"),F_0, F_0, F_FILENEW, F_0, F_JUMPHIST_NEXT, F_0, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+O �Ɂu�A�E�g�v�b�g�v��ǉ�
		{ 'O', _T("O"),F_0, F_0, F_FILEOPEN, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+P �Ɂu����v, Shift+Ctrl+P �Ɂu����v���r���[�v, Ctrl+Alt+P �Ɂu�y�[�W�ݒ�v��ǉ�
		//Oct. 20, 2000 JEPRO	Alt+P �Ɂu�ړ�����: �O�ցv��ǉ�
		{ 'P', _T("P"),F_0, F_0, F_PRINT, F_PRINT_PREVIEW, F_JUMPHIST_PREV, F_0, F_PRINT_PAGESETUP, F_0 },
		//Jan. 24, 2001	JEPRO	Ctrl+Q �Ɂu�L�[���蓖�Ĉꗗ���R�s�[�v��ǉ�
		{ 'Q', _T("Q"),F_0, F_0, F_CREATEKEYBINDLIST, F_0, F_0, F_0, F_0, F_0 },
		//2001.12.03 hor Alt+R ���uRTRIM�v�Ɋ���
		{ 'R', _T("R"),F_0, F_0, F_REPLACE_DIALOG, F_0, F_RTRIM, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+S �Ɂu���O��t���ĕۑ��v��ǉ�
		{ 'S', _T("S"),F_0, F_0, F_FILESAVE, F_FILESAVEAS_DIALOG, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+Alt+T �Ɂu���E�ɕ��ׂĕ\���v��ǉ�
		//Jan. 21, 2001	JEPRO	Ctrl+T �Ɂu�^�O�W�����v�v, Shift+Ctrl+T �Ɂu�^�O�W�����v�o�b�N�v��ǉ�
		{ 'T', _T("T"),F_0, F_0, F_TAGJUMP, F_TAGJUMPBACK, F_0, F_0, F_TILE_H, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+Alt+U �Ɂu���݂̃E�B���h�E���Ő܂�Ԃ��v��ǉ�
		//Jan. 16, 2001	JEPRO	Ctrl+U �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+U �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
		{ 'U', _T("U"),F_0, F_0, F_LineCutToStart, F_LineDeleteToStart, F_0, F_0, F_WRAPWINDOWWIDTH, F_0 },
		{ 'V', _T("V"),F_0, F_0, F_PASTE, F_0, F_0, F_0, F_0, F_0 },
		{ 'W', _T("W"),F_0, F_0, F_SELECTWORD, F_0, F_0, F_0, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+X ���u�J�X�^�����j���[1�v���u����`�v�ɕύX���u�J�X�^�����j���[1�v�� Alt+1 �Ɉړ�
		{ 'X', _T("X"),F_0, F_0, F_CUT, F_0, F_0, F_0, F_0, F_0 },
		{ 'Y', _T("Y"),F_0, F_0, F_REDO, F_0, F_0, F_0, F_0, F_0 },
		{ 'Z', _T("Z"),F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		/* �L�� */
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+- �Ɂu�㉺�ɕ����v��ǉ�
		// 2002.02.08 hor Ctrl+-�Ƀt�@�C�������R�s�[��ǉ�
		{ 0x00bd, _T("-"),F_0, F_0, F_COPYFNAME, F_SPLIT_V, F_0, F_0, F_0, F_0 },
		{ 0x00de, _T("^(�p��')"),F_0, F_0, F_COPYTAG, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ �Ɂu���E�ɕ����v��ǉ�
		{ 0x00dc, _T("\\"),F_0, F_0, F_COPYPATH, F_SPLIT_H, F_0, F_0, F_0, F_0 },
		//Sept. 20, 2000 JEPRO	Ctrl+@ �Ɂu�t�@�C�����e��r�v��ǉ�  //Oct. 15, 2000 JEPRO�u�I��͈͓��S�s�R�s�[�v�ɕύX
		{ 0x00c0, _T("@(�p��`)"),F_0, F_0, F_COPYLINES, F_0, F_0, F_0, F_0, F_0 },
		//	Aug. 16, 2000 genta
		//	���Ό����̊��ʂɂ����ʌ�����ǉ�
		{ 0x00db, _T("["),F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+; �Ɂu�c���ɕ����v��ǉ�	//Jan. 16, 2001	Alt+; �Ɂu���t�}���v��ǉ�
		{ 0x00bb, _T(";"),F_0, F_0, F_0, F_SPLIT_VH, F_INS_DATE, F_0, F_0, F_0 },
		//Sept. 14, 2000 JEPRO	Ctrl+: �Ɂu�I��͈͓��S�s�s�ԍ��t���R�s�[�v��ǉ�	//Jan. 16, 2001	Alt+: �Ɂu�����}���v��ǉ�
		{ 0x00ba, _T(":"),F_0, F_0, F_COPYLINESWITHLINENUMBER, F_0, F_INS_TIME, F_0, F_0, F_0 },
		{ 0x00dd, _T("]"),F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bc, _T(","),F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		//Sept. 14, 2000 JEPRO	Ctrl+. �Ɂu�I��͈͓��S�s���p���t���R�s�[�v��ǉ�
		{ 0x00be, _T("."),F_0, F_0, F_COPYLINESASPASSAGE, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bf, _T("/"),F_0, F_0, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		//	Nov. 15, 2000 genta PC/AT�L�[�{�[�h�ɍ��킹�ăL�[�R�[�h��ύX
		//	PC98�~�ς̂��߁C�]���̃L�[�R�[�h�ɑΉ����鍀�ڂ�ǉ��D
		{ 0x00e2, _T("_"),F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00df, _T("_(PC-98)"),F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	�����ĕ\��������Ȃ������łĂ��Ă��܂��̂ŃA�v���P�[�V�����L�[���A�v���L�[�ɒZ�k
		{ VK_APPS, _T("�A�v���L�["),F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON }
	};
	const int	nKeyDataInitNum = sizeof( KeyDataInit ) / sizeof( KeyDataInit[0] );
	//	From Here 2007.11.04 genta �o�b�t�@�I�[�o�[�����h�~
	if( nKeyDataInitNum > sizeof( pShareData->m_pKeyNameArr ) / sizeof( pShareData->m_pKeyNameArr[0])) {
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, _T("��҂ɋ����ė~�����G���["),
			_T("�L�[�ݒ萔�ɑ΂���DLLSHARE::m_nKeyNameArr[]�̃T�C�Y���s�����Ă��܂�") );
		return false;
	}
	//	To Here 2007.11.04 genta �o�b�t�@�I�[�o�[�����h�~
	for( int i = 0; i < nKeyDataInitNum; ++i ){
		SetKeyNameArrVal(
			pShareData,
			i,
			KeyDataInit[i].nKeyCode,
			KeyDataInit[i].pszKeyName,
			KeyDataInit[i].nFuncCode_0,
			KeyDataInit[i].nFuncCode_1,
			KeyDataInit[i].nFuncCode_2,
			KeyDataInit[i].nFuncCode_3,
			KeyDataInit[i].nFuncCode_4,
			KeyDataInit[i].nFuncCode_5,
			KeyDataInit[i].nFuncCode_6,
			KeyDataInit[i].nFuncCode_7
		 );
	}
	pShareData->m_nKeyNameArrNum = nKeyDataInitNum;
	return true;
}

/*!	@brief ���L������������/�c�[���o�[

	�c�[���o�[�֘A�̏���������

	@author genta
	@date 2005.01.30 genta CShareData::Init()���番���D
		����ݒ肵�Ȃ��ň�C�Ƀf�[�^�]������悤�ɁD
*/
void CShareData::InitToolButtons(DLLSHAREDATA* pShareData)
{
		/* �c�[���o�[�{�^���\���� */
//Sept. 16, 2000 JEPRO
//	CShareData_new2.cpp�łł��邾���n���ƂɏW�܂�悤�ɃA�C�R���̏��Ԃ�啝�ɓ���ւ����̂ɔ����ȉ��̏����ݒ�l��ύX
	static const int DEFAULT_TOOL_BUTTONS[] = {
		1,	//�V�K�쐬
		25,		//�t�@�C�����J��(DropDown)
		3,		//�㏑���ۑ�		//Sept. 16, 2000 JEPRO 3��11�ɕύX	//Oct. 25, 2000 11��3
		4,		//���O��t���ĕۑ�	//Sept. 19, 2000 JEPRO �ǉ�
		0,

		33,	//���ɖ߂�(Undo)	//Sept. 16, 2000 JEPRO 7��19�ɕύX	//Oct. 25, 2000 19��33
		34,	//��蒼��(Redo)	//Sept. 16, 2000 JEPRO 8��20�ɕύX	//Oct. 25, 2000 20��34
		0,

		87,	//�ړ�����: �O��	//Dec. 24, 2000 JEPRO �ǉ�
		88,	//�ړ�����: ����	//Dec. 24, 2000 JEPRO �ǉ�
		0,

		225,	//����		//Sept. 16, 2000 JEPRO 9��22�ɕύX	//Oct. 25, 2000 22��225
		226,	//��������	//Sept. 16, 2000 JEPRO 16��23�ɕύX	//Oct. 25, 2000 23��226
		227,	//�O������	//Sept. 16, 2000 JEPRO 17��24�ɕύX	//Oct. 25, 2000 24��227
		228,	//�u��		// Oct. 7, 2000 JEPRO �ǉ�
		229,	//�����}�[�N�̃N���A	//Sept. 16, 2000 JEPRO 41��25�ɕύX(Oct. 7, 2000 25��26)	//Oct. 25, 2000 25��229
		230,	//Grep		//Sept. 16, 2000 JEPRO 14��31�ɕύX	//Oct. 25, 2000 31��230
		232,	//�A�E�g���C�����	//Dec. 24, 2000 JEPRO �ǉ�
		0,

		264,	//�^�C�v�ʐݒ�ꗗ	//Sept. 16, 2000 JEPRO �ǉ�
		265,	//�^�C�v�ʐݒ�		//Sept. 16, 2000 JEPRO 18��36�ɕύX	//Oct. 25, 2000 36��265
		266,	//���ʐݒ�			//Sept. 16, 2000 JEPRO 10��37�ɕύX �������u�ݒ�v���p�e�B�V�[�g�v����ύX	//Oct. 25, 2000 37��266
		0,		//Oct. 8, 2000 jepro ���s�̂��߂ɒǉ�
		346,	//�R�}���h�ꗗ	//Oct. 8, 2000 JEPRO �ǉ�
	};

	//	�c�[���o�[�A�C�R�����̍ő�l�𒴂��Ȃ����߂̂��܂��Ȃ�
	//	�ő�l�𒴂��Ē�`���悤�Ƃ���Ƃ����ŃR���p�C���G���[�ɂȂ�܂��D
	char dummy[ _countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBARBUTTONS ? 1:0 ];
	dummy[0]=0;

	memcpy_raw(
		pShareData->m_Common.m_sToolBar.m_nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	/* �c�[���o�[�{�^���̐� */
	pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat = !IsVisualStyle();			/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */	// 2006.06.23 ryoji �r�W���A���X�^�C���ł͏����l���m�[�}���ɂ���
	
}


/*!	@brief ���L������������/�|�b�v�A�b�v���j���[

	�|�b�v�A�b�v���j���[�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
*/
void CShareData::InitPopupMenu(DLLSHAREDATA* pShareData)
{
	/* �J�X�^�����j���[ �K��l */
	
	CommonSetting_CustomMenu& rMenu = m_pShareData->m_Common.m_sCustomMenu;

	/* �E�N���b�N���j���[ */
	int n = 0;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_UNDO;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'U';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_REDO;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'R';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_CUT;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'T';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'C';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PASTE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'P';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_DELETE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'D';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	//Nov. 9, 2000 JEPRO �uCRLF���s�ŃR�s�[�v��ǉ�
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'L';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY_ADDCRLF;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'H';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	//Nov. 9, 2000 JEPRO �u��`�\��t���v�𕜊�
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'X';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_SELECTALL;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'A';
	n++;

	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 3, 2000 JEPRO �ȉ��Ɂu�^�O�W�����v�v�Ɓu�^�O�W�����v�o�b�N�v��ǉ�
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'G';		//Nov. 9, 2000 JEPRO �u�R�s�[�v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(T��G)
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'B';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 15, 2000 JEPRO �ȉ��Ɂu�I��͈͓��S�s�R�s�[�v�Ɓu���p���t���R�s�[�v��ǉ�
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYLINES;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '@';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '.';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYPATH;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\\';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'F';		//Nov. 9, 2000 JEPRO �u��蒼���v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(R��F)
	n++;
	rMenu.m_nCustMenuItemNumArr[0] = n;

	/* �J�X�^�����j���[�P */
	rMenu.m_nCustMenuItemNumArr[1] = 7;
	rMenu.m_nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
	rMenu.m_nCustMenuItemKeyArr [1][0] = 'O';		//Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (F��O)
	rMenu.m_nCustMenuItemFuncArr[1][1] = F_FILESAVE;
	rMenu.m_nCustMenuItemKeyArr [1][1] = 'S';
	rMenu.m_nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
	rMenu.m_nCustMenuItemKeyArr [1][2] = 'N';		//Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (O��N)
	rMenu.m_nCustMenuItemFuncArr[1][3] = F_TOLOWER;
	rMenu.m_nCustMenuItemKeyArr [1][3] = 'L';
	rMenu.m_nCustMenuItemFuncArr[1][4] = F_TOUPPER;
	rMenu.m_nCustMenuItemKeyArr [1][4] = 'U';
	rMenu.m_nCustMenuItemFuncArr[1][5] = F_0;
	rMenu.m_nCustMenuItemKeyArr [1][5] = '\0';
	rMenu.m_nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
	rMenu.m_nCustMenuItemKeyArr [1][6] = 'C';

	/* �^�u���j���[ */	//@@@ 2003.06.14 MIK
	n = 0;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'S';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVEAS_DIALOG;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'A';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'R';	// 2007.06.26 ryoji B -> R
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE_OPEN;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'L';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_WINCLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'C';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILE_REOPEN;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'W';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVERIGHT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVELEFT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_SEPARATE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'E';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTNEXT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'X';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTPREV;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'V';
	n++;
	rMenu.m_nCustMenuItemNumArr[CUSTMENU_INDEX_FOR_TABWND] = n;
}


#include "CNormalProcess.h"

//GetDllShareData�p�O���[�o���ϐ�
DLLSHAREDATA* g_theDLLSHAREDATA = NULL;

//DLLSHAREDATA�ւ̊ȈՃA�N�Z�T
DLLSHAREDATA& GetDllShareData()
{
	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

//DLLSHAREDATA���m�ۂ�����A�܂�������ĂԁB
void DLLSHAREDATA::OnInit()
{
	g_theDLLSHAREDATA = this;
}
