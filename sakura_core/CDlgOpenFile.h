//	$Id$
/*!	@file
	@brief �t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/08/10 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgOpenFile;

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include "CShareData.h"
#include "CEol.h"


/*!	�t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CDlgOpenFile
{
public:
	/*
	||  Constructors
	*/
	CDlgOpenFile();
	~CDlgOpenFile();
	/*
	||  Attributes & Operations
	*/
	void Create( HINSTANCE, HWND, const char*, const char*, const char** = NULL,const char** = NULL);
	//void Create( HINSTANCE, HWND, const char*, const char* );
	//	2002/08/21 moca	�����ǉ�
	BOOL DoModal_GetOpenFileName( char*, bool bSetCurDir = false );	/* �J���_�C�A���O ���[�_���_�C�A���O�̕\�� */
	//	2002/08/21 30,2002 moca	�����ǉ�
	BOOL DoModal_GetSaveFileName( char*, bool bSetCurDir = false );	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */
	BOOL DoModalOpenDlg( char* , int*, BOOL* );	/* �J���_�C�A�O ���[�_���_�C�A���O�̕\�� */
	//	Feb. 9, 2001 genta	�����ǉ�
	//	Jul. 26, 2003 ryoji BOM�p�����ǉ�
	BOOL DoModalSaveDlg( char* , int*, CEOL*, BOOL* );	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */

//	INT_PTR DispatchEvent(	HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND			m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h */
	char			m_szInitialDir[_MAX_PATH + 1];		/* �u�J���v�ł̏����f�B���N�g�� */
	OPENFILENAME	m_ofn;							/* �u�t�@�C�����J���v�_�C�A���O�p�\���� */
	int				m_nCharCode;					/* �����R�[�h */
//	char			m_szHelpFile[_MAX_PATH + 1];
//	int				m_nHelpTopicID;
	CEOL			m_cEol;	//	Feb. 9, 2001 genta
	bool			m_bUseEol;	//	Feb. 9, 2001 genta

	//	Jul. 26, 2003 ryoji BOM
	BOOL			m_bBom;	//!< BOM��t���邩�ǂ���
	bool			m_bUseBom;	//!< BOM�̗L����I������@�\�𗘗p���邩�ǂ���

protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */


/*[EOF]*/
