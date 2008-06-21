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
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, Uchi

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
#include "CDataProfile.h"
#include "basis/SakuraBasis.h"
#include "config/maxdata.h"

//! �ǂ�����ł��A�N�Z�X�ł���A���L�f�[�^�A�N�Z�T�B2007.10.30 kobake
struct DLLSHAREDATA;
DLLSHAREDATA& GetDllShareData();

struct EditNode {
	int				m_nIndex;
	int				m_nGroup;					/*!< �O���[�vID */							//@@@ 2007.06.20 ryoji
	HWND			m_hWnd;
	WIN_CHAR		m_szTabCaption[_MAX_PATH];	/*!< �^�u�E�C���h�E�p�F�L���v�V������ */	//@@@ 2003.05.31 MIK
	SFilePath		m_szFilePath;				/*!< �^�u�E�C���h�E�p�F�t�@�C���� */		//@@@ 2006.01.28 ryoji
	BOOL			m_bIsGrep;					/*!< Grep�̃E�B���h�E�� */					//@@@ 2006.01.28 ryoji
	UINT			m_showCmdRestore;			/*!< ���̃T�C�Y�ɖ߂��Ƃ��̃T�C�Y��� */	//@@@ 2007.06.20 ryoji
	BOOL			m_bClosing;					/*!< �I�������i�u�Ō�̃t�@�C������Ă�(����)���c���v�p�j */	//@@@ 2007.06.20 ryoji

	HWND GetHwnd() const{ return m_hWnd; }
};


//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "recent/CMRU.h"
#include "recent/CMRUFolder.h"

//20020129 aroka
#include "funccode.h"
#include "mem/CMemory.h"

#include "CMutex.h"	// 2007.07.07 genta

#include "doc/CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "doc/CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

#include "EditInfo.h"
#include "doc/CDocTypeSetting.h"


/*!	�����I�v�V����
	20020118 aroka
*/
struct GrepInfo {
	CNativeW		cmGrepKey;			//!< �����L�[
	CNativeT		cmGrepFile;			//!< �����Ώۃt�@�C��
	CNativeT		cmGrepFolder;		//!< �����Ώۃt�H���_
	SSearchOption	sGrepSearchOption;	//!< �����I�v�V����
	bool			bGrepSubFolder;		//!< �T�u�t�H���_����������
	bool			bGrepOutputLine;	//!< ���ʏo�͂ŊY���s���o�͂���
	int				nGrepOutputStyle;	//!< ���ʏo�͌`��
	ECodeType		nGrepCharSet;		//!< �����R�[�h�Z�b�g
};


//! ����ݒ�
#define POS_LEFT	0
#define POS_CENTER	1
#define POS_RIGHT	2
#define HEADER_MAX	100
#define FOOTER_MAX	HEADER_MAX
struct PRINTSETTING {
	TCHAR			m_szPrintSettingName[32 + 1];		/*!< ����ݒ�̖��O */
	TCHAR			m_szPrintFontFaceHan[LF_FACESIZE];	/*!< ����t�H���g */
	TCHAR			m_szPrintFontFaceZen[LF_FACESIZE];	/*!< ����t�H���g */
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
	EDIT_CHAR		m_szHeaderForm[3][HEADER_MAX];		/* 0:���񂹃w�b�_�B1:�����񂹃w�b�_�B2:�E�񂹃w�b�_�B*/
	BOOL			m_bFooterUse[3];					/* �t�b�^���g���Ă��邩�H	*/
	EDIT_CHAR		m_szFooterForm[3][FOOTER_MAX];		/* 0:���񂹃t�b�^�B1:�����񂹃t�b�^�B2:�E�񂹃t�b�^�B*/
};



//! �}�N�����
struct MacroRec {
	TCHAR	m_szName[MACRONAME_MAX];	//!< �\����
	TCHAR	m_szFile[_MAX_PATH+1];	//!< �t�@�C����(�f�B���N�g�����܂܂Ȃ�)
	bool	m_bReloadWhenExecute;	//	���s���ɓǂݍ��݂Ȃ������i�f�t�H���gon�j
	
	bool IsEnabled() const { return m_szFile[0] != _T('\0'); }
	const TCHAR* GetTitle() const { return m_szName[0] == _T('\0') ? m_szFile: m_szName; }	// 2007.11.02 ryoji �ǉ�
};
//	To Here Sep. 14, 2001 genta

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
//! �^�O�W�����v���
struct TagJump {
	HWND		hwndReferer;				//!< �Q�ƌ��E�B���h�E
	CLogicPoint	point;						//!< ���C��, �J����
};

//	Aug. 15, 2000 genta
//	Backup Flags
const int BKUP_YEAR		= 32;
const int BKUP_MONTH	= 16;
const int BKUP_DAY		= 8;
const int BKUP_HOUR		= 4;
const int BKUP_MIN		= 2;
const int BKUP_SEC		= 1;



//	2004.05.13 Moca
//! �E�B���h�E�T�C�Y�E�ʒu�̐�����@
enum eWINSIZEMODE{
	WINSIZEMODE_DEF = 0, //!< �w��Ȃ�
	WINSIZEMODE_SAVE = 1, //!< �p��(�ۑ�)
	WINSIZEMODE_SET = 2   //!< ���ڎw��(�Œ�)
};

//2007.09.28 kobake Common�\���̂�
#include "share/CommonSetting.h"


//! ini�t�H���_�ݒ�	// 2007.05.31 ryoji
struct IniFolder {
	bool m_bInit;							// �������σt���O
	bool m_bReadPrivate;					// �}���`���[�U�pini����̓ǂݏo���t���O
	bool m_bWritePrivate;					// �}���`���[�U�pini�ւ̏������݃t���O
	TCHAR m_szIniFile[_MAX_PATH];			// EXE���ini�t�@�C���p�X
	TCHAR m_szPrivateIniFile[_MAX_PATH];	// �}���`���[�U�p��ini�t�@�C���p�X
};	/* ini�t�H���_�ݒ� */


#include "util/StaticType.h"

#include "basis/CStrictInteger.h"



//! ���L�f�[�^�̈�
//2007.09.23 kobake m_nSEARCHKEYArrNum,      m_szSEARCHKEYArr      �� m_aSearchKeys      �ɂ܂Ƃ߂܂���
//2007.09.23 kobake m_nREPLACEKEYArrNum,     m_szREPLACEKEYArr     �� m_aReplaceKeys     �ɂ܂Ƃ߂܂���
//2007.09.23 kobake m_nGREPFILEArrNum,       m_szGREPFILEArr       �� m_aGrepFiles       �ɂ܂Ƃ߂܂���
//2007.09.23 kobake m_nGREPFOLDERArrNum,     m_szGREPFOLDERArr     �� m_aGrepFolders     �ɂ܂Ƃ߂܂���
//2007.09.23 kobake m_szCmdArr,              m_nCmdArrNum          �� m_aCommands        �ɂ܂Ƃ߂܂���
//2007.09.23 kobake m_nTagJumpKeywordArrNum, m_szTagJumpKeywordArr �� m_aTagJumpKeywords �ɂ܂Ƃ߂܂���
//2007.12.13 kobake DLLSHAREDATA�ւ̊ȈՃA�N�Z�T��p��

struct DLLSHAREDATA{
	void OnInit();

	//	Oct. 27, 2000 genta
	//!	�f�[�^�\�� Version
	/*	�f�[�^�\���̈قȂ�o�[�W�����̓����N����h������
		�K���擪�ɂȂ��Ă͂Ȃ�Ȃ��D
	*/
	unsigned int		m_vStructureVersion;

	/* ���ʍ�ƈ�(�ۑ����Ȃ�) */
	//2007.09.16 kobake char�^���ƁA��ɕ�����ł���Ƃ�������������̂ŁABYTE�^�ɕύX�B�ϐ������ύX�B
	//           UNICODE�łł́A�]���ɗ̈���g�����Ƃ��\�z����邽�߁AANSI�ł�2�{�m�ہB
private:
	BYTE				m_pWork[32000*sizeof(TCHAR)];
public:
	template <class T>
	T* GetWorkBuffer(){ return reinterpret_cast<T*>(m_pWork); }

	template <class T>
	size_t GetWorkBufferCount(){ return sizeof(m_pWork)/sizeof(T); }

	STypeConfig& GetTypeSetting(CTypeConfig cDocumentType)
	{
		int n = cDocumentType.GetIndex();
		assert(n>=0 && n<_countof(m_Types));
		return m_Types[n];
	}

public:
	EditInfo			m_EditInfo_MYWM_GETFILEINFO;

	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
	LONG				m_nSequences;	/* �E�B���h�E�A�� */
	LONG				m_nGroupSequences;	// �^�u�O���[�v�A��	// 2007.06.20 ryoji
	/**** ���ʍ�ƈ�(�ۑ�����) ****/
	int					m_nEditArrNum;	//short->int�ɏC��	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//�ő�l�C��	@@@ 2003.05.31 MIK

	//From Here 2003.05.31 MIK
	//WINDOWPLACEMENT		m_TabWndWndpl;					//�^�u�E�C���h�E���̃E�C���h�E���
	//To Here 2003.05.31 MIK
	BOOL				m_bEditWndChanging;				// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji

//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRU���o�R���Ă��������B
	int					m_nMRUArrNum;
	EditInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//���C�ɓ���	//@@@ 2003.04.08 MIK

//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRUFolder���o�R���Ă��������B
	int						m_nOPENFOLDERArrNum;
	StaticString<TCHAR,_MAX_PATH>	m_szOPENFOLDERArr[MAX_OPENFOLDER];
	bool					m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	int					m_nTransformFileNameArrNum;
	TCHAR				m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	TCHAR				m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_SEARCHKEY,  const WCHAR*>	m_aSearchKeys;
	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_REPLACEKEY, const WCHAR*>	m_aReplaceKeys;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFILE,   const TCHAR*>	m_aGrepFiles;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>	m_aGrepFolders;

	SFilePath			m_szMACROFOLDER;		/* �}�N���p�t�H���_ */
	SFilePath			m_szIMPORTFOLDER;	// �ݒ�C���|�[�g�p�t�H���_
	
	//	Sep. 14, 2001 genta
	MacroRec			m_MacroTable[MAX_CUSTMACRO];	//!< �L�[���蓖�ėp�}�N���e�[�u��

	// 2004/06/21 �^�O�W�����v�@�\�ǉ�
	int					m_TagJumpNum;					//!< �^�O�W�����v���̗L���f�[�^��
	int					m_TagJumpTop;					//!< �X�^�b�N�̈�ԏ�̈ʒu
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< �^�O�W�����v���


	StaticVector< StaticString<TCHAR, MAX_CMDLEN>, MAX_CMDARR > m_aCommands;

	/**** ini�t�H���_�ݒ� ****/
	IniFolder			m_IniFolder;

	/**** ���ʐݒ� ****/
	CommonSetting		m_Common;

	/* �L�[���蓖�� */
	int					m_nKeyNameArrNum;			/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	KEYDATA				m_pKeyNameArr[100];			/* �L�[���蓖�ĕ\ */

	/**** ����y�[�W�ݒ� ****/
	PRINTSETTING		m_PrintSettingArr[MAX_PRINTSETTINGARR];

	/* �����L�[���[�h�ݒ� */
	CKeyWordSetMgr		m_CKeyWordSetMgr;					/* �����L�[���[�h */
	char				m_szKeyWordSetDir[MAX_PATH];		/* �����L�[���[�h�t�@�C���̃f�B���N�g�� */

	// �������p�S�p�L���b�V��
	BYTE				m_bCharWidthCache[0x10000/4];		//16KB �������p�S�p�L���b�V�� 2008/5/16 Uchi
	int					m_nCharWidthCacheTest;				//cache��ꌟ�o

private:
	/* **** �^�C�v�ʐݒ� **** */
	STypeConfig				m_Types[MAX_TYPES];

public:
	/*	@@@ 2002.1.24 YAZAKI
		�L�[�{�[�h�}�N���́A�L�^�I���������_�Ńt�@�C���um_szKeyMacroFileName�v�ɏ����o�����Ƃɂ���B
		m_bRecordingKeyMacro��TRUE�̂Ƃ��́A�L�[�{�[�h�}�N���̋L�^���Ȃ̂ŁAm_szKeyMacroFileName�ɃA�N�Z�X���Ă͂Ȃ�Ȃ��B
	*/
	BOOL				m_bRecordingKeyMacro;		/* �L�[�{�[�h�}�N���̋L�^�� */
	HWND				m_hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	TCHAR				m_szKeyMacroFileName[MAX_PATH];	/* �L�[�{�[�h�}�N���̃t�@�C���� */

//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	/* **** ���̑��̃_�C�A���O **** */
//	BOOL				m_bGetStdout;		/* �O���R�}���h���s�́u�W���o�͂𓾂�v */
	int					m_nExecFlgOpt;		/* �O���R�}���h���s�I�v�V���� */	//	2006.12.03 maru �I�v�V�����̊g���̂���
	BOOL				m_bLineNumIsCRLF;	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */

	int					m_nDiffFlgOpt;		/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
	
	TCHAR				m_szTagsCmdLine[_MAX_PATH];	/* TAGS�R�}���h���C���I�v�V���� */	//@@@ 2003.05.12 MIK
	int					m_nTagsOpt;			/* TAGS�I�v�V����(�`�F�b�N) */	//@@@ 2003.05.12 MIK

	//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_TAGJUMP_KEYWORD >		m_aTagJumpKeywords;

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
	static CMutex g_cEditArrMutex;

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
	CTypeConfig GetDocumentType( const TCHAR* pszFilePath );			/* �t�@�C���p�X��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	CTypeConfig GetDocumentTypeExt( const TCHAR* pszExt );				/* �g���q��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	
	BOOL AddEditWndList( HWND, int nGroup = 0 );				/* �ҏW�E�B���h�E�̓o�^ */	// 2007.06.26 ryoji nGroup�����ǉ�
	void DeleteEditWndList( HWND );								/* �ҏW�E�B���h�E���X�g����̍폜 */
	
	void ResetGroupId( void );									/* �O���[�v��ID���Z�b�g���� */
	EditNode* GetEditNode( HWND hWnd );							/* �ҏW�E�B���h�E�����擾���� */
	int GetGroupId( HWND hWnd );								/* �O���[�vID���擾���� */
	bool IsSameGroup( HWND hWnd1, HWND hWnd2 );					/* ����O���[�v���ǂ����𒲂ׂ� */
	bool ReorderTab( HWND hSrcTab, HWND hDstTab );				/* �^�u�ړ��ɔ����E�B���h�E�̕��ёւ� 2007.07.07 genta */
	HWND SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] );/* �^�u�����ɔ����E�B���h�E���� 2007.07.07 genta */
	EditNode* GetEditNodeAt( int nGroup, int nIndex );			/* �w��ʒu�̕ҏW�E�B���h�E�����擾���� */
	EditNode* GetTopEditNode( HWND hWnd );						/* �擪�̕ҏW�E�B���h�E�����擾���� */
	HWND GetTopEditWnd( HWND hWnd );							/* �擪�̕ҏW�E�B���h�E���擾���� */
	bool IsTopEditWnd( HWND hWnd ){ return (GetTopEditWnd( hWnd ) == hWnd); }	/* �擪�̕ҏW�E�B���h�E���ǂ����𒲂ׂ� */

	BOOL RequestCloseAllEditor( BOOL bExit, int nGroup );		/* �S�ҏW�E�B���h�E�֏I���v�����o�� */	// 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�	// 2007.06.20 ryoji nGroup�����ǉ�
	
	//MRU�n
	BOOL IsPathOpened( const TCHAR* pszPath, HWND* phwndOwner ); /* �w��t�@�C�����J����Ă��邩���ׂ� */
	BOOL ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, ECodeType nCharCode );/* �w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F */	// 2007.03.16

	int GetEditorWindowsNum( int nGroup, bool bExcludeClosing = true );				/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */	// 2007.06.20 ryoji nGroup�����ǉ�	// 2008.04.19 ryoji bExcludeClosing�����ǉ�
	BOOL PostMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast, int nGroup = 0 );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */	// 2007.06.20 ryoji nGroup�����ǉ�
	BOOL SendMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast, int nGroup = 0 );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂邷�� */	// 2007.06.20 ryoji nGroup�����ǉ�
	int GetOpenedWindowArr( EditNode** , BOOL, BOOL bGSort = FALSE );				/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ� */
	static BOOL IsEditWnd( HWND );								/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
	static void SetKeyNameArrVal(
		DLLSHAREDATA*, int, short, TCHAR*,
		EFunctionCode, EFunctionCode, EFunctionCode, EFunctionCode,
		EFunctionCode, EFunctionCode, EFunctionCode, EFunctionCode );									/* KEYDATA�z��Ƀf�[�^���Z�b�g */
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
	void SetTraceOutSource( HWND hwnd ){ m_hwndTraceOutSource = hwnd; }	/* TraceOut�N�����E�B���h�E�̐ݒ� */
	BOOL LoadShareData( void );	/* ���L�f�[�^�̃��[�h */
	void SaveShareData( void );	/* ���L�f�[�^�̕ۑ� */
	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile );	/* �\���ݒ�t�@�C������ini�t�@�C�������擾���� */	// 2007.09.04 ryoji
	void GetIniFileName( LPTSTR pszIniFileName, BOOL bRead = FALSE );	/* ini�t�@�C�����̎擾 */	// 2007.05.19 ryoji
	BOOL IsPrivateSettings( void ){ return m_pShareData->m_IniFolder.m_bWritePrivate; }			/* ini�t�@�C���̕ۑ��悪���[�U�ʐݒ�t�H���_���ǂ��� */	// 2007.05.25 ryoji
	BOOL ShareData_IO_2( bool );	/* ���L�f�[�^�̕ۑ� */
	static void IO_ColorSet( CDataProfile* , const WCHAR* , ColorInfo* );	/* �F�ݒ� I/O */ // Feb. 12, 2006 D.S.Koba

//	int			m_nStdToolBarButtons; 2004.03.30 Moca ���g�p

	//@@@ 2002.2.2 YAZAKI
	//	Jun. 14, 2003 genta �����ǉ��D�����ύX
	int		GetMacroFilename( int idx, TCHAR* pszPath, int nBufLen ); // idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����
	bool		BeReloadWhenExecuteMacro( int idx );	//	idx�Ŏw�肵���}�N���́A���s���邽�тɃt�@�C����ǂݍ��ސݒ肩�H
	void		AddToSearchKeyArr( const wchar_t* pszSearchKey );	//	m_aSearchKeys��pszSearchKey��ǉ�����
	void		AddToReplaceKeyArr( const wchar_t* pszReplaceKey );	//	m_aReplaceKeys��pszReplaceKey��ǉ�����
	void		AddToGrepFileArr( const TCHAR* pszGrepFile );		//	m_aGrepFiles��pszGrepFile��ǉ�����
	void		AddToGrepFolderArr( const TCHAR* pszGrepFolder );	//	m_aGrepFolders.size()��pszGrepFolder��ǉ�����

	//@@@ 2002.2.3 YAZAKI
	bool		ExtWinHelpIsSet( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��ɁA�O���w���v���ݒ肳��Ă��邩�B
	const TCHAR*	GetExtWinHelp( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��́A�O���w���v�t�@�C�������擾�B
	bool		ExtHTMLHelpIsSet( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��ɁA�O��HTML�w���v���ݒ肳��Ă��邩�B
	const TCHAR*	GetExtHTMLHelp( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�t�@�C�������擾�B
	bool		HTMLHelpIsSingle( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�u�r���[�A�𕡐��N�����Ȃ��v��ON�����擾�B
	
	//@@@ 2002.2.9 YAZAKI
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, const TCHAR* szDateFormat );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, const TCHAR* szTimeFormat );
	
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

	// move Export�AImport�Ɏg�p 2008/5/24 Uchi
	void ShareData_IO_CustMenu( CDataProfile&, CommonSetting_CustomMenu&, bool );
	void ShareData_IO_KeyBind( CDataProfile&, int, KEYDATA[], bool);

protected:
	/*
	||  �����w���p�֐�
	*/
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;
	HWND			m_hwndTraceOutSource;	// TraceOutA()�N�����E�B���h�E�i���������N�������w�肵�Ȃ��Ă��ނ悤�Ɂj

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
	bool InitKeyAssign(DLLSHAREDATA*); // 2007.11.04 genta �N�����~�̂��ߒl��Ԃ�
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfigs(DLLSHAREDATA*);
	void InitPopupMenu(DLLSHAREDATA*);
	
	// Feb. 12, 2006 D.S.Koba
	void ShareData_IO_Mru( CDataProfile& );
	void ShareData_IO_Keys( CDataProfile& );
	void ShareData_IO_Grep( CDataProfile& );
	void ShareData_IO_Folders( CDataProfile& );
	void ShareData_IO_Cmd( CDataProfile& );
	void ShareData_IO_Nickname( CDataProfile& );
	void ShareData_IO_Common( CDataProfile& );
	void ShareData_IO_Toolbar( CDataProfile& );
//	void ShareData_IO_CustMenu( CDataProfile& );	move 2008/5/24
	void ShareData_IO_Font( CDataProfile& );
//	void ShareData_IO_KeyBind( CDataProfile& );		move 2008/5/24
	void ShareData_IO_Print( CDataProfile& );
	void ShareData_IO_Types( CDataProfile& );
	void ShareData_IO_KeyWords( CDataProfile& );
	void ShareData_IO_Macro( CDataProfile& );
	void ShareData_IO_Statusbar( CDataProfile& );	// 2008/6/21 Uchi
	void ShareData_IO_Other( CDataProfile& );

	int GetOpenedWindowArrCore( EditNode** , BOOL, BOOL bGSort = FALSE );			/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ��i�R�A�������j */
};




///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */
