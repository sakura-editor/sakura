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
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include "global.h"
#include "CAutoSave.h"
#include "CDocLineMgr.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CShareData.h"
#include <string>

//#include <windows.h>
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct FileInfo; // 20050705 aroka
class CFuncInfoArr;
struct oneRule; // 2004.04.11 genta �p�����[�^����struct���폜���邽�߁Ddoxygen�΍�
class CEditApp;

typedef CEditWnd* P_CEditWnd;

//! �����֘A���̊Ǘ�
//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
class SAKURA_CORE_API CEditDoc
{
public:
	/*
	||  Constructors
	*/
	CEditDoc(CEditApp* pcApp);
	~CEditDoc();

	/*
	||  �������n�����o�֐�
	*/
	BOOL Create( HINSTANCE, CImageListMgr* );
	void Init( void );	/* �����f�[�^�̃N���A */
	void InitAllView();	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */

	/*
	|| ���
	*/
	BOOL IsEnableUndo( void );		/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
	BOOL IsEnableRedo( void );		/* Redo(��蒼��)�\�ȏ�Ԃ��H */
	BOOL IsEnablePaste( void );		/* �N���b�v�{�[�h����\��t���\���H */
	void GetFileInfo( FileInfo* ) const;	/* �ҏW�t�@�C�������i�[ */ //2007.10.24 kobake �֐����ύX: SetFileInfo��GetFileInfo

	/* ���낢�� */
	BOOL OnFileClose( void );	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s */
	BOOL HandleCommand( EFunctionCode );
	BOOL SelectFont( LOGFONT* );

	/* �t�@�C�����J�� */
	bool FileRead(
		TCHAR*		pszPath,			//!< [in/out]
		bool*		pbOpened,			//!< [out] ���łɊJ����Ă�����
		ECodeType	nCharCode,			/*!< [in] �����R�[�h��� */
		bool		bReadOnly,			/*!< [in] �ǂݎ���p�� */
		bool		bConfirmCodeChange	/*!< [in] �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
	);

	//	Feb. 9, 2001 genta �����ǉ�
	BOOL FileWrite( const TCHAR*, enumEOLType cEolType );
	bool SaveFile( const TCHAR* path );	//	�t�@�C���̕ۑ��i�ɔ������낢��j
	void OpenFile( const TCHAR* filename = NULL, ECodeType nCharCode = CODE_AUTODETECT,
		bool bReadOnly = FALSE );	//	Oct. 9, 2004 genta CEditView���ړ�
	void FileClose( void );	/* ����(����) */	// 2006.12.30 ryoji
	void FileCloseOpen( const TCHAR* filename = NULL, ECodeType nCharCode = CODE_AUTODETECT, bool bReadOnly = FALSE );	/* ���ĊJ�� */	// 2006.12.30 ryoji
	BOOL FileSave( bool warnbeep = true, bool askname = true );	/* �㏑���ۑ� */	// 2006.12.30 ryoji
	BOOL FileSaveAs_Dialog( void );				/* ���O��t���ĕۑ��_�C�A���O */	// 2006.12.30 ryoji
	BOOL FileSaveAs( const TCHAR* filename );	/* ���O��t���ĕۑ� */	// 2006.12.30 ryoji

	int MakeBackUp( const TCHAR* target_file );	/* �o�b�N�A�b�v�̍쐬 */
	void SetParentCaption( void );	/* �e�E�B���h�E�̃^�C�g�����X�V */	// 2007.03.08 ryoji bKillFocus�p�����[�^������
	BOOL OpenPropertySheet( int/*, int*/ );	/* ���ʐݒ� */
	BOOL OpenPropertySheetTypes( int, int );	/* �^�C�v�ʐݒ� */

	BOOL OpenFileDialog( HWND, const TCHAR*, TCHAR*, ECodeType*, bool* );	/* �u�t�@�C�����J���v�_�C�A���O */
	void OnChangeSetting( void );	/* �r���[�ɐݒ�ύX�𔽉f������ */
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
	void SetReferer( HWND , int, int );	/* �^�O�W�����v���ȂǎQ�ƌ��̏���ێ����� */
#endif
	//	Jul. 26, 2003 ryoji BOM�I�v�V�����ǉ�
	BOOL SaveFileDialog( TCHAR*, ECodeType*, CEOL* pcEol = NULL, BOOL* pbBomExist = NULL );	/* �u�t�@�C������t���ĕۑ��v�_�C�A���O */

	void CheckFileTimeStamp( void );	/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
	void ReloadCurrentFile( ECodeType, bool );/* ����t�@�C���̍ăI�[�v�� */

	//	May 15, 2000 genta
	CEOL  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEOL& t){ m_cNewLineCode = t; }

	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );

	//	Aug. 21, 2000 genta
	CPassiveTimer	m_cAutoSave;	//!<	�����ۑ��Ǘ�
	void	CheckAutoSave(void);
	void	ReloadAutoSaveParam(void);	//	�ݒ��SharedArea����ǂݏo��

	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME��Ԃ̐ݒ�

	//	Sep. 9, 2002 genta
	const TCHAR* GetFilePath(void) const { return m_szFilePath; }
	bool IsFilePathAvailable(void) const { return m_szFilePath[0] != _T('\0'); }
	//	2002.10.13 Moca
	const TCHAR* GetFileName(void) const; //!< �t�@�C����(�p�X�Ȃ�)���擾
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
	void SetReadOnly( bool flag){ m_bReadOnly = flag; }
	
	//	Jan. 22, 2002 genta Modified Flag�̐ݒ�
	void SetModified( bool flag, bool redraw);
	/** �t�@�C�����C�������ǂ���
		@retval true �t�@�C���͏C������Ă���
		@retval false �t�@�C���͏C������Ă��Ȃ�
	*/
	bool IsModified( void ) const { return m_bIsModified; }

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
	const wchar_t* ExParam_SkipCond(const wchar_t* pszSource, int part);
	int ExParam_Evaluate( const wchar_t* pCond );

	
	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

protected:
	
	void SetFilePath(const TCHAR* szFile);	// Sep. 9, 2002 genta
public:
	void			MakeFuncList_C( CFuncInfoArr*,bool bVisibleMemberFunc = true );		/* C/C++�֐����X�g�쐬 */
	void 			MakeFuncList_PLSQL( CFuncInfoArr* );	/* PL/SQL�֐����X�g�쐬 */
	void 			MakeTopicList_txt( CFuncInfoArr* );		/* �e�L�X�g�E�g�s�b�N���X�g�쐬 */
	void			MakeFuncList_Java( CFuncInfoArr* );		/* Java�֐����X�g�쐬 */
	void			MakeTopicList_cobol( CFuncInfoArr* );	/* COBOL �A�E�g���C����� */
	void			MakeTopicList_asm( CFuncInfoArr* );		/* �A�Z���u�� �A�E�g���C����� */
	void			MakeFuncList_Perl( CFuncInfoArr* );		/* Perl�֐����X�g�쐬 */	//	Sep. 8, 2000 genta
	void			MakeFuncList_VisualBasic( CFuncInfoArr* );/* Visual Basic�֐����X�g�쐬 */ //June 23, 2001 N.Nakatani
	void			MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr ); /* Python �A�E�g���C�����*/ // 2007.02.08 genta
	void			MakeTopicList_wztxt(CFuncInfoArr*);		/* �K�w�t���e�L�X�g �A�E�g���C����� */ // 2003.05.20 zenryaku
	void			MakeTopicList_html(CFuncInfoArr*);		/* HTML �A�E�g���C����� */ // 2003.05.20 zenryaku
	void			MakeTopicList_tex(CFuncInfoArr*);		/* TeX �A�E�g���C����� */ // 2003.07.20 naoh
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ���[���t�@�C�����g���ă��X�g�쐬 2002.04.01 YAZAKI */
	int				ReadRuleFile( const TCHAR* pszFilename, oneRule* pcOneRule, int nMaxCount );	/* ���[���t�@�C���Ǎ� 2002.04.01 YAZAKI */

// From Here 2001.12.03 hor
	void			MakeFuncList_BookMark( CFuncInfoArr* );	/* �u�b�N�}�[�N���X�g�쐬 */
// To Here 2001.12.03 hor

	/*
	||  �����w���p�֐�
	*/
	void ExpandParameter(const WChar* pszSource, WChar* pszBuffer, int nBufferLen);	//	pszSource��W�J���āApszBuffer�ɃR�s�[�B
protected:
	void DoFileLock( void );	/* �t�@�C���̔r�����b�N */
	void DoFileUnLock( void );	/* �t�@�C���̔r�����b�N���� */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
	
	// 2005.11.21 aroka
	bool FormatBackUpPath( TCHAR* szNewPath, const TCHAR* target_file );	/* �o�b�N�A�b�v�p�X�̍쐬 */

public:
	HWND GetSplitterHwnd() const;
	HWND GetOwnerHwnd() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	P_CEditWnd&		m_pcEditWnd;	//	Sep. 10, 2002

	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;

private:
//	HWND			m_hwndParent;		/* �e�E�B���h�E�iCEditWnd���Ǘ��j�n���h�� */

protected:
	int				m_nSettingType;
	bool			m_nSettingTypeLocked;	//	������ʂ̈ꎞ�ݒ���
	bool			m_bIsModified;

private:
	SFilePath		m_szFilePath;	/* ���ݕҏW���̃t�@�C���̃p�X */

public: /* �e�X�g�p�ɃA�N�Z�X������ύX */
	BOOL			m_bGrepRunning;				/* Grep������ */
	int				m_nCommandExecNum;			/* �R�}���h���s�� */
	FILETIME		m_FileTime;					/* �t�@�C���̍ŏI�X�V���t */
	ECodeType		m_nCharCode;				/* �����R�[�h��� */
	BOOL			m_bBomExist;	//!< �ۑ�����BOM��t���邩�ǂ��� Jul. 26, 2003 ryoji 
	bool			m_bInsMode;		//!< �}���E�㏑�����[�h Oct. 2, 2005 genta

	//	May 15, 2000 genta
protected:
	CEOL 			m_cNewLineCode;				//	Enter�������ɑ}��������s�R�[�h���

public:
	CEOL			m_cSaveLineCode;			//	�ۑ����̉��s�R�[�h��ʁiEOL_NONE:�ϊ��Ȃ��j

	bool			m_bReadOnly;				/* �ǂݎ���p���[�h */
	BOOL			m_bDebugMode;				/* �f�o�b�O���j�^���[�h */
	BOOL			m_bGrepMode;				/* Grep���[�h�� */
	wchar_t			m_szGrepKey[1024];			/* Grep���[�h�̏ꍇ�A���̌����L�[ */
public:
	COpeBuf			m_cOpeBuf;					/* �A���h�D�o�b�t�@ */

	TCHAR*			m_pszAppName;		/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HINSTANCE		m_hInstance;		/* �C���X�^���X�n���h�� */
	DLLSHAREDATA*	m_pShareData;
	BOOL			m_bDoing_UndoRedo;	/* �A���h�D�E���h�D�̎��s���� */
	EShareMode	m_nFileShareModeOld;	/* �t�@�C���̔r�����䃂�[�h */
	HANDLE			m_hLockedFile;			/* ���b�N���Ă���t�@�C���̃n���h�� */
	CSMacroMgr*		m_pcSMacroMgr;	//!< �}�N��
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

};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
