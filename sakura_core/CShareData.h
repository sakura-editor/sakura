/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, aroka, Moca, MIK, YAZAKI, hor
	Copyright (C) 2003, Moca, aroka, MIK, genta
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, MIK, genta, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CShareData;

#ifndef _CSHAREDATA_H_
#define _CSHAREDATA_H_

#include <windows.h>
#include <commctrl.h>
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CPrint.h"
#include "CProfile.h"

//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "CMRU.h"
#include "CMRUFolder.h"

//20020129 aroka
#include "funccode.h"
#include "CMemory.h"

#include "CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  20,	//Jul. 12, 2001 JEPRO �^�C�v�ʐݒ�̍ő�ݒ萔��16���瑝�₵��
	MAX_TOOLBARBUTTONS			= 384,	//Oct. 22, 2000 JEPRO �A�C�R���̍ő�o�^����128���₵��(256��384)
	MAX_CUSTOM_MENU				=  25,
	MAX_CUSTOM_MENU_NAME_LEN	=  32,
	MAX_CUSTOM_MENU_ITEMS		=  48,
	MAX_PRINTSETTINGARR			=   8,

	//	From Here Sep. 14, 2001 genta
	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_DATETIMEFOREMAT_LEN		= 100,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,	//@@@ 2001.11.17 add MIK

	MAX_KEYHELP_FILE			= 20,	//@@@ 2006.04.10 fon

	MAX_MARKLINES_LEN			= 1023,	// 2002.01.18 hor
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,	/// 2002.11.24 Moca

	/*! �o�^�ł���}�N���̐�
		@date 2005.01.30 genta 50�ɑ��₵��
	*/
	MAX_CUSTMACRO				= 50,

	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	MAX_TAGJUMPNUM				= 100,	// �^�u�W�����v���ő�l
	MAX_TAGJUMP_KEYWORD			= 30,	//�^�O�W�����v�p�L�[���[�h�ő�o�^��2005.04.04 MIK
	MAX_KEYWORDSET_PER_TYPE		= 10,	// 2004.01.23 genta (for MIK) �^�C�v�ʐݒ薈�̃L�[���[�h�Z�b�g��
	MAX_VERTLINES = 10,	// 2005.11.08 Moca �w�茅�c��
};



/*! �t�@�C�����

	@date 2002.03.07 genta m_szDocType�ǉ�
	@date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y�ǉ�
*/
struct FileInfo {
	int		m_nViewTopLine;			/*!< �\����̈�ԏ�̍s(0�J�n) */
	int		m_nViewLeftCol;			/*!< �\����̈�ԍ��̌�(0�J�n) */
	int		m_nX;					/*!< �J�[�\��  �����ʒu(�s������̃o�C�g��) */
	int		m_nY;					/*!< �J�[�\��  �����ʒu(�܂�Ԃ������s�ʒu) */
	int		m_bIsModified;			/*!< �ύX�t���O */
	int		m_nCharCode;			/*!< �����R�[�h��� */
	char	m_szPath[_MAX_PATH];	/*!< �t�@�C���� */
	BOOL	m_bIsGrep;				/*!< Grep�̃E�B���h�E�� */
	BOOL	m_bIsDebug;				/*!< �f�o�b�O���[�h��(�A�E�g�v�b�g�E�C���h�E) */
	char	m_szGrepKey[1024];
	char	m_szMarkLines[MAX_MARKLINES_LEN + 1];	/*!< �u�b�N�}�[�N�̕����s���X�g */
	char	m_szDocType[MAX_DOCTYPE_LEN + 1];	/*!< �����^�C�v */
	int		m_nWindowSizeX;			/*!< �E�B���h�E  ��(�s�N�Z����) */
	int		m_nWindowSizeY;			/*!< �E�B���h�E  ����(�s�N�Z����) */
	int		m_nWindowOriginX;		/*!< �E�B���h�E  �����ʒu(�s�N�Z�����E�}�C�i�X�l���L��) */
	int		m_nWindowOriginY;		/*!< �E�B���h�E  �����ʒu(�s�N�Z�����E�}�C�i�X�l���L��) */
	
	// Mar. 7, 2002 genta
	// Constructor �m���ɏ��������邽��
	FileInfo() : m_nViewTopLine( -1 ), m_nViewLeftCol( -1 ),
		m_nX( -1 ), m_nY( -1 ), m_bIsModified( 0 ),
		m_nCharCode( CODE_AUTODETECT ), m_bIsGrep( FALSE ), m_bIsDebug( FALSE ),
		m_nWindowSizeX( -1 ), m_nWindowSizeY( -1 ),
		//	2004.05.13 Moca �g�w�薳���h��-1����CW_USEDEFAULT�ɕύX
		m_nWindowOriginX( CW_USEDEFAULT ), m_nWindowOriginY( CW_USEDEFAULT )
	{
		m_szPath[0] = '\0';
		m_szMarkLines[0] = '\0';
		m_szDocType[0] = '\0';
	}
};

/*!	�����I�v�V����
	20020118 aroka
*/
struct GrepInfo {
	CMemory		cmGrepKey;			/*!< �����L�[ */
	CMemory		cmGrepFile;			/*!< �����Ώۃt�@�C�� */
	CMemory		cmGrepFolder;		/*!< �����Ώۃt�H���_ */
	bool		bGrepWordOnly;		/*!< �P��P�ʂŒT�� */
	bool		bGrepSubFolder;		/*!< �T�u�t�H���_���������� */
	bool		bGrepNoIgnoreCase;	/*!< �啶���Ə���������ʂ��� */
	bool		bGrepRegularExp;	/*!< ���K�\�����g�p���� */
	bool		bGrepOutputLine;	/*!< ���ʏo�͂ŊY���s���o�͂��� */
	int			nGrepOutputStyle;	/*!< ���ʏo�͌`�� */
	int			nGrepCharSet;		/*!< �����R�[�h�Z�b�g */
};


struct EditNode {
	int				m_nIndex;
	HWND			m_hWnd;
	char			m_szTabCaption[_MAX_PATH];	/*!< �^�u�E�C���h�E�p�F�L���v�V������ */	//@@@ 2003.05.31 MIK
	char			m_szFilePath[_MAX_PATH];	/*!< �^�u�E�C���h�E�p�F�t�@�C���� */		//@@@ 2006.01.28 ryoji
	BOOL			m_bIsGrep;					/*!< Grep�̃E�B���h�E�� */					//@@@ 2006.01.28 ryoji
};

//! ����ݒ�
#define POS_LEFT	0
#define POS_CENTER	1
#define POS_RIGHT	2
#define HEADER_MAX	100
#define FOOTER_MAX	HEADER_MAX
struct PRINTSETTING {
	char			m_szPrintSettingName[32 + 1];		/*!< ����ݒ�̖��O */
	char			m_szPrintFontFaceHan[LF_FACESIZE];	/*!< ����t�H���g */
	char			m_szPrintFontFaceZen[LF_FACESIZE];	/*!< ����t�H���g */
	int				m_nPrintFontWidth;					/*!< ����t�H���g��(1/10mm�P�ʒP��) */
	int				m_nPrintFontHeight;					/*!< ����t�H���g����(1/10mm�P�ʒP��) */
	int				m_nPrintDansuu;						/*!< �i�g�̒i�� */
	int				m_nPrintDanSpace;					/*!< �i�ƒi�̌���(1/10mm�P��) */
	int				m_nPrintLineSpacing;				/*!< ����t�H���g�s�� �����̍����ɑ΂��銄��(%) */
	int				m_nPrintMarginTY;					/*!< ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginBY;					/*!< ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginLX;					/*!< ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginRX;					/*!< ����p���}�[�W�� �E(mm�P��) */
	int				m_nPrintPaperOrientation;			/*!< �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2) */
	int				m_nPrintPaperSize;					/*!< �p���T�C�Y */
	BOOL			m_bPrintWordWrap;					/*!< �p�����[�h���b�v���� */
	BOOL			m_bPrintKinsokuHead;				/*!< �s���֑����� */	//@@@ 2002.04.09 MIK
	BOOL			m_bPrintKinsokuTail;				/*!< �s���֑����� */	//@@@ 2002.04.09 MIK
	BOOL			m_bPrintKinsokuRet;					/*!< ���s�����̂Ԃ牺�� */	//@@@ 2002.04.13 MIK
	BOOL			m_bPrintKinsokuKuto;				/*!< ��Ǔ_�̂Ԃ炳�� */	//@@@ 2002.04.17 MIK
	BOOL			m_bPrintLineNumber;					/*!< �s�ԍ���������� */


	MYDEVMODE		m_mdmDevMode;						/*!< �v�����^�ݒ� DEVMODE�p */
	BOOL			m_bHeaderUse[3];					/* �w�b�_���g���Ă��邩�H	*/
	char			m_szHeaderForm[3][HEADER_MAX];		/* 0:���񂹃w�b�_�B1:�����񂹃w�b�_�B2:�E�񂹃w�b�_�B*/
	BOOL			m_bFooterUse[3];					/* �t�b�^���g���Ă��邩�H	*/
	char			m_szFooterForm[3][FOOTER_MAX];		/* 0:���񂹃t�b�^�B1:�����񂹃t�b�^�B2:�E�񂹃t�b�^�B*/
};


//! �F�ݒ�
struct ColorInfo {
	int			m_nColorIdx;
	BOOL		m_bDisp;			/* �F����/�\�� ������ */
	BOOL		m_bFatFont;			/* ������ */
	BOOL		m_bUnderLine;		/* �A���_�[���C���� */
	COLORREF	m_colTEXT;			/* �O�i�F(�����F) */
	COLORREF	m_colBACK;			/* �w�i�F */
	char		m_szName[32];		/* ���O */
	char		m_cReserved[60];
};

//! �F�ݒ�(�ۑ��p)
struct ColorInfoIni {
	const char*	m_pszName;			/* �F�� */
	BOOL		m_bDisp;			/* �F����/�\�� ������ */
	BOOL		m_bFatFont;			/* ������ */
	BOOL		m_bUnderLine;		/* �A���_�[���C���� */
	COLORREF	m_colTEXT;			/* �O�i�F(�����F) */
	COLORREF	m_colBACK;			/* �w�i�F */
};

//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	char	m_szKeyword[100];	//���K�\���L�[���[�h
	int	m_nColorIndex;		//�F�w��ԍ�
};
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< �����̐����̍ő咷 -1 */
struct KeyHelpInfo {
	int		m_nUse;						/*!< ������ �g�p����/���Ȃ� */
	char	m_szAbout[DICT_ABOUT_LEN];	/*!< �����̐���(�����t�@�C����1�s�ڂ��琶��) */
	char	m_szPath[_MAX_PATH];		/*!< �t�@�C���p�X */
};
//@@@ 2006.04.10 fon ADD-end

//! �^�C�v�ʐݒ�
struct Types {
	int					m_nIdx;
	char				m_szTypeName[64];				/*!< �^�C�v�����F���� */
	char				m_szTypeExts[64];				/*!< �^�C�v�����F�g���q���X�g */
	int					m_nMaxLineSize;					/*!< �܂�Ԃ������� */
	int					m_nColmSpace;					/*!< �����ƕ����̌��� */
	int					m_nLineSpace;					/*!< �s�Ԃ̂����� */
	int					m_nTabSpace;					/*!< TAB�̕����� */
	int					m_bTabArrow;					/*!< �^�u���\�� */	//@@@ 2003.03.26 MIK
	char				m_szTabViewString[17];			/*!< TAB�\�������� */	// 2003.1.26 aroka �T�C�Y�g��
	int					m_bInsSpace;					/* �X�y�[�X�̑}�� */	// 2001.12.03 hor
	// 2005.01.13 MIK �z��
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	/*!< �L�[���[�h�Z�b�g */

	CLineComment		m_cLineComment;					/*!< �s�R�����g�f���~�^ */			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComment;				/*!< �u���b�N�R�����g�f���~�^ */	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					/*!< �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	char				m_szIndentChars[64];			/*!< ���̑��̃C���f���g�Ώە��� */

	int					m_nColorInfoArrNum;				/*!< �F�ݒ�z��̗L���� */
	ColorInfo			m_ColorInfoArr[64];				/*!< �F�ݒ�z�� */

	int					m_bLineNumIsCRLF;				/*!< �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	int					m_nLineTermType;				/*!< �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
	char				m_cLineTermChar;				/*!< �s�ԍ���؂蕶�� */
	int					m_nVertLineIdx[MAX_VERTLINES];	/*!< �w�茅�c�� */

	BOOL				m_bWordWrap;					/*!< �p�����[�h���b�v������ */
	BOOL				m_bKinsokuHead;					/*!< �s���֑������� */	//@@@ 2002.04.08 MIK
	BOOL				m_bKinsokuTail;					/*!< �s���֑������� */	//@@@ 2002.04.08 MIK
	BOOL				m_bKinsokuRet;					/*!< ���s�����̂Ԃ牺�� */	//@@@ 2002.04.13 MIK
	BOOL				m_bKinsokuKuto;					/*!< ��Ǔ_�̂Ԃ炳�� */	//@@@ 2002.04.17 MIK
	char				m_szKinsokuHead[200];			/*!< �s���֑����� */	//@@@ 2002.04.08 MIK
	char				m_szKinsokuTail[200];			/*!< �s���֑����� */	//@@@ 2002.04.08 MIK

	int					m_nCurrentPrintSetting;			/*!< ���ݑI�����Ă������ݒ� */

	int					m_nDefaultOutline;				/*!< �A�E�g���C����͕��@ */
	char				m_szOutlineRuleFilename[_MAX_PATH];	/*!< �A�E�g���C����̓��[���t�@�C�� */

	int					m_nSmartIndent;					/*!< �X�}�[�g�C���f���g��� */
	int					m_nImeState;	//	Nov. 20, 2000 genta ����IME���

	//	2001/06/14 asa-o �⊮�̃^�C�v�ʐݒ�
	char				m_szHokanFile[_MAX_PATH];		/*!< ���͕⊮ �P��t�@�C�� */
	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	int					m_bUseHokanByFile;				/*!< ���͕⊮ �J���Ă���t�@�C�����������T�� */
	//	2001/06/19 asa-o
	int					m_bHokanLoHiCase;				/*!< ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */

	char				m_szExtHelp[_MAX_PATH];		/* �O���w���v�P */
	char				m_szExtHtmlHelp[_MAX_PATH];		/* �O��HTML�w���v */
	BOOL				m_bHtmlHelpIsSingle;			/* HtmlHelp�r���[�A�͂ЂƂ� */
	
	
//@@@ 2001.11.17 add start MIK
	BOOL	m_bUseRegexKeyword;	/* ���K�\���L�[���[�h���g����*/
	int	m_nRegexKeyMagicNumber;	/* ���K�\���L�[���[�h�X�V�}�W�b�N�i���o�[ */
	struct RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];	/* ���K�\���L�[���[�h */
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	BOOL				m_bUseKeyWordHelp;			/* �L�[���[�h�����Z���N�g�@�\���g���� */
	int					m_nKeyHelpNum;					/* �L�[���[�h�����̍��� */
	struct	KeyHelpInfo	m_KeyHelpArr[MAX_KEYHELP_FILE];	/* �L�[���[�h�����t�@�C�� */
	BOOL				m_bUseKeyHelpAllSearch;			/* �q�b�g�������̎���������(&A) */
	BOOL				m_bUseKeyHelpKeyDisp;			/* 1�s�ڂɃL�[���[�h���\������(&W) */
	BOOL				m_bUseKeyHelpPrefix;			/* �I��͈͂őO����v����(&P) */
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Common����ړ��B
	BOOL				m_bAutoIndent;					/* �I�[�g�C���f���g */
	BOOL				m_bAutoIndent_ZENSPACE;			/* ���{��󔒂��C���f���g */
	BOOL				m_bRTrimPrevLine;				/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	int					m_nIndentLayout;				/* �܂�Ԃ���2�s�ڈȍ~���������\�� */
	
	//	Sep. 10, 2002 genta
	int					m_bUseDocumentIcon;	/*!< �t�@�C���Ɋ֘A�Â���ꂽ�A�C�R�����g�� */

}; /* Types */

//! �}�N�����
struct MacroRec {
	char	m_szName[MACRONAME_MAX];	//<! �\����
	char	m_szFile[_MAX_PATH+1];	//<! �t�@�C����(�f�B���N�g�����܂܂Ȃ�)
	BOOL	m_bReloadWhenExecute;	//	���s���ɓǂݍ��݂Ȃ������i�f�t�H���gon�j
	
	bool IsEnabled() const { return m_szFile[0] != '\0'; }
};
//	To Here Sep. 14, 2001 genta

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
//! �^�O�W�����v���
struct TagJump {
	HWND	hwndReferer;				//<! �Q�ƌ��E�B���h�E
	POINT	point;						//<! ���C��, �J����
};

//	Aug. 15, 2000 genta
//	Backup Flags
const int BKUP_YEAR		= 32;
const int BKUP_MONTH	= 16;
const int BKUP_DAY		= 8;
const int BKUP_HOUR		= 4;
const int BKUP_MIN		= 2;
const int BKUP_SEC		= 1;

//	Aug. 21, 2000 genta
const int BKUP_AUTO		= 64;

// Apr. 05, 2003 genta WindowCaption�p�̈�i�ϊ��O�j�̒���
const int MAX_CAPTION_CONF_LEN = 256;

//	2004.05.13 Moca
//! �E�B���h�E�T�C�Y�E�ʒu�̐�����@
enum eWINSIZEMODE{
	WINSIZEMODE_DEF = 0, //!< �w��Ȃ�
	WINSIZEMODE_SAVE = 1, //!< �p��(�ۑ�)
	WINSIZEMODE_SET = 2   //!< ���ڎw��(�Œ�)
};


//! ���ʐݒ�
struct Common {

	//	Jul. 3, 2000 genta
	//	�A�N�Z�X�֐�(�Ȉ�)
	//	int���r�b�g�P�ʂɕ������Ďg��
	//	��4bit��CaretType�ɓ��ĂĂ���(�����̗\��ő��߂Ɏ���Ă���)
	int		GetCaretType(void) const { return m_nCaretType & 0xf; }
	void	SetCaretType(const int f){ m_nCaretType &= ~0xf; m_nCaretType |= f & 0xf; }

	//	Aug. 15, 2000 genta
	//	Backup�ݒ�̃A�N�Z�X�֐�
	int		GetBackupType(void) const { return m_nBackUpType; }
	void	SetBackupType(int n){ m_nBackUpType = n; }

	bool	GetBackupOpt(int flag) const { return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(int flag, bool value){
		m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 );
	}

	//	�o�b�N�A�b�v��
	int		GetBackupCount(void) const { return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value){
		m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff );
	}

	//	�o�b�N�A�b�v�̊g���q�擪����(1����)
	int		GetBackupExtChar(void) const { return ( m_nBackUpType_Opt2 >> 16 ) & 0xff ; }
	void	SetBackupExtChar(int value){
		m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xff00ffff) | (( value & 0xff ) << 16 );
	}

	//	Aug. 21, 2000 genta
	//	����Backup
	bool	IsAutoBackupEnabled(void) const { return GetBackupOpt( BKUP_AUTO ); }
	void	EnableAutoBackup(bool flag){ SetBackupOpt( BKUP_AUTO, flag ); }

	int		GetAutoBackupInterval(void) const { return m_nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i){ m_nBackUpType_Opt3 = i; }

	//	Backup�ڍאݒ�̃A�N�Z�X�֐�
	int		GetBackupTypeAdv(void) const { return m_nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n){ m_nBackUpType_Opt4 = n; }

	//	Oct. 27, 2000 genta
	//	�J�[�\���ʒu�𕜌����邩�ǂ���
	bool	GetRestoreCurPosition(void) const { return m_bRestoreCurPosition != 0; }
	void	SetRestoreCurPosition(bool i){ m_bRestoreCurPosition = i; }

	// 2002.01.16 hor �u�b�N�}�[�N�𕜌����邩�ǂ���
	bool	GetRestoreBookmarks(void) const { return m_bRestoreBookmarks != 0; }
	void	SetRestoreBookmarks(bool i){ m_bRestoreBookmarks = i; }

	//	Nov. 12, 2000 genta
	//	�t�@�C���ǂݍ��ݎ���MIME��decode���s����
	bool	GetAutoMIMEdecode(void) const { return m_bAutoMIMEdecode != 0; }
	void	SetAutoMIMEdecode(bool i){ m_bAutoMIMEdecode = i; }

	// Oct. 03, 2004 genta �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��
	bool	GetQueryIfCodeChange(void) const { return m_bQueryIfCodeChange != 0; }
	void	SetQueryIfCodeChange(bool i){ m_bQueryIfCodeChange = i; }
	
	// Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	bool	GetAlertIfFileNotExist(void) const { return m_bAlertIfFileNotExist != 0; }
	void	SetAlertIfFileNotExist(bool i){ m_bAlertIfFileNotExist = i; }
	

	//	����: �ݒ�t�@�C������̓ǂݍ��ݎ���INT�Ƃ��Ĉ������߁Cbool�^���g���Ă͂����Ȃ��D
	//	sizeof(int) != sizeof(bool)���ƃf�[�^��j�󂵂Ă��܂��D

	int					m_nCaretType;					/* �J�[�\���̃^�C�v 0=win 1=dos  */
	int					m_bIsINSMode;					/* �}���^�㏑�����[�h */
	int					m_bIsFreeCursorMode;			/* �t���[�J�[�\�����[�h�� */
	BOOL				m_bStopsBothEndsWhenSearchWord;	/* �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩 */
	BOOL				m_bStopsBothEndsWhenSearchParagraph;	/* �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩 */

	BOOL				m_bRestoreCurPosition;			//	�t�@�C�����J�����Ƃ��J�[�\���ʒu�𕜌����邩
	BOOL				m_bRestoreBookmarks;			// 2002.01.16 hor �u�b�N�}�[�N�𕜌����邩�ǂ���

	int					m_bAddCRLFWhenCopy;				/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
	int					m_nRepeatedScrollLineNum;		/* �L�[���s�[�g���̃X�N���[���s�� */
	BOOL				m_nRepeatedScroll_Smooth;		/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	BOOL				m_bCloseAllConfirm;				/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
	BOOL				m_bExitConfirm;					/* �I�����̊m�F������ */
	int					m_bRegularExp;					/* �����^�u��  1==���K�\�� */
	int					m_bLoHiCase;					/* �����^�u��  1==�p�啶���������̋�� */
	int					m_bWordOnly;					/* �����^�u��  1==�P��̂݌��� */
	int					m_bConsecutiveAll;				/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji
	int					m_bNOTIFYNOTFOUND;				/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	int					m_bSelectedArea;				/* �u��  �I��͈͓��u�� */
	int					m_bGrepSubFolder;				/* Grep: �T�u�t�H���_������ */
	BOOL				m_bGrepOutputLine;				/* Grep: �s���o�͂��邩�Y�����������o�͂��邩 */
	int					m_nGrepOutputStyle;				/* Grep: �o�͌`�� */
	int					m_bGrepDefaultFolder;			/* Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ��� */
	int					m_nGrepCharSet;					/* Grep: �����R�[�h�Z�b�g */ // 2002/09/20 Moca Add
	int					m_bGrepRealTimeView;			/* 2003.06.16 Moca Grep���ʂ̃��A���^�C���\�� */

	BOOL				m_bGTJW_RETURN;					/* �G���^�[�L�[�Ń^�O�W�����v */
	BOOL				m_bGTJW_LDBLCLK;				/* �_�u���N���b�N�Ń^�O�W�����v */
	LOGFONT				m_lf;							/* ���݂̃t�H���g��� */
	BOOL				m_bFontIs_FIXED_PITCH;			/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
	BOOL				m_bBackUp;						/* �o�b�N�A�b�v�̍쐬 */
	BOOL				m_bBackUpDialog;				/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
	BOOL				m_bBackUpFolder;				/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
	char				m_szBackUpFolder[_MAX_PATH];	/* �o�b�N�A�b�v���쐬����t�H���_ */
	int 				m_nBackUpType;					/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	int 				m_nBackUpType_Opt1;				/* �o�b�N�A�b�v�t�@�C�����F�I�v�V����1 */
	int 				m_nBackUpType_Opt2;				/* �o�b�N�A�b�v�t�@�C�����F�I�v�V����2 */
	int 				m_nBackUpType_Opt3;				/* �o�b�N�A�b�v�t�@�C�����F�I�v�V����3 */
	int 				m_nBackUpType_Opt4;				/* �o�b�N�A�b�v�t�@�C�����F�I�v�V����4 */
	int 				m_nBackUpType_Opt5;				/* �o�b�N�A�b�v�t�@�C�����F�I�v�V����5 */
	int 				m_nBackUpType_Opt6;				/* �o�b�N�A�b�v�t�@�C�����F�I�v�V����6 */
	BOOL				m_bBackUpDustBox;			/* �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�� */	//@@@ 2001.12.11 add MIK
	BOOL				m_bBackUpPathAdvanced;				/* 20051107 aroka �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� */
	char				m_szBackUpPathAdvanced[_MAX_PATH];	/* 20051107 aroka �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� */
	int					m_nFileShareMode;				/* �t�@�C���̔r�����䃂�[�h */
	char				m_szExtHelp[_MAX_PATH];		/* �O���w���v�P */
	char				m_szExtHtmlHelp[_MAX_PATH];		/* �O��HTML�w���v */
	LOGFONT				m_lf_kh;						/* �L�[���[�h�w���v�̃t�H���g��� */		// ai 02/05/21 Add
	BOOL				m_bUseCaretKeyWord;				/* �L�����b�g�ʒu�̒P����������� */		// 2006.03.24 fon

	int					m_nToolBarButtonNum;			/* �c�[���o�[�{�^���̐� */
	int					m_nToolBarButtonIdxArr[MAX_TOOLBARBUTTONS];	/* �c�[���o�[�{�^���\���� */
	int					m_bToolBarIsFlat;				/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
	int					m_nMRUArrNum_MAX;
	int					m_nOPENFOLDERArrNum_MAX;
	BOOL				m_bDispTOOLBAR;					/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
	BOOL				m_bDispSTATUSBAR;				/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */

	BOOL				m_bDispFUNCKEYWND;				/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
	int					m_nFUNCKEYWND_Place;			/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	int					m_nFUNCKEYWND_GroupNum;			// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

	BOOL				m_bDispTabWnd;					//�^�u�E�C���h�E�\������	//@@@ 2003.05.31 MIK
	BOOL				m_bDispTabWndMultiWin;			//�^�u���܂Ƃ߂Ȃ�	//@@@ 2003.05.31 MIK
	BOOL				m_bTab_RetainEmptyWin;			//!< �Ō�̕���������ꂽ�Ƃ�(����)���c��
	BOOL				m_bTab_CloseOneWin;		//!< �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���

	char				m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//�^�u�E�C���h�E�L���v�V����	//@@@ 2003.06.13 MIK
	BOOL				m_bSameTabWidth;				//�^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
	BOOL				m_bDispTabIcon;					//�^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
	BOOL				m_bSortTabList;					//�^�u�ꗗ���\�[�g����	//@@@ 2006.03.23 fon
	BOOL				m_bTab_ListFull;				//�^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji

	BOOL				m_bSplitterWndHScroll;			// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�
	BOOL				m_bSplitterWndVScroll;			// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�

	/* �J�X�^�����j���[��� */
	char				m_szCustMenuNameArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int					m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	int					m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_szMidashiKigou[256];			/* ���o���L�� */
	char				m_szInyouKigou[32];				/* ���p�� */
	int					m_bUseHokan;					/* ���͕⊮�@�\���g�p���� */

	int					m_nSaveWindowSize;	// 2004.05.13 Moca �E�B���h�E�T�C�Y�p���E�Œ� eWINSIZEMODE�ɏ�����
	int					m_nWinSizeType;
	int					m_nWinSizeCX;
	int					m_nWinSizeCY;
	int					m_nSaveWindowPos;	// 2004.05.13 Moca �E�B���h�E�ʒu�p���E�Œ� eWINSIZEMODE�ɏ�����
	int					m_nWinPosX;
	int					m_nWinPosY;

	BOOL				m_bUseTaskTray;				/* �^�X�N�g���C�̃A�C�R�����g�� */
	BOOL				m_bStayTaskTray;			/* �^�X�N�g���C�̃A�C�R�����풓 */

	WORD				m_wTrayMenuHotKeyCode;		/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
	WORD				m_wTrayMenuHotKeyMods;		/* �^�X�N�g���C���N���b�N���j���[ �L�[ */

	BOOL				m_bUseOLE_DragDrop;			/* OLE�ɂ��h���b�O & �h���b�v���g�� */
	BOOL				m_bUseOLE_DropSource;		/* OLE�ɂ��h���b�O���ɂ��邩 */


	BOOL				m_bDispExitingDialog;			/* �I���_�C�A���O��\������ */
	BOOL				m_bEnableUnmodifiedOverwrite;	/* ���ύX�ł��㏑�����邩 */
	BOOL				m_bJumpSingleClickURL;			/* URL�̃V���O���N���b�N��Jump */
	BOOL				m_bSelectClickedURL;			/* URL���N���b�N���ꂽ��I�����邩 */
	BOOL				m_bGrepExitConfirm;				/* Grep���[�h�ŕۑ��m�F���邩 */

	int					m_nRulerHeight;				/* ���[���[���� */
	int					m_nRulerBottomSpace;		/* ���[���[�ƃe�L�X�g�̌��� */
	int					m_nRulerType;				/* ���[���[�̃^�C�v */
	int					m_nLineNumRightSpace;		// �s�ԍ��̉E�̃X�y�[�X Sep. 18, 2002 genta
	int					m_nVertLineOffset;			// �c���̕`����W�I�t�Z�b�g 2005.11.10 Moca

	BOOL				m_bCopyAndDisablSelection;	/* �R�s�[������I������ */
	BOOL				m_bHtmlHelpIsSingle;		/* HtmlHelp�r���[�A�͂ЂƂ� */
	BOOL				m_bCompareAndTileHorz;		/* ������r��A���E�ɕ��ׂĕ\�� */
	BOOL				m_bDropFileAndClose;		/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	int					m_nDropFileNumMax;			/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	BOOL				m_bCheckFileTimeStamp;		/* �X�V�̊Ď� */
	BOOL				m_bNotOverWriteCRLF;		/* ���s�͏㏑�����Ȃ� */
	RECT				m_rcOpenDialog;				/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
	BOOL				m_bAutoCloseDlgFind;		/* �����_�C�A���O�������I�ɕ��� */
	BOOL				m_bScrollBarHorz;			/* �����X�N���[���o�[���g�� */
	BOOL				m_bAutoCloseDlgFuncList;	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	BOOL				m_bAutoCloseDlgReplace;		/* �u�� �_�C�A���O�������I�ɕ��� */
	BOOL				m_bSearchAll;				/* �擪�i�����j����Č��� 2002.01.26 hor */
	BOOL				m_bAutoColmnPaste;			/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */

	BOOL				m_bHokanKey_RETURN;	/* VK_RETURN	�⊮����L�[���L��/���� */
	BOOL				m_bHokanKey_TAB;	/* VK_TAB		�⊮����L�[���L��/���� */
	BOOL				m_bHokanKey_RIGHT;	/* VK_RIGHT		�⊮����L�[���L��/���� */
	BOOL				m_bHokanKey_SPACE;	/* VK_SPACE		�⊮����L�[���L��/���� */


	int					m_nDateFormatType;						//���t�����̃^�C�v
	char				m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];//���t����
	int					m_nTimeFormatType;						//���������̃^�C�v
	char				m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];//��������

	BOOL				m_bMenuIcon;		/* ���j���[�ɃA�C�R����\������ */
	BOOL				m_bAutoMIMEdecode;	// �t�@�C���ǂݍ��ݎ���MIME��decode���s����
	BOOL				m_bQueryIfCodeChange;	// Oct. 03, 2004 genta �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��
	BOOL				m_bAlertIfFileNotExist;	// Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������

	BOOL				m_bMarkUpBlankLineEnable;	//2002.02.08 aroka,hor
	BOOL				m_bFunclistSetFocusOnJump;	//2002.02.08 hor

	// Apr. 5, 2003 genta
	char				m_szWindowCaptionActive[MAX_CAPTION_CONF_LEN];
	char				m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];

	//2004.10.28
	char				m_szMigemoDll[_MAX_PATH];		/* migemo dll */
	char				m_szMigemoDict[_MAX_PATH];		/* migemo dict */

//	int					m_nTagDepth;	//�_�C���N�g�^�O�W�����v�K�w

	// 20060201 aroka �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L��
	int					m_bRememberOutlineWindowPos;
	int					m_widthOutlineWindow;
	int					m_heightOutlineWindow;
	int					m_xOutlineWindowPos;
	int					m_yOutlineWindowPos;

	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
	BOOL				m_bNoFilterSaveNew;		// �V�K����ۑ����͑S�t�@�C���\��
	BOOL				m_bNoFilterSaveFile;	// �V�K�ȊO����ۑ����͑S�t�@�C���\��
}; /* Common */

//! ���L�f�[�^�̈�
struct DLLSHAREDATA {
	//	Oct. 27, 2000 genta
	//!	�f�[�^�\�� Version
	/*	�f�[�^�\���̈قȂ�o�[�W�����̓����N����h������
		�K���擪�ɂȂ��Ă͂Ȃ�Ȃ��D
	*/
	unsigned int		m_vStructureVersion;
	/* ���ʍ�ƈ�(�ۑ����Ȃ�) */
	char				m_szWork[32000];
	FileInfo			m_FileInfo_MYWM_GETFILEINFO;

	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
	LONG				m_nSequences;	/* �E�B���h�E�A�� */
	/**** ���ʍ�ƈ�(�ۑ�����) ****/
	int					m_nEditArrNum;	//short->int�ɏC��	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//�ő�l�C��	@@@ 2003.05.31 MIK

	//From Here 2003.05.31 MIK
	WINDOWPLACEMENT		m_TabWndWndpl;					//�^�u�E�C���h�E���̃E�C���h�E���
	//To Here 2003.05.31 MIK

//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRU���o�R���Ă��������B
	int					m_nMRUArrNum;
	FileInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//���C�ɓ���	//@@@ 2003.04.08 MIK

//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRUFolder���o�R���Ă��������B
	int					m_nOPENFOLDERArrNum;
	char				m_szOPENFOLDERArr[MAX_OPENFOLDER][_MAX_PATH];
	bool				m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	int					m_nTransformFileNameArrNum;
	char				m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	char				m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	int					m_nSEARCHKEYArrNum;
	char				m_szSEARCHKEYArr[MAX_SEARCHKEY][_MAX_PATH];
//	bool				m_bSEARCHKEYArrFavorite[MAX_SEARCHKEY];	//���C�ɓ���	//@@@ 2003.04.08 MIK
	int					m_nREPLACEKEYArrNum;
	char				m_szREPLACEKEYArr[MAX_REPLACEKEY][_MAX_PATH];
//	bool				m_bREPLACEKEYArrFavorite[MAX_REPLACEKEY];	//���C�ɓ���	//@@@ 2003.04.08 MIK
	int					m_nGREPFILEArrNum;
	char				m_szGREPFILEArr[MAX_GREPFILE][_MAX_PATH];
//	bool				m_bGREPFILEArrFavorite[MAX_GREPFILE];	//���C�ɓ���	//@@@ 2003.04.08 MIK
	int					m_nGREPFOLDERArrNum;
	char				m_szGREPFOLDERArr[MAX_GREPFOLDER][_MAX_PATH];
//	bool				m_bGREPFOLDERArrFavorite[MAX_GREPFOLDER];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	char				m_szMACROFOLDER[_MAX_PATH];		/* �}�N���p�t�H���_ */
	char				m_szIMPORTFOLDER[_MAX_PATH];	/* �ݒ�C���|�[�g�p�t�H���_ */
	
	//	Sep. 14, 2001 genta
	MacroRec			m_MacroTable[MAX_CUSTMACRO];	//!< �L�[���蓖�ėp�}�N���e�[�u��

	// 2004/06/21 �^�O�W�����v�@�\�ǉ�
	int					m_TagJumpNum;					//!< �^�O�W�����v���̗L���f�[�^��
	int					m_TagJumpTop;					//!< �X�^�b�N�̈�ԏ�̈ʒu
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< �^�O�W�����v���

//	MRU_ExtCmd			m_MRU_ExtCmd;	/* MRU �O���R�}���h */
	char				m_szCmdArr[MAX_CMDARR][MAX_CMDLEN];
	int					m_nCmdArrNum;
//	bool				m_bCmdArrFavorite[MAX_CMDARR];	//���C�ɓ���	//@@@ 2003.04.08 MIK


	/**** ���ʐݒ� ****/
	Common				m_Common;

	/* �L�[���蓖�� */
	int					m_nKeyNameArrNum;			/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	KEYDATA				m_pKeyNameArr[100];			/* �L�[���蓖�ĕ\ */

	/**** ����y�[�W�ݒ� ****/
	PRINTSETTING		m_PrintSettingArr[MAX_PRINTSETTINGARR];

	/* �����L�[���[�h�ݒ� */
	CKeyWordSetMgr		m_CKeyWordSetMgr;					/* �����L�[���[�h */
	char				m_szKeyWordSetDir[MAX_PATH];		/* �����L�[���[�h�t�@�C���̃f�B���N�g�� */

	/* **** �^�C�v�ʐݒ� **** */
	Types				m_Types[MAX_TYPES];

	/*	@@@ 2002.1.24 YAZAKI
		�L�[�{�[�h�}�N���́A�L�^�I���������_�Ńt�@�C���um_szKeyMacroFileName�v�ɏ����o�����Ƃɂ���B
		m_bRecordingKeyMacro��TRUE�̂Ƃ��́A�L�[�{�[�h�}�N���̋L�^���Ȃ̂ŁAm_szKeyMacroFileName�ɃA�N�Z�X���Ă͂Ȃ�Ȃ��B
	*/
	BOOL				m_bRecordingKeyMacro;		/* �L�[�{�[�h�}�N���̋L�^�� */
	HWND				m_hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	char				m_szKeyMacroFileName[MAX_PATH];	/* �L�[�{�[�h�}�N���̃t�@�C���� */

//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	/* **** ���̑��̃_�C�A���O **** */
	BOOL				m_bGetStdout;		/* �O���R�}���h���s�́u�W���o�͂𓾂�v */
	BOOL				m_bLineNumIsCRLF;	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */

	int					m_nDiffFlgOpt;		/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
	
	char				m_szTagsCmdLine[_MAX_PATH];	/* TAGS�R�}���h���C���I�v�V���� */	//@@@ 2003.05.12 MIK
	int					m_nTagsOpt;			/* TAGS�I�v�V����(�`�F�b�N) */	//@@@ 2003.05.12 MIK
	//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
	int					m_nTagJumpKeywordArrNum;
	char				m_szTagJumpKeywordArr[MAX_TAGJUMP_KEYWORD][_MAX_PATH];
//	bool				m_bTagJumpKeywordArrFavorite[MAX_TAGJUMPKEYWORD];	//���C�ɓ���
	BOOL				m_bTagJumpICase;	//!< �啶���������𓯈ꎋ
	BOOL				m_bTagJumpAnyWhere;	//!< ������̓r���Ƀ}�b�`
	//To Here 2005.04.03 MIK
};



/*!	@brief ���L�f�[�^�̊Ǘ�

	CShareData��CProcess�̃����o�ł��邽�߁C���҂̎����͓���ł��D
	�{����CProcess�I�u�W�F�N�g��ʂ��ăA�N�Z�X����ׂ��ł����C
	CProcess���̃f�[�^�̈�ւ̃|�C���^��static�ϐ��ɕۑ����邱�Ƃ�
	Singleton�̂悤�ɂǂ�����ł��A�N�Z�X�ł���\���ɂȂ��Ă��܂��D

	���L�������ւ̃|�C���^��m_pShareData�ɕێ����܂��D���̃����o��
	���J����Ă��܂����CCShareData�ɂ����Map/Unmap����邽�߂�
	ChareData�̏��łɂ���ă|�C���^m_pShareData�������ɂȂ邱�Ƃ�
	���ӂ��Ă��������D

	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
class SAKURA_CORE_API CShareData
{
public:
	/*
	||	Singleton��
	*/
	static CShareData* getInstance();

protected:
	static CShareData* _instance;

public:
	/*
	||  Constructors
	*/
	CShareData();
	~CShareData();

	/*
	||  Attributes & Operations
	*/
	bool Init(void);	/* CShareData�N���X�̏��������� */
	DLLSHAREDATA* GetShareData(){ return m_pShareData; }		/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	int GetDocumentType( const char* pszFilePath );				/* �t�@�C���p�X��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	int GetDocumentTypeExt( const char* pszExt );				/* �g���q��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	BOOL AddEditWndList( HWND );								/* �ҏW�E�B���h�E�̓o�^ */
	void DeleteEditWndList( HWND );								/* �ҏW�E�B���h�E���X�g����̍폜 */

	BOOL RequestCloseAllEditor( BOOL bExit );					/* �S�ҏW�E�B���h�E�֏I���v�����o�� */	// 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	BOOL IsPathOpened( const char*, HWND* );					/* �w��t�@�C�����J����Ă��邩���ׂ� */
	int GetEditorWindowsNum( void );							/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
	BOOL PostMessageToAllEditors( UINT, WPARAM, LPARAM, HWND );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
	BOOL SendMessageToAllEditors( UINT, WPARAM, LPARAM, HWND );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂邷�� */
	int GetOpenedWindowArr( EditNode** , BOOL );				/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ� */
	static BOOL IsEditWnd( HWND );								/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
	static void SetKeyNameArrVal(
		DLLSHAREDATA*, int, short, char*,
		short, short, short, short,
		short, short, short, short );									/* KEYDATA�z��Ƀf�[�^���Z�b�g */
//	static void SetKeyNameArrVal( DLLSHAREDATA*, int, short, char* );	/* KEYDATA�z��Ƀf�[�^���Z�b�g */ // 20050818 aroka ���g�p�Ȃ̂ō폜
	static LONG MY_RegSetVal(
		HKEY hKey,				// handle of key to set value for
		LPCTSTR lpValueName,	// address of value to set
		CONST BYTE *lpData,		// address of value data
		DWORD cbData 			// size of value data
	);
	static LONG MY_RegQuerVal(
		HKEY hKey,				// handle of key to set value for
		LPCTSTR lpValueName,	// address of value to set
		BYTE *lpData,			// address of value data
		DWORD cbData 			// size of value data
	);
	void TraceOut( LPCTSTR lpFmt, ...);	/* �f�o�b�O���j�^�ɏo�� */
	BOOL LoadShareData( void );	/* ���L�f�[�^�̃��[�h */
	void SaveShareData( void );	/* ���L�f�[�^�̕ۑ� */
	BOOL ShareData_IO_2( bool );	/* ���L�f�[�^�̕ۑ� */
	static void IO_ColorSet( CProfile* , const char* , ColorInfo* );	/* �F�ݒ� I/O */ // Feb. 12, 2006 D.S.Koba

//	int			m_nStdToolBarButtons; 2004.03.30 Moca ���g�p

	//@@@ 2002.2.2 YAZAKI
	//	Jun. 14, 2003 genta �����ǉ��D�����ύX
	int		GetMacroFilename( int idx, char* pszPath, int nBufLen ); // idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����
	bool		BeReloadWhenExecuteMacro( int idx );	//	idx�Ŏw�肵���}�N���́A���s���邽�тɃt�@�C����ǂݍ��ސݒ肩�H
	void		AddToSearchKeyArr( const char* pszSearchKey );	//	m_szSEARCHKEYArr��pszSearchKey��ǉ�����
	void		AddToReplaceKeyArr( const char* pszReplaceKey );	//	m_szREPLACEKEYArr��pszReplaceKey��ǉ�����
	void		AddToGrepFileArr( const char* pszGrepFile );		//	m_szGREPFILEArr��pszGrepFile��ǉ�����
	void		AddToGrepFolderArr( const char* pszGrepFolder );	//	m_nGREPFOLDERArrNum��pszGrepFolder��ǉ�����

	//@@@ 2002.2.3 YAZAKI
	bool		ExtWinHelpIsSet( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��ɁA�O���w���v���ݒ肳��Ă��邩�B
	char*		GetExtWinHelp( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��́A�O���w���v�t�@�C�������擾�B
	bool		ExtHTMLHelpIsSet( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��ɁA�O��HTML�w���v���ݒ肳��Ă��邩�B
	char*		GetExtHTMLHelp( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�t�@�C�������擾�B
	bool		HTMLHelpIsSingle( int nType = -1 );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�u�r���[�A�𕡐��N�����Ȃ��v��ON�����擾�B
	
	//@@@ 2002.2.9 YAZAKI
	const char* MyGetDateFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen );
	const char* MyGetTimeFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen );
	const char* MyGetDateFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen, int nDateFormatType, char* szDateFormat );
	const char* MyGetTimeFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen, int nTimeFormatType, char* szTimeFormat );
	
	// 2002.11.24 Moca Add
//	LPTSTR GetTransformFileList( LPCTSTR*, LPTSTR*, int );
//	LPTSTR GetTransformFileName( LPCTSTR, LPTSTR, int );
	LPTSTR GetTransformFileNameFast( LPCTSTR, LPTSTR, int );
	int TransformFileName_MakeCache( void );
	static LPCTSTR GetFilePathFormat( LPCTSTR, LPTSTR, int, LPCTSTR, LPCTSTR );
	static bool ExpandMetaToFolder( LPCTSTR, LPTSTR, int );

	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	void PushTagJump(const TagJump *);		//!< �^�O�W�����v���̕ۑ�
	bool PopTagJump(TagJump *);				//!< �^�O�W�����v���̎Q��

protected:
	/*
	||  �����w���p�֐�
	*/
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;

//	long GetModuleDir(char* , long );	/* ���̎��s�t�@�C���̂���f�B���N�g����Ԃ��܂� */
	/* MRU��OPENFOLDER���X�g�̑��݃`�F�b�N�Ȃ�
	���݂��Ȃ��t�@�C����t�H���_��MRU��OPENFOLDER���X�g����폜����
	 */
//@@@ 2002.01.03 YAZAKI CMRU�ACMRUFolder�Ɉړ������B
//	void CheckMRUandOPENFOLDERList( void );

	// �t�@�C�����ȈՕ\���p�L���b�V��
	int m_nTransformFileNameCount; // �L����
	TCHAR m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];

	//	Jan. 30, 2005 genta �������֐��̕���
	void InitKeyword(DLLSHAREDATA*);
	void InitKeyAssign(DLLSHAREDATA*);
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfig(DLLSHAREDATA*);
	void InitPopupMenu(DLLSHAREDATA*);
	
	// Feb. 12, 2006 D.S.Koba
	void ShareData_IO_Mru( CProfile& );
	void ShareData_IO_Keys( CProfile& );
	void ShareData_IO_Grep( CProfile& );
	void ShareData_IO_Folders( CProfile& );
	void ShareData_IO_Cmd( CProfile& );
	void ShareData_IO_Nickname( CProfile& );
	void ShareData_IO_Common( CProfile& );
	void ShareData_IO_Toolbar( CProfile& );
	void ShareData_IO_CustMenu( CProfile& );
	void ShareData_IO_Font( CProfile& );
	void ShareData_IO_KeyBind( CProfile& );
	void ShareData_IO_Print( CProfile& );
	void ShareData_IO_Types( CProfile& );
	void ShareData_IO_KeyWords( CProfile& );
	void ShareData_IO_Macro( CProfile& );
	void ShareData_IO_Other( CProfile& );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */


/*[EOF]*/
