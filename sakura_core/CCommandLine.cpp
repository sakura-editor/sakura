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
#include "CShareData.h"
#include "etc_uty.h"
#include <tchar.h>
#include <io.h>
#include <string.h>

CCommandLine* CCommandLine::_instance=0;

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
		"CODE", 4, 1001,
		"R", 1, 1002,
		"NOWIN", 5, 1003,
		"GREPMODE", 8, 1100,
		"GREPDLG", 7, 1101,
		"DEBUGMODE", 9, 1999,
		NULL, 0, 0
	};

	/*!
		�R�}���h���C���I�v�V����
		���Ɉ�����������
	*/
	static const _CmdLineOpt _COptWithA[] = {
		"X", 1, 1,
		"Y", 1, 2,
		"VX", 2, 3,
		"VY", 2, 4,
		"TYPE", 4, 5,	//!< �^�C�v�ʐݒ� Mar. 7, 2002 genta
		"GKEY", 4, 101,
		"GFILE", 5, 102,
		"GFOLDER", 7, 103,
		"GOPT", 4, 104,
		NULL, 0, 0
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
	bool*		pbGrepKanjiCode_AutoDetect,
	bool*		pbGrepOutputLine,
	bool*		pbGrepWordOnly,
	int	*		pnGrepOutputStyle,
	bool*		pbDebugMode,
	bool*		pbNoWindow,	//!< [out] TRUE: �ҏWWindow���J���Ȃ�
	FileInfo*	pfi,
	bool*		pbReadOnly	//!< [out] TRUE: Read Only
)
{
	bool			bGrepMode;
	bool			bGrepDlg;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	bool			bGrepSubFolder;
	bool			bGrepLoHiCase;
	bool			bGrepRegularExp;
	bool			bGrepKanjiCode_AutoDetect;
	bool			bGrepOutputLine;
	bool			bGrepWordOnly;
	int				nGrepOutputStyle;
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
	bGrepKanjiCode_AutoDetect = false;
	bGrepOutputLine = false;
	bGrepWordOnly = false;
	nGrepOutputStyle = 1;
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
				if( '0' <= p[-1] && p[-1] <= '5' )
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
				strcpy( fi.m_szPath, cmWork.GetPtr( NULL/*&nDummy*/ ) );	/* �t�@�C���� */
			}else{
				strcpy( fi.m_szPath, pszToken );							/* �t�@�C���� */
			}
		}else{
			++pszToken;	//	�擪��'-'��skip
			char *arg;
			switch( CheckCommandLine( pszToken, &arg ) ){
			case 1: //	X
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nX = atoi( arg ) - 1;
				break;
			case 2:	//	Y
				fi.m_nY = atoi( arg ) - 1;
				break;
			case 3:	// VX
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nViewLeftCol = atoi( arg ) - 1;
				break;
			case 4:	//	VY
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nViewTopLine = atoi( arg ) - 1;
				break;
			case 5:	//	T
				//	Mar. 7, 2002 genta
				//	�t�@�C���^�C�v�̋����w��
				strncpy( fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
				fi.m_szDocType[ MAX_DOCTYPE_LEN ]= '\0';
				break;
			case 1001:	//	CODE
				fi.m_nCharCode = atoi( arg );
				break;
			case 1002:	//	R
				bReadOnly = true;
				break;
			case 1003:	//	NOWIN
				bNoWindow = true;
				break;
			case 1100:	//	GREPMODE
				bGrepMode = true;
				break;
			case 1101:	//	GREPDLG
				bGrepDlg = true;
				break;
			case 101:	//	GKEY
				//	�O���""����菜��
				cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case 102:	//	GFILE
				//	�O���""����菜��
				cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case 103:	//	GFOLDER
				cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case 104:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* �T�u�t�H���_������������� */
						bGrepSubFolder = true;	break;
					case 'L':	/* �p�啶���Ɖp����������ʂ��� */
						bGrepLoHiCase = true;	break;
					case 'R':	/* ���K�\�� */
						bGrepRegularExp = true;	break;
					case 'K':	/* �����R�[�h�������� */
						bGrepKanjiCode_AutoDetect = true;	break;
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
			case 1999:
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
	*pbGrepKanjiCode_AutoDetect = bGrepKanjiCode_AutoDetect;
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
		&m_gi.bGrepKanjiCode_AutoDetect,
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
