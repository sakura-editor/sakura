//	$Id$
/************************************************************
  global.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#include <windows.h>
// �ȉ��� ifdef �u���b�N�� DLL ����ȒP�ɃG�N�X�|�[�g������}�N�����쐬����W���I�ȕ��@�ł��B 
// ���� DLL ���̂��ׂẴt�@�C���̓R�}���h���C���Œ�`���ꂽ SAKURA_CORE_EXPORTS �V���{��
// �ŃR���p�C������܂��B���̃V���{���͂��� DLL ���g�p����ǂ̃v���W�F�N�g��ł�����`�łȂ�
// ��΂Ȃ�܂���B���̕��@�ł̓\�[�X�t�@�C���ɂ��̃t�@�C�����܂ނ��ׂẴv���W�F�N�g�� DLL 
// ����C���|�[�g���ꂽ���̂Ƃ��� SAKURA_CORE_API �֐����Q�Ƃ��A���̂��߂��� DLL �͂��̃}�N 
// ���Œ�`���ꂽ�V���{�����G�N�X�|�[�g���ꂽ���̂Ƃ��ĎQ�Ƃ��܂��B
#ifdef SAKURA_CORE_EXPORTS
#define SAKURA_CORE_API __declspec(dllexport)
#else
#define SAKURA_CORE_API __declspec(dllimport)
#endif

#if defined(__BORLANDC__)
#define __forceinline
#define _itoa itoa
#define _stricmp stricmp
#define _memicmp memicmp
#ifndef __max
#define __max(a,b)	((a)>=(b)?(a):(b))
#endif
#ifndef STRICT
#define STRICT
#endif
#endif

#define IS_KEYWORD_CHAR(c) ((c) == '#' || (c) == '$' || __iscsym( (c) ))


SAKURA_CORE_API extern const char* GSTR_APPNAME;


#ifdef _DEBUG
	#ifndef	GSTR_EDITWINDOWNAME
	#define	GSTR_EDITWINDOWNAME "TextEditorWindow_DEBUG"
	#endif
#else
	#ifndef	GSTR_EDITWINDOWNAME
	#define	GSTR_EDITWINDOWNAME "TextEditorWindow"
	#endif
#endif

#ifdef _DEBUG
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor_DEBUG"
	#endif
#else
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor"
	#endif
#endif

#ifdef _DEBUG
	#ifndef	GSTR_CEDITAPP
#define	GSTR_CEDITAPP "CEditApp_DEBUG"
	#endif
#else
	#ifndef	GSTR_CEDITAPP
	#define	GSTR_CEDITAPP "CEditApp"
	#endif
#endif

#ifdef _DEBUG
	#ifndef	GSTR_CSHAREDATA
	#define	GSTR_CSHAREDATA "CShareData_DEBUG"
	#endif
#else
	#ifndef	GSTR_CSHAREDATA
	#define	GSTR_CSHAREDATA "CShareData"
	#endif
#endif

//#ifndef	GSTR_REG_ROOTPATH
//#define	GSTR_REG_ROOTPATH "Software\\BugSoft\\sakura"
//#endif


/* �E�B���h�E��ID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000


/* �����R�[�h�Z�b�g��� */
SAKURA_CORE_API enum enumCodeType {
	CODE_SJIS,				/* SJIS */
	CODE_JIS,				/* JIS */
	CODE_EUC,				/* EUC */
	CODE_UNICODE,			/* Unicode */
	CODE_UTF8,				/* UTF-8 */
	CODE_UTF7,				/* UTF-7 */
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99	/* �����R�[�h�������� */
};

SAKURA_CORE_API extern const char* gm_pszCodeNameArr_1[];
SAKURA_CORE_API extern const char* gm_pszCodeNameArr_2[];
SAKURA_CORE_API extern const char* gm_pszCodeNameArr_3[];

//const char* GetCodeTypeName( enumCodeType nCodeTYpe )
//{ 
//	const char* pszCodeTypeNameArr[] = {
//		"SJIS",
//		"JIS",
//		"EUC",
//		"Unicode",
//		"UTF-8",
//		"UTF-7"
//	};
//	return pszCodeTypeNameArr[nCodeTYpe];
//}

/* �A�E�g���C����͂̎�� */
SAKURA_CORE_API enum enumOutlineType {
	OUTLINE_C,
	OUTLINE_CPP,
	OUTLINE_PLSQL,
	OUTLINE_TEXT,
	OUTLINE_JAVA,
	OUTLINE_COBOL,
	OUTLINE_ASM,
	OUTLINE_PERL,	//	Sep. 8, 2000 genta
	OUTLINE_CODEMAX,
	OUTLINE_UNKNOWN	= 99
};

/* �s�I�[�q�̎�� */
SAKURA_CORE_API enum enumEOLType {
	EOL_NONE,
	EOL_CRLF_UNICODE,
	EOL_CRLF,
	EOL_LFCR,
	EOL_LF,
	EOL_CR,
	EOL_CODEMAX,
	EOL_UNKNOWN = -1
};

/* �s�I�[�q�̃f�[�^�� */
SAKURA_CORE_API enum enumEOLLen {
	LEN_EOL_NONE			= 0,
	LEN_EOL_CRLF_UNICODE	= 4,
	LEN_EOL_CRLF			= 2,
	LEN_EOL_LFCR			= 2,
	LEN_EOL_LF				= 1,
	LEN_EOL_CR				= 1
//	LEN_EOL_UNKNOWN		0
};


#define EOL_TYPE_NUM			6

/* �s�I�[�q�̔z�� */
SAKURA_CORE_API extern const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM]; 

//	May 15, 2000 genta
//	CEOL�ֈړ�
//	From Here
/* �s�I�[�q�̃f�[�^�̔z�� */
// SAKURA_CORE_API extern const char* gm_pszEolDataArr[EOL_TYPE_NUM];

/* �s�I�[�q�̃f�[�^���̔z�� */
// SAKURA_CORE_API extern const int gm_pnEolLenArr[EOL_TYPE_NUM];

/* �s�I�[�q�̕\�����̔z�� */
// SAKURA_CORE_API extern const char* gm_pszEolNameArr[EOL_TYPE_NUM];

/* �s�I�[�q�̎�ނ𒲂ׂ� */
// SAKURA_CORE_API enumEOLType GetEOLType( const char*, int );
//	To Here

/* �I��̈�`��p�p�����[�^ */
SAKURA_CORE_API extern const COLORREF	SELECTEDAREA_RGB;
SAKURA_CORE_API extern const int		SELECTEDAREA_ROP2;


/* �X�}�[�g�C���f���g��� */
SAKURA_CORE_API enum enumSmartIndentType {
	SMARTINDENT_NONE,
	SMARTINDENT_CPP
};

///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */
