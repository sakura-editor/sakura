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
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include "global.h"
#include "CAutoSaveAgent.h"
#include "doc/CDocLineMgr.h"
#include "CEol.h"
#include "func/CFuncLookup.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "env/CShareData.h"
#include <string>
#include "doc/CDocListener.h"
#include "util/design_template.h"
#include "CBackupAgent.h"
#include "CAutoReloadAgent.h"
#include "util/file.h" //CFileTime
#include "io/CFile.h"
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
	//�R���X�g���N�^�E�f�X�g���N�^
	CEditDoc(CEditApp* pcApp);
	~CEditDoc();

	//������
	BOOL Create( CImageListMgr* );
	void InitDoc();	/* �����f�[�^�̃N���A */
	void InitAllView();	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
	void Clear();

	//�ݒ�
	void SetFilePathAndIcon(const TCHAR* szFile);	// Sep. 9, 2002 genta

	//����
	ECodeType	GetDocumentEncoding() const;				//!< �h�L�������g�̕����R�[�h���擾
	void		SetDocumentEncoding(ECodeType eCharCode);	//!< �h�L�������g�̕����R�[�h��ݒ�
	ECodeType	GetDefaultDocumentEncoding() const;			//!< �e������ʂŐݒ肷��f�t�H���g�����R�[�h���擾
	bool IsModificationForbidden( EFunctionCode nCommand );	//!< �w��R�}���h�ɂ�鏑���������֎~����Ă��邩�ǂ���	//Aug. 14, 2000 genta
	void GetSaveInfo(SSaveInfo* pSaveInfo) const;			//!< �Z�[�u�����擾

	//���
	void GetEditInfo( EditInfo* ) const;	//!< �ҏW�t�@�C�������擾 //2007.10.24 kobake �֐����ύX: SetFileInfo��GetEditInfo
	bool IsAcceptLoad() const;				//!< ���̃E�B���h�E��(�V�����E�B���h�E���J������)�V�����t�@�C�����J���邩

	//�C�x���g
	BOOL HandleCommand( EFunctionCode );
	void OnChangeSetting();		/* �r���[�ɐݒ�ύX�𔽉f������ */
	BOOL OnFileClose();			/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s */

	void RunAutoMacro( int idx, LPCTSTR pszSaveFilePath = NULL );	// 2006.09.01 ryoji �}�N���������s

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

	//�������ϐ�
public:
	int				m_nTextWrapMethodCur;		// �܂�Ԃ����@					// 2008.05.30 nasukoji
	bool			m_bTextWrapMethodCurTemp;	// �܂�Ԃ����@�ꎞ�ݒ�K�p��	// 2008.05.30 nasukoji
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */



