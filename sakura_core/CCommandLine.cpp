/*!	@file
	@brief �R�}���h���C���p�[�T

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditApp��蕪��
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2005, D.S.Koba, genta, susu
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CCommandLine.h"
#include "CMemory.h"
#include "etc_uty.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "CRunningTimer.h"
// �֐����}�N���Ē�`����̂� my_icmp.h �͍Ō�ɒu��	// 2006.10.25 ryoji
#include "my_icmp.h"
#include "charcode.h"  // 2006.06.28 rastiv
#include "debug.h"

CCommandLine* CCommandLine::_instance = NULL;

/* �R�}���h���C���I�v�V�����p�萔 */
#define CMDLINEOPT_R			1002
#define CMDLINEOPT_NOWIN		1003
#define CMDLINEOPT_WRITEQUIT	1004
#define CMDLINEOPT_GREPMODE		1100
#define CMDLINEOPT_GREPDLG		1101
#define CMDLINEOPT_DEBUGMODE	1999
#define CMDLINEOPT_NOMOREOPT	1998
#define CMDLINEOPT_X			1
#define CMDLINEOPT_Y			2
#define CMDLINEOPT_VX			3
#define CMDLINEOPT_VY			4
#define CMDLINEOPT_TYPE			5
#define CMDLINEOPT_CODE			6
#define CMDLINEOPT_SX			7
#define CMDLINEOPT_SY			8
#define CMDLINEOPT_WX			9
#define CMDLINEOPT_WY			10
#define CMDLINEOPT_GKEY			101
#define CMDLINEOPT_GFILE		102
#define CMDLINEOPT_GFOLDER		103
#define CMDLINEOPT_GOPT			104
#define CMDLINEOPT_GCODE		105
#define CMDLINEOPT_GROUP		500

/*!
	�R�}���h���C���̃`�F�b�N���s���āA�I�v�V�����ԍ���
	����������ꍇ�͂��̐擪�A�h���X��Ԃ��B
	CCommandLine::ParseCommandLine()�Ŏg����B

	@return �I�v�V�����̔ԍ��B�ǂ�ɂ��Y�����Ȃ��Ƃ���0��Ԃ��B

	@author genta
	@date Apr. 6, 2001
	@date 2006.10.25 ryoji �I�v�V����������̑啶������������ʂ��Ȃ�
*/
int CCommandLine::CheckCommandLine(
	LPSTR  str, //!< [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
	int quotelen, //!< [in] �I�v�V���������̈��p���̒����D�I�v�V�����S�̂����p���ň͂܂�Ă���ꍇ�̍l���D
	char** arg	//!< [out] ����������ꍇ�͂��̐擪�ւ̃|�C���^
)
{
	/*!
		�R�}���h���C���I�v�V������͗p�\���̔z��
	*/
	struct _CmdLineOpt {
		LPCSTR opt;	//!< �I�v�V����������
		int len;	//!< �I�v�V�����̕����񒷁i�v�Z���Ȃ����߁j
		int value;	//!< �ϊ���̒l
	};

	/*!
		�R�}���h���C���I�v�V����
		���Ɉ��������Ȃ�����
	*/
	static const _CmdLineOpt _COptWoA[] = {
		{"R", 1,			CMDLINEOPT_R},
		{"-", 1,			CMDLINEOPT_NOMOREOPT},
		{"NOWIN", 5,		CMDLINEOPT_NOWIN},
		{"WQ", 2,			CMDLINEOPT_WRITEQUIT},	// 2007.05.19 ryoji sakuext�p�ɒǉ�
		{"GREPMODE", 8,		CMDLINEOPT_GREPMODE},
		{"GREPDLG", 7,		CMDLINEOPT_GREPDLG},
		{"DEBUGMODE", 9,	CMDLINEOPT_DEBUGMODE},
		{NULL, 0, 0}
	};

	/*!
		�R�}���h���C���I�v�V����
		���Ɉ�����������
	*/
	static const _CmdLineOpt _COptWithA[] = {
		{"X", 1,			CMDLINEOPT_X},
		{"Y", 1,			CMDLINEOPT_Y},
		{"VX", 2,			CMDLINEOPT_VX},
		{"VY", 2,			CMDLINEOPT_VY},
		{"SX", 2,			CMDLINEOPT_SX},
		{"SY", 2,			CMDLINEOPT_SY},
		{"WX", 2,			CMDLINEOPT_WX},
		{"WY", 2,			CMDLINEOPT_WY},
		{"CODE", 4,			CMDLINEOPT_CODE},	// 2002/09/20 Moca _COptWoA����ړ�
		{"TYPE", 4,			CMDLINEOPT_TYPE},	//!< �^�C�v�ʐݒ� Mar. 7, 2002 genta
		{"GKEY", 4,			CMDLINEOPT_GKEY},
		{"GFILE", 5,		CMDLINEOPT_GFILE},
		{"GFOLDER", 7,		CMDLINEOPT_GFOLDER},
		{"GOPT", 4,			CMDLINEOPT_GOPT},
		{"GCODE", 5,		CMDLINEOPT_GCODE},	// 2002/09/21 Moca �ǉ�
		{"GROUP", 5,		CMDLINEOPT_GROUP},	// 2007.06.26 ryoji
		{NULL, 0, 0}
	};

	const _CmdLineOpt *ptr;
	int len = lstrlen( str ) - quotelen;

	//	����������ꍇ���Ɋm�F
	for( ptr = _COptWithA; ptr->opt != NULL; ptr++ ){
		if( len >= ptr->len &&	//	����������Ă��邩
			//	�I�v�V���������̒����`�F�b�N
			( str[ptr->len] == '=' || str[ptr->len] == ':' ) &&
			//	������̔�r
			_memicmp( str, ptr->opt, ptr->len ) == 0 ){		// 2006.10.25 ryoji memcmp() -> _memicmp()
			*arg = str + ptr->len + 1;
			return ptr->value;
		}
	}

	//	�������Ȃ��ꍇ
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ ){
		if( len == ptr->len &&	//	�����`�F�b�N
			//	������̔�r
			_memicmp( str, ptr->opt, ptr->len ) == 0 ){		// 2006.10.25 ryoji memcmp() -> _memicmp()
			return ptr->value;
		}
	}
	return 0;	//	�Y������
}

/*! �R�}���h���C���̉��

	WinMain()����Ăяo�����B
	
	@date 2005-08-24 D.S.Koba �֐���static����߁C�����o�ϐ��������œn���̂���߂�
	@date 2007.09.09 genta Visual Studio���e�X�̈��������߉�ɂ�""�ň͂ޖ��ɑΉ��D
		�I�v�V������""�ň͂܂ꂽ�ꍇ�ɑΉ�����D
		���������-�Ŏn�܂�t�@�C�������w��ł��Ȃ��Ȃ�̂ŁC
		����ȍ~�I�v�V������͂����Ȃ��Ƃ��� "--" �I�v�V������V�݂���D
	
	@note
	���ꂪ�Ăяo���ꂽ���_�ł͋��L�������̏��������������Ă��Ȃ����߁C
	���L�������ɃA�N�Z�X���Ă͂Ȃ�Ȃ��D
*/
void CCommandLine::ParseCommandLine( void )
{
	MY_RUNNINGTIMER( cRunningTimer, "CCommandLine::Parse" );

	//	May 30, 2000 genta
	//	���s�t�@�C���������ƂɊ����R�[�h���Œ肷��D
	{
		char	exename[512];
		::GetModuleFileName( NULL, exename, 512 );

		int		len = strlen( exename );

		for( char *p = exename + len - 1; p > exename; p-- ){
			if( *p == '.' ){
				if( '0' <= p[-1] && p[-1] <= '6' )
					m_fi.m_nCharCode = p[-1] - '0';
				break;
			}
		}
	}


	TCHAR	szPath[_MAX_PATH + 1];
	bool	bFind = false;
	bool	bParseOptDisabled = false;	// 2007.09.09 genta �I�v�V������͂��s��Ȃ��C�t�@�C�����Ƃ��Ĉ���
	int		nPos;
	int		i, j;
//	WIN32_FIND_DATA	w32fd;
//	HANDLE			hFind;
	if( m_pszCmdLineSrc[0] != '-' ){
		memset( (char*)szPath, 0, sizeof( szPath ) );
		i = 0;
		j = 0;
		for( ; i < sizeof( szPath ) - 1 && i <= (int)lstrlen(m_pszCmdLineSrc); ++i ){
			if( m_pszCmdLineSrc[i] != ' ' && m_pszCmdLineSrc[i] != '\0' ){
				szPath[j] = m_pszCmdLineSrc[i];
				++j;
				continue;
			}
			/* �t�@�C���̑��݂ƁA�t�@�C�����ǂ������`�F�b�N */
			if( -1 != _access( szPath, 0 ) ){
//? 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
//?				hFind = ::FindFirstFile( szPath, &w32fd );
//?				::FindClose( hFind );
//?				if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
//?					w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
//?				}else{
					bFind = true;
					break;
//?				}
			}
			szPath[j] = m_pszCmdLineSrc[i];
			++j;
		}
	}
	if( bFind ){
		strcpy( m_fi.m_szPath, szPath );	/* �t�@�C���� */
		nPos = j + 1;
	}else{
		nPos = 0;
	}
	LPSTR pszCmdLineWork = new char[lstrlen( m_pszCmdLineSrc ) + 1];
	strcpy( pszCmdLineWork, m_pszCmdLineSrc );
	int nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	LPSTR pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	while( pszToken != NULL ){
#ifdef _DEBUG
	MYTRACE( "OPT=[%s]\n", pszToken );
#endif
		//	2007.09.09 genta �I�v�V�������胋�[���ύX�D�I�v�V������͒�~��""�ň͂܂ꂽ�I�v�V�������l��
		if( !bFind && ( bParseOptDisabled ||
			! (pszToken[0] == '-' || pszToken[0] == '"' && pszToken[1] == '-' ) )){
			if( pszToken[0] == '\"' ){
				CMemory cmWork;
				//	Nov. 3, 2005 genta
				//	�����̃N�H�[�e�[�V�����������ꍇ���l�����āC
				//	�Ōオ�_�u���N�H�[�g�̏ꍇ�̂ݎ�菜��
				//	�t�@�C�����ɂ͎g���Ȃ������Ȃ̂Ńt�@�C�����Ɋ܂܂�Ă���ꍇ�͍l���s�v
				//	�܂�SHIFT-JIS��2�o�C�g�ڂ̍l�����s�v
				//	Nov. 27, 2005 genta
				//	�������_�u���N�H�[�g1�̏ꍇ�ɁC����1���ŏ��ƍŌ��1����
				//	���ԈႦ�āC�C���f�b�N�X-1�ɃA�N�Z�X���Ă��܂��̂�h�����߂ɒ������`�F�b�N����
				//	�t�@�C�����̌��ɂ���Option����͂��邽�߁C���[�v�͌p��
				int len = lstrlen( pszToken + 1 );
				if( len > 0 ){
					cmWork.SetData( &pszToken[1], len - ( pszToken[len] == '"' ? 1 : 0 ));
					cmWork.Replace( "\"\"", "\"" );
					strcpy( m_fi.m_szPath, cmWork.GetPtr() );	/* �t�@�C���� */
				}
				else {
					m_fi.m_szPath[0] = '\0';
				}
			}else{
				strcpy( m_fi.m_szPath, pszToken );							/* �t�@�C���� */
			}

			// Nov. 11, 2005 susu
			// �s���ȃt�@�C�����̂܂܂��ƃt�@�C���ۑ����_�C�A���O���o�Ȃ��Ȃ�̂�
			// �ȒP�ȃt�@�C���`�F�b�N���s���悤�ɏC��
			if (!memcmp(m_fi.m_szPath, "file:///", 8)) {
				char tmp_str[_MAX_PATH + 1];
				strcpy(tmp_str, &(m_fi.m_szPath[8]));
				strcpy(m_fi.m_szPath, tmp_str);
			}
			int len = strlen(m_fi.m_szPath);
			for (int i = 0; i < len ; i ++) {
				if ( (m_fi.m_szPath[i] == '<' ||	//	0x3C
					  m_fi.m_szPath[i] == '>' ||	//	0x3E
					  m_fi.m_szPath[i] == '?' ||	//	0x3F
					  m_fi.m_szPath[i] == '"' ||	//	0x22
					  m_fi.m_szPath[i] == '|' ||	//	0x7C
					  m_fi.m_szPath[i] == '*' ||	//	0x2A
					  0
					 ) &&
					( i ==0 || (i > 0 && ! _IS_SJIS_1( (unsigned char)(m_fi.m_szPath[i - 1] )) ))){
						char msg_str[_MAX_PATH + 1];
						sprintf( msg_str, "%s\r\n��L�̃t�@�C�����͕s���ł��B�t�@�C������ \\ / : * ? \" < > | �̕����͎g���܂���B ", m_fi.m_szPath );
						MessageBox( NULL, msg_str, "FileNameError", MB_OK);
						m_fi.m_szPath[0] = '\0';
						break;
				}
			}

		}else{
			int qlen = 0;
			if( *pszToken == '"' ){
				++pszToken;	// 2007.09.09 genta �擪��"�̓X�L�b�v
				qlen = 1;
			}
			++pszToken;	//	�擪��'-'��skip
			char *arg;
			switch( CheckCommandLine( pszToken, qlen, &arg ) ){
			case CMDLINEOPT_X: //	X
				/* �s���w���1�J�n�ɂ��� */
				m_fi.m_nX = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_Y:	//	Y
				m_fi.m_nY = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VX:	// VX
				/* �s���w���1�J�n�ɂ��� */
				m_fi.m_nViewLeftCol = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VY:	//	VY
				/* �s���w���1�J�n�ɂ��� */
				m_fi.m_nViewTopLine = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_SX: //	SX
				m_fi.m_nWindowSizeX = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_SY:	//	SY
				m_fi.m_nWindowSizeY = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_WX: //	WX
				m_fi.m_nWindowOriginX = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_WY:	//	WY
				m_fi.m_nWindowOriginY = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_TYPE:	//	TYPE
				//	Mar. 7, 2002 genta
				//	�t�@�C���^�C�v�̋����w��
				strncpy( m_fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
				m_fi.m_szDocType[ MAX_DOCTYPE_LEN ]= '\0';
				break;
			case CMDLINEOPT_CODE:	//	CODE
				m_fi.m_nCharCode = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_R:	//	R
				m_bReadOnly = true;
				break;
			case CMDLINEOPT_NOWIN:	//	NOWIN
				m_bNoWindow = true;
				break;
			case CMDLINEOPT_WRITEQUIT:	//	WRITEQUIT	// 2007.05.19 ryoji sakuext�p�ɒǉ�
				m_bWriteQuit = true;
				m_bNoWindow = true;	// 2007.09.05 ryoji -WQ���w�肳�ꂽ��-NOWIN���w�肳�ꂽ�Ƃ��Ĉ���
				break;
			case CMDLINEOPT_GREPMODE:	//	GREPMODE
				m_bGrepMode = true;
				break;
			case CMDLINEOPT_GREPDLG:	//	GREPDLG
				m_bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	//	GKEY
				//	�O���""����菜��
				m_gi.cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFILE:	//	GFILE
				//	�O���""����菜��
				m_gi.cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFOLDER:	//	GFOLDER
				m_gi.cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GOPT:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* �T�u�t�H���_������������� */
						m_gi.bGrepSubFolder = true;	break;
					case 'L':	/* �p�啶���Ɖp����������ʂ��� */
						m_gi.bGrepNoIgnoreCase = true;	break;
					case 'R':	/* ���K�\�� */
						m_gi.bGrepRegularExp = true;	break;
					case 'K':	/* �����R�[�h�������� */
						// 2002/09/21 Moca �݊����ێ��̂��߂̏���
						m_gi.nGrepCharSet = CODE_AUTODETECT;	break;
					case 'P':	/* �s���o�͂��邩�Y�����������o�͂��邩 */
						m_gi.bGrepOutputLine = true;	break;
					case 'W':	/* �P��P�ʂŒT�� */
						m_gi.bGrepWordOnly = true;	break;
					case '1':	/* Grep: �o�͌`�� */
						m_gi.nGrepOutputStyle = 1;	break;
					case '2':	/* Grep: �o�͌`�� */
						m_gi.nGrepOutputStyle = 2;	break;
					}
				}
				break;
			// 2002/09/21 Moca Grep�ł̕����R�[�h�Z�b�g �ǉ�
			case CMDLINEOPT_GCODE:
				m_gi.nGrepCharSet = AtoiOptionInt( arg );	break;
			case CMDLINEOPT_GROUP:	// GROUP	// 2007.06.26 ryoji
				m_nGroup = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_DEBUGMODE:
				m_bDebugMode = true;
				break;
			case CMDLINEOPT_NOMOREOPT:	// 2007.09.09 genta ����ȍ~��������
				bParseOptDisabled = true;
				break;
			}
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	}
	delete [] pszCmdLineWork;

	/* �t�@�C���� */
	if( '\0' != m_fi.m_szPath[0] ){
		/* �V���[�g�J�b�g(.lnk)�̉��� */
		if( TRUE == ResolveShortcutLink( NULL, m_fi.m_szPath, szPath ) ){
			strcpy( m_fi.m_szPath, szPath );
		}
		/* �����O�t�@�C�������擾���� */
		if( TRUE == ::GetLongFileName( m_fi.m_szPath, szPath ) ){
			strcpy( m_fi.m_szPath, szPath );
		}

		/* MRU������擾 */

	}

	return;
}

/*! 
	�V���O���g���F�v���Z�X�ŗB��̃C���X�^���X
*/
CCommandLine* CCommandLine::Instance(LPSTR cmd)
{
		if( !_instance ){
			_instance = new CCommandLine(cmd);
		}
		return _instance;
}

/*! 
	�R���X�g���N�^
	
	@date 2005-08-24 D.S.Koba ParseCommandLine()�ύX�ɂ�胁���o�ϐ��ɏ����l���
*/
CCommandLine::CCommandLine(LPSTR cmd) : 
	m_pszCmdLineSrc(cmd)
{
	m_bGrepMode				= false;
	m_bGrepDlg				= false;
	m_bDebugMode			= false;
	m_bNoWindow				= false;
	m_bWriteQuit			= false;
	m_gi.bGrepSubFolder		= false;
	m_gi.bGrepNoIgnoreCase	= false;
	m_gi.bGrepRegularExp	= false;
	m_gi.nGrepCharSet		= CODE_SJIS;
	m_gi.bGrepOutputLine	= false;
	m_gi.bGrepWordOnly		= false;
	m_gi.nGrepOutputStyle	= 1;
	m_bReadOnly				= false;
	m_nGroup				= 0;		// 2007.06.26 ryoji
	
	ParseCommandLine();
}
/*[EOF]*/
