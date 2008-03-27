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
#include "CAutoSaveAgent.h"
#include "CDocLineMgr.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CShareData.h"
#include <string>
#include "CDocListener.h"
#include "util/design_template.h"
#include "CBackupAgent.h"
#include "CAutoReloadAgent.h"
#include "util/file.h" //CFileTime
#include "CFile.h"
#include "CDocEditor.h"
#include "CDocFile.h"
#include "CDocFileOperation.h"
#include "CDocOutline.h"
#include "CDocType.h"
#include "CDocLocker.h"

class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct EditInfo; // 20050705 aroka
class CFuncInfoArr;
class CEditApp;

typedef CEditWnd* P_CEditWnd;
/*!
	�����֘A���̊Ǘ�

	@date 2002.02.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2007.12.13 kobake GetDocumentEncoding�쐬
	@date 2007.12.13 kobake SetDocumentEncoding�쐬
	@date 2007.12.13 kobake IsViewMode�쐬
*/
class SAKURA_CORE_API CEditDoc
: public CDocSubject
, public TInstanceHolder<CEditDoc>
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
	BOOL Create( CImageListMgr* );
	void InitDoc();	/* �����f�[�^�̃N���A */
	void InitAllView();	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */

	/*
	|| ���
	*/
	void GetEditInfo( EditInfo* ) const;	/* �ҏW�t�@�C�������i�[ */ //2007.10.24 kobake �֐����ύX: SetFileInfo��GetEditInfo
	void GetSaveInfo(SSaveInfo* pSaveInfo) const;

	/*
	|| ����
	*/
	ECodeType	GetDocumentEncoding() const;				//!< �h�L�������g�̕����R�[�h���擾
	void		SetDocumentEncoding(ECodeType eCharCode);	//!< �h�L�������g�̕����R�[�h��ݒ�

	/* ���낢�� */
	BOOL OnFileClose();	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s */
	BOOL HandleCommand( EFunctionCode );
	BOOL SelectFont( LOGFONT* );



	void OnChangeSetting( void );	/* �r���[�ɐݒ�ύX�𔽉f������ */



	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );

	/*! @brief ���̃E�B���h�E�ŐV�����t�@�C�����J���邩

		�V�����E�B���h�E���J�����Ɍ��݂̃E�B���h�E���ė��p�ł��邩�ǂ����̃e�X�g���s���D
		�ύX�ς݁C�t�@�C�����J���Ă���CGrep�E�B���h�E�C�A�E�g�v�b�g�E�B���h�E�̏ꍇ�ɂ�
		�ė��p�s�D

		@author Moca
		@date 2005.06.24 Moca
	*/
	bool IsAcceptLoad() const;

	void SetFilePathAndIcon(const TCHAR* szFile);	// Sep. 9, 2002 genta

public:
	HWND GetSplitterHwnd() const;
	HWND GetOwnerHwnd() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//�Q��
	P_CEditWnd&		m_pcEditWnd;	//	Sep. 10, 2002

	//�f�[�^�\��
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;

	//�e��@�\
public:
	CDocFile			m_cDocFile;
	CDocFileOperation	m_cDocFileOperation;
	CDocEditor			m_cDocEditor;
	CDocType			m_cDocType;

	//�w���p
public:
	CBackupAgent		m_cBackupAgent;
	CAutoSaveAgent		m_cAutoSaveAgent;		//!< �����ۑ��Ǘ�
	CAutoReloadAgent	m_cAutoReloadAgent;
	CDocOutline			m_cDocOutline;
	CDocLocker			m_cDocLocker;

	//���I���
public:
	int				m_nCommandExecNum;			//!< �R�}���h���s��

	//�����
public:
	CFuncLookup		m_cFuncLookup;				//!< �@�\���C�@�\�ԍ��Ȃǂ�resolve
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
