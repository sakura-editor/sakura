//	$Id$
/*!	@file
	@brief �R�}���h���C���p�[�T

	@author aroka
	@date	2002/01/08 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditApp��蕪��

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CCommandLine.h"
#include "CMemory.h"
//#include "CShareData.h"	2002/03/24 YAZAKI
#include "etc_uty.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "CRunningTimer.h"

CCommandLine* CCommandLine::_instance = NULL;

/* �R�}���h���C���I�v�V�����p�萔 */
#define CMDLINEOPT_R			1002
#define CMDLINEOPT_NOWIN		1003
#define CMDLINEOPT_GREPMODE		1100
#define CMDLINEOPT_GREPDLG		1101
#define CMDLINEOPT_DEBUGMODE	1999
#define CMDLINEOPT_X			1
#define CMDLINEOPT_Y			2
#define CMDLINEOPT_VX			3
#define CMDLINEOPT_VY			4
#define CMDLINEOPT_TYPE			5
#define CMDLINEOPT_CODE			6
#define CMDLINEOPT_GKEY			101
#define CMDLINEOPT_GFILE		102
#define CMDLINEOPT_GFOLDER		103
#define CMDLINEOPT_GOPT			104
#define CMDLINEOPT_GCODE		105
/*!
	�R�}���h���C���̃`�F�b�N���s���āA�I�v�V�����ԍ���
	����������ꍇ�͂��̐擪�A�h���X��Ԃ��B
	CCommandLine::ParseCommandLine()�Ŏg����B

	@return �I�v�V�����̔ԍ��B�ǂ�ɂ��Y�����Ȃ��Ƃ���0��Ԃ��B

	@author genta
	@date Apr. 6, 2001
*/
int CCommandLine::CheckCommandLine(
	LPSTR  str, //!< [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
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
		{"NOWIN", 5,		CMDLINEOPT_NOWIN},
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
		{"CODE", 4,			CMDLINEOPT_CODE},	// 2002/09/20 Moca _COptWoA����ړ�
		{"TYPE", 4,			CMDLINEOPT_TYPE},	//!< �^�C�v�ʐݒ� Mar. 7, 2002 genta
		{"GKEY", 4,			CMDLINEOPT_GKEY},
		{"GFILE", 5,		CMDLINEOPT_GFILE},
		{"GFOLDER", 7,		CMDLINEOPT_GFOLDER},
		{"GOPT", 4,			CMDLINEOPT_GOPT},
		{"GCODE", 5,		CMDLINEOPT_GCODE},	// 2002/09/21 Moca �ǉ�
		{NULL, 0, 0}
	};

	const _CmdLineOpt *ptr;
	int len = lstrlen( str );

	//	����������ꍇ���Ɋm�F
	for( ptr = _COptWithA; ptr->opt != NULL; ptr++ ){
		if( len >= ptr->len &&	//	����������Ă��邩
			//	�I�v�V���������̒����`�F�b�N
			( str[ptr->len] == '=' || str[ptr->len] == ':' ) &&
			//	������̔�r
			memcmp( str, ptr->opt, ptr->len ) == 0 ){
			*arg = str + ptr->len + 1;
			return ptr->value;
		}
	}

	//	�������Ȃ��ꍇ
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ ){
		if( len == ptr->len &&	//	�����`�F�b�N
			//	������̔�r
			memcmp( str, ptr->opt, ptr->len ) == 0 ){
			return ptr->value;
		}
	}
	return 0;	//	�Y������
}

/*! �R�}���h���C���̉��

	WinMain()����Ăяo�����B
	
	@note
	���ꂪ�Ăяo���ꂽ���_�ł͋��L�������̏��������������Ă��Ȃ����߁C
	���L�������ɃA�N�Z�X���Ă͂Ȃ�Ȃ��D
*/
void CCommandLine::ParseCommandLine(
	LPCSTR	pszCmdLineSrc,	//!< [in]�R�}���h���C��������
	bool*		pbGrepMode,	//!< [out] TRUE: Grep Mode
	bool*		pbGrepDlg,	//!< [out] TRUE: Grep Dialog�\��
	CMemory*	pcmGrepKey,	//!< [out] Grep��Key
	CMemory*	pcmGrepFile,
	CMemory*	pcmGrepFolder,
	bool*		pbGrepSubFolder,
	bool*		pbGrepLoHiCase,
	bool*		pbGrepRegularExp,
	int*		pnGrepCharSet,
	bool*		pbGrepOutputLine,
	bool*		pbGrepWordOnly,
	int	*		pnGrepOutputStyle,
	bool*		pbDebugMode,
	bool*		pbNoWindow,	//!< [out] TRUE: �ҏWWindow���J���Ȃ�
	FileInfo*	pfi,
	bool*		pbReadOnly	//!< [out] TRUE: Read Only
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CCommandLine::Parse" );

	bool			bGrepMode;
	bool			bGrepDlg;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	bool			bGrepSubFolder;
	bool			bGrepLoHiCase;
	bool			bGrepRegularExp;
	bool			bGrepOutputLine;
	bool			bGrepWordOnly;
	int				nGrepOutputStyle;
	int				nGrepCharSet;
	bool			bDebugMode;
	bool			bNoWindow;
	FileInfo		fi;
	bool			bReadOnly;
	LPSTR			pszCmdLineWork;
	int				nCmdLineWorkLen;
	bool			bFind;
//	WIN32_FIND_DATA	w32fd;
//	HANDLE			hFind;
	TCHAR			szPath[_MAX_PATH + 1];
	int				i;
	int				j;
	int				nPos;
	LPSTR			pszToken;
	CMemory			cmWork;
	//const LPSTR	pszOpt;
	//int			nOptLen;

	bGrepMode = false;
	bGrepDlg = false;

	bGrepSubFolder = false;
	bGrepLoHiCase = false;
	bGrepRegularExp = false;
	bGrepOutputLine = false;
	bGrepWordOnly = false;
	nGrepOutputStyle = 1;
	nGrepCharSet = CODE_SJIS;
	bDebugMode = false;
	bNoWindow = false;

	//	Oct. 19, 2001 genta �����l��-1�ɂ��āC�w��L��/�����𔻕ʉ\�ɂ��Ă݂�
	//	Mar. 7, 2002 genta �������̓R���X�g���N�^�ōs���D
//	fi.m_nViewTopLine = -1;				/* �\����̈�ԏ�̍s(0�J�n) */
//	fi.m_nViewLeftCol = -1;				/* �\����̈�ԍ��̌�(0�J�n) */
//	fi.m_nX = -1;						/* �J�[�\�� �����ʒu(�s������̃o�C�g��) */
//	fi.m_nY = -1;						/* �J�[�\�� �����ʒu(�܂�Ԃ������s�ʒu) */
//	fi.m_bIsModified = 0;				/* �ύX�t���O */
//	fi.m_nCharCode = CODE_AUTODETECT;	/* �����R�[�h��� *//* �����R�[�h�������� */
//	fi.m_szPath[0] = '\0';				/* �t�@�C���� */
	bReadOnly = false;					/* �ǂݎ���p�� */

	//	May 30, 2000 genta
	//	���s�t�@�C���������ƂɊ����R�[�h���Œ肷��D
	{
		char	exename[512];
		::GetModuleFileName( NULL, exename, 512 );

		int		len = strlen( exename );

		for( char *p = exename + len - 1; p > exename; p-- ){
			if( *p == '.' ){
				if( '0' <= p[-1] && p[-1] <= '6' )
					fi.m_nCharCode = p[-1] - '0';
				break;
			}
		}
	}





	bFind = false;
	if( pszCmdLineSrc[0] != '-' ){
		memset( (char*)szPath, 0, sizeof( szPath ) );
		i = 0;
		j = 0;
		for( ; i < sizeof( szPath ) - 1 && i <= (int)lstrlen(pszCmdLineSrc); ++i ){
			if( pszCmdLineSrc[i] != ' ' && pszCmdLineSrc[i] != '\0' ){
				szPath[j] = pszCmdLineSrc[i];
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
			szPath[j] = pszCmdLineSrc[i];
			++j;
		}
	}
	if( bFind ){
		strcpy( fi.m_szPath, szPath );	/* �t�@�C���� */
		nPos = j + 1;
	}else{
		nPos = 0;
	}
	pszCmdLineWork = new char[lstrlen( pszCmdLineSrc ) + 1];
	strcpy( pszCmdLineWork, pszCmdLineSrc );
	nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	while( pszToken != NULL ){
		if( !bFind && pszToken[0] != '-' ){
			if( pszToken[0] == '\"' ){
				cmWork.SetData( &pszToken[1],  lstrlen( pszToken ) - 2 );
				cmWork.Replace( "\"\"", "\"" );
				strcpy( fi.m_szPath, cmWork.GetPtr() );	/* �t�@�C���� */
			}else{
				strcpy( fi.m_szPath, pszToken );							/* �t�@�C���� */
			}
		}else{
			++pszToken;	//	�擪��'-'��skip
			char *arg;
			switch( CheckCommandLine( pszToken, &arg ) ){
			case CMDLINEOPT_X: //	X
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nX = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_Y:	//	Y
				fi.m_nY = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VX:	// VX
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nViewLeftCol = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VY:	//	VY
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nViewTopLine = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_TYPE:	//	TYPE
				//	Mar. 7, 2002 genta
				//	�t�@�C���^�C�v�̋����w��
				strncpy( fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
				fi.m_szDocType[ MAX_DOCTYPE_LEN ]= '\0';
				break;
			case CMDLINEOPT_CODE:	//	CODE
				fi.m_nCharCode = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_R:	//	R
				bReadOnly = true;
				break;
			case CMDLINEOPT_NOWIN:	//	NOWIN
				bNoWindow = true;
				break;
			case CMDLINEOPT_GREPMODE:	//	GREPMODE
				bGrepMode = true;
				break;
			case CMDLINEOPT_GREPDLG:	//	GREPDLG
				bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	//	GKEY
				//	�O���""����菜��
				cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFILE:	//	GFILE
				//	�O���""����菜��
				cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFOLDER:	//	GFOLDER
				cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GOPT:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* �T�u�t�H���_������������� */
						bGrepSubFolder = true;	break;
					case 'L':	/* �p�啶���Ɖp����������ʂ��� */
						bGrepLoHiCase = true;	break;
					case 'R':	/* ���K�\�� */
						bGrepRegularExp = true;	break;
					case 'K':	/* �����R�[�h�������� */
						// 2002/09/21 Moca �݊����ێ��̂��߂̏���
						nGrepCharSet = CODE_AUTODETECT;	break;
					case 'P':	/* �s���o�͂��邩�Y�����������o�͂��邩 */
						bGrepOutputLine = true;	break;
					case 'W':	/* �P��P�ʂŒT�� */
						bGrepWordOnly = true;	break;
					case '1':	/* Grep: �o�͌`�� */
						nGrepOutputStyle = 1;	break;
					case '2':	/* Grep: �o�͌`�� */
						nGrepOutputStyle = 2;	break;
					}
				}
				break;
			// 2002/09/21 Moca Grep�ł̕����R�[�h�Z�b�g �ǉ�
			case CMDLINEOPT_GCODE:
				nGrepCharSet = AtoiOptionInt( arg );	break;
			case CMDLINEOPT_DEBUGMODE:
				bDebugMode = true;
				break;
			}
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	}
	delete [] pszCmdLineWork;

	/* �t�@�C���� */
	if( '\0' != fi.m_szPath[0] ){
		/* �V���[�g�J�b�g(.lnk)�̉��� */
		if( TRUE == ResolveShortcutLink( NULL, fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}
		/* �����O�t�@�C�������擾���� */
		if( TRUE == ::GetLongFileName( fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}

		/* MRU������擾 */

	}

	/* �������ʂ��i�[ */
	*pbGrepMode					= bGrepMode;
	*pbGrepDlg					= bGrepDlg;
	*pcmGrepKey					= cmGrepKey;
	*pcmGrepFile				= cmGrepFile;
	*pcmGrepFolder				= cmGrepFolder;
	*pbGrepSubFolder			= bGrepSubFolder;
	*pbGrepLoHiCase				= bGrepLoHiCase;
	*pbGrepRegularExp			= bGrepRegularExp;
	*pnGrepCharSet				= nGrepCharSet;
	*pbGrepOutputLine			= bGrepOutputLine;
	*pbGrepWordOnly				= bGrepWordOnly;
	*pnGrepOutputStyle			= nGrepOutputStyle;

	*pbDebugMode				= bDebugMode;
	*pbNoWindow					= bNoWindow;
	*pfi						= fi;
	*pbReadOnly					= bReadOnly;
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
*/
CCommandLine::CCommandLine(LPSTR cmd) : 
	m_pszCmdLineSrc(cmd)
{
	ParseCommandLine(
		m_pszCmdLineSrc,
		&m_bGrepMode,
		&m_bGrepDlg,
		&m_gi.cmGrepKey,
		&m_gi.cmGrepFile,
		&m_gi.cmGrepFolder,
		&m_gi.bGrepSubFolder,
		&m_gi.bGrepNoIgnoreCase,
		&m_gi.bGrepRegularExp,
		&m_gi.nGrepCharSet,		//  2002/09/20 Moca
		&m_gi.bGrepOutputLine,
		&m_gi.bGrepWordOnly,	//	Jun. 25, 2001 genta
		&m_gi.nGrepOutputStyle,
		&m_bDebugMode,
		&m_bNoWindow,
		&m_fi,
		&m_bReadOnly
	);
}
/*[EOF]*/
