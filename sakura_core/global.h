/*!	@file
	@brief ���ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, Stonee, genta, jepro, MIK
	Copyright (C) 2001, jepro, hor, MIK
	Copyright (C) 2002, MIK, genta, aroka, YAZAKI, Moca, KK, novice
	Copyright (C) 2003, MIK, genta, zenryaku, naoh
	Copyright (C) 2004, Kazika
	Copyright (C) 2005, MIK, Moca, genta
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>

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
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#define __access access
#ifndef STRICT
#define STRICT
#endif
#endif


/*
SAKURA_CORE_API extern const TCHAR* GSTR_APPNAME;
SAKURA_CORE_API extern const CHAR*  GSTR_APPNAME_A;
*/
// �A�v�����B2007.09.21 kobake ����
#ifdef _UNICODE
	#define _APP_NAME_(TYPE) TYPE("sakuraW2")
#else
	#define _APP_NAME_(TYPE) TYPE("sakuraA")
#endif

#ifdef _DEBUG
	#define _APP_NAME_2_(TYPE) TYPE("(�f�o�b�O�� ") TYPE(__DATE__) TYPE(")")
#else
	#define _APP_NAME_2_(TYPE) TYPE("(") TYPE(__DATE__) TYPE(")")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) //��:UNICODE�f�o�b�O��_T("sakuraW2(�f�o�b�O��)")

#define GSTR_APPNAME    (_GSTR_APPNAME_(_T)   )
#define GSTR_APPNAME_A  (_GSTR_APPNAME_(ATEXT))
#define GSTR_APPNAME_W  (_GSTR_APPNAME_(LTEXT))





//2007.09.20 kobake �f�o�b�O���ʁA�萔�T�t�B�b�N�X
#ifdef _DEBUG
	#define _DEBUG_SUFFIX_ "_DEBUG"
#else
	#define _DEBUG_SUFFIX_ ""
#endif

//2007.09.20 kobake �r���h�R�[�h���ʁA�萔�T�t�B�b�N�X
#ifdef _UNICODE
	#define _CODE_SUFFIX_ "W"
#else
	#define _CODE_SUFFIX_ "A"
#endif


//2007.09.05 ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����
#define	GSTR_EDITWINDOWNAME (_T("TextEditorWindow") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))


//2002.01.08 aroka  �R���g���[���v���Z�X�ƋN�������̂��߂Ƀ~���[�e�b�N�X����ǉ�
//2006.04.10 ryoji  �R���g���[���v���Z�X�����������������C�x���g�t���O����ǉ�
//2007.09.05 kobake ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����
#define	GSTR_MUTEX_SAKURA					(_T("MutexSakuraEditor")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))


//2007.09.05 kobake ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����
#define	GSTR_CEDITAPP	(_T("CControlTray") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))


//2007.09.05 kobake ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����
#define	GSTR_CSHAREDATA	(_T("CShareData") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))


//	Dec. 2, 2002 genta
//	�Œ�t�@�C����
#define FN_APP_ICON  _T("my_appicon.ico")
#define FN_GREP_ICON _T("my_grepicon.ico")
#define FN_TOOL_BMP  _T("my_icons.bmp")

//	�W���A�v���P�[�V�����A�C�R�����\�[�X��
#ifdef _DEBUG
	#define ICON_DEFAULT_APP IDI_ICON_DEBUG
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#else
	#define ICON_DEFAULT_APP IDI_ICON_STD
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#endif



/* �E�B���h�E��ID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000

#include "charset/charset.h"

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
	OUTLINE_WZTXT,		// 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
	OUTLINE_HTML,		// 2003.05.20 zenryaku HTML�A�E�g���C�����
	OUTLINE_TEX,		// 2003.07.20 naoh TeX�A�E�g���C�����
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ���[���t�@�C���p
	OUTLINE_PYTHON,		//	2007.02.08 genta Python�A�E�g���C�����
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
	OUTLINE_UNKNOWN	= 99
};
typedef enumOutlineType EOutlineType; //2007.11.29 kobake

/* �_�C�A���O�\�����@ */ // �A�E�g���C���E�B���h�E�p�ɍ쐬 20060201 aroka
SAKURA_CORE_API enum enumShowDlg {
	SHOW_NORMAL			= 0,
	SHOW_RELOAD			= 1,
	SHOW_TOGGLE			= 2,
};


/* �I��̈�`��p�p�����[�^ */
SAKURA_CORE_API extern const COLORREF	SELECTEDAREA_RGB;
SAKURA_CORE_API extern const int		SELECTEDAREA_ROP2;


/* �X�}�[�g�C���f���g��� */
SAKURA_CORE_API enum enumSmartIndentType {
	SMARTINDENT_NONE,
	SMARTINDENT_CPP
};
typedef enumSmartIndentType ESmartIndentType; //2007.11.29 kobake




// Stonee ���F 2000/01/12
// ������ύX�����Ƃ��́Aglobal.cpp ��g_ColorAttributeArr�̒�`���ύX���ĉ������B
//	From Here Sept. 18, 2000 JEPRO ���Ԃ�啝�ɓ���ւ���
SAKURA_CORE_API enum enumColorIndexType {
	COLORIDX_TEXT		= 0,	/* �e�L�X�g */
	COLORIDX_RULER		= 1,	/* ���[���[ */
	COLORIDX_CARET		= 2,	/* �L�����b�g */	// 2006.12.07 ryoji
	COLORIDX_CARET_IME	= 3,	/* IME�L�����b�g */	// 2006.12.07 ryoji
	COLORIDX_UNDERLINE	= 4,	/* �J�[�\���s�A���_�[���C�� */
	COLORIDX_GYOU		= 5,	/* �s�ԍ� */
	COLORIDX_GYOU_MOD	= 6,	/* �s�ԍ�(�ύX�s) */
	COLORIDX_TAB		= 7,	/* TAB�L�� */
	COLORIDX_SPACE		= 8,	/* ���p�� */ //2002.04.28 Add by KK �ȍ~�S��+1
	COLORIDX_ZENSPACE	= 9,	/* ���{��� */
	COLORIDX_CTRLCODE	= 10,	/* �R���g���[���R�[�h */
	COLORIDX_CRLF		= 11,	/* ���s�L�� */
	COLORIDX_WRAP		= 12,	/* �܂�Ԃ��L�� */
	COLORIDX_VERTLINE	= 13,	/* �w�茅�c�� */	// 2005.11.08 Moca
	COLORIDX_EOF		= 14,	/* EOF�L�� */
	COLORIDX_DIGIT		= 15,	/* ���p���l */	//@@@ 2001.02.17 by MIK	//�F�ݒ�Ver.3���烆�[�U�t�@�C���ɑ΂��Ă͕�����ŏ������Ă���̂Ń��i���o�����O���Ă��悢. Mar. 7, 2001 JEPRO noted
	COLORIDX_SEARCH		= 16,	/* ���������� */
	COLORIDX_KEYWORD1	= 17,	/* �����L�[���[�h1 */ // 2002/03/13 novice
	COLORIDX_KEYWORD2	= 18,	/* �����L�[���[�h2 */ // 2002/03/13 novice	//MIK ADDED
	COLORIDX_KEYWORD3   = 19,	/* �����L�[���[�h3 */ // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4   = 20,	/* �����L�[���[�h4 */
	COLORIDX_KEYWORD5   = 21,	/* �����L�[���[�h5 */
	COLORIDX_KEYWORD6   = 22,	/* �����L�[���[�h6 */
	COLORIDX_KEYWORD7   = 23,	/* �����L�[���[�h7 */
	COLORIDX_KEYWORD8   = 24,	/* �����L�[���[�h8 */
	COLORIDX_KEYWORD9   = 25,	/* �����L�[���[�h9 */
	COLORIDX_KEYWORD10  = 26,	/* �����L�[���[�h10 */
	COLORIDX_COMMENT	= 27,	/* �s�R�����g */						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING	= 28,	/* �V���O���N�H�[�e�[�V���������� */	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING	= 29,	/* �_�u���N�H�[�e�[�V���������� */		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL		= 30,	/* URL */								//Dec. 4, 2000 shifted by MIK
	COLORIDX_REGEX1		= 31,	/* ���K�\���L�[���[�h1 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2		= 32,	/* ���K�\���L�[���[�h2 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3		= 33,	/* ���K�\���L�[���[�h3 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4		= 34,	/* ���K�\���L�[���[�h4 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5		= 35,	/* ���K�\���L�[���[�h5 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6		= 36,	/* ���K�\���L�[���[�h6 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7		= 37,	/* ���K�\���L�[���[�h7 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8		= 38,	/* ���K�\���L�[���[�h8 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9		= 39,	/* ���K�\���L�[���[�h9 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10	= 40,	/* ���K�\���L�[���[�h10 */  //@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND = 41,	/* DIFF�ǉ� */	//@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE = 42,	/* DIFF�ǉ� */	//@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE = 43,	/* DIFF�ǉ� */	//@@@ 2002.06.01 MIK
	COLORIDX_BRACKET_PAIR = 44,	/* �Ί��� */	// 02/09/18 ai Add
	COLORIDX_MARK		= 45,	/* �u�b�N�}�[�N */	// 02/10/16 ai Add

	//�J���[�̍Ō�
	COLORIDX_LAST		= 46,	//Dec. 4, 2000 @@@2001.02.17 renumber by MIK				/* 2002/10/16 ai Mod 34��35 */

	//�J���[�\������p
	COLORIDX_BLOCK1		= 47,	/* �u���b�N�R�����g1(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���) */	/* 02/10/16 ai Mod 35��36 */
	COLORIDX_BLOCK2		= 48,	/* �u���b�N�R�����g2(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���) */	/* 02/10/16 ai Mod 36��37 */

	// -- -- �ʖ� -- -- //
	COLORIDX_DEFAULT	= COLORIDX_TEXT,

	// -- -- 1000-1099 : �J���[�\������p(���K�\���L�[���[�h) -- -- //
};
//	To Here Sept. 18, 2000
typedef enumColorIndexType EColorIndexType;

//���K�\���L�[���[�h��EColorIndexType�l�����֐�
inline EColorIndexType MakeColorIndexType_RegularExpression(int nRegExpIndex)
{
	return (EColorIndexType)(1000 + nRegExpIndex);
}


//@@@ From Here 2003.05.31 MIK
/*! �^�u�E�C���h�E�p���b�Z�[�W�T�u�R�}���h */
SAKURA_CORE_API enum enumTabWndNotifyType {
	TWNT_REFRESH	= 0,		//�ĕ\��
	TWNT_ADD		= 1,		//�E�C���h�E�o�^
	TWNT_DEL		= 2,		//�E�C���h�E�폜
	TWNT_ORDER		= 3,		//�E�C���h�E�����ύX
	TWNT_FILE		= 4,		//�t�@�C�����ύX
	TWNT_MODE_ENABLE= 5,		//�^�u���[�h�L����	//2004.07.14 Kazika �ǉ�
	TWNT_MODE_DISABLE= 6,		//�^�u���[�h������	//2004.08.27 Kazika �ǉ�
	TWNT_WNDPL_ADJUST= 7,		//�E�B���h�E�ʒu���킹	// 2007.04.03 ryoji �ǉ�
};

/*! �o�[�̕\���E��\�� */
SAKURA_CORE_API	enum enumBarChangeNotifyType {
	MYBCN_TOOLBAR	= 0,		//�c�[���o�[
	MYBCN_FUNCKEY	= 1,		//�t�@���N�V�����L�[
	MYBCN_TAB		= 2,		//�^�u
	MYBCN_STATUSBAR	= 3,		//�X�e�[�^�X�o�[
};
//@@@ To Here 2003.05.31 MIK

//�^�u�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_TABWND		24
//�E�N���b�N���j���[�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


/*!< �F�^�C�v */
//@@@ From Here 2006.12.18 ryoji
#define COLOR_ATTRIB_FORCE_DISP		0x00000001
//#define COLOR_ATTRIB_NO_TEXT		0x00000010	�\��l
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	�\��l
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

typedef struct ColorAttributeData_t{
	TCHAR*			szName;
	unsigned int	fAttribute;
} ColorAttributeData;

SAKURA_CORE_API extern const ColorAttributeData g_ColorAttributeArr[];
//@@@ To Here 2006.12.18 ryoji

/*!< �ݒ�l�̏���E���� */
//	���[���̍���
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

// Feb. 18, 2003 genta �ő�l�̒萔���ƒl�ύX
const int LINESPACE_MAX = 128;
const int COLUMNSPACE_MAX = 64;


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

//	Aug. 14, 2005 genta �萔��`�ǉ�
// 2007.09.07 kobake �萔���ύX: MAXLINESIZE��MAXLINEKETAS
// 2007.09.07 kobake �萔���ύX: MINLINESIZE��MINLINEKETAS
const int MAXLINEKETAS		= 10240;	//!< 1�s�̌����̍ő�l
const int MINLINEKETAS		= 10;		//!< 1�s�̌����̍ŏ��l

const int LINEREADBUFSIZE	= 10240;	//!< �t�@�C������1�s���f�[�^��ǂݍ��ނƂ��̃o�b�t�@�T�C�Y

/**	�}�E�X�N���b�N�ƃL�[��`�̑Ή�

	@date 2007.11.04 genta �V�K�쐬�D���l����Ɣ͈̓T�C�Y��`�̂���
*/
enum MOUSEFUNCTION_ASSIGN {
	MOUSEFUNCTION_DOUBLECLICK	= 0,	//!< �_�u���N���b�N
	MOUSEFUNCTION_RIGHT			= 1,	//!< �E�N���b�N
	MOUSEFUNCTION_CENTER		= 2,	//!< ���N���b�N
	MOUSEFUNCTION_LEFTSIDE		= 3,	//!< ���T�C�h�N���b�N
	MOUSEFUNCTION_RIGHTSIDE		= 4,	//!< �E�T�C�h�N���b�N
	MOUSEFUNCTION_TRIPLECLICK	= 5,	//!< �g���v���N���b�N
	MOUSEFUNCTION_QUADCLICK		= 6,	//!< �N�A�h���v���N���b�N
	MOUSEFUNCTION_KEYBEGIN		= 7,	//!< �}�E�X�ւ̊��蓖�Č����{���̃L�[���蓖�Đ擪INDEX
};

//2007.09.06 kobake �ǉ�
//!��������
enum ESearchDirection{
	SEARCH_BACKWARD = 0, //�O������ (�O������)
	SEARCH_FORWARD  = 1, //������� (��������) (����)
};

//2007.09.06 kobake �ǉ�
struct SSearchOption{
//	ESearchDirection	eDirection;
//	bool	bPrevOrNext;	//!< false==�O������ true==�������
	bool	bRegularExp;	//!< true==���K�\��
	bool	bLoHiCase;		//!< true==�p�啶���������̋��
	bool	bWordOnly;		//!< true==�P��̂݌���

	SSearchOption() : bRegularExp(false), bLoHiCase(false), bWordOnly(false) { }
	SSearchOption(
		bool _bRegularExp,
		bool _bLoHiCase,
		bool _bWordOnly
	)
	: bRegularExp(_bRegularExp)
	, bLoHiCase(_bLoHiCase)
	, bWordOnly(_bWordOnly)
	{
	}
	void Reset()
	{
		bRegularExp = false;
		bLoHiCase   = false;
		bWordOnly   = false;
	}

	//���Z�q
	bool operator == (const SSearchOption& rhs) const
	{
		//�Ƃ肠����memcmp�ł�����
		return memcmp(this,&rhs,sizeof(*this))==0;
	}
	bool operator != (const SSearchOption& rhs) const
	{
		return !operator==(rhs);
	}

};



//2007.10.02 kobake CEditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
class CEditWnd;
extern CEditWnd* g_pcEditWnd;


/* �J���[�������C���f�b�N�X�ԍ��̕ϊ� */	//@@@ 2002.04.30
SAKURA_CORE_API int GetColorIndexByName( const TCHAR *name );
SAKURA_CORE_API const TCHAR* GetColorNameByIndex( int index );


///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */


/*[EOF]*/
