/*!	@file
	@brief �t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/08/10 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2003, ryoji
	Copyright (C) 2004, genta, MIK
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <CommDlg.h>
#include <vector>
#include "CEol.h"
#include "basis/CMyString.h"

struct DLLSHAREDATA;
struct SLoadInfo;	// doc/CDocListener.h
struct SSaveInfo;	// doc/CDocListener.h

// 2005.10.29 ryoji
// Windows 2000 version of OPENFILENAME.
// The new version has three extra members.
// See CommDlg.h
#if (_WIN32_WINNT >= 0x0500)
struct OPENFILENAMEZ : public OPENFILENAME {
};
#else
struct OPENFILENAMEZ : public OPENFILENAME {
  void *        pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
};
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif // (_WIN32_WINNT >= 0x0500)


/*!	�t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CDlgOpenFile
{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CDlgOpenFile();
	~CDlgOpenFile();
	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const TCHAR*				pszUserWildCard,
		const TCHAR*				pszDefaultPath,
		const std::vector<LPCTSTR>& vMRU			= std::vector<LPCTSTR>(),
		const std::vector<LPCTSTR>& vOPENFOLDER		= std::vector<LPCTSTR>()
	);

	//����
	bool DoModal_GetOpenFileName( TCHAR*, bool bSetCurDir = false );	/* �J���_�C�A���O ���[�_���_�C�A���O�̕\�� */	//2002/08/21 moca	�����ǉ�
	bool DoModal_GetSaveFileName( TCHAR*, bool bSetCurDir = false );	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */	//2002/08/21 30,2002 moca	�����ǉ�
	bool DoModalOpenDlg( SLoadInfo* pLoadInfo, std::vector<std::tstring>* );	/* �J���_�C�A�O ���[�_���_�C�A���O�̕\�� */
	bool DoModalSaveDlg( SSaveInfo*	pSaveInfo, bool bSimpleMode );	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */

public:
	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND			m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	DLLSHAREDATA*	m_pShareData;

	SFilePath		m_szDefaultWildCard;	/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h�i�ۑ����̊g���q�⊮�ł��g�p�����j */
	SFilePath		m_szInitialDir;			/* �u�J���v�ł̏����f�B���N�g�� */
	OPENFILENAMEZ	m_ofn;							/* 2005.10.29 ryoji OPENFILENAMEZ�u�t�@�C�����J���v�_�C�A���O�p�\���� */
	ECodeType		m_nCharCode;					/* �����R�[�h */

	CEol			m_cEol;		//	Feb. 9, 2001 genta
	bool			m_bUseEol;	//	Feb. 9, 2001 genta
	
	bool			m_bBom;		//!< BOM��t���邩�ǂ���	//	Jul. 26, 2003 ryoji BOM
	bool			m_bUseBom;	//!< BOM�̗L����I������@�\�𗘗p���邩�ǂ���

	SFilePath		m_szPath;	// �g���q�̕⊮�����O�ōs�����Ƃ��̃t�@�C���p�X	// 2006.11.10 ryoji

protected:
	/*
	||  �����w���p�֐�
	*/

	//	May 29, 2004 genta �G���[�������܂Ƃ߂� (advised by MIK)
	void	DlgOpenFail(void);

	// 2005.11.02 ryoji OS �o�[�W�����Ή��� OPENFILENAME �������p�֐�
	void InitOfn( OPENFILENAMEZ* );

	// 2005.11.02 ryoji �������C�A�E�g�ݒ菈��
	static void InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl );

	// 2005.10.29 ryoji �R���{�{�b�N�X�̃h���b�v�_�E��������
	static void OnCmbDropdown( HWND hwnd );

	// 2006.09.03 Moca �t�@�C���_�C�A���O�̃G���[���
	//! ���g���C�@�\�t�� GetOpenFileName
	bool _GetOpenFileNameRecover( OPENFILENAMEZ* ofn );
	//! ���g���C�@�\�t�� GetOpenFileName
	bool GetSaveFileNameRecover( OPENFILENAMEZ* ofn );

	friend UINT_PTR CALLBACK OFNHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */



