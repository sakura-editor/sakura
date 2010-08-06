//2007.09.28 kobake Common����

#pragma once

#include "CKeyWordSetMgr.h"
#include "func/CKeyBind.h"
#include "func/CFuncLookup.h" //MacroRec
#include "io/CFile.h" //EShareMode

// Apr. 05, 2003 genta WindowCaption�p�̈�i�ϊ��O�j�̒���
static const int MAX_CAPTION_CONF_LEN = 256;

static const int MAX_DATETIMEFOREMAT_LEN	= 100;
static const int MAX_CUSTOM_MENU			=  25;
static const int MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const int MAX_CUSTOM_MENU_ITEMS		=  48;
static const int MAX_TOOLBAR_BUTTON_ITEMS	= 512;	//�c�[���o�[�ɓo�^�\�ȃ{�^���ő吔	
static const int MAX_TOOLBAR_ICON_X			=  32;	//�A�C�R��BMP�̌���
static const int MAX_TOOLBAR_ICON_Y			=  15;	//�A�C�R��BMP�̒i��
static const int MAX_TOOLBAR_ICON_COUNT		= MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y; // =480
//Oct. 22, 2000 JEPRO �A�C�R���̍ő�o�^����128���₵��(256��384)	
//2010/3/14 Uchi �A�C�R���̍ő�o�^����32���₵��(384��416)
//2010/6/26 syat �A�C�R���̍ő�o�^����15�i�ɑ��₵��(416��480)


// ���łƈႢ�Abool�^�g����悤�ɂ��Ă���܂� by kobake

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �S��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_General
{
	//	Jul. 3, 2000 genta
	//	�A�N�Z�X�֐�(�Ȉ�)
	//	int���r�b�g�P�ʂɕ������Ďg��
	//	��4bit��CaretType�ɓ��ĂĂ���(�����̗\��ő��߂Ɏ���Ă���)
	int		GetCaretType(void) const { return m_nCaretType & 0xf; }
	void	SetCaretType(const int f){ m_nCaretType &= ~0xf; m_nCaretType |= f & 0xf; }

	//�J�[�\��
	int		m_nCaretType;							// �J�[�\���̃^�C�v 0=win 1=dos 
	bool	m_bIsINSMode;							// �}���^�㏑�����[�h
	bool	m_bIsFreeCursorMode;					// �t���[�J�[�\�����[�h��
	BOOL	m_bStopsBothEndsWhenSearchWord;			// �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩
	BOOL	m_bStopsBothEndsWhenSearchParagraph;	// �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩
	BOOL	m_bNoCaretMoveByActivation;				// �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ�  2007.10.02 nasukoji (add by genta)

	//�X�N���[��
	CLayoutInt		m_nRepeatedScrollLineNum;		// �L�[���s�[�g���̃X�N���[���s��
	BOOL	m_nRepeatedScroll_Smooth;		// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	int		m_nPageScrollByWheel;			// �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���Ńy�[�WUP/DOWN����	// 2009.01.17 nasukoji
	int		m_nHorizontalScrollByWheel;		// �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������		// 2009.01.17 nasukoji

	//�^�X�N�g���C
	BOOL	m_bUseTaskTray;					// �^�X�N�g���C�̃A�C�R�����g��
	BOOL	m_bStayTaskTray;				// �^�X�N�g���C�̃A�C�R�����풓
	WORD	m_wTrayMenuHotKeyCode;			// �^�X�N�g���C���N���b�N���j���[ �L�[ */
	WORD	m_wTrayMenuHotKeyMods;			// �^�X�N�g���C���N���b�N���j���[ �L�[ */

	//����
	int		m_nMRUArrNum_MAX;				// �t�@�C���̗���MAX
	int		m_nOPENFOLDERArrNum_MAX;		// �t�H���_�̗���MAX

	//�m�[�J�e�S��
	BOOL	m_bCloseAllConfirm;				// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����	// 2006.12.25 ryoji
	BOOL	m_bExitConfirm;					// �I�����̊m�F������

	//INI���ݒ�̂�
	BOOL	m_bDispExitingDialog;			// �I���_�C�A���O��\������
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �E�B���h�E                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2004.05.13 Moca
//! �E�B���h�E�T�C�Y�E�ʒu�̐�����@
enum EWinSizeMode{
	WINSIZEMODE_DEF		= 0, //!< �w��Ȃ�
	WINSIZEMODE_SAVE	= 1, //!< �p��(�ۑ�)
	WINSIZEMODE_SET		= 2   //!< ���ڎw��(�Œ�)
};

struct CommonSetting_Window
{
	//��{�ݒ�
	BOOL			m_bDispTOOLBAR;				// ����E�B���h�E���J�����Ƃ��c�[���o�[��\������
	BOOL			m_bDispSTATUSBAR;			// ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������
	BOOL			m_bDispFUNCKEYWND;			// ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������
	BOOL			m_bMenuIcon;				// ���j���[�ɃA�C�R����\������ (�A�C�R���t�����j���[)
	BOOL			m_bScrollBarHorz;			// �����X�N���[���o�[���g��
	BOOL			m_bUseCompotibleBMP;		// �č��p�݊��r�b�g�}�b�v���g�� 2007.09.09 Moca

	//�ʒu�Ƒ傫���̐ݒ�
	EWinSizeMode	m_eSaveWindowSize;			// �E�B���h�E�T�C�Y�p���E�Œ� EWinSizeMode�ɏ����� 2004.05.13 Moca
	int				m_nWinSizeType;
	int				m_nWinSizeCX;
	int				m_nWinSizeCY;
	EWinSizeMode	m_eSaveWindowPos;			// �E�B���h�E�ʒu�p���E�Œ� EWinSizeMode�ɏ����� 2004.05.13 Moca
	int				m_nWinPosX;
	int				m_nWinPosY;

	//�t�@���N�V�����L�[
	int				m_nFUNCKEYWND_Place;		// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	int				m_nFUNCKEYWND_GroupNum;		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

	//���[���[�E�s�ԍ�
	int				m_nRulerHeight;				// ���[���[����
	int				m_nRulerBottomSpace;		// ���[���[�ƃe�L�X�g�̌���
	int				m_nRulerType;				// ���[���[�̃^�C�v $$$���g�p���ۂ�
	int				m_nLineNumRightSpace;		// �s�ԍ��̉E�̃X�y�[�X Sep. 18, 2002 genta

	//�����E�B���h�E
	BOOL			m_bSplitterWndHScroll;		// �����E�B���h�E�̐����X�N���[���̓������Ƃ� 2001/06/20 asa-o
	BOOL			m_bSplitterWndVScroll;		// �����E�B���h�E�̐����X�N���[���̓������Ƃ� 2001/06/20 asa-o

	//�^�C�g���o�[
	TCHAR			m_szWindowCaptionActive  [MAX_CAPTION_CONF_LEN];
	TCHAR			m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];

	//INI���ݒ�̂�
	int				m_nVertLineOffset;			// �c���̕`����W�I�t�Z�b�g 2005.11.10 Moca
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �^�u�o�[                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_TabBar
{
	BOOL		m_bDispTabWnd;					//�^�u�E�C���h�E�\������	//@@@ 2003.05.31 MIK
	BOOL		m_bDispTabWndMultiWin;			//�^�u���܂Ƃ߂Ȃ�	//@@@ 2003.05.31 MIK
	BOOL		m_bTab_RetainEmptyWin;			//!< �Ō�̕���������ꂽ�Ƃ�(����)���c��
	BOOL		m_bTab_CloseOneWin;				//!< �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���

	wchar_t		m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//�^�u�E�C���h�E�L���v�V����	//@@@ 2003.06.13 MIK
	BOOL		m_bSameTabWidth;				//�^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
	BOOL		m_bDispTabIcon;					//�^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
	BOOL		m_bSortTabList;					//�^�u�ꗗ���\�[�g����	//@@@ 2006.03.23 fon
	BOOL		m_bTab_ListFull;				//�^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji

	BOOL		m_bChgWndByWheel;				//�}�E�X�z�C�[���ŃE�B���h�E�؂�ւ�	//@@@ 2006.03.26 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ҏW                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Edit
{
	//�R�s�[
	bool	m_bAddCRLFWhenCopy;			// �܂�Ԃ��s�ɉ��s��t���ăR�s�[
	BOOL	m_bEnableNoSelectCopy;		// �I���Ȃ��ŃR�s�[���\�ɂ��� 2007.11.18 ryoji
	BOOL	m_bCopyAndDisablSelection;	// �R�s�[������I������
	bool	m_bEnableLineModePaste;		// ���C�����[�h�\��t�����\�ɂ���  2007.10.08 ryoji

	//�h���b�O���h���b�v
	BOOL	m_bUseOLE_DragDrop;			// OLE�ɂ��h���b�O & �h���b�v���g��
	BOOL	m_bUseOLE_DropSource;		// OLE�ɂ��h���b�O���ɂ��邩

	//�㏑�����[�h
	BOOL	m_bNotOverWriteCRLF;		// ���s�͏㏑�����Ȃ�

	//�N���b�J�u��URL
	BOOL	m_bJumpSingleClickURL;		// URL�̃V���O���N���b�N��Jump $$$���g�p
	BOOL	m_bSelectClickedURL;		// URL���N���b�N���ꂽ��I�����邩


	// (�_�C�A���O���ږ���)
	BOOL	m_bAutoColmnPaste;			// ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �t�@�C��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_File
{
public:
	// �J�[�\���ʒu�𕜌����邩�ǂ���  Oct. 27, 2000 genta
	bool	GetRestoreCurPosition() const		{ return m_bRestoreCurPosition; }
	void	SetRestoreCurPosition(bool i)		{ m_bRestoreCurPosition = i; }

	// �u�b�N�}�[�N�𕜌����邩�ǂ���  2002.01.16 hor
	bool	GetRestoreBookmarks() const			{ return m_bRestoreBookmarks; }
	void	SetRestoreBookmarks(bool i)			{ m_bRestoreBookmarks = i; }

	// �t�@�C���ǂݍ��ݎ���MIME��decode���s����  Nov. 12, 2000 genta
	bool	GetAutoMIMEdecode() const			{ return m_bAutoMIMEdecode; }
	void	SetAutoMIMEdecode(bool i)			{ m_bAutoMIMEdecode = i; }

	// �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��  Oct. 03, 2004 genta
	bool	GetQueryIfCodeChange() const		{ return m_bQueryIfCodeChange; }
	void	SetQueryIfCodeChange(bool i)		{ m_bQueryIfCodeChange = i; }
	
	// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������  Oct. 09, 2004 genta
	bool	GetAlertIfFileNotExist() const		{ return m_bAlertIfFileNotExist; }
	void	SetAlertIfFileNotExist(bool i)		{ m_bAlertIfFileNotExist = i; }

public:
	//�t�@�C���̔r�����䃂�[�h
	EShareMode		m_nFileShareMode;
	bool			m_bCheckFileTimeStamp;	// �X�V�̊Ď�

	//�t�@�C���̕ۑ�
	bool	m_bEnableUnmodifiedOverwrite;	// ���ύX�ł��㏑�����邩

	//�u���O��t���ĕۑ��v�Ńt�@�C���̎�ނ�[���[�U�[�w��]�̂Ƃ��̃t�@�C���ꗗ�\��
	//�t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
	bool	m_bNoFilterSaveNew;				// �V�K����ۑ����͑S�t�@�C���\��
	bool	m_bNoFilterSaveFile;			// �V�K�ȊO����ۑ����͑S�t�@�C���\��

	//�t�@�C���I�[�v��
	bool	m_bDropFileAndClose;			// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	int		m_nDropFileNumMax;				// ��x�Ƀh���b�v�\�ȃt�@�C����
	bool	m_bRestoreCurPosition;			// �t�@�C�����J�����Ƃ��J�[�\���ʒu�𕜌����邩
	bool	m_bRestoreBookmarks;			// �u�b�N�}�[�N�𕜌����邩�ǂ��� 2002.01.16 hor
	bool	m_bAutoMIMEdecode;				// �t�@�C���ǂݍ��ݎ���MIME��decode���s����
	bool	m_bQueryIfCodeChange;			// �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s�� Oct. 03, 2004 genta
	bool	m_bAlertIfFileNotExist;			// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������ Oct. 09, 2004 genta
	bool	m_bAlertIfLargeFile;			// �J�����Ƃ����t�@�C���T�C�Y���傫���o�C�C�Ɍx������
	int		m_nAlertFileSize;				// �x�����n�߂�t�@�C���T�C�Y
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �o�b�N�A�b�v                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	Aug. 15, 2000 genta
//	Backup Flags
enum EBackupOptionFlag{
	BKUP_YEAR		= 32,
	BKUP_MONTH		= 16,
	BKUP_DAY		= 8,
	BKUP_HOUR		= 4,
	BKUP_MIN		= 2,
	BKUP_SEC		= 1,

	//	Aug. 21, 2000 genta
	BKUP_AUTO		= 64,
};

struct CommonSetting_Backup
{
public:
	//	Aug. 15, 2000 genta
	//	Backup�ݒ�̃A�N�Z�X�֐�
	int		GetBackupType(void) const			{ return m_nBackUpType; }
	void	SetBackupType(int n)				{ m_nBackUpType = n; }

	bool	GetBackupOpt(EBackupOptionFlag flag) const			{ return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(EBackupOptionFlag flag, bool value)	{ m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 ); }

	//	�o�b�N�A�b�v��
	int		GetBackupCount(void) const			{ return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value)			{ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff ); }

	//	�o�b�N�A�b�v�̊g���q�擪����(1����)
	int		GetBackupExtChar(void) const		{ return ( m_nBackUpType_Opt2 >> 16 ) & 0xff ; }
	void	SetBackupExtChar(int value)			{ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xff00ffff) | (( value & 0xff ) << 16 ); }

	//	Aug. 21, 2000 genta
	//	����Backup
	bool	IsAutoBackupEnabled(void) const		{ return GetBackupOpt( BKUP_AUTO ); }
	void	EnableAutoBackup(bool flag)			{ SetBackupOpt( BKUP_AUTO, flag ); }

	int		GetAutoBackupInterval(void) const	{ return m_nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i)		{ m_nBackUpType_Opt3 = i; }

	//	Backup�ڍאݒ�̃A�N�Z�X�֐�
	int		GetBackupTypeAdv(void) const { return m_nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n){ m_nBackUpType_Opt4 = n; }

public:
	bool		m_bBackUp;					// �ۑ����Ƀo�b�N�A�b�v���쐬����
	bool		m_bBackUpDialog;			// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	bool		m_bBackUpFolder;			// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
	bool		m_bBackUpFolderRM;			// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����(�����[�o�u�����f�B�A�̂�)
	SFilePath	m_szBackUpFolder;			// �o�b�N�A�b�v���쐬����t�H���_
	int 		m_nBackUpType;				// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
	int 		m_nBackUpType_Opt1;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����1
	int 		m_nBackUpType_Opt2;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����2
	int 		m_nBackUpType_Opt3;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����3
	int 		m_nBackUpType_Opt4;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����4
	int 		m_nBackUpType_Opt5;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����5
	int 		m_nBackUpType_Opt6;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����6
	bool		m_bBackUpDustBox;			// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	bool		m_bBackUpPathAdvanced;		// �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� 20051107 aroka
	SFilePath	m_szBackUpPathAdvanced;		// �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� 20051107 aroka
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format
{
	//���t����
	int			m_nDateFormatType;							//���t�����̃^�C�v
	TCHAR		m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];	//���t����

	//��������
	int			m_nTimeFormatType;							//���������̃^�C�v
	TCHAR		m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];	//��������

	//���o���L��
	wchar_t		m_szMidashiKigou[256];

	//���p��
	wchar_t		m_szInyouKigou[32];
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search
{
	SSearchOption	m_sSearchOption;			// �����^�u��  ����

	int				m_bConsecutiveAll;			// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	int				m_bNOTIFYNOTFOUND;			// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	int				m_bSelectedArea;			// �u��  �I��͈͓��u��

	int				m_bGrepSubFolder;			// Grep: �T�u�t�H���_������
	BOOL			m_bGrepOutputLine;			// Grep: �s���o�͂��邩�Y�����������o�͂��邩
	int				m_nGrepOutputStyle;			// Grep: �o�͌`��
	int				m_bGrepDefaultFolder;		// Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	ECodeType		m_nGrepCharSet;				// Grep: �����R�[�h�Z�b�g // 2002/09/20 Moca Add

	//Grep - Grep
	BOOL			m_bCaretTextForSearch;		// �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� 2006.08.23 ryoji
	TCHAR			m_szRegexpLib[_MAX_PATH];	// �g�p���鐳�K�\��DLL  2007.08.22 genta

	BOOL			m_bGrepExitConfirm;			// Grep���[�h�ŕۑ��m�F���邩
	int				m_bGrepRealTimeView;		// Grep���ʂ̃��A���^�C���\�� 2003.06.16 Moca

	BOOL			m_bGTJW_RETURN;				// �G���^�[�L�[�Ń^�O�W�����v
	BOOL			m_bGTJW_LDBLCLK;			// �_�u���N���b�N�Ń^�O�W�����v

	//�����E�u���_�C�A���O
	BOOL			m_bAutoCloseDlgFind;		// �����_�C�A���O�������I�ɕ���
	BOOL			m_bAutoCloseDlgReplace;		// �u�� �_�C�A���O�������I�ɕ���
	BOOL			m_bSearchAll;				// �擪�i�����j����Č��� 2002.01.26 hor

	//INI���ݒ�̂�
	BOOL			m_bUseCaretKeyWord;			// �L�����b�g�ʒu�̒P�����������		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �L�[���蓖��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//���L���������\����
struct CommonSetting_KeyBind{
	/* �L�[���蓖�� */
	int					m_nKeyNameArrNum;			/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	KEYDATA				m_pKeyNameArr[100];			/* �L�[���蓖�ĕ\ */
	BOOL				m_bCreateAccelTblEachWin;	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)	// 2009.08.15 nasukoji
};





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_CustomMenu
{
	WCHAR			m_szCustMenuNameArr   [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int				m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	EFunctionCode	m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	KEYCODE			m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �c�[���o�[                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_ToolBar
{
	int			m_nToolBarButtonNum;			// �c�[���o�[�{�^���̐�
	int			m_nToolBarButtonIdxArr[MAX_TOOLBAR_BUTTON_ITEMS];	// �c�[���o�[�{�^���\����
	int			m_bToolBarIsFlat;				// �t���b�g�c�[���o�[�ɂ���^���Ȃ�
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �����L�[���[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_SpecialKeyword
{
	/* �����L�[���[�h�ݒ� */
	CKeyWordSetMgr		m_CKeyWordSetMgr;					/* �����L�[���[�h */
	char				m_szKeyWordSetDir[MAX_PATH];		/* �����L�[���[�h�t�@�C���̃f�B���N�g�� */
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �x��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper
{
	//���͕⊮�@�\
	BOOL		m_bHokanKey_RETURN;				// VK_RETURN	�⊮����L�[���L��/����
	BOOL		m_bHokanKey_TAB;				// VK_TAB		�⊮����L�[���L��/����
	BOOL		m_bHokanKey_RIGHT;				// VK_RIGHT		�⊮����L�[���L��/����
	BOOL		m_bHokanKey_SPACE;				// VK_SPACE		�⊮����L�[���L��/���� $$$�قږ��g�p

	//�O���w���v�̐ݒ�
	TCHAR		m_szExtHelp[_MAX_PATH];

	//�O��HTML�w���v�̐ݒ�
	TCHAR		m_szExtHtmlHelp[_MAX_PATH];
	BOOL		m_bHtmlHelpIsSingle;			// HtmlHelp�r���[�A�͂ЂƂ� (�r���[�A�𕡐��N�����Ȃ�)

	//migemo�ݒ�
	TCHAR		m_szMigemoDll[_MAX_PATH];		// migemo dll
	TCHAR		m_szMigemoDict[_MAX_PATH];		// migemo dict

	//�L�[���[�h�w���v
	LOGFONT		m_lf_kh;						// �L�[���[�h�w���v�̃t�H���g���		// ai 02/05/21 Add
	INT			m_ps_kh;						// �L�[���[�h�w���v�̃t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj	// 2009.10.01 ryoji

	//INI���ݒ�̂�
	int			m_bUseHokan;					// ���͕⊮�@�\���g�p����
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �}�N��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Macro
{
	TCHAR			m_szKeyMacroFileName[MAX_PATH];	//!< �L�[�{�[�h�}�N���̃t�@�C����
	MacroRec		m_MacroTable[MAX_CUSTMACRO];	//!< �L�[���蓖�ėp�}�N���e�[�u��	//	Sep. 14, 2001 genta
	SFilePath		m_szMACROFOLDER;	/* �}�N���p�t�H���_ */
	int				m_nMacroOnOpened;			/* �I�[�v���㎩�����s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
	int				m_nMacroOnTypeChanged;		/* �^�C�v�ύX�㎩�����s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
	int				m_nMacroOnSave;		/* �ۑ��O�������s�}�N���ԍ� */	//@@@ 2006.09.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �t�@�C�����\��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_FileName
{
	int			m_nTransformFileNameArrNum;
	TCHAR		m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	TCHAR		m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];	//���C�ɓ���	//@@@ 2003.04.08 MIK
};





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�E�g���C��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �h�b�L���O�z�u
enum EDockSide{
	DOCKSIDE_FLOAT,			//!< �t���[�e�B���O
	DOCKSIDE_LEFT,			//!< ���h�b�L���O
	DOCKSIDE_TOP,			//!< ��h�b�L���O
	DOCKSIDE_RIGHT,			//!< �E�h�b�L���O
	DOCKSIDE_BOTTOM,		//!< ���h�b�L���O
	DOCKSIDE_UNDOCKABLE = -1,//!< �h�b�L���O�֎~
};

struct CommonSetting_OutLine
{
	// 20060201 aroka �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L��
	int			m_bRememberOutlineWindowPos;
	int			m_widthOutlineWindow;
	int			m_heightOutlineWindow;
	int			m_xOutlineWindowPos;
	int			m_yOutlineWindowPos;

	int			m_nOutlineDockSet;			// �A�E�g���C����͂̃h�b�L���O�ʒu�p�����@(0:���ʐݒ�, 1:�^�C�v�ʐݒ�)
	BOOL		m_bOutlineDockSync;			// �A�E�g���C����͂̃h�b�L���O�ʒu�𓯊�����
	BOOL		m_bOutlineDockDisp;			// �A�E�g���C����͕\���̗L��
	EDockSide	m_eOutlineDockSide;			// �A�E�g���C����̓h�b�L���O�z�u
	int			m_cxOutlineDockLeft;		// �A�E�g���C���̍��h�b�L���O��
	int			m_cyOutlineDockTop;			// �A�E�g���C���̏�h�b�L���O��
	int			m_cxOutlineDockRight;		// �A�E�g���C���̉E�h�b�L���O��
	int			m_cyOutlineDockBottom;		// �A�E�g���C���̉��h�b�L���O��

	//IDD_FUNCLIST (�c�[�� - �A�E�g���C�����)
	BOOL		m_bAutoCloseDlgFuncList;	// �A�E�g���C���_�C�A���O�������I�ɕ���
	BOOL		m_bFunclistSetFocusOnJump;	// �t�H�[�J�X���ڂ� 2002.02.08 hor
	BOOL		m_bMarkUpBlankLineEnable;	// ��s�𖳎����� 2002.02.08 aroka,hor
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �t�@�C�����e��r                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare
{
	//�t�@�C�����e��r�_�C�A���O
	BOOL		m_bCompareAndTileHorz;		// ������r��A���E�ɕ��ׂĕ\��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �r���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View
{
	//INI���ݒ�̂�
	LOGFONT		m_lf;						// ���݂̃t�H���g���
	BOOL		m_bFontIs_FIXED_PITCH;		// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	INT			m_nPointSize;				// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj	// 2009.10.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others
{
	//INI���ݒ�̂�
	RECT		m_rcOpenDialog;				// �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �X�e�[�^�X�o�[                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2008/6/21	Uchi
struct CommonSetting_Statusbar
{
	// �������R�[�h�̎w��
	BOOL		m_bDispUniInSjis;				// SJIS�ŕ����R�[�h�l��Unicode�ŕ\������
	BOOL		m_bDispUniInJis;				// JIS�ŕ����R�[�h�l��Unicode�ŕ\������
	BOOL		m_bDispUniInEuc;				// EUC�ŕ����R�[�h�l��Unicode�ŕ\������
	BOOL		m_bDispUtf8Codepoint;			// UTF-8���R�[�h�|�C���g�ŕ\������
	BOOL		m_bDispSPCodepoint;				// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\������
	BOOL		m_bDispSelCountByByte;			// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �v���O�C��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�v���O�C�����
enum EPluginState {
	PLS_NONE,			//!< �v���O�C���e�[�u���ɓo�^���Ȃ�
	PLS_INSTALLED,		//!< �ǉ����ꂽ
	PLS_UPDATED,		//!< �X�V���ꂽ
	PLS_STOPPED,		//!< ��~���Ă���
	PLS_LOADED,			//!< �ǂݍ��܂ꂽ
	PLS_DELETED			//!< �폜���ꂽ
};

struct PluginRec
{
	WCHAR			m_szId[MAX_PLUGIN_ID];		//!< �v���O�C��ID
	WCHAR			m_szName[MAX_PLUGIN_NAME];	//!< �v���O�C���a��
	EPluginState	m_state;					//!< �v���O�C����ԁB�ݒ�t�@�C���ɕۑ�������������̂݁B
	int 			m_nCmdNum;					//!< �v���O�C�� �R�}���h�̐�	// 2010/7/3 Uchi
};

struct CommonSetting_Plugin
{
	BOOL			m_bEnablePlugin;			//!< �v���O�C�����g�p���邩�ǂ���
	PluginRec		m_PluginTable[MAX_PLUGIN];	//!< �v���O�C���e�[�u��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���C�����j���[                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ���
enum EMainMenuType {
	T_NODE,			// Node
	T_LEAF,			// �@�\
	T_SEPARATOR,	// ��ؐ�
	T_SPECIAL,		// ����@�\
}; 

class CMainMenu {
public:
	EMainMenuType	m_nType;		// ���
	EFunctionCode	m_nFunc;		// Function
	WCHAR			m_sKey[2];		// �A�N�Z�X�L�[
	WCHAR			m_sName[MAX_MAIN_MENU_NAME_LEN+1];	// ���O
	int 			m_nLevel;
};

struct CommonSetting_MainMenu
{
	int				m_nMenuTopIdx[MAX_MAINMENU_TOP];	//!< ���C�����j���[�g�b�v���x��
	int 			m_nMainMenuNum;
	CMainMenu		m_cMainMenuTbl[MAX_MAINMENU];		//!< ���C�����j���[�f�[�^
	bool 			m_bMainMenuKeyParentheses;			//!< �A�N�Z�X�L�[��( )�t�ŕ\��
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                                                             //
//                          �܂Ƃ�                             //
//                                                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ���ʐݒ�
struct CommonSetting
{
	CommonSetting_General			m_sGeneral;			// �S��
	CommonSetting_Window			m_sWindow;			// �E�B���h�E
	CommonSetting_TabBar			m_sTabBar;			// �^�u�o�[
	CommonSetting_Edit				m_sEdit;			// �ҏW
	CommonSetting_File				m_sFile;			// �t�@�C��
	CommonSetting_Backup			m_sBackup;			// �o�b�N�A�b�v
	CommonSetting_Format			m_sFormat;			// ����
	CommonSetting_Search			m_sSearch;			// ����
	CommonSetting_KeyBind			m_sKeyBind;			// �L�[���蓖��
	//
	CommonSetting_CustomMenu		m_sCustomMenu;		// �J�X�^�����j���[
	CommonSetting_ToolBar			m_sToolBar;			// �c�[���o�[
	CommonSetting_SpecialKeyword	m_sSpecialKeyword;	// �����L�[���[�h
	CommonSetting_Helper			m_sHelper;			// �x��
	CommonSetting_Macro				m_sMacro;			// �}�N��
	CommonSetting_FileName			m_sFileName;		// �t�@�C�����\��
	//
	CommonSetting_OutLine			m_sOutline;
	CommonSetting_Compare			m_sCompare;
	CommonSetting_View				m_sView;
	CommonSetting_Others			m_sOthers;
	//
	CommonSetting_Statusbar			m_sStatusbar;		// �X�e�[�^�X�o�[		// 2008/6/21 Uchi
	CommonSetting_Plugin			m_sPlugin;			// �v���O�C�� 2009/11/30 syat
	CommonSetting_MainMenu			m_sMainMenu;		// ���C�����j���[		// 2010/5/15 Uchi
};
