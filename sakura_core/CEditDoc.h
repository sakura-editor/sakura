//	$Id$
/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include <windows.h>
//#include "CMemory.h"  // 2002/2/10 aroka
#include "CDocLineMgr.h"
#include "CLayoutMgr.h"
//#include "COpe.h" // 2002/2/10 aroka
//#include "COpeBlk.h" // 2002/2/10 aroka
#include "COpeBuf.h"
#include "CDlgFind.h"
#include "CDlgReplace.h"
//@@#include "CProp1.h"
#include "CShareData.h"
//#include "CFuncInfoArr.h" // 2002/2/10 aroka
//@@@ 2002.01.14 YAZAKI �s�v
//#include "CSplitBoxWnd.h"
#include "CEditView.h"
#include "CSplitterWnd.h"
#include "CDlgOpenFile.h"
//#include "CDlgSendMail.h"
#include "CDlgGrep.h"
#include "CDlgJump.h"
#include "CPropCommon.h"
#include "CPropTypes.h"
#include "CDlgFuncList.h"
//#include "CDlgTest.h"
#include "CHokanMgr.h"
#include "CAutoSave.h"
//#include "CImageListMgr.h" // 2002/2/10 aroka
//#include "CSMacroMgr.h" // 2002/2/10 aroka
#include "CFuncLookup.h"
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka

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
	BOOL MakeBackUp( void );	/* �o�b�N�A�b�v�̍쐬 */
	void SetParentCaption( BOOL = FALSE );	/* �e�E�B���h�E�̃^�C�g�����X�V */
	BOOL OpenPropertySheet( int/*, int*/ );	/* ���ʐݒ� */
	BOOL OpenPropertySheetTypes( int, int );	/* �^�C�v�ʐݒ� */


	BOOL OpenFileDialog( HWND, const char*, char*, int*, BOOL* );	/* �u�t�@�C�����J���v�_�C�A���O */
	void OnChangeSetting( void );	/* �r���[�ɐݒ�ύX�𔽉f������ */
	void SetReferer( HWND , int, int );	/* �^�O�W�����v���ȂǎQ�ƌ��̏���ێ����� */
	BOOL SaveFileDialog( char*, int*, CEOL* pcEol = NULL );	/* �u�t�@�C������t���ĕۑ��v�_�C�A���O */

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
	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME��Ԃ̐ݒ�

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


protected:
	int				m_nSettingType;
	bool			m_nSettingTypeLocked;	//	������ʂ̈ꎞ�ݒ���
	//	Jan. 22, 2002 genta public -> protected�Ɉړ�
	/*! �ύX�t���O
		@date 2002.01.22 genta public��protected�Ɉړ��D�^��int��bool�ɁD
	*/
	bool			m_bIsModified;

public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	/* ���͕⊮ */
	CHokanMgr		m_cHokanMgr;
	BOOL			m_bGrepRunning;				/* Grep������ */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	BOOL			m_bPrintPreviewMode;		/* ����v���r���[���[�h�� */
	int				m_nCommandExecNum;			/* �R�}���h���s�� */
	char			m_szFilePath[_MAX_PATH];	/* ���ݕҏW���̃t�@�C���̃p�X */
	FILETIME		m_FileTime;					/* �t�@�C���̍ŏI�X�V���t */
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;
	int				m_nCharCode;				/* �����R�[�h��� */

	//	May 15, 2000 genta
	CEOL 			m_cNewLineCode;				//	Enter�������ɑ}��������s�R�[�h���
	CEOL			m_cSaveLineCode;			//	�ۑ����̉��s�R�[�h��ʁiEOL_NONE:�ϊ��Ȃ��j


	BOOL			m_bReadOnly;				/* �ǂݎ���p���[�h */
	BOOL			m_bDebugMode;				/* �f�o�b�O���j�^���[�h */
	BOOL			m_bGrepMode;				/* Grep���[�h�� */
	char			m_szGrepKey[1024];			/* Grep���[�h�̏ꍇ�A���̌����L�[ */
	HWND			m_hWnd;						/* �ҏW�E�B���h�E�n���h���iCSplitterWnd���Ǘ��j */
	COpeBuf			m_cOpeBuf;					/* �A���h�D�o�b�t�@ */
public:
	void			MakeFuncList_C( CFuncInfoArr* );		/* C/C++�֐����X�g�쐬 */
	void 			MakeFuncList_PLSQL( CFuncInfoArr* );	/* PL/SQL�֐����X�g�쐬 */
	void 			MakeTopicList_txt( CFuncInfoArr* );		/* �e�L�X�g�E�g�s�b�N���X�g�쐬 */
	void			MakeFuncList_Java( CFuncInfoArr* );		/* Java�֐����X�g�쐬 */
	void			MakeTopicList_cobol( CFuncInfoArr* );	/* COBOL �A�E�g���C����� */
	void			MakeTopicList_asm( CFuncInfoArr* );		/* �A�Z���u�� �A�E�g���C����� */
	void			MakeFuncList_Perl( CFuncInfoArr* );		/* Perl�֐����X�g�쐬 */	//	Sep. 8, 2000 genta
	void			MakeFuncList_VisualBasic( CFuncInfoArr* );/* Visual Basic�֐����X�g�쐬 */ //June 23, 2001 N.Nakatani
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ���[���t�@�C�����g���ă��X�g�쐬 2002.04.01 YAZAKI */
	int				ReadRuleFile( char* pszFilename, struct oneRule* pcOneRule );	/* ���[���t�@�C���Ǎ� 2002.04.01 YAZAKI */

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
	char			m_szInitialDir[_MAX_PATH + 1];		/* �u�J���v�ł̏����f�B���N�g�� */
	OPENFILENAME	m_ofn;							/* �u�t�@�C�����J���v�_�C�A���O�p�\���� */
	CHOOSEFONT		m_cf;				/* �t�H���g�I���_�C�A���O�p */

//@@	CProp1			m_cProp1;			/* �ݒ�v���p�e�B�V�[�g */
	CPropCommon		m_cPropCommon;
	CPropTypes		m_cPropTypes;

	int				m_nFileShareModeOld;	/* �t�@�C���̔r�����䃂�[�h */
	HFILE			m_hLockedFile;			/* ���b�N���Ă���t�@�C���̃n���h�� */

	HWND			m_hwndReferer;	/* �Q�ƌ��E�B���h�E */
	int				m_nRefererX;	/* �Q�ƌ� �s������̃o�C�g�ʒu�� */
	int				m_nRefererLine;	/* �Q�ƌ��s �܂�Ԃ������̕����s�ʒu */
	
	//	Sep. 29, 2001 genta
	CSMacroMgr*		m_pcSMacroMgr;	//!< �}�N��
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���B
//	CKeyMacroMgr	m_CKeyMacroMgr;	//	�L�[�{�[�h�}�N�� @@@ 2002.1.24 YAZAKI DLLSHAREDATA����ړ�
	//	Oct. 2, 2001 genta
	CFuncLookup		m_cFuncLookup;	//!< �@�\���C�@�\�ԍ��Ȃǂ�resolve

//	CDlgTest*		m_pcDlgTest;

	/*
	||  �����w���p�֐�
	*/
	void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);	//	pszSource��W�J���āApszBuffer�ɃR�s�[�B
protected:
	void DoFileLock( void );	/* �t�@�C���̔r�����b�N */
	void DoFileUnLock( void );	/* �t�@�C���̔r�����b�N���� */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
