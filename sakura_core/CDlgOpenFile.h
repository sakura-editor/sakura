//	$Id$
/************************************************************************

	CDlgOpenFile.h

    �t�@�C���I�[�v���_�C�A���O
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/08/10  �V�K�쐬


************************************************************************/

class CDlgOpenFile;

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include "CShareData.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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
	void Create( HINSTANCE, HWND, const char*, const char*, const char**,const char** );
	BOOL DoModal_GetOpenFileName( char* );	/* �J���_�C�A���O�@���[�_���_�C�A���O�̕\�� */
	BOOL DoModal_GetSaveFileName( char* );	/* �ۑ��_�C�A���O�@���[�_���_�C�A���O�̕\�� */
	BOOL DoModalOpenDlg( char* , int*, BOOL* );	/* �J���_�C�A���O�@���[�_���_�C�A���O�̕\�� */
	BOOL DoModalSaveDlg( char* , int* );	/* �ۑ��_�C�A���O�@���[�_���_�C�A���O�̕\�� */

	BOOL DispatchEvent(	HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND			m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h */
	char			m_szInitialDir[_MAX_PATH + 1];		/* �u�J���v�ł̏����f�B���N�g��	*/
	OPENFILENAME	m_ofn;							/* �u�t�@�C�����J���v�_�C�A���O�p�\���� */
	int				m_nCharCode;					/* �����R�[�h */
//	char			m_szHelpFile[_MAX_PATH + 1];
//	int				m_nHelpTopicID;

protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */

/*[EOF]*/
