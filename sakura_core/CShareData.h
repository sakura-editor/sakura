//	$Id$
/************************************************************************
	CShareData.h
	�v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X
	Copyright (C) 1998-2000, Norio Nakatani

	UPDATE:
	CREATE: 1998/5/26  �V�K�쐬
************************************************************************/

class CShareData;

#ifndef _CSHAREDATA_H_
#define _CSHAREDATA_H_

#include <windows.h>
#include <commctrl.h>
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CPrint.h"
#include "CKeyMacroMgr.h"
#include "CProfile.h"

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_MRU						=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z ��36�ɂȂ�̂ł���ɍ��킹��30��36�ɕύX
	MAX_OPENFOLDER				=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z ��36�ɂȂ�̂ł���ɍ��킹��30��36�ɕύX
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  16,
//	MAX_TOOLBARBUTTONS			= 256,
	MAX_TOOLBARBUTTONS			= 384,	//Oct. 22, 2000 JEPORO �A�C�R���̍ő�o�^����128���₵��(256��384)
	MAX_CUSTOM_MENU				=  25,
	MAX_CUSTOM_MENU_NAME_LEN	=  32,
	MAX_CUSTOM_MENU_ITEMS		=  48,
	MAX_PRINTSETTINGARR			=   8,

	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_DATETIMEFOREMAT_LEN		= 100,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32

};


///* �O���R�}���h��� */
//struct ExtCmd {
//	char m_szCmd[MAX_EXTCMDLEN + 1];
//	/* ������ */
//	void ExtCmd::Init(void){
//		m_szCmd[0] = '\0';
//		return;
//	}
//};


///* MRU �O���R�}���h��� */
//struct MRU_ExtCmd {
//	int		m_nExtCmdArrNum;
//	ExtCmd	m_ExtCmdArr[MAX_EXTCMDMRUNUM];
//};

/* �t�@�C����� */
struct FileInfo {
	int		m_nViewTopLine;			/* �\����̈�ԏ�̍s(0�J�n) */
	int		m_nViewLeftCol;			/* �\����̈�ԍ��̌�(0�J�n) */
	int		m_nX;					/* �J�[�\��  �����ʒu(�s������̃o�C�g��) */
	int		m_nY;					/* �J�[�\��  �����ʒu(�܂�Ԃ������s�ʒu) */
	int		m_bIsModified;			/* �ύX�t���O */
	int		m_nCharCode;			/* �����R�[�h��� */
	char	m_szPath[_MAX_PATH];	/* �t�@�C���� */
	BOOL	m_bIsGrep;				/* Grep�̃E�B���h�E�� */
	char	m_szGrepKey[1024];
};

struct EditNode {
	int				m_nIndex;
	HWND			m_hWnd;
};

struct PRINTSETTING {
	char			m_szPrintSettingName[32 + 1];		/* ����ݒ�̖��O */
//	char			m_szPrintFontFace[LF_FACESIZE];		/* ����t�H���g */
	char			m_szPrintFontFaceHan[LF_FACESIZE];	/* ����t�H���g */
	char			m_szPrintFontFaceZen[LF_FACESIZE];	/* ����t�H���g */
	int				m_nPrintFontWidth;					/* ����t�H���g��(1/10mm�P�ʒP��) */
	int				m_nPrintFontHeight;					/* ����t�H���g����(1/10mm�P�ʒP��) */
	int				m_nPrintDansuu;						/* �i�g�̒i�� */
	int				m_nPrintDanSpace;					/* �i�ƒi�̌���(1/10mm�P��) */
	int				m_nPrintLineSpacing;				/* ����t�H���g�s�� �����̍����ɑ΂��銄��(%) */
	int				m_nPrintMarginTY;					/* ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginBY;					/* ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginLX;					/* ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginRX;					/* ����p���}�[�W�� �E(mm�P��) */
	int				m_nPrintPaperOrientation;			/* �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2) */
	int				m_nPrintPaperSize;					/* �p���T�C�Y */
	BOOL			m_bPrintWordWrap;					/* �p�����[�h���b�v���� */
	BOOL			m_bPrintLineNumber;					/* �s�ԍ���������� */


	MYDEVMODE		m_mdmDevMode;						/* �v�����^�ݒ� DEVMODE�p */
	BOOL			m_bHeaderUse[3];
	char			m_szHeaderForm[3][100];
	BOOL			m_bFooterUse[3];
	char			m_szFooterForm[3][100];
};

// Stonee ���F 2000/01/12
// ������ύX�����Ƃ��́ACCshareData_new.cpp ��colorIDXKeyName�̒�`���ύX���ĉ������B

//	From Here Sept. 18, 2000 JEPRO ���Ԃ�啝�ɓ���ւ���
//#define COLORIDX_TEXT			0	/* �e�L�X�g */
//#define COLORIDX_GYOU			1	/* �s�ԍ� */
//#define COLORIDX_CRLF			2	/* ���s�L�� */
//#define COLORIDX_TAB			3	/* TAB�L�� */
//#define COLORIDX_ZENSPACE		4	/* ���{��� */
//#define COLORIDX_EOF			5	/* EOF�L�� */
//#define COLORIDX_KEYWORD		6	/* �����L�[���[�h */
//#define COLORIDX_COMMENT		7	/* �R�����g */
//#define COLORIDX_SSTRING		8	/* �V���O���N�H�[�e�[�V���������� */
//#define COLORIDX_WSTRING		9	/* �_�u���N�H�[�e�[�V���������� */
//#define COLORIDX_UNDERLINE	10	/* �J�[�\���s�A���_�[���C��	*/
//#define COLORIDX_WRAP			11	/* �܂�Ԃ��L��	*/
//#define COLORIDX_CTRLCODE		12	/* �R���g���[���R�[�h */
//#define COLORIDX_URL			13	/* URL */
//#define COLORIDX_SEARCH		14	/* ���������� */
//#define COLORIDX_GYOU_MOD		15	/* �s�ԍ�(�ύX�s) */
//#define COLORIDX_RULER		16	/* ���[���[ */
//#define COLORIDX_LAST			17
#define COLORIDX_TEXT			0	/* �e�L�X�g */
#define COLORIDX_RULER			1	/* ���[���[ */
#define COLORIDX_UNDERLINE		2	/* �J�[�\���s�A���_�[���C��	*/
#define COLORIDX_GYOU			3	/* �s�ԍ� */
#define COLORIDX_GYOU_MOD		4	/* �s�ԍ�(�ύX�s) */
#define COLORIDX_TAB			5	/* TAB�L�� */
#define COLORIDX_ZENSPACE		6	/* ���{��� */
#define COLORIDX_CTRLCODE		7	/* �R���g���[���R�[�h */
#define COLORIDX_CRLF			8	/* ���s�L�� */
#define COLORIDX_WRAP			9	/* �܂�Ԃ��L��	*/
#define COLORIDX_EOF			10	/* EOF�L�� */
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
//#ifdef COMPILE_COLOR_DIGIT
#define COLORIDX_DIGIT			11	/* ���p���l */	//@@@ 2001.02.17 by MIK	//�F�ݒ�Ver.3���烆�[�U�t�@�C���ɑ΂��Ă͕�����ŏ������Ă���̂Ń��i���o�����O���Ă��悢. Mar. 7, 2001 JEPRO noted
#define COLORIDX_SEARCH			12	/* ���������� */
#define COLORIDX_KEYWORD		13	/* �����L�[���[�h1 */
#define COLORIDX_KEYWORD2		14	/* �����L�[���[�h2 */					//MIK ADDED
#define COLORIDX_COMMENT		15	/* �R�����g */							//Dec. 4, 2000 shifted by MIK
#define COLORIDX_SSTRING		16	/* �V���O���N�H�[�e�[�V���������� */	//Dec. 4, 2000 shifted by MIK
#define COLORIDX_WSTRING		17	/* �_�u���N�H�[�e�[�V���������� */		//Dec. 4, 2000 shifted by MIK
#define COLORIDX_URL			18	/* URL */								//Dec. 4, 2000 shifted by MIK
#define COLORIDX_LAST			19											//Dec. 4, 2000 @@@2001.02.17 renumber by MIK
//#else
//#define COLORIDX_LAST			18											//Dec. 4, 2000
//#endif
//	To Here Sept. 18, 2000

/* �F�ݒ� */
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

struct ColorInfoIni {
	const char*	m_pszName;			/* �F�� */
	BOOL		m_bDisp;			/* �F����/�\�� ������ */
	BOOL		m_bFatFont;			/* ������ */
	BOOL		m_bUnderLine;		/* �A���_�[���C���� */
	COLORREF	m_colTEXT;			/* �O�i�F(�����F) */
	COLORREF	m_colBACK;			/* �w�i�F */
};

///* Ver 0.3.5.0�̐ݒ�f�[�^�\���� */
//#include "CShareData_0_3_5_0.h"




struct Types {
	int					m_nIdx;
	char				m_szTypeName[64];				/* �^�C�v�����F���� */
	char				m_szTypeExts[64];				/* �^�C�v�����F�g���q���X�g */
	int					m_nMaxLineSize;					/* �܂�Ԃ������� */
	int					m_nColmSpace;					/* �����ƕ����̌��� */
	int					m_nLineSpace;					/* �s�Ԃ̂����� */
	int					m_nTabSpace;					/* TAB�̕����� */
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	char				m_szTabViewString[9];			/* TAB�\�������� */
//#endif
//	int					m_bDispLINE;					/* �s�ԍ��\���^��\�� */
//	BOOL				m_bDispTAB;						/* �^�u�L����\������ */
//	BOOL				m_bDispZENSPACE;				/* ���{��󔒂�\������ */
//	BOOL				m_bDispCRLF;					/* ���s�L����\������ */
//	BOOL				m_bDispEOF;						/* EOF��\������ */
//	BOOL				m_bDispCCPPKEYWORD;				/* �����L�[���[�h��\������ */
//	BOOL				m_bDispCOMMENT;					/* �R�����g��\������ */
//	BOOL				m_bDispSSTRING;					/* �V���O���N�H�[�e�[�V�����������\������ */
//	BOOL				m_bDispWSTRING;					/* �_�u���N�H�[�e�[�V�����������\������ */
//	BOOL				m_bDispUNDERLINE;				/* �J�[�\���s�A���_�[���C����\������ */
//	COLORREF			m_colorTEXT;					/* �e�L�X�g�F */
//	COLORREF			m_colorBACK;					/* �w�i�F */
//	COLORREF			m_colorCRLF;					/* ���s�̐F */
//	COLORREF			m_colorCRLFBACK;				/* ���s�w�i�̐F */
//	COLORREF			m_colorGYOU;					/* �s�ԍ��̐F */
//	COLORREF			m_colorGYOUBACK;				/* �s�ԍ��w�i�̐F */
//	COLORREF			m_colorTAB;						/* TAB�����̐F */
//	COLORREF			m_colorTABBACK;					/* TAB�����w�i�̐F */
//	COLORREF			m_colorZENSPACE;				/* �S�p�X�y�[�X�����̐F */
//	COLORREF			m_colorZENSPACEBACK;			/* �S�p�X�y�[�X�����w�i�̐F */
//	COLORREF			m_colorEOF;						/* EOF�̐F */
//	COLORREF			m_colorEOFBACK;					/* EOF�w�i�̐F */
//	COLORREF			m_colorCCPPKEYWORD;				/* �����L�[���[�h�̐F */
//	COLORREF			m_colorCCPPKEYWORDBACK;			/* �����L�[���[�h�w�i�̐F */
	int					m_nKeyWordSetIdx;				/* �L�[���[�h�Z�b�g1 */
	int					m_nKeyWordSetIdx2;				/* �L�[���[�h�Z�b�g2 */	//Dec. 4, 2000 MIK
//	COLORREF			m_colorCOMMENT;					/* �R�����g�F */
//	COLORREF			m_colorCOMMENTBACK;				/* �R�����g�w�i�̐F */
//	COLORREF			m_colorSSTRING;					/* �V���O���N�H�[�e�[�V����������F */
//	COLORREF			m_colorSSTRINGBACK;				/* �V���O���N�H�[�e�[�V����������w�i�̐F */
//	COLORREF			m_colorWSTRING;					/* �_�u���N�H�[�e�[�V����������F */
//	COLORREF			m_colorWSTRINGBACK;				/* �_�u���N�H�[�e�[�V����������w�i�̐F */
//	COLORREF			m_colorUNDERLINE;				/* �J�[�\���s�A���_�[���C���̐F */
	char				m_szLineComment[16];			/* �s�R�����g�f���~�^ */
	char				m_szLineComment2[16];			/* �s�R�����g�f���~�^2 */
	//	May. 12, 2001 genta
	int					m_nLineCommentPos;				//< �s�R�����g1�̊J�n�ʒu(�����͎w�薳��)
	int					m_nLineCommentPos2;				//< �s�R�����g2�̊J�n�ʒu(�����͎w�薳��)
	
	char				m_szBlockCommentFrom[16];		/* �u���b�N�R�����g�f���~�^(From) */
	char				m_szBlockCommentTo[16];			/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	char				m_szBlockCommentFrom2[16];		/* �u���b�N�R�����g�f���~�^(From) */ //@@@ 2001.03.10 by MIK
	char				m_szBlockCommentTo2[16];		/* �u���b�N�R�����g�f���~�^(To) */ //@@@ 2001.03.10 by MIK
//#endif
	int					m_nStringType;					/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	char				m_szIndentChars[64];			/* ���̑��̃C���f���g�Ώە��� */
//	int					m_nKEYWORDCASE;					/* �L�[���[�h�̉p�啶����������� */

	int					m_nColorInfoArrNum;				/* �F�ݒ�z��̗L���� */
	ColorInfo			m_ColorInfoArr[64];				/* �F�ݒ�z�� */

	int					m_bLineNumIsCRLF;				/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	int					m_nLineTermType;				/* �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
	char				m_cLineTermChar;				/* �s�ԍ���؂蕶�� */

//	char				m_bUseLBCC;						/* �֑��������s���� */
	BOOL				m_bWordWrap;					/* �p�����[�h���b�v������ */

	int					m_nCurrentPrintSetting;			/* ���ݑI�����Ă������ݒ� */

	int					m_nDefaultOutline;				/* �A�E�g���C����͕��@ */

	int					m_nSmartIndent;					/* �X�}�[�g�C���f���g��� */
	int					m_nImeState;	//	Nov. 20, 2000 genta ����IME���

}; /* Types */

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

	//	Oct. 27, 2000 genta
	//	�J�[�\���ʒu�𕜌����邩�ǂ���
	bool	GetRestoreCurPosition(void) const { return m_bRestoreCurPosition != 0; }
	void	SetRestoreCurPosition(bool i){ m_bRestoreCurPosition = i; }

	//	Nov. 12, 2000 genta
	//	�t�@�C���ǂݍ��ݎ���MIME��decode���s����
	bool	GetAutoMIMEdecode(void) const { return m_bAutoMIMEdecode != 0; }
	void	SetAutoMIMEdecode(bool i){ m_bAutoMIMEdecode = i; }

	//	����: �ݒ�t�@�C������̓ǂݍ��ݎ���INT�Ƃ��Ĉ������߁Cbool�^���g���Ă͂����Ȃ��D
	//	sizeof(int) != sizeof(bool)���ƃf�[�^��j�󂵂Ă��܂��D

	int					m_nCaretType;					/* �J�[�\���̃^�C�v 0=win 1=dos  */
	int					m_bIsINSMode;					/* �}���^�㏑�����[�h */
	int					m_bIsFreeCursorMode;			/* �t���[�J�[�\�����[�h�� */
	int					m_bAutoIndent;					/* �I�[�g�C���f���g */
	int					m_bAutoIndent_ZENSPACE;			/* ���{��󔒂��C���f���g */
	BOOL				m_bRestoreCurPosition;			//	�t�@�C�����J�����Ƃ��J�[�\���ʒu�𕜌����邩
//	int					m_bEnableLineISlog;				/* ���p�~���s�ԍ����  �����s�^�_���s */

//	char				m_szEMailUserName[_MAX_PATH];	/* ���[�����[�U�[�� */
//	char				m_szEMailUserAddress[_MAX_PATH];/* ���[���A�h���X */
//	char				m_szSMTPServer[_MAX_PATH];		/* SMTP�z�X�g���E�A�h���X */
//	int					m_nSMTPPort;					/* SMTP�|�[�g�ԍ�(�ʏ��25) */

	int					m_bAddCRLFWhenCopy;				/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
	int					m_nRepeatedScrollLineNum;		/* �L�[���s�[�g���̃X�N���[���s�� */
	BOOL				m_nRepeatedScroll_Smooth;		/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	BOOL				m_bExitConfirm;					/* �I�����̊m�F������ */
//	short				m_nKeyNameArrNum;				/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
//	KEYDATA				m_pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */
	int					m_bRegularExp;					/* �����^�u��  1==���K�\�� */
	int					m_bLoHiCase;					/* �����^�u��  1==�p�啶���������̋�� */
	int					m_bWordOnly;					/* �����^�u��  1==�P��̂݌��� */
	int					m_bNOTIFYNOTFOUND;				/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	int					m_bSelectedArea;				/* �u��       �I��͈͓��u�� */
	int					m_bGrepSubFolder;				/* Grep: �T�u�t�H���_������ */
	BOOL				m_bGrepOutputLine;				/* Grep: �s���o�͂��邩�Y�����������o�͂��邩 */
	int					m_nGrepOutputStyle;				/* Grep: �o�͌`�� */

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
	int					m_nFileShareMode;				/* �t�@�C���̔r�����䃂�[�h */
	char				m_szExtHelp1[_MAX_PATH];		/* �O���w���v�P */
	char				m_szExtHtmlHelp[_MAX_PATH];		/* �O��HTML�w���v */


	int					m_nToolBarButtonNum;			/* �c�[���o�[�{�^���̐� */
	int					m_nToolBarButtonIdxArr[MAX_TOOLBARBUTTONS];	/* �c�[���o�[�{�^���\���� */
	int					m_bToolBarIsFlat;				/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
	int					m_nMRUArrNum_MAX;
	int					m_nOPENFOLDERArrNum_MAX;
	BOOL				m_bDispTOOLBAR;					/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
	BOOL				m_bDispSTATUSBAR;				/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */

	BOOL				m_bDispFUNCKEYWND;				/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
	int					m_nFUNCKEYWND_Place;			/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */

	/* �J�X�^�����j���[��� */
	char				m_szCustMenuNameArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int					m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	int					m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_szMidashiKigou[256];			/* ���o���L�� */
	char				m_szInyouKigou[32];				/* ���p�� */
	int					m_bUseKeyWordHelp;				/* �L�[���[�h�w���v���g�p���� */
	char				m_szKeyWordHelpFile[_MAX_PATH];	/* �L�[���[�h�w���v �����t�@�C�� */
	int					m_bUseHokan;					/* ���͕⊮�@�\���g�p���� */
	char				m_szHokanFile[_MAX_PATH];		/* ���͕⊮ �P��t�@�C�� */
	BOOL				m_bGrepKanjiCode_AutoDetect;	/* Grep: �����R�[�h�������� */
	int					m_bHokanLoHiCase;				/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
//	PRINTSETTING		m_PrintSettingArr[MAX_PRINTSETTINGARR];
	BOOL				m_bSaveWindowSize;				/* �E�B���h�E�T�C�Y�p�� */
	int					m_nWinSizeType;
	int					m_nWinSizeCX;
	int					m_nWinSizeCY;


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

//	BOOL				m_bRulerDisp;				/* ���[���[�\�� */
	int					m_nRulerHeight;				/* ���[���[���� */
	int					m_nRulerBottomSpace;		/* ���[���[�ƃe�L�X�g�̌��� */
	int					m_nRulerType;				/* ���[���[�̃^�C�v */


	BOOL				m_bCopyAndDisablSelection;	/* �R�s�[������I������ */
	BOOL				m_bHtmlHelpIsSingle;		/* HtmlHelp�r���[�A�͂ЂƂ� */
	BOOL				m_bCompareAndTileHorz;		/* ������r��A���E�ɕ��ׂĕ\�� *
//	BOOL				m_bCompareAndTileHorz;		/* ������r��A���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
	BOOL				m_bDropFileAndClose;		/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	int					m_nDropFileNumMax;			/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	BOOL				m_bCheckFileTimeStamp;		/* �X�V�̊Ď� */
	BOOL				m_bNotOverWriteCRLF;		/* ���s�͏㏑�����Ȃ� */
	RECT				m_rcOpenDialog;				/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
	BOOL				m_bAutoCloseDlgFind;		/* �����_�C�A���O�������I�ɕ��� */
	BOOL				m_bScrollBarHorz;			/* �����X�N���[���o�[���g�� */
	BOOL				m_bAutoCloseDlgFuncList;	/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	BOOL				m_bAutoCloseDlgReplace;		/* �u�� �_�C�A���O�������I�ɕ��� */
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


}; /* Common */



struct DLLSHAREDATA {
	//	Oct. 27, 2000 genta
	//	�f�[�^�\�� Version
	//	�f�[�^�\���̈قȂ�o�[�W�����̓����N����h������
	//	�K���擪�ɂȂ��Ă͂Ȃ�Ȃ��D
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
	short				m_nEditArrNum;
	EditNode			m_pEditArr[MAX_EDITWINDOWS + 1];
	int					m_nMRUArrNum;
	FileInfo			m_fiMRUArr[MAX_MRU];
	int					m_nOPENFOLDERArrNum;
	char				m_szOPENFOLDERArr[MAX_OPENFOLDER][_MAX_PATH];
	int					m_nSEARCHKEYArrNum;
	char				m_szSEARCHKEYArr[MAX_SEARCHKEY][_MAX_PATH];
	int					m_nREPLACEKEYArrNum;
	char				m_szREPLACEKEYArr[MAX_REPLACEKEY][_MAX_PATH];
	int					m_nGREPFILEArrNum;
	char				m_szGREPFILEArr[MAX_GREPFILE][_MAX_PATH];
	int					m_nGREPFOLDERArrNum;
	char				m_szGREPFOLDERArr[MAX_GREPFOLDER][_MAX_PATH];

	char				m_szMACROFOLDER[_MAX_PATH];		/* �}�N���p�t�H���_ */
	char				m_szIMPORTFOLDER[_MAX_PATH];	/* �ݒ�C���|�[�g�p�t�H���_ */

//	MRU_ExtCmd			m_MRU_ExtCmd;	/* MRU �O���R�}���h */
	char				m_szCmdArr[MAX_CMDARR][MAX_CMDLEN];
	int					m_nCmdArrNum;


	/**** ���ʐݒ� ****/
//	BOOL				m_nCommonModify;	/* �ύX�t���O(���ʐݒ�̑S��) */
	Common				m_Common;

	/* �ύX�t���O �t�H���g */
//	BOOL				m_bFontModify;

	/* �L�[���蓖�� */
//	BOOL				m_bKeyBindModify;			/* �ύX�t���O �L�[���蓖�� */
//	BOOL				m_bKeyBindModifyArr[100];	/* �ύX�t���O �L�[���蓖��(�L�[����) */
	short				m_nKeyNameArrNum;			/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	KEYDATA				m_pKeyNameArr[100];			/* �L�[���蓖�ĕ\ */

	/**** ����y�[�W�ݒ� ****/
//	BOOL				m_bPrintSettingModify;							/* �ύX�t���O(����̑S��) */
//	BOOL				m_bPrintSettingModifyArr[MAX_PRINTSETTINGARR];	/* �ύX�t���O(����ݒ育��) */
	PRINTSETTING		m_PrintSettingArr[MAX_PRINTSETTINGARR];

	/* �����L�[���[�h�ݒ� */
//	BOOL				m_bKeyWordSetModify;				/* �ύX�t���O(�L�[���[�h�̑S��) */
//	BOOL				m_bKeyWordSetModifyArr[MAX_SETNUM];	/* �ύX�t���O(�L�[���[�h�̃Z�b�g����) */
	CKeyWordSetMgr		m_CKeyWordSetMgr;					/* �����L�[���[�h */
	char				m_szKeyWordSetDir[MAX_PATH];		/* �����L�[���[�h�t�@�C���̃f�B���N�g�� */

	/* **** �^�C�v�ʐݒ� **** */
//	BOOL				m_nTypesModifyArr[MAX_TYPES];	/* �ύX�t���O(�^�C�v�ʐݒ�) */
	Types				m_Types[MAX_TYPES];

	CKeyMacroMgr		m_CKeyMacroMgr;				/* �L�[���[�h�}�N���̃o�b�t�@ */
	BOOL				m_bRecordingKeyMacro;		/* �L�[�{�[�h�}�N���̋L�^�� */
	HWND				m_hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
};



/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CShareData
{
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
	DLLSHAREDATA* GetShareData( const char*, int* );			/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	BOOL AddEditWndList( HWND );								/* �ҏW�E�B���h�E�̓o�^ */
	void DeleteEditWndList( HWND );								/* �ҏW�E�B���h�E���X�g����̍폜 */

//	void AddMRUList( const char* );								/* MRU���X�g�ւ̓o�^ */
	void AddMRUList( FileInfo*  );								/* MRU���X�g�ւ̓o�^ */
	void AddOPENFOLDERList( const char* );						/* �J�����t�H���_���X�g�ւ̓o�^ */
	BOOL IsExistInMRUList( const char* , FileInfo*  );			/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
	BOOL RequestCloseAllEditor( void );							/* �S�ҏW�E�B���h�E�֏I���v�����o�� */
	BOOL IsPathOpened( const char*, HWND* );					/* �w��t�@�C�����J����Ă��邩���ׂ� */
	int GetEditorWindowsNum( void );							/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
	BOOL PostMessageToAllEditors( UINT, WPARAM, LPARAM, HWND );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
	BOOL SendMessageToAllEditors( UINT, WPARAM, LPARAM, HWND );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂邷�� */
	int GetOpenedWindowArr( EditNode** , BOOL );				/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ� */
	static BOOL IsEditWnd( HWND );								/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
	static void SetTBBUTTONVal( TBBUTTON*, int, int, BYTE, BYTE, DWORD, int );	/* TBBUTTON�\���̂Ƀf�[�^���Z�b�g */
	static void SetKeyNameArrVal(
		DLLSHAREDATA*, int, short, char*,
		short, short, short, short,
		short, short, short, short );									/* KEYDATA�z��Ƀf�[�^���Z�b�g */
	static void SetKeyNameArrVal( DLLSHAREDATA*, int, short, char* );	/* KEYDATA�z��Ƀf�[�^���Z�b�g */
//	static void SetKeyNames( DLLSHAREDATA* );							/* �L�[���̂̃Z�b�g */
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
//#ifdef _DEBUG
	void TraceOut( LPCTSTR lpFmt, ...);	/* �f�o�b�O���j�^�ɏo�� */
//#endif
	BOOL LoadShareData( void );	/* ���L�f�[�^�̃��[�h */
	void SaveShareData( void );	/* ���L�f�[�^�̕ۑ� */
#if 0
	LONG MY_RegVal_IO(
		BOOL			bRead,
		HKEY			hKey,			// handle of key to set value for
		LPCTSTR			lpValueName,	// address of value to set
		int				nRegCnvID,		//
		BYTE *			lpDataSrc,		// address of value data
		DWORD			cbDataSrc 		// size of value data,
	);
	BOOL ShareData_IO( BOOL );		/* ���L�f�[�^�̕ۑ� */
#endif
	BOOL ShareData_IO_2( BOOL );	/* ���L�f�[�^�̕ۑ� */
	static void IO_ColorSet( CProfile* , BOOL , const char* , ColorInfo* );	/* �F�ݒ� I/O */

//	static BOOL LoadShareData_0_3_5_0( DLLSHAREDATA_0_3_5_0* );	/* Ver0.3.5.0�p�ݒ�f�[�^�̃��[�h */
//	void TakeOver_0_3_5_0( DLLSHAREDATA_0_3_5_0* );				/* Ver0.3.5.0�p�ݒ�f�[�^�������p�� */

	TBBUTTON	m_tbMyButton[MAX_TOOLBARBUTTONS];	/* �c�[���o�[�̃{�^�� */
	int			m_nMyButtonNum;
	int			m_nStdToolBarButtons;

protected:
	/*
	||  �����w���p�֐�
	*/
	const char*		m_pszAppName;
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;

//	long GetModuleDir(char* , long );	/* ���̎��s�t�@�C���̂���f�B���N�g����Ԃ��܂� */
	/* MRU��OPENFOLDER���X�g�̑��݃`�F�b�N�Ȃ�
	���݂��Ȃ��t�@�C����t�H���_��MRU��OPENFOLDER���X�g����폜����
	 */
	void CheckMRUandOPENFOLDERList( void );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */


/*[EOF]*/
