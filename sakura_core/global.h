//	$Id$
/*!	@file
	@brief ���ʒ�`

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>
//#include <locale.h>

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

#ifdef SAKURA_NO_DLL	//@@@ 2001.12.30 add MIK
#undef SAKURA_CORE_API
#define SAKURA_CORE_API
#endif	//SAKURA_NO_DLL

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

//Oct. 31, 2000 JEPRO TeX Keyword �̂��߂�'\'��ǉ�	//Nov. 9, 2000 JEPRO HSP Keyword �̂��߂�'@'��ǉ�
//#define IS_KEYWORD_CHAR(c) ((c) == '#' || (c) == '$' || __iscsym( (c) ))
//#define IS_KEYWORD_CHAR(c) ((c) == '#'/*35*/ || (c) == '$'/*36*/ || (c) == '@'/*64*/ || (c) == '\\'/*92*/ || __iscsym( (c) ))
SAKURA_CORE_API	extern const unsigned char gm_keyword_char[256];	//@@@ 2002.04.27
#define IS_KEYWORD_CHAR(c)	((int)(gm_keyword_char[(unsigned char)((c) & 0xff)]))	//@@@ 2002.04.27 ���P�[���Ɉˑ����Ȃ�


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

//20020108 aroka �R���g���[���v���Z�X�ƋN�������̂��߂Ƀ~���[�e�b�N�X����ǉ�
#ifdef _DEBUG
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor_DEBUG"
	#define	GSTR_MUTEX_SAKURA_CP _T("MutexSakuraEditorCP_DEBUG")
	#define	GSTR_MUTEX_SAKURA_INIT _T("MutexSakuraEditorInit_DEBUG")
	#endif
#else
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor"
	#define	GSTR_MUTEX_SAKURA_CP _T("MutexSakuraEditorCP")
	#define	GSTR_MUTEX_SAKURA_INIT _T("MutexSakuraEditorInit")
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
	CODE_UNICODEBE,			/* Unicode BigEndian */
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
	OUTLINE_VB,	//	June 23, 2001 N.Nakatani
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ���[���t�@�C���p
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
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


#define EOL_TYPE_NUM	6

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




// Stonee ���F 2000/01/12
// ������ύX�����Ƃ��́Aglobal.cpp ��colorIDXKeyName�̒�`���ύX���ĉ������B
//	From Here Sept. 18, 2000 JEPRO ���Ԃ�啝�ɓ���ւ���
SAKURA_CORE_API enum enumColorIndexType {
	COLORIDX_TEXT		= 0,	/* �e�L�X�g */
	COLORIDX_RULER		= 1,	/* ���[���[ */
	COLORIDX_UNDERLINE	= 2,	/* �J�[�\���s�A���_�[���C�� */
	COLORIDX_GYOU		= 3,	/* �s�ԍ� */
	COLORIDX_GYOU_MOD	= 4,	/* �s�ԍ�(�ύX�s) */
	COLORIDX_TAB		= 5,	/* TAB�L�� */
	COLORIDX_SPACE		= 6,	/* ���p�� */ //2002.04.28 Add by KK �ȍ~�S��+1
	COLORIDX_ZENSPACE	= 7,	/* ���{��� */
	COLORIDX_CTRLCODE	= 8,	/* �R���g���[���R�[�h */
	COLORIDX_CRLF		= 9,	/* ���s�L�� */
	COLORIDX_WRAP		= 10,	/* �܂�Ԃ��L�� */
	COLORIDX_EOF		= 11,	/* EOF�L�� */
	COLORIDX_DIGIT		= 12,	/* ���p���l */	//@@@ 2001.02.17 by MIK	//�F�ݒ�Ver.3���烆�[�U�t�@�C���ɑ΂��Ă͕�����ŏ������Ă���̂Ń��i���o�����O���Ă��悢. Mar. 7, 2001 JEPRO noted
	COLORIDX_SEARCH		= 13,	/* ���������� */
	COLORIDX_KEYWORD1	= 14,	/* �����L�[���[�h1 */ // 2002/03/13 novice
	COLORIDX_KEYWORD2	= 15,	/* �����L�[���[�h2 */ // 2002/03/13 novice	//MIK ADDED
	COLORIDX_COMMENT	= 16,	/* �s�R�����g */						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING	= 17,	/* �V���O���N�H�[�e�[�V���������� */	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING	= 18,	/* �_�u���N�H�[�e�[�V���������� */		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL		= 19,	/* URL */								//Dec. 4, 2000 shifted by MIK
	COLORIDX_REGEX1		= 20,	/* ���K�\���L�[���[�h1 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2		= 21,	/* ���K�\���L�[���[�h2 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3		= 22,	/* ���K�\���L�[���[�h3 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4		= 23,	/* ���K�\���L�[���[�h4 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5		= 24,	/* ���K�\���L�[���[�h5 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6		= 25,	/* ���K�\���L�[���[�h6 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7		= 26,	/* ���K�\���L�[���[�h7 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8		= 27,	/* ���K�\���L�[���[�h8 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9		= 28,	/* ���K�\���L�[���[�h9 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10	= 29,	/* ���K�\���L�[���[�h10 */  //@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND = 30,	/* DIFF�ǉ� */	//@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE = 31,	/* DIFF�ǉ� */	//@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE = 32,	/* DIFF�ǉ� */	//@@@ 2002.06.01 MIK
	COLORIDX_BRACKET_PAIR = 33,	/* �Ί��� */	// 02/09/18 ai Add

	//�J���[�̍Ō�
	COLORIDX_LAST		= 34,	//Dec. 4, 2000 @@@2001.02.17 renumber by MIK				/* 02/09/18 ai Mod 33��34 */

	//�J���[�\������p
	COLORIDX_BLOCK1		= 35,	/* �u���b�N�R�����g1(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���) */	/* 02/09/18 ai Mod 34��35 */
	COLORIDX_BLOCK2		= 36	/* �u���b�N�R�����g2(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���) */	/* 02/09/18 ai Mod 35��36 */

	//1000-1099 : �J���[�\������p(���K�\���L�[���[�h)
};
//	To Here Sept. 18, 2000



/*!< �F�^�C�v */
SAKURA_CORE_API extern const char* const colorIDXKeyName[];

/*!< �ݒ�l�̏���E���� */
//	���[���̍���
#define IDC_SPIN_nRulerHeight_MIN	2
#define IDC_SPIN_nRulerHeight_MAX	32

// novice 2002/09/14
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
#define BOOL	int
#endif

///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */


/*[EOF]*/
