/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, MIK, hor
	Copyright (C) 2002, genta, Moca, YAZAKI
	Copyright (C) 2003, genta, ryoji, zenryaku, naoh
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, Moca, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include "global.h"
#include "CAutoSave.h"
#include "CDlgFind.h"
#include "CDlgFuncList.h"
#include "CDlgGrep.h"
#include "CDlgJump.h"
#include "CDlgOpenFile.h"
#include "CDlgReplace.h"
#include "CDocLineMgr.h"
#include "CEditView.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CHokanMgr.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CPropCommon.h"
#include "CPropTypes.h"
#include "CShareData.h"
#include "CSplitterWnd.h"
//#include <windows.h>
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct FileInfo; // 20050705 aroka
class CFuncInfoArr;
struct oneRule; // 2004.04.11 genta �p�����[�^����struct���폜���邽�߁Ddoxygen�΍�

//! �����֘A���̊Ǘ�
//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
class SAKURA_CORE_API CEditDoc
{
public:
	/*
	||  Constructors
	*/
	CEditDoc();
	~CEditDoc();

	/*
	||  �������n�����o�֐�
	*/
	BOOL Create( HINSTANCE, HWND, CImageListMgr* /*, int, int, int, int*/ );
	void Init( void );	/* �����f�[�^�̃N���A */
	void InitAllView();	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */

	/*
	|| ���
	*/
	BOOL IsTextSelected( void );	/* �e�L�X�g���I������Ă��邩 */
	BOOL IsEnableUndo( void );		/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
	BOOL IsEnableRedo( void );		/* Redo(��蒼��)�\�ȏ�Ԃ��H */
	BOOL IsEnablePaste( void );		/* �N���b�v�{�[�h����\��t���\���H */
	void SetFileInfo( FileInfo* );	/* �ҏW�t�@�C�������i�[ */

	/* ���낢�� */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W�f�B�X�p�b�`�� */
	void OnMove( int , int , int , int );
	BOOL OnFileClose( void );	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s */
	BOOL HandleCommand( int );
	void SetActivePane( int );	/* �A�N�e�B�u�ȃy�C����ݒ� */
	int GetActivePane( void );	/* �A�N�e�B�u�ȃy�C�����擾 */
	void RedrawInactivePane( void );	/* ��A�N�e�B�u�ȃy�C����Redraw���� */
	BOOL DetectWidthOfLineNumberAreaAllPane( BOOL bRedraw );	/* ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j */
	BOOL SelectFont( LOGFONT* );
	BOOL FileRead( /*const*/ char* , BOOL*, int, BOOL, BOOL );	/* �t�@�C�����J�� */
	//	Feb. 9, 2001 genta �����ǉ�
	BOOL FileWrite( const char*, enumEOLType cEolType );
	bool SaveFile( const char* path );	//	�t�@�C���̕ۑ��i�ɔ������낢��j
	void OpenFile( const char *filename = NULL, int nCharCode = CODE_AUTODETECT,
		BOOL bReadOnly = FALSE );	//	Oct. 9, 2004 genta CEditView���ړ�

	int MakeBackUp( const char* target_file );	/* �o�b�N�A�b�v�̍쐬 */
	void SetParentCaption( BOOL = FALSE );	/* �e�E�B���h�E�̃^�C�g�����X�V */
	BOOL OpenPropertySheet( int/*, int*/ );	/* ���ʐݒ� */
	BOOL OpenPropertySheetTypes( int, int );	/* �^�C�v�ʐݒ� */

	BOOL OpenFileDialog( HWND, const char*, char*, int*, BOOL* );	/* �u�t�@�C�����J���v�_�C�A���O */
	void OnChangeSetting( void );	/* �r���[�ɐݒ�ύX�𔽉f������ */
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
	void SetReferer( HWND , int, int );	/* �^�O�W�����v���ȂǎQ�ƌ��̏���ێ����� */
#endif
	//	Jul. 26, 2003 ryoji BOM�I�v�V�����ǉ�
	BOOL SaveFileDialog( char*, int*, CEOL* pcEol = NULL, BOOL* pbBomExist = NULL );	/* �u�t�@�C������t���ĕۑ��v�_�C�A���O */

	void CheckFileTimeStamp( void );	/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
	void ReloadCurrentFile( BOOL, BOOL );/* ����t�@�C���̍ăI�[�v�� */

	//	May 15, 2000 genta
	CEOL  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEOL& t){ m_cNewLineCode = t; }

	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );

	//	Aug. 21, 2000 genta
	CPassiveTimer	m_cAutoSave;	//!<	�����ۑ��Ǘ�
	void	CheckAutoSave(void);
	void	ReloadAutoSaveParam(void);	//	�ݒ��SharedArea����ǂݏo��

	//	Aug. 31, 2000 genta
	const CEditView& ActiveView(void) const { return m_cEditViewArr[m_nActivePaneIndex]; }
	CEditView& ActiveView(void) { return m_cEditViewArr[m_nActivePaneIndex]; }
	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME��Ԃ̐ݒ�

	//	Sep. 9, 2002 genta
	const char* GetFilePath(void) const { return m_szFilePath; }
	bool IsFilePathAvailable(void) const { return m_szFilePath[0] != '\0'; }
	//	2002.10.13 Moca
	const char* GetFileName(void) const; //!< �t�@�C����(�p�X�Ȃ�)���擾
	void SetDocumentIcon(void);	// Sep. 10, 2002 genta

	//	Nov. 29, 2000 From Here	genta
	//	�ݒ�̈ꎞ�ύX���Ɋg���q�ɂ�鋭���I�Ȑݒ�ύX�𖳌��ɂ���
	void LockDocumentType(void){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(void){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(void){ return m_nSettingTypeLocked; }
	//	Nov. 29, 2000 To Here
	//	Nov. 23, 2000 From Here	genta
	//	������ʏ��̐ݒ�C�擾Interface
	void SetDocumentType(int type, bool force)	//	������ʂ̐ݒ�
	{
		if( (!m_nSettingTypeLocked) || force ){
			m_nSettingType = type;
			UnlockDocumentType();
			m_pShareData->m_Types[m_nSettingType].m_nRegexKeyMagicNumber++;	//@@@ 2001.11.17 add MIK
			SetDocumentIcon();	// Sep. 11, 2002 genta
		}
	}
	int GetDocumentType(void) const	//!<	������ʂ̓ǂݏo��
	{
		return m_nSettingType;
	}
	Types& GetDocumentAttribute(void) const	//!<	�ݒ肳�ꂽ�������ւ̎Q�Ƃ�Ԃ�
	{
		return m_pShareData->m_Types[m_nSettingType];
	}
	//	Nov. 23, 2000 To Here

	//	May 18, 2001 genta
	//! ReadOnly��Ԃ̓ǂݏo��
	BOOL IsReadOnly( void ){ return m_bReadOnly; }
	//! ReadOnly��Ԃ̐ݒ�
	void SetReadOnly( BOOL flag){ m_bReadOnly = flag; }
	
	//	Jan. 22, 2002 genta Modified Flag�̐ݒ�
	void SetModified( bool flag, bool redraw);
	/** �t�@�C�����C�������ǂ���
		@retval true �t�@�C���͏C������Ă���
		@retval false �t�@�C���͏C������Ă��Ȃ�
	*/
	bool IsModified( void ) const { return m_bIsModified; }

	//	Jan. 28, 2002 genta Modified Flag�̐ݒ� (�b��BOOL Version)
	void SetModified( bool flag, BOOL redraw){
		SetModified( flag, redraw != FALSE );
	}
	void SetModified( BOOL flag, bool redraw){
		SetModified( flag != FALSE, redraw );
	}
	
	/*! @brief ���̃E�B���h�E�ŐV�����t�@�C�����J���邩

		�V�����E�B���h�E���J�����Ɍ��݂̃E�B���h�E���ė��p�ł��邩�ǂ����̃e�X�g���s���D
		�ύX�ς݁C�t�@�C�����J���Ă���CGrep�E�B���h�E�C�A�E�g�v�b�g�E�B���h�E�̏ꍇ�ɂ�
		�ė��p�s�D

		@author Moca
		@date 2005.06.24 Moca
	*/
	bool IsFileOpenInThisWindow( void ){
		return !( IsModified() || IsFilePathAvailable() || m_bGrepMode || m_bDebugMode );
	}

	void AddToMRU(void); // Mar. 30, 2003 genta
	// Mar. 31, 2003 genta ExpandParameter�⏕�֐�
	const char* ExParam_SkipCond(const char* pszSource, int part);
	int ExParam_Evaluate( const char* pCond );

	//	Aug. 14, 2005 genta TAB���Ɛ܂�Ԃ��ʒu�̍X�V
	void ChangeLayoutParam( bool bShowProgress, int nTabSize, int nMaxLineSize );
	
	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

protected:
	int				m_nSettingType;
	bool			m_nSettingTypeLocked;	//	������ʂ̈ꎞ�ݒ���
	//	Jan. 22, 2002 genta public -> protected�Ɉړ�
	/*! �ύX�t���O
		@date 2002.01.22 genta public��protected�Ɉړ��D�^��int��bool�ɁD
	*/
	bool			m_bIsModified;
	//	Sep. 9, 2002 genta protected�Ɉړ�
	char			m_szFilePath[_MAX_PATH];	/* ���ݕҏW���̃t�@�C���̃p�X */
	
	void SetFilePath(const char*);	// Sep. 9, 2002 genta

public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	/* ���͕⊮ */
	CHokanMgr		m_cHokanMgr;
	BOOL			m_bGrepRunning;				/* Grep������ */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	BOOL			m_bPrintPreviewMode;		/* ����v���r���[���[�h�� */
	int				m_nCommandExecNum;			/* �R�}���h���s�� */
	FILETIME		m_FileTime;					/* �t�@�C���̍ŏI�X�V���t */
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;
	int				m_nCharCode;				/* �����R�[�h��� */
	BOOL			m_bBomExist;	//!< �ۑ�����BOM��t���邩�ǂ��� Jul. 26, 2003 ryoji 
	bool			m_bInsMode;		//!< �}���E�㏑�����[�h Oct. 2, 2005 genta

	//	May 15, 2000 genta
protected:
	CEOL 			m_cNewLineCode;				//	Enter�������ɑ}��������s�R�[�h���
public:
	CEOL			m_cSaveLineCode;			//	�ۑ����̉��s�R�[�h��ʁiEOL_NONE:�ϊ��Ȃ��j


	BOOL			m_bReadOnly;				/* �ǂݎ���p���[�h */
	BOOL			m_bDebugMode;				/* �f�o�b�O���j�^���[�h */
	BOOL			m_bGrepMode;				/* Grep���[�h�� */
	char			m_szGrepKey[1024];			/* Grep���[�h�̏ꍇ�A���̌����L�[ */
	HWND			m_hWnd;						/* �ҏW�E�B���h�E�n���h���iCSplitterWnd���Ǘ��j */
	COpeBuf			m_cOpeBuf;					/* �A���h�D�o�b�t�@ */
public:
	void			MakeFuncList_C( CFuncInfoArr*,bool bVisibleMemberFunc = true );		/* C/C++�֐����X�g�쐬 */
	void 			MakeFuncList_PLSQL( CFuncInfoArr* );	/* PL/SQL�֐����X�g�쐬 */
	void 			MakeTopicList_txt( CFuncInfoArr* );		/* �e�L�X�g�E�g�s�b�N���X�g�쐬 */
	void			MakeFuncList_Java( CFuncInfoArr* );		/* Java�֐����X�g�쐬 */
	void			MakeTopicList_cobol( CFuncInfoArr* );	/* COBOL �A�E�g���C����� */
	void			MakeTopicList_asm( CFuncInfoArr* );		/* �A�Z���u�� �A�E�g���C����� */
	void			MakeFuncList_Perl( CFuncInfoArr* );		/* Perl�֐����X�g�쐬 */	//	Sep. 8, 2000 genta
	void			MakeFuncList_VisualBasic( CFuncInfoArr* );/* Visual Basic�֐����X�g�쐬 */ //June 23, 2001 N.Nakatani
	void			MakeTopicList_wztxt(CFuncInfoArr*);		/* �K�w�t���e�L�X�g �A�E�g���C����� */ // 2003.05.20 zenryaku
	void			MakeTopicList_html(CFuncInfoArr*);		/* HTML �A�E�g���C����� */ // 2003.05.20 zenryaku
	void			MakeTopicList_tex(CFuncInfoArr*);		/* TeX �A�E�g���C����� */ // 2003.07.20 naoh
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ���[���t�@�C�����g���ă��X�g�쐬 2002.04.01 YAZAKI */
	int				ReadRuleFile( const char* pszFilename, oneRule* pcOneRule, int nMaxCount );	/* ���[���t�@�C���Ǎ� 2002.04.01 YAZAKI */

// From Here 2001.12.03 hor
	void			MakeFuncList_BookMark( CFuncInfoArr* );	/* �u�b�N�}�[�N���X�g�쐬 */
// To Here 2001.12.03 hor

	CSplitterWnd	m_cSplitterWnd;				/* �����t���[�� */
	CEditView		m_cEditViewArr[4];			/* �r���[ */
	int				m_nActivePaneIndex;			/* �A�N�e�B�u�ȃr���[ */
//	HWND			m_hwndActiveDialog;			/* �A�N�e�B�u�Ȏq�_�C�A���O */
	CDlgFind		m_cDlgFind;					/* �u�����v�_�C�A���O */
	CDlgReplace		m_cDlgReplace;				/* �u�u���v�_�C�A���O */
	CDlgJump		m_cDlgJump;					/* �u�w��s�փW�����v�v�_�C�A���O */
//	CDlgSendMail	m_cDlgSendMail;				/* ���[�����M�_�C�A���O */
	CDlgGrep		m_cDlgGrep;					/* Grep�_�C�A���O */
	CDlgFuncList	m_cDlgFuncList;				/* �A�E�g���C����͌��ʃ_�C�A���O */
	CEditWnd*		m_pcEditWnd;	//	Sep. 10, 2002
	/*
	||  �����o�ϐ�
	*/
	char*			m_pszAppName;		/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HINSTANCE		m_hInstance;		/* �C���X�^���X�n���h�� */
	HWND			m_hwndParent;		/* �e�E�B���h�E�iCEditWnd���Ǘ��j�n���h�� */

	DLLSHAREDATA*	m_pShareData;

//@@@ 2002.01.14 YAZAKI �s�g�p�̂���
//	COpeBlk*		m_pcOpeBlk;			/* ����u���b�N */
	BOOL			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */
	CDlgOpenFile	m_cDlgOpenFile;	/* �t�@�C���I�[�v���_�C�A���O */
	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h */
//	2004.03.28 Moca ���g�p�ϐ��폜

//	2004.02.16 Moca �����o����O��
//	CHOOSEFONT		m_cf;				/* �t�H���g�I���_�C�A���O�p */

//@@	CProp1			m_cProp1;			/* �ݒ�v���p�e�B�V�[�g */
	CPropCommon		m_cPropCommon;
	CPropTypes		m_cPropTypes;

	int				m_nFileShareModeOld;	/* �t�@�C���̔r�����䃂�[�h */
	HFILE			m_hLockedFile;			/* ���b�N���Ă���t�@�C���̃n���h�� */

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
	HWND			m_hwndReferer;	/* �Q�ƌ��E�B���h�E */
	int				m_nRefererX;	/* �Q�ƌ� �s������̃o�C�g�ʒu�� */
	int				m_nRefererLine;	/* �Q�ƌ��s �܂�Ԃ������̕����s�ʒu */
#endif
	
	//	Sep. 29, 2001 genta
	CSMacroMgr*		m_pcSMacroMgr;	//!< �}�N��
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���B
//	CKeyMacroMgr	m_CKeyMacroMgr;	//	�L�[�{�[�h�}�N�� @@@ 2002.1.24 YAZAKI DLLSHAREDATA����ړ�
	//	Oct. 2, 2001 genta
	CFuncLookup		m_cFuncLookup;	//!< �@�\���C�@�\�ԍ��Ȃǂ�resolve

	//	From Here Dec. 4, 2002 genta
	//	�X�V�Ď��ƍX�V�ʒm
	/*!
		�t�@�C�����X�V���ꂽ�ꍇ�ɍēǍ����s�����ǂ����̃t���O
	*/
	enum WatchUpdate {
		WU_QUERY,	//!< �ēǍ����s�����ǂ����_�C�A���O�{�b�N�X�Ŗ₢���킹��
		WU_NOTIFY,	//!< �X�V���ꂽ���Ƃ��X�e�[�^�X�o�[�Œʒm
		WU_NONE		//!< �X�V�Ď����s��Ȃ�
	};
	WatchUpdate		m_eWatchUpdate;	//!< �X�V�Ď����@
	//	To Here Dec. 4, 2002 genta

//	CDlgTest*		m_pcDlgTest;

	/*
	||  �����w���p�֐�
	*/
	void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);	//	pszSource��W�J���āApszBuffer�ɃR�s�[�B
protected:
	void DoFileLock( void );	/* �t�@�C���̔r�����b�N */
	void DoFileUnLock( void );	/* �t�@�C���̔r�����b�N���� */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
	
	//	Aug. 14, 2005 genta
	int* SavePhysPosOfAllView(void);
	void RestorePhysPosOfAllView( int* posary );

	// 2005.11.21 aroka
	bool FormatBackUpPath( char* szNewPath, const char* target_file );	/* �o�b�N�A�b�v�p�X�̍쐬 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
